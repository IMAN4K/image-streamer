/*
 * This is a JPEG image streamer implementation over UDP using Qt's GUI backend
 * Copyright (C) 2022 Iman Ahmadvand
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * It is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/

#include "ImageStreamer/Utility.h"

#include <cassert>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QImageWriter>
#include <QtCore/QBuffer>
#include <QtCore/QDebug>

#ifdef Q_OS_LINUX
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#elif defined(Q_OS_WINDOWS)
#include <Windows.h>
#include <wingdi.h>
#pragma comment (lib, "User32.lib")
#pragma comment (lib, "Gdi32.lib")
#include <QtWinExtras>
#endif // Q_OS_LINUX

namespace utility {

static constexpr char IMG_FORMAT[] = "JPEG";

#ifdef Q_OS_LINUX
/* WebCore/plugins/qt/QtX11ImageConversion.cpp */
QImage qimageFromXImage(XImage* xi) {
    QImage::Format format = QImage::Format_ARGB32_Premultiplied;
    if (xi->depth == 24)
        format = QImage::Format_RGB32;
    else if (xi->depth == 16)
        format = QImage::Format_RGB16;

    QImage image = QImage(reinterpret_cast<uchar*>(xi->data), xi->width, xi->height, xi->bytes_per_line, format).copy();

    // we may have to swap the byte order
    if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && xi->byte_order == MSBFirst)
            || (QSysInfo::ByteOrder == QSysInfo::BigEndian && xi->byte_order == LSBFirst)) {

        for (int i = 0; i < image.height(); i++) {
            if (xi->depth == 16) {
                ushort* p = reinterpret_cast<ushort*>(image.scanLine(i));
                ushort* end = p + image.width();
                while (p < end) {
                    *p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
                    p++;
                }
            } else {
                uint* p = reinterpret_cast<uint*>(image.scanLine(i));
                uint* end = p + image.width();
                while (p < end) {
                    *p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000)
                         | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
                    p++;
                }
            }
        }
    }

    // fix-up alpha channel
    if (format == QImage::Format_RGB32) {
        QRgb* p = reinterpret_cast<QRgb*>(image.bits());
        for (int y = 0; y < xi->height; ++y) {
            for (int x = 0; x < xi->width; ++x)
                p[x] |= 0xff000000;
            p += xi->bytes_per_line / 4;
        }
    }

    return image;
}

#endif // Q_OS_LINUX
QPixmap captureScreen(const QRect& area) {
    QRect screen; /* interested display area */
    QImage qimage; /* result image */

#ifdef Q_OS_LINUX
    QPoint cursorPos;

    Display* display = XOpenDisplay(nullptr);
    Window root = DefaultRootWindow(display);

    XWindowAttributes gwa;
    XGetWindowAttributes(display, root, &gwa);

    const auto goodArea = QRect(0, 0, gwa.width, gwa.height).contains(area);
    if (!goodArea) {
        screen = QRect(0, 0, gwa.width, gwa.height);
        cursorPos = QCursor::pos();
    } else {
        screen = area;
        cursorPos = QCursor::pos() - screen.topLeft();
    }

    XImage* image = XGetImage(display, root, screen.x(), screen.y(), screen.width(), screen.height(), AllPlanes, ZPixmap);
    assert(nullptr != image);

    qimage = qimageFromXImage(image);

    /* draw mouse cursor into QImage
     * https://msnkambule.wordpress.com/2010/04/09/capturing-a-screenshot-showing-mouse-cursor-in-kde/
     * https://github.com/rprichard/x11-canvas-screencast/blob/master/CursorX11.cpp#L31
     * */
    {
        XFixesCursorImage* cursor = XFixesGetCursorImage(display);
        cursorPos -= QPoint(cursor->xhot, cursor->yhot);
        std::vector<uint32_t> pixels(cursor->width * cursor->height);
        for (size_t i = 0; i < pixels.size(); ++i)
            pixels[i] = cursor->pixels[i];
        QImage cursorImage((uchar*)(pixels.data()), cursor->width, cursor->height, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&qimage);
        painter.drawImage(cursorPos, cursorImage);
        XFree(cursor);
    }

    XDestroyImage(image);
    XDestroyWindow(display, root);
    XCloseDisplay(display);

#elif defined(Q_OS_WINDOWS)
    HWND hwnd = GetDesktopWindow();
    HDC hdc = GetWindowDC(hwnd);
    HDC hdcMem = CreateCompatibleDC(hdc);

    RECT rect = { 0, 0, GetDeviceCaps(hdc, HORZRES), GetDeviceCaps(hdc, VERTRES) };
    const auto goodArea = QRect(rect.left, rect.top, rect.right, rect.bottom).contains(area);
    if (!goodArea) {
        screen = QRect(rect.left, rect.top, rect.right, rect.bottom);
    } else {
        screen = area;
    }

    HBITMAP hbitmap(nullptr);
    hbitmap = CreateCompatibleBitmap(hdc, screen.width(), screen.height());
    SelectObject(hdcMem, hbitmap);
    BitBlt(hdcMem, 0, 0, screen.width(), screen.height(), hdc, screen.x(), screen.y(), SRCCOPY);

    /* draw mouse cursor into DC
     * https://stackoverflow.com/a/48925443/5446734
     * */
    CURSORINFO cursor = { sizeof(cursor) };
    if (GetCursorInfo(&cursor) && cursor.flags == CURSOR_SHOWING) {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        ICONINFO info = { sizeof(info) };
        GetIconInfo(cursor.hCursor, &info);
        const int x = (cursor.ptScreenPos.x - rect.left - rect.left - info.xHotspot) - screen.left();
        const int y = (cursor.ptScreenPos.y - rect.left - rect.left - info.yHotspot) - screen.top();
        BITMAP bmpCursor = { 0 };
        GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
        DrawIconEx(hdcMem, x, y, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight,
                   0, nullptr, DI_NORMAL);
    }

    qimage = QtWin::imageFromHBITMAP(hdc, hbitmap, screen.width(), screen.height());
#endif // Q_OS_LINUX

    return QPixmap::fromImage(qimage);
}

QPixmap pixmapFromRawData(const QByteArray& data) {
    QPixmap pix;

    if (!pix.loadFromData(data, IMG_FORMAT))
        qWarning() << QStringLiteral("Failed to load image data(%1)").arg(data.size());

    return pix;
}

QByteArray pixmapToRawData(const QPixmap& image, int quality) {
    QByteArray data;

    if (!image.isNull()) {
        QBuffer buff(&data);
        QImageWriter writer(&buff, IMG_FORMAT);
        writer.setQuality(quality);
        writer.setOptimizedWrite(true);
        writer.setProgressiveScanWrite(true);
        writer.write(image.toImage());
    }

    return data;
}
} // namespace utility

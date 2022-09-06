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

#ifndef IMAGE_UTILITY_H
#define IMAGE_UTILITY_H

#include <QtCore/QRect>
#include <QtGui/QPixmap>

namespace utility {
///
/// \brief captureScreen
/// Take an screenshot from primary display screen(include mouse cursor) in specified area
/// \b Note: the image format is JPEG
/// \param area the screen area to take the shot from, defualt to (0, 0, screen.width, screen.height)
/// \return a \l QPixmap object represent the result image
///
QPixmap captureScreen(const QRect& area = QRect());

///
/// \brief pixmapFromRawData
/// Construct a pixmap from raw data
/// \b input data should layout as JPEG image
/// \return valid QPixmap if any
///
QPixmap pixmapFromRawData(const QByteArray&);

///
/// \brief pixmapToRawData
/// Convert given pixmap to raw data with given level of quality
/// \param quality image quality for convert operation
/// \return buffer contain the image data if any
///
QByteArray pixmapToRawData(const QPixmap&, int quality);
} // namespace utility

#endif // IMAGE_UTILITY_H

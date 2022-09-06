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

#include <ImageStreamer/ImageStreamer.h>
#include <ImageStreamer/Utility.h>

#include <QtCore>
#include <QtWidgets>

int main(int argc, char* argv[]) {
    new QApplication(argc, argv);

    QLabel label;

    constexpr auto kPort = 55100;

    ImageStreamer receiver;
    receiver.listen(QHostAddress::LocalHost, kPort);
    QObject::connect(&receiver, QOverload<QPixmap>::of(&ImageStreamer::ready), [&](QPixmap pix) {
        label.setScaledContents(true);
        label.setPixmap(pix);
        label.show();
    });

    ImageStreamer sender;
    sender.update(QHostAddress(QHostAddress::LocalHost), kPort);
    sender.send(utility::captureScreen(), 100);

    return QCoreApplication::exec();
}

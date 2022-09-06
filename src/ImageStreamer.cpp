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

#include "ImageStreamer/ImageStreamer.h"
#include "ImageStreamer/Utility.h"

#include <cassert>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QEvent>
#include <QtCore/QDebug>

static constexpr auto MAX_CHUNK_SIZE = 1400;

ImageStreamer::ImageStreamer(QObject* parent) : QObject(parent), _msd(1), _mss(MAX_CHUNK_SIZE) {
    QObject::connect(&_socket, &QUdpSocket::bytesWritten, this, &ImageStreamer::_onWrite);
    QObject::connect(&_socket, &QUdpSocket::readyRead, this, &ImageStreamer::_onRead);
    QObject::connect(&_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &ImageStreamer::_onError);
}

ImageStreamer::~ImageStreamer() = default;

void ImageStreamer::update(const QHostAddress& host, quint16 port) {
    _host = host;
    _port = port;
}

void ImageStreamer::listen(const QHostAddress& host, quint16 port) {
    if (!_socket.bind(host, port))
        _dbg(QStringLiteral("Failed to bind to %1:%2 (%3)").arg(host.toString()).arg(port).arg(_socket.errorString()));
}

void ImageStreamer::send(const QByteArray& image_data) {
    if (!isStreaming() && !image_data.isEmpty()) {
        _dbg(QStringLiteral("Start sending image data(%1)...").arg(image_data.size()));
        _buffer.clear();
        _buffer.append(image_data);
        _calculateChunkSize(_buffer.size());
        const auto total = static_cast<float>(_buffer.size()) / static_cast<float>(_chunkSize);
        const auto real = static_cast<qint16>(qRound(total));
        _sendTotalChunk(real);
    } else {
        Q_EMIT sent();
    }
}

void ImageStreamer::send(const QPixmap& image, int quality) {
    send(utility::pixmapToRawData(image, quality));
}

bool ImageStreamer::isStreaming() const {
    return !_buffer.isEmpty();
}

quint16 ImageStreamer::getMSD() const {
    return _msd;
}

void ImageStreamer::setMSD(const quint16& MSD) {
    _msd = MSD;
}

quint16 ImageStreamer::getMSS() const {
    return _mss;
}

void ImageStreamer::setMSS(const quint16& MSS) {
    _mss = MSS;
}

void ImageStreamer::_onWrite(qint64) {
    if (!_buffer.isEmpty()) {
        const auto passed = qAbs(_lastSentTime - QDateTime::currentMSecsSinceEpoch());
        if (passed > getMSD()) {
            QTimer::singleShot(1, this, &ImageStreamer::_send);
        } else {
            /* wait to satisfy MSD */
            QTimer::singleShot(qAbs(passed - getMSD()), this, &ImageStreamer::_send);
        }
    } else if (_buffer.isEmpty()) { /* last chunk was sent */
        _dbg(QStringLiteral("Full image data sent."));
        _buffer.clear();
        Q_EMIT sent();
    }
}

void ImageStreamer::_onRead() {
    const auto size(_socket.pendingDatagramSize());

    if (_socket.hasPendingDatagrams() && (size <= MAX_CHUNK_SIZE)) {
        QByteArray chunk;
        {
            char buff[MAX_CHUNK_SIZE] = { 0 };
            assert(MAX_CHUNK_SIZE >= size);
            const auto read = _socket.readDatagram(buff, size);
            chunk = QByteArray::fromRawData(buff, size);
            assert(read == chunk.size());
        }

        if (_totalChunks > 0) {
            _totalChunks--;
            _buffer.append(chunk);
            _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
        } else if (!_buffer.isEmpty()) {
            _dbg(QStringLiteral("Full image data received(%1)").arg(_buffer.size()));
            Q_EMIT ready(_buffer);
            Q_EMIT ready(utility::pixmapFromRawData(_buffer));
            _buffer.clear();
            _totalChunks = 0;
        }

        /* check for total chunk size */
        const auto totalSize = (sizeof (qint16) + sizeof (quint16));
        if (chunk.size() == totalSize) {
            qint16 total = *reinterpret_cast<const qint16*>(chunk.mid(0, 2).constData());
            quint16 checksum = *reinterpret_cast<const quint16*>(chunk.mid(2, 2).constData());
            const quint16 calculated = qChecksum(chunk.constData(), sizeof (total), Qt::ChecksumIso3309);
            if (calculated == checksum && total > 0) {
                _totalChunks = total;
                _dbg(QStringLiteral("Start receiving image data(%1)").arg(total));
                _lastReceiveTime = QDateTime::currentMSecsSinceEpoch();
            } else {
                _dbg(QStringLiteral("Checksum failed!"));
            }
        }
    }
}

void ImageStreamer::_onError(QAbstractSocket::SocketError err) {
    _dbg(QStringLiteral("Socket error: %1").arg(_socket.errorString()));
    _clear();

    switch (err) {
        case QAbstractSocket::ConnectionRefusedError:
            break;
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            break;
        case QAbstractSocket::SocketAccessError:
            break;
        case QAbstractSocket::SocketResourceError:
            break;
        case QAbstractSocket::SocketTimeoutError:
            break;
        case QAbstractSocket::DatagramTooLargeError:
            break;
        case QAbstractSocket::NetworkError:
            break;
        case QAbstractSocket::AddressInUseError:
            break;
        case QAbstractSocket::SocketAddressNotAvailableError:
            break;
        case QAbstractSocket::UnsupportedSocketOperationError:
            break;
        case QAbstractSocket::UnfinishedSocketOperationError:
            break;
        case QAbstractSocket::ProxyAuthenticationRequiredError:
            break;
        case QAbstractSocket::SslHandshakeFailedError:
            break;
        case QAbstractSocket::ProxyConnectionRefusedError:
            break;
        case QAbstractSocket::ProxyConnectionClosedError:
            break;
        case QAbstractSocket::ProxyConnectionTimeoutError:
            break;
        case QAbstractSocket::ProxyNotFoundError:
            break;
        case QAbstractSocket::ProxyProtocolError:
            break;
        case QAbstractSocket::OperationError:
            break;
        case QAbstractSocket::SslInternalError:
            break;
        case QAbstractSocket::SslInvalidUserDataError:
            break;
        case QAbstractSocket::TemporaryError:
            break;
        case QAbstractSocket::UnknownSocketError:
            break;

    }
}

void ImageStreamer::_calculateChunkSize(int /*image_size*/) {
    _chunkSize = MAX_CHUNK_SIZE;
    if (_chunkSize > getMSS())
        _chunkSize = getMSS();

    _dbg(QStringLiteral("Chunksize => %1").arg(_chunkSize));
}

void ImageStreamer::_send() {
    const auto len = (_chunkSize > _buffer.size()) ? _buffer.size() : _chunkSize;
    const auto chunk(_buffer.left(len));
    _buffer.remove(0, len);
    _lastSentTime = QDateTime::currentMSecsSinceEpoch();
    _socket.writeDatagram(chunk, _host, _port);
}

void ImageStreamer::_sendTotalChunk(qint16 total /* total slices the receiver expect */) {
    if (total > 0) {
        QByteArray data;
        data.append(reinterpret_cast<const char*>(&total), sizeof (total));
        const quint16 checksum = qChecksum(data.constData(), static_cast<uint>(data.size()), Qt::ChecksumType::ChecksumIso3309);
        data.append(reinterpret_cast<const char*>(&checksum), sizeof (checksum));
        _socket.writeDatagram(data, data.size(), _host, _port);
        _lastSentTime = QDateTime::currentSecsSinceEpoch();
    }
}

void ImageStreamer::_dbg(const QString& msg) {
#ifndef QT_NO_DEBUG
    qDebug() << "[" << metaObject()->className() << "] " << msg;
#endif
}

void ImageStreamer::_clear() {
    _buffer.clear();
    _totalChunks = -1;
}

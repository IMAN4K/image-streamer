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

#ifndef IMAGE_STREAMER_H
#define IMAGE_STREAMER_H

#include <QtCore/QObject>
#include <QtNetwork/QUdpSocket>
#include <QtGui/QPixmap>

/*!
 * \brief The ImageStreamer class
 * An asynchronous helper class to manage image data streaming(assemble/re-assemble) over UDP link
 * Compressed image data is splitted into chunks with the minimum delay of MSD and maximum size of MSS.
 */
class ImageStreamer : public QObject {
    Q_OBJECT

  public:
    explicit ImageStreamer(QObject* parent = nullptr);
    ~ImageStreamer() override;

    /*!
     * \brief update
     *  Update the ip + port for sending datagrams
     * \param ip the peer host address
     * \param port the peer port number
     */
    void update(const QHostAddress& ip, quint16 port);

    /*!
     * \brief listen
     *  Listen on specified Ip + Port for incoming datagrams
     * \param ip
     * \param port
     */
    void listen(const QHostAddress& ip, quint16 port);

    /*!
     * \brief send
     * Send image data \a image_data over UDP link.
     * \param imageData the compressed image binary data
     */
    void send(const QByteArray& imageData);

    /*!
     * \brief send
     *  An overloaded method to send an image
     * \param image Image to send as an \l QPixmap object
     * \param quality the quality of image to be send(0-100), or -1 for system default
     */
    void send(const QPixmap& image, int quality = 50);

    /*!
     * \brief isStreaming
     *  Check if an image is already on streaming channel or not
     * \return return true if image data is not empty
     */
    bool isStreaming() const;

    /*!
     * \brief setMSD
     *  Set the minimum amount of delay between chunks
     * \b the default value is 1 ms
     * \param the amount of delay in ms
     */
    void setMSD(const quint16& getMSD);
    quint16 getMSD() const;

    /*!
     * \brief setMSS
     *  Set the maximum segment(chunk) size. image data is splitted into sizes not larger than MSS
     * \b the default value is 15360 byte(15KB)
     * \param MSS segment size in byte(B)
     */
    void setMSS(const quint16& getMSS);
    quint16 getMSS() const;

  private Q_SLOTS:
    void _onWrite(qint64);
    void _onRead();
    void _onError(QAbstractSocket::SocketError);
    void _calculateChunkSize(int image_size /* in byte */); /* calculate real chunk(segment) size */
    void _send();
    void _sendTotalChunk(qint16);
    void _dbg(const QString&);
    void _clear();

  Q_SIGNALS:
    /*!
     * \brief sent
     * This signal is triggered whenever an image data completely has been sent
     */
    void sent();

    /*!
     * \brief ready
     * This signal is triggered whenever new image data is available to read
     */
    void ready(QByteArray);

    /*!
     * \brief ready
     * This signal is triggered whenever new valid image is available to read
     */
    void ready(QPixmap);

  private:
    QUdpSocket _socket;
    QByteArray _buffer;

    QHostAddress _host;
    quint16 _port;

    quint16 _msd;
    quint16 _mss;
    quint16 _chunkSize = 0; /* real chunk size */

    qint64 _lastSentTime;
    qint16 _totalChunks = -1; /* notify receiver for total image chunks */

    qint64 _lastReceiveTime;
};

#endif // IMAGE_STREAMER_H

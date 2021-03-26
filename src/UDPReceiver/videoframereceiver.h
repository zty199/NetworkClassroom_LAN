#ifndef VIDEOFRAMERECEIVER_H
#define VIDEOFRAMERECEIVER_H

#include <QThread>
#include <QUdpSocket>
#include <QNetworkInterface>
#include <QImage>

#include "util.h"

class VideoFrameReceiver : public QThread
{
    Q_OBJECT

public:
    VideoFrameReceiver(QNetworkInterface interface, QHostAddress address);
    ~VideoFrameReceiver() override;

protected:
    void run() override;

private:
    QUdpSocket *video_socket;
    QHostAddress groupAddress;
    quint16 video_port;
    QNetworkInterface interface;
    QHostAddress address;

private slots:
    void on_videoReadyRead();

signals:
    void videoFrameReceived(QImage);

};

#endif // VIDEOFRAMERECEIVER_H

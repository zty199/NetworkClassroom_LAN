#ifndef VIDEOFRAMESENDER_H
#define VIDEOFRAMESENDER_H

#include <QRunnable>
#include <QImage>
#include <QUdpSocket>
#include <QNetworkInterface>

#include "util.h"
#include "config.h"

class VideoFrameSender : public QRunnable
{
public:
    explicit VideoFrameSender(QNetworkInterface interface,
                              QHostAddress address,
                              QImage image,
                              QObject *parent = nullptr);
    ~VideoFrameSender() override;

protected:
    void run() override;

private:
    QImage *image;
    QObject *parent;

    QUdpSocket *video_socket;
    QHostAddress groupAddress;
    quint16 video_port;
    QNetworkInterface interface;
    QHostAddress address;

};

#endif // VIDEOFRAMESENDER_H

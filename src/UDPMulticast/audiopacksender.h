#ifndef AUDIOPACKSENDER_H
#define AUDIOPACKSENDER_H

#include <QRunnable>
#include <QUdpSocket>
#include <QNetworkInterface>

#include "util.h"
#include "config.h"

class AudioPackSender : public QRunnable
{
public:
    explicit AudioPackSender(QNetworkInterface interface,
                             QHostAddress address,
                             AudioPack ap);
    ~AudioPackSender() override;

protected:
    void run() override;

private:
    AudioPack *ap;

    QUdpSocket *audio_socket;
    QHostAddress groupAddress;
    quint16 audio_port;
    QNetworkInterface interface;
    QHostAddress address;

};

#endif // AUDIOPACKSENDER_H

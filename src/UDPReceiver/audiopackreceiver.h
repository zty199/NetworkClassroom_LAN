#ifndef AUDIOPACKRECEIVER_H
#define AUDIOPACKRECEIVER_H

#include <QThread>
#include <QUdpSocket>
#include <QNetworkInterface>

#include "util.h"

class AudioPackReceiver : public QThread
{
    Q_OBJECT

public:
    AudioPackReceiver(QNetworkInterface interface, QHostAddress address);
    ~AudioPackReceiver() override;

protected:
    void run() override;

private:
    QUdpSocket *audio_socket;
    QHostAddress groupAddress;
    quint16 audio_port;
    QNetworkInterface interface;
    QHostAddress address;

private slots:
    void on_audioReadyRead();

signals:
    void audioPackReceived(AudioPack);

};

#endif // AUDIOPACKRECEIVER_H

#ifndef AUDIOPACKRECEIVER_H
#define AUDIOPACKRECEIVER_H

#include <QThread>
#include <QUdpSocket>

#include "util.h"

class AudioPackReceiver : public QThread
{
    Q_OBJECT
public:
    AudioPackReceiver();
    ~AudioPackReceiver() override;

protected:
    void run() override;

private:
    QUdpSocket *audio_socket;
    QHostAddress groupAddress;
    quint16 audio_port;

private slots:
    void on_audioReadyRead();

signals:
    void audioPackReceived(AudioPack);

};

#endif // AUDIOPACKRECEIVER_H

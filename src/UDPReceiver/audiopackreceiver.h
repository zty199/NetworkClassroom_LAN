#ifndef AUDIOPACKRECEIVER_H
#define AUDIOPACKRECEIVER_H

#include <QThread>
#include <QUdpSocket>

class AudioPackReceiver : public QThread
{
    Q_OBJECT
public:
    AudioPackReceiver();
    ~AudioPackReceiver();

protected:
    void run() override;

private:
    QUdpSocket *audio_socket;
    QHostAddress groupAddress;
    quint16 audio_port;

    struct PackageHeader
    {
        quint32 uTransPackageHdrSize;
        quint32 uTransPackageSize;
        quint32 uDataSize;
        quint32 uDataPackageNum;
        quint32 uDataPackageCurrIndex;
        quint32 uDataPackageOffset;
    };

    struct AudioPack
    {
        char data[1024 * 16];
        int len;
    };

private slots:
    void on_audioReadyRead();

signals:
    void audioPackReceived(AudioPack);

};

#endif // AUDIOPACKRECEIVER_H

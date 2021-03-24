#ifndef AUDIOPACKRECEIVER_H
#define AUDIOPACKRECEIVER_H

#include <QThread>
#include <QUdpSocket>

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

    struct PackageHeader
    {
        qint32 TransPackageHdrSize;
        qint32 TransPackageSize;
        qint32 DataSize;
        qint32 DataPackageNum;
        qint32 DataPackageCurrIndex;
        qint32 DataPackageOffset;
        qint64 DataPackageTimeStamp;
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

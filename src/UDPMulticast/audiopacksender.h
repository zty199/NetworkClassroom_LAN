#ifndef AUDIOPACKSENDER_H
#define AUDIOPACKSENDER_H

#include <QRunnable>
#include <QUdpSocket>

#define UDP_MAX_SIZE    1200    // UDP 数据包最大长度   * MTU = 1500，故数据包大小 1500 - 20（IP头）- 8（UDP头）

class AudioPackSender : public QRunnable
{
public:
    explicit AudioPackSender(char *ap);
    ~AudioPackSender() override;

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
        char data[1024 * 16];   // 单个音频数据包大小设为 16K，音质 44K/128Kbps（？）
        int len;
    } *ap;

};

#endif // AUDIOPACKSENDER_H

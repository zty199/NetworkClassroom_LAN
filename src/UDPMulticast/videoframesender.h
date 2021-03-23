#ifndef VIDEOFRAMESENDER_H
#define VIDEOFRAMESENDER_H

#include <QRunnable>
#include <QImage>
#include <QUdpSocket>

#define UDP_MAX_SIZE    1200    // UDP 数据包最大长度   * MTU = 1500，故数据包大小 1500 - 20（IP头）- 8（UDP头）

class VideoFrameSender : public QRunnable
{
public:
    explicit VideoFrameSender(QImage image, QObject *parent = nullptr);
    ~VideoFrameSender() override;

protected:
    void run() override;

private:
    QImage *image;
    QObject *parent;

    QUdpSocket *video_socket;
    QHostAddress groupAddress;
    quint16 video_port;

    // UDP 包头
    struct PackageHeader
    {
        quint32 uTransPackageHdrSize;      // 包头大小(sizeof(PackageHeader))
        quint32 uTransPackageSize;         // 当前包头的大小(sizeof(PackageHeader) + 当前数据包长度)
        quint32 uDataSize;                 // 数据的总大小
        quint32 uDataPackageNum;           // 数据被分成包的个数
        quint32 uDataPackageCurrIndex;     // 数据包当前的帧号
        quint32 uDataPackageOffset;        // 数据包在整个数据中的偏移
    };

};

#endif // VIDEOFRAMESENDER_H

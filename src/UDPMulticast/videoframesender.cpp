#include "videoframesender.h"

#include <QBuffer>
#include <QDateTime>

VideoFrameSender::VideoFrameSender(QNetworkInterface interface,
                                   QHostAddress address,
                                   QImage image,
                                   QObject *parent) :
    parent(parent),
    interface(interface),
    address(address)
{
    this->image = new QImage(image);
    setAutoDelete(true);
}

VideoFrameSender::~VideoFrameSender()
{
    video_socket->waitForBytesWritten();
    video_socket->flush();
    video_socket->close();
    delete video_socket;
    delete image;
}

void VideoFrameSender::run()
{
    // 回传图像至主线程（避免 GUI 卡顿）
    QMetaObject::invokeMethod(parent, "on_videoFrameSent", Q_ARG(QImage, *image));

    groupAddress = GROUP_ADDR;
    video_port = VIDEO_PORT;

    // 初始化 video_socket
    video_socket = new QUdpSocket;
    video_socket->bind(address, video_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);   // 绑定地址端口
    video_socket->joinMulticastGroup(groupAddress, interface);                                          // 添加到组播，绑定组播网卡
    video_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);                                  // 尝试优化套接字以降低延迟
    video_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);                              // 设置 TTL 属性
    // video_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);                         // 本机禁止接收
    video_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1920 * 1080 * 16);       // 缓冲区最大存储 4 张 1080p 位图

    // 暂存帧图像
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadWrite);
    image->save(&buffer, "JPEG");
    buffer.close();

    qint64 res;
    qint32 dataLength = buffer.data().size();
    uchar *dataBuffer = (uchar *)buffer.data().data();

    qint32 packetNum = dataLength / PACKET_MAX_SIZE;
    qint32 lastPacketSize = dataLength % PACKET_MAX_SIZE;
    qint32 currentPacketIndex = 0;
    if(lastPacketSize != 0)
    {
        packetNum++;
    }

    PackageHeader packageHead;
    packageHead.TransPackageHdrSize = sizeof(packageHead);
    packageHead.DataSize = dataLength;
    packageHead.DataPackageNum = packetNum;
    packageHead.DataPackageTimeStamp = QDateTime::currentMSecsSinceEpoch();

    uchar frameBuffer[sizeof(packageHead) + PACKET_MAX_SIZE];
    memset(frameBuffer, 0, sizeof(packageHead) + PACKET_MAX_SIZE);

    // 发送空数据包（仅包含帧数据包大小）
    packageHead.TransPackageSize = packageHead.TransPackageHdrSize + PACKET_MAX_SIZE;
    packageHead.DataPackageCurrIndex = 0;
    packageHead.DataPackageOffset = 0;
    memcpy(frameBuffer, &packageHead, packageHead.TransPackageHdrSize);
    res = video_socket->writeDatagram(
                (const char *)frameBuffer, packageHead.TransPackageSize,
                groupAddress, video_port);
    if(res < 0)
    {
        qDebug() << "video_socket: Video Frame Size Send Failed!";
    }

    while(currentPacketIndex < packetNum)
    {
        if(currentPacketIndex < (packetNum - 1))
        {
            packageHead.TransPackageSize = packageHead.TransPackageHdrSize + PACKET_MAX_SIZE;
            packageHead.DataPackageCurrIndex = currentPacketIndex + 1;
            packageHead.DataPackageOffset = currentPacketIndex * PACKET_MAX_SIZE;
            memcpy(frameBuffer, &packageHead, packageHead.TransPackageHdrSize);
            memcpy(frameBuffer + packageHead.TransPackageHdrSize, dataBuffer + packageHead.DataPackageOffset, PACKET_MAX_SIZE);

            res = video_socket->writeDatagram(
                        (const char *)frameBuffer, packageHead.TransPackageSize,
                        groupAddress, video_port);

            if(res < 0)
            {
                qDebug() << "video_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
        else
        {
            packageHead.TransPackageSize = packageHead.TransPackageHdrSize + lastPacketSize;
            packageHead.DataPackageCurrIndex = currentPacketIndex + 1;
            packageHead.DataPackageOffset = currentPacketIndex * PACKET_MAX_SIZE;
            memcpy(frameBuffer, &packageHead, packageHead.TransPackageHdrSize);
            memcpy(frameBuffer + packageHead.TransPackageHdrSize, dataBuffer + packageHead.DataPackageOffset, lastPacketSize);

            res = video_socket->writeDatagram(
                        (const char *)frameBuffer, packageHead.TransPackageSize,
                        groupAddress, video_port);

            if(res < 0)
            {
                qDebug() << "video_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
    }
}

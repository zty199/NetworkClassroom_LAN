#include "videoframesender.h"

#include <QBuffer>

VideoFrameSender::VideoFrameSender(QImage image, QObject *parent) :
    parent(parent)
{
    this->image = new QImage(image);
    setAutoDelete(true);
}

VideoFrameSender::~VideoFrameSender()
{
    delete video_socket;
    delete image;
}

void VideoFrameSender::run()
{
    QMetaObject::invokeMethod(parent, "on_videoFrameSent", Q_ARG(QImage, *image));

    groupAddress = QHostAddress("239.0.0.1");
    video_port = 8888;

    // 初始化 video_socket
    video_socket = new QUdpSocket;
    video_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);                          // 设置套接字属性
    video_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1920 * 1080 * 16);   // 缓冲区最大存储 4 张 1080p 位图

    // 暂存帧图像
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::ReadWrite);
    image->save(&buffer, "JPEG");

    qint64 res;
    quint32 dataLength = buffer.data().size();
    uchar *dataBuffer = (uchar *)buffer.data().data();

    qint32 packetNum = dataLength / UDP_MAX_SIZE;
    qint32 lastPaketSize = dataLength % UDP_MAX_SIZE;
    qint32 currentPacketIndex = 0;
    if(lastPaketSize != 0)
    {
        packetNum++;
    }

    PackageHeader packageHead;
    packageHead.uTransPackageHdrSize = sizeof(packageHead);
    packageHead.uDataSize = dataLength;
    packageHead.uDataPackageNum = packetNum;

    uchar frameBuffer[sizeof(packageHead) + UDP_MAX_SIZE];
    memset(frameBuffer, 0, sizeof(packageHead) + UDP_MAX_SIZE);

    // 发送空数据包（仅包含帧数据包大小）
    packageHead.uTransPackageSize = packageHead.uTransPackageHdrSize + UDP_MAX_SIZE;
    packageHead.uDataPackageCurrIndex = 0;
    packageHead.uDataPackageOffset = 0;
    memcpy(frameBuffer, &packageHead, packageHead.uTransPackageHdrSize);
    res = video_socket->writeDatagram(
                (const char *)frameBuffer, packageHead.uTransPackageSize,
                groupAddress, video_port);
    if(res < 0)
    {
        qDebug() << "video_socket: Video Frame Size Send Failed!";
    }

    while(currentPacketIndex < packetNum)
    {
        if(currentPacketIndex < (packetNum - 1))
        {
            packageHead.uTransPackageSize = packageHead.uTransPackageHdrSize + UDP_MAX_SIZE;
            packageHead.uDataPackageCurrIndex = currentPacketIndex + 1;
            packageHead.uDataPackageOffset = currentPacketIndex * UDP_MAX_SIZE;
            memcpy(frameBuffer, &packageHead, packageHead.uTransPackageHdrSize);
            memcpy(frameBuffer + packageHead.uTransPackageHdrSize, dataBuffer + packageHead.uDataPackageOffset, UDP_MAX_SIZE);

            res = video_socket->writeDatagram(
                        (const char *)frameBuffer, packageHead.uTransPackageSize,
                        groupAddress, video_port);

            if(res < 0)
            {
                qDebug() << "video_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
        else
        {
            packageHead.uTransPackageSize = packageHead.uTransPackageHdrSize + (dataLength - currentPacketIndex * UDP_MAX_SIZE);
            packageHead.uDataPackageCurrIndex = currentPacketIndex + 1;
            packageHead.uDataPackageOffset = currentPacketIndex * UDP_MAX_SIZE;
            memcpy(frameBuffer, &packageHead, packageHead.uTransPackageHdrSize);
            memcpy(frameBuffer + packageHead.uTransPackageHdrSize, dataBuffer + packageHead.uDataPackageOffset, dataLength - currentPacketIndex * UDP_MAX_SIZE);

            res = video_socket->writeDatagram(
                        (const char *)frameBuffer, packageHead.uTransPackageSize,
                        groupAddress, video_port);

            if(res < 0)
            {
                qDebug() << "video_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
    }
}

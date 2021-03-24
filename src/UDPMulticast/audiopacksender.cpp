#include "audiopacksender.h"

#include <QDateTime>

AudioPackSender::AudioPackSender(char *ap)
{
    this->ap = new AudioPack;
    memcpy(this->ap, ap, sizeof(*(this->ap)));
    setAutoDelete(true);
}

AudioPackSender::~AudioPackSender()
{
    delete audio_socket;
    delete ap;
}

void AudioPackSender::run()
{
    groupAddress = QHostAddress("239.0.0.1");
    audio_port = 8889;

    audio_socket = new QUdpSocket;
    audio_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);                  // 设置套接字属性
    audio_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024 * 64);  // 缓冲区最大存储 4个 数据包（单个 16K）

    qint64 res;
    qint32 dataLength = ap->len;
    uchar *dataBuffer = (uchar *)ap->data;

    qint32 packetNum = dataLength / UDP_MAX_SIZE;
    qint32 lastPaketSize = dataLength % UDP_MAX_SIZE;
    qint32 currentPacketIndex = 0;
    if(lastPaketSize != 0)
    {
        packetNum++;
    }

    PackageHeader packageHead;
    packageHead.TransPackageHdrSize = sizeof(packageHead);
    packageHead.DataSize = dataLength;
    packageHead.DataPackageNum = packetNum;
    packageHead.DataPackageTimeStamp = QDateTime::currentMSecsSinceEpoch();

    uchar frameBuffer[sizeof(packageHead) + UDP_MAX_SIZE];
    memset(frameBuffer, 0, sizeof(packageHead) + UDP_MAX_SIZE);

    packageHead.TransPackageSize = packageHead.TransPackageHdrSize + UDP_MAX_SIZE;
    packageHead.DataPackageCurrIndex = 0;
    packageHead.DataPackageOffset = 0;
    memcpy(frameBuffer, &packageHead, packageHead.TransPackageHdrSize);
    res = audio_socket->writeDatagram(
                (const char *)frameBuffer, packageHead.TransPackageSize,
                groupAddress, audio_port);
    if(res < 0)
    {
        qDebug() << "audio_socket: Audio Pack Size Send Failed!";
    }

    while(currentPacketIndex < packetNum)
    {
        if(currentPacketIndex < (packetNum - 1))
        {
            packageHead.TransPackageSize = packageHead.TransPackageHdrSize + UDP_MAX_SIZE;
            packageHead.DataPackageCurrIndex = currentPacketIndex + 1;
            packageHead.DataPackageOffset = currentPacketIndex * UDP_MAX_SIZE;
            memcpy(frameBuffer, &packageHead, packageHead.TransPackageHdrSize);
            memcpy(frameBuffer + packageHead.TransPackageHdrSize, dataBuffer + packageHead.DataPackageOffset, UDP_MAX_SIZE);

            res = audio_socket->writeDatagram(
                        (const char *)frameBuffer, packageHead.TransPackageSize,
                        groupAddress, audio_port);

            if(res < 0)
            {
                qDebug() << "audio_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
        else
        {
            packageHead.TransPackageSize = packageHead.TransPackageHdrSize + (dataLength - currentPacketIndex * UDP_MAX_SIZE);
            packageHead.DataPackageCurrIndex = currentPacketIndex + 1;
            packageHead.DataPackageOffset = currentPacketIndex * UDP_MAX_SIZE;
            memcpy(frameBuffer, &packageHead, packageHead.TransPackageHdrSize);
            memcpy(frameBuffer + packageHead.TransPackageHdrSize, dataBuffer + packageHead.DataPackageOffset, dataLength - currentPacketIndex * UDP_MAX_SIZE);

            res = audio_socket->writeDatagram(
                        (const char *)frameBuffer, packageHead.TransPackageSize,
                        groupAddress, audio_port);

            if(res < 0)
            {
                qDebug() << "video_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
    }
}

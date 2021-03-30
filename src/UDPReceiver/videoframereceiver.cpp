#include "videoframereceiver.h"

VideoFrameReceiver::VideoFrameReceiver(QNetworkInterface interface,
                                       QHostAddress address) :
    interface(interface),
    address(address)
{

}

VideoFrameReceiver::~VideoFrameReceiver()
{
    video_socket->close();
    delete video_socket;
}

void VideoFrameReceiver::run()
{
    groupAddress = QHostAddress("239.0.0.1");
    video_port = 8888;

    video_socket = new QUdpSocket;
    video_socket->bind(address, video_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    video_socket->joinMulticastGroup(groupAddress, interface);
    video_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    video_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1920 * 1080 * 16);

    /*
     * 此处必须指定连接方式为 Qt::DirectConnection，否则 SLOT 函数会在主线程内被执行，而不是子线程
     */
    connect(video_socket, SIGNAL(readyRead()), this, SLOT(on_videoReadyRead()), Qt::DirectConnection);
    exec();
}

void VideoFrameReceiver::on_videoReadyRead()
{
    qint64 res;

    while(video_socket->hasPendingDatagrams())
    {
        QByteArray byteArray;
        byteArray.resize(static_cast<int>(video_socket->pendingDatagramSize()));
        res = video_socket->readDatagram(byteArray.data(), video_socket->pendingDatagramSize());
        if(res < 0)
        {
            qDebug() << "video_socket: Read Datagram Failed!";
            return;
        }

        static qint32 num = 1;
        static qint32 size = 0;
        static qint64 timestamp = 0;
        Q_UNUSED(timestamp)
        static VideoPack vp;

        PackageHeader *packageHead = (PackageHeader *)byteArray.data();
        if(packageHead->DataPackageCurrIndex == 0)
        {
            /*
            vp.len = size;
            QImage image;
            if(image.loadFromData(vp.data, vp.len, "JPEG"))
            {
                emit videoFrameReceived(image);
            }
            */

            // qDebug() << "New Image Arrived!";
            num = 1;
            size = 0;
            timestamp = packageHead->DataPackageTimeStamp;
            memset(&vp, 0, sizeof(vp));
            return;
        }

        /*
        if(packageHead->DataPackageTimeStamp < timestamp)
        {
            return;
        }
        */

        num++;
        size += packageHead->TransPackageSize - packageHead->TransPackageHdrSize;

        if(size > packageHead->DataSize)
        {
            // qDebug() << "image too big 1";
            num = 1;
            size = 0;
            memset(&vp, 0, sizeof(vp));
            return;
        }

        if(packageHead->DataPackageOffset > 1920 * 1080 * 4)
        {
            // qDebug() << "image too big 2";
            num = 1;
            size = 0;
            memset(&vp, 0, sizeof(vp));
            return;
        }

        memcpy(vp.data + packageHead->DataPackageOffset, byteArray.data() + packageHead->TransPackageHdrSize,
               packageHead->TransPackageSize - packageHead->TransPackageHdrSize);

        if((packageHead->DataPackageCurrIndex == packageHead->DataPackageNum) && (size == packageHead->DataSize))
        {
            vp.len = packageHead->DataSize;
            QImage image;
            if(image.loadFromData(vp.data, vp.len, "JPEG"))
            {
                emit videoFrameReceived(image);
            }

            num = 1;
            size = 0;
            memset(&vp, 0, sizeof(vp));
        }
    }
}

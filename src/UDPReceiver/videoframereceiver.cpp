#include "videoframereceiver.h"

VideoFrameReceiver::VideoFrameReceiver()
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
    video_socket->bind(QHostAddress::AnyIPv4, video_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);  // 绑定组播地址端口
    video_socket->joinMulticastGroup(groupAddress);         // 添加到组播，绑定到读套接字上
    video_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1920 * 1080 * 16); // 缓冲区最大存储 4 张 1080p 位图

    /*
     * 此处必须指定连接方式为 Qt::DirectConnection，否则 SLOT 函数会在主线程内被执行，而不是子线程
     */
    connect(video_socket, SIGNAL(readyRead()), this, SLOT(on_videoReadyRead()), Qt::DirectConnection);
    exec();
}

void VideoFrameReceiver::on_videoReadyRead()
{
    qint32 res;

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

        // 接收到停止信号则清空画面
        if(QString(byteArray) == "Stop")
        {
            emit videoFrameReceived(QImage());
            return;
        }

        static quint32 num = 1;
        static quint32 size = 0;
        static VideoPack vp;

        PackageHeader *packageHead = (PackageHeader *)byteArray.data();
        if(packageHead->uDataPackageCurrIndex == 0)
        {
            // qDebug() << "New Image Arrived!";
            num = 1;
            size = 0;
            memset(&vp, 0, sizeof(vp));
            return;
        }

        num++;
        size += packageHead->uTransPackageSize - packageHead->uTransPackageHdrSize;

        if(size > packageHead->uDataSize)
        {
            // qDebug() << "image too big 1";
            num = 1;
            size = 0;
            memset(&vp, 0, sizeof(vp));
            return;
        }

        if(packageHead->uDataPackageOffset > 1920 * 1080 * 4)
        {
            // qDebug() << "image too big 2";
            num = 1;
            size = 0;
            memset(&vp, 0, sizeof(vp));
            return;
        }

        memcpy(vp.data + packageHead->uDataPackageOffset, byteArray.data() + packageHead->uTransPackageHdrSize,
               packageHead->uTransPackageSize - packageHead->uTransPackageHdrSize);

        if((packageHead->uDataPackageCurrIndex == packageHead->uDataPackageNum) && (size == packageHead->uDataSize))
        {
            vp.len = packageHead->uDataSize;
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

#include "audiopackreceiver.h"

AudioPackReceiver::AudioPackReceiver()
{

}

AudioPackReceiver::~AudioPackReceiver()
{
    audio_socket->close();
    delete audio_socket;
}

void AudioPackReceiver::run()
{
    groupAddress = QHostAddress("239.0.0.1");
    audio_port = 8889;

    audio_socket = new QUdpSocket;
    audio_socket->bind(QHostAddress::AnyIPv4, audio_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);  // 绑定组播地址端口
    audio_socket->joinMulticastGroup(groupAddress);         // 添加到组播，绑定到读套接字上
    audio_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 64);       // 缓冲区最大存储 4个 数据包（单个 16K）

    connect(audio_socket, SIGNAL(readyRead()), this, SLOT(on_audioReadyRead()), Qt::DirectConnection);
    exec();
}

void AudioPackReceiver::on_audioReadyRead()
{
    qint32 res;

    while(audio_socket->hasPendingDatagrams())
    {
        QByteArray byteArray;
        byteArray.resize(static_cast<int>(audio_socket->pendingDatagramSize()));
        res = audio_socket->readDatagram(byteArray.data(), audio_socket->pendingDatagramSize());
        if(res < 0)
        {
            qDebug() << "audio_socket: Read Datagram Failed!";
            return;
        }

        static quint32 num = 1;
        static quint32 size = 0;
        static AudioPack ap;

        PackageHeader *packageHead = (PackageHeader *)byteArray.data();
        if(packageHead->uDataPackageCurrIndex == 0)
        {
            // qDebug() << "New Packet Arrived!";
            num = 1;
            size = 0;
            memset(&ap, 0, sizeof(ap));
            return;
        }

        num++;
        size += packageHead->uTransPackageSize - packageHead->uTransPackageHdrSize;

        if(size > packageHead->uDataSize)
        {
            // qDebug() << "packet too big 1";
            num = 1;
            size = 0;
            memset(&ap, 0, sizeof(ap));
            return;
        }

        if(packageHead->uDataPackageOffset > 1024 * 16)
        {
            // qDebug() << "packet too big 2";
            num = 1;
            size = 0;
            memset(&ap, 0, sizeof(ap));
            return;
        }

        memcpy(ap.data + packageHead->uDataPackageOffset, byteArray.data() + packageHead->uTransPackageHdrSize,
               packageHead->uTransPackageSize - packageHead->uTransPackageHdrSize);

        if((packageHead->uDataPackageCurrIndex == packageHead->uDataPackageNum) && (size == packageHead->uDataSize))
        {
            ap.len = packageHead->uDataSize;
            emit audioPackReceived(ap);

            num = 1;
            size = 0;
            memset(&ap, 0, sizeof(ap));
        }
    }
}

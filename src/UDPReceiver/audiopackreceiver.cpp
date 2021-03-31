#include "audiopackreceiver.h"

AudioPackReceiver::AudioPackReceiver(QNetworkInterface interface,
                                     QHostAddress address) :
    interface(interface),
    address(address)
{

}

AudioPackReceiver::~AudioPackReceiver()
{
    audio_socket->close();
    delete audio_socket;
}

void AudioPackReceiver::run()
{
    groupAddress = GROUP_ADDR;
    audio_port = AUDIO_PORT;

    audio_socket = new QUdpSocket;
    audio_socket->bind(address, audio_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    audio_socket->joinMulticastGroup(groupAddress, interface);
    audio_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    audio_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 64);

    connect(audio_socket, SIGNAL(readyRead()), this, SLOT(on_audioReadyRead()), Qt::DirectConnection);
    exec();
}

void AudioPackReceiver::on_audioReadyRead()
{
    qint64 res;

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

        static qint32 num = 1;
        static qint32 size = 0;
        static qint64 timestamp = 0;
        Q_UNUSED(timestamp)
        static AudioPack ap;

        PackageHeader *packageHead = (PackageHeader *)byteArray.data();
        if(packageHead->DataPackageCurrIndex == 0)
        {
            /*
            ap.len = size;
            emit audioPackReceived(ap);
            */

            // qDebug() << "New Packet Arrived!";
            num = 1;
            size = 0;
            timestamp = packageHead->DataPackageTimeStamp;
            memset(&ap, 0, sizeof(ap));
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
            // qDebug() << "packet too big 1";
            num = 1;
            size = 0;
            memset(&ap, 0, sizeof(ap));
            return;
        }

        if(packageHead->DataPackageOffset > 1024 * 16)
        {
            // qDebug() << "packet too big 2";
            num = 1;
            size = 0;
            memset(&ap, 0, sizeof(ap));
            return;
        }

        memcpy(ap.data + packageHead->DataPackageOffset, byteArray.data() + packageHead->TransPackageHdrSize,
               packageHead->TransPackageSize - packageHead->TransPackageHdrSize);

        if((packageHead->DataPackageCurrIndex == packageHead->DataPackageNum) && (size == packageHead->DataSize))
        {
            ap.len = packageHead->DataSize;
            emit audioPackReceived(ap);

            num = 1;
            size = 0;
            memset(&ap, 0, sizeof(ap));
        }
    }
}

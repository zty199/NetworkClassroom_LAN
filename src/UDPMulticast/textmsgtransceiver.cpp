#include "textmsgtransceiver.h"

#include <QDateTime>

TextMsgTransceiver::TextMsgTransceiver(QNetworkInterface interface,
                                       QHostAddress address,
                                       QString name) :
    interface(interface),
    address(address),
    name(name)
{

}

TextMsgTransceiver::~TextMsgTransceiver()
{
    text_socket->close();
    delete text_socket;
}

void TextMsgTransceiver::run()
{
    groupAddress = QHostAddress("239.0.0.1");
    text_port = 8890;

    text_socket = new QUdpSocket;
    text_socket->bind(address, text_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    text_socket->joinMulticastGroup(groupAddress, interface);
    text_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    text_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    // text_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
    text_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024 * 64);
    text_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 64);

    connect(text_socket, SIGNAL(readyRead()), this, SLOT(on_textReadyRead()), Qt::DirectConnection);

    QString datetime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    QString body = name + "[" + address.toString() + "] Connected\t" + datetime + "\n";
    QString msg = body + "\n";

    qint64 res;
    res = text_socket->writeDatagram(msg.toUtf8().data(), msg.size(), groupAddress, text_port);
    if(res < 0)
    {
        qDebug() << "text_socket: Connect Msg Send Failed!";
    }
    else
    {
        emit textAppend(msg);
    }

    exec();
}

void TextMsgTransceiver::on_textReadyRead()
{
    qint64 res;

    while(text_socket->hasPendingDatagrams())
    {
        QByteArray byteArray;
        byteArray.resize(text_socket->pendingDatagramSize());
        res = text_socket->readDatagram(byteArray.data(), byteArray.size());
        if(res < 0)
        {
            qDebug() << "text_socket: Read Datagram Failed!";
            return;
        }
        emit textAppend(QString(byteArray));
    }
}

void TextMsgTransceiver::on_textSend(QString body)
{
    QString datetime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    QString head = name + "[" + address.toString() + "]\t" + datetime + "\n";
    QString msg = head + "\n" + body + "\n\n";

    qint64 res;
    res = text_socket->writeDatagram(msg.toUtf8().data(), msg.size(), groupAddress, text_port);
    if(res < 0)
    {
        qDebug() << "text_socket: Text Msg Send Failed!";
        return;
    }
    emit textAppend(msg);   // 此处需禁止本机回环接收组播信息，否则重复显示
}

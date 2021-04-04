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
    textsend_socket->close();
    textrecv_socket->close();
    delete textsend_socket;
    delete textrecv_socket;
}

void TextMsgTransceiver::run()
{
    groupAddress = GROUP_ADDR;
    text_port = TEXT_PORT;

    textsend_socket = new QUdpSocket;
    textrecv_socket = new QUdpSocket;

    textsend_socket->bind(address, text_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    textsend_socket->joinMulticastGroup(groupAddress, interface);
    textsend_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    textsend_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
#ifdef QT_DEBUG
    textsend_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
#else
    textsend_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
#endif
    textsend_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024 * 4);

    textrecv_socket->bind(QHostAddress::AnyIPv4, text_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    textrecv_socket->joinMulticastGroup(groupAddress, interface);
    textrecv_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    textrecv_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 4);

    connect(textrecv_socket, SIGNAL(readyRead()), this, SLOT(on_textReadyRead()), Qt::DirectConnection);

    QString datetime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    QString body = name + "[" + address.toString() + "] Connected\t" + datetime + "\n";
    QString msg = body + "\n";

    qint64 res;
    res = textsend_socket->writeDatagram(msg.toUtf8().data(), msg.size(), groupAddress, text_port);
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

    while(textrecv_socket->hasPendingDatagrams())
    {
        QByteArray byteArray;
        byteArray.resize(static_cast<qint32>(textrecv_socket->pendingDatagramSize()));
        res = textrecv_socket->readDatagram(byteArray.data(), byteArray.size());
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
    res = textsend_socket->writeDatagram(msg.toUtf8().data(), msg.size(), groupAddress, text_port);
    if(res < 0)
    {
        qDebug() << "text_socket: Text Msg Send Failed!";
        return;
    }
    emit textAppend(msg);
}

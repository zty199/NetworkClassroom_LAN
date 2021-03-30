#include "filesender.h"

#include <QFileInfo>
#include <QElapsedTimer>
#include <QCoreApplication>

FileSender::FileSender(QHostAddress address, QString fileName) :
    address(address),
    fileName(fileName)
{
    setAutoDelete(true);
}

FileSender::~FileSender()
{
    file_socket->close();
    delete file_socket;
}

void FileSender::run()
{
    file_port = 8888;

    file_socket = new QTcpSocket;
    file_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    file_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024 * 64);

    // 连接客户端
    file_socket->connectToHost(address, file_port);
    if(!file_socket->waitForConnected())
    {
        qDebug() << file_socket->errorString();
        return;
    }

    // 分包发送文件
    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    // 获取文件名和文件大小
    fileName = QFileInfo(file).fileName();
    fileSize = QFileInfo(file).size();
    QString tmp = "File\n" + fileName + "\n" + QString::number(fileSize);

    qint64 res;
    QByteArray byteArray;

    qint32 packetNum = fileSize / UDP_MAX_SIZE;
    qint32 lastPacketSize = fileSize % UDP_MAX_SIZE;
    qint32 currentPacketIndex = 0;
    if(lastPacketSize != 0)
    {
        packetNum++;
    }

    res = file_socket->write(tmp.toUtf8().data(), UDP_MAX_SIZE);
    if(res < 0)
    {
        qDebug() << "file_socket: File Info Send Failed!";
    }

    // 延时发送文件，防止 TCP 粘包（非阻塞）
    QElapsedTimer timer;
    timer.start();
    while(timer.elapsed() < 500)
    {
        QCoreApplication::processEvents();
    }

    while(currentPacketIndex < packetNum)
    {
        if(currentPacketIndex < (packetNum - 1))
        {
            byteArray = file.read(UDP_MAX_SIZE);

            res = file_socket->write(byteArray.data(), UDP_MAX_SIZE);

            if(res < 0)
            {
                qDebug() << "file_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
        else
        {
            byteArray = file.read(lastPacketSize);

            res = file_socket->write(byteArray.data(), lastPacketSize);

            if(res < 0)
            {
                qDebug() << "file_socket: Packet Send Failed!";
            }

            currentPacketIndex++;
        }
    }

    file.close();

    file_socket->waitForBytesWritten();
    file_socket->flush();

    file_socket->waitForDisconnected();
}

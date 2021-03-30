#include "filereceiver.h"

#include <QFileDialog>
#include <QMessageBox>
// #include <QDesktopServices>

FileReceiver::FileReceiver(QHostAddress m_address) :
    m_address(m_address)
{

}

FileReceiver::~FileReceiver()
{
    file_server->close();
    delete file_server;
}

void FileReceiver::run()
{
    file_server = new QTcpServer;
    file_port = 8888;

    file_server->listen(m_address, file_port);
    connect(file_server, SIGNAL(newConnection()), this, SLOT(on_newConnection()), Qt::DirectConnection);
    exec();
}

void FileReceiver::on_newConnection()
{
    while(file_server->hasPendingConnections())
    {
        file_socket = file_server->nextPendingConnection();
        // qDebug() << file_socket->peerAddress().toString() << file_socket->peerPort();
        file_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
        file_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 64);

        connect(file_socket, SIGNAL(readyRead()), this, SLOT(on_fileReadyRead()), Qt::DirectConnection);
        connect(file_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    }
}

void FileReceiver::on_fileReadyRead()
{
    qint64 res;

    static qint64 receivedBytes = 0;

    QByteArray byteArray;
    byteArray.resize(file_socket->bytesAvailable());
    byteArray = file_socket->readAll();

    if(!QString(byteArray).indexOf("File\n"))
    {
        fileName = QString(byteArray).split("\n").at(1);
        fileSize = QString(byteArray).split("\n").at(2).toInt(0);
        // qDebug() << fileName << fileSize;

        // 另存为文件
        QString suffix = QFileInfo(fileName).suffix();
        QString tmp = QFileDialog::getSaveFileName(nullptr,
                                                   "Save As",
                                                   QDir::homePath() + "/Desktop/" + fileName,
                                                   "*." + suffix);
        if(tmp.isEmpty())
        {
            tmp = QDir::homePath() + "/Desktop/" + fileName;
        }

        file = new QFile(tmp);
        if(!file->open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(nullptr, "Critical", "File Save Failed!", QMessageBox::Ok);
            return;
        }
        return;
    }

    if((res = file->write(byteArray.data(), byteArray.size())) < 0)
    {
        qDebug() << "file_receive: File Write Failed!";
        return;
    }
    else
    {
        receivedBytes += res;
    }

    if(receivedBytes == fileSize)
    {
        file->close();
        // qDebug() << QFileInfo(*file).size();
        // QDesktopServices::openUrl(QFileInfo(*file).absolutePath());
        delete file;

        file_socket->disconnectFromHost();
        file_socket->close();
        delete file_socket;

        receivedBytes = 0;
    }
}

void FileReceiver::displayError(QAbstractSocket::SocketError error)
{
    switch(error)
    {
    case QAbstractSocket::RemoteHostClosedError:
        break;
    default:
        qDebug() << file_socket->errorString();
    }
}

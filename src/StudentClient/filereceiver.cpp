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
    file_port = FILE_PORT;

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
        file_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 8);     // 缓冲区超过 8K 会引起 程序异常结束 或 readyRead() 只触发一次

        connect(file_socket, SIGNAL(readyRead()), this, SLOT(on_fileReadyRead()), Qt::DirectConnection);
        connect(file_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)), Qt::DirectConnection);
    }
}

void FileReceiver::on_fileReadyRead()
{
    qint64 res;

    static qint64 receivedBytes = 0;

    QByteArray byteArray;
    byteArray.resize(static_cast<qint32>(file_socket->bytesAvailable()));
    file_socket->read(byteArray.data(), byteArray.size());      // 使用 file_socket->realAll() 会引起 程序异常结束

    if(!QString(byteArray).indexOf("File\n"))
    {
        fileName = QString(byteArray).split("\n").at(1);
        fileSize = QString(byteArray).split("\n").at(2).toInt();
        // qDebug() << fileName << fileSize;

        // 另存为文件
        QString suffix = QFileInfo(fileName).suffix();
        QString tmp = QFileDialog::getSaveFileName(nullptr,
                                                   "Save As",
                                                   QDir::homePath() + "/Desktop/" + fileName,
                                                   "*." + suffix);
        // 若文件路径为空 或 所选路径无法写入
        if(tmp.isEmpty() || !QFileInfo(QFileInfo(tmp).absolutePath()).permissions().testFlag(QFile::WriteUser))
        {
            tmp = QDir::homePath() + "/Desktop/" + fileName;
        }

        file = new QFile(tmp);
        if(!file->open(QIODevice::WriteOnly))
        {
            QMessageBox::critical(nullptr, "Critical", "File Save Failed!", QMessageBox::Ok);
            return;
        }

        // 处理粘包部分
        if(byteArray.size() > PACKET_MAX_SIZE)
        {
            res = file->write(byteArray.data() + PACKET_MAX_SIZE, byteArray.size() - PACKET_MAX_SIZE);
            if(res < 0)
            {
                qDebug() << "file_receiver: File Write Failed!";
                file->close();
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
        return;
    }

    if((res = file->write(byteArray.data(), byteArray.size())) < 0)
    {
        qDebug() << "file_receiver: File Write Failed!";
        file->close();
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

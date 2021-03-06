#ifndef FILERECEIVER_H
#define FILERECEIVER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>
#include <QFileInfo>

#include "util.h"
#include "config.h"

class FileReceiver : public QThread
{
    Q_OBJECT

public:
    FileReceiver(QHostAddress m_address);
    ~FileReceiver() override;

protected:
    void run() override;

private:
    QTcpServer *file_server;
    QTcpSocket *file_socket;
    QHostAddress m_address;
    quint16 file_port;

    QString fileName;
    qint64 fileSize;
    QFile *file;

private slots:
    void on_newConnection();
    void on_fileReadyRead();
    void displayError(QAbstractSocket::SocketError);

signals:
    void fileReceived(bool flag);
    void fileReceivedProgress(int value);

};

#endif // FILERECEIVER_H

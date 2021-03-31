#ifndef FILESENDER_H
#define FILESENDER_H

#include <QRunnable>
#include <QHostAddress>
#include <QTcpSocket>

#include "util.h"
#include "config.h"

class FileSender : public QRunnable
{
public:
    FileSender(QHostAddress address, QString fileName);
    ~FileSender();

protected:
    void run() override;

private:
    QHostAddress address;
    QString fileName;
    qint64 fileSize;

    QTcpSocket *file_socket;
    quint16 file_port;

};

#endif // FILESENDER_H

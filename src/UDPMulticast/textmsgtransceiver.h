#ifndef TEXTMSGTRANSCEIVER_H
#define TEXTMSGTRANSCEIVER_H

#include <QThread>
#include <QUdpSocket>
#include <QNetworkInterface>

class TextMsgTransceiver : public QThread
{
    Q_OBJECT

public:
    TextMsgTransceiver(QNetworkInterface interface, QHostAddress address, QString name);
    ~TextMsgTransceiver() override;

protected:
    void run() override;

private:
    QUdpSocket *text_socket;
    QHostAddress groupAddress;
    quint16 text_port;
    QNetworkInterface interface;
    QHostAddress address;
    QString name;

private slots:
    void on_textReadyRead();
    Q_INVOKABLE void on_textSend(QString);

signals:
    void textAppend(QString);

};

#endif // TEXTMSGTRANSCEIVER_H

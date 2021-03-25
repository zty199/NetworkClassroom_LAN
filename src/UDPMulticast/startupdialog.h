#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>
#include <QNetworkInterface>
#include <QHostAddress>

namespace Ui {
class StartUpDialog;
}

class StartUpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartUpDialog(QWidget *parent = nullptr);
    ~StartUpDialog();

protected:
    void mousePressEvent(QMouseEvent *) override;

private:
    Ui::StartUpDialog *ui;

    QList<QNetworkInterface> availableInterfaces;
    QNetworkInterface m_interface;
    QHostAddress m_address;
    bool flag_multicast;

    void initUI();

private slots:
    void on_cb_network_currentIndexChanged(int index);
    void on_btn_multicast_clicked();

    void on_studentConnected(int);

    void on_btn_start_clicked();

signals:
    void multicastReady(QNetworkInterface, QHostAddress);
    void startUp();

};

#endif // STARTUPDIALOG_H

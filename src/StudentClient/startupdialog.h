#ifndef STARTUPDIALOG_H
#define STARTUPDIALOG_H

#include <QDialog>
#include <QNetworkInterface>

namespace Ui {
class StartUpDialog;
}

class StartUpDialog : public QDialog
{
    Q_OBJECT

public:
    StartUpDialog(QWidget *mainWindow, QWidget *parent = nullptr);
    ~StartUpDialog() override;

protected:
    void mousePressEvent(QMouseEvent *) override;

private:
    Ui::StartUpDialog *ui;

    QList<QNetworkInterface> availableInterfaces;
    QNetworkInterface m_interface;
    QHostAddress m_address;
    QString m_name;
    bool flag_connect;

    QWidget *mainWindow;

    void initUI();
    void initConnections();

private slots:
    void on_cb_network_currentIndexChanged(int index);
    void on_btn_connect_clicked();
    void on_btn_start_clicked();
    void on_teacherConnected();

signals:
    void connectReady(QNetworkInterface, QHostAddress, QString);
    void connectNotReady();
    void startUp();

};

#endif // STARTUPDIALOG_H

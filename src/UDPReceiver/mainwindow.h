#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QAudio>
#include <QAudioOutput>
#include <QUdpSocket>

#include "startupdialog.h"

#include "videoframereceiver.h"
#include "audiopackreceiver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainWindow *ui;

    StartUpDialog *m_startup;
    QNetworkInterface m_interface;
    QHostAddress m_address;
    QString m_name;
    QTimer *command_timer;
    QHostAddress teacher_address;
    bool flag_startup;

    QList<QAudioDeviceInfo> availableDevices;
    QAudioOutput *m_audioOutput;
    QIODevice *m_audioDevice;
    QAudioFormat format;
    bool flag_audio;

    QUdpSocket *command_socket;
    QHostAddress groupAddress;
    quint16 command_port;

    VideoFrameReceiver *video_receiver;
    AudioPackReceiver *audio_receiver;

    void initUdpConnections();
    void initOutputDevice();
    void initUI();
    void initConnections();

private slots:
    void on_connectReady(QNetworkInterface, QHostAddress, QString);
    void on_connectNotReady();
    void on_startUp();
    void on_commandTimeOut();
    void on_commandReadyRead();

    void on_videoFrameReceived(QImage);
    void on_btn_audio_clicked();
    void on_cb_device_currentIndexChanged(int index);
    void on_slider_volume_valueChanged(int value);
    void on_volumeChanged(int value);
    void on_audioPackReceived(AudioPack);

signals:
    void teacherConnected();

    void volumeChanged(int value);

};

#endif // MAINWINDOW_H

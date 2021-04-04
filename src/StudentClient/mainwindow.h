#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QAudio>
#include <QAudioOutput>

#include "startupdialog.h"
#include "textchatdialog.h"

#include "videoframereceiver.h"
#include "audiopackreceiver.h"
#include "filereceiver.h"
#include "textmsgtransceiver.h"

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

    QList<QAudioDeviceInfo> availableDevices;
    QAudioOutput *m_audioOutput;
    QIODevice *m_audioDevice;
    QAudioFormat format;
    bool flag_audio;

    QUdpSocket *commandsend_socket;
    QUdpSocket *commandrecv_socket;
    QHostAddress groupAddress;
    quint16 command_port;

    VideoFrameReceiver *video_receiver;
    AudioPackReceiver *audio_receiver;
    FileReceiver *file_receiver;
    TextMsgTransceiver *text_transceiver;

    StartUpDialog *m_startup;
    QNetworkInterface m_interface;
    QHostAddress m_address;
    QString m_name;
    QTimer *command_timer;
    QHostAddress teacher_address;
    bool flag_startup;

    TextChatDialog *m_textchat;
    bool flag_text;

    void initUdpConnections();
    void initOutputDevice();
    void initUI();
    void initConnections();

private slots:
    void on_videoFrameReceived(QImage);
    void on_btn_audio_clicked();
    void on_cb_device_currentIndexChanged(int index);
    void on_slider_volume_valueChanged(int value);
    void on_volumeChanged(int value);
    void on_audioPackReceived(AudioPack);
    void on_btn_textChat_clicked();

    void on_connectReady(QNetworkInterface, QHostAddress, QString);
    void on_connectNotReady();
    void on_startUp();
    void on_commandTimeOut();
    void on_commandReadyRead();

    void on_textAppend(QString);
    void on_textSend(QString);

signals:
    void teacherConnected();
    void volumeChanged(int value);
    void textAppend(QString);

};

#endif // MAINWINDOW_H

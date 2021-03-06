#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QCamera>
#include <QCameraInfo>
#include <QScreen>
#include <QLabel>
#include <QTimer>
#include <QAudio>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QThreadPool>

#include "startupdialog.h"
#include "videosurface.h"
#include "screenpen.h"
#include "filesendprogress.h"
#include "textchatdialog.h"

#include "videoframesender.h"
#include "audiopacksender.h"
#include "filesender.h"
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

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon *m_tray;
    QMenu *t_menu;
    QAction *t_show;
    QAction *t_about;
    QAction *t_exit;

    QList<QCameraInfo> availableCameras;
    QCamera *m_camera;
    VideoSurface *m_viewFinder;
    QList<QCameraViewfinderSettings> viewSets;
    bool flag_camera;

    QScreen *m_screen;
    QLabel *m_cursor;
    QSize screenRes;
    int screenTimer;
    QTimer *screen_timer;
    ScreenPen *m_screenPen;
    bool flag_screen;

    QList<QAudioDeviceInfo> availableDevices;
    QAudioInput *m_audioInput;
    QIODevice *m_audioDevice;
    QAudioFormat format;
    bool flag_audio;

    QUdpSocket *command_socket;
    QHostAddress groupAddress;
    quint16 command_port;

    QThreadPool *video_threadPool;
    QThreadPool *audio_threadPool;
    QThreadPool *file_threadPool;

    TextMsgTransceiver *text_transceiver;

    StartUpDialog *m_startup;
    QNetworkInterface m_interface;
    QHostAddress m_address;
    QString m_name;
    QTimer *command_timer;
    QMap<QString, QString> stuMap;
    int stuNum;
    bool flag_startup;

    FileSendProgress *m_progress;

    TextChatDialog *m_textchat;
    bool flag_text;

    void initUdpConnections();
    void initCamDevice();
    void initInputDevice();
    void initUI();
    void initTray();
    void initConnections();
    void initCamera();

private slots:
    void on_messageReceived(QString);
    void on_exitTriggered(bool checked);
    void on_trayActivated(QSystemTrayIcon::ActivationReason reason);

    void on_btn_camera_clicked();
    void on_cb_camera_currentIndexChanged(int index);
    void on_cb_camRes_currentIndexChanged(int index);
    void on_videoFrameChanged(QVideoFrame);

    void on_btn_screen_clicked();
    void on_cb_screenRes_currentIndexChanged(int index);
    void on_cb_screenHz_currentIndexChanged(int index);
    void on_btn_screenPen_clicked();
    void on_screenTimeOut();
    void on_mouseMove();

    Q_INVOKABLE void on_videoFrameSent(QImage);     // Q_INVOKABLE ?????????????????????????????????????????? QMetaObject ???????????? QRunnable ?????????????????????

    void on_btn_audio_clicked();
    void on_cb_device_currentIndexChanged(int index);
    void on_slider_volume_valueChanged(int value);
    void on_volumeChanged(int value);
    void on_deviceReadyRead();

    void on_btn_fileTrans_clicked();
    void on_btn_signIn_clicked();
    void on_btn_textChat_clicked();

    void on_multicastReady(QNetworkInterface, QHostAddress, QString);
    void on_multicastNotReady();
    void on_startUp();
    void on_commandTimeOut();
    void on_commandReadyRead();

    void on_textAppend(QString);
    void on_textSend(QString);

    void on_btn_about_clicked();

signals:
    void volumeChanged(int value);
    void studentConnected(int);
    void textAppend(QString);

};

#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QScreen>
#include <QLabel>
#include <QTimer>
#include <QAudio>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QUdpSocket>
#include <QThreadPool>

#include "startupdialog.h"

#include "videosurface.h"
#include "screenpen.h"

#include "videoframesender.h"
#include "audiopacksender.h"

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
    void closeEvent(QCloseEvent *) override;

private:
    Ui::MainWindow *ui;

    StartUpDialog *m_startup;
    QNetworkInterface m_interface;
    QHostAddress m_address;
    QTimer *command_timer;
    int stuNum;

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

    void initUdpConnections();
    void initInputDevice();
    void initUI();
    void initConnections();
    void initCamera();

private slots:
    void on_multicastReady(QNetworkInterface, QHostAddress);
    void on_startUp();
    void on_commandTimeOut();
    void on_commandReadyRead();

    void on_btn_screen_clicked();
    void on_cb_screenRes_currentIndexChanged(int index);
    void on_cb_screenHz_currentIndexChanged(int index);
    void on_btn_screenPen_clicked();
    void on_screenTimeOut();
    void on_btn_camera_clicked();
    void on_cb_camera_currentIndexChanged(int index);
    void on_cb_camRes_currentIndexChanged(int index);
    void on_videoFrameChanged(QVideoFrame);
    Q_INVOKABLE void on_videoFrameSent(QImage);     // Q_INVOKABLE 用来修饰成员函数，使其能够被 QMetaObject 调用（从 QRunnable 子线程中调用）
    void on_btn_audio_clicked();
    void on_cb_device_currentIndexChanged(int index);
    void on_slider_volume_valueChanged(int value);
    void on_volumeChanged(int value);
    void on_deviceReadyRead();

    void on_mouseMove();

signals:
    void studentConnected(int);

    void volumeChanged(int value);

};

#endif // MAINWINDOW_H

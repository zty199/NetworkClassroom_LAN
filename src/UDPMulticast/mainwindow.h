#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCamera>
#include <QCameraInfo>
#include <QScreen>
#include <QAudio>
#include <QAudioInput>
#include <QAudioDeviceInfo>
#include <QTimer>
#include <QUdpSocket>
#include <QHostAddress>

#include "videosurface.h"

#define UDP_MAX_SIZE 1472  // UDP 数据包建议长度   * MTU = 1500，1500 - 20（IP头）- 8（UDP头）

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

    QList<QCameraInfo> availableCameras;
    QCamera *m_camera;
    VideoSurface *m_viewFinder;
    QList<QCameraViewfinderSettings> viewSets;
    bool flag_camera;

    QScreen *m_screen;
    QList<QAudioDeviceInfo> availableDevices;
    QAudioInput *m_audioInput;
    QIODevice *m_audioDevice;
    QAudioFormat format;
    QTimer *m_timer;
    bool flag_screen;
    bool flag_audio;

    QUdpSocket *video_socket;
    QUdpSocket *audio_socket;
    QHostAddress groupAddress;
    quint16 video_port;
    quint16 audio_port;

    struct videoPack
    {
        char data[1024];
        int lens;
    };

    void initUdpConnections();
    void initInputDevice();
    void initUI();
    void initConnections();
    void initCamera();

private slots:
    void on_btn_camera_clicked();
    void on_cb_camera_currentIndexChanged(int index);
    void on_cb_resolution_currentIndexChanged(int index);
    void on_videoFrameChanged(QVideoFrame);
    void on_btn_screen_clicked();
    void on_timeOut();
    void on_btn_audio_clicked();
    void on_cb_device_currentIndexChanged(int index);
    void on_audioReadyRead();

    void on_mouseMove();

};

#endif // MAINWINDOW_H

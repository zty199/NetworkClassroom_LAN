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

#include "screenpen.h"

#define UDP_MAX_SIZE    1472    // UDP 数据包最大长度   * MTU = 1500，故数据包大小 1500 - 20（IP头）- 8（UDP头）

#define SAMPLE_RATE     44100   // 采样频率
#define SAMPLE_SIZE     16      // 采样位数
#define CHANNEL_COUNT   2       // 声道数

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
    QTimer *m_timer;
    ScreenPen *m_screenPen;
    bool flag_screen;

    QList<QAudioDeviceInfo> availableDevices;
    QAudioInput *m_audioInput;
    QIODevice *m_audioDevice;
    QAudioFormat format;
    bool flag_audio;

    QUdpSocket *video_socket;
    QUdpSocket *audio_socket;
    QHostAddress groupAddress;
    quint16 video_port;
    quint16 audio_port;

    struct videoPack
    {
        char data[1024 * 16];   // 单个音频数据包大小设为 16K，音质 44K/128Kbps（？）
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
    void on_btn_screenPen_clicked();
    void on_btn_audio_clicked();
    void on_cb_device_currentIndexChanged(int index);
    void on_slider_volume_valueChanged(int value);
    void on_volumeChanged(int value);
    void on_deviceReadyRead();

    void on_mouseMove();

signals:
    void volumeChanged(int value);

};

#endif // MAINWINDOW_H

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

#include "videosurface.h"
#include "screenpen.h"

#include "videoframesender.h"
#include "audiopacksender.h"

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
    QLabel *m_cursor;
    QTimer *m_timer;
    ScreenPen *m_screenPen;
    bool flag_screen;

    QList<QAudioDeviceInfo> availableDevices;
    QAudioInput *m_audioInput;
    QIODevice *m_audioDevice;
    QAudioFormat format;
    bool flag_audio;

    QUdpSocket *video_socket;
    QHostAddress groupAddress;
    quint16 video_port;

    QThreadPool *video_threadPool;
    QThreadPool *audio_threadPool;

    struct AudioPack
    {
        char data[1024 * 16];   // 单个音频数据包大小设为 16K，音质 44K/128Kbps（？）
        int len;
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
    Q_INVOKABLE void on_videoFrameSent(QImage);     // Q_INVOKABLE 用来修饰成员函数，使其能够被 QMetaObject 调用（从 QRunnable 子线程中调用）
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QHostAddress>
#include <QAudio>
#include <QAudioOutput>

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

    QList<QAudioDeviceInfo> availableDevices;
    QAudioOutput *m_audioOutput;
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
        char data[1024 * 16];
        int lens;
    } vp;

    void initUdpConnections();
    void initOutputDevice();
    void initUI();
    void initConnections();

private slots:
    void on_videoReadyRead();
    void on_btn_audio_clicked();
    void on_cb_device_currentIndexChanged(int index);
    void on_slider_volume_valueChanged(int value);
    void on_volumeChanged(int value);
    void on_audioReadyRead();
    void on_deviceReadyWrite();

signals:
    void volumeChanged(int value);
    void readyWrite();

};

#endif // MAINWINDOW_H

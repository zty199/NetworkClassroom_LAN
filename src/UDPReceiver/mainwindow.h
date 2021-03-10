#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QHostAddress>
#include <QAudio>
#include <QAudioOutput>

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

    QList<QAudioDeviceInfo> availableDevices;
    QAudioOutput *m_audioOutput;
    QIODevice *m_audioDevice;
    QAudioFormat format;

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
    void initOutputDevice();
    void initUI();
    void initConnections();

private slots:
    void on_videoReadyRead();
    void on_cb_device_currentIndexChanged(int index);
    void on_audioReadyRead();

};

#endif // MAINWINDOW_H
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAudio>
#include <QAudioOutput>
#include <QUdpSocket>

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

private:
    Ui::MainWindow *ui;

    QList<QAudioDeviceInfo> availableDevices;
    QAudioOutput *m_audioOutput;
    QIODevice *m_audioDevice;
    QAudioFormat format;
    bool flag_audio;

    QUdpSocket *audio_socket;
    QHostAddress groupAddress;
    quint16 audio_port;

    VideoFrameReceiver *video_receiver;
    AudioPackReceiver *audio_receiver;

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

signals:
    void volumeChanged(int value);

};

#endif // MAINWINDOW_H

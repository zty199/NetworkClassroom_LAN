#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMetaType>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)),
    flag_audio(true),
    audio_socket(new QUdpSocket(this)),
    groupAddress("239.0.0.1"),
    audio_port(8889)
{
    ui->setupUi(this);

    initUdpConnections();
    initOutputDevice();
    initUI();
    initConnections();

    video_receiver = new VideoFrameReceiver;
    connect(video_receiver, SIGNAL(videoFrameReceived(QImage)), this, SLOT(on_videoFrameReceived(QImage)));
    video_receiver->start();

    audio_receiver = new AudioPackReceiver;
    /*
     * 在线程中通过信号槽传递信息时，参数默认放到队列中
     * AudioPack 是自定义的结构体，不是 Qt 自带的参数结构，无法放入队列
     * 将不识别的参数结构进行注册，让 Qt 能够识别
     */
    qRegisterMetaType<AudioPack>("AudioPack");  // 注册 AudioPack 类型
    connect(audio_receiver, SIGNAL(audioPackReceived(AudioPack)), this, SLOT(on_audioPackReceived(AudioPack)));
    audio_receiver->start();

    qDebug() << "Initialization Finished!";
}

MainWindow::~MainWindow()
{
    video_receiver->quit();
    video_receiver->wait();
    delete video_receiver;

    audio_receiver->quit();
    audio_receiver->wait();
    delete audio_receiver;

    delete ui;
}

void MainWindow::initUdpConnections()
{
    // audio_socket->bind(QHostAddress::AnyIPv4, audio_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);  // 绑定组播地址端口
    // audio_socket->joinMulticastGroup(groupAddress);         // 添加到组播，绑定到读套接字上
    // audio_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 64);       // 缓冲区最大存储 4个 数据包（单个 16K）
}

void MainWindow::initOutputDevice()
{
    format.setSampleRate(SAMPLE_RATE);
    format.setChannelCount(CHANNEL_COUNT);
    format.setSampleSize(SAMPLE_SIZE);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);

    QAudioFormat format = this->format;
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if(!info.isFormatSupported(this->format))
    {
        format = info.nearestFormat(this->format);
    }

    m_audioOutput = new QAudioOutput(info, format, this);
}

void MainWindow::initUI()
{
    foreach(const QAudioDeviceInfo &device, availableDevices)
    {
        ui->cb_device->addItem(device.deviceName(), availableDevices.indexOf(device));
    }
}

void MainWindow::initConnections()
{
    connect(this, SIGNAL(volumeChanged(int)), this, SLOT(on_volumeChanged(int)));
}

void MainWindow::on_videoFrameReceived(QImage image)
{
    if(image.isNull())
    {
        ui->videoViewer->clear();
    }
    else
    {
        ui->videoViewer->setPixmap(QPixmap::fromImage(image).scaled(ui->videoViewer->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
    }
}

void MainWindow::on_btn_audio_clicked()
{
    static int curVolume;

    if(!flag_audio)
    {
        // 仅静音当前音频流
        ui->slider_volume->setValue(curVolume);
        qDebug() << "Audio Share Unmuted!";
        flag_audio = true;
    }
    else
    {
        curVolume = ui->slider_volume->value();
        ui->slider_volume->setValue(0);
        qDebug() << "Audio Share Muted!";
        flag_audio = false;

        // 刷新音频输出可用设备列表
        if(availableDevices == QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        {
            return;
        }

        int index = -1;
        QAudioDeviceInfo curOutput;
        if(ui->cb_device->currentIndex() > index)
        {
            curOutput = availableDevices.at(ui->cb_device->currentIndex());
        }
        ui->cb_device->disconnect();
        ui->cb_device->clear();

        availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
        if(!availableDevices.isEmpty())
        {
            for(int i = 0; i < availableDevices.size(); i++)
            {
                ui->cb_device->addItem(availableDevices.at(i).deviceName(), i);
                if(availableDevices.at(i).deviceName() == curOutput.deviceName())
                {
                    index = i;
                }
            }

            if(index < 0)
            {
                index = 0;
            }

            ui->cb_device->setCurrentIndex(index);
        }

        connect(ui->cb_device, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cb_device_currentIndexChanged(int)));
    }
}

void MainWindow::on_cb_device_currentIndexChanged(int index)
{
    /*
     * 测试中发现，Linux 系统中 alsa_output 等音频输出设备不支持调节音量
     * 调节音量提示 QAudioOutput(pulseaudio): pa_stream_begin_write, error = Invalid argument
     * 再次选择该设备后会导致程序卡死
     */
#ifdef Q_OS_LINUX
    if(!ui->cb_device->currentText().indexOf("alsa_output"))
    {
        ui->btn_audio->setDisabled(true);
        ui->slider_volume->setDisabled(true);
        ui->volume->setDisabled(true);
    }
    else
    {
        ui->btn_audio->setEnabled(true);
        ui->slider_volume->setEnabled(true);
        ui->volume->setEnabled(true);
    }
#endif
    if(flag_audio)
    {
        m_audioOutput->stop();
    }

    QAudioFormat format = this->format;
    QAudioDeviceInfo info(availableDevices.at(index));
    if(!info.isFormatSupported(this->format))
    {
        format = info.nearestFormat(this->format);
    }

    /*
     * 删除旧音频输出设备并新建设备，
     * 在 Linux 上会引起程序异常结束，原因未知
     */
    // delete m_audioOutput;
    m_audioOutput = new QAudioOutput(info, format, this);

    if(flag_audio)
    {
        m_audioDevice = m_audioOutput->start();
#ifdef Q_OS_LINUX
        if(ui->cb_device->currentText().indexOf("alsa_output"))
        {
#endif
            m_audioOutput->setVolume(qreal(ui->slider_volume->value()) / 100);
#ifdef Q_OS_LINUX
        }
#endif
    }

    qDebug() << "Output Device: " << info.deviceName();
    qDebug() << format.sampleRate() << " " << format.channelCount() << " " << format.sampleSize() << " " << format.codec();
}

void MainWindow::on_slider_volume_valueChanged(int value)
{
    // 滑动条调节音频输出设备音量（仅调节音频流音量而非设备全局音量），范围 0.0 ~ 1.0
    m_audioOutput->setVolume(qreal(value) / 100);
    emit this->volumeChanged(value);

    // 音量调节则解除静音
    if(!flag_audio)
    {
        flag_audio = true;
    }
}

void MainWindow::on_volumeChanged(int value)
{
    // 同步显示当前音量
    ui->volume->setText("Volume: " + QString::number(value));
}

void MainWindow::on_audioPackReceived(AudioPack ap)
{
    m_audioDevice->write(ap.data, ap.len);
    m_audioDevice->waitForBytesWritten(-1);

    m_audioDevice->reset();
}

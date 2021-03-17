#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QBuffer>
#include <QImageReader>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)),
    flag_audio(true),
    video_socket(new QUdpSocket(this)),
    audio_socket(new QUdpSocket(this)),
    groupAddress("239.0.0.1"),
    video_port(8888),
    audio_port(8889)
{
    ui->setupUi(this);

    initUdpConnections();
    initOutputDevice();
    initUI();
    initConnections();
    qDebug() << "Initialization Finished!";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUdpConnections()
{
    video_socket->bind(QHostAddress::AnyIPv4, video_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);  // 绑定广播地址端口
    video_socket->joinMulticastGroup(groupAddress);         // 添加到组播，绑定到读套接字上
    video_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1920 * 1080 * 4); // 缓冲区最大存储 1080p 位图

    // video_socket->bind(QHostAddress::LocalHost, video_port);

    audio_socket->bind(QHostAddress::AnyIPv4, audio_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);  // 绑定广播地址端口
    audio_socket->joinMulticastGroup(groupAddress);         // 添加到组播，绑定到读套接字上
    audio_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 64);   // 缓冲区最大存储 4个 数据包（单个 16K）

    // audio_socket->bind(QHostAddress::LocalHost, audio_port);
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
    if(availableDevices.isEmpty())
    {
        ui->cb_device->setDisabled(true);
    }
    else
    {
        foreach(const QAudioDeviceInfo &device, availableDevices)
        {
            ui->cb_device->addItem(device.deviceName(), availableDevices.indexOf(device));
        }
    }
}

void MainWindow::initConnections()
{
    connect(video_socket, SIGNAL(readyRead()), this, SLOT(on_videoReadyRead()));
    connect(audio_socket, SIGNAL(readyRead()), this, SLOT(on_audioReadyRead()));
    connect(this, SIGNAL(volumeChanged(int)), this, SLOT(on_volumeChanged(int)));
}

void MainWindow::on_videoReadyRead()
{
    qint64 res;                     // UDP 结果
    static qint64 totalBytes = 0,   // 总字节数     * static 保证变量只初始化一次
            receivedBytes = 0;      // 已接收字节数

    QByteArray byteArray;
    byteArray.resize(static_cast<int>(video_socket->pendingDatagramSize()));
    res = video_socket->readDatagram(byteArray.data(), video_socket->pendingDatagramSize());
    if(res < 0)
    {
        qDebug() << "video_socket: Read Datagram Failed!";
        return;
    }

    // 接收到停止信号则清空画面
    if(QString(byteArray) == "Stop")
    {
        ui->videoViewer->clear();
        return;
    }

    // 接收到帧大小信息则暂存
    if(byteArray.contains(QString("size=").toUtf8()))
    {
         QStringList list = QString(byteArray).split("=");
         totalBytes = list.at(1).toInt();
         // qDebug() << "totalBytes = " << totalBytes;
         receivedBytes = 0;
         return;
    }

    static uchar buf[1920 * 1080 * 4];  // 暂存图片数据（最大 1080p 位图）
    memcpy(buf + receivedBytes, byteArray.data(), static_cast<size_t>(res));
    receivedBytes += res;

    // 帧图像接收完成则显示
    if(receivedBytes >= totalBytes)
    {
        QPixmap pixmap;
        pixmap.loadFromData(buf, static_cast<uint>(receivedBytes), "JPEG");
        ui->videoViewer->setPixmap(pixmap.scaled(ui->videoViewer->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

        // 清空数据
        totalBytes = 0;
        receivedBytes = 0;
        memset(buf, 0, sizeof(buf));
    }
}

void MainWindow::on_btn_audio_clicked()
{
    if(!flag_audio)
    {
        m_audioDevice = m_audioOutput->start();
        /*
         * 当接收到的音频数据包完成拼接后，触发单独的信号槽写入设备，
         * 便于切换音频输出设备时，只断开音频设备输入输出流，
         * 而不断开 audio_socket 的信号槽，保证音频数据继续接收
         */
        connect(this, SIGNAL(readyWrite()), this, SLOT(on_deviceReadyWrite()));
        qDebug() << "Audio Share Started!";
        flag_audio = true;

        emit audio_socket->readyRead(); // 音频共享终止接收后恢复播放，需要手动触发 audio_socket->readyRead() 信号
    }
    else
    {
        // 终止音频传输时，先断开设备输入输出流，并且断开 audio_cocket 信号槽停止接收音频数据
        disconnect(this, SIGNAL(readyWrite()), this, SLOT(on_deviceReadyWrite()));
        m_audioOutput->stop();

        qDebug() << "Audio Share Stopped!";
        flag_audio = false;

        // 刷新音频输出可用设备列表
        if(availableDevices == QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        {
            return;
        }

        int index = -1;
        QAudioDeviceInfo curOutput = availableDevices.at(ui->cb_device->currentIndex());
        ui->cb_device->disconnect();
        ui->cb_device->clear();

        availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);
        if(availableDevices.isEmpty())
        {
            ui->btn_audio->setDisabled(true);
            ui->cb_device->setDisabled(true);
        }
        else
        {
            for(int i = 0; i < availableDevices.size(); i++)
            {
                ui->cb_device->addItem(availableDevices.at(i).deviceName(), i);
                if(availableDevices.at(i).deviceName() == curOutput.deviceName())
                {
                    index = i;
                }
            }
        }

        if(index < 0)
        {
            index = 0;
        }
        connect(ui->cb_device, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cb_device_currentIndexChanged(int)));
        ui->cb_device->setCurrentIndex(index);
        emit ui->cb_device->currentIndexChanged(index);
    }
}

void MainWindow::on_cb_device_currentIndexChanged(int index)
{
    if(flag_audio)
    {
        disconnect(this, SIGNAL(readyWrite()), this, SLOT(on_deviceReadyWrite()));
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
        connect(this, SIGNAL(readyWrite()), this, SLOT(on_deviceReadyWrite()));
    }

    qDebug() << "Output Device: " << info.deviceName();
    qDebug() << format.sampleRate() << " " << format.channelCount() << " " << format.sampleSize() << " " << format.codec();
}

void MainWindow::on_slider_volume_valueChanged(int value)
{
    // 滑动条调节音频输出设备音量（仅调节音频流音量而非设备全局音量），范围 0.0 ~ 1.0
    m_audioOutput->setVolume(qreal(value) / 100);
    emit this->volumeChanged(value);
}

void MainWindow::on_volumeChanged(int value)
{
    // 同步显示当前音量
    ui->volume->setText("Volume: " + QString::number(value));
}

void MainWindow::on_audioReadyRead()
{
    // 音频共享关闭接收时不执行
    if(!flag_audio)
    {
        return;
    }

    qint64 res;                     // UDP 结果
    static qint64 totalBytes = sizeof(videoPack::data) + sizeof(int),   // 总字节数     * static 保证变量只初始化一次
            receivedBytes = 0;      // 已接收字节数

    QByteArray byteArray;
    byteArray.resize(static_cast<int>(audio_socket->pendingDatagramSize()));
    res = audio_socket->readDatagram(byteArray.data(), audio_socket->pendingDatagramSize());
    if(res < 0)
    {
        qDebug() << "audio_socket: Read Datagram Failed!";
        return;
    }

    // 接收到开始信号则清空上个数据包并准备写入新的数据包
    if(QString(byteArray) == "Begin")
    {
        receivedBytes = 0;
        memset(&vp, 0, sizeof(vp));
        m_audioDevice->reset();     // 返回输入缓冲区开头，避免溢出
        return;
    }

    memcpy(reinterpret_cast<char *>(&vp) + receivedBytes, byteArray.data(), static_cast<size_t>(res));
    receivedBytes += res;

    // 音频数据包接收完成则触发写入音频输出设备信号槽
    if(receivedBytes >= totalBytes)
    {
        emit readyWrite();
    }
}

void MainWindow::on_deviceReadyWrite()
{
    m_audioDevice->write(vp.data, vp.lens);
    m_audioDevice->waitForBytesWritten(-1);

    memset(&vp, 0, sizeof(vp));
    m_audioDevice->reset();
}

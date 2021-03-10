#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QBuffer>
#include <QImageReader>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)),
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
    qDebug() << Qt::endl << "Initialization Finished!" << Qt::endl;
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
    audio_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 1024);

    // audio_socket->bind(QHostAddress::LocalHost, audio_port);
}

void MainWindow::initOutputDevice()
{
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
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
    m_audioDevice = m_audioOutput->start();
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
}

void MainWindow::on_videoReadyRead()
{
    qint64 res;                     // UDP 结果
    static qint64 totalBytes = 0,   // 总字节数     * static 保证变量只初始化一次
            receivedBytes = 0;      // 已接收字节数

    QByteArray byteArray;
    byteArray.resize(static_cast<int>(video_socket->pendingDatagramSize()));
    res = video_socket->readDatagram(byteArray.data(), video_socket->pendingDatagramSize());

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
        ui->videoViewer->setPixmap(pixmap.scaled(ui->videoViewer->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

        // 清空数据
        totalBytes = 0;
        receivedBytes = 0;
        memset(buf, 0, sizeof(buf));
    }
}

void MainWindow::on_cb_device_currentIndexChanged(int index)
{
    m_audioOutput->stop();
    audio_socket->disconnect();

    QAudioFormat format = this->format;
    QAudioDeviceInfo info(availableDevices.at(index));
    if(!info.isFormatSupported(this->format))
    {
        format = info.nearestFormat(this->format);
    }

    m_audioOutput = new QAudioOutput(info, format, this);
    m_audioDevice = m_audioOutput->start();
    connect(audio_socket, SIGNAL(readyRead()), this, SLOT(on_audioReadyRead()));
    qDebug() << "Output Device: " << info.deviceName();
    qDebug() << format.sampleRate() << " " << format.channelCount() << " " << format.sampleSize() << " " << format.codec();
}

void MainWindow::on_audioReadyRead()
{
    // qint64 res; // UDP 结果

    videoPack vp;
    memset(&vp, 0, sizeof(vp));
    /* res = */ audio_socket->readDatagram(reinterpret_cast<char*>(&vp), sizeof(vp));
    // qDebug() << res;

    // 写入音频输出数据
    m_audioDevice->write(vp.data, vp.lens);
}

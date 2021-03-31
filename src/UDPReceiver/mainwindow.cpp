#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMetaType>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)),
    flag_audio(true),
    command_socket(new QUdpSocket(this)),
    groupAddress(GROUP_ADDR),
    command_port(COMMAND_PORT),
    m_startup(new StartUpDialog(this)),
    command_timer(new QTimer(this)),
    flag_startup(false),
    m_textchat(new TextChatDialog(this)),
    flag_text(false)
{
    ui->setupUi(this);

    initOutputDevice();
    initUI();
    initConnections();

    qDebug() << "Initialization Finished!";
}

MainWindow::~MainWindow()
{
    if(flag_startup)
    {
        delete video_receiver;
        delete audio_receiver;
        delete file_receiver;
        delete text_transceiver;
    }

    command_socket->close();
    delete command_socket;

    command_timer->stop();
    delete command_timer;

    delete m_startup;
    delete m_textchat;
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *)
{
    QString tmp = QString("Student\n") + m_address.toString() + QString("\n") + m_name + QString("\nDisconnect");
    command_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);

    if(flag_startup)
    {
        video_receiver->quit();
        video_receiver->wait();
        audio_receiver->quit();
        audio_receiver->wait();
        file_receiver->quit();
        file_receiver->wait();
        text_transceiver->quit();
        text_transceiver->wait();
    }
}

void MainWindow::initUdpConnections()
{
    command_socket->close();
    command_socket->bind(m_address, command_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress); // 绑定地址端口
    command_socket->joinMulticastGroup(groupAddress, m_interface);                                          // 添加到组播，绑定组播网卡
    command_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);                                    // 尝试优化套接字以降低延迟
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

    m_startup->show();
}

void MainWindow::initConnections()
{
    connect(this, SIGNAL(volumeChanged(int)), this, SLOT(on_volumeChanged(int)));
    connect(m_startup, SIGNAL(connectReady(QNetworkInterface,QHostAddress,QString)), this, SLOT(on_connectReady(QNetworkInterface,QHostAddress,QString)));
    connect(m_startup, SIGNAL(connectNotReady()), this, SLOT(on_connectNotReady()));
    connect(m_startup, SIGNAL(startUp()), this, SLOT(on_startUp()));
    connect(command_timer, SIGNAL(timeout()), this, SLOT(on_commandTimeOut()));
    connect(m_textchat, SIGNAL(textSend(QString)), this, SLOT(on_textSend(QString)));
}

void MainWindow::on_videoFrameReceived(QImage image)
{
    if(image.isNull())
    {
        ui->videoViewer->clear();
    }
    else
    {
        ui->videoViewer->setPixmap(QPixmap::fromImage(image).scaled(ui->videoViewer->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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

void MainWindow::on_btn_textChat_clicked()
{
    if(!flag_text)
    {
        m_textchat->show();
    }
    else
    {
        m_textchat->hide();
    }
}

void MainWindow::on_connectReady(QNetworkInterface interface, QHostAddress address, QString name)
{
    m_interface = interface;
    m_address = address;
    m_name = name;

    initUdpConnections();
    connect(command_socket, SIGNAL(readyRead()), this, SLOT(on_commandReadyRead()));
    emit command_socket->readyRead();
}

void MainWindow::on_connectNotReady()
{
    command_timer->stop();
}

void MainWindow::on_startUp()
{
    // 发送客户端信息
    qint64 res;
    QString tmp = QString("Student\n") + m_address.toString() + QString("\n") + m_name + QString("\nConnect");
    res = command_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);
    if(res < 0)
    {
        qDebug() << "command_socket: Student IP Send Failed!";
    }

    // command_timer->start(5000);  // 定时发送客户端信息

    // 启动音视频和文件接收，文字聊天线程
    video_receiver = new VideoFrameReceiver(m_interface, m_address);
    connect(video_receiver, SIGNAL(videoFrameReceived(QImage)), this, SLOT(on_videoFrameReceived(QImage)));
    video_receiver->start();

    audio_receiver = new AudioPackReceiver(m_interface, m_address);
    /*
     * 在线程中通过信号槽传递信息时，参数默认放到队列中
     * AudioPack 是自定义的结构体，不是 Qt 自带的参数结构，无法放入队列
     * 将不识别的参数结构进行注册，让 Qt 能够识别
     */
    qRegisterMetaType<AudioPack>("AudioPack");  // 注册 AudioPack 类型
    connect(audio_receiver, SIGNAL(audioPackReceived(AudioPack)), this, SLOT(on_audioPackReceived(AudioPack)));
    audio_receiver->start();

    file_receiver = new FileReceiver(m_address);
    file_receiver->start();

    text_transceiver = new TextMsgTransceiver(m_interface, m_address, m_name);
    connect(text_transceiver, SIGNAL(textAppend(QString)), this, SLOT(on_textAppend(QString)));
    text_transceiver->start();

    flag_startup = true;

    this->show();
}

void MainWindow::on_commandTimeOut()
{
    // 定时发送客户端信息
    qint64 res;
    QString tmp = QString("Student\n") + m_address.toString() + QString("\n") + m_name + QString("\nConnect");
    res = command_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);
    if(res < 0)
    {
        qDebug() << "command_socket: Student IP Send Failed!";
    }
}

void MainWindow::on_commandReadyRead()
{
    // 接收服务端发送信息
    qint64 res;
    QByteArray byteArray;

    while(command_socket->hasPendingDatagrams())
    {
        byteArray.resize(command_socket->pendingDatagramSize());
        res = command_socket->readDatagram(byteArray.data(), byteArray.size());
        if(res < 0)
        {
            qDebug() << "command_socket: Read Datagram Failed!";
        }

        // 接收到服务端上线信息
        if(!QString(byteArray).indexOf("Teacher\n"))
        {
            teacher_address = QHostAddress(QString(byteArray).split("\n").at(1));
            emit teacherConnected();
            return;
        }

        // 接收到服务端视频共享结束信息
        if(!QString(byteArray).indexOf("Stop"))
        {
            video_receiver->quit();
            video_receiver->wait();
            video_receiver->disconnect();

            ui->videoViewer->clear();

            connect(video_receiver, SIGNAL(videoFrameReceived(QImage)), this, SLOT(on_videoFrameReceived(QImage)));

            video_receiver->start();
            return;
        }
    }
}

void MainWindow::on_textAppend(QString msg)
{
    emit textAppend(msg);
}

void MainWindow::on_textSend(QString body)
{
    QMetaObject::invokeMethod(text_transceiver, "on_textSend", Q_ARG(QString, body));
}

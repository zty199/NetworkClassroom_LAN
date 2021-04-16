#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QShortcut>
#include <QMetaType>
#include <QDesktopServices>
#include <QUrl>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    flag_fullscreen(false),
    m_tray(new QSystemTrayIcon),
    t_menu(new QMenu),
    t_show(new QAction(tr("Show MainWindow"))),
    t_about(new QAction(tr("About"))),
    t_exit(new QAction(tr("Exit"))),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)),
    flag_audio(true),
    commandsend_socket(new QUdpSocket(this)),
    commandrecv_socket(new QUdpSocket(this)),
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
    initTray();
    initConnections();

    qDebug() << "Initialization Finished!";
}

MainWindow::~MainWindow()
{
    commandsend_socket->close();
    commandrecv_socket->close();
    delete commandsend_socket;
    delete commandrecv_socket;

    command_timer->stop();
    delete command_timer;

    if(flag_startup)
    {
        delete video_receiver;
        delete audio_receiver;
        delete file_receiver;
        delete text_transceiver;
    }

    delete m_tray;
    delete ui;
}

void MainWindow::initUdpConnections()
{
    commandsend_socket->close();
    commandsend_socket->bind(m_address, command_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    commandsend_socket->joinMulticastGroup(groupAddress, m_interface);
    commandsend_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    commandsend_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
    // 这里是直连服务器 IP 的单播 socket，故不用设置 MulticastLoopback
    commandsend_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024 * 4);

    commandrecv_socket->close();
    commandrecv_socket->bind(QHostAddress::AnyIPv4, command_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);
    commandrecv_socket->joinMulticastGroup(groupAddress, m_interface);
    commandrecv_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
#ifdef Q_OS_WINDOWS
#ifdef QT_DEBUG
    commandrecv_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);
#else
    commandrecv_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);
#endif
#endif
    commandrecv_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 4);
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

void MainWindow::initTray()
{
    // 初始化托盘图标
    t_menu->addAction(t_show);
    t_menu->addAction(t_about);
    t_menu->addAction(t_exit);
    m_tray->setContextMenu(t_menu);
    m_tray->setIcon(QIcon::fromTheme(":/icons/icons/client.png"));
    m_tray->setToolTip(this->windowTitle());

    m_tray->setVisible(true);
}

void MainWindow::initConnections()
{
    connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Enter), this), &QShortcut::activated, this, [=]()
    {
        if(!flag_fullscreen)
        {
            this->showFullScreen();
            flag_fullscreen = true;
        }
        else
        {
            this->showNormal();
            flag_fullscreen = false;
        }
    });

    connect(t_show, &QAction::triggered, this, [=]()
    {
        if(flag_startup)
        {
            this->setWindowState(Qt::WindowActive);
            this->activateWindow();
            this->show();
        }
        else
        {
            m_startup->setWindowState(Qt::WindowActive);
            m_startup->activateWindow();
            m_startup->show();
        }
    });
    connect(t_about, &QAction::triggered, this, [=]()
    {
        QString m_locale = QLocale::system().name().split("_").at(0);

        if(m_locale == "zh")
        {
            QDesktopServices::openUrl(QUrl("https://gitee.com/zty199/NetworkClassroom_LAN"));
        }
        else
        {
            QDesktopServices::openUrl(QUrl("https://github.com/zty199/NetworkClassroom_LAN"));
        }
    });
    connect(t_exit, &QAction::triggered, this, &MainWindow::on_exitTriggered);
    connect(m_tray, &QSystemTrayIcon::activated, this, &MainWindow::on_trayActivated);

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

void MainWindow::on_exitTriggered(bool checked)
{
    Q_UNUSED(checked)

    QString tmp = QString("Student\n") + m_address.toString() + QString("\n") + m_name + QString("\nDisconnect");
    commandsend_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);

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

    // 退出程序
    QApplication::quit();
}

void MainWindow::on_trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    // 响应托盘图标点击事件
    switch(reason)
    {
    case QSystemTrayIcon::Trigger:
    {
        if(flag_startup)
        {
            this->setWindowState(Qt::WindowActive);
            this->activateWindow();
            this->show();
        }
        else
        {
            m_startup->setWindowState(Qt::WindowActive);
            m_startup->activateWindow();
            m_startup->show();
        }
    }
        break;
    default:
        break;
    }
}

void MainWindow::on_btn_audio_clicked()
{
    // 按钮仅静音当前音频流，不关闭设备
    static int curVolume;

    if(!flag_audio)
    {
        ui->slider_volume->setValue(curVolume);
        ui->btn_audio->setIcon(QIcon::fromTheme(":/icons/icons/audio-output-start.png"));
        flag_audio = true;
    }
    else
    {
        if(!ui->slider_volume->value())
        {
            curVolume = 100;
        }
        else
        {
            curVolume = ui->slider_volume->value();
        }

        ui->slider_volume->setValue(0);
        ui->btn_audio->setIcon(QIcon::fromTheme(":/icons/icons/audio-output-stop.png"));
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
    if(!value)
    {
        qDebug() << "Audio Share Muted!";
        flag_audio = false;
        return;
    }

    // 音量调节则解除静音
    if(!flag_audio)
    {
        qDebug() << "Audio Share Unmuted!";
        flag_audio = true;
    }
}

void MainWindow::on_volumeChanged(int value)
{
    // 同步显示当前音量
    ui->volume->setText(tr("Volume: ") + QString::number(value));
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
    connect(commandrecv_socket, SIGNAL(readyRead()), this, SLOT(on_commandReadyRead()));
    emit commandrecv_socket->readyRead();
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
    res = commandsend_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);
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

    ui->btn_audio->setIcon(QIcon::fromTheme(":/icons/icons/audio-output-start.png"));

    this->show();
}

void MainWindow::on_commandTimeOut()
{
    // 定时发送客户端信息
    qint64 res;
    QString tmp = QString("Student\n") + m_address.toString() + QString("\n") + m_name + QString("\nConnect");
    res = commandsend_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);
    if(res < 0)
    {
        qDebug() << "command_socket: Student IP Send Failed!";
    }
}

void MainWindow::on_commandReadyRead()
{
    // 接收服务器发送信息
    qint64 res;
    QByteArray byteArray;

    while(commandrecv_socket->hasPendingDatagrams())
    {
        byteArray.resize(static_cast<qint32>(commandrecv_socket->pendingDatagramSize()));
        res = commandrecv_socket->readDatagram(byteArray.data(), byteArray.size());
        if(res < 0)
        {
            qDebug() << "command_socket: Read Datagram Failed!";
        }

        // 接收到服务器上线信息
        if(!QString(byteArray).indexOf("Teacher\n"))
        {
            teacher_address = QHostAddress(QString(byteArray).split("\n").at(1));
            emit teacherConnected();
            return;
        }

        // 接收到服务器视频共享结束信息
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

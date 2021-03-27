#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMetaType>
#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_startup(new StartUpDialog(this)),
    command_timer(new QTimer(this)),
    flag_startup(false),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)),
    flag_audio(true),
    command_socket(new QUdpSocket(this)),
    groupAddress("239.0.0.1"),
    command_port(8887)
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
    }

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
    }
}

void MainWindow::initUdpConnections()
{
    command_socket->close();
    command_socket->bind(m_address, command_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress); // 绑定地址端口
    command_socket->setMulticastInterface(m_interface);                                                     // 设置组播网卡
    command_socket->joinMulticastGroup(groupAddress);                                                       // 添加到组播，绑定到读套接字上
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

    this->hide();
    m_startup->show();
}

void MainWindow::initConnections()
{
    connect(m_startup, SIGNAL(connectReady(QNetworkInterface,QHostAddress,QString)), this, SLOT(on_connectReady(QNetworkInterface,QHostAddress,QString)));
    connect(m_startup, SIGNAL(connectNotReady()), this, SLOT(on_connectNotReady()));
    connect(m_startup, SIGNAL(startUp()), this, SLOT(on_startUp()));
    connect(command_timer, SIGNAL(timeout()), this, SLOT(on_commandTimeOut()));
    connect(command_socket, SIGNAL(readyRead()), this, SLOT(on_commandReadyRead()));
    connect(this, SIGNAL(volumeChanged(int)), this, SLOT(on_volumeChanged(int)));
}

void MainWindow::on_connectReady(QNetworkInterface interface, QHostAddress address, QString name)
{
    m_interface = interface;
    m_address = address;
    m_name = name;

    initUdpConnections();
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
    // qDebug() << tmp;
    res = command_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);
    if(res < 0)
    {
        qDebug() << "command_socket: Student IP Send 1 Failed!";
    }

    // command_timer->start(5000);

    // 启动音视频接收线程
    video_receiver = new VideoFrameReceiver(m_interface, m_address);
    audio_receiver = new AudioPackReceiver(m_interface, m_address);
    /*
     * 在线程中通过信号槽传递信息时，参数默认放到队列中
     * AudioPack 是自定义的结构体，不是 Qt 自带的参数结构，无法放入队列
     * 将不识别的参数结构进行注册，让 Qt 能够识别
     */
    qRegisterMetaType<AudioPack>("AudioPack");  // 注册 AudioPack 类型
    connect(video_receiver, SIGNAL(videoFrameReceived(QImage)), this, SLOT(on_videoFrameReceived(QImage)));
    connect(audio_receiver, SIGNAL(audioPackReceived(AudioPack)), this, SLOT(on_audioPackReceived(AudioPack)));
    video_receiver->start();
    audio_receiver->start();

    flag_startup = true;

    this->show();
}

void MainWindow::on_commandTimeOut()
{
    // 定时发送客户端信息
    qint64 res;
    QString tmp = QString("Student\n") + m_address.toString() + QString("\n") + m_name + QString("\nConnect");
    // qDebug() << tmp;
    res = command_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), teacher_address, command_port);
    if(res < 0)
    {
        qDebug() << "command_socket: Student IP Send 2 Failed!";
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

        // 接收到文件信息
        if(!QString(byteArray).indexOf("File\n"))
        {
            QString fileName = QString(byteArray).split("\n").at(1);
            qint64 fileSize = QString(byteArray).split("\n").at(2).toInt();

            // 询问是否接收文件
            QMessageBox::StandardButton button = QMessageBox::question(this,
                                                                   "File Transfer",
                                                                   "New File Transfer Requested. Receive?",
                                                                   QMessageBox::Yes | QMessageBox::No,
                                                                   QMessageBox::Yes);
            if(button == QMessageBox::Yes)
            {
                //保留源文件后缀名格式
                QString suffix = QFileInfo(fileName).suffix();

                QString tmp = fileName;
                // 另存为文件对话框
                fileName = QFileDialog::getSaveFileName(this,
                                                        "Save As",
                                                        QDir::homePath() + "/Desktop/" + fileName,
                                                        QString("*." + suffix));
                if(fileName.isEmpty())
                {
                    fileName = QDir::homePath() + "/Desktop/" + tmp;
                }
                qDebug() << QFileInfo(fileName).absoluteFilePath() << fileSize;

                /*
                 * Send TCP Connection to Server Here (in a new subthread)
                 */
            }
        }
    }
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

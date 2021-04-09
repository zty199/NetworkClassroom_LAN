#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QLocale>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_tray(new QSystemTrayIcon),
    t_menu(new QMenu),
    t_show(new QAction("Show MainWindow")),
    t_about(new QAction("About")),
    t_exit(new QAction("Exit")),
    availableCameras(QCameraInfo::availableCameras()),
    flag_camera(false),
    m_screen(QApplication::primaryScreen()),
    m_cursor(new QLabel),
    screen_timer(new QTimer(this)),
    m_screenPen(new ScreenPen),
    flag_screen(false),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioInput)),
    flag_audio(false),
    command_socket(new QUdpSocket(this)),
    groupAddress(GROUP_ADDR),
    command_port(COMMAND_PORT),
    video_threadPool(new QThreadPool(this)),
    audio_threadPool(new QThreadPool(this)),
    file_threadPool(new QThreadPool(this)),
    m_startup(new StartUpDialog(this)),
    command_timer(new QTimer(this)),
    stuNum(0),
    flag_startup(false),
    m_progress(new FileSendProgress(this)),
    m_textchat(new TextChatDialog(this)),
    flag_text(false)
{
    ui->setupUi(this);

    // 初始化摄像头设备
    initCamDevice();

    // 初始化音频输入设备
    initInputDevice();

    // 初始化 UI 设备列表
    initUI();

    // 初始化托盘
    initTray();

    // 初始化信号槽
    initConnections();

    qDebug() << "Initialization Finished!";
}

MainWindow::~MainWindow()
{
    delete video_threadPool;
    delete audio_threadPool;
    delete file_threadPool;

    if(flag_startup)
    {
        delete text_transceiver;
    }

    delete m_tray;
    delete ui;
}

void MainWindow::initUdpConnections()
{
    // 此处客户端返回为单播信息，不需要分为两个 socket
    command_socket->close();
    command_socket->bind(m_address, command_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);             // 绑定地址端口
    command_socket->joinMulticastGroup(groupAddress, m_interface);                                                      // 添加到组播，绑定组播网卡
    command_socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);                                                // 尝试优化套接字以降低延迟
    command_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);                                            // 设置套接字属性
#ifdef QT_DEBUG
    command_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 1);                                       // 本机允许接收组播回环信息
#else
    command_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);                                       // 本机禁止接收（本地测试中需置为 1）
#endif
    command_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, 1024 * 4);                             // 设置发送缓冲区
    command_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 4);                          // 设置接收缓冲区
}

void MainWindow::initCamDevice()
{
    m_viewFinder = new VideoSurface(this);
    m_camera = new QCamera(QCameraInfo::defaultCamera(), this);

    // 设置摄像头图像源
    m_camera->setViewfinder(m_viewFinder);
}

void MainWindow::initInputDevice()
{
    // 初始化默认音频输入格式
    format.setSampleRate(SAMPLE_RATE);
    format.setChannelCount(CHANNEL_COUNT);
    format.setSampleSize(SAMPLE_SIZE);
    format.setCodec("audio/pcm");
    format.setSampleType(QAudioFormat::SignedInt);
    format.setByteOrder(QAudioFormat::LittleEndian);

    // 判断音频输入设备是否支持该格式
    QAudioFormat format = this->format;
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
    if(!info.isFormatSupported(this->format))
    {
        format = info.nearestFormat(this->format);
    }

    // 初始化音频输入设备
    m_audioInput = new QAudioInput(info, format, this);
}

void MainWindow::initUI()
{
    // 初始化鼠标标记
    m_cursor->setWindowFlag(Qt::FramelessWindowHint, true);
    m_cursor->setWindowFlag(Qt::WindowStaysOnTopHint, true);
    m_cursor->resize(5, 5);
    m_cursor->setAutoFillBackground(true);
    m_cursor->setStyleSheet("background-color: rgb(255, 255, 255);");
    m_cursor->hide();

    // 初始化按钮样式
    ui->btn_screen->setStyleSheet("text-align: left; padding-left: 10px;");
    ui->btn_camera->setStyleSheet("text-align: left; padding-left: 10px;");
    ui->btn_audio->setStyleSheet("text-align: left; padding-left: 10px;");

    // 初始化屏幕画笔
    m_screenPen->hide();                    // 启动时不显示屏幕画笔
    ui->btn_screenPen->setDisabled(true);   // 禁用屏幕画笔按钮

    // 初始化屏幕分辨率和刷新率下拉框
    ui->cb_screenRes->addItem("360P(Low)", QSize(640, 360));
    ui->cb_screenRes->addItem("720P(Medium)", QSize(1280, 720));
    ui->cb_screenRes->addItem("1080P(High)", QSize(1920, 1080));
    ui->cb_screenHz->addItem("30Hz", 30);
    ui->cb_screenHz->addItem("60Hz", 15);

    // 禁用摄像头分辨率下拉框（摄像头设备启动后才可以使用）
    ui->cb_camRes->setDisabled(true);

    // 初始化主界面设备列表
    foreach(const QCameraInfo &camera, availableCameras)
    {
        ui->cb_camera->addItem(camera.description(), availableCameras.indexOf(camera));
    }

    foreach(const QAudioDeviceInfo &device, availableDevices)
    {
        ui->cb_device->addItem(device.deviceName(), availableDevices.indexOf(device));
    }

    // 显示启动界面
    m_startup->show();
}

void MainWindow::initTray()
{
    // 初始化托盘图标
    t_menu->addAction(t_show);
    t_menu->addAction(t_about);
    t_menu->addAction(t_exit);
    m_tray->setContextMenu(t_menu);
    m_tray->setIcon(QIcon::fromTheme(":/icons/icons/camera-start.svg"));
    m_tray->setToolTip(this->windowTitle());

    m_tray->setVisible(true);
}

void MainWindow::initConnections()
{
    connect(t_show, &QAction::triggered, this, [=]()
    {
        if(flag_startup)
        {
            this->setWindowState(Qt::WindowActive); // 若窗口处于最小化状态则需要该语句
            this->activateWindow();                 // 激活窗口获取焦点
            this->show();                           // 若窗口被关闭则需要该语句
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
        emit ui->btn_about->clicked();
    });
    connect(t_exit, &QAction::triggered, this, &MainWindow::on_exitTriggered);
    connect(m_tray, &QSystemTrayIcon::activated, this, &MainWindow::on_trayActivated);

    connect(screen_timer, SIGNAL(timeout()), this, SLOT(on_screenTimeOut()));
    connect(this, SIGNAL(volumeChanged(int)), this, SLOT(on_volumeChanged(int)));
    connect(screen_timer, SIGNAL(timeout()), this, SLOT(on_mouseMove()));    // 屏幕共享时标记鼠标位置
    connect(m_startup, SIGNAL(multicastReady(QNetworkInterface,QHostAddress,QString)), this, SLOT(on_multicastReady(QNetworkInterface,QHostAddress,QString)));
    connect(m_startup, SIGNAL(multicastNotReady()), this, SLOT(on_multicastNotReady()));
    connect(m_startup, SIGNAL(startUp()), this, SLOT(on_startUp()));
    connect(command_timer, SIGNAL(timeout()), this, SLOT(on_commandTimeOut()));
    connect(m_textchat, SIGNAL(textSend(QString)), this, SLOT(on_textSend(QString)));
}

void MainWindow::initCamera()
{
    // 初始化摄像头支持的图像格式列表（只取 JPEG 格式）
    viewSets = m_camera->supportedViewfinderSettings();
    foreach(const QCameraViewfinderSettings &viewSet, viewSets)
    {
        // qDebug() << "max rate = " << viewSet.maximumFrameRate() << " min rate = " << viewSet.minimumFrameRate() << " resolution = " << viewSet.resolution() << " Format = " << viewSet.pixelFormat() << "" << viewSet.pixelAspectRatio();
        QString resolution = QString::number(viewSet.resolution().width()) + "x" + QString::number(viewSet.resolution().height());
        if(ui->cb_camRes->findText(resolution) < 0)
        {
            ui->cb_camRes->addItem(resolution, viewSets.indexOf(viewSet));
        }
    }
}

void MainWindow::on_exitTriggered(bool checked)
{
    Q_UNUSED(checked)

    // 停止所有共享
    if(flag_camera)
    {
        emit ui->btn_camera->clicked();
    }
    if(flag_screen)
    {
        emit ui->btn_screen->clicked();
    }
    if(flag_audio)
    {
        emit ui->btn_audio->clicked();
    }

    // 等待所有子线程结束
    video_threadPool->clear();
    video_threadPool->waitForDone();

    audio_threadPool->clear();
    audio_threadPool->waitForDone();

    file_threadPool->clear();
    file_threadPool->waitForDone();

    if(flag_startup)
    {
        text_transceiver->quit();
        text_transceiver->wait();
    }

    // 向客户端发送停止信号
    command_socket->writeDatagram(QString("Stop").toUtf8().data(), QString("Stop").toUtf8().size(), groupAddress, command_port);

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

void MainWindow::on_btn_camera_clicked()
{
    // 判断摄像头开关状态并进行对应操作
    if(!flag_camera)
    {
        m_camera->start();
        connect(m_viewFinder, SIGNAL(videoFrameChanged(QVideoFrame)), this, SLOT(on_videoFrameChanged(QVideoFrame)));
        qDebug() << "Camera Started!";

        ui->btn_camera->setIcon(QIcon::fromTheme(":/icons/icons/camera-start.svg"));

        // 摄像头设备启动后才能获取支持的图像格式列表
        initCamera();

        ui->cb_camRes->setEnabled(true);
        flag_camera = true;

        // 摄像头功能与屏幕共享功能互斥（暂时）
        flag_screen = true;
        emit ui->btn_screen->clicked();
    }
    else
    {
        m_viewFinder->disconnect();
        m_camera->stop();
        qDebug() << "Camera Stopped!";

        ui->btn_camera->setIcon(QIcon::fromTheme(":/icons/icons/camera-stop.svg"));

        video_threadPool->clear();
        video_threadPool->waitForDone();
        command_socket->writeDatagram(QString("Stop").toUtf8().data(), QString("Stop").toUtf8().size(), groupAddress, command_port);

        ui->videoViewer->clear();

        /*
         * 摄像头关闭后，分辨率下拉框清空，
         * 直接清空会触发对应选项，须先断开信号槽
         */
        ui->cb_camRes->setDisabled(true);
        ui->cb_camRes->disconnect();
        ui->cb_camRes->clear();
        connect(ui->cb_camRes, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cb_camRes_currentIndexChanged(int)));

        flag_camera = false;

        // 刷新摄像头可用设备列表
        if(availableCameras == QCameraInfo::availableCameras())
        {
            return;
        }

        int index = -1;
        QCameraInfo curCam;
        if(ui->cb_camera->currentIndex() > index)
        {
            curCam = availableCameras.at(ui->cb_camera->currentIndex());
        }
        ui->cb_camera->disconnect();
        ui->cb_camera->clear();

        availableCameras = QCameraInfo::availableCameras();
        if(!availableCameras.isEmpty())
        {
            // 记录当前设备选项
            for(int i = 0; i < availableCameras.size(); i++)
            {
                ui->cb_camera->addItem(availableCameras.at(i).description(), i);
                if(availableCameras.at(i).deviceName() == curCam.deviceName())
                {
                    index = i;
                }
            }

            if(index < 0)
            {
                index = 0;
            }

            ui->cb_camera->setCurrentIndex(index);
        }

        connect(ui->cb_camera, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cb_camera_currentIndexChanged(int)));
    }
}

void MainWindow::on_cb_camera_currentIndexChanged(int index)
{
    if(flag_camera)
    {
        m_viewFinder->disconnect();
        m_camera->stop();
        ui->videoViewer->clear();
        ui->cb_camRes->disconnect();
        ui->cb_camRes->clear();
        connect(ui->cb_camRes, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cb_camRes_currentIndexChanged(int)));
    }

    delete m_camera;
    delete m_viewFinder;
    m_viewFinder = new VideoSurface(this);
    m_camera = new QCamera(availableCameras.at(index));
    m_camera->setViewfinder(m_viewFinder);

    if(flag_camera)
    {
        m_camera->start();
        connect(m_viewFinder, SIGNAL(videoFrameChanged(QVideoFrame)), this, SLOT(on_videoFrameChanged(QVideoFrame)));
        initCamera();
    }

    qDebug() << "Camera: " << availableCameras.at(index).description();
}

void MainWindow::on_cb_camRes_currentIndexChanged(int index)
{
    m_camera->setViewfinderSettings(viewSets.at(index));
    qDebug() << "Resolution: " << viewSets.at(index).resolution().width() << "x" << viewSets.at(index).resolution().height();
}

void MainWindow::on_videoFrameChanged(QVideoFrame frame)
{
    QVideoFrame tmp(frame);
    tmp.map(QAbstractVideoBuffer::ReadOnly);
    if(!tmp.isValid())
    {
        // qDebug() << "Empty Frame!";
        return;
    }

    // 通过 QVideoFrame 对象构建 QImage 对象
    QImage image(tmp.bits(),
                 tmp.width(),
                 tmp.height(),
                 tmp.bytesPerLine(),
                 QVideoFrame::imageFormatFromPixelFormat(tmp.pixelFormat()));

    // 释放拷贝内存
    tmp.unmap();

    // 如果图像未变化则不发送（占 CPU，需要其他方式处理）
    /*
    static QImage oldImage;

    if(oldImage == image)
    {
        return;
    }
    oldImage = image;
    */

    // 将 QImage 对象上下镜像   * Linux 不需要......
#ifdef Q_OS_WINDOWS
    image = image.mirrored(false, true);    // 左右/上下镜像
#elif defined Q_OS_LINUX
    image = image.mirrored(false, false);
#endif
    image = image.scaled(image.size().boundedTo(QSize(1920, 1080)), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 分辨率高于 1080p 则压缩

    if(image.size().height() < 720)
    {
        video_threadPool->setMaxThreadCount(1);
    }
    else
    {
        /*
         * 对 CPU 单核性能较弱的机型来说，建议使用多线程处理 720p 图像
         * 由于接收端为单线程接收，不建议使用过多线程
         */
        video_threadPool->setMaxThreadCount(2);
    }
    video_threadPool->start(new VideoFrameSender(m_interface, m_address, image, this));
}

void MainWindow::on_btn_screen_clicked()
{
    if(!flag_screen)
    {
        ui->btn_screenPen->setEnabled(true);    // 启用屏幕画笔按钮

        screen_timer->setInterval(screenTimer);
        screen_timer->start();                  // 启用计时器开始截图

        m_cursor->show();                       // 显示指针标记

        qDebug() << "Screen Share Started!";

        ui->btn_screen->setIcon(QIcon::fromTheme(":/icons/icons/screen-share-start.svg"));

        flag_screen = true;

        flag_camera = true;
        emit ui->btn_camera->clicked();
    }
    else
    {
        m_cursor->hide();
        screen_timer->stop();
        qDebug() << "Screen Share Stopped!";

        ui->btn_screen->setIcon(QIcon::fromTheme(":/icons/icons/screen-share-stop.svg"));

        // 终止视频传输时发送信号
        video_threadPool->clear();
        video_threadPool->waitForDone();
        command_socket->writeDatagram(QString("Stop").toUtf8(), QString("Stop").toUtf8().size(), groupAddress, command_port);

        ui->videoViewer->clear();

        flag_screen = false;
        ui->btn_screenPen->setDisabled(true);
    }
}

void MainWindow::on_cb_screenRes_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    screenRes = ui->cb_screenRes->currentData().value<QSize>();

    // 限制 1080p@30Hz
    if(screenRes == QSize(1920, 1080))
    {
        ui->cb_screenHz->removeItem(1);
    }
    else
    {
        if(ui->cb_screenHz->count() == 1)
        {
            ui->cb_screenHz->addItem("60Hz", 15);
        }
    }
}

void MainWindow::on_cb_screenHz_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    if(flag_screen)
    {
        screen_timer->stop();
    }

    screenTimer = ui->cb_screenHz->currentData().value<int>();
    screen_timer->setInterval(screenTimer);

    if(flag_screen)
    {
        screen_timer->start();
    }
}

void MainWindow::on_btn_screenPen_clicked()
{
    this->showMinimized();
    m_screenPen->showFullScreen();
}

void MainWindow::on_screenTimeOut()
{
    QImage image = m_screen->grabWindow(0).toImage();    // 截取桌面图像

    /*
    static QImage oldImage;

    if(oldImage == image)
    {
        return;
    }
    oldImage = image;
    */

    image = image.scaled(image.size().boundedTo(screenRes), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if(image.size().height() < 720)
    {
        video_threadPool->setMaxThreadCount(1);
    }
    else
    {
        video_threadPool->setMaxThreadCount(2);
    }
    video_threadPool->start(new VideoFrameSender(m_interface, m_address, image, this));
}

void MainWindow::on_mouseMove()
{
    // 由于屏幕共享看不到鼠标，尝试标记鼠标位置
    static QPoint oldPoint;
    QPoint point = QCursor::pos(); // 获取鼠标的绝对位置
    if(oldPoint != point)
    {
        m_cursor->move(point.x() + 1, point.y() + 1);
        // qDebug() << "鼠标移动 " << point;
        oldPoint = point;
    }
}

void MainWindow::on_videoFrameSent(QImage image)
{
    // 本地窗口预览（由子线程回传图像）
    ui->videoViewer->setPixmap(QPixmap::fromImage(image).scaled(ui->videoViewer->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
}

void MainWindow::on_btn_audio_clicked()
{
    if(!flag_audio)
    {
        m_audioDevice = m_audioInput->start();
        connect(m_audioDevice, SIGNAL(readyRead()), this, SLOT(on_deviceReadyRead()));
        qDebug() << "Audio Share Started!";

        ui->btn_audio->setIcon(QIcon::fromTheme(":/icons/icons/audio-input-start.svg"));

        flag_audio = true;
    }
    else
    {
        // 终止音频传输时，要先断开设备输入输出流，再停止设备，避免程序异常退出
        m_audioDevice->disconnect();
        m_audioInput->stop();
        qDebug() << "Audio Share Stopped!";

        ui->btn_audio->setIcon(QIcon::fromTheme(":/icons/icons/audio-input-stop.svg"));

        flag_audio = false;

        // 刷新音频输入可用设备列表
        if(availableDevices == QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
        {
            return;
        }

        int index = -1;
        QAudioDeviceInfo curInput;
        if(ui->cb_device->currentIndex() > index)
        {
            curInput = availableDevices.at(ui->cb_device->currentIndex());
        }
        ui->cb_device->disconnect();
        ui->cb_device->clear();

        availableDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
        if(!availableDevices.isEmpty())
        {
            for(int i = 0; i < availableDevices.size(); i++)
            {
                ui->cb_device->addItem(availableDevices.at(i).deviceName(), i);
                if(availableDevices.at(i).deviceName() == curInput.deviceName())
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
    // 切换音频输入设备时，要先断开设备输入输出流，再停止设备，避免程序异常退出
    if(flag_audio)
    {
        m_audioDevice->disconnect();
        m_audioInput->stop();
    }

    QAudioFormat format = this->format;
    QAudioDeviceInfo info(availableDevices.at(index));
    if(!info.isFormatSupported(this->format))
    {
        format = info.nearestFormat(this->format);
    }

    delete m_audioInput;
    m_audioInput = new QAudioInput(info, format, this);

    if(flag_audio)
    {
        m_audioDevice = m_audioInput->start();
        connect(m_audioDevice, SIGNAL(readyRead()), this, SLOT(on_deviceReadyRead()));
        m_audioInput->setVolume(qreal(ui->slider_volume->value()) / 100);
    }

    qDebug() << "Input Device: " << info.deviceName();
    qDebug() << format.sampleRate() << " " << format.channelCount() << " " << format.sampleSize() << " " << format.codec();
}

void MainWindow::on_slider_volume_valueChanged(int value)
{
    // 滑动条调节音频输入设备音量（仅调节音频流音量而非设备全局音量），范围 0.0 ~ 1.0
    m_audioInput->setVolume(qreal(value) / 100);
    emit this->volumeChanged(value);
}

void MainWindow::on_volumeChanged(int value)
{
    // 同步显示当前音量
    ui->volume->setText("Volume: " + QString::number(value));
}

void MainWindow::on_deviceReadyRead()
{
    // 初始化音频数据包结构
    static AudioPack ap;
    memset(&ap, 0, sizeof(ap));

    // 读入音频输入数据
    ap.len = static_cast<int>(m_audioDevice->read(ap.data, sizeof(ap.data)));
    // qDebug() << QString(vp.data).toUtf8();

    audio_threadPool->setMaxThreadCount(1);
    audio_threadPool->start(new AudioPackSender(m_interface, m_address, ap));
}

void MainWindow::on_btn_fileTrans_clicked()
{
    // 打开文件对话框
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Select File to Transfer",
                                                    QDir::homePath() + "/Desktop/",
                                                    "All Files (*.*)");
    if(fileName.isEmpty())
    {
        return;
    }

    // 打开文件
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Critical", "File Open Failed!", QMessageBox::Ok);
        return;
    }
    file.close();

    // 显示文件传输总进度
    m_progress->show();

    // 获取当前连接客户端列表
    QList<QHostAddress> stuList;
    QMap<QString, QString>::iterator it;
    for(it = stuMap.begin(); it != stuMap.end(); it++)
    {
        stuList.append(QHostAddress(it.key()));
    }

    qint32 listSize = stuList.size(), count = 0;
    file_threadPool->setMaxThreadCount(2);
    foreach(const QHostAddress &address, stuList)
    {
        file_threadPool->start(new FileSender(address, fileName));

        count++;
        m_progress->setValue(static_cast<int>(qreal(count) / listSize));
    }

    // 传输完成后隐藏进度条
    m_progress->hide();
}

void MainWindow::on_btn_signIn_clicked()
{
    // 生成文件名
    QString fileName = "SignIn_" + QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

    // 保存文件对话框
    fileName = QFileDialog::getSaveFileName(this,
                                            "Export Sign-In Sheet",
                                            QDir::homePath() + "/Desktop/" + fileName,
                                            "Text Files (*.txt)");
    if(fileName.isEmpty())
    {
        return;
    }

    // 写入文件
    QFile file(fileName);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Critical", "File Export Failed!", QMessageBox::Ok);
        return;
    }

    QTextStream stream(&file);
    QString newLine = "Index\tIP\t\tName\n";
    stream << newLine;

    int index = 1;
    QMap<QString, QString>::iterator it;
    for(it = stuMap.begin(); it != stuMap.end(); it++)
    {
        newLine = QString::number(index++) + "\t" + it.key() + "\t" + it.value() + "\n";
        stream << newLine;
    }
    stream.flush();
    file.close();

    // 询问是否打开所在文件夹
    QMessageBox::StandardButton res = QMessageBox::information(this,
                                                               "Information",
                                                               "File saved. Show in Explorer?",
                                                               QMessageBox::Ok | QMessageBox::Cancel);
    if(res == QMessageBox::Ok)
    {
        QDesktopServices::openUrl(QFileInfo(file).absolutePath());  // 打开文件所在文件夹
    }
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

void MainWindow::on_btn_about_clicked()
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
}

void MainWindow::on_multicastReady(QNetworkInterface interface, QHostAddress address, QString name)
{
    m_interface = interface;
    m_address = address;
    m_name = name;

    // 初始化 UDP 连接
    initUdpConnections();
    connect(command_socket, SIGNAL(readyRead()), this, SLOT(on_commandReadyRead()));
    emit command_socket->readyRead();

    // 定时组播服务端 IP
    command_timer->start(5000);
}

void MainWindow::on_multicastNotReady()
{
    command_timer->stop();
}

void MainWindow::on_startUp()
{
    // 启动文字聊天线程
    text_transceiver = new TextMsgTransceiver(m_interface, m_address, m_name);
    connect(text_transceiver, SIGNAL(textAppend(QString)), this, SLOT(on_textAppend(QString)));
    text_transceiver->start();

    this->show();

    flag_startup = true;
}

void MainWindow::on_commandTimeOut()
{
    // 定时组播服务端 IP
    qint64 res;
    QString tmp = "Teacher\n" + m_address.toString();
    res = command_socket->writeDatagram(tmp.toUtf8().data(), tmp.toUtf8().size(), groupAddress, command_port);
    if(res < 0)
    {
        qDebug() << "commandsend_socket: Teacher IP Send Failed!";
    }
}

void MainWindow::on_commandReadyRead()
{
    // 接收客户端发送信息
    qint64 res;
    QByteArray byteArray;

    while(command_socket->hasPendingDatagrams())
    {
        byteArray.resize(static_cast<qint32>(command_socket->pendingDatagramSize()));
        res = command_socket->readDatagram(byteArray.data(), byteArray.size());
        if(res < 0)
        {
            qDebug() << "commandsend_socket: Read Datagram Failed!";
        }

        // 接收到学生信息
        if(!QString(byteArray).indexOf("Student\n"))
        {
            QHostAddress stu(QString(byteArray).split("\n").at(1));
            QString stuName = QString(byteArray).split("\n").at(2);
            QString op = QString(byteArray).split("\n").at(3);
            // qDebug() << stu << stuName << op;

            if(op == "Connect")
            {
                if(!stuMap.contains(stu.toString()))
                {
                    stuNum++;
                }
                stuMap[stu.toString()] = stuName;
            }
            if(op == "Disconnect")
            {
                if(stuMap.contains(stu.toString()))
                {
                    QMap<QString, QString>::iterator it = stuMap.find(stu.toString());
                    stuMap.erase(it);
                    stuNum--;
                }
            }
            emit studentConnected(stuNum);
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

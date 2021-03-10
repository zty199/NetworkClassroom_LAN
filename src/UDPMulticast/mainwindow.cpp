#include "mainwindow.h"
#include "ui_mainwindow.h"

// #include <QNetworkInterface>
#include <QBuffer>
#include <QEvent>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    availableCameras(QCameraInfo::availableCameras()),
    // m_camera(new QCamera(QCameraInfo::defaultCamera(), this)),
    m_viewFinder(new VideoSurface(this)),
    flag_camera(false),
    m_screen(QApplication::primaryScreen()),
    availableDevices(QAudioDeviceInfo::availableDevices(QAudio::AudioInput)),
    m_timer(new QTimer(this)),
    flag_screen(false),
    flag_audio(false),
    video_socket(new QUdpSocket(this)),
    audio_socket(new QUdpSocket(this)),
    groupAddress("239.0.0.1"),
    video_port(8888),
    audio_port(8889)
{
    ui->setupUi(this);

    // 初始化 UDP 连接
    initUdpConnections();

    /*
     * 正常情况下，载入设备列表（调用 initUI();）时会自动触发下拉框选择第一项，
     * 可以不用对摄像头设备进行初始化。
     *
     * 但是，实际测试中，这样操作会导致默认音频输入设备格式不对，原因未知。
     * 故先对音频输入设备进行单独初始化。
     */
    initInputDevice();

    // 初始化 UI 设备列表
    initUI();

    // 初始化信号槽
    initConnections();

    // 设置摄像头图像源
    m_camera->setViewfinder(m_viewFinder);

    // 初始化计时器状态（用于桌面共享）
    m_timer->stop();

    qDebug() << Qt::endl << "Initialization Finished!" << Qt::endl;
}

MainWindow::~MainWindow()
{
    video_socket->writeDatagram(QString("Stop").toUtf8().data(), QString("Stop").toUtf8().size(), groupAddress, video_port);
    delete ui;
}

void MainWindow::initUdpConnections()
{
    // 检查网卡是否支持 UDP 组播
    /*
    QNetworkInterface intf;
    QList<QNetworkInterface> list = QNetworkInterface::allInterfaces();  // 获取系统里所有的网卡对象;
    foreach(intf, list)
    {
        // 找出处在执行状态，能支持组播的网卡对象
        if((intf.flags() & QNetworkInterface::IsRunning) && (intf.flags() & QNetworkInterface::CanMulticast))
        {
            break;
        }
    }
    qDebug() << intf.name();
    */

    // video_socket->bind(QHostAddress::AnyIPv4, video_port, QUdpSocket::ReuseAddressHint | QUdpSocket::ShareAddress);  // 绑定广播地址端口（发送端可以不绑定）
    // video_socket->setMulticastInterface(intf);                                   // 设置组播网卡
    video_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);          // 设置套接字属性
    audio_socket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);          // 设置套接字属性
    // video_socket->setSocketOption(QAbstractSocket::MulticastLoopbackOption, 0);  // 禁止本机接收
}

void MainWindow::initInputDevice()
{
    // 初始化默认音频输入格式
    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(16);
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
    m_audioDevice = m_audioInput->start();
    connect(m_audioDevice, SIGNAL(readyRead()), this, SLOT(on_audioReadyRead()));
    qDebug() << "Input Device: " << info.deviceName();
    qDebug() << format.sampleRate() << " " << format.channelCount() << " " << format.sampleSize() << " " << format.codec();

    // 启动时不传输音频
    flag_audio = true;
    on_btn_audio_clicked();
}

void MainWindow::initUI()
{
    ui->cb_resolution->setDisabled(true);   // 禁用摄像头分辨率下拉框（摄像头设备初始化后才可以使用）

    // 初始化主界面设备列表（可用设备列表为空则禁用相关选项）
    if(availableCameras.isEmpty())
    {
        ui->btn_camera->setDisabled(true);
        ui->cb_camera->setDisabled(true);
    }
    else
    {
        foreach(const QCameraInfo &camera, availableCameras)
        {
            ui->cb_camera->addItem(camera.description(), availableCameras.indexOf(camera));
        }
    }

    if(availableDevices.isEmpty())
    {
        ui->btn_audio->setDisabled(true);
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
    connect(m_viewFinder, SIGNAL(videoFrameChanged(QVideoFrame)), this, SLOT(on_videoFrameChanged(QVideoFrame)));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(on_timeOut()));
    // connect(m_timer, SIGNAL(timeout()), this, SLOT(on_mouseMove()));    // 屏幕共享时标记鼠标位置
}

void MainWindow::initCamera()
{
    // 初始化摄像头支持的图像格式列表（只取 JPEG 格式）
    viewSets = m_camera->supportedViewfinderSettings();
    foreach(const QCameraViewfinderSettings &viewSet, viewSets)
    {
        // qDebug() << "max rate = " << viewSet.maximumFrameRate() << " min rate = " << viewSet.minimumFrameRate() << " resolution = " << viewSet.resolution() << " Format = " << viewSet.pixelFormat() << "" << viewSet.pixelAspectRatio();
        if(viewSet.pixelFormat() == QVideoFrame::Format_Jpeg)
        {
            ui->cb_resolution->addItem(QString::number(viewSet.resolution().width()) + "x" + QString::number(viewSet.resolution().height()), viewSets.indexOf(viewSet));
        }
    }
}

void MainWindow::on_btn_camera_clicked()
{
    // 判断摄像头开关状态并进行对应操作
    if(!flag_camera)
    {
        m_camera->start();
        qDebug() << "Camera Started!";

        // 摄像头启动后才能获取支持的图像格式列表
        initCamera();

        //ui->cb_camera->setDisabled(true);
        ui->cb_resolution->setEnabled(true);
        flag_camera = true;

        // 摄像头功能与屏幕共享功能互斥（暂时）
        flag_screen = true;
        on_btn_screen_clicked();
    }
    else
    {
        // 终止视频传输时发送信号
        video_socket->writeDatagram(QString("Stop").toUtf8().data(), QString("Stop").toUtf8().size(), groupAddress, video_port);

        m_camera->stop();
        qDebug() << "Camera Stopped!";
        ui->videoViewer->clear();
        //ui->cb_camera->setEnabled(true);

        /*
         *  摄像头关闭后，分辨率下拉框清空，
         *  直接清空会触发对应选项，须先断开信号槽
         */
        ui->cb_resolution->setDisabled(true);
        ui->cb_resolution->disconnect();
        ui->cb_resolution->clear();
        connect(ui->cb_resolution, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cb_resolution_currentIndexChanged(int)));

        flag_camera = false;
    }
}

void MainWindow::on_cb_camera_currentIndexChanged(int index)
{
    if(flag_camera)
    {
        m_camera->stop();
        ui->cb_resolution->disconnect();
        ui->cb_resolution->clear();
    }

    m_camera = new QCamera(availableCameras.at(index));

    if(flag_camera)
    {
        m_camera->start();
        initCamera();
        connect(ui->cb_resolution, SIGNAL(currentIndexChanged(int)), this, SLOT(on_cb_resolution_currentIndexChanged(int)));
    }

    qDebug() << "Camera: " << availableCameras.at(index).description();
}

void MainWindow::on_cb_resolution_currentIndexChanged(int index)
{
    m_camera->setViewfinderSettings(viewSets.at(index));
    qDebug() << "Resolution: " << viewSets.at(index).resolution().width() << "x" << viewSets.at(index).resolution().height();
}

void MainWindow::on_videoFrameChanged(QVideoFrame frame)
{
    // static QImage oldImage;

    QVideoFrame tmp(frame);
    tmp.map(QAbstractVideoBuffer::ReadOnly);
    if(!tmp.isValid())
    {
        qDebug() << "Empty Frame!";
        return;
    }

    // 通过 QVideoFrame 对象构建 QImage 对象
    QImage image(tmp.bits(),
                 tmp.width(),
                 tmp.height(),
                 tmp.bytesPerLine(),
                 QVideoFrame::imageFormatFromPixelFormat(tmp.pixelFormat()));

    // 如果图像未变化则不发送（占 CPU，需要其他方式处理）
    /*
    if(oldImage == image)
    {
        return;
    }
    oldImage = image;
    */

    // 将 QImage 对象上下镜像   * Linux 似乎不需要......
#ifdef Q_OS_WINDOWS
    image = image.mirrored(false, true);    // 左右/上下镜像
#elif defined Q_OS_LINUX
    image = image.mirrored(false, false);   // 左右/上下镜像
#endif
    image.scaled(image.size().boundedTo(QSize(1920, 1080)), Qt::KeepAspectRatio, Qt::SmoothTransformation); // 分辨率高于 1080p 则压缩

    // 本地窗口预览
    ui->videoViewer->setPixmap(QPixmap::fromImage(image).scaled(ui->videoViewer->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

    // 暂存帧图像
    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer, "JPEG");

    qint64 res,                 // UDP 结果
            sentBytes = 0,      // 已发送字节数
            len = UDP_MAX_SIZE; // 本次发送字节数

    video_socket->writeDatagram(QString("size=%1").arg(byteArray.size()).toUtf8(),
                                QString("size=%1").arg(byteArray.size()).toUtf8().size(),
                                groupAddress,
                                video_port);
    video_socket->waitForBytesWritten();    // 等待数据发送完成

    // qDebug() << "totalBytes = " << byteArray.size();

    while(sentBytes < byteArray.size())
    {
        if(sentBytes + len > byteArray.size())
        {
            len = byteArray.size() - sentBytes;
        }
        else
        {
            len = UDP_MAX_SIZE;
        }

        if((res = video_socket->writeDatagram(byteArray.data() + sentBytes, len, groupAddress, video_port)) != len)
        {
            qDebug() << "res = " << res << " sentBytes = " << len;
            break;
        }
        video_socket->waitForBytesWritten();

        sentBytes += len;
    }
}

void MainWindow::on_btn_screen_clicked()
{
    if(!flag_screen)
    {
        this->showMinimized();  // 屏幕共享时隐藏主窗口
        m_timer->start(40);     // 每隔 40ms 触发（等同于 25Hz 刷新率）
        qDebug() << "Screen Share Started!";
        flag_screen = true;
        flag_camera = true;
        on_btn_camera_clicked();
    }
    else
    {
        video_socket->writeDatagram(QString("Stop").toUtf8(), QString("Stop").toUtf8().size(), groupAddress, video_port);
        m_timer->stop();
        qDebug() << "Screen Share Stopped!";
        ui->videoViewer->clear();
        flag_screen = false;
    }
}

void MainWindow::on_timeOut()
{
    // static QImage oldImage;

    QImage image = m_screen->grabWindow(0).toImage();    // 截取桌面图像

    /*
    if(oldImage == image)
    {
        return;
    }
    oldImage = image;
    */

    image.scaled(image.size().boundedTo(QSize(1920, 1080)), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 本地窗口预览
    ui->videoViewer->setPixmap(QPixmap::fromImage(image).scaled(ui->videoViewer->size(), Qt::KeepAspectRatio, Qt::FastTransformation));

    QByteArray byteArray;
    QBuffer buffer(&byteArray);

    buffer.open(QIODevice::ReadWrite);
    image.save(&buffer, "JPEG");

    qint64 res, sentBytes = 0, len = UDP_MAX_SIZE;

    video_socket->writeDatagram(QString("size=%1").arg(byteArray.size()).toUtf8(),
                                QString("size=%1").arg(byteArray.size()).toUtf8().size(),
                                groupAddress,
                                video_port);
    video_socket->waitForBytesWritten();

    // qDebug() << "totalBytes = " << byteArray.size();

    while(sentBytes < byteArray.size())
    {
        if(sentBytes + len > byteArray.size())
        {
            len = byteArray.size() - sentBytes;
        }
        else
        {
            len = UDP_MAX_SIZE;
        }

        if((res = video_socket->writeDatagram(byteArray.data() + sentBytes, len, groupAddress, video_port)) != len)
        {
            qDebug() << "res = " << res << " sentBytes = " << len;
            break;
        }
        video_socket->waitForBytesWritten();

        sentBytes += len;
    }
}

void MainWindow::on_btn_audio_clicked()
{
    if(!flag_audio)
    {
        m_audioDevice = m_audioInput->start();
        connect(m_audioDevice, SIGNAL(readyRead()), this, SLOT(on_audioReadyRead()));
        qDebug() << "Audio Share Started!";
        flag_audio = true;
    }
    else
    {
        // 终止音频传输时，需要停止设备并断开设备输入输出流，避免内存溢出
        m_audioInput->stop();
        m_audioDevice->disconnect();

        qDebug() << "Audio Share Stopped!";
        flag_audio = false;
    }
}

void MainWindow::on_cb_device_currentIndexChanged(int index)
{
    // 切换音频设备时，需要停止设备并断开设备输入输出流，避免内存溢出
    if(flag_audio)
    {
        m_audioInput->stop();
        m_audioDevice->disconnect();
    }

    QAudioFormat format = this->format;
    QAudioDeviceInfo info(availableDevices.at(index));
    if(!info.isFormatSupported(this->format))
    {
        format = info.nearestFormat(this->format);
    }

    m_audioInput = new QAudioInput(info, format, this);

    if(flag_audio)
    {
        m_audioDevice = m_audioInput->start();
        connect(m_audioDevice, SIGNAL(readyRead()), this, SLOT(on_audioReadyRead()));
    }

    qDebug() << "Input Device: " << info.deviceName();
    qDebug() << format.sampleRate() << " " << format.channelCount() << " " << format.sampleSize() << " " << format.codec();
}

void MainWindow::on_audioReadyRead()
{
    qint64 res; // UDP 结果

    // 初始化音频数据包结构
    videoPack vp;
    memset(&vp, 0, sizeof(vp));

    // 读入音频输入数据
    vp.lens = static_cast<int>(m_audioDevice->read(vp.data, sizeof(vp.data)));
    // qDebug() << QString(vp.data).toUtf8();

    if((res = audio_socket->writeDatagram(reinterpret_cast<const char*>(&vp), sizeof(vp), groupAddress, audio_port)) != sizeof(vp))
    {
        qDebug() << "res = " << res << " sentBytes = " << sizeof(vp);
        return;
    }
}

void MainWindow::on_mouseMove()
{
    // 由于屏幕共享看不到鼠标，尝试标记鼠标位置
    static QLabel label;
    label.setWindowFlag(Qt::FramelessWindowHint, true);
    label.resize(20, 20);
    label.setAutoFillBackground(true);
    label.setStyleSheet("background-color: rgb(0, 0, 0);");
    label.show();

    static QPoint oldPoint;
    QPoint point = QCursor::pos(); // 获取鼠标的绝对位置
    if(oldPoint != point)
    {
        label.move(point.x() + 1, point.y() + 1);
        qDebug() << "鼠标移动";
        qDebug() << point;
        oldPoint = point;
    }
}
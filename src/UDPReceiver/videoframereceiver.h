#ifndef VIDEOFRAMERECEIVER_H
#define VIDEOFRAMERECEIVER_H

#include <QThread>
#include <QUdpSocket>
#include <QImage>

class VideoFrameReceiver : public QThread
{
    Q_OBJECT

public:
    VideoFrameReceiver();
    ~VideoFrameReceiver();

protected:
    void run() override;

private:
    QUdpSocket *video_socket;
    QHostAddress groupAddress;
    quint16 video_port;

    struct PackageHeader
    {
        quint32 uTransPackageHdrSize;
        quint32 uTransPackageSize;
        quint32 uDataSize;
        quint32 uDataPackageNum;
        quint32 uDataPackageCurrIndex;
        quint32 uDataPackageOffset;
    };

    struct VideoPack
    {
        uchar data[1920 * 1080 * 4];
        int len;
    };

private slots:
    void on_videoReadyRead();

signals:
    void videoFrameReceived(QImage);

};

#endif // VIDEOFRAMERECEIVER_H

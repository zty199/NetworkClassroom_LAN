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
    ~VideoFrameReceiver() override;

protected:
    void run() override;

private:
    QUdpSocket *video_socket;
    QHostAddress groupAddress;
    quint16 video_port;

    struct PackageHeader
    {
        qint32 TransPackageHdrSize;
        qint32 TransPackageSize;
        qint32 DataSize;
        qint32 DataPackageNum;
        qint32 DataPackageCurrIndex;
        qint32 DataPackageOffset;
        qint64 DataPackageTimeStamp;
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

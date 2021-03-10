#include "videosurface.h"

VideoSurface::VideoSurface(QObject *parent):
    QAbstractVideoSurface(parent)
{

}

QList<QVideoFrame::PixelFormat> VideoSurface::supportedPixelFormats(
        QAbstractVideoBuffer::HandleType) const
{
    // 返回图像的处理格式
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB32;
}

bool VideoSurface::present(const QVideoFrame &frame)
{
    emit videoFrameChanged(frame);
    return true;
}

#ifndef UTIL_H
#define UTIL_H

#include <QtGlobal>

// UDP 包头
struct PackageHeader
{
    qint32 TransPackageHdrSize;     // 包头大小(sizeof(PackageHeader))
    qint32 TransPackageSize;        // 当前包头的大小(sizeof(PackageHeader) + 当前数据包长度)
    qint32 DataSize;                // 数据的总大小
    qint32 DataPackageNum;          // 数据被分成包的个数
    qint32 DataPackageCurrIndex;    // 数据包当前的帧号
    qint32 DataPackageOffset;       // 数据包在整个数据中的偏移
    qint64 DataPackageTimeStamp;    // 数据包时间戳
};

// 视频帧数据包
struct VideoPack
{
    uchar data[1920 * 1080 * 4];    // 单帧视频数据包最大 1080p 位图
    int len;
};

// 音频数据包
struct AudioPack
{
    char data[1024 * 16];           // 单个音频数据包大小设为 16K，音质 44K/128Kbps（？）
    int len;
};

#endif // UTIL_H

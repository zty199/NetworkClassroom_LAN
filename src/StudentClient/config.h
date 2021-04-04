#ifndef CONFIG_H
#define CONFIG_H

#define PACKET_MAX_SIZE int(1200)                       // 数据包最大长度   * MTU = 1500，故数据包大小 1500 - 20（IP头）- 8（UDP头） = 1472

#define GROUP_ADDR      QHostAddress("239.0.0.1")       // 组播地址
#define COMMAND_PORT    quint16(8887)                   // 命令组播端口
#define VIDEO_PORT      quint16(8888)                   // 视频组播端口
#define AUDIO_PORT      quint16(8889)                   // 音频组播端口
#define TEXT_PORT       quint16(8890)                   // 文字组播端口
#define FILE_PORT       quint16(8888)                   // 文件传输端口

#define SAMPLE_RATE     int(44100)                      // 采样频率
#define SAMPLE_SIZE     int(16)                         // 采样位数
#define CHANNEL_COUNT   int(2)                          // 声道数

#endif // CONFIG_H

# 局域网网络课堂软件
 中文 | [英文](https://github.com/zty199/NetworkClassroom_LAN)

 一个简单的基于 UDP 组播的网络课堂软件 Demo，包含基本的多媒体传输。仅适用于局域网。

## 已实现的功能
* 基于 **UDP 组播** 实现了基本的多媒体传输功能，包括摄像头、桌面共享，语音、立体声混音设备等数据传输。

* 支持 1080p@30Hz 或 720p@60Hz 视频传输。（对于 **CPU** 和 **网络带宽** 要求较高）

* 支持摄像头、声音输入输出设备动态切换。

* 支持摄像头分辨率调节。

* 支持音频输入输出流音量调节。（仅调节 **应用程序** 音量，音频设备全局音量由系统控制）

* 支持刷新可用设备列表。

* 支持屏幕共享时使用白板和屏幕标注。

* 支持多线程收发数据，避免单线程造成 GUI 卡顿。

* 支持文件传输，基于 TCP 协议。

* 支持文本消息发送。

* 支持学生签到表导出。

## 目前存在的问题
* 使用无线网卡时，UDP 组播极为 **不稳定** ，收发数据包均存在严重丢包问题。推荐使用 **有线网卡** 。

* 目前摄像头和桌面画面由 CPU（？）直接编码为 **JPEG** 格式并缩放进行传输，原始画面分辨率高于 **1080p** 会造成卡顿。

* Linux 下获取的音频/视频设备名称 **不明确** （由设备驱动决定），声音设备列表中大多数设备不可用。（多数是声卡硬件原始预留的端口，未连接任何物理设备）

* 可用设备列表不能实时刷新，使用设备过程中移除设备可能造成程序崩溃，新插入设备不会自动识别，需要关闭对应功能后才能刷新设备列表。

## 后续计划实现的功能
* 尝试使用 ffmpeg 对视频流和音频流进行编码，降低 CPU 占用。

## 参考资料
* [Qt Documentation](https://doc.qt.io/)

* [Qt中文文档](https://www.qtdoc.cn/)

* [Qt通过UDP传图片，实现自定义分包和组包](https://blog.csdn.net/caoshangpa/article/details/52681572)

* [Qt5::QCamera查询和设置摄像头的分辨率和帧率](https://blog.csdn.net/qq_28581781/article/details/99707091)

* [QT组播实现多人屏幕共享的程序](https://blog.csdn.net/jklinux/article/details/72236372)

* [QT音频开发：使用QAudioInput+QAudioOutput实现录音机功能，支持选择指定声卡录音，指定扬声器放音。](https://blog.csdn.net/xiaolong1126626497/article/details/105669037)

* [Qt实现tcp发送和接收文件](https://blog.csdn.net/weixin_40355471/article/details/110391887)

* [Qt多网卡组播问题解决方法](https://blog.csdn.net/sun_xf1/article/details/106423552)

* [扯淡的多播参数IP_MULTICAST_LOOP](https://blog.csdn.net/weixin_34014277/article/details/89985878)

* [QT多线程的使用](https://www.cnblogs.com/coolcpp/p/qt-thread.html)

* [Qt QRunnable的使用](https://blog.csdn.net/qq_43711348/article/details/103983857)

* [QRunnable中，如何接收tcp连接信息](https://jingyan.baidu.com/article/dca1fa6f140f54f1a440520b.html)

* [QT编程问题小结（编译、多线程、UDP Socket等）](https://blog.csdn.net/rabbitjerry/article/details/70947807)

* [QT基于UDP通信的多线程编程问题](https://blog.csdn.net/kamereon/article/details/49582617)

* [解决 Qt 对象跨线程调用问题](https://blog.csdn.net/u012321968/article/details/108214644)

* [QMap 的增删改查](https://blog.csdn.net/hejinjing_tom_com/article/details/48103455)

* [Qt子窗体关闭时，不执行析构函数](https://blog.csdn.net/u012199908/article/details/40109169)

* [Qt最小化到托盘、恢复并置顶](https://blog.csdn.net/wayrboy/article/details/79117012)

* [qtSingleApplication使用总结](https://blog.csdn.net/iamsujin/article/details/53257038)

## 感谢
* [haidragon/Qtliveradio](https://github.com/haidragon/Qtliveradio)

* [sonichy/HTYScreenPen](https://github.com/sonichy/HTYScreenPen)

* [qtproject/qt-solutions](https://github.com/qtproject/qt-solutions)

## 声明
该项目为大学本科毕业设计，仅供学习交流使用。

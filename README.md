# NetworkClassroom_LAN
 [Chinese](https://gitee.com/zty199/NetworkClassroom_LAN) | English

 A simple network classroom Demo based on UDP Multicast, including basic multimedia transmission. Only works in LAN.

## Realized Features
* Support basic multimedia transmission based on **UDP Multicast** , including cam, screen share, mic, stereo mixer, etc.

* Support 1080p@30Hz or 720p@60Hz video transmission. (Because of HIGH **CPU** and **Network** consumption)

* Support dynamic switch between different devices (cam / audio IO).

* Support cam resolution adjustment.

* Support volume adjustment. (Only **current audio stream** affected, global device volume is controlled by system)

* Support available devices rescan.

* Support white board and screen mark when sharing screen.

* Support multithread data processing to avoid lags in GUI.

* Support file transmission based on TCP connection.

* Support text message transfer.

* Support student sign-in sheet export.

## Existing Issues
* When using wireless cards, UDP Multicast is extremely **Unreliable**, both sending and receiving ends meet serious packet loss. **Ethernet cards** recommended.

* Video frames captured from cam and screen are directly encoded into **JPEG** and scaled by CPU(?), source resolution higher than **1080p** may cause serious lags on GUI.

* Cam / Audio IO devices' names are Inaccurate on Linux (based on device drivers), most of audio IO devices shown in combobox don't work (not physically connected to any devices, just ports reserved).

* Available devices list can't refresh in real time. Choosing one that is removed may cause crash, and new devices connected won't show up unless corresponding function button is clicked.

## Future Plan
* Try to use ffmpeg to deal with audio and video.

## Reference
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

## Credit
* [haidragon/Qtliveradio](https://github.com/haidragon/Qtliveradio)

* [sonichy/HTYScreenPen](https://github.com/sonichy/HTYScreenPen)

* [itay-grudev/SingleApplication](https://github.com/itay-grudev/SingleApplication)

## Clarification
This project is my graduation design for undergraduates. Only for study and communication.

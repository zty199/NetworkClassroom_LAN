# NetworkClassroom_LAN
 [Chinese](https://gitee.com/zty199/NetworkClassroom_LAN) | English

 A simple network classroom Demo based on UDP Multicast, including basic multimedia transmission. Only works in LAN.

## Realized Features
* Support basic multimedia transmission based on UDP Multicast , including cam, screen share, mic, stereo mixer, etc.

* Support 1080p video transmission. (Temporarily limit to 720p because of HIGH CPU and Network consumption)

* Support dynamic switch between different devices (cam / audio IO).

* Support cam resolution adjustment.

* Support volume adjustment. (Only current audio stream affected)

## Existing Issues
* When using wireless cards, UDP Multicast is extremely Unreliable, both sending and receiving ends meet serious packet loss. Ethernet cards recommended.

* Video frames captured from cam and screen are directly encoded into JPEG by CPU(?), source resolution higher than 720p can cause serious lags on GUI.

* Cam / Audio IO devices' names are Inaccurate on Linux (based on device drivers), most of audio IO devices shown in combobox don't work (not physically connected to any devices, just ports reserved).

* Available devices list can't refresh in real time. Choosing one that is removed may cause crash, and new devices connected won't show up.

## Future Plan
Real-time Device List, White Board, Screen Mark, File Transfer, Text Message Transmission, Student Sign-in, etc.

## Reference
* [Qt Documentation](https://doc.qt.io/)

* [Qt中文文档](https://www.qtdoc.cn/)

* [Qt通过UDP传图片，实现自定义分包和组包](https://blog.csdn.net/caoshangpa/article/details/52681572)

* [Qt5::QCamera查询和设置摄像头的分辨率和帧率](https://blog.csdn.net/qq_28581781/article/details/99707091)

* [QT组播实现多人屏幕共享的程序](https://blog.csdn.net/jklinux/article/details/72236372)

* [QT音频开发：使用QAudioInput+QAudioOutput实现录音机功能，支持选择指定声卡录音，指定扬声器放音。](https://blog.csdn.net/xiaolong1126626497/article/details/105669037)

* [Qt多网卡组播问题解决方法](https://blog.csdn.net/sun_xf1/article/details/106423552)

## Clarification
This project is my graduation design for undergraduates. Only for study and communication.

#-------------------------------------------------
#
# Project created by QtCreator 2021-01-14T21:29:40
#
#-------------------------------------------------

QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TeacherServer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../../third-party/qtsingleapplication/src/qtsingleapplication.pri)

CONFIG += c++11

# 禁止输出 qWarning / qDebug 信息
CONFIG(release, debug|release): DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT

SOURCES += \
        audiopacksender.cpp \
        filesender.cpp \
        filesendprogress.cpp \
        main.cpp \
        mainwindow.cpp \
        screenpen.cpp \
        startupdialog.cpp \
        textchatdialog.cpp \
        textmsgtransceiver.cpp \
        videoframesender.cpp \
        videosurface.cpp

HEADERS += \
        audiopacksender.h \
        config.h \
        filesender.h \
        filesendprogress.h \
        mainwindow.h \
        screenpen.h \
        startupdialog.h \
        textchatdialog.h \
        textmsgtransceiver.h \
        util.h \
        videoframesender.h \
        videosurface.h

FORMS += \
        filesendprogress.ui \
        mainwindow.ui \
        screenpen.ui \
        startupdialog.ui \
        textchatdialog.ui

RESOURCES += \
        resources.qrc

TRANSLATIONS += \
        translations/zh_CN.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

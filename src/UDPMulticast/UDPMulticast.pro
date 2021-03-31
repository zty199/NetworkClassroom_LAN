#-------------------------------------------------
#
# Project created by QtCreator 2021-01-14T21:29:40
#
#-------------------------------------------------

QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UDPMulticast
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

CONFIG += c++11

SOURCES += \
        audiopacksender.cpp \
        filesender.cpp \
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
        mainwindow.h \
        screenpen.h \
        startupdialog.h \
        textchatdialog.h \
        textmsgtransceiver.h \
        util.h \
        videoframesender.h \
        videosurface.h

FORMS += \
        mainwindow.ui \
        screenpen.ui \
        startupdialog.ui \
        textchatdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

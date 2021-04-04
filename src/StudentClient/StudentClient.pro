#-------------------------------------------------
#
# Project created by QtCreator 2021-01-15T02:05:18
#
#-------------------------------------------------

QT       += core gui multimedia network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = StudentClient
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

CONFIG += c++11 debug_and_release

# Release 禁止输出 qWarning / qDebug 信息
CONFIG (release, debug|release)
{
DEFINES += QT_NO_WARNING_OUTPUT QT_NO_DEBUG_OUTPUT
}

SOURCES += \
        audiopackreceiver.cpp \
        filereceiver.cpp \
        main.cpp \
        mainwindow.cpp \
        startupdialog.cpp \
        textchatdialog.cpp \
        textmsgtransceiver.cpp \
        videoframereceiver.cpp

HEADERS += \
        audiopackreceiver.h \
        config.h \
        filereceiver.h \
        mainwindow.h \
        startupdialog.h \
        textchatdialog.h \
        textmsgtransceiver.h \
        util.h \
        videoframereceiver.h

FORMS += \
        mainwindow.ui \
        startupdialog.ui \
        textchatdialog.ui

RESOURCES += \
        resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

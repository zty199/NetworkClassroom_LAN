#include "mainwindow.h"
#include <QtSingleApplication>

#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QtSingleApplication a(QString("StudentClient"), argc, argv);
    if(a.isRunning())
    {
        a.sendMessage("already_running");
        return EXIT_SUCCESS;
    }

    a.setQuitOnLastWindowClosed(false);
#ifdef Q_OS_WINDOWS
    a.setStyle("fusion");
#endif

    QTranslator ts;
    if(QLocale::system().name().split("_").at(0) == "zh")
    {
        ts.load("zh_CN.qm", ":/translations/translations");
        a.installTranslator(&ts);
    }

    MainWindow w;
    a.setActivationWindow(&w, false);
    QObject::connect(&a, SIGNAL(messageReceived(QString)), &w, SLOT(on_messageReceived(QString)));

    // w.show();

    return a.exec();
}

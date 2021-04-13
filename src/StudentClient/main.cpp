#include "mainwindow.h"
#include <QApplication>

#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false); // 存在托盘时，关闭窗口程序仍然运行
    a.setStyle("fusion");

    QTranslator ts;
    ts.load("zh_CN.qm", ":/translations/translations");

    if(QLocale::system().name().split("_").at(0) == "zh")
    {
        a.installTranslator(&ts);
    }

    MainWindow w;
    // w.show();

    return a.exec();
}

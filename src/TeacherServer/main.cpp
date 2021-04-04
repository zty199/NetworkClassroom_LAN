#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    // w.show();    // 启动时先显示启动界面，隐藏主界面

    return a.exec();
}

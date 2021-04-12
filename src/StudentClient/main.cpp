#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false); // 存在托盘时，关闭窗口程序仍然运行
    a.setStyle("fusion");

    MainWindow w;
    // w.show();

    return a.exec();
}

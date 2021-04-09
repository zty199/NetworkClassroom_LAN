#include "mainwindow.h"
#include <QApplication>

#include <QStyle>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setQuitOnLastWindowClosed(false); // 存在托盘时，关闭窗口程序仍然运行
    a.setStyle("fusion");

    /*
    QPalette darkPalette;
    //
    QColor darkColor(45, 45, 45);
    QColor disabledColor(127, 127, 127);
    QColor defaultTextColor(210, 210, 210);
    //
    darkPalette.setColor(QPalette::Window, darkColor);
    darkPalette.setColor(QPalette::Button, darkColor);
    darkPalette.setColor(QPalette::AlternateBase, darkColor);
    //
    darkPalette.setColor(QPalette::Text, defaultTextColor);
    darkPalette.setColor(QPalette::ButtonText, defaultTextColor);
    darkPalette.setColor(QPalette::WindowText, defaultTextColor);
    darkPalette.setColor(QPalette::ToolTipBase, defaultTextColor);
    darkPalette.setColor(QPalette::ToolTipText, defaultTextColor);
    //
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
    darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
    //
    darkPalette.setColor(QPalette::Base, QColor(18, 18, 18));
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    //
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(darkPalette);
    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
    */

    MainWindow w;
    // w.show();    // 启动时先显示启动界面，隐藏主界面

    return a.exec();
}

#ifndef TEXTCHATDIALOG_H
#define TEXTCHATDIALOG_H

#include <QDialog>

namespace Ui {
class TextChatDialog;
}

class TextChatDialog : public QDialog
{
    Q_OBJECT

public:
    TextChatDialog(QWidget *mainWindow, QWidget *parent = nullptr);
    ~TextChatDialog();

private:
    Ui::TextChatDialog *ui;

    QWidget *mainWindow;

    void initUI();
    void initConnections();

private slots:
    void on_pushButton_clicked();
    void on_textAppend(QString);

signals:
    void textSend(QString);

};

#endif // TEXTCHATDIALOG_H

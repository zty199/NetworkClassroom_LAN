#include "textchatdialog.h"
#include "ui_textchatdialog.h"

#include <QShortcut>

TextChatDialog::TextChatDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextChatDialog)
{
    ui->setupUi(this);

    initUI();
    initConnections();
}

TextChatDialog::~TextChatDialog()
{
    delete ui;
}

void TextChatDialog::initUI()
{
    this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    ui->pushButton->setFocus();
}

void TextChatDialog::initConnections()
{
    // 添加键盘快捷键
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Enter), this), SIGNAL(activated()), this, SLOT(on_pushButton_clicked()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Return), this), SIGNAL(activated()), this, SLOT(on_pushButton_clicked()));

    connect(this->parent(), SIGNAL(textAppend(QString)), this, SLOT(on_textAppend(QString)));
}

void TextChatDialog::on_pushButton_clicked()
{
    QString body = ui->textEdit->toPlainText();
    emit textSend(body);

    ui->textEdit->clear();
}

void TextChatDialog::on_textAppend(QString msg)
{
    ui->textBrowser->append(msg);
}

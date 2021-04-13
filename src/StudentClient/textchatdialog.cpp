#include "textchatdialog.h"
#include "ui_textchatdialog.h"

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
    this->setWindowFlag(Qt::WindowMinimizeButtonHint, true);
    ui->pushButton->setFocus();
}

void TextChatDialog::initConnections()
{
    connect(this->parent(), SIGNAL(textAppend(QString)), this, SLOT(on_textAppend(QString)));
}

void TextChatDialog::on_pushButton_clicked()
{
    if(ui->textEdit->toPlainText().isEmpty())
    {
        return;
    }

    QString body = ui->textEdit->toPlainText();
    emit textSend(body);

    ui->textEdit->clear();
}

void TextChatDialog::on_textAppend(QString msg)
{
    ui->textBrowser->append(msg);
}

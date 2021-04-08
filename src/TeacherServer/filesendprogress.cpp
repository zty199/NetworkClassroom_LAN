#include "filesendprogress.h"
#include "ui_filesendprogress.h"

FileSendProgress::FileSendProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileSendProgress)
{
    ui->setupUi(this);
}

FileSendProgress::~FileSendProgress()
{
    delete ui;
}

void FileSendProgress::setValue(int value)
{
    ui->progressBar->setValue(value);
}
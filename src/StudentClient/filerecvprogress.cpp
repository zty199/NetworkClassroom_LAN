#include "filerecvprogress.h"
#include "ui_filerecvprogress.h"

FileRecvProgress::FileRecvProgress(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileRecvProgress)
{
    ui->setupUi(this);

    this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    this->setWindowFlag(Qt::WindowMinimizeButtonHint, true);
}

FileRecvProgress::~FileRecvProgress()
{
    delete ui;
}

void FileRecvProgress::setValue(int value)
{
    ui->progressBar->setValue(value);
}

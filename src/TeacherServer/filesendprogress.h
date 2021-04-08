#ifndef FILESENDPROGRESS_H
#define FILESENDPROGRESS_H

#include <QDialog>

namespace Ui {
class FileSendProgress;
}

class FileSendProgress : public QDialog
{
    Q_OBJECT

public:
    explicit FileSendProgress(QWidget *parent = nullptr);
    ~FileSendProgress();

    void setValue(int value);

private:
    Ui::FileSendProgress *ui;

};

#endif // FILESENDPROGRESS_H

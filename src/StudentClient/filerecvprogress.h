#ifndef FILERECVPROGRESS_H
#define FILERECVPROGRESS_H

#include <QDialog>

namespace Ui {
class FileRecvProgress;
}

class FileRecvProgress : public QDialog
{
    Q_OBJECT

public:
    explicit FileRecvProgress(QWidget *parent = nullptr);
    ~FileRecvProgress();

    void setValue(int value);

private:
    Ui::FileRecvProgress *ui;

};

#endif // FILERECVPROGRESS_H

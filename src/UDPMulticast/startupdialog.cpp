#include "startupdialog.h"
#include "ui_startupdialog.h"

#include <QNetworkAddressEntry>

StartUpDialog::StartUpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartUpDialog),
    availableInterfaces(QNetworkInterface::allInterfaces()),
    flag_multicast(false)
{
    ui->setupUi(this);

    initUI();

    connect(this->parent(), SIGNAL(studentConnected(int)), this, SLOT(on_studentConnected(int)));
}

StartUpDialog::~StartUpDialog()
{
    delete ui;
}

void StartUpDialog::mousePressEvent(QMouseEvent *)
{
    setFocus();
}

void StartUpDialog::initUI()
{
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    ui->btn_start->setDisabled(true);

    for(int i = 0; i < availableInterfaces.size(); i++)
    {
        if((availableInterfaces.at(i).flags() & QNetworkInterface::IsRunning)
                && (availableInterfaces.at(i).flags() & QNetworkInterface::CanMulticast))
        {
            ui->cb_network->addItem(availableInterfaces.at(i).humanReadableName(), i);
        }
    }
}

void StartUpDialog::on_cb_network_currentIndexChanged(int index)
{
    m_interface = availableInterfaces.at(ui->cb_network->itemData(index).value<int>());
}

void StartUpDialog::on_btn_multicast_clicked()
{
    if(!flag_multicast)
    {
        ui->cb_network->setDisabled(true);
        ui->lineEdit->setDisabled(true);
        ui->btn_start->setEnabled(true);

        // 获取当前网卡 IP
        QList<QNetworkAddressEntry> list = m_interface.addressEntries();
        foreach(const QNetworkAddressEntry &entry, list)
        {
            if(entry.ip().protocol() != QAbstractSocket::IPv4Protocol)
            {
                continue;
            }
            else
            {
                m_address = entry.ip();
            }
        }

        if(ui->lineEdit->text().isEmpty())
        {
            ui->lineEdit->setText(m_address.toString());
        }
        else
        {
            ui->lb_welcome->setText("Welcome, " + ui->lineEdit->text() + "!");
        }

        flag_multicast = true;

        emit multicastReady(m_interface, m_address);
    }
    else
    {
        ui->cb_network->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->lineEdit->clear();
        ui->lb_welcome->setText("Welcome!");
        ui->btn_start->setDisabled(true);
        flag_multicast = false;

        int index = -1;
        QNetworkInterface curInterface;
        if(ui->cb_network->currentIndex() > index)
        {
            curInterface = availableInterfaces.at(ui->cb_network->currentData().value<int>());
        }
        ui->cb_network->clear();

        availableInterfaces = QNetworkInterface::allInterfaces();
        if(!availableInterfaces.isEmpty())
        {
            // 记录当前设备选项
            for(int i = 0; i < availableInterfaces.size(); i++)
            {
                if((availableInterfaces.at(i).flags() & QNetworkInterface::IsRunning)
                        && (availableInterfaces.at(i).flags() & QNetworkInterface::CanMulticast))
                {
                    ui->cb_network->addItem(availableInterfaces.at(i).humanReadableName(), i);
                    if(availableInterfaces.at(i).name() == curInterface.name())
                    {
                        index = i;
                    }
                }
            }

            if(index < 0)
            {
                index = 0;
            }

            ui->cb_network->setCurrentIndex(index);
        }
    }
}

void StartUpDialog::on_studentConnected(int num)
{
    if(num > 0)
    {
        ui->lb_connected->setText(QString("%1 student(s) connected...").arg(num));
    }
}

void StartUpDialog::on_btn_start_clicked()
{
    this->hide();
    emit startUp();
}

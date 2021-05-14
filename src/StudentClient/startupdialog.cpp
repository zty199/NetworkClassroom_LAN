#include "startupdialog.h"
#include "ui_startupdialog.h"

StartUpDialog::StartUpDialog(QWidget *mainWindow,
                             QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StartUpDialog),
    availableInterfaces(QNetworkInterface::allInterfaces()),
    flag_connect(false),
    mainWindow(mainWindow)
{
    ui->setupUi(this);

    this->setAttribute(Qt::WA_DeleteOnClose, true);

    initUI();
    initConnections();
}

StartUpDialog::~StartUpDialog()
{
    delete ui;
    QApplication::quit();
}

void StartUpDialog::mousePressEvent(QMouseEvent *)
{
    this->setFocus();
}

void StartUpDialog::initUI()
{
    this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    this->setWindowFlag(Qt::WindowMinimizeButtonHint, true);

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

void StartUpDialog::initConnections()
{
    connect(mainWindow, SIGNAL(teacherConnected()), this, SLOT(on_teacherConnected()));
}

void StartUpDialog::on_cb_network_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    m_interface = availableInterfaces.at(ui->cb_network->currentData().value<int>());
}

void StartUpDialog::on_btn_connect_clicked()
{
    if(!flag_connect)
    {
        ui->cb_network->setDisabled(true);
        ui->lineEdit->setDisabled(true);

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
        if(m_address.isNull())
        {
            // IP 为空则终止
            flag_connect = true;
            emit ui->btn_connect->clicked();
            return;
        }

        if(ui->lineEdit->text().isEmpty())
        {
            ui->lineEdit->setText(m_address.toString());
        }
        m_name = ui->lineEdit->text();
        ui->lb_welcome->setText(tr("Welcome, ") + m_name + tr("!"));

        flag_connect = true;

        emit connectReady(m_interface, m_address, m_name);
    }
    else
    {
        ui->cb_network->setEnabled(true);
        ui->lineEdit->setEnabled(true);
        ui->lineEdit->clear();
        ui->lb_welcome->setText(tr("Welcome!"));
        ui->lb_connect->setText(tr("Waiting for teacher..."));
        ui->btn_start->setDisabled(true);

        flag_connect = false;

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
                    if(availableInterfaces.at(i).humanReadableName() == curInterface.humanReadableName())
                    {
                        index = ui->cb_network->count() - 1;
                    }
                }
            }

            if(index < 0)
            {
                index = 0;
            }

            ui->cb_network->setCurrentIndex(index);
        }

        emit connectNotReady();
    }
}

void StartUpDialog::on_btn_start_clicked()
{
    this->hide();
    emit startUp();
}

void StartUpDialog::on_teacherConnected()
{
    ui->lb_connect->setText(tr("Teacher Connected!"));
    ui->btn_start->setEnabled(true);
}

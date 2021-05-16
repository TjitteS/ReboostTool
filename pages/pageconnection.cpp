/*
    Original copyright 2018 Benjamin Vedder benjamin@vedder.se and the VESC Tool project ( https://github.com/vedderb/vesc_tool )
    Now forked to:
    Danny Bokma github@diebie.nl

    This file is part of BMS Tool.

    DieBieMS Tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    DieBieMS Tool is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pageconnection.h"
#include "ui_pageconnection.h"
#include "widgets/helpdialog.h"
#include "utility.h"
#include <QMessageBox>
#include <QSettings>

PageConnection::PageConnection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageConnection)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);

    //QString lastTcpServer = QSettings().value("tcp_server", ui->tcpServerEdit->text()).toString();
    //ui->tcpServerEdit->setText(lastTcpServer);

    //int lastTcpPort =QSettings().value("tcp_port", ui->tcpPortBox->value()).toInt();
    //ui->tcpPortBox->setValue(lastTcpPort);

    mMPPT = 0;
    mTimer = new QTimer(this);

    connect(mTimer, SIGNAL(timeout()),
            this, SLOT(timerSlot()));

    mTimer->start(20);
}

PageConnection::~PageConnection()
{
    delete ui;
}

MPPTInterface *PageConnection::bms() const
{
    return mMPPT;
}

void PageConnection::setMPPT(MPPTInterface *dieBieMS)
{
    mMPPT = dieBieMS;

    connect(mMPPT->bleDevice(), SIGNAL(scanDone(QVariantMap,bool)),this, SLOT(bleScanDone(QVariantMap,bool)));

    QString lastBleAddr = QSettings().value("ble_addr").toString();
    if (lastBleAddr != "") {
        QString setName = mMPPT->getBleName(lastBleAddr);

        QString name;
        if (!setName.isEmpty()) {
            name += setName;
            name += " [";
            name += lastBleAddr;
            name += "]";
        } else {
            name = lastBleAddr;
        }
        //ui->bleDevBox->insertItem(0, name, lastBleAddr);
    }

    on_serialRefreshButton_clicked();
}

void PageConnection::timerSlot()
{
    if (mMPPT) {
        QString str = mMPPT->getConnectedPortName();
        if (str != ui->statusLabel->text()) {
            ui->statusLabel->setText(mMPPT->getConnectedPortName());
        }

        // CAN fwd
        if (ui->canFwdButton->isChecked() != mMPPT->commands()->getSendCan()) {
            ui->canFwdButton->setChecked(mMPPT->commands()->getSendCan());
        }

        if (ui->canFwdBox->value() != mMPPT->commands()->getCanSendId()) {
            ui->canFwdBox->setValue(mMPPT->commands()->getCanSendId());;
        }
    }
}

void PageConnection::on_serialRefreshButton_clicked()
{
    if (mMPPT) {
        ui->serialPortBox->clear();
        QList<VSerialInfo_t> ports = mMPPT->listSerialPorts();
        foreach(const VSerialInfo_t &port, ports) {
            ui->serialPortBox->addItem(port.name, port.systemPath);
        }
        ui->serialPortBox->setCurrentIndex(0);
    }
}

void PageConnection::on_serialDisconnectButton_clicked()
{
    if (mMPPT) {
        mMPPT->disconnectPort();
    }
}

void PageConnection::on_serialConnectButton_clicked()
{
    if (mMPPT) {
        mMPPT->connectSerial(ui->serialPortBox->currentData().toString(),
                             ui->serialBaudBox->value());
    }
}





void PageConnection::on_canFwdBox_valueChanged(int arg1)
{
    if (mMPPT) {
        mMPPT->commands()->setCanSendId(arg1);
    }
}

void PageConnection::on_helpButton_clicked()
{
    if (mMPPT) {
        HelpDialog::showHelp(this, mMPPT->infoConfig(), "help_can_forward");
    }
}

void PageConnection::on_canFwdButton_toggled(bool checked)
{
    if (mMPPT) {
        mMPPT->commands()->setSendCan(checked);
    }
}

void PageConnection::on_autoConnectButton_clicked()
{
    Utility::autoconnectBlockingWithProgress(mMPPT, this);
}



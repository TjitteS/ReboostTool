/*
    Original copyright 2018 Benjamin Vedder benjamin@vedder.se and the VESC Tool project ( https://github.com/vedderb/vesc_tool )
    Now forked to:
    Tjitte@tpee.nl

    This file is part of Reboost Tool.

    Reboost Tool is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Reboost Tool. is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "pageslavegeneral.h"
#include "ui_pageslavegeneral.h"

PageSlaveGeneral::PageSlaveGeneral(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageSlaveGeneral)
{
    ui->setupUi(this);
    mMPPT = 0;
}

PageSlaveGeneral::~PageSlaveGeneral()
{
    delete ui;
}

MPPTInterface *PageSlaveGeneral::bms() const {
    return mMPPT;
}

void PageSlaveGeneral::setMPPT(MPPTInterface *dieBieMS) {
    mMPPT = dieBieMS;

    if (mMPPT) {
        ui->slaveSensorsTab->addRowSeparator(tr("Shunt"));
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "shuntHCFactor");
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "shuntHCOffset");
        ui->slaveSensorsTab->addRowSeparator(tr("NTC specifications"));
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "NTCHiAmpPCBTopResistor");
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "NTCHiAmpPCB25Deg");
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "NTCHiAmpPCBBeta");
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "NTCHiAmpExtTopResistor");
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "NTCHiAmpExt25Deg");
        ui->slaveSensorsTab->addParamRow(mMPPT->mpptConfig(), "NTCHiAmpExtBeta");
    }
}

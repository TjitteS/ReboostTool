#include "pagecansettings.h"
#include "ui_pagecansettings.h"

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

PageCanSettings::PageCanSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageCanSettings)
{
    ui->setupUi(this);
}

PageCanSettings::~PageCanSettings()
{
    delete ui;
}

void PageCanSettings::SetMPPT(MPPTInterface *mppt){
    if(mppt){
          mMPPT = mppt;
          ui->CanSettings->addRowSeparator("CAN-Interface Settings");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"can_baudrate");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"can_samplepoint");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"can_generalCanId");

          ui->CanSettings->addRowSeparator("Soft Limmits");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_OutputVoltageLimitSoft");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_InputVoltageLimitSoft");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_OutputCurrentLimitSoft");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_InputCurrentLimitSoft");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_TemperatureLimitStart");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_TemperatureLimitEnd");


          ui->CanSettings->addRowSeparator("Tracking settings");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_meterFilterCoeficient");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_PO_Stepsize");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_PO_Timestep");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_PO_maxgradient");

          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_jump_PowerThreshold");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_jump_Rate");

          ui->CanSettings->addRowSeparator("Other settings");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_PhaseHighSideEnableCurrent");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_PhaseCurrentMin");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_outputEnable");

    }
}

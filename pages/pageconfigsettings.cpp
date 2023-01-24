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
#include "pageconfigsettings.h"
#include "ui_pageconfigsettings.h"

PageconfigSettings::PageconfigSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageconfigSettings)
{
    ui->setupUi(this);
}

PageconfigSettings::~PageconfigSettings()
{
    delete ui;

}

void PageconfigSettings::SetMPPT(MPPTInterface *mppt){

    if(mppt){
       mMPPT = mppt;
       ui->Settings->addRowSeparator("Controller");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_SwitchingFrequency");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_ControllerFrequency");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_DeadTime");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_L");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_Cin");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_Cout");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_RLint");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_Q");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_Klim");

       ui->Settings->addRowSeparator("Hard Litmits");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_OutputVoltageLimitHard");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_InputVoltageLimitHard");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_OutputCurrentLimitHard");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_InputCurrentLimitHard");

       //ui->Settings->addRowSeparator("Other settings");
       //ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_PhaseHighSideEnableCurrent");
      // ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_PhaseCurrentMin");
       //ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_outputEnable");


      }
}

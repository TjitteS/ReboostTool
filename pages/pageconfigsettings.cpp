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
       ui->Settings->addRowSeparator("Limmits");

       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_OutputVoltageLimitSoft");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_OutputVoltageLimitHard");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_InputVoltageLimitSoft");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_InputVoltageLimitHard");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_OutputCurrentLimitSoft");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_OutputCurrentLimitHard");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_InputCurrentLimitSoft");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_InputCurrentLimitHard");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_PhaseHighSideEnableCurrent");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_PhaseCurrentMin");
       ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_outputEnable");


       //ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_StringVmppReset");
       //ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_StringVoc");
       //ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_UVLO");
       //ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_PO_Stepsize");
       //ui->Settings->addParamRow(mMPPT->mpptConfig(),"settings_PO_Period");
      }
}
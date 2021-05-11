#include "pagecansettings.h"
#include "ui_pagecansettings.h"

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

          ui->CanSettings->addRowSeparator("Tracking settings");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"settings_meterFilterCoeficient");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_PO_Stepsize");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_PO_Timestep");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_PO_maxgradient");

          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_jump_PowerThreshold");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_jump_rangeMax");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_jump_rangeMin");
          ui->CanSettings->addParamRow(mMPPT->mpptConfig(),"mppt_jump_Rate");

    }
}

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
    }
}

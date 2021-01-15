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
      }
}

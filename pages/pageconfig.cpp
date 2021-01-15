#include "pageconfig.h"
#include "ui_pageconfig.h"

PageConfig::PageConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageConfig)
{
    ui->setupUi(this);
    layout()->setContentsMargins(0, 0, 0, 0);
    mMPPT = 0;
}

PageConfig::~PageConfig()
{
    delete ui;
}

MPPTInterface *PageConfig::mppt() const
{
    return mMPPT;
}

void PageConfig::setMPPT(MPPTInterface *mppt){
    mMPPT = mppt;

    if (mMPPT) {
        ConfigParam *p = mMPPT->infoConfig()->getParam("config_description");
        if (p != 0) {
            ui->textEdit->setHtml(p->description);
        } else {
            ui->textEdit->setText("Configuraiton Description not found.");
        }
    }
}

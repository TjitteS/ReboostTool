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

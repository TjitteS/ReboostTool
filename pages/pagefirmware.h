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
#ifndef PAGEFIRMWARE_H
#define PAGEFIRMWARE_H

#include <QWidget>
#include "mpptinterface.h"

namespace Ui {
class PageFirmware;
}

class PageFirmware : public QWidget
{
    Q_OBJECT

public:
    explicit PageFirmware(QWidget *parent = 0);
    ~PageFirmware();

    MPPTInterface *bms() const;
    void setMPPT(MPPTInterface *dieBieMS);

private slots:
    void fwUploadStatus(const QString &status, double progress, bool isOngoing);
    void fwVersionReceived(int major, int minor, QString hw, QByteArray uuid);
    void updateHwList(QString hw = "");
    void updateFwList();
    void updateBlList(QString hw = "");

    void on_chooseButton_clicked();
    void on_uploadButton_clicked();
    void on_readVersionButton_clicked();
    void on_cancelButton_clicked();
    void on_changelogButton_clicked();

private:
    Ui::PageFirmware *ui;
    MPPTInterface *mMPPT;

};

#endif // PAGEFIRMWARE_H

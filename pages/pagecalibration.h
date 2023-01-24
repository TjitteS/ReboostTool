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
#ifndef PAGECALIBRATION_H
#define PAGECALIBRATION_H

#include <QWidget>
#include "mpptinterface.h"
#include "configparams.h"

namespace Ui {
class PageCalibration;
}

class PageCalibration : public QWidget
{
    Q_OBJECT

public:
    explicit PageCalibration(QWidget *parent = nullptr);
    ~PageCalibration();

    MPPTInterface *mppt() const;
    void setMPPT(MPPTInterface *mppt);


private:
    Ui::PageCalibration *ui;
    MPPTInterface *mMPPT;
    MPPT_VALUES mMPPTdata;
    QVector<double> voltageRef;
    QVector<double> voltageVinData;
    QVector<double> voltageVoutData;
    QVector<double> currentRef;
    QVector<double> currentIinData;
    QVector<double> currentIoutData;

    double regresionGetOffset(QVector<double> ref, QVector<double> data, double Gain);
    double regresionGetGain(QVector<double> ref, QVector<double> data);

private slots:
    void currentAddDatapoint();
    void currentCalc();
    void currentDataDelete();
    void voltageAddDatapoint();
    void voltageCalc();
    void voltageDataDelete();
    void updateValues(MPPT_VALUES v);
};

#endif // PAGECALIBRATION_H

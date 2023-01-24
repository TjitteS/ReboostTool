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
#ifndef PAGESWEEP_H
#define PAGESWEEP_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include "mpptinterface.h"

namespace Ui {
class PageSweep;
}

class PageSweep : public QWidget
{
    Q_OBJECT

public:
    explicit PageSweep(QWidget *parent = nullptr);
    ~PageSweep();

    MPPTInterface *getmppt() const;
    void setMPPT(MPPTInterface *mppt);

private slots:
    void timerSlot();
    void valuesReceived(MPPT_VALUES values);
    void sweepDataReceifed(QVector<double> vs, QVector<double> is);
    void sweepButtonClicked();
    void saveSweep();

private:
    Ui::PageSweep *ui;
    MPPTInterface *mMPPT;

    QTimer *mTimer;

    QVector<double> mvs;
    QVector<double> mis;
    QVector<double> mps;
    QVector<double> vpath;
    QVector<double> ipath;
    QVector<double> ppath;

    bool mUpdateValPlot;
};

#endif // PAGESWEEP_H

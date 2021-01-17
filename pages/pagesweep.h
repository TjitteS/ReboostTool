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

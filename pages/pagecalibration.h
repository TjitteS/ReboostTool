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

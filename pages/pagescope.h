#ifndef PAGESCOPE_H
#define PAGESCOPE_H

#include <QWidget>
#include <QVector>
#include <QTimer>
#include "mpptinterface.h"

#define SCOPE_MAX_SAMPLES 4096

namespace Ui {
    class PageScope;
}

typedef enum{
    SourceIndex_Iind,
    SourceIndex_Ihigh,
    SourceIndex_Vlow,
    SourceIndex_Vhigh,
    SourceIndex_Ilow,
    SourceIndex_Power,
    SourceIndex_Eff,
    SourceIndex_Iind_Filtered,
    SourceIndex_Ihigh_Filtered,
    SourceIndex_Vlow_Filtered,
    SourceIndex_Vhigh_Filtered,
    SourceIndex_Ilow_Filtered,
    SourceIndex_Power_Filtered,
    SourceIndex_Eff_Filtered,
}ConverterScopeSourceIndex_t;

typedef enum{
    Unit_Volt,
    Unit_Current,
    Unit_Power,
    Unit_Eff,
}Unit_t;

class PageScope : public QWidget
{
    Q_OBJECT
public:
    explicit PageScope(QWidget *parent = nullptr);
    ~PageScope();

    MPPTInterface *getmppt() const;
    void setMPPT(MPPTInterface *mppt);

private slots:
    void RunButtonClicked();
    void StepButtonClicked();
    void SaveButtonCLicked();
    void ScopeDataReceived(int channel, int index, QVector<double> vs);
    void ScopeSetSamplerate(float sr);
    void timerSlot();
    void ScopeDataReady();

private:
    Ui::PageScope *ui;
    MPPTInterface *mMPPT;

    QTimer *mTimer;
    ConverterScopeSourceIndex_t ch1Source;
    ConverterScopeSourceIndex_t ch2Source;
    Unit_t ch1Unit;
    Unit_t ch2Unit;
    float samplerate;
    int samples;
    int pretrigger;
    int divider;
    bool onFault;

    bool getNewData;

    QVector<double> time;
    QVector<double> ch1;
    QVector<double> ch2;

    Unit_t GetUnit(ConverterScopeSourceIndex_t source);
    void UpdateAxis();
    void UpdateData();


};



#endif // PAGESCOPE_H

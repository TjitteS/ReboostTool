#include "pagecalibration.h"
#include "ui_pagecalibration.h"

PageCalibration::PageCalibration(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageCalibration)
{

    ui->setupUi(this);
    connect(ui->currentaddData,SIGNAL(clicked()),this,SLOT(currentAddDatapoint()));
    connect(ui->currentCalc,SIGNAL(clicked()),this,SLOT(currentCalc()));
    connect(ui->currentDeleteDataPoint,SIGNAL(clicked()),this,SLOT(currentDataDelete()));
    connect(ui->voltageAddData,SIGNAL(clicked()),this,SLOT(voltageAddDatapoint()));
    connect(ui->voltageCalc,SIGNAL(clicked()),this,SLOT(voltageCalc()));
    connect(ui->voltageDeleteData,SIGNAL(clicked()),this,SLOT(voltageDataDelete()));

}

PageCalibration::~PageCalibration()
{
    delete ui;
}

void PageCalibration::updateValues(MPPT_VALUES v){
    mMPPTdata = v;
}

void PageCalibration::setMPPT(MPPTInterface *mppt){
    mMPPT = mppt;
    if(mMPPT){
        connect(mMPPT->commands(),SIGNAL(valuesReceived(MPPT_VALUES)),this,SLOT(updateValues(MPPT_VALUES)));

        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_InputCurrentGain");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_InputCurrentOffset");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_OutputCurrentGain");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_OutputCurrentOffset");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_InputVoltageGain");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_InputVoltageOffset");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_OutputVoltageGain");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_OutputVoltageOffset");
        ui->calibraitonTable->addParamRow(mMPPT->mpptConfig(),"cal_Calibrated");

    }
}

void PageCalibration::currentAddDatapoint(){
    currentRef.append(ui->currentRefValue->value());
    currentIinData.append(mMPPTdata.Iin);
    currentIoutData.append(mMPPTdata.Iout);

    if(ui->currentAutoIncrement->isChecked()){
        double c = ui->currentRefValue->value();
        c += ui->currentAutoIncrementValue->value();
        ui->currentRefValue->setValue(c);
    }
    QString t = "";
    for(int i = 0; i < currentRef.size(); i++){
        QString text;
        text.sprintf("%05.2f\t%05.2f\t%05.2f\r\n", currentRef[i], currentIinData[i],currentIoutData[i]);
        t += text;
    }

    ui->currentDataText->setText(t);

}
void PageCalibration::currentCalc(){
    QVector<double> IinRawdata;
    double inah = mMPPT->mpptConfig()->getParam("cal_InputCurrentOffset")->valDouble/1000;
    double inbh = mMPPT->mpptConfig()->getParam("cal_InputCurrentGain")->valDouble;
    QVector<double> IoutRawdata;
    double outah = mMPPT->mpptConfig()->getParam("cal_InputCurrentOffset")->valDouble/1000;
    double outbh = mMPPT->mpptConfig()->getParam("cal_InputCurrentGain")->valDouble;

    for(int i = 0; i < currentRef.size();i++){
        IinRawdata.append((currentIinData[i]-inah)/inbh);
        IoutRawdata.append((currentIoutData[i]-outah)/outbh);

    }

    double IinGain   = regresionGetGain(currentRef,IinRawdata);
    double IinOffset = regresionGetOffset(currentRef,IinRawdata,IinGain);
    double IoutGain   = regresionGetGain(currentRef,IoutRawdata);
    double IoutOffset = regresionGetOffset(currentRef,IoutRawdata,IoutGain);

    mMPPT->mpptConfig()->updateParamDouble("cal_InputCurrentOffset", IinOffset *1000);
    mMPPT->mpptConfig()->updateParamDouble("cal_InputCurrentGain",IinGain);
    mMPPT->mpptConfig()->updateParamDouble("cal_OutputCurrentOffset", IoutOffset *1000);
    mMPPT->mpptConfig()->updateParamDouble("cal_OutputCurrentGain",IoutGain);
}
void PageCalibration::voltageDataDelete(){
    voltageRef.clear();
    voltageVinData.clear();
    voltageVoutData.clear();
    ui->voltageDataText->setText("Cleared");
}
void PageCalibration::currentDataDelete(){
    currentRef.clear();
    currentIinData.clear();
    currentIoutData.clear();
    ui->currentDataText->setText("Cleared");
}
void PageCalibration::voltageAddDatapoint(){
    voltageRef.append(ui->voltRefValue->value());
    voltageVinData.append(mMPPTdata.Vin);
    voltageVoutData.append(mMPPTdata.Vout);

    if(ui->voltautoIncrement->isChecked()){
        double c = ui->voltRefValue->value();
        c += ui->voltautoIncrementValue->value();
        ui->voltRefValue->setValue(c);
    }
    QString t = "";
    for(int i = 0; i < voltageRef.size(); i++){
        QString text;
        text.sprintf("%05.2f\t%05.2f\t%05.2f\r\n", voltageRef[i], voltageVinData[i],voltageVoutData[i]);
        t += text;
    }

    ui->voltageDataText->setText(t);
}
void PageCalibration::voltageCalc(){
    QVector<double> VinRawdata;
    double inah = mMPPT->mpptConfig()->getParam("cal_InputVoltageOffset")->valDouble/1000;
    double inbh = mMPPT->mpptConfig()->getParam("cal_InputVoltageGain")->valDouble;
    QVector<double> VoutRawdata;
    double outah = mMPPT->mpptConfig()->getParam("cal_InputVoltageOffset")->valDouble/1000;
    double outbh = mMPPT->mpptConfig()->getParam("cal_InputVoltageGain")->valDouble;

    for(int i = 0; i < voltageRef.size();i++){
        VinRawdata.append((voltageVinData[i]-inah)/inbh);
        VoutRawdata.append((voltageVoutData[i]-outah)/outbh);

    }

    double VinGain   = regresionGetGain(voltageRef,VinRawdata);
    double VinOffset = regresionGetOffset(voltageRef,VinRawdata,VinGain);
    double VoutGain   = regresionGetGain(voltageRef,VoutRawdata);
    double VoutOffset = regresionGetOffset(voltageRef,VoutRawdata,VoutGain);

    mMPPT->mpptConfig()->updateParamDouble("cal_InputVoltageOffset", VinOffset *1000);
    mMPPT->mpptConfig()->updateParamDouble("cal_InputVoltageGain",VinGain);
    mMPPT->mpptConfig()->updateParamDouble("cal_OutputVoltageOffset", VoutOffset *1000);
    mMPPT->mpptConfig()->updateParamDouble("cal_OutputVoltageGain",VoutGain);
}


double PageCalibration::regresionGetGain(QVector<double> ref, QVector<double> data){
    double xmean = 0;
    double ymean = 0;

    int size = ref.size();
    for (int i = 0; i < size; i++){
        xmean += data[i];
        ymean += ref[i];
    }

    xmean /= size;
    ymean /= size;

    double SSxx = 0;
    double SSxy = 0;
    for (int i = 0; i < size; i++){
        SSxy += (data[i]-xmean)*(ref[i]-ymean);
        SSxx += (data[i]-xmean)*(data[i]-xmean);
    }
    return SSxy / SSxx;
}

double PageCalibration::regresionGetOffset(QVector<double> ref, QVector<double> data, double Gain){
    double xmean = 0;
    double ymean = 0;

    int size = ref.size();
    for (int i = 0; i < size; i++){
        xmean += data[i];
        ymean += ref[i];
    }
    xmean /= size;
    ymean /= size;

    return ymean - (Gain*xmean);
}


#include "pagescope.h"
#include "ui_pagescope.h"

PageScope::PageScope(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageScope)
{
    //Setup UI
    ui->setupUi(this);
    connect(ui->Run, SIGNAL(clicked()), this, SLOT(RunButtonClicked()));
    connect(ui->Step, SIGNAL(clicked()), this, SLOT(StepButtonClicked()));
    //connect(ui->Step, SIGNAL(clicked()), this, SLOT(SaveButtonCLicked()));
    mMPPT = 0;


    ui->Ch1source->addItem("Inductor Current", SourceIndex_Iind);
    ui->Ch1source->addItem("High-side Current",SourceIndex_Ihigh);
    ui->Ch1source->addItem("Low-side Voltage", SourceIndex_Vlow);
    ui->Ch1source->addItem("High-side Voltage",SourceIndex_Vhigh);
    ui->Ch1source->addItem("Low-side Current", SourceIndex_Ilow);
    ui->Ch1source->addItem("Power",            SourceIndex_Power);
    ui->Ch1source->addItem("Efficiency",       SourceIndex_Eff);
    ui->Ch1source->addItem("Inductor Current(F)", SourceIndex_Iind_Filtered);
    ui->Ch1source->addItem("High-side Current(F)",SourceIndex_Ihigh_Filtered);
    ui->Ch1source->addItem("Low-side Voltage(F)", SourceIndex_Vlow_Filtered);
    ui->Ch1source->addItem("High-side Voltage(F)",SourceIndex_Vhigh_Filtered);
    ui->Ch1source->addItem("Low-side Current(F)", SourceIndex_Ilow_Filtered);
    ui->Ch1source->addItem("Power(F)",            SourceIndex_Power_Filtered);
    ui->Ch1source->addItem("Efficiency(F)",       SourceIndex_Eff_Filtered);

    ui->Ch2source->addItem("Inductor Current", SourceIndex_Iind);
    ui->Ch2source->addItem("High-side Current",SourceIndex_Ihigh);
    ui->Ch2source->addItem("Low-side Voltage", SourceIndex_Vlow);
    ui->Ch2source->addItem("High-side Voltage",SourceIndex_Vhigh);
    ui->Ch2source->addItem("Low-side Current", SourceIndex_Ilow);
    ui->Ch2source->addItem("Power",            SourceIndex_Power);
    ui->Ch2source->addItem("Efficiency",       SourceIndex_Eff);
    ui->Ch2source->addItem("Inductor Current(F)", SourceIndex_Iind_Filtered);
    ui->Ch2source->addItem("High-side Current(F)",SourceIndex_Ihigh_Filtered);
    ui->Ch2source->addItem("Low-side Voltage(F)", SourceIndex_Vlow_Filtered);
    ui->Ch2source->addItem("High-side Voltage(F)",SourceIndex_Vhigh_Filtered);
    ui->Ch2source->addItem("Low-side Current(F)", SourceIndex_Ilow_Filtered);
    ui->Ch2source->addItem("Power(F)",            SourceIndex_Power_Filtered);
    ui->Ch2source->addItem("Efficiency(F)",       SourceIndex_Eff_Filtered);

    //Initialise Scope Plot
    ui->ScopePlot->addGraph();
    ui->ScopePlot->addGraph();
    ui->ScopePlot->graph(0)->setPen(QPen(Qt::green));
    ui->ScopePlot->graph(1)->setPen(QPen(Qt::red));
    ui->ScopePlot->graph(0)->setName("Channel 1");
    ui->ScopePlot->graph(1)->setName("Channel 2");
    ui->ScopePlot->xAxis->setLabel("Time [ms]");
    ui->ScopePlot->legend->setVisible(true);
    UpdateAxis();


    mTimer = new QTimer(this);
    mTimer->start(100);
    connect(mTimer, SIGNAL(timeout()),this,SLOT(timerSlot()));

}

PageScope::~PageScope()
{
    delete ui;
}

void PageScope::timerSlot(){
    if(getNewData){
        getNewData = false;
        mMPPT->commands()->ScopeGetData();
    }
}

MPPTInterface * PageScope::getmppt() const{
    return mMPPT;
}
void PageScope::setMPPT(MPPTInterface *mppt){

    if(mppt){

        mMPPT = mppt;
        connect(mMPPT->commands(), SIGNAL(scopeDataReceived(int, int, QVector<double>)),this, SLOT(ScopeDataReceived(int, int, QVector<double>)));
        connect(mMPPT->commands(), SIGNAL(scopeSampleRateReceived(float)),this, SLOT(ScopeSetSamplerate(float)));

    }
}
Unit_t PageScope::GetUnit(ConverterScopeSourceIndex_t source){
    switch(source){
    case SourceIndex_Iind:
        return Unit_Current;
        break;
    case SourceIndex_Ihigh:
        return  Unit_Current;
        break;
    case SourceIndex_Vlow:
        return  Unit_Volt;
        break;
    case SourceIndex_Vhigh:
        return  Unit_Volt;
        break;
    case SourceIndex_Ilow:
        return  Unit_Current;
        break;
    case SourceIndex_Power:
        return Unit_Power;
        break;
    case SourceIndex_Eff:
        return  Unit_Eff;
        break;
    case SourceIndex_Iind_Filtered:
        return Unit_Current;
        break;
    case SourceIndex_Ihigh_Filtered:
        return Unit_Current;
        break;
    case SourceIndex_Vlow_Filtered:
        return Unit_Volt;
        break;
    case SourceIndex_Vhigh_Filtered:
        return Unit_Volt;
        break;
    case SourceIndex_Ilow_Filtered:
        return Unit_Current;
        break;
    case SourceIndex_Power_Filtered:
        return Unit_Power;
        break;
    case SourceIndex_Eff_Filtered:
        return Unit_Eff;
        break;
    }
}

void PageScope::UpdateAxis(){
    ch1Source = static_cast<ConverterScopeSourceIndex_t>(ui->Ch1source->currentIndex());
    ch2Source = static_cast<ConverterScopeSourceIndex_t>(ui->Ch2source->currentIndex());
    samples = ui->samples->value();
    pretrigger = ui->pretrigger->value()*samples/100;
    divider = ui->divider->value();

    time = QVector<double>(samples);
    ch1 = QVector<double>(samples);
    ch2 = QVector<double>(samples);

    printf("Unit = 1");

    if(samples > SCOPE_MAX_SAMPLES){
        samples = SCOPE_MAX_SAMPLES;
    }

    ch1Unit = GetUnit(ch1Source);
    ch2Unit = GetUnit(ch2Source);

    switch(ch1Unit){
    case Unit_Volt:
        ui->ScopePlot->yAxis->setLabel("Voltage [V]");
        break;
    case Unit_Current:
        ui->ScopePlot->yAxis->setLabel("Current [A]");
        break;

    case Unit_Power:
        ui->ScopePlot->yAxis->setLabel("Power [W]");
        break;

    case Unit_Eff:
        ui->ScopePlot->yAxis->setLabel("Efficiency [%]");
        break;
    }

    if(ch1Unit == ch2Unit){
        ui->ScopePlot->graph(1)->setValueAxis(ui->ScopePlot->yAxis);
        ui->ScopePlot->yAxis2->setVisible(false);
    }else{
        ui->ScopePlot->graph(1)->setValueAxis(ui->ScopePlot->yAxis2);
        ui->ScopePlot->yAxis2->setVisible(true);

        switch(ch2Unit){
        case Unit_Volt:
            ui->ScopePlot->yAxis2->setLabel("Voltage [V]");
            break;
        case Unit_Current:
            ui->ScopePlot->yAxis2->setLabel("Current [A]");
            break;

        case Unit_Power:
            ui->ScopePlot->yAxis2->setLabel("Power [W]");
            break;

        case Unit_Eff:
            ui->ScopePlot->yAxis2->setLabel("Efficiency [%]");
            break;
        }
    }
    ui->ScopePlot->replot();
}

void PageScope::UpdateData(){
    ui->ScopePlot->graph(0)->setData(time, ch1);
    ui->ScopePlot->graph(1)->setData(time, ch2);
    ui->ScopePlot->rescaleAxes();
    ui->ScopePlot->replot();
}

void PageScope::RunButtonClicked(){
    UpdateAxis();
    if(mMPPT){
        mMPPT->commands()->ScopeRun(samples, pretrigger, divider, ch1Source, ch2Source);
        mTimer->start(2000);
        getNewData = true;
    }
}

void PageScope::StepButtonClicked(){
    UpdateAxis();
    if(mMPPT){
        mMPPT->commands()->ScopeCurrentStep(samples, pretrigger, divider, ch1Source, ch2Source, ui->I0->value(), ui->I1->value());
        mTimer->start(2000);
        getNewData = true;
    }
}

void PageScope::SaveButtonCLicked(){

}
void PageScope::ScopeDataReceived(int channel, int index, QVector<double> values){
    qDebug( "Data Received." );

    if (channel == 0){
        for(int i = 0; i < values.length(); i++){
            ch1[i+index] = values[i];
        }
    }
    if (channel == 1){
        for(int i = 0; i < values.length(); i++){
            ch2[i+index] = values[i];
        }
    }

    UpdateData();
}
void PageScope::ScopeSetSamplerate(float sr){
    qDebug("Sampe rate received.");

    samplerate = sr;
    for(int i = 0; i  <samples; i++){
        time[i] = 1000/sr*(i-pretrigger);

    }

}

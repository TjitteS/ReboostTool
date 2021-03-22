#include "pagesweep.h"
#include "ui_pagesweep.h"

PageSweep::PageSweep(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageSweep)
{
    ui->setupUi(this);
    connect(ui->Sweep,SIGNAL(clicked()),this,SLOT(sweepButtonClicked()));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(saveSweep()));

    mMPPT = 0;

    mTimer = new QTimer(this);
    mTimer->start(20);

    mUpdateValPlot = false;

    ui->sweepPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    int graphindex = 0;

    ui->sweepPlot->addGraph();
    ui->sweepPlot->graph(graphindex)->setPen(QPen(Qt::black));
    ui->sweepPlot->graph(graphindex)->setName("I-V");
    graphindex++;

    ui->sweepPlot->addGraph(ui->sweepPlot->xAxis,ui->sweepPlot->yAxis2);
    ui->sweepPlot->graph(graphindex)->setPen(QPen(Qt::red));
    ui->sweepPlot->graph(graphindex)->setName("P-V");
    graphindex++;

    ui->sweepPlot->addGraph();
    ui->sweepPlot->graph(graphindex)->setPen(QPen(Qt::green));
    ui->sweepPlot->graph(graphindex)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
    ui->sweepPlot->graph(graphindex)->setLineStyle(QCPGraph::lsNone);
    ui->sweepPlot->graph(graphindex)->setName("Path");
    graphindex++;

    ui->sweepPlot->addGraph(ui->sweepPlot->xAxis,ui->sweepPlot->yAxis2);
    ui->sweepPlot->graph(graphindex)->setPen(QPen(Qt::green));
    ui->sweepPlot->graph(graphindex)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc));
    ui->sweepPlot->graph(graphindex)->setLineStyle(QCPGraph::lsNone);
    ui->sweepPlot->graph(graphindex)->setName("Path");
    graphindex++;

    vpath.append(0);
    ipath.append(0);
    ppath.append(0);


    //ui->sweepPlot->legend->setVisible(true);
    //ui->sweepPlot->legend->setFont(legendFont);
    ui->sweepPlot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignRight|Qt::AlignBottom);
    ui->sweepPlot->legend->setBrush(QBrush(QColor(255,255,255,230)));
    ui->sweepPlot->xAxis->setLabel("Voltage (V)");
    ui->sweepPlot->yAxis->setLabel("Current (A)");
    ui->sweepPlot->yAxis2->setLabel("Power (W)");
    ui->sweepPlot->yAxis->setRange(0, 60);
    ui->sweepPlot->yAxis2->setVisible(true);

    connect(mTimer, SIGNAL(timeout()),this,SLOT(timerSlot()));
}

PageSweep::~PageSweep()
{
    delete ui;
}
MPPTInterface * PageSweep::getmppt() const{
    return mMPPT;
}
void PageSweep::setMPPT(MPPTInterface *mppt){

    if(mppt){

        mMPPT = mppt;
        connect(mMPPT->commands(), SIGNAL(valuesReceived(MPPT_VALUES)),this, SLOT(valuesReceived(MPPT_VALUES)));
        connect(mMPPT->commands(), SIGNAL(sweepDataReceifed(QVector<double>, QVector<double>)),this, SLOT(sweepDataReceifed(QVector<double>, QVector<double>)));

    }
}

void PageSweep::timerSlot(){
    if(mUpdateValPlot){
        int index = 0;
        ui->sweepPlot->graph(index++)->setData(mvs,mis);
        ui->sweepPlot->graph(index++)->setData(mvs,mps);

        ui->sweepPlot->graph(index++)->setData(vpath,ipath);
        ui->sweepPlot->graph(index++)->setData(vpath,ppath);


        //ui->sweepPlot->rescaleAxes();

        ui->sweepPlot->replot();
        mUpdateValPlot = false;
    }
}

void PageSweep::valuesReceived(MPPT_VALUES values){
    vpath[0] = values.Vin;
    ipath[0] = values.Ipv;
    ppath[0] = values.Power;
    mUpdateValPlot = true;
}

void PageSweep::sweepDataReceifed(QVector<double> vs, QVector<double> is){
    mis = is;
    mvs = vs;

    double maxv = 0;
    double maxi = 0;
    double maxp = 0;
    double minv = 1e6;
    double mini = 1e6;
    double minp = 1e6;

    mps.clear();
    for(int i =0; i < mis.size(); i++){
        mps.append(mis[i]*mvs[i]);

        if(mvs[i] > maxv)maxv=mvs[i];
        if(mis[i] > maxi)maxi=mis[i];
        if(mps[i] > maxp)maxp=mps[i];
        if(mvs[i] < minv)minv=mvs[i];
        if(mis[i] < mini)mini=mis[i];
        if(mps[i] < minp)minp=mps[i];
    }

    ui->sweepPlot->yAxis->setRange (mini, maxi*1.05);
    ui->sweepPlot->yAxis2->setRange(minp, maxp*1.05);
    ui->sweepPlot->xAxis->setRange (minv, maxv*1.05);
    mUpdateValPlot = true;
}

void PageSweep::sweepButtonClicked(){
    if(mMPPT){
        double start = ui->Start->value();
        double end = ui->End->value();
        mMPPT->commands()->getSweep(start,end);
    }
}

void PageSweep::saveSweep(){
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Sweep"), "",
            tr("csv (*.csv);;All Files (*)"));

    if (fileName.isEmpty()){
            return;
    }
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),file.errorString());
            return;
        }
        QTextStream out(&file);
        out << "V\tI\tP\n";
        for(int i =0; i < mis.size(); i++){
            out << mvs[i] << '\t' << mis[i] << '\t'<< mps[i] << '\n';
        }
    }
}


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "HT6022.h"
#include "worker.h"
#include "dso.h"
#include <QDebug>



HT6022_DeviceTypeDef Device;

workerThread worker;
typedef struct DSO_CHANNEL{
    bool Enabled;
    double VScale;
    HT6022_IRTypeDef VRange;
    double VOffset;
    double Vdiv;
    uint8_t Zero;
}DSO_CHANNEL;

DSO_CHANNEL Channel1,Channel2;

double CursorX1,CursorX2,CursorY1,CursorY2;
double VTrigger;
double Ts,Tdiv;
int DSOMode = 0, DSOStatus = 0;
int ChTrigger = 1, RiseTrigger = 1;
HT6022_DataSizeTypeDef MemDepth = HT6022_32KB;
QVector<double>x_vec(HT6022_32KB);
QCPItemLine *vCursorX1,*vCursorX2,*vCursorTrigger;
QCPItemText *MemDepthText;
unsigned char i1;




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    int res;
    if(!HT6022_Init())
    {
        res= HT6022_FirmwareUpload();
        if (res == HT6022_SUCCESS || res == HT6022_LOADED)
        {

            while(res = HT6022_DeviceOpen (&Device) != 0)
               {
                sleep(1);
                qDebug() << "Waiting for device..." << endl;
                ui->statusBar->showMessage("Loading Firmware...",0);
                QApplication::processEvents();
            }
                Channel1.Enabled = true;
                ui->checkBoxCH1ON->setChecked(true);
                Channel2.Enabled = true;
                ui->checkBoxCH2ON->setChecked(true);
                ui->comboMemDepth->setCurrentIndex(3);
                HT6022_SetSR (&Device, HT6022_100KSa);
                HT6022_SetCH1IR (&Device, HT6022_10V);
                HT6022_SetCH2IR (&Device, HT6022_10V);
                VTrigger = 0.5;
                Ts = 2e-6;
                Tdiv = 1e-3;
                Channel1.Vdiv = 1;
                Channel1.VScale = 5;
                Channel2.Vdiv = 1;
                Channel2.VScale = 5;
                ui->comboBoxV1div->setCurrentIndex(1);
                ui->comboBoxV2div->setCurrentIndex(1);
                ui->comboSampling->setCurrentIndex(TDIV_1MS);
                ui->statusBar->showMessage("Device initialized.",0);
                qDebug() << "Init values set\r\n";
                worker.start();
                worker.blockSignals(1);
                ui->actionSave_to_file->setEnabled(false);
        }
        else
           {

            exit(res);
        }
    }
    else
    {
        exit(-1);
    }
    setupPlot(ui->customPlot);
    connect(&worker, SIGNAL(dataReady()), this, SLOT(updatePlot()));
}

void MainWindow::setupPlot(QCustomPlot *customPlot)
{
    int i;
  // create graph and assign data to it:
  customPlot->addGraph();
  //customPlot->graph(0)->setData(x, y);
  customPlot->addGraph();

  customPlot->addGraph();//trigger


  // set axes ranges, so we see all data:
  //customPlot->xAxis->setRange(0, 16);
  // add the text label at the top:
  QCPItemText *channelText = new QCPItemText(customPlot);
MemDepthText = new QCPItemText(customPlot);
  vCursorX1 = new QCPItemLine(customPlot);
  vCursorX1->setPen(QColor(Qt::white));
  vCursorX2 = new QCPItemLine(customPlot);
  vCursorX2->setPen(QColor(Qt::white));
  vCursorTrigger = new QCPItemLine(customPlot);
  vCursorTrigger->setPen(QColor(Qt::darkYellow));
  customPlot->addItem(channelText);


  MemDepthText->position->setCoords(0,-0.95);
  MemDepthText->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
  MemDepthText->setText("32Kpts");
  MemDepthText->setFont(QFont(font().family(), 10));
  MemDepthText->setColor(QColor(Qt::yellow));

  channelText->position->setCoords(0, 1); // move 10 pixels to the top from bracket center anchor
  channelText->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
  channelText->setText("CH1");
  channelText->setFont(QFont(font().family(), 10));
  channelText->setColor(QColor(Qt::yellow));
  customPlot->xAxis->setTickLabels(0);
  customPlot->yAxis->setTickLabels(0);

  customPlot->xAxis->setRange(0, 10 * Tdiv);
  for(i=0;i<HT6022_32KB;i++)
      x_vec[i]=i*Ts;
  customPlot->yAxis->setRange(-1,1);
  customPlot->xAxis->setAutoTickStep(false);
  customPlot->xAxis->setTickStep(Tdiv);
  customPlot->yAxis->setAutoTickStep(false);
  customPlot->yAxis->setTickStep(0.25);
  customPlot->graph(0)->setPen(QPen(Qt::yellow));
  customPlot->axisRect()->setBackground(Qt::black);
  customPlot->graph(1)->setPen(QPen(Qt::cyan));
  customPlot->setInteractions(QCP::iRangeDrag);
  connect(customPlot->yAxis,    SIGNAL(rangeChanged(QCPRange)), this,   SLOT(onYRangeChanged(QCPRange)));
}

void MainWindow::onYRangeChanged(const QCPRange &range)
{
    QCPRange boundedRange = range;
    boundedRange.lower = -1;
    boundedRange.upper = 1;



        ui->customPlot->yAxis->setRange(boundedRange);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btnExit_clicked()
{
    HT6022_DeviceClose(&Device);
    HT6022_Exit();
    exit(0);
}

void MainWindow::on_btnGet_clicked()
{
    if(DSOStatus == 0)
       {
            ui->actionSave_to_file->setEnabled(false);
            worker.blockSignals(0);
            DSOStatus = 1;
            ui->btnGet->setText("STOP");
            if(DSOMode == 2)
            {
                ui->customPlot->graph(0)->clearData();
                ui->customPlot->graph(1)->clearData();
                ui->customPlot->replot();
            }
    }
    else if(DSOStatus == 1)
       {
        ui->actionSave_to_file->setEnabled(true);
        worker.blockSignals(1);
        DSOStatus = 0;
        ui->btnGet->setText("ARM");
    }
}

void MainWindow::updatePlot(){
    int i,triggerIdx,trigger2Idx;
QVector<double>y1_vec(HT6022_32KB);
QVector<double>y2_vec(HT6022_32KB);
double CH1max=-1e6,CH1min=1e6,CH1Vpp;
double CH2max=-1e6,CH2min=1e6,CH2Vpp;
for(i=0;i<HT6022_32KB;i++)
   {
    //y1[i] = (10.0 * ((double)CH1[i]/255.0 -128.0));
   // y2[i] = (10.0 * ((double)CH2[i]/255.0 -128.0));
    y1_vec[i] = Channel1.VScale * (worker.CH1[i]-128.0)/128.0;
    y2_vec[i] = Channel2.VScale * (worker.CH2[i]-128.0)/128.0;

}

    triggerIdx = 0;
    trigger2Idx = 0;
    for(i=1000;i>4;i--)
    {
        if(RiseTrigger == 0)
        {
            if(ChTrigger == 1)
               {
                    if(y1_vec[i]< VTrigger && y1_vec[i+1]>=VTrigger && y1_vec[i+2] >= y1_vec[i+1])
                    {
                        if(triggerIdx == 0)
                        {
                            triggerIdx = i;

                        }
                        else
                        {
                            trigger2Idx = i;

                            break;
                        }

                    }
            }
            else
            {
                if(y2_vec[i]< VTrigger && y2_vec[i+1]>=VTrigger && y2_vec[i+2] >= y2_vec[i+1])
                {
                    if(triggerIdx == 0)
                    {
                        triggerIdx = i;
                    }
                    else
                    {
                        trigger2Idx = i;
                        break;
                    }

                }
            }
        }
        else
        {
            if(ChTrigger == 1)
               {
                    if(y1_vec[i]> VTrigger && y1_vec[i+1]<=VTrigger && y1_vec[i+2] <= y1_vec[i+1])
                    {
                        if(triggerIdx == 0)
                        {
                            triggerIdx = i;

                        }
                        else
                        {
                            trigger2Idx = i;

                            break;
                        }

                    }
            }
            else
            {
                if(y2_vec[i]> VTrigger && y2_vec[i+1]<=VTrigger && y2_vec[i+2] <= y2_vec[i+1])
                {
                    if(triggerIdx == 0)
                    {
                        triggerIdx = i;
                    }
                    else
                    {
                        trigger2Idx = i;
                        break;
                    }

                }
            }
        }
    }
    char freqStr[10];
    float2engStr(freqStr,fabs(1/((trigger2Idx - triggerIdx)*Ts)));
    ui->lblfreq->setText(QString(freqStr) + "Hz");
    for(i=0;i<HT6022_32KB;i++)
    {
        x_vec[i] = (i-triggerIdx)*Ts;
        if(y1_vec[i]>CH1max)
            CH1max =  y1_vec[i];
        if(y1_vec[i]<CH1min)
            CH1min =  y1_vec[i];
        if(y2_vec[i]>CH2max)
            CH2max =  y2_vec[i];
        if(y2_vec[i]<CH2min)
            CH2min =  y2_vec[i];
    }
    CH1Vpp = fabs(CH1max - CH1min);
    CH2Vpp = fabs(CH2max - CH2min);
    ui->lblMaxVal->setText( QString::number(CH1max,'g',2) + "V");
    ui->lblminVal->setText( QString::number(CH1min,'g',2) + "V");
    ui->lblVppVal->setText( QString::number(CH1Vpp,'g',2) + "V");

    ui->lblMaxVal_2->setText( QString::number(CH2max,'g',2) + "V");
    ui->lblminVal_2->setText( QString::number(CH2min,'g',2) + "V");
    ui->lblVppVal_2->setText( QString::number(CH2Vpp,'g',2) + "V");

   // while(!(y1_vec[0]<2 && y1_vec[1]>=2))
    //    y1_vec.removeFirst();
    //ui->customPlot->graph(1)->setData(x_vec,y2_vec);
    for(i=0;i<HT6022_32KB;i++)
    {
        y1_vec[i] = y1_vec[i]/(4*Channel1.Vdiv) + Channel1.VOffset;
        y2_vec[i] = y2_vec[i]/(4*Channel2.Vdiv) + Channel2.VOffset;

    }

    if(DSOMode == 2 && triggerIdx > 0)
    {
        ui->actionSave_to_file->setEnabled(true);
        worker.blockSignals(1);
        DSOStatus = 0;
        ui->btnGet->setText("ARM");
        ui->customPlot->graph(0)->clearData();
        ui->customPlot->graph(1)->clearData();
        if(Channel1.Enabled)
            ui->customPlot->graph(0)->setData(x_vec,y1_vec);

        if(Channel2.Enabled)
            ui->customPlot->graph(1)->setData(x_vec,y2_vec);

        ui->customPlot->replot();
    }
    else if(DSOMode < 2)
      {
        ui->customPlot->graph(0)->clearData();
        ui->customPlot->graph(1)->clearData();
        if(Channel1.Enabled)
            ui->customPlot->graph(0)->setData(x_vec,y1_vec);

        if(Channel2.Enabled)
            ui->customPlot->graph(1)->setData(x_vec,y2_vec);

        ui->customPlot->replot();
    }
}

void MainWindow::on_comboBoxV2div_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0://2V
        Channel2.VRange = HT6022_10V;
        Channel2.Vdiv = 2.0;
        Channel2.VScale = 5.0;
        break;
    case 1://1V
        Channel2.VRange = HT6022_10V;
        Channel2.Vdiv = 1.0;
        Channel2.VScale = 5.0;
        break;
    case 2://0.5V
        Channel2.VRange = HT6022_5V;
        Channel2.Vdiv= 0.5;
        Channel2.VScale = 2.5;
        break;
    case 3://0.2V
        Channel2.VRange = HT6022_5V;
        Channel2.Vdiv = 0.2;
        Channel2.VScale = 2.5;
        break;
    case 4://0.1V
        Channel2.VRange = HT6022_2V;
        Channel2.Vdiv = 0.1;
        Channel2.VScale = 1.0;
        break;
    }
    HT6022_SetCH2IR (&Device, Channel2.VRange);
}

void MainWindow::on_comboBoxV1div_currentIndexChanged(int index)
{

    switch(index)
    {
    case 0://2V
        Channel1.VRange = HT6022_10V;
        Channel1.Vdiv = 2.0;
        Channel1.VScale = 5.0;
        break;
    case 1://1V
        Channel1.VRange = HT6022_10V;
        Channel1.Vdiv = 1.0;
        Channel1.VScale = 5.0;
        break;
    case 2://0.5V
        Channel1.VRange = HT6022_5V;
        Channel1.Vdiv= 0.5;
        Channel1.VScale = 2.5;
        break;
    case 3://0.2V
        Channel1.VRange = HT6022_5V;
        Channel1.Vdiv = 0.2;
        Channel1.VScale = 2.5;
        break;
    case 4://0.1V
        Channel1.VRange = HT6022_2V;
        Channel1.Vdiv = 0.1;
        Channel1.VScale = 1.0;
        break;
    }
    HT6022_SetCH1IR (&Device, Channel1.VRange);
}

void MainWindow::on_comboSampling_currentIndexChanged(int index)
{
    HT6022_SRTypeDef SR;
    int i;
    switch(index)
    {
    case TDIV_100NS://100ns
        SR = HT6022_48MSa;
        Tdiv = 100e-9;
        Ts = 1/48e6;
        break;
    case TDIV_200NS://200ns
        SR = HT6022_48MSa;
        Tdiv = 200e-9;
        Ts = 1/48e6;
        break;
    case TDIV_500NS://500ns
        SR = HT6022_16MSa;
        Tdiv = 500e-9;
        Ts = 1/16e6;
        break;
    case TDIV_1US://1us
        SR = HT6022_8MSa;
        Tdiv = 1e-6;
        Ts = 1/8e6;
        break;
    case TDIV_2US://2us
        SR = HT6022_8MSa;
        Tdiv = 2e-6;
        Ts = 1/8e6;
        break;
    case TDIV_5US://5us
        SR = HT6022_8MSa;
        Tdiv = 5e-6;
        Ts = 1/8e6;
        break;
    case TDIV_10US://10us
        SR = HT6022_4MSa;
        Tdiv = 1e-5;
        Ts = 1/4e6;
        break;
    case TDIV_20US://20us
        SR = HT6022_4MSa;
        Tdiv = 2e-5;
        Ts = 1/4e6;
        break;
    case TDIV_50US://50us
        SR = HT6022_4MSa;
        Tdiv = 5e-5;
        Ts = 1/4e6;
        break;
    case TDIV_100US://100us
        SR = HT6022_1MSa;
        Tdiv = 1e-4;
        Ts = 1e-6;
        break;
    case TDIV_200US://200us
        SR = HT6022_1MSa;
        Tdiv = 2e-4;
        Ts = 1e-6;
        break;
    case TDIV_500US://500us
        SR = HT6022_1MSa;
        Tdiv = 5e-4;
        Ts = 1e-6;
        break;
    case TDIV_1MS://1ms
        SR = HT6022_500KSa;
        Tdiv = 1e-3;
        Ts = 2e-6;
        break;
    case TDIV_2MS://2ms
        SR = HT6022_500KSa;
        Tdiv = 2e-3;
        Ts = 2e-6;
        break;
    case TDIV_5MS://5ms
        SR = HT6022_500KSa;
        Tdiv = 5e-3;
        Ts = 2e-6;
        break;
    case TDIV_10MS://10ms
        SR = HT6022_100KSa;
        Tdiv = 10e-3;
        Ts = 1e-5;
        break;
    case TDIV_20MS://20ms
        SR = HT6022_100KSa;
        Tdiv = 20e-3;
        Ts = 1e-5;
        break;
    case TDIV_50MS://50ms
        SR = HT6022_100KSa;
        Tdiv = 50e-3;
        Ts = 1e-5;
        break;
    case TDIV_100MS://100ms
        SR = HT6022_100KSa;
        Tdiv = 100e-3;
        Ts = 1e-5;
        break;

    }
    ui->customPlot->xAxis->setRange(0, 10 * Tdiv);
    for(i=0;i<HT6022_32KB;i++)
        x_vec[i]=i*Ts;
    ui->customPlot->xAxis->setAutoTickStep(false);
    ui->customPlot->xAxis->setTickStep(Tdiv);

    HT6022_SetSR (&Device,SR);
}



void MainWindow::on_dialTrigger_valueChanged(int value)
{
    if(ChTrigger == 1)
       {
        VTrigger = Channel1.VScale*((double)value/100.0);
        ui->labelTrigger->setText(QString::number(VTrigger,'g',2));
        vCursorTrigger->start->setCoords(-1,VTrigger/(4*Channel1.Vdiv)+Channel1.VOffset);
        vCursorTrigger->end->setCoords(1,VTrigger/(4*Channel1.Vdiv)+Channel1.VOffset);
    }
    else
    {
        VTrigger = Channel2.VScale*((double)value/100.0) ;
        ui->labelTrigger->setText(QString::number(VTrigger,'g',2));
        vCursorTrigger->start->setCoords(-1,VTrigger/(4*Channel2.Vdiv)+Channel2.VOffset);
        vCursorTrigger->end->setCoords(1,VTrigger/(4*Channel2.Vdiv)+Channel2.VOffset);
    }
    ui->customPlot->replot();
}



void MainWindow::on_dialCursorX1_valueChanged(int value)
{
    char valueStr[10];
    CursorX1 = (double)value*Tdiv/100;
    vCursorX1->start->setCoords(CursorX1,-Channel1.VScale);
    vCursorX1->end->setCoords(CursorX1,Channel1.VScale);
    float2engStr(valueStr,fabs(CursorX1-CursorX2));
    ui->labelCurX12->setText(valueStr);
    ui->customPlot->replot();

}





void MainWindow::on_dialCursorX2_valueChanged(int value)
{
    char valueStr[10];
    CursorX2 = ((double)value*Tdiv)/100;
    vCursorX2->start->setCoords(CursorX2,-Channel1.VScale);
    vCursorX2->end->setCoords(CursorX2,Channel1.VScale);
    float2engStr(valueStr,fabs(CursorX1-CursorX2));
    ui->labelCurX12->setText(valueStr);
    ui->customPlot->replot();
}



void MainWindow::on_dialCursorV1_valueChanged(int value)
{
    Channel1.VOffset = (double)value/100.0;
    if(ChTrigger == 1)
       {
    vCursorTrigger->start->setCoords(-1,VTrigger/(4*Channel1.Vdiv)+Channel1.VOffset);
    vCursorTrigger->end->setCoords(1,VTrigger/(4*Channel1.Vdiv)+Channel1.VOffset);

    }
    ui->customPlot->replot();
}

void MainWindow::on_dialCursorV1_2_valueChanged(int value)
{
    Channel2.VOffset = (double)value/100.0;
    if(ChTrigger == 2)
      {
    vCursorTrigger->start->setCoords(-1,VTrigger/(4*Channel2.Vdiv)+Channel2.VOffset);
    vCursorTrigger->end->setCoords(1,VTrigger/(4*Channel2.Vdiv)+Channel2.VOffset);

    }
    ui->customPlot->replot();
}

void MainWindow::on_comboBoxCHSel_currentIndexChanged(int index)
{
    ChTrigger = index + 1;
    if(ChTrigger == 1)
    {
        Channel1.Enabled = true;
        ui->checkBoxCH1ON->setChecked(true);
    }
    else
    {
        Channel2.Enabled = true;
        ui->checkBoxCH2ON->setChecked(true);
    }
}

void MainWindow::on_comboBox_4_currentIndexChanged(int index)
{
    switch(index)
    {
    case 0:
        DSOMode = 0;
        break;
    case 1:
        DSOMode = 1;
        break;
    case 2:
        DSOMode = 2;
        break;
    }
}

void MainWindow::on_comboMemDepth_currentIndexChanged(int index)
{
    switch(index)
       {
        case 0:
        MemDepth = HT6022_1MB;
        MemDepthText->setText("1Mpts");
        break;
    case 1:
        MemDepth = HT6022_512KB;
       MemDepthText->setText("512Kpts");
        break;
    case 2:
        MemDepth = HT6022_128KB;
        MemDepthText->setText("128Kpts");
        break;
    case 3:
        MemDepth = HT6022_32KB;
        //MemDepthText->setText("32Kpts");
        break;
    case 4:
        MemDepth = HT6022_16KB;
        //MemDepthText->setText("32Kpts");
        break;
    case 5:
        MemDepth = HT6022_8KB;
        //MemDepthText->setText("32Kpts");
        break;
    default:
        break;
    }

}

void MainWindow::on_actionSave_to_file_triggered()
{
    int i;
    FILE * datafile;
        datafile = fopen("/Users/carmix/Sviluppo/datasample.txt","wt");
        fprintf(datafile,"T,CH1,CH2\r\n");
        for(i=0;i<MemDepth;i++)
            fprintf(datafile,"%3.2f,%3.2f,%3.2f\r\n",
                    double(i)*Ts,
                    Channel1.VScale * (worker.CH1[i]-128.0)/128.0,
                    Channel2.VScale * (worker.CH2[i]-128.0)/128.0);
        fclose(datafile);
}



void MainWindow::on_checkBoxCH1ON_toggled(bool checked)
{
    Channel1.Enabled = checked;
}


void MainWindow::on_checkBoxCH2ON_toggled(bool checked)
{
    Channel2.Enabled = checked;
}


void MainWindow::on_comboBox_rise_currentIndexChanged(int index)
{
    RiseTrigger = index; //0 Rise 1 Fall
}

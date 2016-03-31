#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "HT6022.h"
#include "worker.h"

#include <QDebug>
HT6022_DeviceTypeDef Device;

workerThread worker;
typedef struct DSO_CHANNEL{
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
int ChTrigger = 1;
HT6022_DataSizeTypeDef MemDepth = HT6022_32KB;
QVector<double>x_vec(HT6022_32KB);
QCPItemLine *vCursorX1,*vCursorX2,*vCursorTrigger;
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

                HT6022_SetSR (&Device, HT6022_100KSa);
                HT6022_SetCH1IR (&Device, HT6022_10V);
                HT6022_SetCH2IR (&Device, HT6022_10V);
                VTrigger = 0.5;
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
  vCursorX1 = new QCPItemLine(customPlot);
  vCursorX1->setPen(QColor(Qt::white));
  vCursorX2 = new QCPItemLine(customPlot);
  vCursorX2->setPen(QColor(Qt::white));
  vCursorTrigger = new QCPItemLine(customPlot);
  vCursorTrigger->setPen(QColor(Qt::darkYellow));
  customPlot->addItem(channelText);

  //channelText->position->setParentAnchor(bracket->center);
  channelText->position->setCoords(0, 1); // move 10 pixels to the top from bracket center anchor
  channelText->setPositionAlignment(Qt::AlignTop|Qt::AlignLeft);
  channelText->setText("CH1");
  channelText->setFont(QFont(font().family(), 10));
  channelText->setColor(QColor(Qt::yellow));
  customPlot->xAxis->setTickLabels(0);
  customPlot->yAxis->setTickLabels(0);
  Ts = 1e-5;
  Tdiv = 1e-3;
  Channel1.Vdiv = 1;
  Channel1.VScale = 5;
  Channel2.Vdiv = 1;
  Channel2.VScale = 5;
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
for(i=0;i<HT6022_32KB;i++)
   {
    //y1[i] = (10.0 * ((double)CH1[i]/255.0 -128.0));
   // y2[i] = (10.0 * ((double)CH2[i]/255.0 -128.0));
    y1_vec[i] = Channel1.VScale * (worker.CH1[i]-128.0)/128.0;
    y2_vec[i] = Channel2.VScale * (worker.CH2[i]-128.0)/128.0;

}

    triggerIdx = 0;
    trigger2Idx = 0;
    for(i=2000;i>4;i--)
    {
        if(ChTrigger == 1)
           {
                if(y1_vec[i]< VTrigger && y1_vec[i+1]>=VTrigger)
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
            if(y2_vec[i]< VTrigger && y2_vec[i+1]>=VTrigger)
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
    }
    CH1Vpp = fabs(CH1max - CH1min);
    ui->lblMaxVal->setText( QString::number(CH1max,'g',2));
    ui->lblminVal->setText( QString::number(CH1min,'g',2));
    ui->lblVppVal->setText( QString::number(CH1Vpp,'g',2));

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
        ui->customPlot->graph(0)->setData(x_vec,y1_vec);
        ui->customPlot->graph(1)->setData(x_vec,y2_vec);
        ui->customPlot->replot();
    }
    else if(DSOMode < 2)
      {
    ui->customPlot->graph(0)->setData(x_vec,y1_vec);
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
    case 0://100ns
        SR = HT6022_4MSa;
        Tdiv = 100e-9;
        Ts = 250e-9;
        break;
    case 1://500ns
        SR = HT6022_4MSa;
        Tdiv = 500e-9;
        Ts = 250e-9;
        break;
    case 2://1us
        SR = HT6022_4MSa;
        Tdiv = 1e-6;
        Ts = 250e-9;
        break;
    case 3://5us
        SR = HT6022_4MSa;
        Tdiv = 5e-6;
        Ts = 250e-9;
        break;
    case 4://10us
        SR = HT6022_4MSa;
        Tdiv = 1e-5;
        Ts = 250e-9;
        break;
    case 5://50us
        SR = HT6022_1MSa;
        Tdiv = 5e-5;
        Ts = 1e-6;
        break;
    case 6://100us
        SR = HT6022_500KSa;
        Tdiv = 1e-4;
        Ts = 2e-6;
        break;
    case 7://200us
        SR = HT6022_500KSa;
        Tdiv = 2e-4;
        Ts = 2e-6;
        break;
    case 8://500us
        SR = HT6022_500KSa;
        Tdiv = 5e-4;
        Ts = 2e-6;
        break;
    case 9://1ms
        SR = HT6022_100KSa;
        Tdiv = 1e-3;
        Ts = 1e-5;
        break;
    case 10://5ms
        SR = HT6022_100KSa;
        Tdiv = 5e-3;
        Ts = 1e-5;
        break;
    case 11://10ms
        SR = HT6022_100KSa;
        Tdiv = 10e-3;
        Ts = 1e-5;
        break;
    case 12://50ms
        SR = HT6022_100KSa;
        Tdiv = 50e-3;
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
        break;
    case 1:
        MemDepth = HT6022_512KB;
        break;
    case 2:
        MemDepth = HT6022_128KB;
        break;
    case 3:
        MemDepth = HT6022_32KB;
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

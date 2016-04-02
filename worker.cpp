#include "worker.h"
#include "HT6022.h"
#include <QDebug>

extern HT6022_DeviceTypeDef Device;
extern HT6022_DataSizeTypeDef MemDepth;

void workerThread::run()
{
    while(1)
    {

    HT6022_ReadData(&Device, CH1, CH2, MemDepth, 0);
    msleep(40);
   /* FILE * datafile;
    datafile = fopen("/Users/carmix/Sviluppo/data.txt","wt");
    for(i=0;i<HT6022_1KB;i++)
        fprintf(datafile,"%d,%d\r\n",CH1[i],CH2[i]);
    fclose(datafile);*/
    emit dataReady();
    }
}


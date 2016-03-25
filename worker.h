#ifndef WORKER_H
#define WORKER_H
#include <QThread>
#include "HT6022.h"

class workerThread : public QThread
{
       Q_OBJECT
public:
    unsigned char CH1[1024*1024];
    unsigned char CH2[1024*1024];
signals:
    void dataReady();
private:
    void run();

};

#endif // WORKER_H

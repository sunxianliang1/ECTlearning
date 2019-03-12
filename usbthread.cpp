#include "usbthread.h"

usbthread::usbthread(QObject *parent) : QThread(parent)
{
    readflag=0;
}

void usbthread::run()
{
    const int QueueSize = 32;//队列数量
    long totalTransferSize=64 ;//缓冲区
    if(electric==8)
        totalTransferSize=64;
    else if(electric==12)
        totalTransferSize=128;
    PUCHAR			*buffersInput = new PUCHAR[QueueSize];
    PUCHAR			*contextsInput = new PUCHAR[QueueSize];
    OVERLAPPED		inOvLap[32];
    ThepBulkIn->SetXferSize(totalTransferSize);
    for (int nCount = 0; nCount < QueueSize; nCount++)
    {
        buffersInput[nCount] = new UCHAR[totalTransferSize];
        inOvLap[nCount].hEvent = CreateEvent(NULL, false, false, NULL);
        memset(buffersInput[nCount], 0xEF, totalTransferSize);
    }
    for (int nCount = 0; nCount < QueueSize; nCount++)
    {
        contextsInput[nCount] = ThepBulkIn->BeginDataXfer(buffersInput[nCount], totalTransferSize, &inOvLap[nCount]);
    }
    long nCount = 0;
    long readLength = totalTransferSize;
    quint8 b1,b2;
    qint16 c,a[32],b[N];
    while (readflag)
    {
        if (!ThepBulkIn->WaitForXfer(&inOvLap[nCount], 2))
        {
            ThepBulkIn->Abort();
            if (ThepBulkIn->LastError == ERROR_IO_PENDING)
                WaitForSingleObject(inOvLap[nCount].hEvent, 2);
        }
        if (ThepBulkIn->FinishDataXfer(buffersInput[nCount], readLength, &inOvLap[nCount], contextsInput[nCount]))
        {
             for(int i=0;i<readLength;i+=2)
             {
                 b2=*(buffersInput[nCount]+i);
                 b1=*(buffersInput[nCount]+i+1);
                 c=b1<<8|b2;
                 a[i/2]=c;
             }
             if(a[0]==0x6556&&a[1]==0x0111&&a[2]==0x001C&&a[N+3]==0x0520)
             {
                 for(int i=3;i<N+3;i++)
                 {
                     b[i-3]=a[i];
                 }
                 emit readdate(b);
             }
        }
        contextsInput[nCount] = ThepBulkIn->BeginDataXfer(buffersInput[nCount], totalTransferSize, &inOvLap[nCount]);
        nCount++;
        if (nCount >= QueueSize)
            nCount = 0;
    }
    ThepBulkIn->Abort();
    for (int j = 0; j< QueueSize; j++)
    {
        CloseHandle(inOvLap[j].hEvent);
        delete[] buffersInput[j];
    }
}

void usbthread::stop()
{
    if(readflag)
        readflag=0;
}

void usbthread::delay(int a)
{
    int i=1000;
    while(a--)
    {
        while(i--);
    }
}

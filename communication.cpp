#include "communication.h"
#include "ui_communication.h"
#include<QDebug>
communication::communication(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::communication)
{
    ui->setupUi(this);
    USBDevice=new CCyUSBDevice();
    readthread=new usbthread(this);
    lightfind=new Qled(this);
    lightempty=new Qled(this);
    lightfull=new Qled(this);
    lightrun=new Qled(this);
    lightfind->setGeometry(25,28,20,20);
    lightempty->setGeometry(25,53,20,20);
    lightfull->setGeometry(25,78,20,20);
    lightrun->setGeometry(25,103,20,20);
    connect(&TimerforFPS,SIGNAL(timeout()),this,SLOT(slot_setFPS()));
    startinit();
}
communication::~communication()
{
    delete USBDevice;
    delete ui;
}
void communication::startinit()//初始化
{
    for(int i=0;i<N;i++)
    {
        intcapdata[i]=0;
        intcapempty[i]=0;
        intcapfull[i]=0;
    }
    Empty_Prepared =0;
    Full_Prepared  =0;
    FlagSearchLow  =0;
    USBConnected   =0;
    startreadflag  =0;
    lightfind->setAlarm(0);
    lightempty->setAlarm(0);
    lightfull->setAlarm(0);
    lightrun->setAlarm(0);
    time=0;
    fps=0;
    count=0;
    QB_SearchLow[0]          =0x6556;QB_SearchLow[1]          =0x0100;QB_SearchLow[2]          =0x0520;
    QB_Calibration_Prepare[0]=0x6556;QB_Calibration_Prepare[1]=0x0101;QB_Calibration_Prepare[2]=0x0520;
    QB_Calibrate_Empty[0]    =0x6556;QB_Calibrate_Empty[1]    =0x0102;QB_Calibrate_Empty[2]    =0x0520;
    QB_Calibrate_Full[0]     =0x6556;QB_Calibrate_Full[1]     =0x0103;QB_Calibrate_Full[2]     =0x0520;
    QB_ReadData[0]           =0x6556;QB_ReadData[1]           =0x0111;QB_ReadData[2]           =0x0520;
}
bool communication::connectUSB()//连接下位机
{
    int   devices = USBDevice->DeviceCount();		//获取连接设备个数
    int   vID, pID;
    int  d = 0;
    do
    {
        USBDevice->Open(d);							//打开USB设备
        vID = USBDevice->VendorID;					//获取VID
        pID = USBDevice->ProductID;					//获取PID
        d++;
    }while ((d < devices) && (vID != 0x04B4) && (pID != 0x00B0));
    if (vID == 0x04B4 && pID == 0x00B0)				//如果是指定的USB设备
    {
        epBulkOut = USBDevice->EndPointOf(0x02);
        epBulkIn = USBDevice->EndPointOf(0x86);
        USBConnected=1;
    }
    else
    {
        USBConnected=0;
    }
    return USBConnected;
}
bool communication::write2USB(qint16 a[])//向下位机发送数据，要求a为a[3]
{
    quint8 buffer[6];
    quint8 b1=0,b2=0;
    for(int i=0;i<3;i++)
    {
        b2=a[i]>>8;//windows usb 传输时低8位在前
        b1=a[i]&0xff;
        buffer[i*2]=b1;
        buffer[i*2+1]=b2;
    }
    PUCHAR pbuffer=(PUCHAR)buffer;
    long length=6;
    OVERLAPPED OutOvlp;
    OutOvlp.hEvent=CreateEvent(NULL,0,0,TEXT("CYUSB_OUT"));
    UCHAR *outtext=epBulkOut->BeginDataXfer(pbuffer,length,&OutOvlp);
    epBulkOut->WaitForXfer(&OutOvlp,100);
    bool writesucceed=epBulkOut->FinishDataXfer(pbuffer,length,&OutOvlp,outtext);
    if(!writesucceed)epBulkOut->Abort();
    return writesucceed;
}
bool communication::read4USB(qint16 a[])//从下位机读取，读取的数据存在a[]中
{
    long len=32;
    bool readsuccsed;
    long len1=len*2;

    OVERLAPPED InOvlp;
    InOvlp.hEvent=CreateEvent(NULL,0,0,TEXT("CYUSB_IN"));
    PUCHAR pbuffer=new UCHAR[len1];
    UCHAR *intext=epBulkIn->BeginDataXfer(pbuffer,len1,&InOvlp);
    if(!epBulkIn->WaitForXfer(&InOvlp,100))
    {
        if (epBulkIn->LastError == ERROR_IO_PENDING)
            WaitForSingleObject(InOvlp.hEvent, 2);
    }
    readsuccsed=epBulkIn->FinishDataXfer(pbuffer,len1,&InOvlp,intext);
     if (readsuccsed)
    {
        quint8 b1=0,b2=0;
        qint16 c=0;
        for(int i=0;i<len;i++)
        {
            b2=*(pbuffer+i*2);
            b1=*(pbuffer+i*2+1);
            c=b1<<8|b2;
            a[i]=c;
        }
    }
    epBulkIn->Abort();
    delete pbuffer;
    return readsuccsed;
}
int  communication::XY_Prepare_Calibration()//标定准备
{
    qint16 Read[32];
    int j=10;
    for (;j>0;j--)
    {
        delay(10);
        bool write=write2USB(QB_Calibration_Prepare);
        delay(10);
        if(!write)continue;
        read4USB(Read);
        if(Read[0]==0x6556&&Read[1]==0x0101&&Read[2]==0x0002&&Read[5]==0x0520)
        {
            if(Read[3]==0x0101)//下位机准备好
            {
                if(Empty_Prepared!=2)//上位机无数据
                {
                    Empty_Prepared=1;
                    lightempty->setAlarm(Empty_Prepared);
                }
            }
            if(Read[4]==0x0101)
            {
                if(Full_Prepared!=2)
                {
                    Full_Prepared=1;
                    lightfull->setAlarm(Full_Prepared);
                }
            }
            j=100;//标定准备成功
            break;
        }
    }
    return j;
}
void communication::on_pushBut_Refind_clicked()//寻找下位机
{
    if(!USBConnected)
    {
        if(connectUSB())
            ui->pushBut_Refind->setText(tr("重新寻找"));
        else
        {
            QMessageBox::information(this, tr("连接USB"), tr("未找到USB设备！"));
            return ;
        }
    }
    int j=100;
    qint16 Read[32]={0};
    read4USB(Read);read4USB(Read);read4USB(Read);read4USB(Read);//先读四次，读空缓冲区
    for (;j>0;j--)
    {
        delay(10);
        bool write=write2USB(QB_SearchLow);
        delay(10);
        if(!write)continue;
        read4USB(Read);
        if (Read[0]==0x6556&&Read[1]==0x0100&&Read[2]==0x0520)
        {
            FlagSearchLow=1;
            lightfind->setAlarm(2);
            int k=XY_Prepare_Calibration();
            if(k==0)
                QMessageBox::information(this, tr("准备标定"), tr("准备超时！"));
            if(k==100)
                QMessageBox::information(this, tr("准备标定"), tr("准备成功！"));
            break;
        }
    }
    if(j==0)//USB连接好但下位机无响应
        QMessageBox::information(this, tr("寻找下位机"), tr("寻找超时！"));
}
void communication::delay(int i)//延时函数
{
    int j;
    while(i--)
    {
        j=10;
        while(j--);
    }

}
void communication::on_pushBut_EmptyCalibration_clicked()//点击空管标定按钮
{
    if(!FlagSearchLow)
    {
        QMessageBox::information(this, tr("警告"), tr("下位机未连接。"));
        return;
    }
    if(Empty_Prepared!=2)
    {
        qint16 Read[N+4];
        int k=0;
        for(int i=0;i<10;i++)
        {
            bool write=write2USB(QB_Calibrate_Empty);
            delay(100);
            if(!write)continue;
            read4USB(Read);
            if(Read[0]==0x6556&&Read[1]==0x0102&&Read[2]==0x001C&&Read[N+3]==0x0520)
            {
                for(int j=3;j<N+3;j++)
                    intcapempty[k++]=Read[j];
                Empty_Prepared=2;
                lightempty->setAlarm(Empty_Prepared);
                QMessageBox::information(this, tr("标定空管"), tr("标定成功。"));
                break;
            }
        }
        if(k==0)
            QMessageBox::information(this, tr("标定空管"), tr("标定失败，请重新标定。"));
    }
    else
    {
        qint16 empty[N]={0},a[N]={0};
        qint16 emptysum[N]={0};
        bool issame=0;
        for(int i=0;i<50;i++)
        {
            for(int j=0;j<N;j++)
            {
                if(empty[j]==intcapdata[j])
                {
                    issame=1;
                    break;
                }
                empty[j]=intcapdata[j];
            }
            if(issame==0)
            {
                for(int j=0;j<N;j++)
                    emptysum[j]=empty[j];
                break;
            }
            issame=0;
        }
        if(!issame)
        {
            QMessageBox::information(this, tr("标定空管"), tr("标定成功。"));
            emit calibration();
        }
        else
        {
            QMessageBox::information(this, tr("标定空管"), tr("标定失败，请重新标定。"));
        }
       // qDebug()<<k;
    }
}
void communication::on_pushBut_FullCalibration_clicked()//点击满管标定按钮
{
    if(!FlagSearchLow)
    {
        QMessageBox::information(this, tr("警告"), tr("下位机未连接。"));
        return;
    }
    if(Full_Prepared!=2)
    {
        qint16 Read[N+4];
        int k=0;
        for(int i=0;i<10;i++)
        {
            delay(10);
            bool write=write2USB(QB_Calibrate_Full);
            delay(100);
            if(!write)continue;
            read4USB(Read);
            if(Read[0]==0x6556&&Read[1]==0x0103&&Read[2]==0x001C&&Read[N+3]==0x0520)
            {
                k=0;
                for(int j=3;j<N+3;j++)
                    intcapfull[k++]=Read[j];
                Full_Prepared=2;
                lightfull->setAlarm(Full_Prepared);
                QMessageBox::information(this, tr("标定满管"), tr("标定成功。"));
                break;
            }
        }
        if(k==0)
            QMessageBox::information(this, tr("标定满管"), tr("标定失败，请重新标定。"));
    }
    else
    {
        qint16 full[N]={0};
        qint16 fullsum[N]={0};
        bool issame=0;
        for(int i=0;i<50;i++)
        {

            for(int j=0;j<N;j++)
            {
                if( full[j]==intcapdata[j])
                {
                    issame=1;
                    break;
                }
                full[j]=intcapdata[j];
            }
            if(issame==0)
            {
                for(int j=0;j<N;j++)
                    fullsum[j]=full[j];
            }
            issame=0;
        }
        if(!issame)
        {
            QMessageBox::information(this, tr("标定满管"), tr("标定成功。"));
            emit calibration();
        }
        else
        {
            QMessageBox::information(this, tr("标定满管"), tr("标定失败，请重新标定。"));
        }
    }
}
void communication::on_pushBut_StartRead_clicked()//开始测量
{
    if(Empty_Prepared==2&&Full_Prepared==2)
    {
        int i=10;
        while(i--)
        {
            if(write2USB(QB_ReadData))
                break;
        }
        if(i==0)
        {
            QMessageBox::information(this, tr("开始测量"), tr("发生错误，未开始测量。"));
            return;
        }
        startreadflag=1;
        TimerforFPS.start(1000);
        connect(readthread,SIGNAL(readdate(qint16*)),this,SLOT(slot_received(qint16*)),Qt::BlockingQueuedConnection);
        lightrun->setAlarm(2);
        readthread->readflag=1;
        readthread->ThepBulkIn=epBulkIn;
        readthread->start();
    }
    else
    {
        QMessageBox::information(this, tr("开始测量"), tr("尚未完全标定。"));
        lightrun->setAlarm(0);
    }
}
void communication::on_pushBut_ReadCalibrition_clicked()//读取标定
{
    QString cal_strname;
    cal_strname=QFileDialog::getOpenFileName(this,tr("打开标定数据"),"",tr("数据文件(*txt)"));
    if(cal_strname!="")
    {
        QFile * cal_filename;
        cal_filename=new QFile(cal_strname);
        cal_filename->open(QIODevice::ReadOnly|QIODevice::Text);
        QTextStream incal;
        incal.setDevice(cal_filename);
        int a[N],b[N];
        QString string;
        string=incal.readLine();
        if (string!="calibration of empty:")
        {
            QMessageBox::warning(this,tr("警告"),tr("文件错误"));
            return;
        }
        for (int i=0;i<N;i++)
        {
            incal>>a[i];
        }
        string=incal.readLine();//读取多余空格
        string=incal.readLine();
        if (string!="calibration of full:")
        {
            QMessageBox::warning(this,tr("警告"),tr("文件错误"));
            return;
        }
        for (int i=0;i<N;i++)
        {
            incal>>b[i];
        }
        cal_filename->close();
        for (int i=0;i<N;i++)
        {
            intcapempty[i]=a[i];
            intcapfull[i]=b[i];
        }
        Empty_Prepared=2;
        lightempty->setAlarm(Empty_Prepared);
        Full_Prepared=2;
        lightfull->setAlarm(Full_Prepared);
    }
}
void communication::on_pushBut_StopRead_clicked()//停止测量
{
    readthread->stop();
    TimerforFPS.stop();
    //向下位机写入寻找指令，使其停止发送数据
    write2USB(QB_SearchLow);write2USB(QB_SearchLow);write2USB(QB_SearchLow);write2USB(QB_SearchLow);
    //下位机为四重缓冲，因此在关闭USB前，读取四次，
    //这里读取8次是因为写入数据到下位机读到数据之间，下位机可能还会发送几组数据
    qint16 Read[32];
    read4USB(Read);read4USB(Read);read4USB(Read);read4USB(Read);
    read4USB(Read);read4USB(Read);read4USB(Read);read4USB(Read);
    disconnect(readthread,SIGNAL(readdate(qint16*)),this,SLOT(slot_received(qint16*)));
    startinit();
}

void communication::slot_received(qint16 *a)
{
    for(int i=0;i<N;i++)
        intcapdata[i]=*(a+i);
    emit received();
    count++;
}

void communication::slot_setFPS()
{
    fps=count;
    qDebug()<<fps;
    count=0;
}

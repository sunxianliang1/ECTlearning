#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <QWidget>
#include <windows.h>
#include<QObject>
#include<QFile>
#include<QTimer>
#include<QTime>
#include"Qled.h"
#include"usbthread.h"
#include"QMessageBox"
#include"option.h"
#include<QTextStream>
#include<QFileDialog>
namespace Ui {
    class communication;
}

class communication : public QWidget
{
    Q_OBJECT
signals:
    void calibration();
    void received();
public:
    explicit communication(QWidget *parent = 0);
    ~communication();

    //计数变量
    float   time;
    float   fps;
    int     count;
    QTimer  TimerforFPS;
    //接收到的变量
    qint16  intcapdata[N];
    qint16  intcapempty[N];
    qint16  intcapfull[N];
    usbthread *readthread;
    bool startreadflag;
    //定义指示灯
    Qled *lightfind,*lightempty,*lightfull,*lightrun;
    CCyUSBDevice *USBDevice;//定义usb端口
    CCyUSBEndPoint *epBulkIn,*epBulkOut;//定义usb端点
    //标志
    int  Empty_Prepared;
    int  Full_Prepared;
    bool FlagSearchLow;
    bool USBConnected;

private slots:
    void on_pushBut_Refind_clicked();
    void on_pushBut_EmptyCalibration_clicked();
    void on_pushBut_FullCalibration_clicked();
    void on_pushBut_StartRead_clicked();
    void on_pushBut_ReadCalibrition_clicked();
    void on_pushBut_StopRead_clicked();
public slots:
    void slot_received(qint16* a);
    void slot_setFPS();
private:
    Ui::communication *ui;


    //协议变量
    qint16  QB_SearchLow[3];//寻找下位机
    qint16  QB_Calibration_Prepare[3];//准备标定
    qint16  QB_Calibrate_Empty[3];//标定空管
    qint16  QB_Calibrate_Full[3];//标定满管
    qint16  QB_ReadData[3];//读取数据


    void delay(int i);
    void startinit();//数据初始化
    bool connectUSB();//连接USB
    bool write2USB(qint16 a[]);
    bool read4USB(qint16 a[]);
    int  XY_Prepare_Calibration();//准备标定
    void XY_Readdata();
};

#endif // COMMUNICATION_H

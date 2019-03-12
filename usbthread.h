#ifndef USBTHREAD_H
#define USBTHREAD_H

#include<option.h>
#include <QObject>
#include<cyapi/CyAPI.h>
#include<QThread>
#include <windows.h>
#pragma comment (lib,"C:\\Program Files (x86)\\Windows Kits\\8.1\\Lib\\winv6.3\\um\\x64\\SetupAPI.lib")
#pragma comment (lib,"C:\\Program Files (x86)\\Microsoft Visual Studio 14.0\\VC\\lib\\amd64\\legacy_stdio_definitions.lib")
#pragma comment (lib,"C:\\Program Files (x86)\\Windows Kits\\8.1\\Lib\\winv6.3\\um\\x64\\User32.lib")
//#pragma comment (lib,"E:\\qtproject\\ECT-USB\\ECT-USB\\cyapi\\CyAPI.lib")
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
class usbthread : public QThread
{
    Q_OBJECT
public:
    explicit usbthread(QObject *parent = 0);
    CCyUSBEndPoint  *ThepBulkIn;
    void run();
    void stop();
    bool readflag;
    void delay(int a);
signals:
    void readdate(qint16 *a);
public slots:
};

#endif // USBTHREAD_H

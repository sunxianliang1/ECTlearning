#ifndef COMPUTE_H
#define COMPUTE_H
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
# pragma execution_character_set("utf-8")
#endif
#include<QFile>
#include<QTextStream>
#include<QMessageBox>
#include<QDir>
#include <QApplication>
#include<QString>
#include"option.h"
class compute
{

private:
    float cm[N];
    float cmfm[N];
    float lmc[N][M];
    float sb[N][M];
    float max;
public:
    float fltcapempty[N];
    float fltcapfull[N];
    float gk[M];
    void  set(float cm1[]);
    void  comp();
    float eigen();
    void  matrixx(float A[][M],float x[],float v[]);
    float slove(float v[]);

    compute();
    ~compute();
};

#endif // COMPUTE_H

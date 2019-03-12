#ifndef CCTIONFACE_VIEW_H
#define CCTIONFACE_VIEW_H

#include <QWidget>
#include"qcustomplot.h"
#include<QFile>
#include<QMessageBox>
#include<QDateTime>

namespace Ui {
class conrect;
}

class cuiconrect : public QWidget
{
    Q_OBJECT
signals:
   void HideIt();
   void ScreenShot();
public:
    explicit cuiconrect(QWidget *parent = 0);
    ~cuiconrect();
    void initt();
    void closeEvent(QCloseEvent *event);
    void plot(const float data[]);
    bool ScreenShotPic(QString a);
    float plotd[200][50];
private slots:
    void on_pushBut_Screenshot_clicked();
private:
    Ui::conrect *ui;
    QCPColorMap *colorMap;
    int nx;
    int ny;
    int node[50][30];
    int num[50];
    QDateTime time_datetime;//系统时间(时间类)
    QString   str_datetime;//系统时间(字符串)
};

#endif // CCTIONFACE_VIEW_H

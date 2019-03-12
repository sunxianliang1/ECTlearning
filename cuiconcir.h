#ifndef CCTTIONIMGVIEW_H
#define CCTTIONIMGVIEW_H

#include <QWidget>
#include"qcustomplot.h"
#include<QFile>
#include<QMessageBox>


namespace Ui {
class concir;
}

class cuiconcir : public QWidget
{
    Q_OBJECT
signals:
   void HideIt();
   void change();
   void ScreenShot();
public:
    explicit cuiconcir(QWidget *parent = 0);
    ~cuiconcir();
    void closeEvent(QCloseEvent *event);
    void plot (const float data[] );
    bool ScreenShotPic(QString a);
private slots:
    void on_dial_sliderMoved(int position);
    void on_pushBut_SreenShot_clicked();

private:
    Ui::concir *ui;
    QCPColorMap *colorMap;
    int nx;
    int ny;
    int node[201][201][6];
    float areph[201][201][6];
    float t;//角度
};

#endif // CCTTIONIMGVIEW_H

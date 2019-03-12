#ifndef CDATAVIEW_H
#define CDATAVIEW_H

#include <QWidget>
#include<QVector>
#include"qcustomplot.h"
#include "option.h"
namespace Ui {
class capcolu;
}

class cuicapcolu : public QWidget
{
    Q_OBJECT
signals:
   void HideIt();
public:
    explicit cuicapcolu(QWidget *parent = 0);
    ~cuicapcolu();
    void plot(const float cdata[]);
    void closeEvent(QCloseEvent *event);
private:
    Ui::capcolu *ui;
    QCPBars *myBars;
    QVector <double> x;
    QVector <double> y;
};

#endif // CDATAVIEW_H

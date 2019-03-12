#ifndef CONCENTRATIONVIEW_H
#define CONCENTRATIONVIEW_H

#include <QWidget>
#include<QFile>
#include<QMessageBox>
#include"option.h"
namespace Ui {
class conline;
}

class cuiconline : public QWidget
{
    Q_OBJECT
signals:
   void HideIt();
public:
    explicit cuiconline(QWidget *parent = 0);
    ~cuiconline();
    void initt();
    void closeEvent(QCloseEvent *event);
    void plot(const float data[50]);
private:
    Ui::conline *ui;
    float area[M];
public:
    QVector <double> x;
    QVector <double> y;
};

#endif // CONCENTRATIONVIEW_H

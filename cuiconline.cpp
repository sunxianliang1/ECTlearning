#include "cuiconline.h"
#include <ui_conline.h>

cuiconline::cuiconline(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::conline)
{
    ui->setupUi(this);
    x.resize(50);
    y.resize(50);
    for (int i=0;i<50;i++)
    {
        x[i]=i;
        y[i]=0;
    }
    ui->graphview->addGraph();
    ui->graphview->graph(0)->setData(x,y);
    ui->graphview->xAxis->setLabel(tr("时间"));
    ui->graphview->yAxis->setLabel(tr("浓度"));
    ui->graphview->xAxis->setRange(0,50);
    ui->graphview->yAxis->setRange(0,1);
    ui->graphview->replot();

    QFile indata("draw/concentration line.txt");
    if(!indata.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::information(this,tr("严重错误"),tr("丢失数据文件(concentration line.txt)，这将导致绘图功能失效。"));
        return;
    }
    QTextStream  inda(&indata);
    int i;
    while(1)
    {
        inda>>i;
        if(i==-1)
            break;
        inda>>area[i];
    }
}

cuiconline::~cuiconline()
{
    delete ui;
}

void cuiconline::initt()
{
    for (int i=0;i<50;i++)
    {
        y[i]=0;
    }
    ui->graphview->graph(0)->setData(x,y);
    ui->graphview->replot();
}

void cuiconline::plot(const float data[])
{
    for (int i=49;i>0;i--)
    {
        y[i]=y[i-1];
    }
    float s=0;
    for(int i=0;i<M;i++)
    {
        s+=data[i+1]*area[i];
    }
    y[0]=s/78.4136;
    ui->graphview->graph(0)->setData(x,y);
    ui->graphview->replot();
    QString a;
    a.setNum(y[0]);
    ui->textEdit->append(a);
}
void cuiconline::closeEvent(QCloseEvent *event)
{
    emit HideIt();
    event->ignore();
}

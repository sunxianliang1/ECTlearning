#include "cuicapcolu.h"
#include <ui_capcolu.h>
cuicapcolu::cuicapcolu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::capcolu)
{
    float ylength=N+0.0;
    ui->setupUi(this);
    x.resize(N);
    y.resize(N);
    for (int i=0;i<N;i++)
    {
        x[i]=i;
        y[i]=(i+1)/ylength;
    }
    myBars = new QCPBars(ui->cdataview->xAxis,ui->cdataview->yAxis);
    myBars->setData(x,y);
    ui->cdataview->xAxis->setLabel(tr("序号"));
    ui->cdataview->yAxis->setLabel(tr("电容"));
    ui->cdataview->xAxis->setRange(-0.5,ylength+0.5);
    ui->cdataview->yAxis->setRange(0,1.5);
    ui->cdataview->replot();
}
cuicapcolu::~cuicapcolu()
{
    delete ui;
}
void cuicapcolu::plot(const float cdata[])
{
    for (int i=0;i<N;i++)
    {
        y[i]=cdata[i];
    }
    myBars->setData(x,y);
    ui->cdataview->replot();
}
void cuicapcolu::closeEvent(QCloseEvent *event)
{
    emit HideIt();
    event->ignore();
}

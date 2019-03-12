#include "cuiconrect.h"
#include <ui_conrect.h>

cuiconrect::cuiconrect(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::conrect)
{
    ui->setupUi(this);
    ui->customPlot->axisRect()->setupFullAxesBox(true);
    //设置颜色图
    colorMap = new QCPColorMap(ui->customPlot->xAxis, ui->customPlot->yAxis);
    nx = 200;
    ny = 20;
    colorMap->data()->setSize(nx, ny); //我们希望颜色图有nx*ny个数据点
    colorMap->data()->setRange(QCPRange(0, 200), QCPRange(-5, 5)); //这两个值得坐标范围
    //现在我们分配一些数据，通过访问色彩映射的QCPColorMapData实例：
    for(int i=0;i<nx;i++)
        for(int j=0;j<ny;j++)
        {
            plotd[i][j]=j/20.0;
            colorMap->data()->setCell(i,j,plotd[i][j]);
        }
    ui->customPlot->yAxis->setTickLabels(0);
    ui->customPlot->yAxis->setTicks(0);
    // 添加颜色刻度尺
    QCPColorScale *colorScale = new QCPColorScale(ui->customPlot);
    ui->customPlot->plotLayout()->addElement(0, 1, colorScale); //图像0行1列添加颜色标尺
    colorScale->setType(QCPAxis::atRight); // 标尺是坐标轴右边的垂直条（右边是默认的）
    colorMap->setColorScale(colorScale);//关联颜色图和颜色刻度尺
    colorMap->setGradient(QCPColorGradient::gpJet); // 设置颜色梯度为预先调整
    colorMap->rescaleDataRange();//重新调整数值维度保证所有数据都存在于颜色梯度垂直条中
    //保证颜色梯度和坐标轴同步他们的底部和顶部边界
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->customPlot);
    ui->customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    // 重绘
    ui->customPlot->rescaleAxes();

    QFile indata("draw/concentration rectangle.txt");
    if(!indata.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::information(this,tr("严重错误"),tr("丢失数据文件(concentration rectangle.txt)，这将导致绘图功能失效。"));
        return;
    }
    QTextStream  inda(&indata);
    int i,j;
    while(1)
    {
        inda>>i>>j;
        num[i]=j;
        if(i==-1)
            break;
        for(int k=0;k<j;k++)
            inda>>node[i][k];
    }
}

void cuiconrect::plot(const float data[])
{
    for(int i=199;i>0;i--)
        for(int j=0;j<20;j++)
            plotd[i][j]=plotd[i-1][j];
    float z;
    for(int j=0;j<ny;j++ )
    {
        z=0;
        for(int k=0;k<num[j];k++)
            z+=data[node[j][k]];
        z=z/num[j];
        plotd[0][j]=z;
    }
    for(int i=0;i<nx;i++)
        for(int j=0;j<ny;j++)
            colorMap->data()->setCell(i,j,plotd[i][j]);

    // 重绘
    ui->customPlot->replot();
}
void cuiconrect::initt()
{
    for(int i=0;i<nx;i++)
        for(int j=0;j<ny;j++)
        {
            plotd[i][j]=j/20.0;
            colorMap->data()->setCell(i,j,plotd[i][j]);
        }
    ui->customPlot->replot();
}

cuiconrect::~cuiconrect()
{
    delete ui;
}
void cuiconrect::closeEvent(QCloseEvent *event)
{
    emit HideIt();
    event->ignore();
}

void cuiconrect::on_pushBut_Screenshot_clicked()
{
    emit ScreenShot();
}
bool cuiconrect::ScreenShotPic(QString a)
{
    return ui->customPlot->saveBmp(a);
}

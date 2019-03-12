#include "cuiconcir.h"
#include <ui_concir.h>
cuiconcir::cuiconcir(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::concir)
{
    ui->setupUi(this);
    t=0;//旋转角度初始化
    //配置坐标区域
    ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom);
    //这个函数允许通过QCP::iRangeDrag（拖放）和iRangeZoom（缩放）来重新调节颜色范围
    ui->customPlot->axisRect()->setupFullAxesBox(true);
    //设置颜色图
    colorMap = new QCPColorMap(ui->customPlot->xAxis, ui->customPlot->yAxis);
    nx = 200;
    ny = 200;
    colorMap->data()->setSize(nx, ny); //我们希望颜色图有nx*ny个数据点
    colorMap->data()->setRange(QCPRange(-5, 5), QCPRange(-5, 5)); //这两个值得坐标范围
    //现在我们分配一些数据，通过访问色彩映射的QCPColorMapData实例：
    double x, y, z;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        if(x*x+y*y>=25)
            z=0.4;
        else
        {
            z=(x*x+y*y)/25;
        }
            colorMap->data()->setCell(xIndex, yIndex, z);
      }
    }
    ui->customPlot->xAxis->setTickLabels(0);
    ui->customPlot->xAxis->setTicks(0);
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

    QFile indata("draw/concentration circle.txt");
    if(!indata.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::information(this,tr("严重错误"),tr("丢失数据文件(concentration circle.txt)，这将导致绘图功能失效。"));
        return;
    }
    QTextStream  inda(&indata);
    int i=0,j;
    while (1)
    {
        inda>>i>>j;
        if (i==-1)
            break;
        for(int k=0;k<6;k++)
        {
            inda>>node[i][j][k];
            inda>>areph[i][j][k];
        }
    }

}
void cuiconcir::plot(const float data[])
{
    //采用最邻近插值的反向映射法
    float aph,z;
    int i1,j1;
    aph=t*3.1416/18.0;
    for (int i=1; i<=nx; i++)
      for (int j=1; j<=ny; j++)
      {
          if(((i-100)*(i-100)+(j-100)*(j-100))<10000)
          {
              z=0;
              i1=int((i-100)*qCos(aph)-(j-100)*qSin(aph)+100);
              if (i1>200)i1=200;
              if (i1<1)  i1=1;
              j1=int((i-100)*qSin(aph)+(j-100)*qCos(aph)+100);
              if (j1>200)j1=200;
              if (j1<1)  j1=1;
              for(int k=0;k<6;k++)
                  z+=data[node[i1][j1][k]]*areph[i1][j1][k];
          }
          else if(((i-100)*(i-100)+(j-100)*(j-100))>10150)
          {
              z=0.4F;
          }
          else
              z=0.9F;
          colorMap->data()->setCell(i-1,j-1,z);
      }
    // 重绘
    ui->customPlot->replot();
}
cuiconcir::~cuiconcir()
{
    delete ui;
}
void cuiconcir::closeEvent(QCloseEvent *event)
{
    emit HideIt();
    event->ignore();
}
void cuiconcir::on_dial_sliderMoved(int position)
{
    t=position;
    emit change();
}
void cuiconcir::on_pushBut_SreenShot_clicked()
{
    emit ScreenShot();
}
bool cuiconcir::ScreenShotPic(QString a)
{
    return ui->customPlot->saveBmp(a);
}

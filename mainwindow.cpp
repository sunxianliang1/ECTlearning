#include "mainwindow.h"
#include "ui_mainwindow.h"

/*************************************
 * ECT成像系统
 * 作者：孙先亮
 *************************************
 * 2016.11.21更新：增加时间显示，增益按钮
 * 电容计算增加最小二乘法
 * 2016.12.06
 * 删除最小二乘法，增加空满管判断机制，消除8个点
 * 2016.12.15
 * 增加回放帧率调节
 * 增加空满管判定成功后直接空管
 * 2016.12.16
 * 增加浓度分布图管壁
 * 浓度数据上限改为1.5
 * 2016.12.18
 * 增加帮助文件“help.docx”的打开
 * 2016.12.24
 * 暂时删除增益调节
 * 增加保存文件设置
 * 文件读写构建类savefile
 * 2016.12.26
 * 增加文件夹路径显示
 * 2017.1.6
 * 界面统计图优化，改为3行平均
 * 浓度曲线改为面积加权
 * 2017.2.9
 * 大幅度更改变量命名，使之更清晰
 * int整型  flt浮点  cui界面类  ui界面变量  mdi中央界面
 * cap电容  con浓度  commun通讯 txt文本  dat二进制
 * data数据 line曲线 cir圆形截面 rect矩形截面 colu柱状图
 * jpg图像
 * 20117.2.15
 * 增加截图功能
 * 截图必须用相对路径。
 * 文件名不能包含/\?:"<>|*
 * 2017.7.14
 * 矩形截面改为20行，切位同心圆
 * 2017.12.11
 * 增加实时标定功能
 * 2017.12.27
 * 改为12电极
 * 2018.1.2
 * 自动计算灵敏场特征值
 * 2018.1.3
 * 适应电极数，只需在源码option.h中更改注释选项
 * 2018.3.2
 * 资源文件移至源文件目录
 * 2018.4.4
 * 修复了一个bug，该bug导致读取文件时若遇到两个相同的时间自动停止读取
 * 修改后使用atEnd()函数判断文件结尾
 * 2019.3.12
 * 读标定版本
 ****/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    ui->setupUi(this);
    //电容数据曲线窗口初始化
    uicapcolu=new cuicapcolu();
    mdicapcolu=ui->mdiArea->addSubWindow(uicapcolu);
    mdicapcolu->hide();
    connect(uicapcolu,SIGNAL(HideIt()),this,SLOT(uicapcoluhide()));

    //浓度曲线窗口初始化
    uiconline=new cuiconline();
    mdiconline=ui->mdiArea->addSubWindow(uiconline);
    mdiconline->hide();
    connect(uiconline,SIGNAL(HideIt()),this,SLOT(uiconlinehide()));

    //圆形界面浓度分布窗口初始化
    uiconcir=new cuiconcir();
    mdiconcir=ui->mdiArea->addSubWindow(uiconcir);
    mdiconcir->show();
    connect(uiconcir,SIGNAL(HideIt()),this,SLOT(uiconcirhide()));
    connect(uiconcir,SIGNAL(change()),this,SLOT(plotconcircle()));
    connect(uiconcir,SIGNAL(ScreenShot()),this,SLOT(CirSreenShot()));
    //矩形截面浓度分布窗口初始化
    uiconrect=new cuiconrect();
    mdiconrect=ui->mdiArea->addSubWindow(uiconrect);
    mdiconrect->hide();
    connect(uiconrect,SIGNAL(HideIt()),this,SLOT(uiconrecthide()));
    connect(uiconrect,SIGNAL(ScreenShot()),this,SLOT(RectSreenShot()));
    //USB窗口初始化
    uicommun=new communication();
    connect(uicommun,SIGNAL(calibration()),this,SLOT(re_calibration()));

    //运行标志初始化
    runningflag=false;
    ccway=bynull;
    pause=false;
    playertime=1000;


    //文件保存位置初始化
    root_save_directory="./data";

    fltcapdata_dat.run=1;
    fltcapdata_txt.run=0;
    intcapdata_txt.run=1;
    fltcondata_txt.run=0;
    fltconcir_txt.run=0;
    fltconrect_txt.run=0;

    ui->statusBar->showMessage(tr("等待中..."));
}
void MainWindow::plotconcircle()//浓度分布图像重绘
{
    uiconcir->plot(fltcondata);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_action_set_commun_triggered()//打开设置串口界面
{
    uicommun->show();
}
void MainWindow::initialization()//两个绘图界面初始化
{
    uiconrect->initt();
    uiconline->initt();
}
void MainWindow::set_k_of_calibration()//标定完成后计算增益倍数
{
    float a[N],amax=0;
    float b[N];
    int k=0;
    for (int i=0;i<N;i++)
    {
        a[i]=uicommun->intcapfull[i]-uicommun->intcapempty[i];
        b[i]=comdata.fltcapfull[i]-comdata.fltcapempty[i];
        if(a[i]>amax)
        {
            amax=a[i];
            k=i;
        }
    }
    k_of_calibration=a[k]/b[k];
}
void MainWindow::on_action_start_commun_triggered()//开始测量按钮
{
    if(ccway==bynull&&runningflag==0)
    {
        if(!uicommun->startreadflag)
        {
            QMessageBox::warning(this,tr("警告"),tr("下位机未正确设置！"));
            return;
        }
        else
        {
            runningflag=1;
            ccway=byreal;
            initialization();
            time_datetime = QDateTime::currentDateTime();
            str_datetime =time_datetime.toString("yyyy-MM-dd_hh-mm-ss.zzz");
            now_save_directory=root_save_directory+"/"+str_datetime;//设定保存位置为“xxx/时间”
            showpath();//显示路径
            QDir dir;
            dir.mkpath(now_save_directory);
            open_file();
            MakeScreenShotPath();
            //标定数据
            set_k_of_calibration();
            calibration.strname=now_save_directory+"/"+"calibration.txt";
            calibration.filename=new QFile(calibration.strname);
            calibration.filename->open(QIODevice::ReadWrite|QIODevice::Text);
            calibration.text.setDevice(calibration.filename);
            calibration.text<<"calibration of empty:"<<endl;
            for (int i=0;i<N;i++)
                calibration.text<<uicommun->intcapempty[i]<<"  ";
            calibration.text<<endl;
            calibration.text<<"calibration of full:"<<endl;
            for (int i=0;i<N;i++)
                calibration.text<<uicommun->intcapfull[i]<<"  ";
            calibration.text<<endl;
            calibration.filename->close();

            QMessageBox::information(this,tr("测量"),tr("测量开始！"));
            ui->statusBar->showMessage(tr("实时测量中..."));

            connect(uicommun,SIGNAL(received()),this,SLOT(read_data_in_seri()));
            connect(uicommun,SIGNAL(received()),this,SLOT(transIntToFlt()));
            connect(uicommun,SIGNAL(received()),this,SLOT(plotevery()));
            connect(uicommun,SIGNAL(received()),this,SLOT(save_data_in_file()));
        }
    }
}
void MainWindow::open_file()
{
    //电容数据.dat
    fltcapdata_dat.strname=now_save_directory+"/"+"capacitance data float.dat";
    fltcapdata_dat.filename=new QFile(fltcapdata_dat.strname);
    fltcapdata_dat.filename->open(QIODevice::ReadWrite);
    fltcapdata_dat.dat.setDevice(fltcapdata_dat.filename);
    //电容原始数据
    intcapdata_txt.strname=now_save_directory+"/"+"capacitance data int.txt";
    intcapdata_txt.filename=new QFile(intcapdata_txt.strname);
    intcapdata_txt.filename->open(QIODevice::ReadWrite|QIODevice::Text);
    intcapdata_txt.text.setDevice(intcapdata_txt.filename);
    //电容数据.txt
    fltcapdata_txt.strname=now_save_directory+"/"+"capacitance data float.txt";
    fltcapdata_txt.filename=new QFile(fltcapdata_txt.strname);
    fltcapdata_txt.filename->open(QIODevice::ReadWrite|QIODevice::Text);
    fltcapdata_txt.text.setDevice(fltcapdata_txt.filename);
    //浓度数据
    fltcondata_txt.strname=now_save_directory+"/"+"concentration data.txt";
    fltcondata_txt.filename=new QFile(fltcondata_txt.strname);
    fltcondata_txt.filename->open(QIODevice::ReadWrite|QIODevice::Text);
    fltcondata_txt.text.setDevice(fltcondata_txt.filename);
    //圆形截面浓度
    fltconcir_txt.strname=now_save_directory+"/"+"concentration circle.txt";
    fltconcir_txt.filename=new QFile(fltconcir_txt.strname);
    fltconcir_txt.filename->open(QIODevice::ReadWrite|QIODevice::Text);
    fltconcir_txt.text.setDevice(fltconcir_txt.filename);
    //矩形截面浓度
    fltconrect_txt.strname=now_save_directory+"/"+"concentration rectangle.txt";
    fltconrect_txt.filename=new QFile(fltconrect_txt.strname);
    fltconrect_txt.filename->open(QIODevice::ReadWrite|QIODevice::Text);
    fltconrect_txt.text.setDevice(fltconrect_txt.filename);

}
void MainWindow::on_action_stop_commun_triggered()//停止测量
{
    if(ccway==byreal&&runningflag)
    {
        fltcapdata_dat.filename->close();
        fltcapdata_txt.filename->close();
        fltcondata_txt.filename->close();
        fltconcir_txt.filename->close();
        fltconrect_txt.filename->close();

        disconnect(uicommun,SIGNAL(received()),this,SLOT(read_data_in_seri()));
        disconnect(uicommun,SIGNAL(received()),this,SLOT(transIntToFlt()));
        disconnect(uicommun,SIGNAL(received()),this,SLOT(plotevery()));
        disconnect(uicommun,SIGNAL(received()),this,SLOT(save_data_in_file()));
        runningflag=0;
        ccway=bynull;
        QMessageBox::information(this,tr("测量"),tr("测量结束！"));
        ui->statusBar->showMessage(tr("等待中..."));
    }
}

void MainWindow::read_data_in_seri()//电容数据读取计算
{
    time_datetime = QDateTime::currentDateTime();
    str_datetime =time_datetime.toString("yyyy.MM.dd-hh:mm:ss.zzz");
    for (int i=0;i<N;i++)//空管基准
    {
        fltcapdata[i]=(uicommun->intcapdata[i]-uicommun->intcapempty[i])/k_of_calibration+comdata.fltcapempty[i];
    }
    float k[N];
    for (int i=0;i<N;i++)
    {
        k[i]=1.0*(uicommun->intcapfull[i]- uicommun->intcapdata[i])/(uicommun->intcapfull[i]-uicommun->intcapempty[i]);
    }
    if((k[0]<0.5)&&(k[14]<0.5))
    {
        if((k[20]<0.5)&&(k[10]<0.5))
        for (int i=0;i<N;i++)//满管基准
        {
            fltcapdata[i]=comdata.fltcapfull[i]-(uicommun->intcapfull[i]- uicommun->intcapdata[i])/k_of_calibration;
        }
    }
}

void MainWindow::re_calibration()
{
    on_action_stop_commun_triggered();
    on_action_start_commun_triggered();
}
void MainWindow::transIntToFlt()//数据计算
{
    //浓度分布计算
    float a=0,b=0;
    for(int i=0;i<N;i++)
    {
        a+=(fltcapdata[i]-comdata.fltcapempty[i])*(fltcapdata[i]-comdata.fltcapempty[i]);
        b+=(fltcapdata[i]-comdata.fltcapfull[i])*(fltcapdata[i]-comdata.fltcapfull[i]);
    }
    if(a<0.0008)
    {
        for(int i=0;i<N;i++)
            fltcapdata[i]=comdata.fltcapempty[i];
    }
    else if(b<0.0008)
    {
        for(int i=0;i<N;i++)
            fltcapdata[i]=comdata.fltcapfull[i];
    }
    comdata.set(fltcapdata);
    comdata.comp();
    for(int i=1;i<M+1;i++)
        fltcondata[i]=comdata.gk[i-1];

}
void MainWindow::plotevery()//绘图
{
   uicapcolu->plot(fltcapdata);
   uiconline->plot(fltcondata);
   uiconcir ->plot(fltcondata);
   uiconrect->plot(fltcondata);
   showtime();
}
void MainWindow::on_action_save_location_triggered()//保存位置
{
    root_save_directory=QFileDialog::getExistingDirectory(this,tr("保存位置"),tr("./"),QFileDialog::ShowDirsOnly);
    if (root_save_directory=="")
        root_save_directory="./data";
}
void MainWindow::save_data_in_file()//保存到文件
{
    if(fltcapdata_dat.run&&ccway==byreal)//保存电容数据（标定后）.dat
    {
        fltcapdata_dat.dat<<time_datetime;
        for (int i=0;i<N;i++)
            fltcapdata_dat.dat<<fltcapdata[i];
    }
    if(fltcapdata_txt.run)//保存电容数据（标定后）.txt
    {
        fltcapdata_txt.text<<str_datetime<<"    ";
        for(int i=0;i<N;i++)
            fltcapdata_txt.text<<fltcapdata[i]<<"  ";
        fltcapdata_txt.text<<endl;
    }
    if(intcapdata_txt.run&&ccway==byreal)//保存电容原始数据（标定前）.txt
    {
        intcapdata_txt.text<<str_datetime<<"    ";
        for(int i=0;i<N;i++)
            intcapdata_txt.text<<uicommun->intcapdata[i]<<"  ";
        intcapdata_txt.text<<endl;
    }
    if(fltcondata_txt.run)//保存浓度数据
        fltcondata_txt.text<<str_datetime<<"    "<<uiconline->y[0]<<endl;
    if(fltconrect_txt.run)//保存浓度分布
    {
        fltconrect_txt.text<<str_datetime<<"    ";
        for(int i=0;i<M+1;i++)
            fltconrect_txt.text<<fltcondata[i]<<"  ";
        fltconrect_txt.text<<endl;
    }
    if(fltconcir_txt.run)
    {
        fltconcir_txt.text<<str_datetime<<"    ";
        for(int i=0;i<50;i++)
            fltconcir_txt.text<<uiconrect->plotd[0][i]<<"    ";
        fltconcir_txt.text<<endl;
    }
}
void MainWindow::on_action_open_file_triggered()//打开文件
{
    if(runningflag)
        QMessageBox::warning(this,tr("警告"),tr("已经在测量了"));
    if(ccway==bynull)
    {
        uicommun->on_pushBut_ReadCalibrition_clicked();
        set_k_of_calibration();
        read_cdata_dat.strname=QFileDialog::getOpenFileName(this,tr("打开历史数据"),root_save_directory,tr("数据文件(*txt)"));
        read_cdata_dat.filename=new QFile(read_cdata_dat.strname);
        if(!read_cdata_dat.filename->open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this,tr("警告"),tr("文件打开失败！"));
            return;
        }
        read_cdata_dat.text.setDevice(read_cdata_dat.filename);
        QFileInfo info(read_cdata_dat.strname);
        now_save_directory = info.absolutePath();
        open_file();
        showpath();
        MakeScreenShotPath();
        readtime=new  QTimer(this);
        ccway=byfile;
    }
}
void MainWindow::on_action_startfile_triggered()//开始从文件读取数据
{
    if(ccway==byfile&&runningflag==0)
    {
        if(pause==1)
        {
            readtime->start();
            pause=0;
            runningflag=1;
        }
        else
        {
            runningflag=1;
            readtime->start(playertime);
            initialization();
            connect(readtime,SIGNAL(timeout()),this,SLOT(read_data_in_file()));
            connect(readtime,SIGNAL(timeout()),this,SLOT(transIntToFlt()));
            connect(readtime,SIGNAL(timeout()),this,SLOT(plotevery()));
            connect(readtime,SIGNAL(timeout()),this,SLOT(save_data_in_file()));
            ui->statusBar->showMessage(tr("历史数据读取中..."));
        }
    }
}
void MainWindow::on_action_pausefile_triggered()//暂停读取数据
{
    if(ccway==byfile&&runningflag)
    {
        if (pause==0)
            readtime->stop();
        pause=1;
        runningflag=0;
    }
}
void MainWindow::on_action_stopfile_triggered()//停止读取
{
    if(ccway==byfile)
    {
        read_cdata_dat.filename->close();
        runningflag=0;
        ccway=bynull;
        pause=0;
        disconnect(readtime,SIGNAL(timeout()),this,SLOT(read_data_in_file()));
        disconnect(readtime,SIGNAL(timeout()),this,SLOT(transIntToFlt()));
        disconnect(readtime,SIGNAL(timeout()),this,SLOT(plotevery()));
        disconnect(readtime,SIGNAL(timeout()),this,SLOT(save_data_in_file()));
        delete readtime;
        ui->statusBar->showMessage(tr("等待中..."));
    }
}
void MainWindow::read_data_in_file()//从文件读取
{
    if(read_cdata_dat.dat.atEnd())
    {
        on_action_stopfile_triggered();
        return;
    }

//    read_cdata_dat.dat>>time_datetime;
//    str_datetime=time_datetime.toString("yyyy.MM.dd-hh:mm:ss.zzz");
//    for (int i=0;i<N;i++)
//    {
//        read_cdata_dat.dat>>fltcapdata[i];
//    }
    QString a;
    read_cdata_dat.text>>a;
    time_datetime=QDateTime::fromString(a,"yyyy.MM.dd-hh:mm:ss.zzz");
    str_datetime=a;
    int cap[28];
    for (int i=0;i<28;i++)
    {
        read_cdata_dat.text>>cap[i];
        fltcapdata[i]=(cap[i]-uicommun->intcapempty[i])/k_of_calibration+comdata.fltcapempty[i];
    }

}
void MainWindow::showtime()
{
    QString a;
    a=str_datetime.left(10);
    ui->textEdit_timeshow->setText(a);
    a=str_datetime.right(12);
    ui->textEdit_timeshow->append(a);
}
void MainWindow::showpath()
{
    ui->textEdit_filepath->setText(now_save_directory);
}

void MainWindow::on_dial_player_sliderMoved(int position)//回放速度
{
    playertime=(1000/position);
    if(ccway==byfile&&runningflag)
        readtime->start(playertime);
}
void MainWindow::MakeScreenShotPath()
{
    QString  filepath;
    filepath=now_save_directory+"/ScreenShot/Circle";
    QDir dir;
    dir.mkpath(filepath);
    filepath=now_save_directory+"/ScreenShot/Rectangle";
    dir.mkpath(filepath);
}

void MainWindow::CirSreenShot()//圆形截面截图
{
    QString  filepath;
    filepath=now_save_directory+"/ScreenShot/Circle";
    QString a=time_datetime.toString("yyyy-MM-dd_hh-mm-ss.zzz");
    filepath=filepath+"/"+a+"C.bmp";
    QDir dir(".");
    filepath=dir.relativeFilePath(filepath);//转为相对路径
    filepath="./"+filepath;
    //文件名不能包含/\?:"<>|*
    uiconcir->ScreenShotPic(filepath);
}
void MainWindow::RectSreenShot()//矩形截面截图
{
    QString  filepath;
    filepath=now_save_directory+"/ScreenShot/Rectangle";
    QString a=time_datetime.toString("yyyy-MM-dd_hh-mm-ss.zzz");
    filepath=filepath+"/"+a+"R.bmp";
    QDir dir(".");
    filepath=dir.relativeFilePath(filepath);
    filepath="./"+filepath;
    uiconrect->ScreenShotPic(filepath);
}

//以下为四个界面的打开按钮实现以及关闭按钮的重新实现
void MainWindow::on_action_capcolu_triggered()
{
    mdicapcolu->show();
}
void MainWindow::uicapcoluhide()
{
    mdicapcolu->hide();
}
void MainWindow::on_action_conline_triggered()
{
    mdiconline->show();
}
void MainWindow::uiconlinehide()
{
    mdiconline->hide();
}
void MainWindow::on_action_concir_triggered()
{
    mdiconcir->show();
}
void MainWindow::uiconcirhide()
{
    mdiconcir->hide();
}
void MainWindow::on_action_conrect_triggered()
{
    mdiconrect->show();
}
void MainWindow::uiconrecthide()
{
    mdiconrect->hide();
}

void MainWindow::closeEvent(QCloseEvent *event)//重写窗口关闭事件
{
    int a;
    a=QMessageBox::question(this,tr("关闭软件"),tr("你确定要退出吗？"),QMessageBox::Yes,QMessageBox::No);
    if(a==QMessageBox::No)
    {
        event->ignore();
    }
    else
    {
        uicommun->close();//将cc的父窗口设为出窗口，这样主窗口关闭时cc同时关闭
        event->accept();
    }
}
void MainWindow::on_action_exit_triggered()//退出按钮
{
    this->close();
}
void MainWindow::on_action_about_triggered()//关于按钮
{
    QMessageBox::about(this,tr("关于"),tr("ECT系统\n版本号：1.0Beta"));
}
void MainWindow::on_action_help_triggered()//帮助按钮
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo("help.docx").absoluteFilePath()));
}
//以下为设置是否保存文件
void MainWindow::on_action_fltcapdata_dat_toggled(bool arg1)
{
    fltcapdata_dat.run=arg1;
}
void MainWindow::on_action_fltcapdata_txt_toggled(bool arg1)
{
    fltcapdata_txt.run=arg1;
}
void MainWindow::on_action_fltcondata_txt_toggled(bool arg1)
{
    fltcondata_txt.run=arg1;
}
void MainWindow::on_action_fltconline_txt_toggled(bool arg1)
{
    fltconcir_txt.run=arg1;
}
void MainWindow::on_action_fltconcir_txt_toggled(bool arg1)
{
    fltconrect_txt.run=arg1;
}
void MainWindow::on_action_intcapdata_txt_toggled(bool arg1)
{
    intcapdata_txt.run=arg1;
}

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include"communication.h"
#include"cuicapcolu.h"
#include"cuiconline.h"
#include"cuiconcir.h"
#include"cuiconrect.h"
#include"compute.h"
#include"option.h"

#include<qmath.h>
#include<QMainWindow>
#include<QFileDialog>
#include<QDir>
#include<QFile>
#include<QFileInfo>
#include<QDateTime>
#include<QDataStream>
#include<QTextStream>
#include<QVector>
#include<QLabel>
#include <QDesktopServices>
#include <QUrl>


namespace Ui {
class MainWindow;
}

struct  savefile
{
    QFile*        filename;
    QString       strname;
    QDataStream   dat;
    QTextStream   text;
    bool          run;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void closeEvent(QCloseEvent *event);
    void showpath();
    void initialization();
    void open_file();
    void MakeScreenShotPath();//创建截图保存文件夹
public slots:
    void  read_data_in_seri();
    void  re_calibration();
    void  transIntToFlt();
    void  plotevery();
    void  plotconcircle();
    void  save_data_in_file();
    void  read_data_in_file();
    void  set_k_of_calibration();
    void  showtime();
    void  CirSreenShot();//圆形截面截图
    void  RectSreenShot();//矩形截面截图

private slots:
    void on_action_set_commun_triggered();
    void on_action_start_commun_triggered();
    void on_action_stop_commun_triggered();
    void on_action_save_location_triggered();
    void on_action_open_file_triggered();
    void on_action_startfile_triggered();
    void on_action_pausefile_triggered();
    void on_action_stopfile_triggered();
    //界面开关
    void on_action_capcolu_triggered();
    void on_action_conline_triggered();
    void on_action_concir_triggered();
    void on_action_conrect_triggered();
    //界面关闭
    void uicapcoluhide();
    void uiconlinehide();
    void uiconcirhide();
    void uiconrecthide();
    void on_action_exit_triggered();
    void on_action_about_triggered();
    void on_action_help_triggered();
    void on_dial_player_sliderMoved(int position);
    //保存文件设置
    void on_action_fltcapdata_dat_toggled(bool arg1);
    void on_action_fltcapdata_txt_toggled(bool arg1);
    void on_action_intcapdata_txt_toggled(bool arg1);
    void on_action_fltcondata_txt_toggled(bool arg1);
    void on_action_fltconline_txt_toggled(bool arg1);
    void on_action_fltconcir_txt_toggled(bool arg1);


private:
    Ui::MainWindow   *ui;
    //子界面
    communication    *uicommun;//通讯设置窗口
    cuicapcolu       *uicapcolu;//电容数据窗口
    cuiconline       *uiconline;//浓度曲线窗口
    cuiconcir        *uiconcir;//圆形形截面浓度窗口
    cuiconrect       *uiconrect;//浓度截面窗口
    QMdiSubWindow    *mdicapcolu;
    QMdiSubWindow    *mdiconline;
    QMdiSubWindow    *mdiconcir;
    QMdiSubWindow    *mdiconrect;

    //数据
    compute  comdata;
    float k_of_calibration;//标定系数
    float b_of_calibration;
    float fltcapdata[N];//电容数据
    float fltcondata[M+1];//圆形形截面浓度数据
    //文件存储
    QString root_save_directory;//文件保存根目录
    QString now_save_directory;//当前文件保存目录
    //保存数据
    savefile fltcapdata_dat;//电容数据.dat
    savefile fltcapdata_txt;//电容数据.txt
    savefile intcapdata_txt;//电容原始数据
    savefile fltcondata_txt;//浓度数据
    savefile fltconcir_txt; //圆形截面浓度
    savefile fltconrect_txt;//矩形截面浓度
    savefile read_cdata_dat;//读取数据
    savefile calibration;   //标定数据
    savefile jpgconcir_jpg; //圆形截面截图
    savefile jpgconrect_jpg;//矩形截面截图
    QString   str_datetime;//系统时间(字符串)
    QDateTime time_datetime;//系统时间(时间类)
    QTimer    *readtime;//定义一个定时器
    //标志
    bool   runningflag;//运行标志（是否在测量）
    bool   pause;//暂停标志
    enum   way {byreal,byfile,bynull}  ccway;//运行模式

    float time;
    int   count;
    float fps;
    int   playertime;

};

#endif // MAINWINDOW_H

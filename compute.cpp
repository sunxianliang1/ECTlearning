#include "compute.h"

compute::compute()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setWindowTitle(QObject::tr("严重错误"));
    QString  FilePath="calibration";
    QString  sinior;
    sinior.setNum(electric);
    FilePath=   FilePath +sinior+'/';
    QFile in1(FilePath+"LMC.txt");
    if(!in1.open(QFile::ReadOnly|QFile::Text))
    {
        msgBox.setText(QObject::tr("丢失灵敏场数据(LMC.txt)!"));
        msgBox.exec();
        return;
    }
    QTextStream  inda(&in1);
    for(int j=0;j<M;j++)
        for(int i=0;i<N;i++)
            inda>>lmc[i][j];

    QFile in2(FilePath+"empty.txt");
    if(!in2.open(QFile::ReadOnly|QFile::Text))
    {
        msgBox.setText(QObject::tr("丢失空管电容数据(empty.txt)!"));
        msgBox.exec();
        return;
    }
    inda.setDevice(&in2);
    for(int i=0;i<N;i++)
        inda>>fltcapempty[i];

    QFile in3(FilePath+"efull.txt");
    if(!in3.open(QFile::ReadOnly|QFile::Text))
    {
        msgBox.setText(QObject::tr("丢失满管电容数据(efull.txt)!"));
        msgBox.exec();
        return;
    }
    inda.setDevice(&in3);
    for(int i=0;i<N;i++)
        inda>>fltcapfull[i];
    for(int i=0;i<N;i++)
        cmfm[i]=fltcapfull[i]-fltcapempty[i];


    //*****************************//
        float s1[M];
        float sa[N][M];
    /*  for i=1:M
            S1(i,1)=0;
            for t=1:N
            S1(i,1)=S1(i,1)+SH(t,i);%S1为S的各列累加和
            end
            for j=1:28
            Sl(j,i)=SH(j,i)/S1(i,1);%同列元素的累加和为1，适应于LBP算法中
            end
        end    *******/
        for(int i=0;i<M;i++)
        {
            s1[i]=0;
            for(int j=0;j<N;j++)
            {
                s1[i]+=lmc[j][i];
            }
            for(int j=0;j<N;j++)
            {
                sa[j][i]=lmc[j][i]/s1[i];
            }
        }
    //*****************************//
        float s2[N];
    /*  for i=1:28
            S2(i,1)=0;
            for t=1:ImNodeSize
                S2(i,1)=S2(i,1)+Sl(i,t);%S2为S的各行累加和
            end
            for j=1:ImNodeSize
                Sh(i,j)=Sl(i,j)/S2(i,1);%同行元素的累加和为1，适用于Landweber算法
            end
        end    */
        for(int i=0;i<N;i++)
        {
            s2[i]=0;
            for(int j=0;j<M;j++)
            {
                s2[i]+=sa[i][j];
            }
            for(int j=0;j<M;j++)
            {
                sb[i][j]=sa[i][j]/s2[i];
            }
        }
        //*****************************//
        /*  a1=Sh'*Sh;
    a2=eig(a1);
    d=max(a2);
    a=2/d;   */


        max=2/eigen();

       // max=31.6312695;//有机玻璃
        //max=5.9566;//水
}
void compute::set(float cm1[])
{
    for(int i=0;i<N;i++)
        cm[i]=cm1[i];
}

void compute::comp()
{
    //*****************************//
    float cmfz[N];
    float cmzz[N];
    /*  for i=1:28
    Cmfz(i,1)=Cm(i,1)-Cempty(i,1);
    Cmfm(i,1)=Cfull(i,1)-Cempty(i,1);
    Cmzz(i,1)=Cmfz(i,1)/Cmfm(i,1);
    end   */
    for(int i=0;i<N;i++)
    {
        cmfz[i]=cm[i]-fltcapempty[i];
        cmzz[i]=cmfz[i]/cmfm[i];
    }
    //*****************************//
    /*
    g=Sh'*Cmzz;
    gk=g;      */
    for(int i=0;i<M;i++)
    {
        gk[i]=0;
        for(int j=0;j<N;j++)
            gk[i]+=sb[j][i]*cmzz[j];
    }
    //*****************************//
    /*  for k=1:50
        gk1=gk-a*Sh'*(Sh*gk-Cmzz);
        for i=1:ImNodeSize
            if gk1(i)<0
                gk1(i)=0;
            elseif gk1(i)>1
                gk1(i)=1;
            end
        end
        gk=gk1;
        end  */
    float  linshi1[N];
    float  linshi2[M];
    float   gk1[M];
    for(int t=0;t<50;t++)
    {
        //sh*gk-Cmzz
        for (int i=0;i<N;i++)
        {
            linshi1[i]=0;
            for(int j=0;j<M;j++)
            {
                linshi1[i]+=sb[i][j]*gk[j];
            }
            linshi1[i]-=cmzz[i];
        }
        //a*Sh'*linshi1
        for(int i=0;i<M;i++)
        {
            linshi2[i]=0;
            for(int j=0;j<N;j++)
            {
                linshi2[i]+=sb[j][i]*linshi1[j];
            }
            linshi2[i]*=max;
        }
        //gk1=gk-linshi2
        for(int i=0;i<M;i++)
        {
            gk1[i]=gk[i]-linshi2[i];
            if (gk1[i]<0)gk1[i]=0;
            if (gk1[i]>1)gk1[i]=1;
            gk[i]=gk1[i];
        }
    }
    //*****************************//
}

float compute::eigen()
{
    float (*A)[M];
    A=new float [M][M];
    for(int i=0;i<M;i++)
    {
        for(int j=0;j<M;j++)
        {
            A[i][j]=0;
            for(int k=0;k<N;k++)
                A[i][j]+=sb[k][i]*sb[k][j];
        }
    }
    float v[M]={0};
    float u[M]={1};
    float m1=0,m2=0;
    float e=1e-10F,delta=1;
    while(delta>=e)
    {
        matrixx(A,u,v);
        m2=m1;
        m1=slove(v);
        for(int i=0;i<M;i++)
            u[i]=v[i]/m1;
        delta=abs(m1-m2);
    }
    return m1;
}

void compute::matrixx(float A[][M], float x[], float v[])
{
    for(int i=0;i<M;i++)
    {
        v[i]=0;
        for(int j=0;j<M;j++)
            v[i]+=A[i][j]*x[j];
    }
}

float compute::slove(float v[])
{
    float max=v[0];
    for(int i=0;i<M-1;i++)
        if (v[i]>max)
            max=v[i];
    return max;
}

compute::~compute()
{

}


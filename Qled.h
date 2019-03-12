#ifndef QCWINDCCATORLAMP_H
#define QCWINDCCATORLAMP_H

#include <QWidget>

class QColor;

class Q_DECL_EXPORT Qled: public QWidget
{
 Q_OBJECT
    Q_PROPERTY(int alarm READ isAlarm WRITE setAlarm);
	
public:    
    explicit Qled(QWidget *parent = 0);
    int isAlarm() const {return m_alarm;}
		       	           
public Q_SLOTS:
    void setAlarm(int);

protected:
	void paintEvent(QPaintEvent *); 
private:
    int m_alarm;
};

#endif

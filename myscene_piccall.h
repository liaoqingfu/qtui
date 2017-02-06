#ifndef MYSCENE_PIC_H  
#define MYSCENE_PIC_H  
  
#include "public.h"




class myscene_pic : public QGraphicsScene  
{  
	Q_OBJECT  
public:  
    explicit myscene_pic(class MyView * pmv , QObject *parent = 0);
	QLabel *label_time;
	QLabel *label_date;  
	QLabel *label_net_status;
	
protected:	

  
signals:  



public slots:

	void  bt_retCallClicked();

private:

    class MyView * pmv;
	QPushButton *bt_retCall;

        void widget_init();

		QLabel *label_back;
  
};	
  
#endif // MYSCENE_H  


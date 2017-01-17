#ifndef MYSCENE_PIC_H  
#define MYSCENE_PIC_H  
  
#include "public.h"




class myscene_pic : public QGraphicsScene  
{  
	Q_OBJECT  
public:  
    explicit myscene_pic(class MyView * pmv , QObject *parent = 0);
  
protected:	

  
signals:  



public slots:

	void  bt_retCallClicked();
//	void recv_slot();

private:

    class MyView * pmv;
	QPushButton *bt_retCall;

        void widget_init();
		void timerEvent( QTimerEvent *event );

		QLabel *label_time;
		QLabel *label_date;
		int m_nTimerId;

		QLabel *label_back;
  
};	
  
#endif // MYSCENE_H  


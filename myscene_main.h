#ifndef MYSCENE_MAIN_H  
#define MYSCENE_MAIN_H  
  
#include "public.h"




class myscene_main : public QGraphicsScene  
{  
	Q_OBJECT  
public:  
    explicit myscene_main(class MyView * pmv , QObject *parent = 0);
	QLabel *label_time;
	QLabel *label_date;  
protected:	
	void mousePressEvent(QGraphicsSceneMouseEvent *event);

  
  
signals:  



public slots:
	void  bt_leftCallClicked();
	void  bt_rightCallClicked( );
	bool verifyLocalTime(QNetworkReply *reply);
	
//	void recv_slot();

private:
	void changed(const QList<QRectF> &region);
    class MyView * pmv;

        void widget_init();
//		void timerEvent( QTimerEvent *event );
		int m_nTimerId;



        QPushButton *bt_leftCall;
		QPushButton *bt_rightCall;
		
		QLabel *label_back;


		QNetworkAccessManager manager;
		void verifyTime();

  
};	
  
#endif // MYSCENE_H  


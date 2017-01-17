#ifndef MYSCENE_LIST_H  
#define MYSCENE_LIST_H  
  
#include "public.h"




class myscene_list : public QGraphicsScene  
{  
		Q_OBJECT  
		public:  
			explicit myscene_list(class MyView * pmv , QObject *parent = 0);
		  
		protected:	
		
		  
		signals:  
		
		
		
		public slots:
		void  bt_retCallClicked();
			
		//	void recv_slot();
		
		private:
		QPushButton *bt_retCall;
		class MyView * pmv;
	
			void widget_init();
			void timerEvent( QTimerEvent *event );
	
			QLabel *label_time;
			QLabel *label_date;
			int m_nTimerId;
	
			QLabel *label_back;


  
};	
  
#endif // MYSCENE_H  


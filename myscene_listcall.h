#ifndef MYSCENE_LIST_H  
#define MYSCENE_LIST_H  
  
#include "public.h"

#define  LIST_MAX_COLUMN 	3
#define  LIST_MAX_ROW    	16

class myscene_list : public QGraphicsScene  
{  
	Q_OBJECT  
	public:  
		explicit myscene_list(class MyView * pmv , QObject *parent = 0);
		QLabel *label_time;
		QLabel *label_date;
		QLabel *label_net_status;
		
	protected:	
	
	  
	signals:  
	
	
	
	public slots:
	void  bt_retCallClicked();
	void bt_listcallClicked( int buttonID);	
	//	void recv_slot();
	
	private:

	class MyView * pmv;

	void widget_init();
	void set_list_content( int pageNum );
	
	QLabel *label_back;
	QPushButton *bt_listcall[LIST_MAX_COLUMN*LIST_MAX_ROW + 6];
	QButtonGroup *bg_listcall;

	int LIST_COLUMN ;
	int LIST_HEIGHT ;

	int curr_page_num;
	int max_page_num;
	int call_id_index;

  
};	
  
#endif // MYSCENE_H  


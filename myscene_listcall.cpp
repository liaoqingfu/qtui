#include "myscene_listcall.h"  
#include <stdio.h>
#include "video/main_api.h"
#include "myview.h" 


//#define  LIST_COLUMN 		cfg.display_col
#define  LIST_ROW    		8

#define LIST_START_X     150  
#define LIST_START_Y     SCREEN_HEIGHT 

#define LIST_END_X      (SCREEN_WID - 150) 

#define  LIST_WIDTH  		( (LIST_END_X - LIST_START_X) / (LIST_ROW ) )

extern ncs_cfg_t cfg;


void  myscene_list::widget_init()
{

	LIST_COLUMN = cfg.display_col;
	LIST_HEIGHT =( SCREEN_HEIGHT/ LIST_COLUMN );
	curr_page_num = 1;
	max_page_num = (cfg.list_count + LIST_COLUMN*LIST_ROW) / LIST_COLUMN*LIST_ROW;
	
	qDebug() << "myscene_list init max page" <<max_page_num; 

    QGraphicsProxyWidget * proxy ;

	label_back = new QLabel();
	label_back->setAttribute(Qt::WA_TranslucentBackground); 
    label_back->setPixmap(QPixmap(":/pic/main-other.bmp"));
	label_back->setGeometry(0,0,SCREEN_WID,SCREEN_HEIGHT);
	//label_back->setText("LIST CALL WINDOWS");
	this->addWidget(label_back);

	label_time = new QLabel("");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
	pe.setColor(QPalette::ButtonText, Qt::white); 
    label_time->setPalette(pe);
    label_time->setAttribute(Qt::WA_TranslucentBackground); 
    label_time->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE) );
    label_time->setGeometry(TIME_POSX,TIME_POSY,TIME_POSW,TIME_POSH);
    proxy = this->addWidget(label_time);
    proxy->setRotation(-90);

	
	label_date = new QLabel("");
	//pe.setColor(QPalette::WindowText,Qt::white);
	label_date->setPalette(pe);
	label_date->setAttribute(Qt::WA_TranslucentBackground); 
	label_date->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE) );
	label_date->setGeometry(DATE_POSX,DATE_POSY,DATE_POSW,DATE_POSH);
	proxy = this->addWidget(label_date);
	proxy->setRotation(-90);

	QPixmap pixmap;

	bg_listcall = new QButtonGroup;
    QString bg_pic;

     for(int i = 0; i < LIST_COLUMN*LIST_ROW ; i++){
        bt_listcall[i] = new QPushButton;

        int xpos =  LIST_START_X + ((int)(i/LIST_COLUMN))*LIST_WIDTH;
        int ypos =  LIST_START_Y - (i%LIST_COLUMN) * LIST_HEIGHT;

        bt_listcall[i]->setGeometry( xpos, ypos, LIST_HEIGHT,LIST_WIDTH);  //pixmap.width() ,pixmap.height());
        bt_listcall[i]->setAttribute(Qt::WA_TranslucentBackground); 
		
		bt_listcall[i]->setStyleSheet("QPushButton{"
                                "border:3px solid white;"  //宽度为3px的红色边框
                                "border-radius:5px}"); //边框角的弧度为8px
		bt_listcall[i]->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*1.5) );
		//bt_listcall[i]->setText(  cfg.list_name[i] );

        bg_listcall->addButton(bt_listcall[i],i);
        proxy = this->addWidget(bt_listcall[i]);
        proxy->setRotation(-90);

    }

	for(int i = 0; i < 6; i++){
		bt_listcall[i] = new QPushButton;

		int xpos =  LIST_END_X + ((int)(i/3))*LIST_WIDTH;
		int ypos =  LIST_START_Y - (i%3) * NUM_Y_HEIGHT;

		bt_listcall[i]->setGeometry( xpos, ypos, NUM_Y_HEIGHT,LIST_WIDTH);  //pixmap.width() ,pixmap.height());
        bt_listcall[i]->setAttribute(Qt::WA_TranslucentBackground); 
		
		bt_listcall[i]->setStyleSheet("QPushButton{"
                                "border:3px solid white;");  //宽度为3px的红色边框

		if( i < 3)
			bg_pic.sprintf(":/pic/list%i.bmp",i );
		else
			bg_pic.sprintf(":/pic/pic%i.bmp",i );
        pixmap.load( bg_pic );
		bt_listcall[i]->setIcon( pixmap );
		bt_listcall[i]->setIconSize( QSize( NUM_Y_HEIGHT,LIST_WIDTH));

		bg_listcall->addButton(bt_listcall[i],i);
        proxy = this->addWidget(bt_listcall[i]);
        proxy->setRotation(-90);
	}

	set_list_content ( curr_page_num ) ;
	connect( bg_listcall ,SIGNAL(buttonClicked(int)), this, SLOT(bt_listcallClicked(int)));

}

void myscene_list::set_list_content( int pageNum )
{


}
	

void myscene_list::bt_numcallClicked( int buttonID)
{
    static int reachMaxLen = 0;

    if ( buttonID < LIST_COLUMN*LIST_ROW ) {
        call_id_index  // = bt_listcall[buttonID]->text();
    }
	else if ( buttonID == LIST_COLUMN*LIST_ROW) //pre page
	{
       set_list_content ( curr_page_num > 1 ? --curr_page_num : curr_page_num ) ;
    }
	else if ( buttonID == LIST_COLUMN*LIST_ROW +1) //call
	{
		qDebug() << "call to" << str_numcall;
       if( pmv->scene_calling->startCall( list_call_id.toLatin1().data() ) > 0) {
			pmv->changeWindowType( WINDOW_TYPE_CALLING); 
			qDebug() << "CHANGE TO CALLING WINDOW" ;
	   	}
    }
    else if ( buttonID == LIST_COLUMN*LIST_ROW + 2) //next page
    {
	   set_list_content ( curr_page_num < max_page_num ? ++curr_page_num : curr_page_num ) ;
    }
           
}
	

myscene_list::myscene_list(MyView * pm,QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();
	pmv = pm;

    widget_init();

}  
	
void  myscene_list::bt_retCallClicked()
{
	pmv->changeWindowType( pmv->topWindowType);
}





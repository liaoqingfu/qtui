#include "myscene_listcall.h"  
#include <stdio.h>
#include "video/main_api.h"
#include "myview.h" 


//#define  LIST_COLUMN 		cfg.display_col
#define  LIST_ROW    		8

#define LIST_START_X     150  
#define LIST_START_Y     SCREEN_HEIGHT 

#define  LIST_WIDTH  		( (SCREEN_WID - LIST_START_X) / (LIST_ROW +2) )

extern ncs_cfg_t cfg;
extern int list_call_id;  //ºô½Ðid

void  myscene_list::widget_init()
{

	LIST_COLUMN = cfg.display_col;
	LIST_HEIGHT =( SCREEN_HEIGHT/ LIST_COLUMN );
	curr_page_num = 1;
	max_page_num = (cfg.list_count + LIST_COLUMN*LIST_ROW) / (LIST_COLUMN*LIST_ROW);
	
	qDebug() << "myscene_list init page" <<max_page_num << "COLUMN" << LIST_COLUMN; 

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

	label_net_status = new QLabel();
    label_net_status->setAttribute(Qt::WA_TranslucentBackground);
    label_net_status->setPixmap(QPixmap(":/pic/offline.bmp"));
    label_net_status->move(15,50);
    proxy = this->addWidget(label_net_status);
    proxy->setRotation(-90);
	
	label_date = new QLabel("");
	//pe.setColor(QPalette::WindowText,Qt::white);
	label_date->setPalette(pe);
	label_date->setAttribute(Qt::WA_TranslucentBackground); 
	label_date->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE) );
	label_date->setGeometry(DATE_POSX,DATE_POSY,DATE_POSW,DATE_POSH);
	proxy = this->addWidget(label_date);
	proxy->setRotation(-90);

	label_listcall = new QLabel();
	label_listcall->setPalette(pe);
	label_listcall->setAttribute(Qt::WA_TranslucentBackground);
	label_listcall->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*2) );
	label_listcall->setGeometry(60,450,400,120);
	proxy = this->addWidget(label_listcall);
	proxy->setRotation(-90);
		

	QPixmap pixmap;

	bg_listcall = new QButtonGroup;
    QString bg_pic;

     for(int i = 0; i < LIST_COLUMN*LIST_ROW + 6; i++){
	 	int xpos ,ypos;
        bt_listcall[i] = new QPushButton;
		if( i < LIST_COLUMN*LIST_ROW ){
	        xpos =  LIST_START_X + ((int)(i/LIST_COLUMN))*LIST_WIDTH;
	        ypos =  LIST_START_Y - (i%LIST_COLUMN) * LIST_HEIGHT;

	        bt_listcall[i]->setGeometry( xpos, ypos, LIST_HEIGHT,LIST_WIDTH); 
			bt_listcall[i]->setAttribute(Qt::WA_TranslucentBackground); 
		
			bt_listcall[i]->setStyleSheet("QPushButton{"  "border:3px solid white; text-align: left;}"); 
			
			bt_listcall[i]->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*1.5) );
		}
		else{
			int j = i - LIST_COLUMN*LIST_ROW ;
			xpos =  LIST_START_X + LIST_ROW*LIST_WIDTH + ((int)( j/3))*LIST_WIDTH;
			ypos =  LIST_START_Y - (j % 3) * NUM_Y_HEIGHT;
			bt_listcall[i]->setGeometry( xpos, ypos, NUM_Y_HEIGHT,LIST_WIDTH);
			if( j < 3 )
				bg_pic.sprintf(":/pic/list%i.bmp",j );
			else
				bg_pic.sprintf(":/pic/pic%i.bmp",j );
			qDebug() << bg_pic;
	        pixmap.load( bg_pic );
			bt_listcall[i]->setIcon( pixmap );
			bt_listcall[i]->setIconSize( QSize( NUM_Y_HEIGHT,LIST_WIDTH));
		}
        

        bg_listcall->addButton(bt_listcall[i],i);
        proxy = this->addWidget(bt_listcall[i]);
        proxy->setRotation(-90);

    }

	set_list_content ( curr_page_num ) ;
	connect( bg_listcall ,SIGNAL(buttonClicked(int)), this, SLOT(bt_listcallClicked(int)));

}

void myscene_list::set_list_content( int pageNum )
{
	for(int i = 0; i < LIST_COLUMN*LIST_ROW ; i++){
		bt_listcall[i]->setText(  cfg.list_name[ (pageNum - 1) * LIST_COLUMN*LIST_ROW + i] );
	}

}
	

void myscene_list::bt_listcallClicked( int buttonID)
{
    static int reachMaxLen = 0;
	qDebug() << "buttonID" << buttonID ;

    if ( buttonID < LIST_COLUMN*LIST_ROW ) {
        call_id_index  = (curr_page_num - 1) * LIST_COLUMN*LIST_ROW + buttonID ;
		label_listcall->setText( cfg.list_target[call_id_index] );
    }
	else if ( buttonID == LIST_COLUMN*LIST_ROW) //pre page
	{
       set_list_content ( curr_page_num > 1 ? --curr_page_num : curr_page_num ) ;
    }
	else if ( buttonID == LIST_COLUMN*LIST_ROW +1) //call
	{
		list_call_id = call_id_index;
		qDebug() << "call to" << cfg.list_target[list_call_id];
		pmv->changeWindowType( WINDOW_TYPE_CALLING); 
       if( pmv->scene_calling->startCall( cfg.list_target[list_call_id].toLatin1().data() , TS_SIP_TALK) > 0) {
			if(cfg.list_link[list_call_id])  { //listcall alarm enable
				gpio_set( GPO_SHORT_ALARM,	 cfg.short_o1_normal_mode ? 0 : 1);
				printf_log(LOG_IS_INFO, " 		---short alarm out start--   \n");
			}
	   	}
	   else
	   		pmv->changeWindowType( WINDOW_TYPE_LIST_CALL); 
    }
    else if ( buttonID == LIST_COLUMN*LIST_ROW + 2) //next page
    {
	   set_list_content ( curr_page_num < max_page_num ? ++curr_page_num : curr_page_num ) ;
    }
	else if ( buttonID < LIST_COLUMN*LIST_ROW + 6) {  //return to main window
	   pmv->changeWindowType( buttonID - LIST_COLUMN*LIST_ROW - 2 ) ; 
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





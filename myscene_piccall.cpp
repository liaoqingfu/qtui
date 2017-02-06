#include "myscene_piccall.h"  
#include <stdio.h>
#include "video/main_api.h"
#include "myview.h" 


void  myscene_pic::widget_init()
{
	qDebug() << "myscene_pic widget_init"; 

    QGraphicsProxyWidget * proxy ;

  /*  bt_stopCall = new QPushButton;

    bt_stopCall->setGeometry( 970, 300, 50 , 80);
	bt_stopCall->setText("STOP");
    //bt_stopCall->setIcon( pixmap );
    //bt_stopCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
    proxy = this->addWidget( bt_stopCall );
    proxy->setRotation(-90);
    connect( bt_stopCall ,SIGNAL(clicked( )), this, SLOT(bt_stopCallClicked( )));*/

	label_back = new QLabel();
	label_back->setAttribute(Qt::WA_TranslucentBackground); 
    label_back->setPixmap(QPixmap(":/pic/main-other.bmp"));
	label_back->setGeometry(0,0,SCREEN_WID,SCREEN_HEIGHT);
	//label_back->setText("PIC CALL WINDOWS");
	this->addWidget(label_back);

	label_net_status = new QLabel();
	label_net_status->setAttribute(Qt::WA_TranslucentBackground);
	label_net_status->setPixmap(QPixmap(":/pic/offline.bmp"));
	label_net_status->move(15,50);
	proxy = this->addWidget(label_net_status);
	proxy->setRotation(-90);

	label_time = new QLabel("");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
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
	pixmap.load( ":/pic/rightCall.bmp" );

	bt_retCall = new QPushButton;
	bt_retCall->setGeometry( 900, 100,pixmap.width() ,pixmap.height());
	bt_retCall->setIcon( pixmap );
	bt_retCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
	this->addWidget( bt_retCall );
	
	connect( bt_retCall ,SIGNAL(clicked( )), this, SLOT(bt_retCallClicked( )));



}

void  myscene_pic::bt_retCallClicked()
{
	pmv->changeWindowType( pmv->topWindowType);
}	

myscene_pic::myscene_pic(MyView * pm,QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();
	pmv = pm;

    widget_init();

}  

/*
char *name[]={"星期日","星期一","星期二","星期三","星期四","星期五","星期六"};   
void main(void)   
{   
        int d,m,y,e,t,f;   
        printf("请输入日:");   
        fflush(stdout);   
        scanf("%d",&d);   
        printf("请输入月:");   
        fflush(stdout);   
        scanf("%d",&m);   
        printf("请输入年:");   
        fflush(stdout);   
        scanf("%d",&y);   
        switch(m)   
        {   
                case 1:e=d;break;   
                case 2:e=31+d;break;   
                case 3:e=59+d;break;   
                case 4:e=90+d;break;   
                case 5:e=120+d;break;   
                case 6:e=151+d;break;   
                case 7:e=181+d;break;   
                case 8:e=212+d;break;   
                case 9:e=243+d;break;   
                case 10:e=273+d;break;   
                case 11:e=304+d;break;   
                case 12:e=334+d;break;   
                default:return;   
        }   
        if(y%4==0&&y%100!=0||y%400==0)   
        {  
                if(m>2)   
                        ++e;   
        }  
        --y;   
        t=y+y/4-y/100+y/400+e;   
        f=t%7;   
        printf("这一天是 %s\n",name[f]);   
}  

*/



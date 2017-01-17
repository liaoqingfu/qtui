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
	label_back->setGeometry(100,100,102,60);
	label_back->setText("PIC CALL WINDOWS");
	this->addWidget(label_back);

	label_time = new QLabel("time:");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
    label_time->setPalette(pe);
    label_time->setAttribute(Qt::WA_TranslucentBackground); 
    label_time->setFont( QFont(FONE_NAME, 40) );
    label_time->move(180,400);
    proxy = this->addWidget(label_time);
    proxy->setRotation(-90);

	
	label_date = new QLabel("time:");
	//pe.setColor(QPalette::WindowText,Qt::white);
	label_date->setPalette(pe);
	label_date->setAttribute(Qt::WA_TranslucentBackground); 
	label_date->setFont( QFont(FONE_NAME, 25) );
	label_date->move(250,470);
	proxy = this->addWidget(label_date);
	proxy->setRotation(-90);
	
	QPixmap pixmap;
	pixmap.load( ":/rightCall.bmp" );

	bt_retCall = new QPushButton;
	bt_retCall->setGeometry( 900, 100,pixmap.width() ,pixmap.height());
	bt_retCall->setIcon( pixmap );
	bt_retCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
	this->addWidget( bt_retCall );
	
	connect( bt_retCall ,SIGNAL(clicked( )), this, SLOT(bt_retCallClicked( )));

	m_nTimerId = startTimer(1000);  
    timerEvent( new QTimerEvent(m_nTimerId) ) ;


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



void myscene_pic::timerEvent( QTimerEvent *event )

{
	QTime qtimeObj = QTime::currentTime();
	static int minute = -1;
	if( pmv->WindowType != WINDOW_TYPE_PIC_CALL)
		return;
	
	if( minute != qtimeObj.minute() ){
		minute = qtimeObj.minute();
		QString str;
		str.sprintf("%02d:%02d",qtimeObj.hour(),qtimeObj.minute() );  //, qtimeObj.second()
		label_time->setText(str);

		label_date->setText(QDate::currentDate().toString(tr("yyyy-MM-dd dddd")));  
	}

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



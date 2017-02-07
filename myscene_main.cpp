#include "myscene_main.h"  
#include <stdio.h>
#include "video/main_api.h"
#include "myview.h" 



extern ncs_cfg_t cfg;
extern "C"  int _Z6xc9000v( );

extern "C"  void pt( );


/*
void myscene_main::verifyTime()
{
    QNetworkRequest request(QUrl("http://open.baidu.com/special/time/"));
    QNetworkReply *reply = manager.get(request);
    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(verifyLocalTime(QNetworkReply*)));
}

bool myscene_main::verifyLocalTime(QNetworkReply *reply)
{
#if 1
    QRegExp rx("window.baidu_time\\((.+)\\)");
    do{
        QString str = QString::fromUtf8(reply->readLine());
        qDebug() << __FUNCTION__ << reply->readLine();
        int pos = str.indexOf(rx);
        if(pos >= 0)
        {
            QDateTime time = QDateTime::fromMSecsSinceEpoch(rx.cap(1).toLongLong());
            if (time.isValid())
            {
                //bool bRet = Util::setSystemTime(time);
				//system_cmd_exec
				QString str= "date -s \"" + time.toString("yyyy-MM-dd hh:mm:ss") + "\"";
    			system_cmd_exec(str.toLatin1().data());
                qDebug() << "system_cmd_exec" << str.toLatin1().data();
                return true;
            }
            break;
        }
    }while(!reply->atEnd());
#endif
    return false;
}
*/
void  myscene_main::widget_init()
{
	qDebug() << "myscene_main widget_init"; 

	//verifyTime();

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
    label_back->setPixmap(QPixmap(":/pic/main.bmp"));
	label_back->setGeometry(0,0,SCREEN_WID,SCREEN_HEIGHT);
	this->addWidget(label_back);

	label_time = new QLabel("");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
    label_time->setPalette(pe);
    label_time->setAttribute(Qt::WA_TranslucentBackground); 
    label_time->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*2) );
    label_time->setGeometry(180,400,300,150);
    proxy = this->addWidget(label_time);
    proxy->setRotation(-90);

	
	label_date = new QLabel("");
	//pe.setColor(QPalette::WindowText,Qt::white);
	label_date->setPalette(pe);
	label_date->setAttribute(Qt::WA_TranslucentBackground); 
	label_date->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*1.5) );
	label_date->setGeometry(250,470,400,150);
	proxy = this->addWidget(label_date);
	proxy->setRotation(-90);

	QPixmap pixmap;

    bt_leftCall = new QPushButton;
    pixmap.load( ":/pic/leftCall.bmp" );
    bt_leftCall->setGeometry( 900, 300,pixmap.width() ,pixmap.height());
    bt_leftCall->setIcon( pixmap );
    bt_leftCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
    this->addWidget( bt_leftCall );

	bt_rightCall = new QPushButton;
    pixmap.load( ":/pic/rightCall.bmp" );
    bt_rightCall->setGeometry( 900, 100,pixmap.width() ,pixmap.height());
    bt_rightCall->setIcon( pixmap );
    bt_rightCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
    this->addWidget( bt_rightCall );
	
	connect( bt_leftCall ,SIGNAL(clicked( )), this, SLOT(bt_leftCallClicked( )));
	connect( bt_rightCall ,SIGNAL(clicked( )), this, SLOT(bt_rightCallClicked( )));

	//m_nTimerId = startTimer(1000);  
   // timerEvent( new QTimerEvent(m_nTimerId) ) ;

}

void  myscene_main::bt_leftCallClicked()
{
	pmv->changeWindowType( WINDOW_TYPE_CALLING );
	if( pmv->scene_calling->startCall(pmv->scene_calling->sip_ua_1->m_call_target[0],TS_SIP_TALK ) > 0)  //cfg.ip_keyleft.toLatin1().data()
		printf( "bt_leftCallClicked:%s\n", pmv->scene_calling->sip_ua_1->m_call_target[0]); //cfg.ip_keyleft	<< cfg.port_keyleft;
	else
		pmv->changeWindowType( WINDOW_TYPE_MAIN);
}

void  myscene_main::bt_rightCallClicked( )
{
	pmv->changeWindowType( WINDOW_TYPE_CALLING );
	if( pmv->scene_calling->startCall( pmv->scene_calling->sip_ua_1->m_call_target[1],TS_SIP_TALK ) > 0)  //cfg.ip_keyright.toLatin1().data()
		printf( "bt_rightCallClicked:%s\n", pmv->scene_calling->sip_ua_1->m_call_target[1]); //cfg.ip_keyright << cfg.port_keyright;
	else
		pmv->changeWindowType( WINDOW_TYPE_MAIN);
}

	

myscene_main::myscene_main(MyView * pm,QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();
	pmv = pm;

    widget_init();

}  

void myscene_main::mousePressEvent(QGraphicsSceneMouseEvent *event)  
{  
	
    QGraphicsScene::mousePressEvent(event);
}  
void myscene_main::changed(const QList<QRectF> &region)
{  
    qDebug() << "myscene_main::changed "<< region;
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



#include "myscene_main.h"  
#include <stdio.h>
#include "video/main_api.h"
#include "myview.h" 

//low lever to trig call
#define LEFT_KEY  (4*32+20)  //CSI0_DATA_EN    5_20      left key
#define RIGHT_KEY  (5*32+31)  //EIM_BCLK        6_31         right key

#define LEFT_LED   (13)  //SD2_DATA2       1_13              left  led
#define RIGHT_LED  (15)  //SD2_DATA0       1_15              right   led

#define GPIO_IN   0
#define GPIO_OUT   1


extern ncs_cfg_t cfg;



void myscene_main::gpio_init(  )
{
	gpio_open(LEFT_KEY,  GPIO_IN); 
	gpio_open(RIGHT_KEY, GPIO_IN); 
	
	gpio_open(LEFT_LED , GPIO_OUT); 
	gpio_open(RIGHT_LED , GPIO_OUT);  

}

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

void  myscene_main::widget_init()
{
	qDebug() << "myscene_main widget_init"; 

	verifyTime();

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
    label_back->setPixmap(QPixmap(":/main.bmp"));
	label_back->setGeometry(0,0,1024,600);
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

    bt_leftCall = new QPushButton;
    pixmap.load( ":/leftCall.bmp" );
    bt_leftCall->setGeometry( 900, 300,pixmap.width() ,pixmap.height());
    bt_leftCall->setIcon( pixmap );
    bt_leftCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
    this->addWidget( bt_leftCall );

	bt_rightCall = new QPushButton;
    pixmap.load( ":/rightCall.bmp" );
    bt_rightCall->setGeometry( 900, 100,pixmap.width() ,pixmap.height());
    bt_rightCall->setIcon( pixmap );
    bt_rightCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
    this->addWidget( bt_rightCall );
	
	connect( bt_leftCall ,SIGNAL(clicked( )), this, SLOT(bt_leftCallClicked( )));
	connect( bt_rightCall ,SIGNAL(clicked( )), this, SLOT(bt_rightCallClicked( )));

	m_nTimerId = startTimer(1000);  
    timerEvent( new QTimerEvent(m_nTimerId) ) ;

	gpio_init(  );

}

void  myscene_main::bt_leftCallClicked()
{
	pmv->changeWindowType( WINDOW_TYPE_NUM_CALL );
	gpio_set( LEFT_LED , 1);
	qDebug() << "bt_leftCallClicked" << cfg.ip_keyleft  << cfg.port_keyleft;
}

void  myscene_main::bt_rightCallClicked( )
{
	gpio_set( RIGHT_LED , 1);
	qDebug() << "bt_rightCallClicked" << cfg.ip_keyright << cfg.port_keyright;
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



void myscene_main::timerEvent( QTimerEvent *event )

{
	QTime qtimeObj = QTime::currentTime();
	static int minute = -1;
	if( pmv->WindowType != WINDOW_TYPE_MAIN)
		return;

	if( gpio_get( LEFT_KEY ) == 0)
		bt_leftCallClicked();

	if( gpio_get( RIGHT_KEY ) == 0)
		bt_rightCallClicked();
	
	if( minute != qtimeObj.minute() ){
		minute = qtimeObj.minute();
		QString str;
		str.sprintf("%02d:%02d",qtimeObj.hour(),qtimeObj.minute() );  //, qtimeObj.second()
		label_time->setText(str);

		label_date->setText(QDate::currentDate().toString(tr("yyyy-MM-dd dddd")));  
	}

} 
/*
char *name[]={"������","����һ","���ڶ�","������","������","������","������"};   
void main(void)   
{   
        int d,m,y,e,t,f;   
        printf("��������:");   
        fflush(stdout);   
        scanf("%d",&d);   
        printf("��������:");   
        fflush(stdout);   
        scanf("%d",&m);   
        printf("��������:");   
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
        printf("��һ���� %s\n",name[f]);   
}  

*/


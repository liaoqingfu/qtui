#include "myscene_num_call.h"  
#include <stdio.h>
#include "video/main_api.h"
#include "myview.h" 



QString  call_num_seq[13] = {"1","2","3","4","5","6","7","8","9","*","0","#","<"};

//struct VIDEO_BUF{
//	char * buf ;
//	int len ;
//};
#define SERVER_PORT  60012
#define LOCAL_PORT  60013

#define QT_CALL_API  0


class videoRead : public QThread
{
public:
   virtual  void run();
};
void videoRead::run()
{
	while( 1 ){

        qDebug() <<  "videoRead" ;
		sleep(3);
	}
}



void  myscene_num_call::widget_init()
{
	QGraphicsProxyWidget * proxy ;
	qDebug() << "widget_init"; 

	udpSocket = new QUdpSocket;
	udpSocket->bind(QHostAddress::LocalHost,LOCAL_PORT);
	qDebug() << QHostAddress::LocalHost;
	connect(udpSocket,SIGNAL(readyRead()),this,SLOT(recv_slot()));

	label_back = new QLabel();
	label_back->setAttribute(Qt::WA_TranslucentBackground); 
    label_back->setPixmap(QPixmap(":/pic/main-other.bmp"));
	label_back->setGeometry(0,0,SCREEN_WID,SCREEN_HEIGHT);
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
	

    label_net_status = new QLabel();
    label_net_status->setAttribute(Qt::WA_TranslucentBackground);
    label_net_status->setPixmap(QPixmap(":/pic/offline.bmp"));
    label_net_status->move(15,50);
    proxy = this->addWidget(label_net_status);
    proxy->setRotation(-90);

    label_bottom_status = new QLabel();
    label_bottom_status->setPalette(pe);
    label_bottom_status->setAttribute(Qt::WA_TranslucentBackground);
    label_bottom_status->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE) );
    label_bottom_status->setGeometry(SCREEN_WID-60,560,560,50);
    proxy = this->addWidget(label_bottom_status);
    proxy->setRotation(-90);

    //call number  
    label_numcall = new QLabel();
	label_numcall->setPalette(pe);
    label_numcall->setAttribute(Qt::WA_TranslucentBackground);
    label_numcall->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*2) );
    label_numcall->setGeometry(80,530,400,120);
    proxy = this->addWidget(label_numcall);
    proxy->setRotation(-90);

    bg_numcall = new QButtonGroup;
    QString bg_pic;
    QPixmap pixmap;


     for(int i = 0; i < NUM_CALL; i++){
        bt_numcall[i] = new QPushButton;

        int xpos = NUM_START_X + ((int)(i/3))*NUM_X_WID;
        int ypos =  NUM_START_Y - (i%3) * NUM_Y_HEIGHT;

        bt_numcall[i]->setGeometry( xpos, ypos, NUM_Y_HEIGHT,NUM_X_WID);  //pixmap.width() ,pixmap.height());
        bt_numcall[i]->setAttribute(Qt::WA_TranslucentBackground); 
        if( i < 12){
			bt_numcall[i]->setStyleSheet("QPushButton{" "border:3px solid white;}");  //宽度为3px的红色边框
			bt_numcall[i]->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*1.5) );
			bt_numcall[i]->setPalette(pe);
			bt_numcall[i]->setText(  call_num_seq[i]  );
        }
		else {
			bg_pic.sprintf(":/pic/pic%i.bmp",i - 12);
	        pixmap.load( bg_pic );
			bt_numcall[i]->setIcon( pixmap );
			bt_numcall[i]->setIconSize( QSize( NUM_Y_HEIGHT,NUM_X_WID));
		}
        	
        bg_numcall->addButton(bt_numcall[i],i);
        proxy = this->addWidget(bt_numcall[i]);
        proxy->setRotation(-90);

    }

    str_numcall = "";

    connect( bg_numcall ,SIGNAL(buttonClicked(int)), this, SLOT(bt_numcallClicked(int)));



}

void myscene_num_call::bt_numcallClicked( int buttonID)
{
    static int reachMaxLen = 0;
    udpSocket->writeDatagram(str_numcall.toLatin1().data(),str_numcall.length(),QHostAddress::LocalHost,SERVER_PORT);

    if( pmv->WindowType == WINDOW_TYPE_NUM_CALL) {
            int num_len = str_numcall.length();

            if ( buttonID < 12) {
                if (num_len < MAX_NUM_LEN ){
                    str_numcall = str_numcall + call_num_seq[buttonID];

                }
                else{
                    label_bottom_status->setText( "Warning: reach max length!" );
                    reachMaxLen = 1;
                }
            }
			else if ( buttonID == 12) {
               if (num_len > 0 ){
                   str_numcall = str_numcall.left( num_len - 1 );
                   if( reachMaxLen ){
                        reachMaxLen = 0;
                        label_bottom_status->setText( "" );
                   }
               }
            }
            else if ( buttonID == 13) {
               qDebug() << "call to" << str_numcall;
			   if( pmv->scene_calling->startCall( str_numcall.toLatin1().data() ,TS_SIP_TALK) > 0) {
					pmv->changeWindowType( WINDOW_TYPE_CALLING); 
					qDebug() << "CHANGE TO CALLING WINDOW" ;
			   	}
            }
            else if ( buttonID == 14) {  //return to main window
               
			   pmv->changeWindowType( WINDOW_TYPE_MAIN) ; 
            }

            label_numcall->setText( str_numcall );

            qDebug() <<str_numcall;
    }

}

void myscene_num_call::recv_slot()
{
	while (udpSocket->hasPendingDatagrams()) {
		   QByteArray datagram;
		   datagram.resize(udpSocket->pendingDatagramSize());
		   QHostAddress sender;
		   quint16 senderPort;
	
		   udpSocket->readDatagram(datagram.data(), datagram.size(),
								   &sender, &senderPort);
	
		  qDebug() << datagram;
	   }


}

myscene_num_call::myscene_num_call(MyView * pm,QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();
	pmv = pm;
    widget_init();
}  
	
/*  
void myscene_num_call::keyPressEvent(QKeyEvent *event)  
{  
    //qDebug("**myscene_num_call::keyPress*");
	return QGraphicsScene::keyPressEvent(event);  
}  
  
void myscene_num_call::mousePressEvent(QGraphicsSceneMouseEvent *event)	
{  
    qDebug("**myscene_num_call::mousePress*");
	QGraphicsScene::mousePressEvent(event);  
}  */


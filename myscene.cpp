#include "myscene.h"  
#include <stdio.h>
#include "video/main_api.h"


#define FIFO_QT_VIDEO  "/tmp/qt_video"
int bVideoShow = 0;

char call_num_seq[13] = {'1','2','3','4','5','6','7','8','9','*','0','#','.'};

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

void MyScene::WriteSettings()
{
    //QSettings settings("Software Inc", "Spreadsheet"); // windows在注册表中建立建 Software Inc -> Spreadsheet
    QSettings settings(CFG_NAME, QSettings::IniFormat); // 当前目录的INI文件
    settings.beginGroup("DevOption");
    settings.setValue("mainFun", 25);
    settings.setValue("subFun", 40);
    settings.setValue("service", 1);
    settings.endGroup();

	settings.beginGroup("video_cfg");
	settings.setValue( "output_w", 1024); //,output_h;int crop_x,crop_y,crop_w,crop_h;	int rotate;
	settings.setValue( "output_h", 600);
	settings.setValue( "crop_x", 0);
	settings.setValue( "crop_y", 0);
	settings.setValue( "crop_w", 0);
	settings.setValue( "crop_h", 0);
	settings.setValue( "rotate", 4);

    settings.endGroup();
}

void MyScene::ReadSettings( QString sItem, int * value)
{
    QSettings settings(CFG_NAME, QSettings::IniFormat);

    * value = settings.value( sItem ).toInt();

}

void MyScene::CallTypeShow( int callTypeNum)
{
    if( callTypeNum == 0 ) {
        label_numcall->show();
        for(int i = 0; i < NUM_CALL; i++){
           bt_numcall[i] ->show();
        }
         connect( bg_numcall ,SIGNAL(buttonClicked(int)), this, SLOT(bt_numcallClicked(int)));
        label_pic_call->hide();

    }
    else  if( callTypeNum == 1 ) {
        label_numcall->hide();
        str_numcall = "";

        label_numcall->setText( str_numcall );
        for(int i = 0; i < NUM_CALL; i++){
           bt_numcall[i] ->hide();
        }

        //label_pic_call->show();
         if ( m_nTimerId != 0 )
        	killTimer(m_nTimerId);
		 m_nTimerId = 0;
		 bVideoShow = 1;
		 
#if QT_CALL_API
			video_init_err = 0;

			if (video_capture_init( image_width, image_height, image_fps) < 0 ) {	
				qDebug() << "video_capture_init fail\n";
				video_init_err |= VIDEO_CAPTURE_ERR;
			}
			
			if (h264_enc_init ( image_width, image_height, image_fps) < 0   ) {	
				qDebug() << "h264_enc_init fail\n";
				video_init_err |= VIDEO_H264_ENC_ERR;
			}

			
			if ((h264_dec_ctx = h264_dec_init( )) == NULL)
			{
				printf("h264_dec_init fail\n");
				video_init_err |= VIDEO_H264_DEC_ERR;
			}
//#else
//			startVideoTest();
#endif
    }
}

void  MyScene::widget_init()
{
    bShowThisWindow = 1;
	
	qDebug() << "widget_init"; 

	udpSocket = new QUdpSocket;
	udpSocket->bind(QHostAddress::LocalHost,LOCAL_PORT);
	qDebug() << QHostAddress::LocalHost;
	connect(udpSocket,SIGNAL(readyRead()),this,SLOT(recv_slot()));

    label_time = new QLabel("time:");
    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::white);
    label_time->setPalette(pe);
    label_time->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_time->setFont( QFont(FONE_NAME, 20) );
    label_time->move(15,560);
    QGraphicsProxyWidget * proxy = this->addWidget(label_time);
    proxy->setRotation(-90);

    label_welcome = new QLabel();
    label_welcome->setPalette(pe);
    label_welcome->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_welcome->setFont( QFont(FONE_NAME, 20) );
    label_welcome->setGeometry(10,380,300,50);
    label_welcome->setText( "IP对讲广播系统" );
    proxy = this->addWidget(label_welcome);
    proxy->setRotation(-90);

    net_status = 0;
    label_net_status = new QLabel();
    label_net_status->setText(tr("网络连接状态       ")  );
    label_net_status->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_net_status->setPixmap(QPixmap(":/offline.bmp"));
    label_net_status->move(15,50);
    proxy = this->addWidget(label_net_status);
    proxy->setRotation(-90);

   label_bottom_status = new QLabel();
    label_bottom_status->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_bottom_status->setFont( QFont(FONE_NAME, 20) );
    label_bottom_status->setGeometry(1024-60,560,560,50);
    proxy = this->addWidget(label_bottom_status);
    proxy->setRotation(-90);


    label_pic_call = new QLabel();
    label_pic_call->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_pic_call->setPixmap(QPixmap(":/pic_call.bmp"));
    label_pic_call->setGeometry(60,0,950,600);
    proxy = this->addWidget(label_pic_call);
    label_pic_call->hide();

    //number call widget init
    label_numcall = new QLabel();
    label_numcall->setAttribute(Qt::WA_TranslucentBackground);//设置 透明
    label_numcall->setFont( QFont(FONE_NAME, 30) );
    label_numcall->setGeometry(80,530,400,120);
    proxy = this->addWidget(label_numcall);
    proxy->setRotation(-90);

    bg_numcall = new QButtonGroup;
    QString bg_pic;
    QPixmap pixmap;


   // for(int i = 0; i < NUM_CALL; i++){
     for(int i = 0; i < NUM_CALL; i++){
        bt_numcall[i] = new QPushButton;
        bg_pic.sprintf(":/input_dp_%i.bmp",i+1);

         pixmap.load( bg_pic );

        int xpos = NUM_START_X + ((int)(i/3))*NUM_X_WID;
        int ypos =  530 - (i%3) * NUM_Y_HEIGHT;

        bt_numcall[i]->setGeometry( xpos, ypos,pixmap.width() ,pixmap.height());
         bt_numcall[i]->setIcon( pixmap );

        bt_numcall[i]->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
        bg_numcall->addButton(bt_numcall[i],i);

      //  qDebug() << bg_pic << "wid"<<pixmap.width() ;
        proxy = this->addWidget(bt_numcall[i]);
        proxy->setRotation(-90);

    }

    str_numcall = "";

    connect( bg_numcall ,SIGNAL(buttonClicked(int)), this, SLOT(bt_numcallClicked(int)));
	   
	WriteSettings();
	ReadSettings( "video_cfg/output_w", &output_w); //,output_h;int crop_x,crop_y,crop_w,crop_h;	int rotate;
	ReadSettings( "video_cfg/output_h", &output_h);
	ReadSettings( "video_cfg/crop_x", &crop_x);
	ReadSettings( "video_cfg/crop_y", &crop_y);
	ReadSettings( "video_cfg/crop_w", &crop_w);
	ReadSettings( "video_cfg/crop_h", &crop_h);
	ReadSettings( "video_cfg/rotate", &rotate);

	//ipu_para_set(output_w, output_h, crop_x , crop_y, crop_w, crop_h, rotate);
	
	//ipu_para_set(1024,600,
	//class videoRead vr;
	//vr.start();


}

void MyScene::bt_numcallClicked( int buttonID)
{
    static int reachMaxLen = 0;
    udpSocket->writeDatagram(str_numcall.toLatin1().data(),str_numcall.length(),QHostAddress::LocalHost,SERVER_PORT);

    if( bShowThisWindow == 1) {
            int num_len = str_numcall.length();

            if ( buttonID < 13) {
                if (num_len < MAX_NUM_LEN ){
                    str_numcall = str_numcall + call_num_seq[buttonID];

                }
                else{
                    //QToolTip::showText ( QPoint(60,530),(QString::fromStdString("Warning: reach max call length!")  ));
                    label_bottom_status->setText( "Warning: reach max length!" );
                    reachMaxLen = 1;
                }
            }
            else if ( buttonID == 13) {
               qDebug() << "call to" << str_numcall;
			   bShowThisWindow = 0;
             //  CallTypeShow( (++call_type) % MAX_CALL_TYPE );
               
            }
            else if ( buttonID == 14) {
               if (num_len > 0 ){
                       str_numcall = str_numcall.left( num_len - 1 );
                       if( reachMaxLen ){
                            reachMaxLen = 0;
                            label_bottom_status->setText( "" );
                       }
                   }
            }

            label_numcall->setText( str_numcall );

            qDebug() <<str_numcall;
    }

}

void MyScene::recv_slot()
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

void MyScene::timerEvent( QTimerEvent *event )

{
	static int count = 0;
   //if( event->timerId() == m_nTimerId ) {
   if( (count++)%FRAME_FPS == 0 ) {
       QTime qtimeObj = QTime::currentTime();

       QString str;
       str.sprintf("%02d:%02d:%02d",qtimeObj.hour(),qtimeObj.minute(), qtimeObj.second() );

       label_time->setText(str);
       //if( call_type == 1)
       //	CallTypeShow( (++call_type) % MAX_CALL_TYPE );
       if( net_status )
	        label_net_status->setPixmap(QPixmap(":/online.bmp"));
	   else
	       label_net_status->setPixmap(QPixmap(":/offline.bmp"));
		
	} 

#if QT_CALL_API
   if( video_init_err == 0){
	   if (video_capture_read(&yuv_buf,&yuv_len) > 0) {
			enc_re_len = h264_enc_process(yuv_buf, yuv_len,&enc_re_data_ptr);	//lhg comment:	yuv --->h264 (data_ptr)
			if( enc_re_data_ptr && enc_re_len > 0){
				h264_dec_run(h264_dec_ctx , enc_re_data_ptr, enc_re_len);
			}
			else
				qDebug() << "h264_enc_process return null";
		}

   }
#endif


	
 /*  {
        QImage *grayImg=new QImage((uchar*)videoBuf,512,300,512,QImage::Format_RGB888);
		QPixmap mp;
		mp.fromImage(  *grayImg);
		label_pic_call->setGeometry(60,0,40,400);
		label_pic_call->show();
		label_pic_call->setPixmap(mp);
   }*/
   

}


MyScene::MyScene(QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();

    widget_init();

    m_nTimerId = startTimer(REF_VIDEO_MS);  //REF_VIDEO_MS    FRAME_FPS    REF_TIME_MS
    timerEvent( new QTimerEvent(m_nTimerId) ) ;

}  
  
void MyScene::keyPressEvent(QKeyEvent *event)  
{  
    //qDebug("**MyScene::keyPress*");
	return QGraphicsScene::keyPressEvent(event);  
}  
  
void MyScene::mousePressEvent(QGraphicsSceneMouseEvent *event)	
{  
   // qDebug("**MyScene::mousePress*");
	QGraphicsScene::mousePressEvent(event);  
}  


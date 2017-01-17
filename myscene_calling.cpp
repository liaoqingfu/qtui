#include "myscene_calling.h"  
#include <stdio.h>

#include "myview.h" 
#include "sipua/include/eXosip2/eXosip.h" 

#define CALL_TIMEOUT   15000    //15 second

#define VIDEO_RTP_FRAME_LEN_MAX		(1024*40)
#define VIDEO_ENABLE  1
#define LOCAL_VIDEO_DISPLAY

#define HARD_KEY_TRIG  0

extern ncs_cfg_t cfg;
extern "C"  int _Z6xc9000v( );

static int bCaptureThread = 0;

void * capture_thread(void * pParam)
{
	static int frameCount = 0;
	char * cap_buf = NULL;
	int cap_len = 0;
	int enc_re_len = 0; 
	char *enc_re_data_ptr = NULL;
	if( bCaptureThread ){
		printf_log(LOG_IS_INFO, "error call capture thread\n");
		return NULL;
	}
	else
		bCaptureThread = 1;
	printf_log(LOG_IS_INFO, "Start capture thread\n");
	myscene_calling * pMysceneCalling = ( myscene_calling * )pParam;
	while(1)
	{
		if( frameCount++ % 100 == 0)
			printf_log(LOG_IS_INFO, " Capture thread frameCount:%d\n",frameCount);
		
		if (video_capture_read(&cap_buf,&cap_len) > 0) {
			enc_re_len = h264_enc_process(cap_buf, cap_len,&enc_re_data_ptr);	//lhg comment:	yuv --->h264 (data_ptr)
			pMysceneCalling->sip_ua_1->m_video_stream.write_rtp_senddata( ( unsigned char*) enc_re_data_ptr, enc_re_len);

			//local display:
			#ifdef LOCAL_VIDEO_DISPLAY
				h264_dec_run( pMysceneCalling->h264_dec_ctx , enc_re_data_ptr, enc_re_len);
			#endif
		}
		if( pMysceneCalling->bExitCaptureThread ) {
			video_capture_destroy();
			break;
		}
	}

	pMysceneCalling->bExitCaptureThread = 0;
	bCaptureThread = 0;
	printf_log(LOG_IS_INFO, " Exit capture thread \n");
	return NULL;
}

void  dec_callback(void * pParam)
{
	if (pParam != NULL )
	{
		int 	nPayloadSize = 0;
		myscene_calling * pMysceneCalling = ( myscene_calling * )pParam;
		unsigned char  pH264RecvBuf[VIDEO_RTP_FRAME_LEN_MAX];
		nPayloadSize = pMysceneCalling->sip_ua_1->m_video_stream.read_rtp_recvdata(pH264RecvBuf, VIDEO_RTP_FRAME_LEN_MAX);
		if( nPayloadSize > 0 )
			h264_dec_run(pMysceneCalling->h264_dec_ctx , (char *)pH264RecvBuf, nPayloadSize);
	
	}
}


void sipEvent_callback(eXosip_event_t *p_event, void *pParam)
{
	myscene_calling * pMysceneCalling = ( myscene_calling * )pParam;
	if (pParam != NULL && p_event != NULL)
	{
		switch (p_event->type)
		{
		case EXOSIP_REGISTRATION_SUCCESS:
			printf_log(LOG_IS_INFO, "EXOSIP_REGISTRATION_SUCCESS\n");
			break;
		case EXOSIP_CALL_RINGING:
			printf_log(LOG_IS_INFO, "EXOSIP_CALL_RINGING\n");
			break;
		case EXOSIP_CALL_ANSWERED:
		case EXOSIP_CALL_ACK:
			printf_log(LOG_IS_INFO, "EXOSIP ANSWERED ACK\n");
			pMysceneCalling->inCall(  );
			break;
		case EXOSIP_CALL_INVITE:
			if( pMysceneCalling->answerCall() > 0){
				pMysceneCalling->pmv->changeWindowType(WINDOW_TYPE_CALLING);
				printf_log(LOG_IS_INFO, "EXOSIP CALL INVITE,ANSWER SUCCESS\n");
			}
			else
				printf_log(LOG_IS_INFO, "EXOSIP CALL INVITE,ANSWER FAILED\n");
			break;
		case EXOSIP_CALL_CLOSED:
			pMysceneCalling->stopCall();
			printf_log(LOG_IS_INFO, "EXOSIP CALL CLOSED\n");
			break;
		default:
			printf_log(LOG_IS_INFO, "other EXOSIP Event:%d\n" , p_event->type);
			break;
		}
	}
}



void  myscene_calling::widget_init()
{
	qDebug() << "myscene_calling widget_init"; 
	SipTalkType = CALL_IDLE;
	pid_capture = 0;

    QGraphicsProxyWidget * proxy ;


	label_back = new QLabel();
	label_back->setAttribute(Qt::WA_TranslucentBackground); 
    label_back->setPixmap(QPixmap(":/main.bmp"));
	label_back->setGeometry(0,0,1024,600);
	this->addWidget(label_back);

	label_backCircle = new QLabel();
	label_backCircle->setAttribute(Qt::WA_TranslucentBackground); 
    label_backCircle->setPixmap(QPixmap(":/calling.bmp"));
	label_backCircle->move(200,100);
	this->addWidget(label_backCircle);

	QPalette pe;
	pe.setColor(QPalette::WindowText,Qt::white);

	/*label_time = new QLabel("time:");
    
    label_time->setPalette(pe);
    label_time->setAttribute(Qt::WA_TranslucentBackground); 
    label_time->setFont( QFont(FONE_NAME, 40) );
    label_time->setGeometry(230,370,150,150);
    proxy = this->addWidget(label_time);
    proxy->setRotation(-90);*/
	
	label_calling = new QLabel("Calling... ...");
	//pe.setColor(QPalette::WindowText,Qt::white);
	label_calling->setPalette(pe);
	label_calling->setAttribute(Qt::WA_TranslucentBackground); 
	label_calling->setFont( QFont(FONE_NAME, 25) );
	label_calling->setGeometry(290,370,250,250);
	proxy = this->addWidget(label_calling);
	proxy->setRotation(-90);

	
	QPixmap pixmap;

    bt_stopCall = new QPushButton;
   // pixmap.load( ":/leftCall.bmp" );
  //  bt_stopCall->setGeometry( 900, 300,pixmap.width() ,pixmap.height());
  //  bt_stopCall->setIcon( pixmap );
   //bt_stopCall->setIconSize( QSize( pixmap.width() -15,pixmap.height() -15));
  	bt_stopCall->setFont( QFont(FONE_NAME, 25) );
    bt_stopCall->move(920,300);
   	bt_stopCall->setText("STOP"); 
	proxy = this->addWidget(bt_stopCall);
	proxy->setRotation(-90);

	connect( bt_stopCall ,SIGNAL(clicked( )), this, SLOT(bt_stopCallClicked( )));

	sip_ua_1 = NULL;
	bExitCaptureThread = 0;

}


int myscene_calling::sip_init_once( char * sipServerIp, char * localId)
{
	struct timeval		ts;
	CSocketEx			socket_spon;
	char				p_file[PATH_MAX];
	string				str_file;

	if( (sip_ua_1 != NULL) )
		return -1;

	printf("serverip:%s, localId:%s\n",sipServerIp, localId);
	
	sip_ua_1 = new CSipUA;
	
	sip_ua_1->set_local_addr(socket_spon.get_first_hostaddr(), SIP_LOCAL_PORT + atoi(localId));
	sip_ua_1->set_register_addr(sipServerIp, SIP_SERVER_PORT);
	sip_ua_1->set_username_password(localId, SIP_SERVER_PASSWORD);
	sip_ua_1->m_audio_stream.set_enable_sound_card(TRUE);
	sip_ua_1->init();
	sip_ua_1->m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);

	ua_usleep(500000);
	get_exe_path(p_file, PATH_MAX);
	str_file = p_file;
	str_file = str_file.substr(0, str_file.find_last_of('\\') + 1);

    ua_get_time(&ts);

	// 添加文件列表
	sprintf(p_file, "%s%s", str_file.c_str(), "1.mp3");
	//sip_ua_1.m_audio_stream.m_audio_file.add_file(p_file);
	sprintf(p_file, "%s%s", str_file.c_str(), "1.wav");
	sip_ua_1->m_audio_stream.m_audio_file.add_file(p_file);
	sip_ua_1->m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
	video_init_error = 0;
	
	sip_ua_1->set_sip_event_callback( sipEvent_callback, this);
	
	if ((h264_dec_ctx = h264_dec_init( )) == NULL)
	{
		printf_log(LOG_IS_INFO,"h264_dec_init fail\n");
		video_init_error = 1;
		return -1;
	}

	if (h264_enc_init ( 352, 288,  25) < 0   ) {	//352, 288,      640, 480,
		printf_log(LOG_IS_INFO,"h264_enc_init fail\n");
		video_init_error += 2;
		return -1;
	}

	return 0;

}


void myscene_calling::startCapture(  )
{
#if VIDEO_ENABLE
	//if( SipTalkType == CALL_VIDEO_OUT)
	{
		if(!video_init_error ) {
			if (video_capture_init( 352, 288, 352, 288,  25) >= 0 ) {
				if (pthread_create(&pid_capture, NULL, capture_thread, this) >= 0)
				{
					sip_ua_1->set_enable_rtp_video(TRUE);
					sip_ua_1->set_video_recvdata_callback( dec_callback , this);
				}
				else
					printf("sequence_execute  create error:%s\n", strerror(errno));
			}
			else{	
				printf("video_capture init error\n");
				video_init_error += 4;
			}		
			
		}
	}
#endif
}

int myscene_calling::startCall( char * dstId)
{
	if( SipTalkType ){
		printf_log(LOG_IS_INFO,"startCall error:%d\n", SipTalkType);
		return SipTalkType;
	}
	if( (sip_ua_1 == NULL) ){
		//if( sip_init_once ( sipServerIp, localId ) < 0) 
		return -1;
	}
	SipTalkType = CALL_AUDIO_OUT;
	label_calling->setText("CALLING... ...");

	startCapture(  );
	
	if( sip_ua_1->talk(dstId, NULL, NULL) ){
		SipTalkType = video_init_error ? CALL_AUDIO_OUT: CALL_VIDEO_OUT;   //should modify to justify video or audio
		//add timeout event
		m_nTimeoutId = startTimer(CALL_TIMEOUT);  
		printf_log(LOG_IS_INFO,"StartCall timeout begin\n");
		
	}

	/*
		switch(command) 
		{
		case 's':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
			sip_ua_1.broadcast(p_dst_id, "WAV", TRUE);
			break;
		case 'w':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_FILE);
			sip_ua_1.broadcast(p_dst_id, "WAV", TRUE);
			break;
		case 'm':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_FILE);
			sip_ua_1.broadcast(p_dst_id, "MP3", TRUE);
			break;
		case 't':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
			sip_ua_1.talk(p_dst_id, NULL, NULL);
			break;
		case 'n':
			sip_ua_1.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
			sip_ua_1.monitor(p_dst_id, NULL, NULL);
			break;
		case 'h':
			sip_ua_1.task_end();
			break;
		case 'a':
			sip_ua_1.answer();
			break;
		case '0':
--1,2,3,4,5,6,7,8
		case '9':
			sprintf(p_file, "%c", command);
			sip_ua_1.send_msg_volume(p_dst_id, p_file, p_file, p_file, p_file);
			break;
		case 'q':
			b_exit = TRUE;
			break;
		default:
			// printf("input error! please input again\n");
			break;
		}
		//sip_ua_1.process_sip_event(NULL, NULL);
		//sip_ua_1.process_rtp_data();
		ua_usleep(100000);
    } while (!b_exit);  */
	return SipTalkType;
}

void 	myscene_calling::inCall(  )
{
	label_calling->setText("TALKING... ...");
	SipTalkType = SipTalkType << 1;  //  CALL_AUDIO_TALK     CALL_VIDEO_TALK;
}


//callee
int myscene_calling::answerCall()
{
	startCapture(	);
	if( sip_ua_1->answer() ){
		SipTalkType = video_init_error ? CALL_AUDIO_TALK: CALL_VIDEO_TALK;
		return SipTalkType;
	}
	return -1;

}
int myscene_calling::stopCall()
{
	if( (sip_ua_1 != NULL) && SipTalkType){
		
		if( bCaptureThread == 1 ) {
			bExitCaptureThread = 1;
			while( 	bExitCaptureThread )  //wait thread to exit
				usleep(100);
		}
		sip_ua_1->set_video_recvdata_callback(NULL, NULL);
		sip_ua_1->task_end();
		SipTalkType = CALL_IDLE;
		pmv->changeWindowType( pmv->topWindowType );
	}
	else{
		printf("sip_ua_1 is %x, or not in talk:%d\n",sip_ua_1,SipTalkType); 
		return -1;
	}

	/*
	h264_dec_uninit( h264_dec_ctx );
	*/

}
	

void  myscene_calling::bt_stopCallClicked()
{
	pmv->changeWindowType( pmv->topWindowType );
	stopCall();
	gpio_set( LEFT_LED , 0);
	qDebug() << "bt_stopCallClicked" << cfg.ip_keyleft  << cfg.port_keyleft;
}


	

myscene_calling::myscene_calling(MyView * pm,QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();
	pmv = pm;

    widget_init();

}  

void myscene_calling::timerEvent( QTimerEvent *event )

{
	if( event->timerId() == m_nTimeoutId)
	{
		if( (SipTalkType != CALL_AUDIO_TALK) && ( SipTalkType != CALL_VIDEO_TALK)) {
			printf_log(LOG_IS_INFO, "Call Timeout, auto stopCall\n");
			killTimer( m_nTimeoutId ); 
			#ifndef LOCAL_VIDEO_DISPLAY
			stopCall();
			#endif
		}
		
	}

} 



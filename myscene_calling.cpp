#include "myscene_calling.h"  
#include <stdio.h>

#include "myview.h" 
#include "sipua/include/eXosip2/eXosip.h" 

#define CALL_TIMEOUT   25000    // second

#define VIDEO_RTP_FRAME_LEN_MAX		(1024*40)
#define VIDEO_ENABLE  1
//#define LOCAL_VIDEO_DISPLAY


#define CAPTURE_WIDTH  1280
#define CAPTURE_HEIGHT 720

#define CAPTURE_OUT_WIDTH   1024
#define CAPTURE_OUT_HEIGHT  600
#define LOL_OUTMODE          0   //WEB 配置参数，线路输出==0

int list_call_id = -1;  //呼叫id

extern ncs_cfg_t cfg;
extern float g_fontResize;
extern "C"  int _Z6xc9000v( );

static int bCaptureThread = 0;
int 	sip_reg_success = 0;

#define AIC32X4_PAGE1		(1 << 7)
//output volume
#define	AIC32X4_HPLGAIN		(AIC32X4_PAGE1 + 16)
#define	AIC32X4_HPRGAIN		(AIC32X4_PAGE1 + 17)
#define	AIC32X4_LOLGAIN		(AIC32X4_PAGE1 + 18)
#define	AIC32X4_LORGAIN		(AIC32X4_PAGE1 + 19)
//input volume
#define AIC32X4_LADCVOL		(83)
#define AIC32X4_RADCVOL		(84)


static const BYTE           g_pOutputVolume[] = { // -63.5 ~ +24dB
                       /*0x81,*/0x81, 0xB0, 0xC4, 0xD8, // -63.5dB, -40, -30, -20...
                                0xEC, 0xF8, 0x00, 0x04, // -10, -4, 0
                                0x08, 0x0C, 0x10, 0x14, 
                                0x18, 0x1C, 0x20, 0x24, // +18dB
                                };
static const BYTE           g_pPHLOVolume[] = { // -6 ~ +29dB
                                0x3A, 0x3B, 0x3C, 0x3D, // -6dB...
                                0x3E, 0x3F, 0x00, 0x02, 
                                0x04, 0x06, 0x08, 0x0A, 
                                0x0C, 0x0E, 0x10, 0x12, // +18dB
                                };
static const BYTE           g_pInputVolume[] = { // -12 ~ +20dB
                                0x68, 0x6C, 0x70, 0x74, // -12dB...
                                0x78, 0x7C, 0x00, 0x04, 
                                0x08, 0x0C, 0x10, 0x14, 
                                0x18, 0x1C, 0x20, 0x24, // +18dB
                                };

void * capture_thread(void * pParam)
{
	static int frameCount = 0;
	char * cap_buf = NULL;
	int cap_len = 0;
	int enc_re_len = 0; 
	char *enc_re_data_ptr = NULL;
	if( bCaptureThread ){
		printf_log(LOG_IS_INFO, "error call capture thread alread run\n");
		return NULL;
	}
	else
		bCaptureThread = 1;
	printf_log(LOG_IS_INFO, "Start capture thread\n");
	myscene_calling * pMysceneCalling = ( myscene_calling * )pParam;
	while(1)
	{
		
		if (video_capture_read(&cap_buf,&cap_len) > 0) {
			if( frameCount++ % 3000 == 0)
			printf_log(LOG_IS_INFO, " Cap-thread frame,w:%d\n",frameCount);
			
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
	static int dec_callback_count = 0;
	if (pParam != NULL )
	{
		int 	nPayloadSize = 0;
		myscene_calling * pMysceneCalling = ( myscene_calling * )pParam;
		unsigned char  pH264RecvBuf[VIDEO_RTP_FRAME_LEN_MAX];
		nPayloadSize = pMysceneCalling->sip_ua_1->m_video_stream.read_rtp_recvdata(pH264RecvBuf, VIDEO_RTP_FRAME_LEN_MAX);
		while( nPayloadSize > 0 )
		{
			/*if( dec_callback_count++ % 200 == 0)
				printf_log(LOG_IS_INFO, " dec_cb,:%d\n",dec_callback_count);
			else	if( dec_callback_count < 8)*/
			//if( dec_callback_count < 100)
			//	printf ("dc:%d\n",dec_callback_count++);
			h264_dec_run(pMysceneCalling->h264_dec_ctx , (char *)pH264RecvBuf, nPayloadSize);
			nPayloadSize = pMysceneCalling->sip_ua_1->m_video_stream.read_rtp_recvdata(pH264RecvBuf, VIDEO_RTP_FRAME_LEN_MAX);
		}
	}
}

 void aic3204_init( )  
{
	 char cmd[64] = {0};
	//输入通道
	sprintf(cmd, "echo b40010 > /proc/aic3204_reg" );//input L:IN2L  page1,0x34  IN2L  10K
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo b60010 > /proc/aic3204_reg" );//input L:IN2R  page1,0x36  IN2R  10K
	system_cmd_exec( cmd ); 

	sprintf(cmd, "echo b70040 > /proc/aic3204_reg" ); //input R:IN1L  page1,0x37  IN1R  10K
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo b90010 > /proc/aic3204_reg" ); //input R:IN1R  page1,0x39  IN1L  10K
	system_cmd_exec( cmd ); 

	//MICPGA
	sprintf(cmd, "echo bb003c > /proc/aic3204_reg" ); //3c: 30db
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo bc003c > /proc/aic3204_reg" ); //3c: 30db
	system_cmd_exec( cmd ); 

	//ADC
	sprintf(cmd, "echo 530000 > /proc/aic3204_reg" );  //adc  0db
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo 540000 > /proc/aic3204_reg" );  //adc  0db
	system_cmd_exec( cmd ); 

	//DAC Channel Digital Volume Control
	sprintf(cmd, "echo 410000 > /proc/aic3204_reg" );  // 0db
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo 420000 > /proc/aic3204_reg" );  // 0db
	system_cmd_exec( cmd ); 

	//HPL,HPR Gain Setting
	sprintf(cmd, "echo 900000 > /proc/aic3204_reg" );  // 0db
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo 910000 > /proc/aic3204_reg" );  // 0db
	system_cmd_exec( cmd ); 

	//LOL  LOR Gain Setting
	sprintf(cmd, "echo 920000 > /proc/aic3204_reg" );  // 0db
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo 930000 > /proc/aic3204_reg" );  // 0db
	system_cmd_exec( cmd ); 

}

 

static int audio_in_vol = -1;
static void __audio_in_vol(int val)
{
	char cmd[64] = {0};
	
	if(audio_in_vol == val) return;
	sprintf(cmd, "echo %02x00%02x > /proc/aic3204_reg" ,AIC32X4_LADCVOL, g_pInputVolume[ val ] );
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo %02x00%02x > /proc/aic3204_reg" , AIC32X4_RADCVOL, g_pInputVolume[ val ] );
	system_cmd_exec( cmd ); 
	audio_in_vol = val;
}

static int LO_out_vol = -1;
static int HP_out_vol = -1;

static void __audio_out_vol(int val, int bLoOut)
{
	char cmd[64] = {0};
	if( bLoOut ) {
		if(LO_out_vol == val) return;
		sprintf(cmd, "echo %02x00%02x > /proc/aic3204_reg"  ,AIC32X4_LOLGAIN,  g_pOutputVolume[ val ]);
		system_cmd_exec( cmd ); 
		sprintf(cmd, "echo %02x00%02x > /proc/aic3204_reg"  , AIC32X4_LORGAIN, g_pOutputVolume[ val ]);
		system_cmd_exec( cmd ); 
		LO_out_vol = val;
	}
	else {
		if(HP_out_vol == val) return;
		sprintf(cmd, "echo %02x00%02x > /proc/aic3204_reg"  ,AIC32X4_HPLGAIN, g_pPHLOVolume[ val ]);
		system_cmd_exec( cmd ); 
		sprintf(cmd, "echo %02x00%02x > /proc/aic3204_reg"  , AIC32X4_HPRGAIN, g_pPHLOVolume[ val ]);
		system_cmd_exec( cmd ); 
		HP_out_vol = val;
	}
	

}

static int voice_channel_sel = -1;
 void voice_outChannel_set(int bLoOutMode)  // line out   or   headphone out
{
	 if( bLoOutMode )
		gpio_set( GPO_LOLR_SPK,  LOLR_UNMUTE);

	if( voice_channel_sel == bLoOutMode)
		return;
	voice_channel_sel = bLoOutMode;
	 char cmd[64] = {0};
	 
	//关闭或开启dac，lol，lor通道
	sprintf(cmd, "echo 8e00%02x > /proc/aic3204_reg" , bLoOutMode ? 8 : 0 );//Left Channel DAC reconstruction filter output is routed to LOL
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo 8f00%02x > /proc/aic3204_reg" , bLoOutMode ? 8 : 0 );//Right Channel DAC reconstruction filter output is routed to LOR
	system_cmd_exec( cmd ); 

	//关闭或开启dac， hpl，hpr通道
	sprintf(cmd, "echo 8c00%02x > /proc/aic3204_reg" , bLoOutMode ? 0 : 8 );//Left Channel DAC reconstruction filter's positive terminal is routed to HPL
	system_cmd_exec( cmd ); 
	sprintf(cmd, "echo 8d00%02x > /proc/aic3204_reg" , bLoOutMode ? 0 : 8 );//Right Channel DAC reconstruction filter's positive terminal is routed to HPR
	system_cmd_exec( cmd );

}
 

void myscene_calling::volume_set(  )
{
	if( sip_ua_1->m_task_type == SIP_TASK_TYPE_BC_INCOMING ||	sip_ua_1->m_task_type == SIP_TASK_TYPE_BC_OUTGOING){
		__audio_out_vol( cfg.broadcast_volume_out ,cfg.mode_out == LOL_OUTMODE);
		voice_outChannel_set( cfg.broadcast_mode_out == LOL_OUTMODE);
	}
	else if( sip_ua_1->m_task_type == SIP_TASK_TYPE_TALK_INCOMING || 	sip_ua_1->m_task_type == SIP_TASK_TYPE_TALK_OUTGOING) {
		__audio_out_vol( cfg.volume_out ,cfg.mode_out == LOL_OUTMODE);
		__audio_in_vol(  cfg.volume_in );
		voice_outChannel_set( cfg.mode_out == LOL_OUTMODE);
	}

}
/*
o 报警输入		 
\由终端或工作站发送		 
\消息格式 -------- [COMMAND];cmd_type=alarm_in;operation=query|reply|report|control;alarm_no=1;alarm_status=0			
\operation --------------------- 操作方式			  
	\report -------------------- 终端主动报告 			
	\query --------------------- 工作站查询 			
	\control ------------------- 工作站控制 			
	\reply --------------------- 终端被动回应查询或控制 		 
\alarm_no ---------------------- 报警输入/输出编号 			
	\1~99 ---------------------- 普通开路闭路报警(输入) 			
	\101~119 ------------------- 防拆报警(输入) 			
	\121~139 ------------------- 回路检测(输入,含8507,8521) 			
	\141~159 ------------------- 喧哗报警(输入) 			
	\161~179 ------------------- 巡更(输入) 		 
\alarm_status ------------------ 报警状态(查询时此参数无效)			 
	\0 ------------------------- 关/断开/故障			   
	\1 ------------------------- 开/闭合/正常	o 报警输出		  

o 报警输出 
	\由终端或工作站发送 
	\消息格式 -------- [COMMAND];cmd_type=alarm_out;operation=query|reply|report;alarm_no=1;alarm_status=0 
	  \operation --------------------- 操作方式(各参数值同报警输入) 
	  \alarm_no ---------------------- 报警输入/输出编号 
		 \501~599 ------------------- 普通开路闭路报警(输出) 
		 \601~619 ------------------- 功放控制(输出) 
		 \621~639 ------------------- 分区控制(输出,4~8分区) 


	*/
	//cmd_type   operation   alarm_no
void sip_event_message_new(eXosip_event_t *p_event,myscene_calling * pMysceneCalling)
{
	osip_body_t				*p_body = NULL;
	char					*p_cmd_type = NULL;
	char					*p_param1 = NULL, *p_param2 = NULL, *p_param3 = NULL;
	int						data1, data2;

	if (MSG_IS_MESSAGE(p_event->request) || MSG_IS_NOTIFY(p_event->request))
	{
		osip_message_get_body(p_event->request, 0, &p_body);
		if (p_body != NULL && p_body->body != NULL)
		{
			do
			{
				if (strncmp(p_body->body, "[COMMAND]", strlen("[COMMAND]")) != 0)
					break;
				p_cmd_type = strstr(p_body->body, "cmd_type");
				if (p_cmd_type == NULL)
					break;
				p_cmd_type += strlen("cmd_type") + 1;
				if (strncmp(p_cmd_type, "alarm_out", strlen("alarm_out")) == 0)
				{
					p_param1 = strstr(p_body->body, "operation");
					p_param2 = strstr(p_body->body, "alarm_no");
					if (p_param1 == NULL || p_param2 == NULL)
						break;
					p_param1 += strlen("operation") + 1;
					p_param2 += strlen("alarm_no") + 1;
					if (strncmp(p_param1, "control", strlen("control")) == 0)
					{
						p_param3 = strstr(p_body->body, "alarm_status");
						if (p_param3 == NULL)
							break;
						p_param3 += strlen("alarm_status") + 1;
						data1 = atoi(p_param2);
						data2 = atoi(p_param3);
						if ((data1 / 100) == 5)
						{
							data1 %= 100;
							gpio_set( GPO_SHORT_ALARM,	 data2 ? 1: 0);
							printf_log(LOG_IS_INFO, "		---short alarm out (%d)--	\n",data2);
						}
					}
				}
				else if (strncmp(p_cmd_type, "volume", strlen("volume")) == 0)
				{
					p_param1 = strstr(p_body->body, "broadcast_output_volume");
					if (p_param1 != NULL)
					{
						p_param1 += strlen("broadcast_output_volume") + 1;
						cfg.broadcast_volume_out = atoi(p_param1);
						pMysceneCalling->volume_set(  );
						printf_log(LOG_IS_INFO, "		---cfg.broadcast_volume_out (%d)--	\n",cfg.broadcast_volume_out);
					}
					p_param1 = strstr(p_body->body, "talk_input_volume");
					p_param2 = strstr(p_body->body, "talk_output_volume");
					if (p_param1 != NULL && p_param2 != NULL)
					{
						p_param1 += strlen("talk_input_volume") + 1;
						p_param2 += strlen("talk_output_volume") + 1;
						cfg.volume_in= atoi(p_param1);
						cfg.volume_out= atoi(p_param2);
						pMysceneCalling->volume_set(  );
						
						printf_log(LOG_IS_INFO, "		---cfg.volume_in (%d)--	\n",cfg.volume_in);
						printf_log(LOG_IS_INFO, "		---cfg.volume_out (%d)--	\n",cfg.volume_out);
					}
				}
			} while (0);
		}
	}
}


void sipEvent_callback(eXosip_event_t *p_event, void *pParam)
{
	static int failCount = 0;
	static int b_verifyTime = 0;
	myscene_calling * pMysceneCalling = ( myscene_calling * )pParam;
	if (pParam != NULL && p_event != NULL)
	{
		switch (p_event->type)
		{
		case EXOSIP_REGISTRATION_SUCCESS:
			printf_log(LOG_IS_INFO, "EXOSIP_REGISTRATION_SUCCESS\n");
			sip_reg_success = 1;
			if( !b_verifyTime ){
				char			p_time[64];
				strftime(p_time, sizeof(p_time), "date -s %Y.%m.%d-%X ", &pMysceneCalling->sip_ua_1->m_server_date);
				printf_log(LOG_IS_INFO, "		time set:%s\n",p_time);
    			system_cmd_exec( p_time );
				b_verifyTime = 1;
			}
			failCount = 0;
			break;
		case EXOSIP_REGISTRATION_FAILURE:
			char			p_time[64];
			if( failCount > 3)
				sip_reg_success = 0;
			++failCount ;
			break;
		case EXOSIP_CALL_RINGING:
			printf_log(LOG_IS_INFO, "EXOSIP_CALL_RINGING\n");
			break;
		case EXOSIP_CALL_REQUESTFAILURE:
			pMysceneCalling->bt_hangupClicked();
			break;
		case EXOSIP_CALL_ANSWERED:
		case EXOSIP_CALL_ACK:
			printf_log(LOG_IS_INFO, "EXOSIP ANSWERED ACK\n");
			pMysceneCalling->inCall(  );//use change sip talk type to cancel timer event
			break;
		case EXOSIP_CALL_INVITE:
			pMysceneCalling->SipTalkType = TS_INCOMING_TALK;
			if( pMysceneCalling->pmv->b_screenSaver )  //唤醒
					pMysceneCalling->pmv->setScreenSaver(0);
			pMysceneCalling->label_calling->setText("INCOME CALL... ...");
			pMysceneCalling->pmv->changeWindowType(WINDOW_TYPE_CALLING);
			if( (pMysceneCalling->sip_ua_1->m_task_type != SIP_TASK_TYPE_BC_INCOMING) &&
				(pMysceneCalling->sip_ua_1->m_task_type != SIP_TASK_TYPE_MONITOR_INCOMING)  ) {  //对讲
				pMysceneCalling->bAnswer_show( 1 ); 
				if( cfg.talk_auto_answer ) {
					if( cfg.talk_auto_answer_time > 0) {
						//pMysceneCalling->emitSignalAutoAnswer();  
						pMysceneCalling->m_auto_answer_count = cfg.talk_auto_answer_time;
						printf_log(LOG_IS_INFO, "\tstart emit Signal AutoAnswer\n");
					}
					else { //talk_auto_answer_time <=0, answer call now
						pMysceneCalling->bt_answerCallClicked() ;
						printf_log(LOG_IS_INFO, "EXOSIP CALL INVITE,auto ANSWER SUCCESS\n");
					}
				}
				else
					printf_log(LOG_IS_INFO, "\trecev call \n");
			}  //广播、监听
			else {
				pMysceneCalling->bAnswer_show( 0 ); 
				pMysceneCalling->bt_answerCallClicked() ;
			}				
			
			break;
		case EXOSIP_CALL_RELEASED :  //EXOSIP_CALL_CLOSED
		case EXOSIP_CALL_CLOSED:
			pMysceneCalling->bt_hangupClicked();
			break;
		case EXOSIP_MESSAGE_NEW:
			sip_event_message_new( p_event ,pMysceneCalling);
			break;
		default:
			printf_log(LOG_IS_INFO, "other EXOSIP Event:%d\n" , p_event->type);
			break;
		}
	}
}


void  myscene_calling::bAnswer_show(int bAnswerShow)
{
	int showFlag = bAnswerShow + (cfg.accessing_talk_hangup ? 2 : 0);
	printf_log(LOG_IS_INFO, "  	bAnswer show:%s , %s\n" , showFlag&1 ? "answer show":"answer hide",\
		showFlag&2 ? "hangup show":"hangup hide");
	switch (showFlag)
	{
		case 3://show two button
			bt_answerCall->setVisible(true); 
			//bt_answerCall->move(920,400);
			bt_hangup->setVisible(true); 
			//bt_hangup->move(920,200);

			break;
		case 1://show Answer  button
			bt_answerCall->setVisible(true); 
			//bt_answerCall->move(920,300);
			bt_hangup->setVisible(false); 
			break;
		case 2://show hangup  button
			bt_answerCall->setVisible(false); 
			bt_hangup->setVisible(true); 
			//bt_hangup->move(920,300);
			break;
		case 0://all  button hide
			bt_answerCall->setVisible(false); 
			bt_hangup->setVisible(false); 
			break;
	}

}

void  myscene_calling::widget_init()
{
	qDebug() << "myscene_calling widget_init"; 
	SipTalkType = TS_IDLE;
	pid_capture = 0;

    QGraphicsProxyWidget * proxy ;
	aic3204_init( ) ;

	label_back = new QLabel();
	label_back->setAttribute(Qt::WA_TranslucentBackground); 
    label_back->setPixmap(QPixmap(":/pic/main.bmp"));//:/pic/input_dp_1.bmp
	label_back->setGeometry(0,0,SCREEN_WID,SCREEN_HEIGHT);
	this->addWidget(label_back);

	label_backCircle = new QLabel();
	label_backCircle->setAttribute(Qt::WA_TranslucentBackground); 
    label_backCircle->setPixmap(QPixmap(":/pic/calling.bmp"));
	label_backCircle->move(200,100);
	this->addWidget(label_backCircle);

	QPalette pe;
	pe.setColor(QPalette::WindowText,Qt::white);

	
	label_calling = new QLabel("Calling... ...");
	//pe.setColor(QPalette::WindowText,Qt::white);
	label_calling->setPalette(pe);
	label_calling->setAttribute(Qt::WA_TranslucentBackground); 
	label_calling->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*(0.5+ g_fontResize)) );
	label_calling->setGeometry(290,370,250,250);
	proxy = this->addWidget(label_calling);
	proxy->setRotation(-90);

	
	QPixmap pixmap;

    bt_hangup = new QPushButton;
  	bt_hangup->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*(0.5+ g_fontResize)) );
    bt_hangup->move(920,400);
   	bt_hangup->setText("STOP"); 
	proxy = this->addWidget(bt_hangup);
	proxy->setRotation(-90);

	bt_answerCall = new QPushButton;
  	bt_answerCall->setFont( QFont(FONE_NAME, TIME_DATE_FONTSIZE*(0.5+ g_fontResize)) );
    bt_answerCall->move(920,200);
   	bt_answerCall->setText("answer"); 
	proxy = this->addWidget(bt_answerCall);
	proxy->setRotation(-90);

	connect( bt_hangup ,SIGNAL(clicked( )), this, SLOT(bt_hangupClicked( )));
	connect( bt_answerCall ,SIGNAL(clicked( )), this, SLOT(bt_answerCallClicked( )));

	m_auto_answer_count = 0;
	m_nTimeAutoAnswer = startTimer(  1000); 

	sip_ua_1 = NULL;
	sip_init_once( cfg.sip_ip.toLatin1().data() );
	bExitCaptureThread = 0;

}


int myscene_calling::sip_init_once( char * sipServerIp)
{
	struct timeval		ts;
	CSocketEx			socket_spon;
	char				p_file[PATH_MAX];
	string				str_file;

	if( (sip_ua_1 != NULL) )
		return -1;

	printf("serverip:%s, \n",sipServerIp);
	cap_pic_width = cfg.xres; 
	cap_pic_height = cfg.yres;
	crop_pic_width = cfg.xres;   //cfg.screen_xres +8; 
	crop_pic_height = cfg.yres; //cfg.screen_yres;
	
	sip_ua_1 = new CSipUA;
	
/*	sip_ua_1->set_local_addr(socket_spon.get_first_hostaddr(), SIP_LOCAL_PORT + atoi(localId));
	sip_ua_1->set_register_addr(sipServerIp, SIP_SERVER_PORT);
	sip_ua_1->set_username_password(localId, SIP_SERVER_PASSWORD);*/

	sip_ua_1->set_local_addr(socket_spon.get_first_hostaddr(), cfg.sip_port );//SIP_LOCAL_PORT
	sip_ua_1->set_register_addr(sipServerIp, cfg.sip_server_port);//SIP_SERVER_PORT
	sip_ua_1->set_username_password(cfg.sip_username.toLatin1().data(), cfg.sip_passwd.toLatin1().data());//SIP_SERVER_PASSWORD
	
	sip_ua_1->m_audio_stream.set_enable_sound_card(TRUE);
	sip_ua_1->set_enable_rtp_video(TRUE);
	sip_ua_1->init();
	sip_ua_1->m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);

	ua_usleep(500000);
	get_exe_path(p_file, PATH_MAX);
	str_file = p_file;
	str_file = str_file.substr(0, str_file.find_last_of('\\') + 1);

    ua_get_time(&ts);

	// 添加文件列表
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

	if (h264_enc_init ( crop_pic_width, crop_pic_height,  25) < 0   ) {	//352, 288,      640, 480,
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
			//capture width,height,   crop width  , height
			if (video_capture_init( cap_pic_width, cap_pic_height, crop_pic_width, crop_pic_height,  25) >= 0 ) {
				if (pthread_create(&pid_capture, NULL, capture_thread, this) >= 0)
				{
					//sip_ua_1->set_enable_rtp_video(TRUE);
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

int myscene_calling::startCall( char * dstId ,talk_type_t talk_type )
{
	if( SipTalkType ){
		printf_log(LOG_IS_INFO,"start Call error:%d\n", SipTalkType);
		return SipTalkType;
	}
	if( (sip_ua_1 == NULL) ){
		printf_log(LOG_IS_ERR,"error: sip_ua_1 is NULL\n");
		return -1;
	}
	bAnswer_show( 0 );
	
	SipTalkType = TS_RING;
	label_calling->setText("CALLING... ...");
	qDebug() << " 		 		---start Call to---  :" << dstId;

	startCapture(  );
	
	if( sip_ua_1->talk(dstId, NULL, NULL) ){
		SipTalkType = talk_type; //video_init_error ? CALL_AUDIO_OUT: CALL_VIDEO_OUT;   //should modify to justify video or audio
		//add timeout event
		volume_set( );
		m_nTimeoutId = startTimer(CALL_TIMEOUT);  
		printf_log(LOG_IS_INFO,"Start Call timeout begin\n");
		
	}
	else {
		SipTalkType = TS_IDLE;
		label_calling->setText("");
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
	bt_answerCall->setVisible(false); 
	SipTalkType = TS_TALKING;  //  CALL_AUDIO_TALK     CALL_VIDEO_TALK;
	pmv->screenSaverStartTime = QDateTime::currentMSecsSinceEpoch();
}


//callee
void myscene_calling::bt_answerCallClicked()
{
	printf_log(LOG_IS_INFO, " 		---soft key of answer--   \n");
	volume_set( );
	
	startCapture(	);
	if( sip_ua_1->answer() ){
		inCall(  );
		return ;
	}
	return ;

}
int myscene_calling::stopCall()
{
	if( (sip_ua_1 != NULL) && SipTalkType){
		SipTalkType = TS_IDLE;
		if( bCaptureThread == 1 ) {
			bExitCaptureThread = 1;
			while( 	bExitCaptureThread )  //wait thread to exit
				usleep(100);
		}
		if( cfg.mode_out == LOL_OUTMODE)
			gpio_set( GPO_LOLR_SPK,	 LOLR_MUTE);
		sip_ua_1->set_video_recvdata_callback(NULL, NULL);
		sip_ua_1->task_end();
		pmv->screenSaverStartTime = QDateTime::currentMSecsSinceEpoch();
		qDebug() << " 		 		---stop Call--   \n";
		
		if( list_call_id >= 0 ){
			if(cfg.list_link[list_call_id]){  //listcall alarm enable
				gpio_set( GPO_SHORT_ALARM,	 cfg.short_o1_normal_mode );
				printf_log(LOG_IS_INFO, " 		---short alarm out end--   \n");
			}
			list_call_id = -1;
		}
	}
	else{
		printf("sip_ua_1 is %x, or not in talk:%d\n",sip_ua_1,SipTalkType); 
		return -1;
	}

	/*
	h264_dec_uninit( h264_dec_ctx );
	*/

}
	

void  myscene_calling::bt_hangupClicked()
{
	stopCall();
	pmv->changeWindowType( pmv->topWindowType );
	qDebug() << "bt_hangupClicked" << cfg.ip_keyleft  << cfg.port_keyleft;
}


	

myscene_calling::myscene_calling(MyView * pm,QObject *parent) :  
	QGraphicsScene(parent)	
{  
    clearFocus();
	pmv = pm;

    widget_init();

} 
	/*
void myscene_calling::start_autoAnser_timer(   )
{
	m_nTimeAutoAnswer = startTimer(cfg.talk_auto_answer_time * 1000); 
}*/

void myscene_calling::timerEvent( QTimerEvent *event )

{
	if( event->timerId() == m_nTimeoutId)
	{
		killTimer( m_nTimeoutId ); 
		if( SipTalkType != TS_TALKING) {
			printf_log(LOG_IS_INFO, "Call Timeout, auto stop Call\n");
			#ifndef LOCAL_VIDEO_DISPLAY
			bt_hangupClicked();
			#endif
		}
		
	}
	else if( event->timerId() == m_nTimeAutoAnswer)
	{
		//killTimer( m_nTimeAutoAnswer ); 
		
		if( m_auto_answer_count > 0){
			--m_auto_answer_count ;
			if( m_auto_answer_count == 0){
				bt_answerCallClicked() ;
				printf_log(LOG_IS_INFO, "timerEvent auto ANSWERed\n");
			}
		}
	}

} 


     

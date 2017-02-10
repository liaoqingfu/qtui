#include "myview.h"  
#include <QKeyEvent>  

#include "api/misc.h"

#define HARD_KEY_TRIG  1

#define  LED_SLEEP_TIME		(1000*200)

#define  LED_BLINK_FAST		((1000*200) / LED_SLEEP_TIME)
#define  LED_BLINK_NORMAL		((1000*600) / LED_SLEEP_TIME)
#define  LED_BLINK_SLOW		((1000*1000) / LED_SLEEP_TIME)

ncs_cfg_t cfg;
extern int 	sip_reg_success;

//cfg.short_i1_alarm_mode     cfg.short_o1_normal_mode

void MyView::gpio_init(  )
{
	gpio_open(GPI_LEFT_KEY,  	 GPIO_IN); 
	gpio_open(GPI_RIGHT_KEY, 	 GPIO_IN); 
	gpio_open(GPI_SHORT_ALARM , GPIO_IN);  
	
	gpio_open(GPO_LED_CALL ,    GPIO_OUT); 
	gpio_open(GPO_LED_WARNING , GPIO_OUT);  
	gpio_open(GPO_LOLR_SPK , GPIO_OUT);  
	gpio_set( GPO_LOLR_SPK,	 LOLR_MUTE);
	gpio_open(GPO_SHORT_ALARM , GPIO_OUT);  
	gpio_set( GPO_SHORT_ALARM,	 cfg.short_o1_normal_mode);
}
void led_display(int state)
{
	static int led_flag = 0; 
	led_flag = led_flag ? 0 : 1; 
	static int count = 0;
	
	switch(state) {
        case  LED_OFFLINE:
            //把有按键灯慢闪
            if( count >= LED_BLINK_NORMAL ) {
	            gpio_set( GPO_LED_CALL,led_flag);
	            gpio_set( GPO_LED_WARNING,led_flag);
            }
            break ;
        case  LED_ONLINE:
            if( count >= LED_BLINK_SLOW ) {
	            gpio_set( GPO_LED_CALL,LED_ON);
	            gpio_set( GPO_LED_WARNING,LED_ON);
            }
            break;
        case  LED_BROADCAST:
            //把有按键灯快闪
			if( count >= LED_BLINK_FAST ) {
				gpio_set( GPO_LED_CALL,led_flag);
            	gpio_set( GPO_LED_WARNING,led_flag);
			}
            break;
		case  LED_TALK:
            if( count >= LED_BLINK_FAST ) {
				gpio_set(GPO_LED_CALL,led_flag);
            	gpio_set(GPO_LED_WARNING,led_flag);
            } 
            break;
        default: //case  LED_RING:
            if( count >= LED_BLINK_NORMAL ) {
				gpio_set(GPO_LED_CALL,led_flag);
            	gpio_set(GPO_LED_WARNING,led_flag);
            } 
            break;
		break;
	}
	++count;
}
/*

void short_in_alarm()
{
	char				p_op_report[] = "report", p_op_reply[] = "reply";
	char				*p_op_sel = p_op_report;

	if (p_msg->param1 == 0) 								// 防拆
		sprintf(p_alarm_no, "%d", 101);
	else if (p_msg->param1 >= 1 && p_msg->param1 <= 0x7D)	// 报警输入端口
		sprintf(p_alarm_no, "%d", p_msg->param1);
	else if (p_msg->param1 == 0x7E) 						// 喧哗
		sprintf(p_alarm_no, "%d", 141);
	else if (p_msg->param1 == 0x7F) 						// 巡更
		sprintf(p_alarm_no, "%d", 161);
	else if (p_msg->param1 >= 0x81 && p_msg->param1 <= 0xFF)// 报警输出端口
	{
		sprintf(p_alarm_no, "%d", p_msg->param1 - 0x81 + 501);
		p_op_sel = p_op_reply;
	}
	if (p_msg->param2 == 0)
		sprintf(p_status, "%d", 0);
	else
		sprintf(p_status, "%d", 1);

	for (i = 0; i < SIP_UA_MESSAGE_TARGET_MAX && m_sip_ua.m_message_target[i] != 0; i++)
	{
		sprintf(p_id, "%d", m_sip_ua.m_message_target[i]);
		m_sip_ua.send_msg_alarm_in(p_id, p_op_sel, p_alarm_no, p_status);
	}

}*/

//key detect ,  led flash
void * keyTrig_led_thread(void * pParam)
{
	static int talkStatus;
	static int b_shortAlarm = 0, short_i1_alarm_status_prev = gpio_get( GPI_SHORT_ALARM );
	int 		short_i1_alarm_status_cur;
	MyView * pMyView = ( MyView * )pParam;
	char				p_alarm_no[5];
	char				p_status[5];
	char	p_op_report[] = "report";
	char	p_id[ SIP_UA_USERNAME_LEN ];
	int  leftKeyValue, rightKeyValue;
	printf_log(LOG_IS_INFO, " Start keyDetect thread \n");
	while(1)
	{
		talkStatus = pMyView->scene_calling->SipTalkType;
		if( talkStatus == TS_IDLE ) {
			#if  HARD_KEY_TRIG
			if( gpio_get( GPI_LEFT_KEY ) == 0)
				pMyView->scene_main->bt_leftCallClicked();

			if( gpio_get( GPI_RIGHT_KEY ) == 0)
				pMyView->scene_main->bt_rightCallClicked();

			#endif
			//short In port detect
			short_i1_alarm_status_cur = gpio_get( GPI_SHORT_ALARM ) ? 1 : 0;
			if( short_i1_alarm_status_cur != short_i1_alarm_status_prev )
			{
				short_i1_alarm_status_prev = short_i1_alarm_status_cur;
				int short_i1_value = cfg.short_i1_alarm_mode ? 1 : 0;
				if( short_i1_value != short_i1_alarm_status_cur )
					b_shortAlarm = 1;
				else
					b_shortAlarm = 0;
				
				for (int i = 0; i < SIP_UA_MESSAGE_TARGET_MAX && pMyView->scene_calling->sip_ua_1->m_message_target[i] != 0; i++)
				{
					sprintf(p_id, "%d", pMyView->scene_calling->sip_ua_1->m_message_target[i]);
					sprintf(p_alarm_no, "%d", 1);
					sprintf(p_status, "%d", b_shortAlarm);
					pMyView->scene_calling->sip_ua_1->send_msg_alarm_in(p_id, p_op_report, p_alarm_no, p_status);  //p_alarm_no, p_status
				}
				printf_log(LOG_IS_INFO, "		---short alarm detected(%s)(%d,%d)--	\n", \
					b_shortAlarm ? "trig" : "recover", cfg.short_i1_alarm_mode, short_i1_alarm_status_cur);
			}
			
			led_display( sip_reg_success );   //sip_reg_success ? LED_ONLINE : LED_OFFLINE
		}
		else {
			if( talkStatus == TS_INCOMING_TALK  || ((talkStatus == TS_TALKING) && cfg.accessing_talk_hangup )){
				leftKeyValue = gpio_get( GPI_LEFT_KEY );
				rightKeyValue = gpio_get( GPI_RIGHT_KEY ) ;
				if( (leftKeyValue == 0) | (rightKeyValue == 0)){
					talkStatus == TS_INCOMING_TALK ? pMyView->scene_calling->bt_answerCallClicked() : pMyView->scene_calling->bt_hangupClicked();
				}
			}
			led_display( talkStatus );  
		}
		usleep( LED_SLEEP_TIME );
	}
	printf_log(LOG_IS_INFO, " Exit keyDetect thread \n");
	return NULL;
}

MyView::~MyView( )
{
	qDebug() << "~MyView exit"; 

}

MyView::MyView(QWidget *parent) :  
	QGraphicsView(parent)  
{  
	ReadAllSettings( );

    WindowType = WINDOW_TYPE_MAIN;

    scene_video = new myscene_video( this );
	scene_video->setSceneRect(0,0,SCREEN_WID,SCREEN_HEIGHT);
	
	scene_calling = new myscene_calling( this );
	scene_calling->setSceneRect(0,0,SCREEN_WID,SCREEN_HEIGHT);

	scene_list = new myscene_list( this );
	scene_list->setSceneRect(0,0,SCREEN_WID,SCREEN_HEIGHT);

	scene_pic = new myscene_pic( this );
	scene_pic->setSceneRect(0,0,SCREEN_WID,SCREEN_HEIGHT);

    scene_num_call = new myscene_num_call( this );
	scene_num_call->setSceneRect(0,0,SCREEN_WID,SCREEN_HEIGHT);
	
    scene_main = new myscene_main( this );
	scene_main->setSceneRect(0,0,SCREEN_WID,SCREEN_HEIGHT);//(0,0,(static_cast<QWidget *>600),(static_cast<QWidget *>600) );
	
	this->setScene(scene_main);
	gpio_init(  );
	
	m_nTimerId = startTimer(1000);  
	timerEvent( new QTimerEvent(m_nTimerId) ) ;
	if (pthread_create(&pid_keyDetect, NULL, keyTrig_led_thread, this) != 0)
		printf("keyDetect thread  create error:%s\n", strerror(errno));
}  

void MyView::timerEvent( QTimerEvent *event )

{
	QTime qtimeObj = QTime::currentTime();
	static int minute = -1;
	static int netStatusChanged = -1;
 {
		QString str;
		str.sprintf("%02d:%02d",qtimeObj.hour(),qtimeObj.minute() ); 

		if(netStatusChanged != sip_reg_success){
			netStatusChanged = sip_reg_success;
			scene_num_call->label_net_status->setPixmap( sip_reg_success ? QPixmap(":/pic/online.bmp") : QPixmap(":/pic/offline.bmp") );
			scene_list->label_net_status->setPixmap( sip_reg_success ? QPixmap(":/pic/online.bmp") : QPixmap(":/pic/offline.bmp") );
			scene_pic->label_net_status->setPixmap( sip_reg_success ? QPixmap(":/pic/online.bmp") : QPixmap(":/pic/offline.bmp") );
		}
			
		if( (minute != -1) )
		{
			switch ( WindowType )
			{
				case WINDOW_TYPE_MAIN:	
					scene_main->label_time->setText(str);
					scene_main->label_date->setText(QDate::currentDate().toString(tr("yyyy-MM-dd dddd")));  
					break;
				case WINDOW_TYPE_NUM_CALL:	
					scene_num_call->label_time->setText(str);
					scene_num_call->label_date->setText(QDate::currentDate().toString(tr("dddd")));  
					break;
				case WINDOW_TYPE_LIST_CALL:  
					scene_list->label_time->setText(str);
					scene_list->label_date->setText(QDate::currentDate().toString(tr("dddd")));  
					break;
				case WINDOW_TYPE_PIC_CALL:
					scene_pic->label_time->setText(str);
					scene_pic->label_date->setText(QDate::currentDate().toString(tr("dddd")));  
					break;

			}
		}
		else{
			scene_main->label_time->setText(str);
			scene_main->label_date->setText(QDate::currentDate().toString(tr("yyyy-MM-dd dddd"))); 
			scene_num_call->label_time->setText(str);
			scene_num_call->label_date->setText(QDate::currentDate().toString(tr("dddd")));  
			scene_list->label_time->setText(str);
			scene_list->label_date->setText(QDate::currentDate().toString(tr("dddd"))); 
			scene_pic->label_time->setText(str);
			scene_pic->label_date->setText(QDate::currentDate().toString(tr("dddd"))); 
		//	scene_main->label_net_status->setPixmap( sip_reg_success ? QPixmap(":/pic/online.bmp") : QPixmap(":/pic/offline.bmp") );
			
		}
		minute = qtimeObj.minute();
	}
} 


void MyView::changeWindowType( int winType )
{

	if( WindowType != winType ) {
		
		switch ( winType )
		{
			case WINDOW_TYPE_MAIN:	
				this->setScene(scene_main);
				break;
				
			case WINDOW_TYPE_CALLING:	
				topWindowType = WindowType;
				this->setScene(scene_calling);
				break;
			case WINDOW_TYPE_NUM_CALL:	
				topWindowType = WINDOW_TYPE_MAIN;
				this->setScene(scene_num_call);
				break;
			case WINDOW_TYPE_LIST_CALL:  
				topWindowType = WINDOW_TYPE_MAIN;
				this->setScene(scene_list);
				break;
			case WINDOW_TYPE_PIC_CALL:
				topWindowType = WINDOW_TYPE_MAIN;
				this->setScene(scene_pic);
				break;
			case WINDOW_TYPE_VIDEO_HALF: 
				if( WindowType < WINDOW_TYPE_VIDEO_HALF )
					topWindowType = WindowType;
				this->setScene(scene_video);
				break;
			case WINDOW_TYPE_VIDEO_FUL:  
				if( WindowType < WINDOW_TYPE_VIDEO_HALF )
					topWindowType = WindowType;
				this->setScene(scene_video);
				scene_video->startVideo();
				break;
			default:
				qDebug() << "<error : unknown WindowType>";
				return ;
		}
	}
	WindowType = winType;
	qDebug() << "		<topW:>" << topWindowType << "<desW:>" <<winType;
}
/*

void MyView::keyPressEvent(QKeyEvent *event)  
{  
   qDebug("*********MyView::keyPressEvent***************");  
	switch (event->key())  
	{  
	case Qt::Key_Left :  
		scale(1.2, 1.2);  
		break;	
	case Qt::Key_Right :  
		scale(1 / 1.2, 1 / 1.2);  
		break;	
	case Qt::Key_Up :  
		rotate(30);  
		break;	
	}  
	QGraphicsView::keyPressEvent(event);  
}  

void MyView::mousePressEvent(QMouseEvent *event)  
{  
    qDebug("***MyView::mousePressEvent*");
	QGraphicsView::mousePressEvent(event);
} 
void MyView::mouseMoveEvent(QMouseEvent *event)  
{  
	//qDebug("************MyView::mouseMoveEvent*****************");  
	QGraphicsView::mouseMoveEvent(event);  
} 

  
void MyView::paintEvent(QPaintEvent *event)  
{  

    QPainter painter(this->viewport());
    painter.setPen( QColor(0x00, 0xb2, 0xee) );//00F5FF QColor(0x00, 0xf5, 0xff)
    painter.setBrush( QColor(0x00, 0xb2, 0xee) );
    painter.drawRect(0,0,60,600);

    painter.drawRect(1024-60,0,1024,600);

     painter.end();
	QGraphicsView::paintEvent(event);  
}  
  */

void MyView::WriteSettings(QString sector, QString sItem,int value)
{
    //QSettings settings("Software Inc", "Spreadsheet"); 
    QSettings settings(CFG_NAME, QSettings::IniFormat); 
    settings.beginGroup(sector);
    settings.setValue(sItem, value);
    settings.endGroup();
/*
	settings.beginGroup("video_cfg");
	settings.setValue( "output_w", 1024); //,output_h;int crop_x,crop_y,crop_w,crop_h;	int rotate;
	settings.endGroup();
	*/
}


QString MyView::getLocalIp()
{

QList<QHostAddress> list = QNetworkInterface::allAddresses();  
    foreach (QHostAddress address, list)  
    {  
        if(address.protocol() == QAbstractSocket::IPv4Protocol)  
        {  
            if (address.toString().contains("127.0."))  
            {  
                continue;  
            }  
           return address.toString();  
        }  
    }  

}
void  cfg_netword_set(char* ip ,char* netmask,char* gw,char* dns1,char* dns2,char* mac_addr)
{
	
    system_cmd_exec("rm -f /etc/resolv.conf");
    if (dns1) system_cmd_exec("echo nameserver %s > /etc/resolv.conf", dns1);
    if (dns2) system_cmd_exec("echo nameserver %s > /etc/resolv.conf", dns2);
    if (mac_addr) system_cmd_exec("ifconfig eth0 hw ether %s", mac_addr);
    if (ip && netmask) system_cmd_exec("ifconfig eth0 %s netmask %s up", ip, netmask);
	if (gw) system_cmd_exec("route add default gw %s", gw);
}

void  mac_addr_get(char* ip,char* mac_addr)
{
    unsigned int ip_addr = ntohl(inet_addr(ip));
    sprintf(mac_addr, "%02x:%02x:%02x:%02x:%02x:%02x", 00, 0x89, (ip_addr >> 24 & 0xff),
        (ip_addr >> 16 & 0xff), (ip_addr >> 8 & 0xff), (ip_addr & 0xff));
}

void MyView::ReadAllSettings( )
{
	int i = 0;
	QString listReadStr;
    QSettings settings(CFG_NAME, QSettings::IniFormat);
	cfg.local_ip = settings.value( "terminal/ip" ).toString();
	cfg.localid = settings.value( "terminal/id" ).toString();
	cfg.netmask = settings.value( "terminal/mask" ).toString();
	cfg.gateway = settings.value( "terminal/gate" ).toString();
	cfg.dns1 = settings.value( "terminal/dns1" ).toString();
	cfg.dns2 = settings.value( "terminal/dns2" ).toString();
	cfg.mac_addr = settings.value( "terminal/mac_addr" ).toString();
	cfg.bDynamicIP = settings.value( "terminal/connection_type" ).toInt();

	 cfg_netword_set(cfg.local_ip.toLatin1().data() ,cfg.netmask.toLatin1().data(),
		cfg.gateway.toLatin1().data() ,cfg.dns1.toLatin1().data(),
		cfg.dns2.toLatin1().data() ,cfg.mac_addr.toLatin1().data());

	cfg.sip_ip = settings.value( "sip/ip" ).toString();
	cfg.sip_username= settings.value( "sip/sip_username" ).toString();
	cfg.sip_passwd= settings.value( "sip/sip_passwd" ).toString();
	
	cfg.sip_port = settings.value( "sip/port" ).toInt();
	cfg.sip_server_port = settings.value( "sip/sip_port" ).toInt();
	cfg.sip_audio_rtp_port = settings.value( "sip/sip_audio_rtp_port" ).toInt();
	cfg.sip_video_rtp_port = settings.value( "sip/sip_video_rtp_port" ).toInt();

	cfg.alone_enable  = settings.value( "alone_cfg/enable" ).toInt();
	cfg.ip_keyleft  = settings.value( "alone_cfg/ip_keyleft" ).toString();
	cfg.ip_keyright = settings.value( "alone_cfg/ip_keyright" ).toString();
	cfg.port_keyleft  = settings.value( "alone_cfg/port_keyleft" ).toInt();
	cfg.port_keyright = settings.value( "alone_cfg/port_keyright" ).toInt();

	cfg.volume_in= settings.value( "talk_cfg/volume_in" ).toInt();
	cfg.volume_out= settings.value( "talk_cfg/volume_out" ).toInt();
	cfg.volume_ring= settings.value( "talk_cfg/volume_ring" ).toInt();
	cfg.mode_in= settings.value( "talk_cfg/mode_in" ).toInt();
	cfg.mode_out= settings.value( "talk_cfg/mode_out" ).toInt();
	cfg.talk_auto_answer= settings.value( "talk_cfg/talk_auto_answer" ).toInt();
	cfg.talk_auto_answer_time = settings.value( "talk_cfg/talk_auto_answer_sec" ).toInt();
	cfg.display_halfScrenn = settings.value( "talk_cfg/display_style" ).toInt();
	
		
	cfg.accessing_talk_hangup = settings.value( "talk_cfg/accessing_talk_hangup" ).toInt();
	cfg.echo= settings.value( "talk_cfg/echo" ).toInt();
	cfg.environment= settings.value( "talk_cfg/environment" ).toInt();
	cfg.display_target= settings.value( "talk_cfg/display_target" ).toInt();

	cfg.broadcast_mode_out= settings.value( "broadcast_cfg/mode_out" ).toInt();
	cfg.broadcast_volume_out= settings.value( "broadcast_cfg/volume_out" ).toInt();

	cfg.monite_volume_in= settings.value( "monite_cfg/environment" ).toInt();
	cfg.monite_mode_in= settings.value( "monite_cfg/mode_in" ).toInt();


	cfg.short_i1_alarm_mode= settings.value( "short_cfg/short_i1_alarm_mode" ).toInt();
	cfg.short_o1_normal_mode= settings.value( "short_cfg/short_o1_normal_mode" ).toInt();
	cfg.enable_removealarm= settings.value( "short_cfg/enable_removealarm" ).toInt();
	cfg.enable_noisealarm= settings.value( "short_cfg/enable_noisealarm" ).toInt();
	cfg.noise_alarm_vol= settings.value( "short_cfg/noise_alarm_vol" ).toInt();
	cfg.noise_alarm_time= settings.value( "short_cfg/noise_alarm_time" ).toInt();

	cfg.screensaver_min= settings.value( "other_cfg/screensaver_min" ).toInt();
	cfg.io_out_pass= settings.value( "other_cfg/io_out_pass" ).toInt();
	cfg.screen_button= settings.value( "other_cfg/screen_button" ).toInt();
	cfg.hdmi_style= settings.value( "other_cfg/hdmi_style" ).toInt();
	

	cfg.display_col= settings.value( "list_cfg/display_col" ).toInt();
	for(i = 0; i < LIST_MAX_NUM; i++){
		listReadStr.sprintf("list_cfg/name%i",i + 1);
		cfg.list_name[i]= settings.value( listReadStr ).toString();
		if( cfg.list_name[i].length() < 1){
			break;
		}
		
		listReadStr.sprintf("list_cfg/target%i",i + 1);
		cfg.list_target[i]= settings.value( listReadStr ).toString();
		
		listReadStr.sprintf("list_cfg/in_num%i",i + 1);
		cfg.list_in_num[i]= settings.value( listReadStr ).toInt();
		
		listReadStr.sprintf("list_cfg/link%i",i + 1);
		cfg.list_link[i]= settings.value( listReadStr ).toInt();

		
		qDebug() <<  cfg.list_name[i] << cfg.list_target[i] << cfg.list_in_num[i] << cfg.list_link[i];
	}
	cfg.list_count = i;
	qDebug() << " list cfg read count: " << i;

	cfg.enable_onvif= settings.value( "video_cfg/enable_onvif" ).toInt();
	cfg.enable_wdr= settings.value( "video_cfg/enable_wdr" ).toInt();
	cfg.resolution = settings.value( "video_cfg/resolution" ).toInt();
	video_image_resolution_get(cfg.resolution, &cfg.xres, &cfg.yres);

	cfg.language= settings.value( "language/language" ).toInt();

	//QString str= "date -s " + year + month + day + hour + minute + "." + second;
    //system_cmd_exec(str.toLatin1().data());
    //强制写入到CMOS
    //system("hwclock -w");
}
/*
ReadSettings( "video_cfg/crop_x", &crop_x);

	memset(enable_str,0,20);
	sprintf(enable_str,"arp_ip%d",i);
	ip_str = (char *)lp_config_get_string(lpconfig, "terminal", enable_str, "255.255.255.255");
	
	memset(enable_str,0,20);
	sprintf(enable_str,"arp_mac%d",i);
	mac_str = (char *)lp_config_get_string(lpconfig, "terminal", enable_str, "FFFFFFFFFFFF");

	system_cmd_exec("arp -s %s %s", ip_str,mac_str);


	dns1 = (char *)lp_config_get_string(lpconfig, "terminal", "dns1", "192.168.1.1");
	dns2 = (char *)lp_config_get_string(lpconfig, "terminal", "dns2", "192.168.1.1");
	ip = (char *)lp_config_get_string(lpconfig, "terminal", "ip", "192.168.1.101");
	netmask = (char *)lp_config_get_string(lpconfig, "terminal", "mask", "255.255.255.0");
	gw = (char *)lp_config_get_string(lpconfig, "terminal", "gate", "192.168.1.1");
	cfg->is_dynamic_ip = lp_config_get_int(lpconfig, "terminal", "is_dynamic_ip", 0);
	cfg->video_port = lp_config_get_int(lpconfig, "terminal", "port_video", 2058);
	temp_addr = lp_config_get_string(lpconfig, "terminal", "mac_addr", "00:89:C0:A8:01:65");
	
	if (cfg->is_dynamic_ip) {
		SPON_LOG_INFO("NOT SUPPORT DHCP\n");
		 cfg_netword_set(NULL ,NULL,gw,gw,gw,cfg->mac_addr);

	system_cmd_exec("rm -f /etc/resolv.conf");
    if (dns1) system_cmd_exec("echo nameserver %s > /etc/resolv.conf", dns1);
    if (dns2) system_cmd_exec("echo nameserver %s > /etc/resolv.conf", dns1);
    if (mac_addr) system_cmd_exec("ifconfig eth0 hw ether %s", mac_addr);
    if (ip && netmask) system_cmd_exec("ifconfig eth0 %s netmask %s up", ip, netmask);
	if (gw) system_cmd_exec("route add default gw %s", gw);



	cfg->talk_response_hangup = lp_config_get_int(lpconfig, "talk_cfg", "accessing_talk_hangup", 1);
	cfg->talk_request_hangup = lp_config_get_int(lpconfig, "talk_cfg", "requesting_talk_hangup", 1);
    //3091v在呼入和呼出时允许挂断打补丁
    cfg->talk_request_hangup = cfg->talk_response_hangup;

	cfg->talk_board_enable_join = lp_config_get_int(lpconfig, "talk_cfg", "talk_board_enable_join", 1);
	cfg->talk_auto_answer = lp_config_get_int(lpconfig, "talk_cfg", "talk_auto_answer", 1);
    cfg->rec_dist = lp_config_get_int(lpconfig, "talk_cfg", "environment", 1);
    cfg->talk_echo = lp_config_get_int(lpconfig, "talk_cfg", "echo", 1);
    cfg->talk_in_sel = lp_config_get_int(lpconfig, "talk_cfg", "mode_in", 0);
    cfg->talk_in_vol = lp_config_get_int(lpconfig, "talk_cfg", "volume_in", 8);
    cfg->talk_out_sel = lp_config_get_int(lpconfig, "talk_cfg", "mode_out", 0);
    cfg->talk_out_vol = lp_config_get_int(lpconfig, "talk_cfg", "volume_out", 8);
    cfg->talk_ring_vol = lp_config_get_int(lpconfig, "talk_cfg", "volume_ring", 8);
    cfg->broad_out_sel = lp_config_get_int(lpconfig, "broadcast_cfg", "mode_out", 0);
    cfg->broad_out_vol = lp_config_get_int(lpconfig, "broadcast_cfg", "volume_out", 8);
	cfg->line_out_type = lp_config_get_int(lpconfig, "broadcast_cfg","mode_audio_out",0);
    cfg->monitor_sel = lp_config_get_int(lpconfig, "monite_cfg", "mode_in", 0);
    cfg->monitor_vol = lp_config_get_int(lpconfig, "monite_cfg", "volume_in", 8);

	cfg->id = lp_config_get_int(lpconfig, "terminal", "id", 3);
	cfg->local_port = lp_config_get_int(lpconfig, "terminal", "port", 2046);
	
	cfg->dissmbl_alarm_enable = lp_config_get_int(lpconfig, "short_cfg", "enable_removealarm", 1);
	cfg->noise_alarm_enable = lp_config_get_int(lpconfig, "short_cfg", "enable_noisealarm", 0);

	cfg->sip_module_enable = lp_config_get_int(lpconfig, "sip", "enable", 0);
	cfg->sip_port = lp_config_get_int(lpconfig, "sip", "sip_port", 0);
	cfg->sip_audio_rtp_port = lp_config_get_int(lpconfig, "sip", "sip_audio_rtp_port", 0);
	cfg->sip_video_rtp_port = lp_config_get_int(lpconfig, "sip", "sip_video_rtp_port", 0);
	cfg->sip_server_port = lp_config_get_int(lpconfig, "sip", "port", 0);

	result = (char *)lp_config_get_string(lpconfig, "sip", "target", "999.");
	safe_copy(cfg->sip_target_str,result);
	result = (char *)lp_config_get_string(lpconfig, "sip", "sip_username", "999");
	safe_copy(cfg->sip_username,result);
	result = (char *)lp_config_get_string(lpconfig, "sip", "sip_passwd", "pincode999");
	safe_copy(cfg->sip_passwd,result);
	result = (char *)lp_config_get_string(lpconfig, "sip", "ip", "192.168.100.105");
	safe_copy(cfg->sip_server_ip,result);
	
	
	cfg->call_type  = lp_config_get_int(lpconfig, "terminal", "call_type", 1);

	result = (char *)lp_config_get_string(lpconfig, "server_cfg", "ip_server1", "192.168.1.13");
	safe_copy(cfg->main_server_ip,result);
	cfg->main_server_port = lp_config_get_int(lpconfig, "server_cfg", "port_server1", 2048);

	result = (char *)lp_config_get_string(lpconfig, "server_cfg", "ip_server2", "192.168.1.14");
	safe_copy(cfg->back_server_ip,result);
    cfg->back_server_port = lp_config_get_int(lpconfig, "server_cfg", "port_server2", 2048);

	result = (char *)lp_config_get_string(lpconfig, "server_cfg", "ip_version", "192.168.1.13");
	safe_copy(cfg->version_server_ip,result);
    cfg->version_server_port = lp_config_get_int(lpconfig, "server_cfg", "port_version", 2048);

	//[short_cfg]
    cfg->noise_alarm_vol = lp_config_get_int(lpconfig, "short_cfg", "noise_alarm_vol", 5);
    cfg->noise_alarm_time = lp_config_get_int(lpconfig, "short_cfg", "noise_alarm_time", 2);

	cfg->trigger_mode_1 = lp_config_get_int(lpconfig, "short_cfg", "short_i1_alarm_mode", 1);
	
	cfg->link_short_out_1 = lp_config_get_int(lpconfig, "short_cfg","short_i1_link_out",0);
	cfg->link_audio_out_1 = lp_config_get_int(lpconfig,"short_cfg","short_i1_link_audio",0);
	cfg->link_audio_volume_1 = lp_config_get_int(lpconfig,"short_cfg","short_i1_link_volume",10);	
	cfg->link_timeout_1 = lp_config_get_int(lpconfig,"short_cfg","short_o1_time",10);
	
	cfg->trigger_mode_2 = lp_config_get_int(lpconfig, "short_cfg", "short_i2_alarm_mode", 1);
	cfg->link_short_out_2 = lp_config_get_int(lpconfig, "short_cfg","short_i2_link_out",0);
	cfg->link_audio_out_2 = lp_config_get_int(lpconfig,"short_cfg","short_i2_link_audio",0);
	cfg->link_audio_volume_2 = lp_config_get_int(lpconfig,"short_cfg","short_i2_link_volume",10);	
	cfg->link_timeout_2 = lp_config_get_int(lpconfig,"short_cfg","short_o2_time",10);

    cfg->short_o1_normal_mode = lp_config_get_int(lpconfig, "short_cfg", "short_o1_normal_mode", 0);
	cfg->short_o2_normal_mode = lp_config_get_int(lpconfig, "short_cfg", "short_o2_normal_mode", 0);

	cfg->borad_check_audio_play_time = lp_config_get_int(lpconfig, "short_cfg", "audio_play_time", 10);
	cfg->board_check_enable = lp_config_get_int(lpconfig, "short_cfg", "enable_speaker_test", 1);
	
	cfg-> cash_server_offline_trigger_enable = lp_config_get_int(lpconfig, "short_cfg", "enable_offdoor", 1);
	cfg-> cash_server_autodoor_trigger_enable = lp_config_get_int(lpconfig, "short_cfg", "enable_autodoor", 0);

//[alone_cfg]
	cfg->enable_onvif =  lp_config_get_int(lpconfig, "video_cfg", "enable_onvif", 1); 
	cfg->enable_wdr =  lp_config_get_int(lpconfig, "video_cfg", "enable_wdr", 1); 
	cfg->resolution =  lp_config_get_int(lpconfig, "video_cfg", "resolution", 1); 
	
	cfg->enmergecy_mode =  lp_config_get_int(lpconfig, "short_cfg", "enmergecy_mode", 1); 
	cfg->enmergecy_prio =  lp_config_get_int(lpconfig, "short_cfg", "enmergecy_prio", 1);	
	cfg->move_trigger_mode =  lp_config_get_int(lpconfig, "short_cfg", "move_trigger_mode", 1);
	cfg->move_trigger_prio =  lp_config_get_int(lpconfig, "short_cfg", "move_trigger_prio", 1);
	cfg->door1_status_mode =  lp_config_get_int(lpconfig, "short_cfg", "door1_status_mode", 1);
	cfg->door1_status_prio =  lp_config_get_int(lpconfig, "short_cfg", "door1_status_prio", 1);
	cfg->door2_status_mode =  lp_config_get_int(lpconfig, "short_cfg", "door2_status_mode", 1);
	cfg->door2_status_prio =  lp_config_get_int(lpconfig, "short_cfg", "door2_status_prio", 1);
	cfg->enable_noisealarm =  lp_config_get_int(lpconfig, "short_cfg", "enable_noisealarm", 1);
	cfg->noisealarm_prio =  lp_config_get_int(lpconfig, "short_cfg", "noisealarm_prio", 1);
	cfg->enable_talk_alarm =  lp_config_get_int(lpconfig, "short_cfg", "enable_talk_alarm", 1);
	cfg->talk_alarm_prio =  lp_config_get_int(lpconfig, "short_cfg", "talk_alarm_prio", 1);

	cfg->extern_in1_mode =  lp_config_get_int(lpconfig, "short_cfg", "extern_in1_mode", 1);
	cfg->extern_in2_mode =  lp_config_get_int(lpconfig, "short_cfg", "extern_in2_mode", 1);

	//默认的报警类型检测
	if (cfg->enmergecy_mode==0) cfg->enmergecy_prio=-1;
	if (cfg->move_trigger_mode==0) cfg->move_trigger_prio=-1;

	if (cfg->door1_status_mode==0) cfg->door1_status_prio=-1;
	else {cfg->door1_status_mode = TRIGGER_MODE_STATUS_REVERSE_LEVEL;}

	if (cfg->door2_status_mode==0) cfg->door2_status_prio=-1;
	else {cfg->door2_status_mode = TRIGGER_MODE_STATUS_REVERSE_LEVEL;}

	if (cfg->enable_noisealarm==0) cfg->noisealarm_prio=-1;
	else {cfg->enable_noisealarm = TRIGGER_MODE_STATUS_FALLING_EDGE;}

	if (cfg->enable_talk_alarm==0) cfg->talk_alarm_prio=-1;
	else {cfg->enable_talk_alarm = TRIGGER_MODE_STATUS_REVERSE_LEVEL;}

    cfg->enable_offline = lp_config_get_int(lpconfig, "alone_cfg", "enable", 1);
    cfg->talk_call_link = lp_config_get_int(lpconfig, "alone_cfg", "talk_call_link", 0);
    
    result = (char *)lp_config_get_string(lpconfig, "alone_cfg", "ip_keyleft", "192.168.1.105");
    safe_copy(cfg->offline_ip1,result);
    cfg->offline_port1 = lp_config_get_int(lpconfig, "alone_cfg", "port_keyleft", 2046);
    
    result = (char *)lp_config_get_string(lpconfig, "alone_cfg", "ip_keyright", "192.168.1.100");
    safe_copy(cfg->offline_ip2,result);
    cfg->offline_port2 = lp_config_get_int(lpconfig, "alone_cfg", "port_keyright", 2046);


    cfg->audio_encode = lp_config_get_int(lpconfig, "audio_encode", "type", 0);
    cfg->talk_samplerate = lp_config_get_int(lpconfig, "talk_samplerate", "rate",8000);
    cfg->server_request_type = lp_config_get_int(lpconfig,"server_cfg","tcp_enable",0/*1 for debug);

    cfg->ftp_enable = lp_config_get_int(lpconfig, "server_cfg", "ftp_enable", 1);
    
	result = (char *)lp_config_get_string(lpconfig, "server_cfg", "ip_ftp", "192.168.1.13");
    safe_copy(cfg->ftp_server_ip,result);
    cfg->ftp_server_port = lp_config_get_int(lpconfig, "server_cfg", "port_ftp", 21);

	result = (char *)lp_config_get_string(lpconfig, "server_cfg", "username_ftp", "admin");
    safe_copy(cfg->ftp_server_user,result);

    result = (char *)lp_config_get_string(lpconfig, "server_cfg", "spon_broadcast_ip", "234.0.0.254");
	safe_copy(cfg->spon_broadcast_ip,result);
	
	result = (char *)lp_config_get_string(lpconfig, "server_cfg", "password_ftp", "admin");
	safe_copy(cfg->ftp_server_passwd,result);

	int login_period = lp_config_get_int(lpconfig, "server_cfg", "interval_request_login", 3);
	cfg->server_request_peroid = (login_period < 3 || login_period > 30)? 3:login_period;

	
	void  cfg_save(void) 
	{
		SPON_LOG_SYS("cp -fr %s  %s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_FILE);
		system_cmd_exec("cp -fr %s	%s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_FILE);
	}
	
	void  cfg_save_back(void) 
	{
		SPON_LOG_SYS("cp -fr %s  %s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_BACK_FILE);
		system_cmd_exec("cp -fr %s	%s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_BACK_FILE);
	}
	
	void  cfg_last_reload(void) 
	{
		SPON_LOG_SYS("cp -fr %s  %s",INI_CONFIG_LOCAL_BACK_FILE,INI_CONFIG_LOCAL_FILE);
		system_cmd_exec("cp -fr %s	%s",INI_CONFIG_LOCAL_BACK_FILE,INI_CONFIG_LOCAL_FILE);
	}
	
	void  cfg_factory_reload(void) 
	{
		SPON_LOG_SYS("cp -fr %s  %s",INI_CONFIG_FACTORY_LOCAL_FILE,INI_CONFIG_LOCAL_FILE);
		system_cmd_exec("cp -fr %s	%s",INI_CONFIG_FACTORY_LOCAL_FILE,INI_CONFIG_LOCAL_FILE);
	}
	#define INI_VERSION_FILE					"/var/www/ini/VersionInfo.txt"
#define INI_CONFIG_FILE 					"/var/www/ini/sys_cfg.txt"
#define INI_CONFIG_BACK_FILE 				"/var/www/ini/sys_cfg_last.txt"

#define INI_CONFIG_LOCAL_FILE 				"/mnt/nand1-2/www/ini/sys_cfg.txt"
#define INI_CONFIG_LOCAL_BACK_FILE 			"/mnt/nand1-2/www/ini/sys_cfg_last.txt"
#define BOARD_ONLINE_FILE 					"/var/www/ini/board_state.txt"
#define INI_CONFIG_FACTORY_LOCAL_FILE 		"/mnt/nand1-2/www/ini/ini_back/sys_cfg.txt"

#define SYSTEM_RING_FILE_PATH_EN  	 		"/mnt/nand1-2/system-res/ringfile_EN"
#define USER_RING_FILE_PATH_EN  			"/mnt/nand1-2/sd/user-res/ringfile_EN"

#define SYSTEM_RING_FILE_PATH_CN  	 		"/mnt/nand1-2/system-res/ringfile_CN"
#define USER_RING_FILE_PATH_CN  			"/mnt/nand1-2/sd/user-res/ringfile_CN"

#define SYSTEM_REMOTE_FILE_PATH  	 		"/mnt/nand1-2/system-res/remotefile"
#define USER_REMOTE_FILE_PATH  				"/mnt/nand1-2/sd/user-res/remotefile"
#define NCS6081_LOG_FILE_PATH				"/mnt/nand1-2/sd/user-res/media_log.txt"

#define VAR_SYSTEM_RING_FILE_PATH  	 		"/var/sys-ringfile"
#define VAR_USER_RING_FILE_PATH  			"/var/user-ringfile"

#define SYSTEM_LOG_FILE  	 				"/var/log/userlog"
#define USER_LOG_PATH  						"/mnt/nand1-2/www"

*/



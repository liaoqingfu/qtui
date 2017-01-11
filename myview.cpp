#include "myview.h"  
#include <QKeyEvent>  


ncs_cfg_t cfg;

MyView::~MyView( )
{
	qDebug() << "~MyView exit"; 

}

MyView::MyView(QWidget *parent) :  
	QGraphicsView(parent)  
{  
    /*
    QPixmap tilePixmap(64, 64);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, 32, 32, color);
    tilePainter.fillRect(32, 32, 32, 32, color);
    tilePainter.end();

    setBackgroundBrush(tilePixmap);*/
    WindowType = WINDOW_TYPE_MAIN;
	
    scene_main = new myscene_main( this );
	scene_main->setSceneRect(0,0,1024,600);//(0,0,(static_cast<QWidget *>600),(static_cast<QWidget *>600) );
	this->setScene(scene_main);

    scene_video = new myscene_video( this );
	scene_video->setSceneRect(0,0,1024,600);

    scene_num_call = new myscene_num_call( this );
	scene_num_call->setSceneRect(0,0,1024,600);

	ReadAllSettings( );
	
}  

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
void MyView::changeWindowType( int winType )
{

	switch ( winType )
	{
		case WINDOW_TYPE_MAIN:	
			this->setScene(scene_main);;
			break;
		case WINDOW_TYPE_NUM_CALL:	
			this->setScene(scene_num_call);;
			break;
		case WINDOW_TYPE_LIST_CALL:  
			this->setScene(scene_num_call);;
			break;
		case WINDOW_TYPE_PIC_CALL:	
			this->setScene(scene_num_call);;
			break;
		case WINDOW_TYPE_VIDEO_HALF: 
			if( WindowType < WINDOW_TYPE_VIDEO_HALF )
				topWindowType = WindowType;
			this->setScene(scene_video);;
			break;
		case WINDOW_TYPE_VIDEO_FUL:  
			if( WindowType < WINDOW_TYPE_VIDEO_HALF )
				topWindowType = WindowType;
			this->setScene(scene_video);;
			break;
		default:
			qDebug() << "<error : unknown WindowType>";
			return ;
	}
	WindowType = winType;
	
}

void MyView::mousePressEvent(QMouseEvent *event)  
{  
    qDebug("***MyView::mousePressEvent*");
	QGraphicsView::mousePressEvent(event);
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
  
void MyView::mouseMoveEvent(QMouseEvent *event)  
{  
	//qDebug("************MyView::mouseMoveEvent*****************");  
	QGraphicsView::mouseMoveEvent(event);  
} 


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
	settings.setValue( "output_h", 600);
	settings.setValue( "crop_x", 0);
	settings.setValue( "crop_y", 0);
	settings.setValue( "crop_w", 0);
	settings.setValue( "crop_h", 0);
	settings.setValue( "rotate", 4);
	settings.endGroup();
	*/
}
/*
QSettings settings;
 int size = settings.beginReadArray("logins");
 for (int i = 0; i < size; ++i) {
     settings.setArrayIndex(i);
     Login login;
     login.userName = settings.value("userName").toString();
     login.password = settings.value("password").toString();
     logins.append(login);
 }
 settings.endArray();

*/


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

void MyView::ReadAllSettings( )
{
    QSettings settings(CFG_NAME, QSettings::IniFormat);
	cfg.local_ip = settings.value( "terminal/ip" ).toString();
	cfg.id = settings.value( "terminal/id" ).toInt();

	cfg.sip_ip = settings.value( "sip/ip" ).toString();
	cfg.sip_username= settings.value( "sip/sip_username" ).toString();
	cfg.sip_passwd= settings.value( "sip/sip_passwd" ).toString();

	cfg.ip_keyleft  = settings.value( "alone_cfg/ip_keyleft" ).toString();
	cfg.ip_keyright = settings.value( "alone_cfg/ip_keyright" ).toString();
	cfg.port_keyleft  = settings.value( "alone_cfg/port_keyleft" ).toInt();
	cfg.port_keyright = settings.value( "alone_cfg/port_keyright" ).toInt();

	
	qDebug() << getLocalIp()  << " cfg.local_ip" << cfg.local_ip;

	if( getLocalIp() != cfg.local_ip )
		system_cmd_exec("ifconfig eth0 %s", cfg.local_ip.toLatin1().data());
	
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
		ncs_cfg_netword_set(NULL ,NULL,gw,gw,gw,cfg->mac_addr);

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
	
	cfg->ncs_cash_server_offline_trigger_enable = lp_config_get_int(lpconfig, "short_cfg", "enable_offdoor", 1);
	cfg->ncs_cash_server_autodoor_trigger_enable = lp_config_get_int(lpconfig, "short_cfg", "enable_autodoor", 0);

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

	
	void ncs_cfg_save(void) 
	{
		SPON_LOG_SYS("cp -fr %s  %s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_FILE);
		system_cmd_exec("cp -fr %s	%s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_FILE);
	}
	
	void ncs_cfg_save_back(void) 
	{
		SPON_LOG_SYS("cp -fr %s  %s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_BACK_FILE);
		system_cmd_exec("cp -fr %s	%s",INI_CONFIG_FILE,INI_CONFIG_LOCAL_BACK_FILE);
	}
	
	void ncs_cfg_last_reload(void) 
	{
		SPON_LOG_SYS("cp -fr %s  %s",INI_CONFIG_LOCAL_BACK_FILE,INI_CONFIG_LOCAL_FILE);
		system_cmd_exec("cp -fr %s	%s",INI_CONFIG_LOCAL_BACK_FILE,INI_CONFIG_LOCAL_FILE);
	}
	
	void ncs_cfg_factory_reload(void) 
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



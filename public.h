#ifndef PUBLIC_H
#define PUBLIC_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "QLabel"
#include "QList"
#include "QNetworkInterface"
#include "QHostAddress"
#include "QAbstractSocket"
#include "QNetworkReply"  
#include "QNetworkAccessManager"  
#include "QNetworkRequest"  
#include "QRegExp"



#include "QPixmap"
#include "QPushButton"
#include "QIcon"
#include "QDebug"
#include "QPainter"
#include "QTime"
#include <QThread>
#include <QTextStream>
#include "QDesktopWidget"
#include "QRect"
#include "QPixmap"
#include "QToolButton"
#include "QGraphicsProxyWidget"
#include "QGraphicsView"
#include "QVBoxLayout"
#include "QHBoxLayout"


#include <QGraphicsSceneMouseEvent>
#include <QPaintEvent>
#include <QKeyEvent>

#include  "QToolButton"
#include "QGraphicsProxyWidget"
#include "QVBoxLayout"
#include "QIcon"
#include "QPixmap"

#include <QtWidgets>
#include <qtextcodec.h>
#include <qfont.h>
#include <qapplication.h>
#include "QToolTip"
#include "api/common.h"

#include <qmath.h>
#define FRAME_FPS     15
#define REF_TIME_MS  1000
#define REF_VIDEO_MS (1000/FRAME_FPS)

#define NUM_CALL 15   //num of call num button
#define MAX_NUM_LEN 10   //lengh of call num
#define MAX_CALL_TYPE  2

#define FONE_NAME "wenquanyi"
#ifdef SYS_WIN
    #define CFG_NAME "d:\sys_cfg.txt"
#else
	
    #define CFG_NAME "/var/www/ini/sys_cfg.txt"
#endif

#define NUM_X_WID       160
#define NUM_START_X     200   //1开始位置

#define NUM_Y_HEIGHT    200


#define WINDOW_TYPE_MAIN     	  0
#define WINDOW_TYPE_NUM_CALL      1
#define WINDOW_TYPE_LIST_CALL     2
#define WINDOW_TYPE_PIC_CALL      3
#define WINDOW_TYPE_MAX_UI        4    //0,1,2,3


#define WINDOW_TYPE_VIDEO_HALF    8
#define WINDOW_TYPE_VIDEO_FUL 	  9



typedef struct {
//[terminal]
	int id;
	QString local_ip;
	int   port;
	QString netmask;
	QString gateway;
	QString dns1;
	QString dns2;
	QString mac_addr;
	

//[sip]
	QString sip_ip;
	int   sip_port;
	QString sip_username;
	QString sip_passwd;

	int sip_audio_rtp_port;
	int sip_video_rtp_port;
	int sip_server_port;

//[alone_cfg]
	int   enable;
	QString ip_keyleft;
	int port_keyleft;
	QString ip_keyright;
	int port_keyright;


//[talk_cfg]
	int volume_in;
	int volume_out;
	int volume_ring;
	int mode_in;
	int mode_out;
	int talk_auto_answer;
	
	int accessing_talk_hangup;
	int echo;
	int environment;
	int display_target;
	
//	[broadcast_cfg]
	int broadcast_volume_out;
	int broadcast_mode_out;
	
//	[monite_cfg]
	int monite_volume_in;
	int monitemode_in;
	
//	[short_cfg]
	int short_i1_alarm_mode;
	int short_o1_normal_mode;
	int enable_removealarm;
	int enable_noisealarm;
	int noise_alarm_vol;
	int noise_alarm_time;
	
//	[other_cfg]
	int screensaver_min;
	int io_out_pass;
	
//	[list_cfg]
	int display_col;
	QString name1;
	int target1;
	int in_num1;
	int link1;
	
	QString name2;
	int target2;
	int in_num2;
	int link2;

	
	QString name3;
	int target3;
	int in_num3;
	int link3;

	
//	[video_cfg]
	int enable_onvif;
	int enable_wdr;
	int resolution;
	
//	[web_cfg]
//	username=admin
//	password=admin
	
//	[language]
	int language;
}ncs_cfg_t;


#endif // PUBLIC_H


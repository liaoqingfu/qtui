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

#include "QMouseEvent"
#include <QProcess>

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

#include<sys/types.h>
#include<sys/stat.h>
#include<errno.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>


#include <qmath.h>
#define FRAME_FPS     15
#define REF_TIME_MS  1000
#define REF_VIDEO_MS (1000/FRAME_FPS)

#define NUM_CALL 18   //num of call num button
#define MAX_NUM_LEN 16   //lengh of call num
#define MAX_CALL_TYPE  2

#define FONE_NAME "wenquanyi"
#ifdef SYS_WIN
    #define CFG_NAME "d:\sys_cfg.txt"
#else
	
    #define CFG_NAME "/var/www/ini/sys_cfg.txt"
#endif


#define SCREEN_WID   		1024
#define SCREEN_HEIGHT   	600

#define NUM_START_X     200   //1开始位置
#define NUM_X_WID       ((SCREEN_WID - NUM_START_X)/6)

#define NUM_START_Y     SCREEN_HEIGHT   //(SCREEN_HEIGHT - (SCREEN_HEIGHT - 3*NUM_Y_HEIGHT)/2)
#define NUM_Y_HEIGHT    (SCREEN_HEIGHT / 3)


#define WINDOW_TYPE_MAIN     	  0

#define WINDOW_TYPE_NUM_CALL      1
#define WINDOW_TYPE_LIST_CALL     2
#define WINDOW_TYPE_PIC_CALL      3
#define WINDOW_TYPE_MAX_UI        4    //0,1,2,3


#define WINDOW_TYPE_VIDEO_HALF    8
#define WINDOW_TYPE_VIDEO_FUL 	  9
#define WINDOW_TYPE_CALLING       10


//low lever to trig call
#define GPO_LED_CALL     (4*32+20)     //CSI0_DATA_EN    5_20      
#define GPO_LED_WARNING  (5*32+31)    //EIM_BCLK        6_31         

#define GPI_LEFT_KEY    (13)  //SD2_DATA2       1_13                     key2    pin42
#define GPI_RIGHT_KEY   (15)  //SD2_DATA0       1_15                   key1    pin44

#define LOLR_UNMUTE   		1
#define LOLR_MUTE   		0

#define GPO_LOLR_SPK     (4*32 + 9)     //DISP0_DAT15     5_9  , LOL,LOR MUTE CONTROL ,  HIGH:OPEN


#define GPO_SHORT_ALARM     (5*32+8)     //NANDF_ALE        6_8     
#define GPI_SHORT_ALARM     (5*32+7)    //NANDF_CLE        6_7   


#define GPIO_IN   0
#define GPIO_OUT   1

#define LED_ON      		0
#define LED_OFF     		1


#define TIME_DATE_FONTSIZE   20
#define TIME_POSX   2
#define TIME_POSY   550
#define TIME_POSW   150
#define TIME_POSH   50

#define DATE_POSX   2
#define DATE_POSY   400
#define DATE_POSW   200
#define DATE_POSH   50


#define LIST_MAX_NUM  128



typedef enum {
    TS_IDLE = 0x0,
    TS_TALK = 0x1, 		//���ǹ̶��ģ�ncs_cfg_t->call_type = 1;��ͨ�Խ���
    TS_SIP_TALK = 0x2,	//SIP�Խ�
    TS_MONITOR,
    TS_BROADCAST,
    TS_RING,
    TS_PLAYIP,
    TS_ALARM,
    TS_BOARD_CHECK,
    TS_SHORT_PLAY,
    TS_REMOTE_PLAY,
    TS_INCOMING_TALK,
    TS_TALKING,
    TS_INVALID,
} talk_type_t;

typedef enum {
	 LED_OFFLINE = 0,
	 LED_ONLINE = 1,
	 LED_TALK = TS_TALKING,
	 LED_RING = TS_RING,
	 LED_BROADCAST = TS_BROADCAST,
}  led_state_t;


typedef struct {
//[terminal]
	QString localid;
	QString local_ip;
	int   port;
	QString netmask;
	QString gateway;
	QString dns1;
	QString dns2;
	QString mac_addr;
	int   bDynamicIP;   //lhg not usable now
	

//[sip]
	QString sip_ip;
	int   sip_port;
	QString sip_username;
	QString sip_passwd;

	int sip_audio_rtp_port;
	int sip_video_rtp_port;
	int sip_server_port;

//[alone_cfg]   
	int   alone_enable;
	QString ip_keyleft;
	int port_keyleft;
	QString ip_keyright;
	int port_keyright;


//[talk_cfg]
	int volume_in;
	int volume_out;
	int volume_ring;
	int mode_in;
	int mode_out;  //�Խ����ѡ��,0:�������������·���
	int talk_auto_answer;
	int talk_auto_answer_time;
	
	int accessing_talk_hangup;
	int echo;
	int environment;
	int display_target;
	int display_halfScrenn;
	
//	[broadcast_cfg]
	int broadcast_volume_out;
	int broadcast_mode_out;
	
//	[monite_cfg]
	int monite_volume_in;
	int monite_mode_in;
	
//	[short_cfg]
	int short_i1_alarm_mode;
	int short_o1_normal_mode;
	int enable_removealarm;
	int enable_noisealarm;
	int noise_alarm_vol;
	int noise_alarm_time;
	
//	[other_cfg]
	int screensaver_sec;
	QString io_out_pass;
	int screen_button;
	int hdmi_style;
	int font_size;
	
//	[list_cfg]
	int display_col;
	int list_count;
	QString list_name[LIST_MAX_NUM];
	QString list_target  [LIST_MAX_NUM];
	int list_in_num  [LIST_MAX_NUM];
	int list_link    [LIST_MAX_NUM];
	
	
//	[video_cfg]
	int enable_onvif;
	int enable_wdr;
	int resolution;
	unsigned int xres;   //camera capture width
	unsigned int yres;   //camera capture height

	
//	[web_cfg]
//	username=admin
//	password=admin
	
//	[language]
	int language;
}ncs_cfg_t;


#endif // PUBLIC_H


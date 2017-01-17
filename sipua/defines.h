/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : defines.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/03/31
* Description  : 3091对讲应用宏定义
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef  __DEFINES_H__
#define  __DEFINES_H__

#include "types.h"
#include "version.h"
#include "ua_global.h"

//----------------- 系统 -----------------
#define SIP_SERVER_USERNAME(id)	((id) + 1000)
#define SPON_SERVER_ID(id)		((id) - 1000)

//----------------- 网络 -----------------
#define BYTE1(a)				(a&0xFF)
#define BYTE2(a)				((a>>8)&0xFF)
#define BYTE3(a)				((a>>16)&0xFF)
#define BYTE4(a)				((a>>24)&0xFF)

#define IP4ADDR						DWORD
#define NET_FRAME_MAX_LEN			1514
#define NET_WAVE_FRAME_ADPCM_SIZE       116
#define NET_WAVE_FRAME_PCM_SIZE         440
#define NET_WAVE_FRAME_PCM_SAMPLES      220
#define NET_MP3_FRAME_MP3_SIZE			320

#define SPON_SERVER_PORT			2048
#define NET_PORT_CTRL1				2046		// 控制端口
#define NET_PORT_SERVER				2048		// 控制端口
#define NET_PORT_CTRL2				2058		// 控制端口
#define NET_PORT_CTRL3				2068		// 控制端口
#define NET_PORT_CTRL_DATA			2050		// 数据端口
#define NET_IP_MULTICAST_CTRL		0xfe0000ea	// 控制IP:234.0.0.254

#define NET_MSG_HDR_SIZE  8
typedef struct {
	WORD    id;                    // 0起始
	BYTE    msg;
	BYTE    param1;
	BYTE    param2;
	BYTE    param3;
	BYTE    param4;
	BYTE    param5;
	BYTE    data[1];
} NET_MSG_HDR, *PNET_MSG_HDR;

#define NET_MSG_HAND_SHAKE              0x10
#define NET_MSG_DATETIME                0x11     
#define NET_MSG_MODIFY_VOLUME           0x12   
#define NET_MSG_SET_HANDSHAKERATE       0x14

#define NET_MSG_LOGON_REQUEST           0x17   
#define NET_MSG_LOGON_STATE             0x1A   

#define NET_MSG_REMOTE_CONTROL          0x1F   

#define NET_MSG_KEYDOWN                 0x20   

#define NET_MSG_NAT_RETURN              0x21

#define NET_MSG_BROADCAST_REQUEST       0x22
#define NET_MSG_BROADCAST_STATE         0x23

#define NET_MSG_TALK_REQUEST            0x24
#define NET_MSG_TALK_STATUS             0x25

#define NET_MSG_IP_REQUEST              0x26
#define NET_MSG_IP_REPLY                0x27

#define NET_MSG_SESSION_REQUEST         0x2D
#define NET_MSG_SESSION_STATE           0x2E

#define NET_MSG_SWITCH_TASK             0x2F    

#define NET_MSG_CONTROL_DOOR            0x30
#define NET_MSG_BROADCAST_BYPASS        0x31 
#define NET_MSG_SHORTOUTPUT_CTRL        0x33

#define NET_MSG_DATA_MP3_FILE           0x40    
#define NET_MSG_DATA_MP3_LIVE           0x41    
#define NET_MSG_DATA_WAVE_FILE          0x44
#define NET_MSG_DATA_WAVE_LIVE          0x45
#define NET_MSG_DATA_TABLE              0x48
#define NET_MSG_DATA_SD_UPDATE          0x49

#define NET_MSG_DATA_TALK_DATA          0x4E

#define NET_MSG_SCREEN_READ             0x6F

#define NET_MSG_STATE_QUERY             0x70		
#define NET_MSG_STATE_REPORT            0x71		
#define NET_MSG_STATE_CTRL              0x72	
#define CONTROL_OP_OUT                      0x00
#define CONTROL_OP_FLUSH_ARP_CACHE          0x01
#define CONTROL_OP_IN                       0x02
#define CONTROL_OP_STOP_ALERT               0x03		//add by zengsuqing Ö§³Ö0x72Ö§³Ö8Â·ÊäÈëÊä³ö¿ØÖÆÆ÷ 141222

#define NET_MSG_CONTROL_APPOWER         0x74    
#define NET_MSG_CONTROL_FORCE           0x75
#define NET_MSG_CONTROL_PAGROUP         0x76
#define NET_MSG_PLAY_REQUEST            0x78
#define NET_MSG_PLAY_STATE              0x79
#define PLAY_STATE_STOP                     0x00
#define PLAY_STATE_PLAYING                  0x01
#define PLAY_STATE_NO_FILE                  0x02
#define PLAY_STATE_NO_DISK                  0x03
#define PLAY_STATE_TASK_BUSY                0x04
#define NET_MSG_SPEAKER_STATE           0x7B

#define NET_MSG_OUT_INFO                0x7F
#define OUTINFO_CONNECT_REQUEST             0x01
#define OUTINFO_CONNECT_REPLY               0x02
#define OUTINFO_HANDSHAKE                   0x03
#define OUTINFO_INFO                        0x04

#define NET_MSG_SETUP_REQUEST           0x80		
#define NET_MSG_SETUP_REQUEST_EX        0x81
#define NET_MSG_DEVICE_CONFIG           0x86

#define NET_MSG_VIRTUAL_KEY             0x90	
#define NET_MSG_SDK_CONTROL             0x91	

#define NET_MSG_SYNC_REQUEST            0xA1
#define NET_MSG_SYNC_STATE              0xA2
#define NET_MSG_SYNC_DATETIME           0xA3

#define NET_MSG_RESET                   0xB1
#define RESET_OP_CPU0                       0x00
#define RESET_OP_ETH0                       0x01
#define RESET_OP_CODEC0                     0x02
#define NET_MSG_MIXER_IN                0xB3
#define MIXERIN_OP_NONE                     0x00
#define MIXERIN_OP_LINE                     0x01
#define MIXERIN_OP_MICROPHONE               0x02
#define NET_MSG_MIXER_OUT               0xB4
#define MIXEROUT_OP_NONE                    0x00
#define MIXEROUT_OP_LINE                    0x01
#define MIXEROUT_OP_SPEAK                   0x02
#define NET_MSG_TALK_CONFIG             0xB5
#define NET_MSG_ECHO_CANCEL             0xB6
#define NET_MSG_RECORDER                0xB7        


#endif //__DEFINES_H__



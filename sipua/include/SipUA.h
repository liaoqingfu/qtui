/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : SipUA.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/11
* Description  : eXosip SIP User Agent 接口程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/

#ifndef __SIP_UA_H__
#define __SIP_UA_H__

#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>

#include "types.h"
#include "ua_port.h"
#include "ua_global.h"
#include "AudioStream.h"
#include "VideoStream.h"
#include "ua_event.h"
#include "EventList.h"

#define SIP_UA_PORT_DEFAULT         5060
#define SIP_RTP_AUDIO_PORT_DEFAULT  10000
#define SIP_RTP_VIDEO_PORT_DEFAULT  30000
#define SIP_RTP_PORT_MIN			10000
#define SIP_RTP_PORT_MAX			55000		// rtcp = rtp + 10000
#define SIP_UA_VER                  "1.0"
#define SIP_UA_STRING               "SipForSpon v" SIP_UA_VER
#define SIP_UA_USERNAME_LEN         32

#define SIP_UA_CALL_KEY_MAX			20
#define SIP_UA_MESSAGE_TARGET_MAX	6

typedef enum sip_status {
	SIP_STATUS_NULL,
	SIP_STATUS_CALL_REQUEST,
	SIP_STATUS_CALLING,
	SIP_STATUS_RINGING,
	SIP_STATUS_TALKING,
	SIP_STATUS_BC_REQUEST,
	SIP_STATUS_BCING,
	SIP_STATUS_MONITORING,
} sip_status_t;

typedef enum sip_task_type {
	SIP_TASK_TYPE_NULL,
	SIP_TASK_TYPE_TALK_OUTGOING,
	SIP_TASK_TYPE_TALK_INCOMING,
	SIP_TASK_TYPE_BC_OUTGOING,
	SIP_TASK_TYPE_BC_INCOMING,
	SIP_TASK_TYPE_MONITOR_OUTGOING,
	SIP_TASK_TYPE_MONITOR_INCOMING,
} sip_task_type_t;

typedef struct sip_channel {
} sip_channel_t;

class CTerminal;
class CSipUA
{
public:
    CSipUA();
    ~CSipUA();

	friend class CTerminal;

	// 初始化设置
    void set_local_addr(const char *p_ip, WORD port);							// 设置本地ip和port(sip协议地址和端口)
    void set_register_addr(const char *p_ip, WORD port);						// 设置注册服务器ip(可以是域名)和port
    void set_username_password(const char *username, const char *password);		// 设置用户名和密码
    void set_contact_addr(const char *p_ip, WORD port);							// 设置联系ip和port(用来控制服务器上显示的ip和port)
    BOOL register_server(void);													// 服务器注册
    BOOL unregister_server(void);												// 服务器注销
    BOOL init(void);															// 初始化
    void set_expires(int expires);												// 设置服务器上的注册失效时间(掉线时间)
	BOOL set_audio_file(const char *p_file);									// 设置音频文件(单个文件)
    void set_sip_event_callback(void(*p_callback)(eXosip_event_t *, void *), void *p_param);// 设置sip事件回调函数
	void set_device_type(int device_type);										// 设置设备类型
	void set_audio_recvdata_callback(void(*p_callback)(void *), void *p_param);	// 设置音频接收数据回调处理(主循环实时性不好)
	void set_video_recvdata_callback(void(*p_callback)(void *), void *p_param);	// 设置视频接收数据回调处理
    void set_enable_rtp_video(BOOL enable);										// 设置是否连接视频(默认不连接)
	// sip事件
    void process_sip_event(void(*p_callback)(eXosip_event_t *, void *), void *p_param);	// sip事件处理
	// 任务
    BOOL talk(const char *callee, const char *src_ext_id, const char *dst_ext_id); // 对讲
    BOOL broadcast(const char *callee, const char *audio_type, BOOL is_term_broadcast); // 广播
    BOOL monitor(const char *callee, const char *src_ext_id, const char *dst_ext_id); // 监听
    BOOL task_end(void);														// 结束任务
	BOOL ring(void);															// 呼叫应答
	BOOL answer(void);															// 接听
	BOOL get_bc_is_multicast(char *p_remote_ip, int *p_remote_port);			// 获取广播是否为组播

	// 传送消息
	BOOL send_msg_volume(const char *p_user_id, const char *p_talk_input, const char *p_talk_output, \
						const char *p_bc_input, const char *p_bc_output);		// 修改终端音量
	BOOL send_msg_alarm_out(const char *p_user_id, const char *p_operation, const char *p_alarm_no, \
						char *p_alarm_status);									// 查询或修改终端报警输出
	BOOL send_msg_alarm_in(const char *p_user_id, const char *p_operation, const char *p_alarm_no, \
						char *p_alarm_status);									// 返回或报告终端报警输入
	// UA事件
	void set_ua_event_callback(void(*p_callback)(CSipUA *, void *), void *p_param);// UA事件回调
	void set_ua_event_semaphore(ua_sem_t *p_sem);								// UA事件信号量

private:
	// 线程处理
	static void *running_thread(void *arg);										// 线程: SIP UA的内部运转
	static BOOL running_sip_event(CSipUA *&p_sip_ua);							// 线程回调函数(处理sip事件)
#ifdef WIN32
    static void CALLBACK on_timer_queue(void *p_param, BOOL timeorwait);		// 定时器处理
#else
    static void on_timer_queue(union sigval sig_val);							// 定时器处理
#endif
	static void play_event_callback(int event_type, DWORD param1, DWORD param2, void *p_param);// 音频文件播放事件

	// rtp数据处理
    void process_sdp_message(sdp_message_t *p_msg_sdp);							// 处理sdp消息
    void process_sdp_audio_message(sdp_message_t *p_msg_sdp);					// 处理sdp音频消息
    void process_sdp_video_message(sdp_message_t *p_msg_sdp);					// 处理sdp视频消息
	BOOL rtp_connect_audio(void);												// 连接rtp音频
	BOOL rtp_connect_video(void);												// 连接rtp视频
	// 任务
    BOOL task_begin(const char *callee, sip_task_type_t task_type, const char *call_info);// 发起任务(对讲,广播,监听)
	// sip事件
	void sip_event_invite(eXosip_event_t *p_event);								// SIP事件(呼叫)
	void sip_event_call_answerd(eXosip_event_t *p_event);						// SIP事件(应答)
	void sip_event_registration_success(eXosip_event_t *p_event);				// SIP事件(注册成功)
	void sip_event_registration_failure(eXosip_event_t *p_event);				// SIP事件(注册失败)

public:
	int						m_task_type;
    int						m_status;
	CAudioStream			m_audio_stream;
	CVideoStream			m_video_stream;
	BOOL					m_registered;										// 是否已注册服务器
	char					m_callee_username[SIP_UA_USERNAME_LEN];				// 会话id

	// UA事件
	CEventList				m_event_list_ua;

	// 配置信息
	char					m_call_target[SIP_UA_CALL_KEY_MAX][SIP_UA_USERNAME_LEN];// 呼叫目标
	int						m_message_target[SIP_UA_MESSAGE_TARGET_MAX];		// 消息目标
	struct tm				m_server_date;										// 日期
	int						m_talk_input_volume;
	int						m_talk_output_volume;
	int						m_bc_input_volume;
	int						m_bc_output_volume;

private:
    struct eXosip_t			*m_pcontext_eXosip;
    char					m_contact_ip[INET6_ADDRSTRLEN];						// 联系IP(用于SIP服务器显示)
	WORD					m_contact_port;										// 联系端口
    char					m_local_ip[INET6_ADDRSTRLEN];
	WORD					m_local_port;
    char					m_net_ip[INET6_ADDRSTRLEN];
	WORD					m_net_port;
    char					m_reg_ip[INET6_ADDRSTRLEN];
	WORD					m_reg_port;
	char					m_username[SIP_UA_USERNAME_LEN];
	char					m_password[SIP_UA_USERNAME_LEN];
	int						m_reg_id;
	BOOL					m_registering;										// 传送注销或注册消息中
	int						m_register_failure_count;
	BOOL					m_unregistering;									// TRUE : 注销中; FALSE : 注册中;
	int						m_expires;											// 注册失效时间
    int						m_register_interval;
    int						m_handshake_interval;
    timer_data_t			m_tdata_reg;										// 注册和握手用的定时器
	int						m_call_id;
	int						m_dialog_id;
	int						m_trans_id;
	osip_message_t			*m_panswer;
	BOOL					m_bc_is_multicast;
	void					(*m_psip_event_callback)(eXosip_event_t *, void *);
	void					*m_pevent_callback_param;
	int						m_device_type;										// 设备类型

	// rtp
	WORD					m_rtp_audio_port;
	const char				*m_pmine_type_audio;
	int						m_payload_type_audio;
	int						m_remote_aport;
	int						m_audio_sample_rate;
	WORD					m_rtp_video_port;
	const char				*m_pmine_type_video;
	char					m_profile_level_id[VIDEO_PROFILE_LEVEL_ID_LEN + 1];
	int						m_payload_type_video;
	int						m_remote_vport;
	int						m_video_sample_rate;
    char					m_remote_ip[INET6_ADDRSTRLEN];
	BOOL					m_enable_rtp_video;									// 是否连接视频(默认不连接)

	// static 变量
	static List<CSipUA *>	m_list_ua;
    static ua_mutex_t		m_mutex_ua;											// m_list_ua的互斥锁
	static BOOL				m_init_static_ua;
	static ua_thread_t		m_thread_ua;

	// UA事件
	ua_sem_t				m_sem_ua_event_callback;				// UA事件触发线程回调的信号量
	void					(*m_pua_event_callback)(CSipUA *p_ua, void *);// UA事件处理回调函数
	void					*m_pua_event_callback_param;			// 回调函数的参数
	ua_sem_t				*m_psem_ua_event;						// UA事件信号量
};

// 设置设备类型
inline void CSipUA::set_device_type(int device_type)
{
	m_device_type = device_type;
}

// 设置sip事件回调函数
inline void CSipUA::set_sip_event_callback(void(*p_callback)(eXosip_event_t *, void *), void *p_param)
{
	m_psip_event_callback = p_callback;
	m_pevent_callback_param = p_param;
}

// UA事件回调
inline void CSipUA::set_ua_event_callback(void(*p_callback)(CSipUA *p_ua, void *), void *p_param)
{
	m_pua_event_callback = p_callback;
	m_pua_event_callback_param = p_param;
}

// UA事件信号量
inline void CSipUA::set_ua_event_semaphore(ua_sem_t *p_sem)
{
	m_psem_ua_event = p_sem;
}

// 设置是否连接视频(默认不连接)
inline void CSipUA::set_enable_rtp_video(BOOL enable)
{
	m_enable_rtp_video = enable;
}

#endif // __SIP_UA_H__





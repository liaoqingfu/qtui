/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : SipUA.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/11
* Description  : eXosip SIP User Agent �ӿڳ���
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

	// ��ʼ������
    void set_local_addr(const char *p_ip, WORD port);							// ���ñ���ip��port(sipЭ���ַ�Ͷ˿�)
    void set_register_addr(const char *p_ip, WORD port);						// ����ע�������ip(����������)��port
    void set_username_password(const char *username, const char *password);		// �����û���������
    void set_contact_addr(const char *p_ip, WORD port);							// ������ϵip��port(�������Ʒ���������ʾ��ip��port)
    BOOL register_server(void);													// ������ע��
    BOOL unregister_server(void);												// ������ע��
    BOOL init(void);															// ��ʼ��
    void set_expires(int expires);												// ���÷������ϵ�ע��ʧЧʱ��(����ʱ��)
	BOOL set_audio_file(const char *p_file);									// ������Ƶ�ļ�(�����ļ�)
    void set_sip_event_callback(void(*p_callback)(eXosip_event_t *, void *), void *p_param);// ����sip�¼��ص�����
	void set_device_type(int device_type);										// �����豸����
	void set_audio_recvdata_callback(void(*p_callback)(void *), void *p_param);	// ������Ƶ�������ݻص�����(��ѭ��ʵʱ�Բ���)
	void set_video_recvdata_callback(void(*p_callback)(void *), void *p_param);	// ������Ƶ�������ݻص�����
    void set_enable_rtp_video(BOOL enable);										// �����Ƿ�������Ƶ(Ĭ�ϲ�����)
	// sip�¼�
    void process_sip_event(void(*p_callback)(eXosip_event_t *, void *), void *p_param);	// sip�¼�����
	// ����
    BOOL talk(const char *callee, const char *src_ext_id, const char *dst_ext_id); // �Խ�
    BOOL broadcast(const char *callee, const char *audio_type, BOOL is_term_broadcast); // �㲥
    BOOL monitor(const char *callee, const char *src_ext_id, const char *dst_ext_id); // ����
    BOOL task_end(void);														// ��������
	BOOL ring(void);															// ����Ӧ��
	BOOL answer(void);															// ����
	BOOL get_bc_is_multicast(char *p_remote_ip, int *p_remote_port);			// ��ȡ�㲥�Ƿ�Ϊ�鲥

	// ������Ϣ
	BOOL send_msg_volume(const char *p_user_id, const char *p_talk_input, const char *p_talk_output, \
						const char *p_bc_input, const char *p_bc_output);		// �޸��ն�����
	BOOL send_msg_alarm_out(const char *p_user_id, const char *p_operation, const char *p_alarm_no, \
						char *p_alarm_status);									// ��ѯ���޸��ն˱������
	BOOL send_msg_alarm_in(const char *p_user_id, const char *p_operation, const char *p_alarm_no, \
						char *p_alarm_status);									// ���ػ򱨸��ն˱�������
	// UA�¼�
	void set_ua_event_callback(void(*p_callback)(CSipUA *, void *), void *p_param);// UA�¼��ص�
	void set_ua_event_semaphore(ua_sem_t *p_sem);								// UA�¼��ź���

private:
	// �̴߳���
	static void *running_thread(void *arg);										// �߳�: SIP UA���ڲ���ת
	static BOOL running_sip_event(CSipUA *&p_sip_ua);							// �̻߳ص�����(����sip�¼�)
#ifdef WIN32
    static void CALLBACK on_timer_queue(void *p_param, BOOL timeorwait);		// ��ʱ������
#else
    static void on_timer_queue(union sigval sig_val);							// ��ʱ������
#endif
	static void play_event_callback(int event_type, DWORD param1, DWORD param2, void *p_param);// ��Ƶ�ļ������¼�

	// rtp���ݴ���
    void process_sdp_message(sdp_message_t *p_msg_sdp);							// ����sdp��Ϣ
    void process_sdp_audio_message(sdp_message_t *p_msg_sdp);					// ����sdp��Ƶ��Ϣ
    void process_sdp_video_message(sdp_message_t *p_msg_sdp);					// ����sdp��Ƶ��Ϣ
	BOOL rtp_connect_audio(void);												// ����rtp��Ƶ
	BOOL rtp_connect_video(void);												// ����rtp��Ƶ
	// ����
    BOOL task_begin(const char *callee, sip_task_type_t task_type, const char *call_info);// ��������(�Խ�,�㲥,����)
	// sip�¼�
	void sip_event_invite(eXosip_event_t *p_event);								// SIP�¼�(����)
	void sip_event_call_answerd(eXosip_event_t *p_event);						// SIP�¼�(Ӧ��)
	void sip_event_registration_success(eXosip_event_t *p_event);				// SIP�¼�(ע��ɹ�)
	void sip_event_registration_failure(eXosip_event_t *p_event);				// SIP�¼�(ע��ʧ��)

public:
	int						m_task_type;
    int						m_status;
	CAudioStream			m_audio_stream;
	CVideoStream			m_video_stream;
	BOOL					m_registered;										// �Ƿ���ע�������
	char					m_callee_username[SIP_UA_USERNAME_LEN];				// �Ựid

	// UA�¼�
	CEventList				m_event_list_ua;

	// ������Ϣ
	char					m_call_target[SIP_UA_CALL_KEY_MAX][SIP_UA_USERNAME_LEN];// ����Ŀ��
	int						m_message_target[SIP_UA_MESSAGE_TARGET_MAX];		// ��ϢĿ��
	struct tm				m_server_date;										// ����
	int						m_talk_input_volume;
	int						m_talk_output_volume;
	int						m_bc_input_volume;
	int						m_bc_output_volume;

private:
    struct eXosip_t			*m_pcontext_eXosip;
    char					m_contact_ip[INET6_ADDRSTRLEN];						// ��ϵIP(����SIP��������ʾ)
	WORD					m_contact_port;										// ��ϵ�˿�
    char					m_local_ip[INET6_ADDRSTRLEN];
	WORD					m_local_port;
    char					m_net_ip[INET6_ADDRSTRLEN];
	WORD					m_net_port;
    char					m_reg_ip[INET6_ADDRSTRLEN];
	WORD					m_reg_port;
	char					m_username[SIP_UA_USERNAME_LEN];
	char					m_password[SIP_UA_USERNAME_LEN];
	int						m_reg_id;
	BOOL					m_registering;										// ����ע����ע����Ϣ��
	int						m_register_failure_count;
	BOOL					m_unregistering;									// TRUE : ע����; FALSE : ע����;
	int						m_expires;											// ע��ʧЧʱ��
    int						m_register_interval;
    int						m_handshake_interval;
    timer_data_t			m_tdata_reg;										// ע��������õĶ�ʱ��
	int						m_call_id;
	int						m_dialog_id;
	int						m_trans_id;
	osip_message_t			*m_panswer;
	BOOL					m_bc_is_multicast;
	void					(*m_psip_event_callback)(eXosip_event_t *, void *);
	void					*m_pevent_callback_param;
	int						m_device_type;										// �豸����

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
	BOOL					m_enable_rtp_video;									// �Ƿ�������Ƶ(Ĭ�ϲ�����)

	// static ����
	static List<CSipUA *>	m_list_ua;
    static ua_mutex_t		m_mutex_ua;											// m_list_ua�Ļ�����
	static BOOL				m_init_static_ua;
	static ua_thread_t		m_thread_ua;

	// UA�¼�
	ua_sem_t				m_sem_ua_event_callback;				// UA�¼������̻߳ص����ź���
	void					(*m_pua_event_callback)(CSipUA *p_ua, void *);// UA�¼�����ص�����
	void					*m_pua_event_callback_param;			// �ص������Ĳ���
	ua_sem_t				*m_psem_ua_event;						// UA�¼��ź���
};

// �����豸����
inline void CSipUA::set_device_type(int device_type)
{
	m_device_type = device_type;
}

// ����sip�¼��ص�����
inline void CSipUA::set_sip_event_callback(void(*p_callback)(eXosip_event_t *, void *), void *p_param)
{
	m_psip_event_callback = p_callback;
	m_pevent_callback_param = p_param;
}

// UA�¼��ص�
inline void CSipUA::set_ua_event_callback(void(*p_callback)(CSipUA *p_ua, void *), void *p_param)
{
	m_pua_event_callback = p_callback;
	m_pua_event_callback_param = p_param;
}

// UA�¼��ź���
inline void CSipUA::set_ua_event_semaphore(ua_sem_t *p_sem)
{
	m_psem_ua_event = p_sem;
}

// �����Ƿ�������Ƶ(Ĭ�ϲ�����)
inline void CSipUA::set_enable_rtp_video(BOOL enable)
{
	m_enable_rtp_video = enable;
}

#endif // __SIP_UA_H__





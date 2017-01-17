/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : Terminal.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : �ն������
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "ua_port.h"
#include "types.h"
#include "SipUA.h"
#include "socket.h"

class CTerminal
{
public:
	CTerminal();
	virtual ~CTerminal();

	void running(void);
	static void sip_event_callback(eXosip_event_t *p_event, void *p_param);
	static void audio_recvdata_callback(void *p_param);
	BOOL send_net_data(BYTE msg, BYTE param1, BYTE param2, BYTE param3, BYTE param4, BYTE param5, BYTE *p_buf, WORD size);

	void sip_event_reg_success(eXosip_event_t *p_event);
	void sip_event_call_ringing(eXosip_event_t *p_event);
	void sip_event_call_answerd(eXosip_event_t *p_event);
	void sip_event_invite(eXosip_event_t *p_event);
	void sip_event_closed(eXosip_event_t *p_event);
	void sip_event_message_new(eXosip_event_t *p_event);

	void net_msg_login_request(SOCKET_DATA *p_socket_data);
	void net_msg_handshake(SOCKET_DATA *p_socket_data);
	void net_msg_ip_request(SOCKET_DATA *p_socket_data);
	void net_msg_talk_request(SOCKET_DATA *p_socket_data);
	void net_msg_talk_status(SOCKET_DATA *p_socket_data);
	void net_msg_talk_data(SOCKET_DATA *p_socket_data);
	void net_msg_broadcast_request(SOCKET_DATA *p_socket_data);
	void net_msg_wave_live(SOCKET_DATA *p_socket_data);
	void net_msg_control_door(SOCKET_DATA *p_socket_data);
	void net_msg_control_shortoutput(SOCKET_DATA *p_socket_data);
	void net_msg_state_report(SOCKET_DATA *p_socket_data);

private:
#ifdef WIN32
    static void CALLBACK on_timer_queue(void *p_param, BOOL timeorwait);		// ��ʱ������
#else
    static void on_timer_queue(union sigval sig_val);									// ��ʱ������
#endif

public:
	int						m_id;								// 1��ʼ
	int						m_talk_id;							// �Խ����������һ��ID
	BOOL					m_login;
    CSipUA					m_sip_ua;
	static CSocketEx		*m_psocket_spon;
	struct sockaddr_in		m_socket_addr;
	int						m_handshake_timeout;
	int						m_audio_format;
private:
    timer_data_t			m_tdata_check_task;					// ������
    timer_data_t			m_tdata_task_status;				// ����״̬: ����
};

#endif // __TERMINAL_H__

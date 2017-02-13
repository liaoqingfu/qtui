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

#ifdef WIN32
#pragma comment(lib, "eXosip.lib")
#pragma comment(lib, "oRTP.lib")
#pragma comment(lib, "osip2.lib")
#pragma comment(lib, "osipparser2.lib")
#pragma comment(lib, "portaudio_x86.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <syslog.h>
#include <netinet/in.h>
#include <getopt.h>
#endif

#include "SipUA.h"
#include "defines.h"
#include "log.h"
#include "PlainParse.h"

#define TIMER_SIVAL_REGISTER        1000
#define TIMER_SIVAL_HANDSHAKE       1001

// static 变量
List<CSipUA *>		CSipUA::m_list_ua;
ua_mutex_t			CSipUA::m_mutex_ua;
BOOL				CSipUA::m_init_static_ua = FALSE;
ua_thread_t			CSipUA::m_thread_ua;

CSipUA::CSipUA()
{
	char	p_strerr[PATH_MAX] = {0};
	char	*ssrc = NULL;
	int		result;
	
	strcpy(m_local_ip, "127.0.0.1");
	m_local_port = SIP_UA_PORT_DEFAULT;
	strcpy(m_contact_ip, "127.0.0.1");
	m_contact_port = SIP_UA_PORT_DEFAULT;
	m_reg_ip[0] = '\0';
	m_reg_port = SIP_UA_PORT_DEFAULT;
	m_username[0] = '\0';
	m_password[0] = '\0';
	m_registering = FALSE;
	m_registered = FALSE;
	m_callee_username[0] = '\0';
	m_expires = 70;
	m_register_interval = 40000;
	m_handshake_interval = 60000;
	m_tdata_reg.p_param = this;
	m_tdata_reg.timerid = NULL;
	m_tdata_reg.sival = TIMER_SIVAL_REGISTER;
	m_task_type = SIP_TASK_TYPE_NULL;
	m_status = SIP_STATUS_NULL;
	m_unregistering = FALSE;
	m_register_failure_count = 0;
	m_call_id = 0;
	m_dialog_id = 0;
	m_trans_id = 0;
	m_reg_id = 0;
	m_bc_is_multicast = FALSE;
	m_panswer = NULL;
	m_net_ip[0] = '\0';
	m_net_port = 0;
	m_psip_event_callback = NULL;
	m_pevent_callback_param = NULL;
	m_device_type = 1;
	m_audio_stream.m_audio_file.set_play_event_callback(play_event_callback, this);

	// 配置信息
	memset(&m_call_target, 0, sizeof (m_call_target));
	memset(&m_message_target, 0, sizeof (m_message_target));
	memset(&m_server_date, 0, sizeof (struct tm));
	m_talk_input_volume = m_talk_input_volume = m_bc_input_volume = m_bc_output_volume = 9;

	// rtp
	m_rtp_audio_port = SIP_RTP_AUDIO_PORT_DEFAULT;
	m_pmine_type_audio = NULL;
	m_remote_aport = 0;
	m_payload_type_audio = 0;
	m_audio_sample_rate = 8000;
	m_rtp_video_port = SIP_RTP_VIDEO_PORT_DEFAULT;
	m_pmine_type_video = NULL;
	memset(m_profile_level_id, 0, sizeof (m_profile_level_id));
	m_remote_vport = 0;
	m_payload_type_video = 0;
	m_video_sample_rate = VIDEO_SAMPLE_RATE;
	m_remote_ip[0] = '\0';
	m_enable_rtp_video = FALSE;

	// UA事件
	ua_sem_init(&m_sem_ua_event_callback, 0);
	m_pua_event_callback = NULL;
	m_pua_event_callback_param = NULL;
	m_psem_ua_event =  NULL;
	
	try
	{
        m_pcontext_eXosip = eXosip_malloc();
        if (m_pcontext_eXosip == NULL)
        {
            sprintf(p_strerr, "[CSipUA::CSipUA(malloc)] : return NULL\n");
            throw 10;
        }
        result = eXosip_init(m_pcontext_eXosip);
        if (result != 0)
        {
            eXosip_quit(m_pcontext_eXosip);
			osip_free(m_pcontext_eXosip);
            m_pcontext_eXosip = NULL;
            sprintf(p_strerr, "[CSipUA::CSipUA(init)](%d) : %s\n", result, osip_strerror(result));
            throw 11;
        }

		if (!m_init_static_ua)
		{
			ua_mutex_init(&m_mutex_ua);
			ua_mutex_lock(&m_mutex_ua);
			m_list_ua.Init();				// 不清楚什么原因,构造函数未调用
			m_list_ua.AddTail(this);
			ua_mutex_unlock(&m_mutex_ua);

			m_init_static_ua = TRUE;
			if (!ua_thread_create(&m_thread_ua, CSipUA::running_thread, this))
			{
				sprintf(p_strerr, "[CSipUA::CSipUA(pthread_create)](%d) : %s\n", errno, strerror(errno));
				throw 12;
			}
		}
		else
		{
			ua_mutex_lock(&m_mutex_ua);
			m_list_ua.Init();				// 不清楚什么原因,构造函数未调用
			m_list_ua.AddTail(this);
			ua_mutex_unlock(&m_mutex_ua);
		}
	}
	catch (int throw_err)
	{
		switch (throw_err)
		{
        case 10:
		default:
			break;
		}
		if (strlen(p_strerr) > 0)
			printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
	}
	catch (std::exception &e)
    {   
		printf_log(LOG_IS_ERR, "[%s][CSipUA::CSipUA()] : %s", m_username, e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[%s][CSipUA::CSipUA()] : error catch!\n", m_username);
    }
}

CSipUA::~CSipUA()
{
    struct timeval		ts;
	int					pos;
	CSipUA				*p_ua = this;

	task_end();
    ua_mutex_lock(&m_mutex_ua);
	pos = m_list_ua.Find(p_ua);
	if (pos > 0)
		m_list_ua.Remove(pos, p_ua);
	ua_mutex_unlock(&m_mutex_ua);
    if (m_registered)
    {
        unregister_server();
        ua_get_time(&ts);
        while (ua_get_timeout(&ts) < 10)
            process_sip_event(NULL, NULL);
    }
    if (m_tdata_reg.timerid != NULL)
        ua_delete_timer_queue(&m_tdata_reg, TRUE);
    if (m_pcontext_eXosip != NULL)
    {
        eXosip_quit(m_pcontext_eXosip);
		osip_free(m_pcontext_eXosip);
		m_pcontext_eXosip = NULL;
    }
	if (m_panswer != NULL)
	{
		osip_message_free(m_panswer);
		m_panswer = NULL;
	}

	// UA事件
	m_psem_ua_event = NULL;
	m_pua_event_callback = NULL;
	m_pua_event_callback_param = NULL;
	ua_sem_destroy(&m_sem_ua_event_callback);
}

// 设置本地ip和port(sip协议地址和端口)
void CSipUA::set_local_addr(const char *p_ip, WORD port)
{
    if (p_ip != NULL)
    {
        strncpy(m_local_ip, p_ip, INET6_ADDRSTRLEN);
        m_local_ip[INET6_ADDRSTRLEN - 1] = '\0';
        strncpy(m_contact_ip, p_ip, INET6_ADDRSTRLEN);
        m_contact_ip[INET6_ADDRSTRLEN - 1] = '\0';
    }
    m_local_port = port;
    m_contact_port = port;
}

// 设置联系ip和port(用来控制服务器上显示的ip和port)
void CSipUA::set_contact_addr(const char *p_ip, WORD port)
{
    if (p_ip != NULL)
    {
        strncpy(m_contact_ip, p_ip, INET6_ADDRSTRLEN);
        m_contact_ip[INET6_ADDRSTRLEN - 1] = '\0';
    }
    m_contact_port = port;
}

// 设置注册服务器ip(可以是域名)和port
void CSipUA::set_register_addr(const char *p_ip, WORD port)
{
    if (p_ip != NULL)
    {
        strncpy(m_reg_ip, p_ip, INET6_ADDRSTRLEN);
        m_reg_ip[INET6_ADDRSTRLEN - 1] = '\0';
    }
    m_reg_port = port;
}

// 设置用户名和密码
void CSipUA::set_username_password(const char *username, const char *password)
{
	int		port;

    strncpy(m_username, username, SIP_UA_USERNAME_LEN);
    m_username[SIP_UA_USERNAME_LEN - 1] = '\0';
	port = SIP_RTP_AUDIO_PORT_DEFAULT + atoi(m_username);
	if (port > SIP_RTP_PORT_MAX)
	{
		while (port > SIP_RTP_PORT_MAX)
			port -= SIP_RTP_PORT_MIN;
	}
	m_rtp_audio_port = port;
	port = SIP_RTP_VIDEO_PORT_DEFAULT + atoi(m_username);
	if (port > SIP_RTP_PORT_MAX)
	{
		while (port > SIP_RTP_PORT_MAX)
			port -= SIP_RTP_PORT_MIN;
	}
	m_rtp_video_port = port;
    strncpy(m_password, password, SIP_UA_USERNAME_LEN);
    m_password[SIP_UA_USERNAME_LEN - 1] = '\0';
}

// 初始化
BOOL CSipUA::init(void)
{
    BOOL                b_result = FALSE;
    int                 result;
	char	            p_strerr[PATH_MAX] = {0};

    try
    {
		if (m_pcontext_eXosip == NULL)
            throw 10;
 		if (!m_audio_stream.init())
			throw 11;
 		if (!m_video_stream.init())
			throw 12;
       result = eXosip_listen_addr(m_pcontext_eXosip, IPPROTO_UDP, m_local_ip, m_local_port, AF_INET, 0);
        if (result != 0)
        {
            sprintf(p_strerr, "[CSipUA::init(listen_addr)](%d) : %s\n", result, osip_strerror(result));
			if (result != OSIP_WRONG_STATE)
				throw 13;
        }
        eXosip_set_user_agent(m_pcontext_eXosip, SIP_UA_STRING);
        result = eXosip_add_authentication_info(m_pcontext_eXosip, m_username, m_username, m_password, NULL, NULL);
        if (result != 0)
        {
            sprintf(p_strerr, "[CSipUA::init(add_authentication_info)](%d) : %s\n", result, osip_strerror(result));
            throw 14;
        }

		if (m_tdata_reg.timerid != NULL)
			ua_delete_timer_queue(&m_tdata_reg, TRUE);
        b_result = ua_create_timer_queue(&m_tdata_reg, on_timer_queue, m_register_interval, m_register_interval, TIMER_SIVAL_REGISTER);
		m_registering = FALSE;
        b_result &= register_server();
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::init()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::init()] : error catch!\n", m_username);
    }

    return b_result;
}

// 服务器注销
BOOL CSipUA::unregister_server(void)
{
	BOOL	unregister = FALSE;
    int     expires = m_expires;

    if (m_registered)
    {
        m_expires = 0;
        if (register_server())
            m_unregistering = TRUE;
        m_expires = expires;
		unregister = TRUE;
    }
	return unregister;
}

// 服务器注册
BOOL CSipUA::register_server(void)
{
    BOOL			b_result = FALSE;
    int				result;
	char			p_strerr[PATH_MAX] = {0};
	char			fromuser[SIP_UA_USERNAME_LEN + SIP_UA_USERNAME_LEN];
	char			proxy[SIP_UA_USERNAME_LEN + SIP_UA_USERNAME_LEN];
	char			contact[SIP_UA_USERNAME_LEN + SIP_UA_USERNAME_LEN];
	char			call_info[64];
	osip_message_t  *p_message = NULL;
	osip_contact_t	*p_contact = NULL;

    try
    {
        if ((m_pcontext_eXosip == NULL) || m_registering)
            throw 10;
        eXosip_lock(m_pcontext_eXosip);
		eXosip_register_remove(m_pcontext_eXosip, m_reg_id);

		sprintf(fromuser, "sip:%s@%s:%d", m_username, m_reg_ip, m_reg_port);
		sprintf(proxy, "sip:%s:%d", m_reg_ip, m_reg_port);
		//sprintf(contact, "sip:%s@%s:%d", m_username, m_contact_ip, m_contact_port);
		if (m_net_ip[0] != '\0' && m_net_port != 0)
			sprintf(contact, "sip:%s@%s:%d", m_username, m_net_ip, m_net_port);
		else
			sprintf(contact, "sip:%s@%s:%d", m_username, m_contact_ip, m_contact_port);
		m_reg_id = eXosip_register_build_initial_register(m_pcontext_eXosip, fromuser, proxy, contact, m_expires, &p_message);
		if (m_reg_id < 1)
		{
			sprintf(p_strerr, "[CSipUA::register_server(register_build_initial_register)] : failed\n");
			throw 11;
		}
        osip_message_set_supported(p_message, "100rel");
        osip_message_set_supported(p_message, "path");
		if (m_registered)
			sprintf(call_info, "<sip:%s>;spon_device;", m_reg_ip);
		else
			sprintf(call_info, "<sip:%s>;spon_device;first_registration;", m_reg_ip);
		osip_message_set_call_info(p_message, call_info);
		osip_message_set_allow(p_message, "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
        result = eXosip_register_send_register(m_pcontext_eXosip, m_reg_id, p_message);
        if (result != 0)
        {
            sprintf(p_strerr, "[CSipUA::register_server(register_send_register)](%d) : %s\n", result, osip_strerror(result));
            throw 12;
        }

        eXosip_unlock(m_pcontext_eXosip);
        m_registering = TRUE;
        b_result = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 12:
        case 11:
            eXosip_unlock(m_pcontext_eXosip);
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::register_server()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::register_server()] : error catch!\n", m_username);
    }

    return b_result;
}

// sip事件处理
void CSipUA::process_sip_event(void(*p_callback)(eXosip_event_t *, void *), void *p_param)
{
	char					p_strerr[PATH_MAX] = {0};
	eXosip_event_t			*p_event = NULL;
	BOOL					b_callback = TRUE;

	// 响应SDP(用于UAC)
	sdp_message_t			*p_msg_rsp = NULL;
	sdp_media_t				*p_md_rsp = NULL;
	char					*p_payload_str = NULL;		// 服务器优先编码值

	osip_message_t			*p_msg_sip = NULL;
	osip_body_t				*p_body = NULL;

    try
    {
        if (m_pcontext_eXosip == NULL)
            throw 10;
        p_event = eXosip_event_wait(m_pcontext_eXosip, 0, 0);
        eXosip_lock(m_pcontext_eXosip);
        eXosip_automatic_action(m_pcontext_eXosip);
        eXosip_unlock(m_pcontext_eXosip);
        if (p_event != NULL)
        {
            switch (p_event->type)
            {
            case EXOSIP_REGISTRATION_SUCCESS:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - registrered successfully\n", m_username);
				sip_event_registration_success(p_event);
                break;
            case EXOSIP_REGISTRATION_FAILURE:
				if (p_event->response != NULL && p_event->response->status_code != 401)
					printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - registrered failed\n", m_username);
				sip_event_registration_failure(p_event);
                break;
            case EXOSIP_CALL_PROCEEDING:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - proceeding\n", m_username);
                break;
            case EXOSIP_CALL_RINGING:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - ringing\n", m_username);
                m_status = SIP_STATUS_CALLING;
				m_call_id = p_event->cid;
				m_dialog_id = p_event->did;
                break;
            case EXOSIP_CALL_ANSWERED:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - call answered\n", m_username);
				sip_event_call_answerd(p_event);
                break;
            case EXOSIP_CALL_REQUESTFAILURE:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - request failure\n", m_username);
				if ((m_status == SIP_STATUS_CALLING || m_status == SIP_STATUS_CALL_REQUEST) \
					&& m_call_id == p_event->cid)
				{
					if (p_callback != NULL)
						p_callback(p_event, p_param);
					b_callback = FALSE;
					m_status = SIP_STATUS_NULL;
					m_call_id = 0;
					m_task_type = SIP_TASK_TYPE_NULL;
					m_audio_stream.rtp_unconnect();
					m_video_stream.rtp_unconnect();
					m_remote_ip[0] = '\0';
					m_remote_aport = m_remote_vport = 0;
					m_bc_is_multicast = FALSE;
				}
				else
				{
					if (m_call_id == 0)
					{
						m_call_id = p_event->cid;
						m_dialog_id = p_event->did;
					}
					b_callback = FALSE;
				}
                break;
            case EXOSIP_CALL_MESSAGE_NEW:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - new call message\n", m_username);
                break;
			case EXOSIP_CALL_RELEASED:
				printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - call released\n", m_username);
				//printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : p_event->cid:%d; m_call_id:%d;\n", m_username, p_event->cid, m_call_id);
				if (m_call_id == p_event->cid)
					b_callback = TRUE;
				else
					b_callback = FALSE;
				break;
            case EXOSIP_CALL_CLOSED:
				printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - call closed\n", m_username);
				//printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : p_event->cid:%d; m_call_id:%d;\n", m_username, p_event->cid, m_call_id);
				//printf("call id : %d-%d; dialog id : %d-%d;\n", m_call_id, p_event->cid, m_dialog_id, p_event->did);
				if (m_call_id == p_event->cid)
				{
					if (p_callback != NULL)
						p_callback(p_event, p_param);
					b_callback = FALSE;
					m_status = SIP_STATUS_NULL;
					m_call_id = 0;
					m_task_type = SIP_TASK_TYPE_NULL;
					m_audio_stream.rtp_unconnect();
					m_video_stream.rtp_unconnect();
					m_remote_ip[0] = '\0';
					m_remote_aport = m_remote_vport = 0;
					m_bc_is_multicast = FALSE;
				}
				else
					b_callback = FALSE;
                break;
            case EXOSIP_CALL_INVITE:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - invite(from %s)\n", m_username, p_event->request->from->displayname);
				if (m_status == SIP_STATUS_NULL)
					sip_event_invite(p_event);
				else
				{
					eXosip_call_send_answer(m_pcontext_eXosip, p_event->tid, 486, NULL);
					b_callback = FALSE;
				}
				//printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : p_event->cid:%d; m_call_id:%d;\n", m_username, p_event->cid, m_call_id);
                break;
            case EXOSIP_CALL_GLOBALFAILURE:		// 未接听就挂断
				printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - global failure\n", m_username);
				break;
            case EXOSIP_CALL_ACK:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - ack\n", m_username);
				if (m_status == SIP_STATUS_RINGING)
				{	// 建立RTP连接
					m_call_id = p_event->cid;
					m_dialog_id = p_event->did;
					switch (m_task_type)
					{
					case SIP_TASK_TYPE_BC_OUTGOING:
					case SIP_TASK_TYPE_BC_INCOMING:
						m_status = SIP_STATUS_BCING;
						break;
					case SIP_TASK_TYPE_MONITOR_OUTGOING:
					case SIP_TASK_TYPE_MONITOR_INCOMING:
						m_status = SIP_STATUS_MONITORING;
						break;
					case SIP_TASK_TYPE_TALK_OUTGOING:
					case SIP_TASK_TYPE_TALK_INCOMING:
					default:
						m_status = SIP_STATUS_TALKING;
						break;
					}

					// 响应SIP消息中SDP分析
					eXosip_lock(m_pcontext_eXosip);
					p_msg_rsp = eXosip_get_remote_sdp(m_pcontext_eXosip, m_dialog_id);
					if (p_msg_rsp != NULL)
					{
						if (m_remote_ip[0] == '\0')
							process_sdp_message(p_msg_rsp);
						sdp_message_free(p_msg_rsp);
						p_msg_rsp = NULL;
					}
					eXosip_unlock(m_pcontext_eXosip);

					if (m_remote_ip[0] == '\0')
					{
						sprintf(p_strerr, "[CSipUA::process_sip_event()] : remote ip is null\n");
						throw 11;
					}
					rtp_connect_audio();
					if (m_pmine_type_video != NULL && m_enable_rtp_video)
						rtp_connect_video();
				}
                break;
			case EXOSIP_MESSAGE_NEW:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - new message\n", m_username);
				if (MSG_IS_MESSAGE(p_event->request) || MSG_IS_NOTIFY(p_event->request))
				{
					osip_message_get_body(p_event->request, 0, &p_body);
					if (p_body != NULL && p_body->body != NULL && p_body->body[0] == '[')
						printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : recived message : \n\t%s\n", m_username, p_body->body);
						//printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : recived message : \n/**********\n%s\n**********/\n", m_username, p_body->body);
				}
				eXosip_message_build_answer(m_pcontext_eXosip, p_event->tid, 202, &p_msg_sip);
				eXosip_message_send_answer(m_pcontext_eXosip, p_event->tid, 202, p_msg_sip);
				break;
			case EXOSIP_CALL_MESSAGE_ANSWERED:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - call message answered\n", m_username);
				break;
            default:
                printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : EVENT - recieved unknown event (type, did, cid) = (%d, %d, %d)\n", m_username, \
                    p_event->type, p_event->did, p_event->cid);
                break;
            }
			if (p_callback != NULL && b_callback)
				p_callback(p_event, p_param);
            eXosip_event_free(p_event);
        }
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::process_sip_event()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::process_sip_event()] : error catch!\n", m_username);
    }
}

// SIP事件(注册成功)
void CSipUA::sip_event_registration_success(eXosip_event_t *p_event)
{
	osip_call_info_t		*p_call_info = NULL;
	osip_generic_param_t	*p_uri_param = NULL;
	int						i, j, size;
	int						call_map_index = 0;
	CPlainParse				parse;

	m_registering = FALSE;
    if (m_unregistering)
	{
        m_unregistering = m_registered = FALSE;
		m_net_ip[0] = '\0';
		m_net_port = 0;
	}
    else
        m_registered = TRUE;
	m_register_failure_count = 0;

	if (m_tdata_reg.timerid != NULL && m_tdata_reg.sival != TIMER_SIVAL_HANDSHAKE)
	{
		ua_delete_timer_queue(&m_tdata_reg, TRUE);
		ua_create_timer_queue(&m_tdata_reg, on_timer_queue, m_handshake_interval, m_handshake_interval, TIMER_SIVAL_HANDSHAKE);
	}

	if (p_event->response != NULL)
	{
		osip_message_get_call_info(p_event->response, 0, &p_call_info);
		if (p_call_info != NULL)
		{
			memset(&m_call_target, 0, sizeof (m_call_target));
			memset(&m_message_target, 0, sizeof (m_message_target));
			memset(&m_server_date, 0, sizeof (struct tm));
			size = osip_list_size(&p_call_info->gen_params);
			for (i = 0; i < size; i++)
			{
				p_uri_param = (osip_generic_param_t *)osip_list_get(&p_call_info->gen_params, i);
				if (p_uri_param != NULL)
				{
					if (strncasecmp(p_uri_param->gname, "call_target", strlen("call_target")) == 0)
					{
						call_map_index = atoi(&p_uri_param->gname[strlen("call_target")]) - 1;
						if (p_uri_param->gvalue != NULL && call_map_index >= 0 && call_map_index < SIP_UA_CALL_KEY_MAX)
						{
							strncpy(m_call_target[call_map_index], p_uri_param->gvalue, SIP_UA_USERNAME_LEN);
							m_call_target[call_map_index][SIP_UA_USERNAME_LEN - 1] = '\0';
						}
					}
					else if (strncasecmp(p_uri_param->gname, "message_target", strlen("message_target")) == 0)
					{
						if (p_uri_param->gvalue != NULL)
						{
							parse.set_delimit('-');
							parse.set_body(p_uri_param->gvalue);
							for (j = 0; j < parse.m_plain_label.count && j < SIP_UA_MESSAGE_TARGET_MAX; j++)
								m_message_target[j] = atoi(parse.m_plain_label.p_text[j]);
						}
					}
					else if (strncasecmp(p_uri_param->gname, "date", strlen("date")) == 0)
					{
						if (p_uri_param->gvalue != NULL)
						{
							parse.set_delimit('-');
							parse.set_body(p_uri_param->gvalue);
							if (parse.m_plain_label.count >= 6)
							{
								m_server_date.tm_year = atoi(parse.m_plain_label.p_text[0]) - 1900;
								m_server_date.tm_mon = atoi(parse.m_plain_label.p_text[1]) - 1;
								m_server_date.tm_mday = atoi(parse.m_plain_label.p_text[2]);
								m_server_date.tm_hour = atoi(parse.m_plain_label.p_text[3]);
								m_server_date.tm_min = atoi(parse.m_plain_label.p_text[4]);
								m_server_date.tm_sec = atoi(parse.m_plain_label.p_text[5]);
								char			p_time[PATH_MAX];
								strftime(p_time, PATH_MAX, "%Y-%m-%d %X", &m_server_date);
								printf_log(LOG_IS_INFO, "[%s][CSipUA::sip_event_registration_success()] : date : %s\n", m_username, p_time);
							}
						}
					}
					else if (strncasecmp(p_uri_param->gname, "talk_input_volume", strlen("talk_input_volume")) == 0)
					{
						if (p_uri_param->gvalue != NULL)
							m_talk_input_volume = atoi(p_uri_param->gvalue);
					}
					else if (strncasecmp(p_uri_param->gname, "talk_output_volume", strlen("talk_output_volume")) == 0)
					{
						if (p_uri_param->gvalue != NULL)
							m_talk_output_volume = atoi(p_uri_param->gvalue);
					}
					else if (strncasecmp(p_uri_param->gname, "broadcast_input_volume", strlen("broadcast_input_volume")) == 0)
					{
						if (p_uri_param->gvalue != NULL)
							m_bc_input_volume = atoi(p_uri_param->gvalue);
					}
					else if (strncasecmp(p_uri_param->gname, "broadcast_output_volume", strlen("broadcast_output_volume")) == 0)
					{
						if (p_uri_param->gvalue != NULL)
							m_bc_output_volume = atoi(p_uri_param->gvalue);
					}
					
					//if (p_uri_param->gvalue != NULL)
					//	printf_log(LOG_IS_INFO, "[%s][CSipUA::sip_event_registration_success()] : call info : %s - %s\n", m_username, p_uri_param->gname, p_uri_param->gvalue);
					//else
					//	printf_log(LOG_IS_INFO, "[%s][CSipUA::sip_event_registration_success()] : call info : %s\n", m_username, p_uri_param->gname);
				}
			}
		}
	}
}

// SIP事件(注册失败)
void CSipUA::sip_event_registration_failure(eXosip_event_t *p_event)
{
	osip_via_t				*p_via_response = NULL, *p_via_reg = NULL;
	osip_generic_param_t	*p_uri_param = NULL;

    m_registering = FALSE;
	if (p_event->response != NULL && p_event->response->status_code == 401)
		m_register_failure_count++;
	if (m_register_failure_count >= 2)
	{
		m_registered = FALSE;
		m_register_failure_count = 0;
		if (m_tdata_reg.timerid != NULL)
			ua_delete_timer_queue(&m_tdata_reg, TRUE);
		ua_create_timer_queue(&m_tdata_reg, on_timer_queue, m_register_interval, m_register_interval, TIMER_SIVAL_REGISTER);
		register_server();
	}
	if (p_event->response != NULL \
		&& (407 == p_event->response->status_code || 401 == p_event->response->status_code) \
		&& m_net_ip[0] == '\0')
	{
		if ((osip_message_get_via(p_event->response, 0, &p_via_response) >= 0)
			&& (osip_message_get_via(p_event->request, 0, &p_via_reg) >= 0))
		{
			if (osip_via_param_get_byname(p_via_response, (char *)"received", &p_uri_param) >= 0)
			{
				printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : REGISTER(%s - %s)\n", m_username, p_uri_param->gname, p_uri_param->gvalue);
				if (p_uri_param->gvalue != NULL)
					osip_clrncpy(m_net_ip, p_uri_param->gvalue, INET6_ADDRSTRLEN - 1);
			}
			if (osip_via_param_get_byname(p_via_response, (char *)"rport", &p_uri_param) >= 0)
			{
				printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sip_event()] : REGISTER(%s - %s)\n", m_username, p_uri_param->gname, p_uri_param->gvalue);
				if (p_uri_param->gvalue != NULL)
					m_net_port = atoi(p_uri_param->gvalue);
			}
			eXosip_masquerade_contact(m_pcontext_eXosip, m_net_ip, m_net_port);
			if (osip_via_param_get_byname(p_via_response, (char *)"received", &p_uri_param) >= 0)
				register_server();
		}
	}
}

// SIP事件(应答)
void CSipUA::sip_event_call_answerd(eXosip_event_t *p_event)
{
	char					p_strerr[PATH_MAX] = {0};
	int						i, size;
	osip_call_info_t		*p_call_info = NULL;
	osip_generic_param_t	*p_uri_param = NULL;
	osip_message_t			*p_msg_sip = NULL;
	// 响应SDP(用于UAC)
	sdp_message_t			*p_msg_rsp = NULL;
	sdp_media_t				*p_md_rsp = NULL;
	char					*p_payload_str = NULL;		// 服务器优先编码值

    try
    {
        if (m_status == SIP_STATUS_CALLING || m_status == SIP_STATUS_BC_REQUEST)
		{
			m_call_id = p_event->cid;
			m_dialog_id = p_event->did;
			switch (m_task_type)
			{
			case SIP_TASK_TYPE_BC_OUTGOING:
			case SIP_TASK_TYPE_BC_INCOMING:
				m_status = SIP_STATUS_BCING;
				break;
			case SIP_TASK_TYPE_MONITOR_OUTGOING:
			case SIP_TASK_TYPE_MONITOR_INCOMING:
				m_status = SIP_STATUS_MONITORING;
				break;
			case SIP_TASK_TYPE_TALK_OUTGOING:
			case SIP_TASK_TYPE_TALK_INCOMING:
			default:
				m_status = SIP_STATUS_TALKING;
				break;
			}

			eXosip_lock(m_pcontext_eXosip);
			eXosip_call_build_ack(m_pcontext_eXosip, m_dialog_id, &p_msg_sip);
			eXosip_call_send_ack(m_pcontext_eXosip, m_dialog_id, p_msg_sip);
			// 响应SIP消息中SDP分析
			p_msg_rsp = eXosip_get_remote_sdp(m_pcontext_eXosip, m_dialog_id);
			if (p_msg_rsp != NULL)
			{
				process_sdp_message(p_msg_rsp);
				sdp_message_free(p_msg_rsp);
				p_msg_rsp = NULL;
			}
			eXosip_unlock(m_pcontext_eXosip);

			if (p_event->response != NULL)
			{
				osip_message_get_call_info(p_event->response, 0, &p_call_info);
				if (p_call_info != NULL)
				{
					size = osip_list_size(&p_call_info->gen_params);
					for (i = 0; i < size; i++)
					{
						p_uri_param = (osip_generic_param_t *)osip_list_get(&p_call_info->gen_params, i);
						if (p_uri_param != NULL)
						{
							switch (p_uri_param->gname[0])
							{
							case 'm':
								if (strncasecmp(p_uri_param->gname, "multi_ip", 8) == 0)
								{
									strncpy(m_remote_ip, p_uri_param->gvalue, INET6_ADDRSTRLEN);
									m_remote_ip[INET6_ADDRSTRLEN - 1] = '\0';
									m_bc_is_multicast = TRUE;
								}
								else if (strncasecmp(p_uri_param->gname, "multi_port", 10) == 0)
									m_remote_aport = m_remote_vport = atoi(p_uri_param->gvalue);
								break;
							default:
								break;
							}
						}
					}
				}
			}

			// 建立RTP连接
			rtp_connect_audio();
			if (m_pmine_type_video != NULL && m_enable_rtp_video)
				rtp_connect_video();
		}
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::sip_event_call_answerd()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::sip_event_call_answerd()] : error catch!\n", m_username);
    }
}

// 处理sdp视频消息
void CSipUA::process_sdp_video_message(sdp_message_t *p_msg_sdp)
{
	int						i, size;
	osip_generic_param_t	*p_uri_param = NULL;
	sdp_media_t				*p_md_rsp = NULL;
	char					*p_payload_str = NULL;		// 服务器优先编码值
	char					*p_level_id = NULL;

	m_pmine_type_video = NULL;
	memset(m_profile_level_id, 0, sizeof (m_profile_level_id));
	if (p_msg_sdp != NULL)
	{
		p_md_rsp = eXosip_get_video_media(p_msg_sdp);
		if (p_md_rsp != NULL)
		{
			// 取服务器支持的最优先的编码方式
			p_payload_str = (char *)osip_list_get(&p_md_rsp->m_payloads, 0);
			m_remote_vport = atoi(p_md_rsp->m_port);
			m_payload_type_video = atoi(p_payload_str);
			printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sdp_video_message()] : payload : %s:%d,%d\n", m_username, m_remote_ip, m_remote_vport, m_payload_type_video);

			size = osip_list_size(&p_md_rsp->a_attributes);
			for (i = 0; i < size; i++)
			{
				p_uri_param = (osip_generic_param_t *)osip_list_get(&p_md_rsp->a_attributes, i);
				if (p_uri_param != NULL)
				{
					if (m_pmine_type_video == NULL && strncasecmp(p_uri_param->gname, "rtpmap", 6) == 0)
					{
						if (strncasecmp(p_uri_param->gvalue, p_payload_str, strlen(p_payload_str)) == 0)
						{
							// 编码方式
							if (strstr(p_uri_param->gvalue, g_mime_type_h264) != NULL)
								m_pmine_type_video = g_mime_type_h264;
							else
							{
								printf_log(LOG_IS_ERR, "[%s][CSipUA::process_sdp_video_message()] : video format(payload : %s) is not supported\n", m_username, p_uri_param->gvalue);
								m_pmine_type_video = g_mime_type_h264;
							}
							// 采样率
							if (strstr(p_uri_param->gvalue, "90000") != NULL)
								m_video_sample_rate = 90000;
							else 
							{
								printf_log(LOG_IS_ERR, "[%s][CSipUA::process_sdp_video_message()] : video sample rate(payload : %s) is not supported\n", m_username, p_uri_param->gvalue);
								m_video_sample_rate = 90000;
							}
						}
					}
					else if (m_profile_level_id[0] == '\0' && strncasecmp(p_uri_param->gname, "fmtp", 4) == 0)
					{
						if (strncasecmp(p_uri_param->gvalue, p_payload_str, strlen(p_payload_str)) == 0)
						{
							// profile_level_id
							p_level_id = strstr(p_uri_param->gvalue, "profile-level-id=");
							if (p_level_id != NULL)
							{
								p_level_id += strlen("profile-level-id=");
								if (strlen(p_level_id) >= VIDEO_PROFILE_LEVEL_ID_LEN)
									memcpy(m_profile_level_id, p_level_id, VIDEO_PROFILE_LEVEL_ID_LEN);
								m_profile_level_id[VIDEO_PROFILE_LEVEL_ID_LEN] = '\0';
							}
						}
					}
				}
			}
		}
	}
}

// 处理sdp音频消息
void CSipUA::process_sdp_audio_message(sdp_message_t *p_msg_sdp)
{
	int						i, size;
	osip_generic_param_t	*p_uri_param = NULL;
	sdp_media_t				*p_md_rsp = NULL;
	char					*p_payload_str = NULL;		// 服务器优先编码值

	m_pmine_type_audio = NULL;
	if (p_msg_sdp != NULL)
	{
		p_md_rsp = eXosip_get_audio_media(p_msg_sdp);
		if (p_md_rsp != NULL)
		{
			// 取服务器支持的最优先的编码方式
			p_payload_str = (char *)osip_list_get(&p_md_rsp->m_payloads, 0);
			m_remote_aport = atoi(p_md_rsp->m_port);
			m_payload_type_audio = atoi(p_payload_str);
			printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sdp_audio_message()] : payload : %s:%d,%d\n", m_username, m_remote_ip, m_remote_aport, m_payload_type_audio);
			if (m_payload_type_audio == 0)
			{
				m_pmine_type_audio = g_mime_type_pcmu;
				m_audio_sample_rate = 8000;
			}

			size = osip_list_size(&p_md_rsp->a_attributes);
			for (i = 0; i < size; i++)
			{
				p_uri_param = (osip_generic_param_t *)osip_list_get(&p_md_rsp->a_attributes, i);
				if (p_uri_param != NULL)
				{
					if (m_pmine_type_audio == NULL && strncasecmp(p_uri_param->gname, "rtpmap", 6) == 0)
					{
						if (strncasecmp(p_uri_param->gvalue, p_payload_str, strlen(p_payload_str)) == 0)
						{
							// 编码方式
							if (strstr(p_uri_param->gvalue, g_mime_type_mp3) != NULL)
								m_pmine_type_audio = g_mime_type_mp3;
							else if (strstr(p_uri_param->gvalue, g_mime_type_l16) != NULL)
								m_pmine_type_audio = g_mime_type_l16;
							else if (strstr(p_uri_param->gvalue, g_mime_type_pcma) != NULL)
								m_pmine_type_audio = g_mime_type_pcma;
							else if (strstr(p_uri_param->gvalue, g_mime_type_pcmu) != NULL)
								m_pmine_type_audio = g_mime_type_pcmu;
							else
							{
								printf_log(LOG_IS_ERR, "[%s][CSipUA::process_sdp_audio_message()] : audio format(payload : %s) is not supported\n", m_username, p_uri_param->gvalue);
								m_pmine_type_audio = g_mime_type_pcmu;
							}
							// 采样率
							if (strstr(p_uri_param->gvalue, "44100") != NULL)
								m_audio_sample_rate = 44100;
							else if (strstr(p_uri_param->gvalue, "22050") != NULL)
								m_audio_sample_rate = 22050;
							else if (strstr(p_uri_param->gvalue, "11025") != NULL)
								m_audio_sample_rate = 11025;
							else if (strstr(p_uri_param->gvalue, "32000") != NULL)
								m_audio_sample_rate = 32000;
							else if (strstr(p_uri_param->gvalue, "16000") != NULL)
								m_audio_sample_rate = 16000;
							else if (strstr(p_uri_param->gvalue, "8000") != NULL)
								m_audio_sample_rate = 8000;
							else 
							{
								printf_log(LOG_IS_ERR, "[%s][CSipUA::process_sdp_audio_message()] : audio sample rate(payload : %s) is not supported\n", m_username, p_uri_param->gvalue);
								m_audio_sample_rate = 8000;
							}
						}
					}
				}
			}
		}
	}
}

// 处理sdp消息
void CSipUA::process_sdp_message(sdp_message_t *p_msg_sdp)
{
	sdp_connection_t		*p_con_rsp = NULL; 

	if (p_msg_sdp != NULL)
	{
		p_con_rsp = eXosip_get_audio_connection(p_msg_sdp);
		if (p_con_rsp != NULL)
			strncpy(m_remote_ip, p_con_rsp->c_addr, INET6_ADDRSTRLEN);
		m_remote_ip[INET6_ADDRSTRLEN - 1] = '\0';
		// 处理音频sdp
		process_sdp_audio_message(p_msg_sdp);
		process_sdp_video_message(p_msg_sdp);
	}
	printf_log(LOG_IS_INFO, "[%s][CSipUA::process_sdp_message()] : payload : audio(%s/%d);video(%s/%d)\n", \
		m_username, m_pmine_type_audio ? m_pmine_type_audio : "?", m_audio_sample_rate, m_pmine_type_video ? m_pmine_type_video : "?", m_video_sample_rate);
}

// SIP事件(呼叫)
void CSipUA::sip_event_invite(eXosip_event_t *p_event)
{
	char					p_strerr[PATH_MAX] = {0};
	int						i, size;
	osip_call_info_t		*p_call_info = NULL;
	osip_generic_param_t	*p_uri_param = NULL;
	osip_from_t				*p_from = NULL;
	// 响应SDP(用于UAC)
	sdp_message_t			*p_msg_rsp = NULL;
	sdp_media_t				*p_md_rsp = NULL;
	char					*p_payload_str = NULL;		// 服务器优先编码值

    try
    {
        if (m_pcontext_eXosip != NULL && p_event != NULL && p_event->type == EXOSIP_CALL_INVITE && m_status == SIP_STATUS_NULL)
        {
            m_status = SIP_STATUS_CALLING;
			m_task_type = SIP_TASK_TYPE_TALK_INCOMING;
			m_call_id = p_event->cid;
			m_dialog_id = p_event->did;
			m_trans_id = p_event->tid;


			// 响应SIP消息中SDP分析
			p_msg_rsp = eXosip_get_remote_sdp(m_pcontext_eXosip, m_dialog_id);
			if (p_msg_rsp != NULL)
			{
				process_sdp_message(p_msg_rsp);
				sdp_message_free(p_msg_rsp);
				p_msg_rsp = NULL;
			}

			if (p_event->request != NULL)
			{
				p_from = osip_message_get_from(p_event->request);
				if (p_event->request->from != NULL)
				{
					strncpy(m_callee_username, p_event->request->from->url->username, SIP_UA_USERNAME_LEN);
					m_callee_username[SIP_UA_USERNAME_LEN - 1] = '\0';
				}

				osip_message_get_call_info(p_event->request, 0, &p_call_info);
				if (p_call_info != NULL)
				{
					size = osip_list_size(&p_call_info->gen_params);
					for (i = 0; i < size; i++)
					{
						p_uri_param = (osip_generic_param_t *)osip_list_get(&p_call_info->gen_params, i);
						if (p_uri_param != NULL)
						{
							if (p_uri_param->gname != NULL && p_uri_param->gvalue != NULL)
							{
								if (strcasecmp(p_uri_param->gname, "task_type") == 0 || strcasecmp(p_uri_param->gname, "type_task") == 0 )
								{
									if (strcasecmp(p_uri_param->gvalue, "talk") == 0)
										m_task_type = SIP_TASK_TYPE_TALK_INCOMING;
									else if (strcasecmp(p_uri_param->gvalue, "monitor") == 0)
										m_task_type = SIP_TASK_TYPE_MONITOR_INCOMING;
									else if (strcasecmp(p_uri_param->gvalue, "broadcast") == 0)
										m_task_type = SIP_TASK_TYPE_BC_INCOMING;
								}
								else if (strcasecmp(p_uri_param->gname, "transmit") == 0)
								{
									if (strcasecmp(p_uri_param->gvalue, "multicast") == 0)
										m_bc_is_multicast = TRUE;
									else
										m_bc_is_multicast = FALSE;
								}
								else if (strcasecmp(p_uri_param->gname, "multi_ip") == 0)
								{
									strncpy(m_remote_ip, p_uri_param->gvalue, INET6_ADDRSTRLEN);
									m_remote_ip[INET6_ADDRSTRLEN - 1] = '\0';
								}
								else if (strcasecmp(p_uri_param->gname, "multi_port") == 0)
									m_remote_aport = m_remote_vport = atoi(p_uri_param->gvalue);
							}
						}
					}
				}
			}
			if (m_audio_stream.m_audio_src == AUDIO_SRC_FILE)
				m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
			ring();
			if (m_task_type == SIP_TASK_TYPE_BC_INCOMING || m_task_type == SIP_TASK_TYPE_MONITOR_INCOMING)
				answer();
		}
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::sip_event_invite()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::sip_event_invite()] : error catch!\n", m_username);
    }
}

// 设置服务器上的注册失效时间(掉线时间)
void CSipUA::set_expires(int expires)
{
    if (expires >= 30)
    {
        m_expires = expires;
        m_handshake_interval = (m_expires - 10) * 1000;
    }            
}

// 定时器处理
#ifdef WIN32
void CALLBACK CSipUA::on_timer_queue(void *p_param, BOOL timeorwait)
{
    timer_data_t    *p_tdata = (timer_data_t *)p_param;
    CSipUA          *p_sip = (CSipUA *)p_tdata->p_param;

	switch (p_tdata->sival)
    {
    case TIMER_SIVAL_REGISTER:
        p_sip->register_server();
        break;
    case TIMER_SIVAL_HANDSHAKE:
        p_sip->register_server();
        break;
    default:
        break;
    }
}
#else
void CSipUA::on_timer_queue(union sigval sig_val)
{
    timer_data_t	*p_tdata = (timer_data_t *)sig_val.sival_ptr;
    CSipUA          *p_sip = (CSipUA *)p_tdata->p_param;

    switch (p_tdata->sival)
    {
    case TIMER_SIVAL_REGISTER:
        p_sip->register_server();
        break;
    case TIMER_SIVAL_HANDSHAKE:
        p_sip->register_server();
        break;
    default:
        break;
    }
}
#endif

// 对讲
BOOL CSipUA::talk(const char *callee, const char *src_ext_id, const char *dst_ext_id)
{
	BOOL			b_talk = FALSE;
	const char		*p_src_ext_id = src_ext_id, *p_dst_ext_id = dst_ext_id;
	char			ext_id_null[] = "0";
	char			*p_call_info = NULL;
	int				len;

	if (src_ext_id == NULL)
		p_src_ext_id = ext_id_null;
	if (dst_ext_id == NULL)
		p_dst_ext_id = ext_id_null;
	if (callee != NULL && strcmp(callee, m_username) != 0)	// 不能发给自己
	{
		len = snprintf(NULL, 0, "<sip:%s>;task_type=talk;src_ext_id=%s;dst_ext_id=%s", m_reg_ip, p_src_ext_id, p_dst_ext_id);
		p_call_info = (char *)ua_malloc(len + 1);
		snprintf(p_call_info, len + 1, "<sip:%s>;task_type=talk;src_ext_id=%s;dst_ext_id=%s", m_reg_ip, p_src_ext_id, p_dst_ext_id);
		b_talk = task_begin(callee, SIP_TASK_TYPE_TALK_OUTGOING, p_call_info);
		ua_free(p_call_info);
		p_call_info = NULL;
	}
	return b_talk;
}

// 广播
BOOL CSipUA::broadcast(const char *callee, const char *audio_type, BOOL is_term_broadcast)
{
	BOOL			b_broadcast = FALSE;
	char			term_broadcast[] = "term";
	char			area_broadcast[] = "area";
	char			audio_type_wav[] = "wav";
	char			audio_type_mp3[] = "mp3";
	char			*p_dst_broadcast = NULL, *p_audio_type = NULL;
	char			*p_call_info = NULL;
	int				len;

	if (is_term_broadcast)
		p_dst_broadcast = term_broadcast;
	else
		p_dst_broadcast = area_broadcast;
	if (strncasecmp(audio_type, audio_type_wav, strlen(audio_type_wav)) == 0)
		p_audio_type = audio_type_wav;
	else if (strncasecmp(audio_type, audio_type_mp3, strlen(audio_type_mp3)) == 0)
		p_audio_type = audio_type_mp3;
	if (is_term_broadcast && strcmp(callee, m_username) == 0)	// 不能发给自己
		callee = NULL;
	if (callee != NULL && p_audio_type != NULL)
	{
		len = snprintf(NULL, 0, "<sip:%s>;task_type=broadcast;class=%s;stream_type=push;data_type=%s;samplerate=22050;target=%s", \
			m_reg_ip, p_dst_broadcast, p_audio_type, callee);
		p_call_info = (char *)ua_malloc(len + 1);
		snprintf(p_call_info, len + 1, "<sip:%s>;task_type=broadcast;class=%s;stream_type=push;data_type=%s;samplerate=22050;target=%s", \
			m_reg_ip, p_dst_broadcast, p_audio_type, callee);
		b_broadcast = task_begin("broadcast", SIP_TASK_TYPE_BC_OUTGOING, p_call_info);
		ua_free(p_call_info);
		p_call_info = NULL;
	}
	return b_broadcast;
}

// 监听
BOOL CSipUA::monitor(const char *callee, const char *src_ext_id, const char *dst_ext_id)
{
	BOOL			b_monitor = FALSE;
	const char		*p_src_ext_id = src_ext_id, *p_dst_ext_id = dst_ext_id;
	char			ext_id_null[] = "0";
	char			*p_call_info = NULL;
	int				len;

	if (src_ext_id == NULL)
		p_src_ext_id = ext_id_null;
	if (dst_ext_id == NULL)
		p_dst_ext_id = ext_id_null;
	if (callee != NULL && strcmp(callee, m_username) != 0)	// 不能发给自己
	{
		len = snprintf(NULL, 0, "<sip:%s>;task_type=monitor;src_ext_id=%s;dst_ext_id=%s", m_reg_ip, p_src_ext_id, p_dst_ext_id);
		p_call_info = (char *)ua_malloc(len + 1);
		snprintf(p_call_info, len + 1, "<sip:%s>;task_type=monitor;src_ext_id=%s;dst_ext_id=%s", m_reg_ip, p_src_ext_id, p_dst_ext_id);

		b_monitor = task_begin(callee, SIP_TASK_TYPE_MONITOR_OUTGOING, p_call_info);
		ua_free(p_call_info);
		p_call_info = NULL;
	}
	return b_monitor;
}

// 发起任务(对讲,广播,监听)
BOOL CSipUA::task_begin(const char *callee, sip_task_type_t task_type, const char *call_info)
{
    BOOL                b_call = FALSE;
    osip_message_t      *p_invite = NULL;
	char                to_callee[SIP_UA_USERNAME_LEN  * 5];
	char                from_caller[SIP_UA_USERNAME_LEN + SIP_UA_USERNAME_LEN];
    int                 result;
    char                containing[1024];
	char				p_strerr[PATH_MAX] = {0};

    try
    {
        if ((m_pcontext_eXosip == NULL) || m_status != SIP_STATUS_NULL || callee == NULL)	//  || !m_registered
            throw 10;
		strncpy(m_callee_username, callee, SIP_UA_USERNAME_LEN);
		m_callee_username[SIP_UA_USERNAME_LEN - 1] = '\0';
		if (strchr(callee, '@') != NULL)	// 无服务器模式
		{
			sprintf(to_callee, "<sip:%s>", callee);
			sprintf(from_caller, "<sip:%s@%s:%d>", m_username, m_local_ip, m_local_port);
		}
		else
		{
			sprintf(to_callee, "<sip:%s@%s:%d>", callee, m_reg_ip, m_reg_port);
			sprintf(from_caller, "<sip:%s@%s:%d>", m_username, m_reg_ip, m_reg_port);
		}
        result = eXosip_call_build_initial_invite(m_pcontext_eXosip, &p_invite, to_callee, from_caller, NULL, SIP_UA_STRING);
        if (result != 0)
        {
            sprintf(p_strerr, "[CSipUA::task_begin(call_build_initial_invite)](%d) : %s\n", result, osip_strerror(result));
            throw 11;
        }
		osip_message_set_call_info(p_invite, call_info);
        osip_message_set_supported(p_invite, "100rel");
		memset(containing, 0, 1024);
        snprintf(containing, 1024,
                "v=0\r\n"
                "o=jack 0 0 IN IP4 %s\r\n"
                "s=talk\r\n"
                "c=IN IP4 %s\r\n"
                "t=0 0\r\n", 
				m_local_ip, m_local_ip);
		result = strlen(containing);
		m_payload_type_audio = m_payload_type_video = 0;
		m_audio_stream.get_media_containing(&containing[result], 1024 - result, m_rtp_audio_port, m_payload_type_audio, NULL, 8000);
		result = strlen(containing);
		m_video_stream.get_media_containing(&containing[result], 1024 - result, m_rtp_video_port, m_payload_type_video, NULL, m_video_sample_rate);
        osip_message_set_body(p_invite, containing, strlen(containing));
        osip_message_set_content_type(p_invite, "application/sdp");
		osip_message_set_allow(p_invite, "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
        
        eXosip_lock(m_pcontext_eXosip);
        result = eXosip_call_send_initial_invite(m_pcontext_eXosip, p_invite);
        eXosip_unlock(m_pcontext_eXosip);
        if (result < 0)
        {
            sprintf(p_strerr, "[CSipUA::task_begin(call_send_initial_invite)](%d) : %s\n", result, osip_strerror(result));
            throw 12;
        }

		switch (task_type)
		{
		case SIP_TASK_TYPE_TALK_OUTGOING:
		case SIP_TASK_TYPE_MONITOR_OUTGOING:
			m_status = SIP_STATUS_CALL_REQUEST;
			break;
		case SIP_TASK_TYPE_BC_OUTGOING:
			m_status = SIP_STATUS_BC_REQUEST;
			break;
		default:
			break;
		}
		m_task_type = task_type;
        b_call = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::task_begin()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::task_begin()] : error catch!\n", m_username);
    }

    return b_call;
}

// 应答
BOOL CSipUA::answer(void)
{
    BOOL                b_answer = FALSE;
	char				p_strerr[PATH_MAX] = {0};
    int                 result;

    try
    {
        if ((m_pcontext_eXosip == NULL) || m_status != SIP_STATUS_RINGING || m_panswer == NULL)
            throw 10;
        eXosip_lock(m_pcontext_eXosip);
		result = eXosip_call_send_answer(m_pcontext_eXosip, m_trans_id, 200, m_panswer);
 		m_panswer = NULL;
       if (result != 0)
        {
			eXosip_call_send_answer(m_pcontext_eXosip, m_trans_id, 603, NULL);
            sprintf(p_strerr, "[CSipUA::answer(call_send_answer)](%d) : %s\n", result, osip_strerror(result));
            throw 11;
        }
        eXosip_unlock(m_pcontext_eXosip);
		b_answer = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 11:
			eXosip_unlock(m_pcontext_eXosip);
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::answer()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::answer()] : error catch!\n", m_username);
    }

    return b_answer;
}

// 呼叫应答
BOOL CSipUA::ring(void)
{
    BOOL                b_ring = FALSE;
	char				p_strerr[PATH_MAX] = {0};
    int                 result;
    char                containing[1024];

    try
    {
        if ((m_pcontext_eXosip == NULL) || m_status != SIP_STATUS_CALLING)
            throw 10;
        eXosip_lock(m_pcontext_eXosip);
        result = eXosip_call_send_answer(m_pcontext_eXosip, m_trans_id, 180, NULL);
        if (result != 0)
        {
            sprintf(p_strerr, "[CSipUA::ring(call_send_answer)](%d) : %s\n", result, osip_strerror(result));
            throw 11;
        }

		if (m_panswer != NULL)
		{
			osip_message_free(m_panswer);
			m_panswer = NULL;
		}
        result = eXosip_call_build_answer(m_pcontext_eXosip, m_trans_id, 200, &m_panswer);
        if (result != 0)
        {
			eXosip_call_send_answer(m_pcontext_eXosip, m_trans_id, 400, NULL);
            sprintf(p_strerr, "[CSipUA::ring(call_send_answer)](%d) : %s\n", result, osip_strerror(result));
            throw 12;
        }
		
		memset(containing, 0, 1024);
        snprintf(containing, 1024,
                "v=0\r\n"
                "o=jack 0 0 IN IP4 %s\r\n"
                "s=talk\r\n"
                "c=IN IP4 %s\r\n"
                "t=0 0\r\n", 
				m_local_ip, m_local_ip);
		result = strlen(containing);
		m_audio_stream.get_media_containing(&containing[result], 1024 - result, m_rtp_audio_port, m_payload_type_audio, m_pmine_type_audio, m_audio_sample_rate);
		if (m_enable_rtp_video)
		{
			result = strlen(containing);
			m_video_stream.get_media_containing(&containing[result], 1024 - result, m_rtp_video_port, m_payload_type_video, m_pmine_type_video, m_video_sample_rate);
		}
        osip_message_set_body(m_panswer, containing, strlen(containing));
        osip_message_set_content_type(m_panswer, "application/sdp");
		osip_message_set_allow(m_panswer, "INVITE, ACK, CANCEL, OPTIONS, BYE, REFER, NOTIFY, MESSAGE, SUBSCRIBE, INFO");
        eXosip_unlock(m_pcontext_eXosip);

		m_status = SIP_STATUS_RINGING;
		b_ring = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 12:
        case 11:
			eXosip_unlock(m_pcontext_eXosip);
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::ring()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::ring()] : error catch!\n", m_username);
    }

    return b_ring;
}

// 结束任务
BOOL CSipUA::task_end(void)
{
    BOOL                b_hang = FALSE;
	char				p_strerr[PATH_MAX] = {0};
    int                 result;

    try
    {
        if ((m_pcontext_eXosip == NULL) || m_status == SIP_STATUS_NULL)	//  || !m_registered
            throw 10;
        eXosip_lock(m_pcontext_eXosip);
        result = eXosip_call_terminate(m_pcontext_eXosip, m_call_id, m_dialog_id);
        eXosip_unlock(m_pcontext_eXosip);
        if (result != 0)
        {
			if (result != OSIP_NOTFOUND && result != OSIP_BADPARAMETER)
				sprintf(p_strerr, "[CSipUA::task_end(call_terminate)](%d) : %s\n", result, osip_strerror(result));
            throw 11;
        }

        b_hang = TRUE;
    }
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "[%s]%s", m_username, p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::task_end()] : %s", m_username, e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[%s][CSipUA::task_end()] : error catch!\n", m_username);
    }
	m_audio_stream.rtp_unconnect();
	m_video_stream.rtp_unconnect();
    m_status = SIP_STATUS_NULL;
	m_call_id = 0;
	m_task_type = SIP_TASK_TYPE_NULL;
	m_remote_ip[0] = '\0';
	m_remote_aport = m_remote_vport = 0;

    return b_hang;
}

// 设置音频文件(单个文件)
BOOL CSipUA::set_audio_file(const char *p_file)
{
	BOOL			b_result = FALSE;

	m_audio_stream.m_audio_file.stop();
	m_audio_stream.m_audio_file.remove_all_file();
	m_audio_stream.m_audio_file.reset_play();
	b_result = m_audio_stream.m_audio_file.add_file(p_file);

	return b_result;
}

// 线程回调函数(处理sip事件)
BOOL CSipUA::running_sip_event(CSipUA *&p_sip_ua)
{
	if (p_sip_ua != NULL)
	{
		p_sip_ua->process_sip_event(p_sip_ua->m_psip_event_callback, p_sip_ua->m_pevent_callback_param);
		if (ua_sem_trywait(&p_sip_ua->m_sem_ua_event_callback))
		{
			if (p_sip_ua->m_pua_event_callback != NULL)
				p_sip_ua->m_pua_event_callback(p_sip_ua, p_sip_ua->m_pua_event_callback_param);
			if (p_sip_ua->m_psem_ua_event != NULL)
				ua_sem_post(p_sip_ua->m_psem_ua_event);
		}
	}

	return TRUE;
}

// 线程: SIP UA的内部运转
void *CSipUA::running_thread(void *arg)
{
	BOOL				b_exit = FALSE;
	char				p_strerr[PATH_MAX] = {0};
	CSipUA				*p_ua = NULL;

	while (!b_exit)
	{
		ua_usleep(1000);
		ua_mutex_lock(&m_mutex_ua);
		if (!m_list_ua.IsEmpty())
			m_list_ua.TraverseHead(running_sip_event);
		else
			b_exit = TRUE;
		ua_mutex_unlock(&m_mutex_ua);
	}
	ua_mutex_destroy(&m_mutex_ua);
	m_init_static_ua = FALSE;

    printf_log(LOG_IS_DEBUG, "[CSipUA::running_thread()] : exit\n");
    ua_thread_exit();
	ua_thread_destroy(&m_thread_ua);
    
    return NULL;
}

// 修改终端音量
BOOL CSipUA::send_msg_volume(const char *p_user_id, const char *p_talk_input, const char *p_talk_output, \
						const char *p_bc_input, const char *p_bc_output)
{
	BOOL			b_send = FALSE;
	char			to_callee[SIP_UA_USERNAME_LEN  * 5];
	char			from_caller[SIP_UA_USERNAME_LEN + SIP_UA_USERNAME_LEN];
	osip_message_t	*p_msg = NULL;
	char			p_cmd_data[PATH_MAX] = {0};

	if (m_pcontext_eXosip != NULL && p_user_id != NULL \
		&& p_talk_input != NULL && p_talk_output != NULL \
		&& p_bc_input != NULL && p_bc_output != NULL)
	{
		sprintf(to_callee, "<sip:%s@%s:%d>", p_user_id, m_reg_ip, m_reg_port);
		sprintf(from_caller, "<sip:%s@%s:%d>", m_username, m_reg_ip, m_reg_port);
		sprintf(p_cmd_data, "[COMMAND];cmd_type=volume;talk_input_volume=%s;talk_output_volume=%s;broadcast_input_volume=%s;broadcast_output_volume=%s", \
			p_talk_input, p_talk_output, p_bc_input, p_bc_output);
		eXosip_message_build_request(m_pcontext_eXosip, &p_msg, "MESSAGE", to_callee, from_caller, NULL);
		osip_message_set_body(p_msg, p_cmd_data, strlen(p_cmd_data));
		eXosip_message_send_request(m_pcontext_eXosip, p_msg);
		b_send = TRUE;
	}

	return b_send;
}

// 查询或修改终端报警输出
BOOL CSipUA::send_msg_alarm_out(const char *p_user_id, const char *p_operation, const char *p_alarm_no, char *p_alarm_status)
{
	BOOL			b_send = FALSE;
	char			to_callee[SIP_UA_USERNAME_LEN  * 5];
	char			from_caller[SIP_UA_USERNAME_LEN + SIP_UA_USERNAME_LEN];
	osip_message_t	*p_msg = NULL;
	char			p_cmd_data[PATH_MAX] = {0};

	if (m_pcontext_eXosip != NULL && p_user_id != NULL \
		&& p_operation != NULL && p_alarm_no != NULL \
		&& p_alarm_status != NULL)
	{
		sprintf(to_callee, "<sip:%s@%s:%d>", p_user_id, m_reg_ip, m_reg_port);
		sprintf(from_caller, "<sip:%s@%s:%d>", m_username, m_reg_ip, m_reg_port);
		sprintf(p_cmd_data, "[COMMAND];cmd_type=alarm_out;operation=%s;alarm_no=%s;alarm_status=%s", \
			p_operation, p_alarm_no, p_alarm_status);
		eXosip_message_build_request(m_pcontext_eXosip, &p_msg, "MESSAGE", to_callee, from_caller, NULL);
		osip_message_set_body(p_msg, p_cmd_data, strlen(p_cmd_data));
		eXosip_message_send_request(m_pcontext_eXosip, p_msg);
		b_send = TRUE;
	}

	return b_send;
}

// 返回或报告终端报警输入
BOOL CSipUA::send_msg_alarm_in(const char *p_user_id, const char *p_operation, const char *p_alarm_no, char *p_alarm_status)
{
	BOOL			b_send = FALSE;
	char			to_callee[SIP_UA_USERNAME_LEN  * 5];
	char			from_caller[SIP_UA_USERNAME_LEN + SIP_UA_USERNAME_LEN];
	osip_message_t	*p_msg = NULL;
	char			p_cmd_data[PATH_MAX] = {0};

	if (m_pcontext_eXosip != NULL && p_user_id != NULL \
		&& p_operation != NULL && p_alarm_no != NULL \
		&& p_alarm_status != NULL)
	{
		sprintf(to_callee, "<sip:%s@%s:%d>", p_user_id, m_reg_ip, m_reg_port);
		sprintf(from_caller, "<sip:%s@%s:%d>", m_username, m_reg_ip, m_reg_port);
		sprintf(p_cmd_data, "[COMMAND];cmd_type=alarm_in;operation=%s;alarm_no=%s;alarm_status=%s", \
			p_operation, p_alarm_no, p_alarm_status);
		eXosip_message_build_request(m_pcontext_eXosip, &p_msg, "MESSAGE", to_callee, from_caller, NULL);
		osip_message_set_body(p_msg, p_cmd_data, strlen(p_cmd_data));
		eXosip_message_send_request(m_pcontext_eXosip, p_msg);
		b_send = TRUE;
	}

	return b_send;
}

// 连接rtp音频
BOOL CSipUA::rtp_connect_video(void)
{
	video_rtp_connect_param_t		param;

	memset(&param, 0, sizeof (param));
	param.p_local_ip = m_local_ip;
	if (m_task_type == SIP_TASK_TYPE_BC_INCOMING && ua_is_multicast(m_remote_ip))
		param.local_port = m_remote_vport;
	else
		param.local_port = m_rtp_video_port;
	param.p_remote_ip = m_remote_ip;
	param.remote_port = m_remote_vport;
	param.payload_index = m_payload_type_video;
	param.p_mime_type = m_pmine_type_video;
	param.sample_rate = m_video_sample_rate;
	if (!(m_task_type == SIP_TASK_TYPE_BC_INCOMING && ua_is_multicast(m_remote_ip)))
		param.enable_rtp_send = TRUE;
	else
		param.enable_rtp_send = FALSE;
	param.enable_rtp_recv = TRUE;
	if (m_task_type == SIP_TASK_TYPE_BC_INCOMING \
			|| m_task_type == SIP_TASK_TYPE_MONITOR_OUTGOING)
		param.enable_memory_fill_send = TRUE;
	else
		param.enable_memory_fill_send = TRUE;
	param.p_user_name = m_username;
	param.p_callee_name = m_callee_username;
	
	return m_video_stream.rtp_connect(&param);
}

// 连接rtp音频
BOOL CSipUA::rtp_connect_audio(void)
{
	rtp_connect_param_t		param;

	memset(&param, 0, sizeof (param));
	param.p_local_ip = m_local_ip;
	if (m_task_type == SIP_TASK_TYPE_BC_INCOMING && ua_is_multicast(m_remote_ip))
		param.local_port = m_remote_aport;
	else
		param.local_port = m_rtp_audio_port;
	param.p_remote_ip = m_remote_ip;
	param.remote_port = m_remote_aport;
	param.payload_index = m_payload_type_audio;
	param.p_mime_type = m_pmine_type_audio;
	param.sample_rate = m_audio_sample_rate;
	//if ((m_task_type == SIP_TASK_TYPE_MONITOR_OUTGOING && m_status == SIP_STATUS_MONITORING) \
	//	|| (m_task_type == SIP_TASK_TYPE_BC_INCOMING && m_status == SIP_STATUS_BCING))
	if (m_task_type == SIP_TASK_TYPE_BC_INCOMING && ua_is_multicast(m_remote_ip))
		param.enable_rtp_send = FALSE;
	else
		param.enable_rtp_send = TRUE;
	//if (m_task_type != SIP_TASK_TYPE_MONITOR_INCOMING && m_prtp_session_audio != NULL)
	param.enable_rtp_recv = TRUE;
	if (m_task_type == SIP_TASK_TYPE_BC_INCOMING \
			|| m_task_type == SIP_TASK_TYPE_MONITOR_OUTGOING)
		param.enable_memory_fill_send = TRUE;
	else
		param.enable_memory_fill_send = FALSE;
	if (m_task_type == SIP_TASK_TYPE_TALK_OUTGOING || m_task_type == SIP_TASK_TYPE_TALK_INCOMING)
		param.use_aec = TRUE;
	else
		param.use_aec = FALSE;
	param.mseconds_per_packet = MSECOND_PER_PACKET;
	if (m_task_type == SIP_TASK_TYPE_BC_INCOMING || m_task_type == SIP_TASK_TYPE_BC_OUTGOING)
		param.b_rtp_buf_max = TRUE;
	else
	{
		param.b_rtp_buf_max = FALSE;
		param.rtp_buf_ms = MIN_MS_PER_FRAME;
	}
	param.p_user_name = m_username;
	param.p_callee_name = m_callee_username;
	
	return m_audio_stream.rtp_connect(&param);
}

// 音频文件播放事件
void CSipUA::play_event_callback(int event_type, DWORD param1, DWORD param2, void *p_param)
{
	CSipUA			*p_ua = (CSipUA *)p_param;
	char			*p_subclass = NULL;
	char			*p_name1 = NULL, *p_name2 = NULL;
	char			p_value1[20], p_value2[20];
	custom_event_t	*p_event = NULL;

	if (p_ua != NULL)
	{
		switch (event_type)
		{
		case PLAY_EVENT_TYPE_OPEN_FILE:
			p_subclass = string_dup(UA_SUBCLASS_OPEN_FILE);
			p_name1 = string_dup(UA_NAME_FILE_INDEX);
			p_name2 = string_dup(UA_NAME_TOTAL_LEN);
			break;
		case PLAY_EVENT_TYPE_PLAY_PROGRESS:
			p_subclass = string_dup(UA_SUBCLASS_PLAY_PROGRESS);
			p_name1 = string_dup(UA_NAME_PLAY_POS);
			p_name2 = string_dup(UA_NAME_TOTAL_LEN);
			break;
		default:
			break;
		}
		if (p_subclass != NULL && p_name1 != NULL)
		{
			sprintf(p_value1, "%d", param1);
			sprintf(p_value2, "%d", param2);
			p_event = p_ua->m_event_list_ua.create_event(p_subclass);
			if (p_event != NULL)
			{
				p_ua->m_event_list_ua.event_add_data(p_event, p_name1, p_value1);
				if (p_name2 != NULL)
					p_ua->m_event_list_ua.event_add_data(p_event, p_name2, p_value2);
				if (p_ua->m_event_list_ua.add_event(p_event))
					ua_sem_post(&p_ua->m_sem_ua_event_callback);

				p_ua->m_event_list_ua.destroy_event(p_event);
				ua_free(p_event);
				p_event = NULL;
			}
		}
		if (p_subclass != NULL)
			ua_free(p_subclass);
		if (p_name1 != NULL)
			ua_free(p_name1);
		if (p_name2 != NULL)
			ua_free(p_name2);
	}
}

// 获取广播是否为组播
BOOL CSipUA::get_bc_is_multicast(char *p_remote_ip, int *p_remote_port)
{
	if (p_remote_ip != NULL)
		strcpy(p_remote_ip, m_remote_ip);
	if (p_remote_port != NULL)
		*p_remote_port = m_remote_aport;
	return m_bc_is_multicast;
}

// 设置音频接收数据回调处理(主循环实时性不好)
void CSipUA::set_audio_recvdata_callback(void(*p_callback)(void *), void *p_param)
{
	m_audio_stream.set_recvdata_callback(p_callback, p_param);
}

// 设置视频接收数据回调处理
void CSipUA::set_video_recvdata_callback(void(*p_callback)(void *), void *p_param)
{
	m_video_stream.set_recvdata_callback(p_callback, p_param);
}

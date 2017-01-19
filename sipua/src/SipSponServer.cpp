/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : SipSponServer.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : Sip和Spon协议管理服务程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef WIN32
#include <sys/msg.h>
#include <arpa/inet.h>
#endif

#include "SipSponServer.h"
#include "log.h"
#include "defines.h"

CSipSponServer::CSipSponServer()
{
	m_term_count_set = 1000;
	ua_sem_init(&m_sem_socket_spon, 0);
	m_thread_is_running = FALSE;
    ua_thread_init(&m_thread);
	m_unlogin_time = 3 * 60 * 1000 / SIP_SERVER_HANDSHAKE_TIME; // 几分钟没有网络数据就认为是掉线
	strcpy(m_server_ip, SIP_SERVER_IP);
	m_server_port = SIP_SERVER_PORT;
}

CSipSponServer::~CSipSponServer()
{
	close();
	ua_sem_destroy(&m_sem_socket_spon);
}

void CSipSponServer::close()
{
	CTerminal			*p_term = NULL;
	BYTE				data[8];
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)data;
	struct sockaddr_in	addr;
	int					i, count;

	count = m_list_term.GetCount();
	if (count > 0)
	{
		for (i = 0; i < count; i++)
		{
			if (m_list_term.GetElem(i + 1, p_term))
				p_term->m_sip_ua.unregister_server();
		}
	}
	if (m_thread_is_running)
	{
        m_thread_is_running = FALSE;
		ua_sem_post(&m_sem_socket_spon);
		ua_thread_join(&m_thread);
		ua_thread_destroy(&m_thread);
	}
	while (m_list_term.RemoveTail(p_term))
		delete p_term;

	memset(data, 0, sizeof (data));
	p_msg->msg = NET_MSG_LOGON_STATE;
	memset(&addr, 0, sizeof (sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(2046);
	addr.sin_addr.s_addr = 0xFE0000EA;
	m_socket_spon.send_data(&addr, data, 8);
}

void CSipSponServer::set_server_addr(const char *p_ip, WORD port)
{
    if (p_ip != NULL)
    {
        strncpy(m_server_ip, p_ip, INET6_ADDRSTRLEN);
        m_server_ip[INET6_ADDRSTRLEN - 1] = '\0';
    }
    m_server_port = port;
}

BOOL CSipSponServer::open(void)
{
	BOOL				b_init = FALSE;
	char				p_strerr[128] = {0};
	BYTE				data[8];
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)data;
	struct sockaddr_in	addr;

	close();

	try
	{
		m_socket_spon.set_hostaddr(m_socket_spon.get_first_hostaddr(), SPON_SERVER_PORT);
		m_socket_spon.open(&m_sem_socket_spon);
		memset(data, 0, sizeof (data));
		p_msg->msg = NET_MSG_LOGON_STATE;
		memset(&addr, 0, sizeof (sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(2046);
		addr.sin_addr.s_addr = 0xFE0000EA;
		m_socket_spon.send_data(&addr, data, 8);

		// 创建线程
		if (!ua_thread_create(&m_thread, CSipSponServer::running_thread, this))
		{
#ifdef WIN32
			SetThreadPriority(m_thread.h, THREAD_PRIORITY_HIGHEST);
#endif
			m_thread_is_running = FALSE;
			sprintf(p_strerr, "[CSipSponServer::open(ua_thread_create)](%d) : %s\n", errno, strerror(errno));
			throw 10;
		}
		else
			m_thread_is_running = TRUE;
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
			printf_log(LOG_IS_ERR, "%s", p_strerr);
	}
	catch (std::exception &e)
    {   
		printf_log(LOG_IS_ERR, "[CSipSponServer::open()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CSipSponServer::open()] : error catch!\n");
    }

	return b_init;
}

void *CSipSponServer::running_thread(void *arg)
{
	CSipSponServer		*p_server = (CSipSponServer *)arg;
	BOOL				b_exit = FALSE;
    SOCKET_DATA			socket_data;
	char				p_strerr[PATH_MAX] = {0};
	struct timeval		ts;
	int					i;
	CTerminal			*p_term = NULL;
	BOOL				handshake_check = FALSE;

	try
	{
        if (p_server == NULL)
        {
            sprintf(p_strerr, "[CSipSponServer::running_thread()] : arg is NULL\n");
            throw 10;
        }
		ua_get_time(&ts);
		while (p_server->m_thread_is_running)
		{
			// 网络消息处理
			if (ua_sem_trywait(&p_server->m_sem_socket_spon))
			{
				while (p_server->m_socket_spon.get_recv_data(&socket_data))
					p_server->process_spon_net_msg(&socket_data);
			}
			// SIP事件处理
			if (ua_get_timeout(&ts) >= SIP_SERVER_HANDSHAKE_TIME)
			{
				ua_get_time(&ts);
				handshake_check = TRUE;
			}
			else
				handshake_check = FALSE;
			for (i = 0; i < p_server->m_list_term.GetCount(); i++)
			{
				if (p_server->m_list_term.GetElem(i + 1, p_term))
				{
					p_term->running();
					if (handshake_check)
					{
						p_term->m_handshake_timeout++;
						if (p_term->m_handshake_timeout >= p_server->m_unlogin_time)
						{
							if (p_server->m_list_term.Remove(i + 1, p_term))
								delete p_term;
						}
					}
				}
				p_term = NULL;
			}

			ua_usleep(500);
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
			printf_log(LOG_IS_ERR, "%s", p_strerr);
	}
	catch (std::exception &e)
    {   
		printf_log(LOG_IS_ERR, "[CSipSponServer::running_thread()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CSipSponServer::running_thread()]:  error catch!\n");
    }

    printf_log(LOG_IS_DEBUG, "[CSipSponServer::running_thread()] : exit\n");
    ua_thread_exit();
    
    return NULL;
}

void CSipSponServer::process_spon_net_msg(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	int					id = p_msg->id + 1;
	CTerminal			*p_term = NULL;

	p_term = find_id(id);

	if (p_msg->msg != NET_MSG_DATA_TALK_DATA && p_msg->msg != NET_MSG_DATA_WAVE_LIVE && p_msg->msg != NET_MSG_HAND_SHAKE)
		printf_log(LOG_IS_DEBUG, "[%d]%X:%d-%d-%d-%d-%d\n", id, p_msg->msg, p_msg->param1, p_msg->param2, p_msg->param3, p_msg->param4, p_msg->param5);
	if (p_msg->msg == NET_MSG_LOGON_REQUEST)
		net_msg_login_request(p_socket_data);
	else if (p_term != NULL)
	{
		switch (p_msg->msg)
		{
		case NET_MSG_HAND_SHAKE:			p_term->net_msg_handshake(p_socket_data); break;
		case NET_MSG_IP_REQUEST:			p_term->net_msg_ip_request(p_socket_data); break;
		case NET_MSG_TALK_REQUEST:			p_term->net_msg_talk_request(p_socket_data); break;
		case NET_MSG_TALK_STATUS:			p_term->net_msg_talk_status(p_socket_data); break;
		case NET_MSG_DATA_TALK_DATA:		p_term->net_msg_talk_data(p_socket_data); break;
		case NET_MSG_BROADCAST_REQUEST:		p_term->net_msg_broadcast_request(p_socket_data); break;
		case NET_MSG_DATA_WAVE_LIVE:		p_term->net_msg_wave_live(p_socket_data); break;
		case NET_MSG_CONTROL_DOOR:			p_term->net_msg_control_door(p_socket_data); break;
		case NET_MSG_SHORTOUTPUT_CTRL:		p_term->net_msg_control_shortoutput(p_socket_data); break;
		case NET_MSG_STATE_REPORT:			p_term->net_msg_state_report(p_socket_data); break;
		default:
			break;
		}
	}
}

void CSipSponServer::net_msg_login_request(SOCKET_DATA *p_socket_data)
{
	NET_MSG_HDR			*p_msg = (NET_MSG_HDR *)p_socket_data->p_buf;
	int					id = p_msg->id + 1;
	CTerminal			*p_term = NULL;

	p_term = find_id(id);
	if (p_term == NULL)
	{
		p_term = create_new_terminal(id, &(p_socket_data->addr_from));
		if (p_term != NULL)
		{
			m_list_term.AddTail(p_term);
		}
	}
	if (p_term != NULL)
		p_term->net_msg_login_request(p_socket_data);
}

CTerminal *CSipSponServer::find_id(int id)
{
	CTerminal			*p_term = NULL;
	int					count = m_list_term.GetCount();
	int					i;

	for (i = 0; i < count; i++)
	{
		if (m_list_term.GetElem(i + 1, p_term) && p_term->m_id == id)
			break;
	}
	if (i >= count)
		p_term = NULL;

	return p_term;
}

CTerminal *CSipSponServer::create_new_terminal(int id, struct sockaddr_in *p_addr)
{
	CTerminal			*p_term = NULL;
	char				p_username[16];
	char				p_addr_buf[INET6_ADDRSTRLEN];

	p_term = new CTerminal;
	if (p_term != NULL)
	{
		p_term->m_id = p_term->m_talk_id = id;
		p_term->m_psocket_spon = &m_socket_spon;
		if (p_addr != NULL)
			p_term->m_socket_addr = *p_addr;
		p_term->m_sip_ua.set_local_addr(m_socket_spon.get_first_hostaddr(), SIP_LOCAL_PORT + id);
#ifdef WIN32
		strcpy(p_addr_buf, inet_ntoa(p_term->m_socket_addr.sin_addr));
#else
		inet_ntop(AF_INET, &(p_term->m_socket_addr.sin_addr), p_addr_buf, INET_ADDRSTRLEN);
#endif
		p_term->m_sip_ua.set_contact_addr(p_addr_buf, htons(p_term->m_socket_addr.sin_port));
		p_term->m_sip_ua.set_register_addr(m_server_ip, m_server_port);
		sprintf(p_username, "%d", SIP_SERVER_USERNAME(id));
		p_term->m_sip_ua.set_username_password(p_username, SIP_SERVER_PASSWORD);
		p_term->m_sip_ua.init();
		p_term->m_sip_ua.m_audio_stream.set_audio_src(AUDIO_SRC_MEMORY);
	}

	return p_term;
}

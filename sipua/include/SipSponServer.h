/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : SipSponServer.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : Sip和Spon协议管理服务程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __SIP_SPON_SERVER_H__
#define __SIP_SPON_SERVER_H__

#include "ua_port.h"
#include "types.h"
#include "LIST.H"
#include "Terminal.h"
#include "socket.h"

#define TERM_COUNT_SET			1000
#define MSGID_SOCKET_SPON		'a'
#define SIP_LOCAL_PORT			6000
#define SIP_SERVER_PORT			5060
#define SIP_SERVER_IP			"192.168.186.13"
#define SIP_SERVER_PASSWORD		"1234"
#define SIP_SERVER_HANDSHAKE_TIME	10000			// 终端握手检测(ms)

class CSipSponServer
{
public:
	CSipSponServer();
	virtual ~CSipSponServer();

	BOOL open(void);
	void close(void);
    void set_server_addr(const char *p_ip, WORD port);

private:
	static void *running_thread(void *arg);
	void process_spon_net_msg(SOCKET_DATA *p_socket_data);
	void net_msg_login_request(SOCKET_DATA *p_socket_data);

	CTerminal *find_id(int id);
	CTerminal *create_new_terminal(int id, struct sockaddr_in *p_addr);

private:

	int					m_term_count_set;
	List<CTerminal *>	m_list_term;
	CSocketEx			m_socket_spon;
	ua_sem_t			m_sem_socket_spon;
	BOOL				m_thread_is_running;
	ua_thread_t			m_thread;      // 后台处理线程
	int					m_unlogin_time;
    char				m_server_ip[INET6_ADDRSTRLEN];
	WORD				m_server_port;
};

#endif // __SIP_SPON_SERVER_H__

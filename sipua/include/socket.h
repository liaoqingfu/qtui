/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : socket.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/03/31
* Description  : 3091�Խ�Ӧ���������
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/

#ifndef __SOCKET_h__
#define __SOCKET_h__

#include "types.h"
#include "SqQueue.h"

#ifdef WIN32
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#endif

#define SEND_BUFFER_SIZE    1480
#define RECV_BUFFER_SIZE    1480
typedef struct tagSOCKET_DATA
{
	struct sockaddr_in	addr_from;
	DWORD				data_len;
	BYTE				p_buf[RECV_BUFFER_SIZE];
} SOCKET_DATA, *PSOCKET_DATA;

#define IP_ADDR_COUNT		3
#ifndef WIN32
#define INVALID_SOCKET		-1
#endif

#define MSG_SOCKET_RECV     100

class CSocketEx
{
public:
    CSocketEx();
    ~CSocketEx();

    void close(void);
    BOOL open(ua_sem_t *psem);
    BOOL set_hostaddr(const char *addr, WORD port);
    void get_hostaddr(const char *addr, WORD *p_port);
    char *get_first_hostaddr(void);
    char *get_next_hostaddr(void);
    static void *thread_recv(void *arg);
    BOOL get_recv_data(SOCKET_DATA *p_data);
	BOOL send_data(struct sockaddr_in *p_addr, BYTE *p_data, WORD size);

private:

public:
	struct sockaddr_in		m_hostaddr;
    int						m_socket;           // �����׽���

private:
    SqQueue<SOCKET_DATA>    m_queue;			// ����������ݵĻ���
    ua_mutex_t				m_mutex_socket;		// ����������ݻ���Ļ�����
    char					m_addr_list[IP_ADDR_COUNT + 1][INET6_ADDRSTRLEN];    // ����IP�б�
    int						m_get_hostaddr;     // ��ȡ����IPʱ�����
    int						m_count_hostaddr;   // ����IP�ĸ���
    int						m_selete_addr;      // ѡ�������IP
	WORD					m_port;             // ����˿�
	ua_sem_t				*m_psem;            // ���������ź���
	ua_thread_t				m_thread;			// �������ݵ��߳�
	SOCKET_DATA				m_socket_data;		// ���յ���������
};

#endif // __SOCKET_h__
 

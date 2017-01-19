/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : socket.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/03/31
* Description  : 3091对讲应用网络程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/

#include "ua_port.h"
#ifdef WIN32
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/msg.h>
#endif

#include "socket.h"
#include "defines.h"
#include "log.h"

CSocketEx::CSocketEx()
{
	m_get_hostaddr = m_count_hostaddr = m_selete_addr = 0;
	memset(m_addr_list, 0, sizeof (m_addr_list));
	m_socket = INVALID_SOCKET;
	m_port = 0;
	m_psem = NULL;
    ua_thread_init(&m_thread);
    ua_mutex_init(&m_mutex_socket);

#ifdef WIN32
	WORD		version = MAKEWORD(2, 0);
	int			result;
	WSADATA		wsa_data;
	result = ::WSAStartup(version, &wsa_data);
	if ((result != 0) || (version != wsa_data.wVersion))
		printf_log(LOG_IS_ERR, "socket version must be greater than or equal to 0x%x", version);
#endif

#ifdef WIN32
	char	lpName[128];
	gethostname(lpName, sizeof(lpName));		
	HOSTENT *pHostent = gethostbyname(lpName);
	if (pHostent != NULL)
	{
		m_count_hostaddr = 0;
		while (m_count_hostaddr < IP_ADDR_COUNT && pHostent->h_addr_list[m_count_hostaddr] != 0)
		{
			strcpy(m_addr_list[m_count_hostaddr], inet_ntoa(*(in_addr *)pHostent->h_addr_list[m_count_hostaddr]));
			m_count_hostaddr++;
		}
	}

#else
	struct ifaddrs 	*p_if_addrs = NULL;
	void			*p_temp_addrs = NULL;
	char			p_addr_buf[INET6_ADDRSTRLEN];

	getifaddrs(&p_if_addrs);
	while (p_if_addrs != NULL)
	{
		if (p_if_addrs->ifa_addr->sa_family == AF_INET)	// IP V4
		{
			p_temp_addrs = &((struct sockaddr_in *)p_if_addrs->ifa_addr)->sin_addr;
			inet_ntop(AF_INET, p_temp_addrs, p_addr_buf, INET_ADDRSTRLEN);
			if (m_count_hostaddr < IP_ADDR_COUNT \
				&& strncmp(p_if_addrs->ifa_name, "eth", 3) == 0)
			{
				strcpy(m_addr_list[m_count_hostaddr], p_addr_buf);
				m_count_hostaddr++;
			}
		}
		else if (p_if_addrs->ifa_addr->sa_family == AF_INET6)	// IP V6
		{
			p_temp_addrs = &((struct sockaddr_in *)p_if_addrs->ifa_addr)->sin_addr;
			inet_ntop(AF_INET6, p_temp_addrs, p_addr_buf, INET6_ADDRSTRLEN);
			if (m_count_hostaddr < IP_ADDR_COUNT)
			{
				strcpy(m_addr_list[m_count_hostaddr], p_addr_buf);
				m_count_hostaddr++;
			}
		}
		p_if_addrs = p_if_addrs->ifa_next;
	}
#endif
}

CSocketEx::~CSocketEx()
{
	close();
	ua_mutex_destroy(&m_mutex_socket);
}

void CSocketEx::close(void)
{
	if (m_socket != INVALID_SOCKET)
	{
		m_psem = NULL;
		ua_thread_join(&m_thread);
		ua_thread_destroy(&m_thread);
        ua_closesokcet(m_socket);
        m_socket = INVALID_SOCKET;
	}
#ifdef WIN32
	::WSACleanup();
#endif
}

void *CSocketEx::thread_recv(void *arg)
{
    CSocketEx         *p_csocket = (CSocketEx *)arg;
	char	        p_strerr[PATH_MAX] = {0};
    socklen_t       addrlen = sizeof (struct sockaddr_in);
    struct timeval  timeout;
    fd_set          fd_socket;
    int             recv_len;

	try
	{
        if (p_csocket == NULL)
        {
            sprintf(p_strerr, "[CSocketEx::thread_recv()] : arg is NULL\n");
            throw 10;
        }
        while (p_csocket->m_psem != NULL)
        {
            timeout.tv_sec = 0;
            timeout.tv_usec = 10000;
            FD_ZERO(&fd_socket);
            FD_SET(p_csocket->m_socket, &fd_socket);
            switch (select(p_csocket->m_socket + 1, &fd_socket, NULL, NULL, &timeout))
            {
            case -1:    // 错误,退出程序
                sprintf(p_strerr, "[CSocketEx::thread_recv(select)](%d) : %s\n", errno, strerror(errno));
                throw 11;
                break;
            case 0:     // 再次轮询
                break;
            default:
                if (FD_ISSET(p_csocket->m_socket, &fd_socket))
                {
                    recv_len = recvfrom(p_csocket->m_socket, (char *)(p_csocket->m_socket_data.p_buf), \
                                RECV_BUFFER_SIZE, 0, \
                                (sockaddr *)&p_csocket->m_socket_data.addr_from, &addrlen);
                    if (recv_len > 0)
                    {
                        ua_mutex_lock(&(p_csocket->m_mutex_socket));
						p_csocket->m_socket_data.data_len = recv_len;
                        p_csocket->m_queue.InsertElem(p_csocket->m_socket_data);
                        ua_mutex_unlock(&(p_csocket->m_mutex_socket));
						ua_sem_post(p_csocket->m_psem);
                    }
                }
            }
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
		printf_log(LOG_IS_ERR, "[CSocketEx::thread_recv()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CSocketEx::thread_recv()] : error catch!\n");
    }

    printf_log(LOG_IS_DEBUG, "[CSocketEx::thread_recv()] : exit\n");
    ua_thread_exit();
    
    return NULL;
}


BOOL CSocketEx::open(ua_sem_t *psem)
{
	BOOL	b_open = FALSE;
	char	p_strerr[PATH_MAX] = {0};

	close();
	m_psem = psem;

	try
	{
	    // 创建socket
		m_socket = socket(AF_INET, SOCK_DGRAM, 0);
		if (m_socket == INVALID_SOCKET)
		{
			sprintf(p_strerr, "[CSocketEx::open(socket)](%d) : %s\n", errno, strerror(errno));
			throw 10;
		}
        // 端口重用
        BOOL    b_reuseaddr = TRUE;
        if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&b_reuseaddr, \
            sizeof(BOOL)) == -1)
        {
			sprintf(p_strerr, "[CSocketEx::open(setsockopt)](%d) : %s\n", errno, strerror(errno));
			throw 11;
        }
        // 设置非阻塞模式
#ifndef WIN32
        if (fcntl(m_socket, F_SETFL, O_NONBLOCK) == -1)
        {
			sprintf(p_strerr, "[CSocketEx::open(fcntl)](%d) : %s\n", errno, strerror(errno));
			throw 12;
        }
#endif
        // 绑定IP和port
		m_hostaddr.sin_family = AF_INET;
#ifdef WIN32
		m_hostaddr.sin_addr.s_addr = inet_addr(m_addr_list[m_selete_addr]);
#else
		inet_pton(AF_INET, m_addr_list[m_selete_addr], &m_hostaddr.sin_addr);
#endif
		m_hostaddr.sin_port = htons(m_port);
		memset(&(m_hostaddr.sin_zero), 0, sizeof (m_hostaddr.sin_zero));
		if (bind(m_socket, (struct sockaddr *)&m_hostaddr, sizeof (m_hostaddr)) == -1)
		{
			sprintf(p_strerr, "[CSocketEx::open(bind)](%d) : %s\n", errno, strerror(errno));
			throw 13;
		}
        // 创建线程
        if (!ua_thread_create(&m_thread, CSocketEx::thread_recv, this))
        {
			sprintf(p_strerr, "[CSocketEx::open(pthread_create)](%d) : %s\n", errno, strerror(errno));
			throw 14;
        }
        
		b_open = TRUE;
	}
	catch (int throw_err)
	{
		switch (throw_err)
		{
		case 14:
		case 13:
		case 12:
        case 11:
            ua_closesokcet(m_socket);
            m_socket = INVALID_SOCKET;
        case 10:
		default:
			break;
		}
		if (strlen(p_strerr) > 0)
    		printf_log(LOG_IS_ERR, "%s", p_strerr);
	}
	catch (std::exception &e)
    {   
		printf_log(LOG_IS_ERR, "[CSocketEx::open()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CSocketEx::open()] : error catch!\n");
    }

	return b_open;
}

BOOL CSocketEx::set_hostaddr(const char *addr, WORD port)
{
	BOOL	b_find = FALSE;
	int		i;

	if (addr != NULL)
	{
		for (i = 0; i < m_count_hostaddr; i++)
		{
			if (strcmp(addr, m_addr_list[i]) == 0)
			{
				m_selete_addr = i;
				m_port = port;
				b_find = TRUE;
				break;
			}
		}
	}

	return b_find;
}

char *CSocketEx::get_next_hostaddr(void)
{
	char	*p_addr = NULL;
	
	if (m_get_hostaddr < m_count_hostaddr)
	{
		p_addr = m_addr_list[m_get_hostaddr];
		m_get_hostaddr += 1;
	}

	return p_addr;
}

char *CSocketEx::get_first_hostaddr(void)
{
	char	*p_addr = NULL;

	m_get_hostaddr = 0;
	if (m_get_hostaddr < m_count_hostaddr)
	{
		p_addr = m_addr_list[m_get_hostaddr];
		m_get_hostaddr += 1;
	}

	return p_addr;
}


BOOL CSocketEx::get_recv_data(SOCKET_DATA *p_data)
{
    BOOL    has_data;
    
    ua_mutex_lock(&m_mutex_socket);
    has_data = m_queue.DeleteElem(*p_data);
    ua_mutex_unlock(&m_mutex_socket);
    return has_data;
}

BOOL CSocketEx::send_data(struct sockaddr_in *p_addr, BYTE *p_data, WORD size)
{
	BOOL		b_send = FALSE;

	if (p_addr != NULL && p_data != NULL && m_socket != INVALID_SOCKET)
	{
		if (sendto(m_socket, (char *)p_data, size, 0, (struct sockaddr *)p_addr, sizeof (struct sockaddr_in)) >= 0)
			b_send = TRUE;
		else
			printf_log(LOG_IS_ERR, "[CSocketEx::send_data(sendto)](%d) : %s\n", errno, strerror(errno));
	}

	return b_send;
}
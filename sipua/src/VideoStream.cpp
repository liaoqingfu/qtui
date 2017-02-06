/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : VideoStream.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/10/31
* Description  : 视频类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#include "ua_port.h"
#include "VideoStream.h"
#include "log.h"
#include "defines.h"

#include <time.h>

#ifndef WIN32
#include <sys/stat.h>
#endif

#define TIMER_SIVAL_VIDEO			3000

//------------- payload type begin -----------------------

char g_mime_type_h264[] = "H264";

// payload_type_h264

//------------- payload type end -----------------------

// 静态变量

List<CVideoStream *> CVideoStream::m_list_vs;
ua_mutex_t		CVideoStream::m_mutex_vs;
timer_data_t	CVideoStream::m_tdata_video;
BOOL			CVideoStream::m_init_static_vs = FALSE;
BOOL			CVideoStream::m_timer_rtp_data = FALSE;


CVideoStream::CVideoStream()
	: m_h264_rtp_unpack(PAYLOAD_TYPE_H264), m_h264_rtp_pack(0x12345678)
{
	char	p_strerr[PATH_MAX] = {0};

	m_video_src = VIDEO_SRC_MEMORY;
	m_video_format = VIDEO_FORMAT_H264;

	// portvideo
	m_enable_video_card = FALSE;
	m_resolution_ratio = 30;
	m_video_input_len = 0;
	m_use_video_card = FALSE;

	// 编解码

	// rtp
	m_rtp_is_connect = FALSE;
	m_prtp_session_video = NULL;
	m_prtp_event_video = NULL;
	m_rtp_read_vrx = m_rtp_write_vrx = m_rtp_read_vtx = m_rtp_write_vtx = 0;
	memset(m_rtp_buf_vrx_len, 0, sizeof (m_rtp_buf_vrx_len));
	memset(m_rtp_buf_vtx_len, 0, sizeof (m_rtp_buf_vtx_len));
	m_rtp_video_ts = 0;
	m_prtp_profile = NULL;
	ortp_init();
	m_enable_rtp_recv = m_enable_rtp_send = m_enable_memory_fill_send = TRUE;
	ua_mutex_init(&m_mutex_unconnect);
	ua_mutex_init(&m_mutex_rtp_buf);
	m_precvdata_callback = NULL;
	m_precvdata_callback_param = NULL;

	m_record = FALSE;
	m_file_vrx = NULL;
	m_file_vtx = NULL;

	initNetwork();

	try
	{
		if (!m_init_static_vs)
		{
			ua_mutex_init(&m_mutex_vs);
			m_tdata_video.p_param = &m_list_vs;
			m_tdata_video.timerid = NULL;
			m_tdata_video.sival = TIMER_SIVAL_VIDEO;
			m_init_static_vs = TRUE;
		}
		ua_mutex_lock(&m_mutex_vs);
		m_list_vs.Init();
		m_list_vs.AddTail(this);
		ua_mutex_unlock(&m_mutex_vs);
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
		printf_log(LOG_IS_ERR, "[CVideoStream::CVideoStream()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CVideoStream::CVideoStream()] : error catch!\n");
    }
}

CVideoStream::~CVideoStream()
{
	int					pos;
	CVideoStream		*p_vs = this;

	m_enable_rtp_recv = m_enable_rtp_send = m_enable_memory_fill_send = FALSE;
	rtp_unconnect();
	ua_mutex_destroy(&m_mutex_unconnect);
	ua_mutex_destroy(&m_mutex_rtp_buf);
	ua_mutex_lock(&m_mutex_vs);
	pos = m_list_vs.Find(p_vs);
	if (pos > 0)
		m_list_vs.Remove(pos, p_vs);
	if (m_list_vs.GetCount() == 0)
	{
		if (m_tdata_video.timerid != NULL)
			ua_delete_timer_event(&m_tdata_video, TRUE);
		m_init_static_vs = FALSE;
		ua_mutex_unlock(&m_mutex_vs);
		ua_mutex_destroy(&m_mutex_vs);
	}
	else
		ua_mutex_unlock(&m_mutex_vs);
	if (m_prtp_profile != NULL)
	{
		rtp_profile_destroy(m_prtp_profile);
		m_prtp_profile = NULL;
	}
	ortp_exit();
	close_record_file();
}

// 初始化
BOOL CVideoStream::init(void)
{
    BOOL                b_result = FALSE;
    //int                 result;
	char	            p_strerr[PATH_MAX] = {0};

    try
    {
		if (m_enable_video_card)
		{
		}
		b_result = TRUE;
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
        printf_log(LOG_IS_ERR, "[CVideoStream::init()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[CVideoStream::init()] : error catch!\n");
    }

    return b_result;
}

// 关闭视频
void CVideoStream::close_video_card(void)
{
	m_use_video_card = FALSE;
}

// 打开视频
BOOL CVideoStream::open_video_card(int resolution_ratio)
{
	char					p_strerr[PATH_MAX] = {0};
	BOOL					b_open = FALSE;

	try
	{
		//return FALSE;	// 模拟没有视频
		if (resolution_ratio < 10 || resolution_ratio > 30)
		{
			sprintf(p_strerr, "[CVideoStream::open_video_card()] : unknown resolution ratio %d\n", resolution_ratio);
			throw 10;
		}
		if (!m_enable_video_card)
		{
			sprintf(p_strerr, "[CVideoStream::open_video_card()] : sound card is disable\n");
			throw 11;
		}
		// 输入设备
		close_video_card();
		m_use_video_card = TRUE;
		b_open = TRUE;
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
		printf_log(LOG_IS_ERR, "[CVideoStream::CVideoStream()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CVideoStream::CVideoStream()] : error catch!\n");
    }

	return b_open;
}

// 视频回调函数
int CVideoStream::video_callback(void)
{
	return 0;
}

// 添加rtp待传送数据
void CVideoStream::write_rtp_senddata(const BYTE *p_send, int len)
{
	ua_mutex_lock(&m_mutex_rtp_buf);
	if (len > VIDEO_RTP_FRAME_LEN_MAX)
		printf_log(LOG_IS_WARNING, "[CVideoStream::write_rtp_senddata()] : The input data is too long!(%d)\n", len);
	else
	{
		memcpy(m_rtp_buf_vtx[m_rtp_write_vtx], p_send, len);
		m_rtp_buf_vtx_len[m_rtp_write_vtx] = len;
		m_rtp_write_vtx++;
		if (m_rtp_write_vtx >= VIDEO_RTP_FRAME_COUNT)
			m_rtp_write_vtx = 0;
	}
	ua_mutex_unlock(&m_mutex_rtp_buf);
	//if (m_file_vtx != NULL)
	//	fwrite(p_send, 1, len, m_file_vtx);
}

// 获取rtp已接收数据
int CVideoStream::read_rtp_recvdata(BYTE *p_recv, int len)
{
	int			read = 0;

	if (m_rtp_read_vrx != m_rtp_write_vrx)
	{
		if (len < m_rtp_buf_vrx_len[m_rtp_read_vrx])
			printf_log(LOG_IS_WARNING, "[CVideoStream::write_rtp_senddata()] : The output buffer is too short!(%d)\n", len);
		else
		{
			ua_mutex_lock(&m_mutex_rtp_buf);
			memcpy(p_recv, m_rtp_buf_vrx[m_rtp_read_vrx], m_rtp_buf_vrx_len[m_rtp_read_vrx]);
			read = m_rtp_buf_vrx_len[m_rtp_read_vrx];
			m_rtp_read_vrx++;
			if (m_rtp_read_vrx >= VIDEO_RTP_FRAME_COUNT)
				m_rtp_read_vrx = 0;
			ua_mutex_unlock(&m_mutex_rtp_buf);
			if (m_file_vrx != NULL)
				fwrite(p_recv, 1, read, m_file_vrx);
		}
	}

	return read;
}

// 处理rtp数据
void CVideoStream::process_rtp_data(void)
{
	int			data_len;
	BYTE		*p_codec_buf = NULL;
	BYTE		*p_h264_buf = NULL, *p_frame_buf = NULL;
	int			h264_len = 0;
	unsigned short frame_len = 0;
	DWORD		codec_buf_len = VIDEO_RTP_FRAME_LEN_MAX;
	mblk_t		*p_mblk = NULL;

	if (m_rtp_is_connect)
	{
		p_codec_buf = (BYTE *)ua_malloc(codec_buf_len);

		if (m_video_input_len > 0)
		{
			if (m_video_src == VIDEO_SRC_VIDEOCARD)
				write_rtp_senddata(m_video_input_buf, m_video_input_len);
			else if (m_video_src == VIDEO_SRC_FILE)
			{
			}
			else if (m_video_src == VIDEO_SRC_MEMORY)
			{
				if (m_enable_rtp_send && m_enable_memory_fill_send)
				{	// freeswitch的rtp中,要想接收数据,必须要发送数据
					memset(p_codec_buf, 0, VIDEO_RTP_FRAME_LEN_MIN);
					write_rtp_senddata(p_codec_buf, VIDEO_RTP_FRAME_LEN_MIN);
				}
			}
			m_video_input_len = 0;
		}

		// 接收
		if (m_enable_rtp_recv)
		{
			data_len = 0;
			if (m_prtp_session_video != NULL)
			{
				p_mblk = rtp_session_recvm_with_ts(m_prtp_session_video, m_rtp_video_ts);
				while (p_mblk != NULL)
				{
					//2017-1-17  
					//mblk_t	*p_mblk_temp = copymsg(p_mblk);
					//rtp_session_sendm_with_ts(m_prtp_session_video, p_mblk_temp, m_rtp_video_ts);
					m_enable_memory_fill_send = FALSE;

					//if (p_mblk->b_wptr - p_mblk->b_rptr > 1472)
					//{
					//	char p_buf[32];
					//	sprintf(p_buf, "\nlen : %d", p_mblk->b_wptr - p_mblk->b_rptr);
					//	OutputDebugString(p_buf);
					//}
					p_h264_buf = m_h264_rtp_unpack.Parse_RTP_Packet(p_mblk->b_rptr, p_mblk->b_wptr - p_mblk->b_rptr, &data_len);
					if (p_h264_buf != NULL)
					{
						if (data_len > VIDEO_RTP_FRAME_LEN_MAX)
							printf_log(LOG_IS_ERR, "[CVideoStream::process_rtp_data()] : The receive buffer is too short!(%d)\n", data_len);
						else
						{
							ua_mutex_lock(&m_mutex_rtp_buf);
							memcpy(m_rtp_buf_vrx[m_rtp_write_vrx], p_h264_buf, data_len);
							m_rtp_buf_vrx_len[m_rtp_write_vrx] = data_len;
							m_rtp_write_vrx++;
							if (m_rtp_write_vrx >= VIDEO_RTP_FRAME_COUNT)
								m_rtp_write_vrx = 0;
							ua_mutex_unlock(&m_mutex_rtp_buf);

							if (m_precvdata_callback != NULL)
								m_precvdata_callback(m_precvdata_callback_param);
						}
						p_h264_buf = NULL;
					}
					freemsg(p_mblk);
					p_mblk = NULL;
					p_mblk = rtp_session_recvm_with_ts(m_prtp_session_video, m_rtp_video_ts);
				}
			}
		}
		// 发送
		if (m_enable_rtp_send)
		{
			while (m_rtp_read_vtx != m_rtp_write_vtx)
			{
				p_h264_buf = &p_codec_buf[10];
				ua_mutex_lock(&m_mutex_rtp_buf);
				data_len = min((int)codec_buf_len - 10, m_rtp_buf_vtx_len[m_rtp_read_vtx]);
				memcpy(p_h264_buf, m_rtp_buf_vtx[m_rtp_read_vtx], data_len);
				m_rtp_read_vtx++;
				if (m_rtp_read_vtx >= VIDEO_RTP_FRAME_COUNT)
					m_rtp_read_vtx = 0;
				ua_mutex_unlock(&m_mutex_rtp_buf);
				if (m_prtp_session_video != NULL)
				{
					if (m_enable_memory_fill_send)
						rtp_session_send_with_ts(m_prtp_session_video, p_h264_buf, data_len, m_rtp_video_ts);
					else
					{
						while (data_len > 4)
						{
							// 查找下一个H264开始标志
							for (h264_len = 5; h264_len < data_len; h264_len++)
							{
								if (p_h264_buf[h264_len - 1] == 0x01 && p_h264_buf[h264_len - 2] == 0x00)
								{
									if (p_h264_buf[h264_len - 3] == 0x00)
									{
										if (p_h264_buf[h264_len - 4] == 0x00)
											h264_len -= 4;
										else
											h264_len -= 3;
										break;
									}
								}
							}
							if (m_h264_rtp_pack.Set(p_h264_buf, h264_len, m_rtp_video_ts, TRUE))
							{
								if (m_file_vtx != NULL)
									fwrite(p_h264_buf, 1, h264_len, m_file_vtx);
								do
								{
									p_frame_buf = m_h264_rtp_pack.Get((unsigned short *)&frame_len);
									if (p_frame_buf != NULL && frame_len > 12)
									{
										p_mblk = rtp_session_create_packet(m_prtp_session_video, RTP_FIXED_HEADER_SIZE, p_frame_buf + 12, frame_len - 12);
										if (p_frame_buf[1] & 0x80)
											rtp_set_markbit(p_mblk, 1);
										rtp_session_sendm_with_ts(m_prtp_session_video, p_mblk, m_rtp_video_ts);
									}
								}while (p_frame_buf != NULL);
							}
							p_h264_buf += h264_len;
							data_len -= h264_len;
						}
					}
				}
				p_h264_buf = NULL;
			}
		}

		m_rtp_video_ts += VIDEO_SAMPLE_RATE / m_resolution_ratio;
		if (p_codec_buf != NULL)
		{
			ua_free(p_codec_buf);
			p_codec_buf = NULL;
		}
	}
}

// 定时器处理
#ifdef WIN32
void CALLBACK CVideoStream::on_timer(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    timer_data		*p_tdata = (timer_data *)dwUser;
    CVideoStream	*p_video_stream = (CVideoStream *)p_tdata->p_param;

	//static int		tick = 0;
	//char			str_buf[64];

    switch (p_tdata->sival)
    {
    case TIMER_SIVAL_VIDEO:
		ua_mutex_lock(&m_mutex_vs);
		//tick++;
		//if (tick >= 50)
		//{
		//	tick = 0;
		//	sprintf(str_buf, "\non_timer : %d", GetTickCount());
		//	OutputDebugString(str_buf);
		//}
		m_timer_rtp_data = FALSE;
		if (!m_list_vs.IsEmpty())
			m_list_vs.TraverseHead(running_rtp_data);
		if (!m_timer_rtp_data)
		{
			if (m_tdata_video.timerid != NULL)
				ua_delete_timer_event(&m_tdata_video, FALSE);
		}
		ua_mutex_unlock(&m_mutex_vs);
        break;
    default:
        break;
    }
}
#else
void CVideoStream::on_timer(union sigval sig_val)
{
    timer_data		*p_tdata = (timer_data *)sig_val.sival_ptr;
    CVideoStream	*p_video_stream = (CVideoStream *)p_tdata->p_param;

    switch (p_tdata->sival)
    {
    case TIMER_SIVAL_VIDEO:
		ua_mutex_lock(&m_mutex_vs);
		m_timer_rtp_data = FALSE;
		if (!m_list_vs.IsEmpty())
			m_list_vs.TraverseHead(running_rtp_data);
		if (!m_timer_rtp_data)
		{
			if (m_tdata_video.timerid != NULL)
				ua_delete_timer(&m_tdata_video, FALSE);
		}
		ua_mutex_unlock(&m_mutex_vs);
        break;
    default:
        break;
    }
}
#endif


// 线程回调函数(处理rtp视频数据)
BOOL CVideoStream::running_rtp_data(CVideoStream *&p_video_stream)
{
	if (p_video_stream != NULL && p_video_stream->m_rtp_is_connect && !p_video_stream->m_use_video_card)
	{
		memset(p_video_stream->m_video_input_buf, 0, sizeof (p_video_stream->m_video_input_buf));
		p_video_stream->m_video_input_len = VIDEO_RTP_FRAME_LEN_MIN;
		m_timer_rtp_data = TRUE;
		p_video_stream->process_rtp_data();
	}
	return TRUE;
}

// 进行一个rtp连接
BOOL CVideoStream::rtp_connect(video_rtp_connect_param_t *p_param)
{
	BOOL			b_find_profile = FALSE;
	PayloadType		*p_payload_type = NULL;
	char			p_strerr[PATH_MAX] = {0};

	try
	{
		rtp_unconnect();
		if (p_param == NULL || p_param->p_local_ip == NULL || p_param->p_remote_ip == NULL || p_param->p_mime_type == NULL)
		{
			sprintf(p_strerr, "[CVideoStream::rtp_connect()] : parameter invalid\n");
			throw 10;
		}
		if (strcasecmp(p_param->p_mime_type, g_mime_type_h264) == 0)
		{
			b_find_profile = TRUE;
			p_payload_type = &payload_type_h264;
			m_video_format = VIDEO_FORMAT_H264;
		}
		if (!b_find_profile)
		{
			sprintf(p_strerr, "[CVideoStream::rtp_connect()] : video format(%s) is not supported\n", p_param->p_mime_type);
			throw 11;
		}
		m_prtp_profile = rtp_profile_clone_full(&av_profile);
		m_prtp_session_video = rtp_session_new(RTP_SESSION_SENDRECV);
		m_prtp_event_video = ortp_ev_queue_new();
		if (m_prtp_session_video == NULL || m_prtp_event_video == NULL || m_prtp_profile == NULL)
		{
			sprintf(p_strerr, "[CVideoStream::rtp_connect()] : rtp initialize failed\n");
			throw 12;
		}
		// 设置负载类型
		if (p_payload_type != NULL)
			rtp_profile_set_payload(m_prtp_profile, p_param->payload_index, p_payload_type);
		rtp_session_set_profile(m_prtp_session_video, m_prtp_profile);
		rtp_session_set_payload_type(m_prtp_session_video, p_param->payload_index);
		rtp_session_register_event_queue(m_prtp_session_video, m_prtp_event_video);
		// 设置远程RTP客户端的的IP和监听端口(即本rtp数据包的发送目的地址)
		rtp_session_set_remote_addr(m_prtp_session_video, p_param->p_remote_ip, p_param->remote_port);
		rtp_session_set_local_addr(m_prtp_session_video, p_param->p_local_ip, p_param->local_port, p_param->local_port + 10000);

		if (ua_is_multicast(p_param->p_remote_ip))
		{
			int						err;
#ifdef WIN32
			// windows上要使用组播,必须使用WSASocket,使用后面WSA_FLAG_MULTIPOINT的标志
			struct sockaddr_in		addr;
			int						flag = 1;
			unsigned long			nonBlock = 1;

			close_socket(m_prtp_session_video->rtp.socket);
			m_prtp_session_video->rtp.socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, (LPWSAPROTOCOL_INFO)NULL, 0, \
														WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF);
			if (m_prtp_session_video->rtp.socket == INVALID_SOCKET)
			{
				sprintf(p_strerr, "[CVideoStream::rtp_connect(WSASocket)](%d) : Fail.\n", getSocketErrorCode());
				throw 13;
			}
			err = setsockopt(m_prtp_session_video->rtp.socket, SOL_SOCKET, SO_REUSEADDR, (CHAR *)&flag, sizeof(flag));
			if (err == SOCKET_ERROR)
			{
				sprintf(p_strerr, "[CVideoStream::rtp_connect(setsockopt)](%d) : Fail.\n", getSocketErrorCode());
				throw 14;
			}
			addr.sin_family = PF_INET;
			addr.sin_addr.s_addr = inet_addr(p_param->p_local_ip);
			addr.sin_port = htons(p_param->local_port);
			memset(&(addr.sin_zero), 0, sizeof (addr.sin_zero));
			err = bind(m_prtp_session_video->rtp.socket, (struct sockaddr FAR *)&addr, sizeof(struct sockaddr)); 
			if (err == SOCKET_ERROR)
			{
				sprintf(p_strerr, "[CVideoStream::rtp_connect(bind)](%d) : Fail.\n", getSocketErrorCode());
				throw 15;
			}
			ioctlsocket(m_prtp_session_video->rtp.socket, FIONBIO , &nonBlock);

			addr.sin_family = PF_INET;
			addr.sin_addr.s_addr = inet_addr(p_param->p_remote_ip);
			addr.sin_port = htons(p_param->remote_port);
			memset(&(addr.sin_zero), 0, sizeof (addr.sin_zero));
			if (WSAJoinLeaf(m_prtp_session_video->rtp.socket, (PSOCKADDR)&addr, sizeof (SOCKADDR), \
				NULL, NULL, NULL, NULL, JL_BOTH) == INVALID_SOCKET)
			{
				sprintf(p_strerr, "[CVideoStream::rtp_connect(WSAJoinLeaf)](%d) : Fail.\n", getSocketErrorCode());
				throw 16;
			}
#else
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = inet_addr(p_param->p_remote_ip);
			mreq.imr_interface.s_addr = INADDR_ANY;
			err = setsockopt(m_prtp_session_video->rtp.socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (SOCKET_OPTION_VALUE) &mreq, sizeof(mreq));
			if (err < 0)
			{
				sprintf(p_strerr, "[CVideoStream::rtp_connect(setsockopt)](%d) : Fail to join address group.\n", getSocketErrorCode());
				throw 13;
			}
#endif
			rtp_session_set_multicast_loopback(m_prtp_session_video, TRUE);	// 必须要设置, 否则不能收到本机发送的数据
			rtp_session_set_multicast_ttl(m_prtp_session_video, 64);
		}

		m_rtp_video_ts = 0;
		m_rtp_read_vrx = m_rtp_write_vrx = m_rtp_read_vtx = m_rtp_write_vtx = 0;
		m_enable_rtp_send = p_param->enable_rtp_send;
		m_enable_rtp_recv = p_param->enable_rtp_recv;
		m_enable_memory_fill_send = p_param->enable_memory_fill_send;

		if (m_record)
			create_record_file(p_param->p_user_name, p_param->p_callee_name);

		if (m_enable_video_card)
			open_video_card(m_resolution_ratio);
		if (!m_use_video_card)
		{
			if (m_tdata_video.timerid == NULL)
				ua_create_timer_event(&m_tdata_video, on_timer, 1000 / m_resolution_ratio, 1000 / m_resolution_ratio, TIMER_SIVAL_VIDEO);
		}

		m_rtp_is_connect = TRUE;
		printf_log(LOG_IS_INFO, "[CVideoStream::rtp_connect()] 2017-01-17 : %s:%d - %s:%d\n", p_param->p_local_ip, p_param->local_port, p_param->p_remote_ip, p_param->remote_port);
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
		printf_log(LOG_IS_ERR, "[CVideoStream::rtp_connect()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CVideoStream::rtp_connect()] : error catch!\n");
    }

	return m_rtp_is_connect;
}

// RTP断开连接
void CVideoStream::rtp_unconnect(void)
{
	m_rtp_is_connect = FALSE;
	ua_mutex_lock(&m_mutex_unconnect);
	close_video_card();
	if (m_prtp_session_video != NULL && m_prtp_event_video != NULL)
		rtp_session_unregister_event_queue(m_prtp_session_video, m_prtp_event_video);
	if (m_prtp_event_video != NULL)
	{
		ortp_ev_queue_destroy(m_prtp_event_video);
		m_prtp_event_video = NULL;
	}
	if (m_prtp_session_video != NULL)
	{
		rtp_session_destroy(m_prtp_session_video);
		m_prtp_session_video = NULL;
	}
	if (m_prtp_profile != NULL)
	{
		rtp_profile_destroy(m_prtp_profile);
		m_prtp_profile = NULL;
	}
	m_video_format = VIDEO_FORMAT_H264;
	close_record_file();
	ua_mutex_unlock(&m_mutex_unconnect);
}

// 获取支持的媒体格式
char *CVideoStream::get_media_containing(char *p_containing, int len, WORD rtp_port, int payload_type, const char *p_mine_type, int sample_rate)
{
	if (p_containing != NULL)
	{
		if (payload_type != 0 && p_mine_type != NULL)
		{
			snprintf(p_containing, len,
					"m=video %d RTP/AVP %d\r\n"
					"a=rtpmap:%d %s/%d\r\n"
					"a=fmtp:%d profile-level-id=428014\r\n",
					rtp_port, payload_type, 
					payload_type, p_mine_type, sample_rate, 
					payload_type);
			p_containing[len - 1] = '\0';
		}
		else
		{
			snprintf(p_containing, len,
					"m=video %d RTP/AVP %d\r\n"
					"a=rtpmap:%d %s/%d\r\n"
					"a=fmtp:%d profile-level-id=428014\r\n",
					rtp_port, PAYLOAD_TYPE_H264, 
					PAYLOAD_TYPE_H264, g_mime_type_h264, VIDEO_SAMPLE_RATE, 
					PAYLOAD_TYPE_H264);
			p_containing[len - 1] = '\0';
		}
	}

	return p_containing;
}

// 关闭录音文件
void CVideoStream::close_record_file(void)
{
	int				i;
	FILE			**pp_file[2] = {&m_file_vrx, &m_file_vtx};
	
	for (i = 0; i < 2; i++)
	{
		if ((*pp_file[i]) != NULL)
		{
			fclose((*pp_file[i]));
			(*pp_file[i]) = NULL;
		}
	}
}

// 创建录音文件
void CVideoStream::create_record_file(const char *p_user_name, const char *p_callee_name)
{
	char			p_strerr[PATH_MAX] = {0};
	char			str_buf[PATH_MAX];
	string			str_path_rx, str_path_tx, str_name;
	string			str_folder[3];
	time_t          now;
	struct tm       *p_tm_local;
	int				i;

	close_record_file();
	if (m_record && p_user_name != NULL && p_callee_name != NULL)
	{
		try
		{
			time(&now);
			p_tm_local = localtime(&now);
			//get_exe_path(str_buf, PATH_MAX);
			sprintf(str_buf, "%s%c", "r:", DIRECTORY_SPLIT_CHAR);
			str_path_rx = str_buf;
			str_path_rx = str_path_rx.substr(0, str_path_rx.find_last_of(DIRECTORY_SPLIT_CHAR) + 1);

			// VIDEO_RECORD_FOLDER_NAME文件夹, 年份文件夹, 月份文件夹创建
			str_folder[0] = VIDEO_RECORD_FOLDER_NAME;
			str_folder[1] = format("%d", p_tm_local->tm_year + 1900);
			str_folder[2] = format("%d", p_tm_local->tm_mon + 1);
			for (i = 0; i < 3; i++)
			{
				str_path_rx += str_folder[i];
#ifdef WIN32
				WIN32_FIND_DATA	hFindFileData;
				HANDLE			hFind = INVALID_HANDLE_VALUE;
				hFind = FindFirstFile(str_path_rx.c_str(), &hFindFileData);
				if (hFind == INVALID_HANDLE_VALUE)
					CreateDirectory(str_path_rx.c_str(), NULL);
				else
					FindClose(hFind);
#else
				int			    find;
				find = access(str_path_rx.c_str(), F_OK);
				if (find == -1)
				{
					if (mkdir(str_path_rx.c_str(), 0764) == -1)
					{
						printf_log(LOG_IS_ERR, "[CVideoStream::create_record_file(mkdir)](%d) : %s\n", errno, strerror(errno));
					}
				}
#endif
				str_path_rx += DIRECTORY_SPLIT_CHAR;
			}

			// 文件名
			sprintf(str_buf, "%d-%d-%d_%d;%d;%d_%s-", p_tm_local->tm_year + 1900, p_tm_local->tm_mon + 1, p_tm_local->tm_mday, \
				p_tm_local->tm_hour, p_tm_local->tm_min, p_tm_local->tm_sec, p_user_name);
			str_path_rx += str_buf;
			str_path_tx = str_path_rx;
			str_path_tx += p_user_name;
			str_path_rx += p_callee_name;
			str_path_tx += ".264";
			str_path_rx += ".264";

			m_file_vtx = fopen(str_path_tx.c_str(), "wb");
			if (m_file_vtx == NULL)
			{
				sprintf(p_strerr, "[CVideoStream::create_record_file(fopen)](%d) : '%s'%s\n", errno, str_path_tx.c_str(), strerror(errno));
				throw 10;
			}
			m_file_vrx = fopen(str_path_rx.c_str(), "wb");
			if (m_file_vrx == NULL)
			{
				sprintf(p_strerr, "[CVideoStream::create_record_file(fopen)](%d) : '%s'%s\n", errno, str_path_rx.c_str(), strerror(errno));
				throw 11;
			}
		}
		catch (int throw_err)
		{
			switch (throw_err)
			{
			case 11:
				fclose(m_file_vtx);
			case 10:
			default:
				break;
			}
			if (strlen(p_strerr) > 0)
				printf_log(LOG_IS_ERR, "%s", p_strerr);
		}
		catch (std::exception &e)
		{   
			printf_log(LOG_IS_ERR, "[CVideoStream::create_record_file()] : %s", e.what());
		}
		catch (...)
		{   
			printf_log(LOG_IS_ERR, "[CVideoStream::create_record_file()] : error catch!\n");
		}
	}
}


// 设置视频接收数据回调处理(主循环实时性不好)
void CVideoStream::set_recvdata_callback(void(*p_callback)(void *), void *p_param)
{
	m_precvdata_callback = p_callback;
	m_precvdata_callback_param = p_param;
}


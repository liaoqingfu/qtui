/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : VideoStream.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/10/31
* Description  : ��Ƶ�����
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __VIDEO_STREAM_H__
#define __VIDEO_STREAM_H__

#include "ortp/ortp.h"
#include "ortp/stun_udp.h"

#include "LIST.H"
#include "H264Pack.h"

#define VIDEO_SRC_MEMORY			0			// �ڴ�(������ͨ�ն�)
#define VIDEO_SRC_VIDEOCARD			1			// ��Ƶ��
#define VIDEO_SRC_FILE				2			// �ļ�

#define VIDEO_FORMAT_H264			0
#define PAYLOAD_TYPE_H264			99

extern char g_mime_type_h264[];

#define VIDEO_SAMPLE_RATE			90000			// ��Ƶ������
#define VIDEO_RTP_FRAME_LEN_MAX		(1024*40)		// ѹ����ÿ֡��С(���)
#define VIDEO_RTP_FRAME_LEN_MIN		512				// ѹ����ÿ֡��С(��С)
#define VIDEO_FRAME_LEN_MAX			(2048*10)		// δѹ��ÿ֡��С
#define VIDEO_RTP_FRAME_COUNT		4

#define VIDEO_RECORD_FOLDER_NAME	"record"

typedef struct video_rtp_connect_param {
	const char		*p_local_ip;
	int				local_port;
	const char		*p_remote_ip;
	int				remote_port;
	int				payload_index;
	const char		*p_mime_type;
	int				sample_rate;
	BOOL			enable_rtp_send;
	BOOL			enable_rtp_recv;
	BOOL			enable_memory_fill_send;	// ѡ��VIDEO_SRC_MEMORYʱ, ��䴫������(freeswitch��rtp��,Ҫ���������,����Ҫ��������)
	const char		*p_user_name;				// �Լ���id
	const char		*p_callee_name;				// �Է���id
} video_rtp_connect_param_t;

class CSipUA;
class CVideoStream
{
public:
	CVideoStream();
	virtual ~CVideoStream();

	friend class CSipUA;

	// ���º����������ⲿ����
	void set_video_src(int video_src);													// ������ƵԴ
	BOOL init(void);																	// ��ʼ��
	void set_enable_video_card(BOOL enable);											// �Ƿ�ʹ����Ƶ��(Ĭ�ϲ�ʹ��)
	void set_enable_record(BOOL enable);												// �Ƿ�¼��(Ĭ�ϲ�¼��)

	// ���º��������ڸ������
	void write_rtp_senddata(const BYTE *p_send, int len);								// ���rtp����������
	int read_rtp_recvdata(BYTE *p_recv, int len);										// ��ȡrtp�ѽ�������
private:
	void close_video_card(void);														// �ر���Ƶ
	BOOL open_video_card(int resolution_ratio);											// ����Ƶ
    static int video_callback(void);													// ��Ƶ�ص�����
	void process_rtp_data(void);														// ����rtp����
	BOOL rtp_connect(video_rtp_connect_param_t *p_param);								// ����һ��rtp����
	void rtp_unconnect(void);															// rtp�Ͽ�����
	char *get_media_containing(char *p_containing, int len, WORD rtp_port, int payload_type, \
							const char *p_mine_type, int sample_rate);					// ��ȡ֧�ֵ�ý���ʽ
	void close_record_file(void);														// �ر�¼���ļ�
	void create_record_file(const char *p_user_name, const char *p_callee_name);		// ����¼���ļ�
	void set_recvdata_callback(void(*p_callback)(void *), void *p_param);				// ������Ƶ�������ݻص�����(��ѭ��ʵʱ�Բ���)

	// ��̬����
#ifdef WIN32
    static void CALLBACK on_timer(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);// ��ʱ������
#else
    static void on_timer(union sigval sig_val);											// ��ʱ������
#endif
	static BOOL running_rtp_data(CVideoStream *&p_sip_ua);								// �̻߳ص�����(����rtp��Ƶ����)

public:
	// file
	int						m_video_src;
	int						m_video_format;

private:

	// ��̬����
	static List<CVideoStream *> m_list_vs;						// ��Ƶ���嵥
	static ua_mutex_t		m_mutex_vs;							// m_list_vs��Ƶ����
	static timer_data_t		m_tdata_video;						// ��Ƶ��ʱ��
	static BOOL				m_init_static_vs;					// ��̬�����Ƿ��ʼ��
	static BOOL				m_timer_rtp_data;					// ��ʱ�����Ƿ�����rtp����

	// video
	BOOL					m_enable_video_card;
	int						m_resolution_ratio;					// ֡��
	BYTE					m_video_input_buf[VIDEO_FRAME_LEN_MAX];
	int						m_video_input_len;
	BOOL					m_use_video_card;

	// rtp
	BOOL					m_rtp_is_connect;
	RtpSession				*m_prtp_session_video;				// RTP�Ự
	ua_mutex_t				m_mutex_unconnect;
	OrtpEvQueue				*m_prtp_event_video;
	ua_mutex_t				m_mutex_rtp_buf;
	BYTE					m_rtp_buf_vrx[VIDEO_RTP_FRAME_COUNT][VIDEO_RTP_FRAME_LEN_MAX];
	int						m_rtp_buf_vrx_len[VIDEO_RTP_FRAME_COUNT];
	int						m_rtp_read_vrx;
	int						m_rtp_write_vrx;
	BYTE					m_rtp_buf_vtx[VIDEO_RTP_FRAME_COUNT][VIDEO_RTP_FRAME_LEN_MAX];
	int						m_rtp_buf_vtx_len[VIDEO_RTP_FRAME_COUNT];
	int						m_rtp_read_vtx;
	int						m_rtp_write_vtx;
	int						m_rtp_video_ts;
	RtpProfile				*m_prtp_profile;
	BOOL					m_enable_rtp_send;
	BOOL					m_enable_rtp_recv;
	BOOL					m_enable_memory_fill_send;
	void					(*m_precvdata_callback)(void *);
	void					*m_precvdata_callback_param;

	BOOL					m_record;							// �Ƿ�¼��
	FILE					*m_file_vrx, *m_file_vtx;			// �����ļ�

	// �����
	CH264_RTP_UNPACK		m_h264_rtp_unpack;
	CH264_RTP_PACK			m_h264_rtp_pack;
};

// ������ƵԴ
inline void CVideoStream::set_video_src(int video_src)
{
	m_video_src = video_src;
}

// �Ƿ�ʹ����Ƶ��(Ĭ�ϲ�ʹ��)
inline void CVideoStream::set_enable_video_card(BOOL enable)
{
	m_enable_video_card = enable;
}

// �Ƿ�¼��(Ĭ�ϲ�¼��)
inline void CVideoStream::set_enable_record(BOOL enable)
{
	m_record = enable;
}

#endif // __VIDEO_STREAM_H__

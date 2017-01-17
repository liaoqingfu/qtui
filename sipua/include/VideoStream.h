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
#ifndef __VIDEO_STREAM_H__
#define __VIDEO_STREAM_H__

#include "ortp/ortp.h"
#include "ortp/stun_udp.h"

#include "LIST.H"
#include "H264Pack.h"

#define VIDEO_SRC_MEMORY			0			// 内存(用于内通终端)
#define VIDEO_SRC_VIDEOCARD			1			// 视频卡
#define VIDEO_SRC_FILE				2			// 文件

#define VIDEO_FORMAT_H264			0
#define PAYLOAD_TYPE_H264			99

extern char g_mime_type_h264[];

#define VIDEO_SAMPLE_RATE			90000			// 视频采样率
#define VIDEO_RTP_FRAME_LEN_MAX		(1024*40)		// 压缩后每帧大小(最大)
#define VIDEO_RTP_FRAME_LEN_MIN		512				// 压缩后每帧大小(最小)
#define VIDEO_FRAME_LEN_MAX			(2048*10)		// 未压缩每帧大小
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
	BOOL			enable_memory_fill_send;	// 选择VIDEO_SRC_MEMORY时, 填充传送数据(freeswitch的rtp中,要想接收数据,必须要发送数据)
	const char		*p_user_name;				// 自己的id
	const char		*p_callee_name;				// 对方的id
} video_rtp_connect_param_t;

class CSipUA;
class CVideoStream
{
public:
	CVideoStream();
	virtual ~CVideoStream();

	friend class CSipUA;

	// 以下函数可用于外部调用
	void set_video_src(int video_src);													// 设置视频源
	BOOL init(void);																	// 初始化
	void set_enable_video_card(BOOL enable);											// 是否使用视频卡(默认不使用)
	void set_enable_record(BOOL enable);												// 是否录像(默认不录像)

	// 以下函数仅限于父类调用
	void write_rtp_senddata(const BYTE *p_send, int len);								// 添加rtp待传送数据
	int read_rtp_recvdata(BYTE *p_recv, int len);										// 获取rtp已接收数据
private:
	void close_video_card(void);														// 关闭视频
	BOOL open_video_card(int resolution_ratio);											// 打开视频
    static int video_callback(void);													// 视频回调函数
	void process_rtp_data(void);														// 处理rtp数据
	BOOL rtp_connect(video_rtp_connect_param_t *p_param);								// 进行一个rtp连接
	void rtp_unconnect(void);															// rtp断开连接
	char *get_media_containing(char *p_containing, int len, WORD rtp_port, int payload_type, \
							const char *p_mine_type, int sample_rate);					// 获取支持的媒体格式
	void close_record_file(void);														// 关闭录像文件
	void create_record_file(const char *p_user_name, const char *p_callee_name);		// 创建录像文件
	void set_recvdata_callback(void(*p_callback)(void *), void *p_param);				// 设置视频接收数据回调处理(主循环实时性不好)

	// 静态函数
#ifdef WIN32
    static void CALLBACK on_timer(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);// 定时器处理
#else
    static void on_timer(union sigval sig_val);											// 定时器处理
#endif
	static BOOL running_rtp_data(CVideoStream *&p_sip_ua);								// 线程回调函数(处理rtp视频数据)

public:
	// file
	int						m_video_src;
	int						m_video_format;

private:

	// 静态变量
	static List<CVideoStream *> m_list_vs;						// 视频流清单
	static ua_mutex_t		m_mutex_vs;							// m_list_vs视频流锁
	static timer_data_t		m_tdata_video;						// 视频定时器
	static BOOL				m_init_static_vs;					// 静态变量是否初始化
	static BOOL				m_timer_rtp_data;					// 定时器里是否处理了rtp数据

	// video
	BOOL					m_enable_video_card;
	int						m_resolution_ratio;					// 帧率
	BYTE					m_video_input_buf[VIDEO_FRAME_LEN_MAX];
	int						m_video_input_len;
	BOOL					m_use_video_card;

	// rtp
	BOOL					m_rtp_is_connect;
	RtpSession				*m_prtp_session_video;				// RTP会话
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

	BOOL					m_record;							// 是否录像
	FILE					*m_file_vrx, *m_file_vtx;			// 像音文件

	// 编解码
	CH264_RTP_UNPACK		m_h264_rtp_unpack;
	CH264_RTP_PACK			m_h264_rtp_pack;
};

// 设置视频源
inline void CVideoStream::set_video_src(int video_src)
{
	m_video_src = video_src;
}

// 是否使用视频卡(默认不使用)
inline void CVideoStream::set_enable_video_card(BOOL enable)
{
	m_enable_video_card = enable;
}

// 是否录像(默认不录像)
inline void CVideoStream::set_enable_record(BOOL enable)
{
	m_record = enable;
}

#endif // __VIDEO_STREAM_H__

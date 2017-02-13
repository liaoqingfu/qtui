/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : AudioStream.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/10/31
* Description  : 音频类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __AUDIO_STREAM_H__
#define __AUDIO_STREAM_H__

#include "ortp/ortp.h"
#include "ortp/stun_udp.h"
#include "portaudio/portaudio.h"
#include "libspeex/config.h"
#include "libspeex/speex_echo.h"
#include "libspeex/speex_preprocess.h"

#include "AudioFile.h"
#include "LIST.H"

#define AUDIO_SRC_MEMORY			0			// 内存(用于内通终端)
#define AUDIO_SRC_SOUNDCARD			1			// 声卡
#define AUDIO_SRC_FILE				2			// 文件

#define AUDIO_FORMAT_PCMU			0
#define AUDIO_FORMAT_PCMA			1
#define AUDIO_FORMAT_L16			2
#define AUDIO_FORMAT_MP3			3

extern char g_mime_type_pcmu[];
extern char g_mime_type_pcma[];
extern char g_mime_type_l16[];
extern char g_mime_type_mp3[];

#define SAMPLE_RATE_MAX				48000								// 最大采样率
#define MSECOND_PER_PACKET			20									// 毫秒/一帧
#define MAX_MS_PER_FRAME			200									// MAX毫秒/一帧
#define MIN_MS_PER_FRAME			140									// MIN毫秒/一帧
#define SAMPLES_PER_FRAME_MAX		(SAMPLE_RATE_MAX * (MAX_MS_PER_FRAME) / 1000)	// 每帧最大采样数
#define RTP_AUDIO_LEN_MAX			(SAMPLES_PER_FRAME_MAX)

#define AEC_FRAME_LEN_8K			128
#define AEC_FRAME_LEN_16K			256
#define AEC_FILTER_LEN_8K			512
#define AEC_FILTER_LEN_16K			1024

#define AUDIO_RECORD_FOLDER_NAME	"record"

typedef struct rtp_connect_param {
	const char		*p_local_ip;
	int				local_port;
	const char		*p_remote_ip;
	int				remote_port;
	int				payload_index;
	const char		*p_mime_type;
	int				sample_rate;
	BOOL			enable_rtp_send;
	BOOL			enable_rtp_recv;
	BOOL			enable_memory_fill_send;	// 选择AUDIO_SRC_MEMORY时, 填充传送数据(freeswitch的rtp中,要想接收数据,必须要发送数据)
	BOOL			use_aec;					// 是否使用回声抵消
	int				mseconds_per_packet;
	BOOL			b_rtp_buf_max;				// 使用最大缓冲
	int				rtp_buf_ms;					// 使用指定时长的缓冲(用于调节延时)
	const char		*p_user_name;				// 自己的id
	const char		*p_callee_name;				// 对方的id
} rtp_connect_param_t;

class CSipUA;
class CAudioStream
{
public:
	CAudioStream();
	virtual ~CAudioStream();

	friend class CSipUA;

	// 以下函数可用于外部调用
	void set_audio_src(int audio_src);													// 设置音频源
	BOOL init(void);																	// 初始化
	void set_enable_sound_card(BOOL enable);											// 是否使用声卡(默认不使用)
	void set_enable_record(BOOL enable);												// 是否录音(默认录音)
	void set_enable_mp3decoder_when_memory(BOOL enable);								// 设置当选择AUDIO_SRC_MEMORY时,是否需要解码
	BOOL get_enable_mp3decoder_when_memory(void);										// 获取当选择AUDIO_SRC_MEMORY时,是否需要解码
	void set_enable_echo_cancel(BOOL enable);											// 是否使用回声抵消(默认不使用)

	// 以下函数仅限于父类调用
	void write_rtp_senddata(const short *p_send, int samples);							// 添加rtp待传送数据(samples:采样个数)
	int read_rtp_recvdata(short *p_recv, int samples);									// 获取rtp已接收数据(recv_len:采样个数),返回采样个数
private:
	void close_soundcard(void);															// 关闭声卡
	BOOL open_soundcard(int sample_rate);												// 打开声卡
    static int audio_callback(const void *input, void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData);											// 音频回调函数
	void process_rtp_data(void);														// 处理rtp数据
	int encoder_data(BYTE *p_dst, short *p_src, int samples);							// 数据编码, samples为采样个数, 返回编码后字节数
	int decoder_data(short *p_dst, int dst_samples, BYTE *p_src, int src_len);			// 数据解码, src_len为字节数, 返回编码后采样个数
	BOOL rtp_connect(rtp_connect_param_t *p_param);										// 进行一个rtp连接
	void rtp_unconnect(void);															// rtp断开连接
	char *get_media_containing(char *p_containing, int len, WORD rtp_port, int payload_type, \
							const char *p_mine_type, int sample_rate);					// 获取支持的媒体格式
	void set_mp3encode_bitrate(int bitrate);											// 设置mp3编码器位流
	void close_record_file(void);														// 关闭录音文件
	void create_record_file(const char *p_user_name, const char *p_callee_name);		// 创建录音文件
	void set_recvdata_callback(void(*p_callback)(void *), void *p_param);				// 设置音频接收数据回调处理(主循环实时性不好)
	void create_echo_canceller(void);													// 创建回声抵消器
	void destroy_echo_canceller(void);													// 销毁回声抵消器

	// 静态函数
#ifdef WIN32
    static void CALLBACK on_timer_event(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);// 定时器处理
    static void CALLBACK on_timer_queue(void *p_param, BOOL timeorwait);				// 定时器处理
#else
    static void on_timer_event(union sigval sig_val);											// 定时器处理
    static void on_timer_queue(union sigval sig_val);											// 定时器处理
#endif
	static BOOL running_rtp_data(CAudioStream *&p_sip_ua);								// 线程回调函数(处理rtp音频数据)

public:
	// file
	CAudioFile				m_audio_file;
	int						m_audio_src;
	int						m_audio_format;

private:

	// 静态变量
	static List<CAudioStream *> m_list_as;						// 音频流清单
	static ua_mutex_t		m_mutex_as;							// m_list_as音频流锁
	static timer_data_t		m_tdata_audio;						// 音频定时器
	static BOOL				m_init_static_as;					// 静态变量是否初始化
	static BOOL				m_timer_rtp_data;					// 定时器里是否处理了rtp数据

	// portaudio
	BOOL					m_enable_sound_card;
	int						m_sample_rate;
	int						m_mseconds_per_packet;
	int						m_samples_per_packet;
	PaStream				*m_ppa_stream;
	short					m_audio_input_buf[SAMPLES_PER_FRAME_MAX];
	int						m_audio_input_len;
	int						m_audio_pa_status;
	BOOL					m_use_sound_card;
	BOOL					m_init_pa;

	// rtp
	BOOL					m_rtp_is_connect;
	RtpSession				*m_prtp_session_audio;				// RTP会话
	ua_mutex_t				m_mutex_unconnect;
	OrtpEvQueue				*m_prtp_event_audio;
	ua_mutex_t				m_mutex_rtp_buf;
	short					m_rtp_buf_arx[RTP_AUDIO_LEN_MAX];
	int						m_rtp_read_arx;
	int						m_rtp_write_arx;
	short					m_rtp_buf_atx[RTP_AUDIO_LEN_MAX];
	int						m_rtp_read_atx;
	int						m_rtp_write_atx;
	int						m_rtp_audio_ts;
	RtpProfile				*m_prtp_profile;
	BOOL					m_enable_rtp_send;
	BOOL					m_enable_rtp_recv;
	BOOL					m_enable_memory_fill_send;
	int						m_rtp_buf_max;
	void					(*m_precvdata_callback)(void *);
	void					*m_precvdata_callback_param;

	BOOL					m_record;							// 是否录音
	FILE					*m_file_arx, *m_file_atx;			// 录音文件

	// 编解码
	mad_decoder_t			m_mp3decoder;						// mp3解码器
	int						m_mp3decode_samplerate;				// mp3解码采样率
	BOOL					m_enable_mp3decoder;				// 当选择AUDIO_SRC_MEMORY时,是否需要解码
	HBE_STREAM				m_mp3encoder;						// mp3编码器
	int						m_mp3encode_bitrate;				// mp3编码位流
	DWORD					m_mp3encode_samples;				// mp3编码的输入采样数量
	DWORD					m_mp3encode_out_size;				// mp3编码的输出缓冲大小
	timer_data_t			m_tdata_mp3decode_samplerate;		// mp3编码采样率改变

	// 回声抵消
	BOOL					m_enable_echo_cancel;				// 是否启用回声抵消(默认不启用)
	BOOL					m_use_aec;							// 是否使用回声抵消(只有对讲时使用)
	int						m_aec_frame_len;					// 回声抵消帧长
	short					*m_paec_ref_buf;					// 参考信号
	int						m_aec_ref_len;
	short					*m_paec_mic_buf;					// 输出信号(回声抵消后的信号)
	int						m_aec_mic_len;
	SpeexEchoState			*m_pecho_st;
	SpeexPreprocessState	*m_ppreprocess_st;
};

// 设置当选择AUDIO_SRC_MEMORY时,是否需要解码
inline void CAudioStream::set_enable_mp3decoder_when_memory(BOOL enable)
{
	m_enable_mp3decoder = enable;
}

// 获取当选择AUDIO_SRC_MEMORY时,是否需要解码
inline BOOL CAudioStream::get_enable_mp3decoder_when_memory(void)
{
	return m_enable_mp3decoder;
}


// 设置音频源
inline void CAudioStream::set_audio_src(int audio_src)
{
	m_audio_src = audio_src;
}

// 是否使用声卡(默认不使用)
inline void CAudioStream::set_enable_sound_card(BOOL enable)
{
	m_enable_sound_card = enable;
}

// 是否录音(默认录音)
inline void CAudioStream::set_enable_record(BOOL enable)
{
	m_record = enable;
}

// 设置mp3编码器位流
inline void CAudioStream::set_mp3encode_bitrate(int bitrate)
{
	m_mp3encode_bitrate = bitrate;
}

// 是否使用回声抵消(默认不使用)
inline void CAudioStream::set_enable_echo_cancel(BOOL enable)
{
	m_enable_echo_cancel = enable;
}

#endif // __AUDIO_STREAM_H__

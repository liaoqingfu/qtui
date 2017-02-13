/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : AudioStream.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/10/31
* Description  : ��Ƶ�����
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

#define AUDIO_SRC_MEMORY			0			// �ڴ�(������ͨ�ն�)
#define AUDIO_SRC_SOUNDCARD			1			// ����
#define AUDIO_SRC_FILE				2			// �ļ�

#define AUDIO_FORMAT_PCMU			0
#define AUDIO_FORMAT_PCMA			1
#define AUDIO_FORMAT_L16			2
#define AUDIO_FORMAT_MP3			3

extern char g_mime_type_pcmu[];
extern char g_mime_type_pcma[];
extern char g_mime_type_l16[];
extern char g_mime_type_mp3[];

#define SAMPLE_RATE_MAX				48000								// ��������
#define MSECOND_PER_PACKET			20									// ����/һ֡
#define MAX_MS_PER_FRAME			200									// MAX����/һ֡
#define MIN_MS_PER_FRAME			140									// MIN����/һ֡
#define SAMPLES_PER_FRAME_MAX		(SAMPLE_RATE_MAX * (MAX_MS_PER_FRAME) / 1000)	// ÿ֡��������
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
	BOOL			enable_memory_fill_send;	// ѡ��AUDIO_SRC_MEMORYʱ, ��䴫������(freeswitch��rtp��,Ҫ���������,����Ҫ��������)
	BOOL			use_aec;					// �Ƿ�ʹ�û�������
	int				mseconds_per_packet;
	BOOL			b_rtp_buf_max;				// ʹ����󻺳�
	int				rtp_buf_ms;					// ʹ��ָ��ʱ���Ļ���(���ڵ�����ʱ)
	const char		*p_user_name;				// �Լ���id
	const char		*p_callee_name;				// �Է���id
} rtp_connect_param_t;

class CSipUA;
class CAudioStream
{
public:
	CAudioStream();
	virtual ~CAudioStream();

	friend class CSipUA;

	// ���º����������ⲿ����
	void set_audio_src(int audio_src);													// ������ƵԴ
	BOOL init(void);																	// ��ʼ��
	void set_enable_sound_card(BOOL enable);											// �Ƿ�ʹ������(Ĭ�ϲ�ʹ��)
	void set_enable_record(BOOL enable);												// �Ƿ�¼��(Ĭ��¼��)
	void set_enable_mp3decoder_when_memory(BOOL enable);								// ���õ�ѡ��AUDIO_SRC_MEMORYʱ,�Ƿ���Ҫ����
	BOOL get_enable_mp3decoder_when_memory(void);										// ��ȡ��ѡ��AUDIO_SRC_MEMORYʱ,�Ƿ���Ҫ����
	void set_enable_echo_cancel(BOOL enable);											// �Ƿ�ʹ�û�������(Ĭ�ϲ�ʹ��)

	// ���º��������ڸ������
	void write_rtp_senddata(const short *p_send, int samples);							// ���rtp����������(samples:��������)
	int read_rtp_recvdata(short *p_recv, int samples);									// ��ȡrtp�ѽ�������(recv_len:��������),���ز�������
private:
	void close_soundcard(void);															// �ر�����
	BOOL open_soundcard(int sample_rate);												// ������
    static int audio_callback(const void *input, void *output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData);											// ��Ƶ�ص�����
	void process_rtp_data(void);														// ����rtp����
	int encoder_data(BYTE *p_dst, short *p_src, int samples);							// ���ݱ���, samplesΪ��������, ���ر�����ֽ���
	int decoder_data(short *p_dst, int dst_samples, BYTE *p_src, int src_len);			// ���ݽ���, src_lenΪ�ֽ���, ���ر�����������
	BOOL rtp_connect(rtp_connect_param_t *p_param);										// ����һ��rtp����
	void rtp_unconnect(void);															// rtp�Ͽ�����
	char *get_media_containing(char *p_containing, int len, WORD rtp_port, int payload_type, \
							const char *p_mine_type, int sample_rate);					// ��ȡ֧�ֵ�ý���ʽ
	void set_mp3encode_bitrate(int bitrate);											// ����mp3������λ��
	void close_record_file(void);														// �ر�¼���ļ�
	void create_record_file(const char *p_user_name, const char *p_callee_name);		// ����¼���ļ�
	void set_recvdata_callback(void(*p_callback)(void *), void *p_param);				// ������Ƶ�������ݻص�����(��ѭ��ʵʱ�Բ���)
	void create_echo_canceller(void);													// ��������������
	void destroy_echo_canceller(void);													// ���ٻ���������

	// ��̬����
#ifdef WIN32
    static void CALLBACK on_timer_event(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);// ��ʱ������
    static void CALLBACK on_timer_queue(void *p_param, BOOL timeorwait);				// ��ʱ������
#else
    static void on_timer_event(union sigval sig_val);											// ��ʱ������
    static void on_timer_queue(union sigval sig_val);											// ��ʱ������
#endif
	static BOOL running_rtp_data(CAudioStream *&p_sip_ua);								// �̻߳ص�����(����rtp��Ƶ����)

public:
	// file
	CAudioFile				m_audio_file;
	int						m_audio_src;
	int						m_audio_format;

private:

	// ��̬����
	static List<CAudioStream *> m_list_as;						// ��Ƶ���嵥
	static ua_mutex_t		m_mutex_as;							// m_list_as��Ƶ����
	static timer_data_t		m_tdata_audio;						// ��Ƶ��ʱ��
	static BOOL				m_init_static_as;					// ��̬�����Ƿ��ʼ��
	static BOOL				m_timer_rtp_data;					// ��ʱ�����Ƿ�����rtp����

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
	RtpSession				*m_prtp_session_audio;				// RTP�Ự
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

	BOOL					m_record;							// �Ƿ�¼��
	FILE					*m_file_arx, *m_file_atx;			// ¼���ļ�

	// �����
	mad_decoder_t			m_mp3decoder;						// mp3������
	int						m_mp3decode_samplerate;				// mp3���������
	BOOL					m_enable_mp3decoder;				// ��ѡ��AUDIO_SRC_MEMORYʱ,�Ƿ���Ҫ����
	HBE_STREAM				m_mp3encoder;						// mp3������
	int						m_mp3encode_bitrate;				// mp3����λ��
	DWORD					m_mp3encode_samples;				// mp3����������������
	DWORD					m_mp3encode_out_size;				// mp3�������������С
	timer_data_t			m_tdata_mp3decode_samplerate;		// mp3��������ʸı�

	// ��������
	BOOL					m_enable_echo_cancel;				// �Ƿ����û�������(Ĭ�ϲ�����)
	BOOL					m_use_aec;							// �Ƿ�ʹ�û�������(ֻ�жԽ�ʱʹ��)
	int						m_aec_frame_len;					// ��������֡��
	short					*m_paec_ref_buf;					// �ο��ź�
	int						m_aec_ref_len;
	short					*m_paec_mic_buf;					// ����ź�(������������ź�)
	int						m_aec_mic_len;
	SpeexEchoState			*m_pecho_st;
	SpeexPreprocessState	*m_ppreprocess_st;
};

// ���õ�ѡ��AUDIO_SRC_MEMORYʱ,�Ƿ���Ҫ����
inline void CAudioStream::set_enable_mp3decoder_when_memory(BOOL enable)
{
	m_enable_mp3decoder = enable;
}

// ��ȡ��ѡ��AUDIO_SRC_MEMORYʱ,�Ƿ���Ҫ����
inline BOOL CAudioStream::get_enable_mp3decoder_when_memory(void)
{
	return m_enable_mp3decoder;
}


// ������ƵԴ
inline void CAudioStream::set_audio_src(int audio_src)
{
	m_audio_src = audio_src;
}

// �Ƿ�ʹ������(Ĭ�ϲ�ʹ��)
inline void CAudioStream::set_enable_sound_card(BOOL enable)
{
	m_enable_sound_card = enable;
}

// �Ƿ�¼��(Ĭ��¼��)
inline void CAudioStream::set_enable_record(BOOL enable)
{
	m_record = enable;
}

// ����mp3������λ��
inline void CAudioStream::set_mp3encode_bitrate(int bitrate)
{
	m_mp3encode_bitrate = bitrate;
}

// �Ƿ�ʹ�û�������(Ĭ�ϲ�ʹ��)
inline void CAudioStream::set_enable_echo_cancel(BOOL enable)
{
	m_enable_echo_cancel = enable;
}

#endif // __AUDIO_STREAM_H__

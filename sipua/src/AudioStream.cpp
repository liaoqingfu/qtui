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
#include "ua_port.h"
#include "AudioStream.h"
#include "log.h"
#include "defines.h"
#include "CodecAlawUlaw.h"

#include <time.h>

#ifndef WIN32
#include <sys/stat.h>
#endif

#define TIMER_SIVAL_AUDIO			2000
#define TIMER_MP3DECODE_SAMPLERATE	2001

//------------- payload type begin -----------------------
char offset127 = 127; 
char offset0xD5 = (char)0xD5; 
char offset0[4] = {0x00, 0x00, 0x00, 0x00};

#define TYPE(val)		(val)
#define CLOCK_RATE(val)		(val)
#define BITS_PER_SAMPLE(val)	(val)
#define ZERO_PATTERN(val)	(val)
#define PATTERN_LENGTH(val)	(val)
#define NORMAL_BITRATE(val)	(val)
#define MIME_TYPE(val)		(val)
#define CHANNELS(val)		(val)
#define FMTP(val)			(val)

char g_mime_type_pcmu[] = "PCMU";
char g_mime_type_pcma[] = "PCMA";
char g_mime_type_l16[] = "L16";
char g_mime_type_mp3[] = "MP3";

#define PAYLOAD_TYPE_L16_8000		104
static PayloadType g_payload_type_l16_8000 = { 
	TYPE( PAYLOAD_AUDIO_CONTINUOUS), 
	CLOCK_RATE(8000), 
	BITS_PER_SAMPLE(16), 
	ZERO_PATTERN( offset0), 
	PATTERN_LENGTH(2), 
	NORMAL_BITRATE(128000), 
	MIME_TYPE (g_mime_type_l16), 
	CHANNELS(1)
}; 

#define PAYLOAD_TYPE_L16_22050		102
static PayloadType g_payload_type_l16_22050 = { 
   TYPE( PAYLOAD_AUDIO_CONTINUOUS), 
   CLOCK_RATE(22050), 
   BITS_PER_SAMPLE(16), 
   ZERO_PATTERN( offset0), 
   PATTERN_LENGTH(2), 
   NORMAL_BITRATE(352800), 
   MIME_TYPE (g_mime_type_l16), 
   CHANNELS(1) 
}; 

#define PAYLOAD_TYPE_MP3			106
static PayloadType g_payload_type_mp3 = { 
   TYPE( PAYLOAD_AUDIO_CONTINUOUS), 
   CLOCK_RATE(8000), 
   BITS_PER_SAMPLE(16), 
   ZERO_PATTERN(NULL), 
   PATTERN_LENGTH(0), 
   NORMAL_BITRATE(128000), 
   MIME_TYPE (g_mime_type_mp3), 
   CHANNELS(2) 
};

typedef struct payload_type_list {
	int				no;
	PayloadType		*p_payload;
} payload_type_list_t;

#define PAYLOAD_PCM_COUNT			2
#define PAYLOAD_MP3					2
#define PAYLOAD_COUNT				3
static payload_type_list_t g_ppayload_type[PAYLOAD_COUNT] = {
	{PAYLOAD_TYPE_L16_8000, &g_payload_type_l16_8000}, 
	{PAYLOAD_TYPE_L16_22050, &g_payload_type_l16_22050}, 
	{PAYLOAD_TYPE_MP3, &g_payload_type_mp3}, 
};

//------------- payload type end -----------------------

// 静态变量
List<CAudioStream *> CAudioStream::m_list_as;
ua_mutex_t		CAudioStream::m_mutex_as;
timer_data_t	CAudioStream::m_tdata_audio;
BOOL			CAudioStream::m_init_static_as = FALSE;
BOOL			CAudioStream::m_timer_rtp_data = FALSE;

CAudioStream::CAudioStream()
{
	char	p_strerr[PATH_MAX] = {0};

	m_audio_src = AUDIO_SRC_FILE;
	m_audio_format = AUDIO_FORMAT_PCMU;

	// portaudio
	m_enable_sound_card = FALSE;
	m_sample_rate = 8000;
	m_mseconds_per_packet = MSECOND_PER_PACKET;
	m_samples_per_packet = m_sample_rate * m_mseconds_per_packet / 1000;
	m_ppa_stream = NULL;
	m_audio_input_len = 0;
	m_audio_pa_status = paComplete;
	m_use_sound_card = FALSE;
	m_init_pa = FALSE;

	// 编解码
	mad_init(&m_mp3decoder);
	m_mp3decode_samplerate = 0;
	m_mp3encoder = 0;
	m_enable_mp3decoder = FALSE;
	m_mp3encode_samples = m_mp3encode_out_size = 0;
	m_mp3encode_bitrate = 128000;
	m_tdata_mp3decode_samplerate.p_param = this;
	m_tdata_mp3decode_samplerate.timerid = NULL;
	m_tdata_mp3decode_samplerate.sival = TIMER_MP3DECODE_SAMPLERATE;

	// rtp
	m_rtp_is_connect = FALSE;
	m_prtp_session_audio = NULL;
	m_prtp_event_audio = NULL;
	m_rtp_read_arx = m_rtp_write_arx = m_rtp_read_atx = m_rtp_write_atx = 0;
	m_rtp_buf_max = RTP_AUDIO_LEN_MAX;
	m_rtp_audio_ts = 0;
	m_prtp_profile = NULL;
	ortp_init();
	m_enable_rtp_recv = m_enable_rtp_send = m_enable_memory_fill_send = TRUE;
	ua_mutex_init(&m_mutex_unconnect);
	ua_mutex_init(&m_mutex_rtp_buf);
	m_precvdata_callback = NULL;
	m_precvdata_callback_param = NULL;

	m_record = FALSE;
	m_file_arx = NULL;
	m_file_atx = NULL;

	initNetwork();

	try
	{
		if (!m_init_static_as)
		{
			ua_mutex_init(&m_mutex_as);
			m_tdata_audio.p_param = &m_list_as;
			m_tdata_audio.timerid = NULL;
			m_tdata_audio.sival = TIMER_SIVAL_AUDIO;
			m_init_static_as = TRUE;
		}
		ua_mutex_lock(&m_mutex_as);
		m_list_as.Init();
		m_list_as.AddTail(this);
		ua_mutex_unlock(&m_mutex_as);
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
		printf_log(LOG_IS_ERR, "[CAudioStream::CAudioStream()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CAudioStream::CAudioStream()] : error catch!\n");
    }
}

CAudioStream::~CAudioStream()
{
	int					pos;
	CAudioStream		*p_as = this;

	m_enable_rtp_recv = m_enable_rtp_send = m_enable_memory_fill_send = FALSE;
	if (m_tdata_mp3decode_samplerate.timerid != NULL)
		ua_delete_timer_queue(&m_tdata_mp3decode_samplerate, TRUE);
	rtp_unconnect();
	ua_mutex_destroy(&m_mutex_unconnect);
	ua_mutex_destroy(&m_mutex_rtp_buf);
	ua_mutex_lock(&m_mutex_as);
	if (m_init_pa)
	{
		Pa_Terminate();
		m_init_pa = FALSE;
	}
	pos = m_list_as.Find(p_as);
	if (pos > 0)
		m_list_as.Remove(pos, p_as);
	if (m_list_as.GetCount() == 0)
	{
		if (m_tdata_audio.timerid != NULL)
			ua_delete_timer_event(&m_tdata_audio, TRUE);
		m_init_static_as = FALSE;
		ua_mutex_unlock(&m_mutex_as);
		ua_mutex_destroy(&m_mutex_as);
	}
	else
		ua_mutex_unlock(&m_mutex_as);
	if (m_prtp_profile != NULL)
	{
		rtp_profile_destroy(m_prtp_profile);
		m_prtp_profile = NULL;
	}
	mad_destroy(&m_mp3decoder);
	if (m_mp3encoder != 0)
	{
		beCloseStream(m_mp3encoder);
		m_mp3encoder = 0;
	}
	ortp_exit();
	close_record_file();
}

// 初始化
BOOL CAudioStream::init(void)
{
    BOOL                b_result = FALSE;
    int                 result;
	char	            p_strerr[PATH_MAX] = {0};

    try
    {
		if (m_enable_sound_card && !m_init_pa)
		{
			result = Pa_Initialize();
			if (result != paNoError)
			{
				sprintf(p_strerr, "[CAudioStream::init(Pa_Initialize)](%d) : %s\n", result, Pa_GetErrorText(result));
				throw 10;
			}
			else
				m_init_pa = TRUE;
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
        printf_log(LOG_IS_ERR, "[CAudioStream::init()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[CAudioStream::init()] : error catch!\n");
    }

    return b_result;
}

// 关闭声卡
void CAudioStream::close_soundcard(void)
{
	PaStream		*ppa_stream_input = m_ppa_stream;
	int				i;

	if (m_ppa_stream != NULL)
	{
		m_ppa_stream = NULL;
		for (i = 0; i < 20; i++)
		{	// 等待回调结束
			if (m_audio_pa_status == paComplete)
				break;
			else
				ua_usleep(1000);
		}
		if (i >= 20)	// 如果未结束, 等待下次调用结束
			m_ppa_stream = ppa_stream_input;
		else
		{
			Pa_StopStream(ppa_stream_input);
			Pa_CloseStream(ppa_stream_input);
		}
	}
	m_use_sound_card = FALSE;
}

// 打开声卡
BOOL CAudioStream::open_soundcard(int sample_rate)
{
	PaStreamParameters		pa_parameters_input, pa_parameters_output;
	char					p_strerr[PATH_MAX] = {0};
	int						result;
	BOOL					b_open = FALSE;

	try
	{
		//return FALSE;	// 模拟没有声卡
		switch (sample_rate)
		{
		case 48000:
		case 44100:
		case 22050:
		case 11025:
		case 32000:
		case 16000:
		case 8000:
			m_sample_rate = m_mp3decode_samplerate = sample_rate;
			m_samples_per_packet = m_sample_rate * m_mseconds_per_packet / 1000;
			break;
		default:
			sprintf(p_strerr, "[CAudioStream::open_soundcard()] : unknown sample rate %d\n", sample_rate);
			throw 10;
			break;
		}
		if (!m_enable_sound_card)
		{
			sprintf(p_strerr, "[CAudioStream::open_soundcard()] : sound card is disable\n");
			throw 11;
		}
		// 输入设备
		close_soundcard();
		pa_parameters_input.device = Pa_GetDefaultInputDevice();
		if (pa_parameters_input.device == paNoDevice)
		{
			sprintf(p_strerr, "[CAudioStream::open_soundcard(Pa_GetDefaultInputDevice)] : No default input device\n");
			throw 12;
		}
		pa_parameters_input.channelCount = 1;
		pa_parameters_input.sampleFormat = paInt16;
		pa_parameters_input.suggestedLatency = Pa_GetDeviceInfo(pa_parameters_input.device)->defaultLowInputLatency;
		pa_parameters_input.hostApiSpecificStreamInfo = NULL;
		// 输出设备
		pa_parameters_output.device = Pa_GetDefaultOutputDevice();
		if (pa_parameters_output.device == paNoDevice)
		{
			sprintf(p_strerr, "[CAudioStream::open_soundcard(Pa_GetDefaultOutputDevice)] : No default output device\n");
			throw 13;
		}
		pa_parameters_output.channelCount = 1;
		pa_parameters_output.sampleFormat = paInt16;
		pa_parameters_output.suggestedLatency = Pa_GetDeviceInfo(pa_parameters_output.device)->defaultLowOutputLatency;
		pa_parameters_output.hostApiSpecificStreamInfo = NULL;

		result = Pa_OpenStream(&m_ppa_stream, 
						&pa_parameters_input,
						&pa_parameters_output,
						m_sample_rate,
						m_samples_per_packet,
						paClipOff,
						audio_callback,
						this);
		if (result != paNoError)
		{
			sprintf(p_strerr, "[CAudioStream::open_soundcard(Pa_OpenStream)](%d) : %s\n", result, Pa_GetErrorText(result));
			throw 14;
		}
		result = Pa_StartStream(m_ppa_stream);
		if (result != paNoError)
		{
			sprintf(p_strerr, "[CAudioStream::open_soundcard(Pa_StartStream)](%d) : %s\n", result, Pa_GetErrorText(result));
			throw 15;
		}
		m_use_sound_card = TRUE;
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
		printf_log(LOG_IS_ERR, "[CAudioStream::CAudioStream()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CAudioStream::CAudioStream()] : error catch!\n");
    }

	return b_open;
}

// 音频回调函数
int CAudioStream::audio_callback(const void *p_input, void *p_output,
                            unsigned long frame_count,
                            const PaStreamCallbackTimeInfo *p_time_info,
                            PaStreamCallbackFlags status_flags,
                            void *p_user_data)
{
	CAudioStream	*p_sip_ua = (CAudioStream *)p_user_data;
	int				pa_status = paComplete;

	if (p_sip_ua != NULL)
	{
		if (p_sip_ua->m_ppa_stream != NULL)
		{
			if (p_input != NULL)
			{
				if (frame_count <= SAMPLES_PER_FRAME_MAX)
					p_sip_ua->m_audio_input_len = frame_count;
				else
					p_sip_ua->m_audio_input_len = SAMPLES_PER_FRAME_MAX;
				memcpy(p_sip_ua->m_audio_input_buf, p_input, p_sip_ua->m_audio_input_len * sizeof (short));
				p_sip_ua->process_rtp_data();
			}
			if (p_output != NULL)
			{
				memset(p_output, 0, frame_count * sizeof (short));
				if (p_sip_ua->m_audio_src == AUDIO_SRC_SOUNDCARD)
					p_sip_ua->read_rtp_recvdata((short *)p_output, frame_count);
			}
			pa_status = paContinue;
		}
	}
	p_sip_ua->m_audio_pa_status = pa_status;

	return pa_status;
}

// 添加rtp待传送数据(send_len:采样个数)
void CAudioStream::write_rtp_senddata(const short *p_send, int samples)
{
	int			i;

	ua_mutex_lock(&m_mutex_rtp_buf);
	for (i = 0; i < samples; i++)
	{
		m_rtp_buf_atx[m_rtp_write_atx++] = p_send[i];
		if (m_rtp_write_atx >= m_rtp_buf_max)
			m_rtp_write_atx = 0;
	}
	ua_mutex_unlock(&m_mutex_rtp_buf);
	//if (m_file_atx != NULL)
	//	fwrite(p_send, 1, samples * sizeof (short), m_file_atx);
}

// 获取rtp已接收数据(recv_len:采样个数),返回采样个数
int CAudioStream::read_rtp_recvdata(short *p_recv, int samples)
{
	int			read = 0;
	int			i, buf_len;

	if (m_rtp_read_arx <= m_rtp_write_arx)
		buf_len = m_rtp_write_arx - m_rtp_read_arx;
	else
		buf_len = m_rtp_buf_max - m_rtp_read_arx + m_rtp_write_arx;
	if (buf_len >= samples)
	{
		ua_mutex_lock(&m_mutex_rtp_buf);
		for (i = 0; i < samples; i++)
		{
			p_recv[i] = m_rtp_buf_arx[m_rtp_read_arx++];//lhg can improve speed  :   memcpy
			if (m_rtp_read_arx >= m_rtp_buf_max)
				m_rtp_read_arx = 0;
		}
		ua_mutex_unlock(&m_mutex_rtp_buf);
		read = samples;
		if (m_file_arx != NULL)
			fwrite(p_recv, 1, samples * sizeof (short), m_file_arx);
	}

	return read;
}

// 处理rtp数据
void CAudioStream::process_rtp_data(void)
{
	int			i, data_len;
	short		*p_sample_buf = NULL;
	BYTE		*p_codec_buf = NULL;
	int			have_move;
	int			frame_len;
	DWORD		sample_buf_len = SAMPLE_RATE_MAX * 16 * 100 / 1000 / 8 / sizeof (short);
	DWORD		codec_buf_len = sample_buf_len;

	if (m_rtp_is_connect)
	{
		p_sample_buf = (short *)ua_malloc(sample_buf_len * sizeof (short));
		p_codec_buf = (BYTE *)ua_malloc(codec_buf_len);

		if (m_audio_input_len > 0)
		{
			if (m_audio_src == AUDIO_SRC_SOUNDCARD)
				write_rtp_senddata(m_audio_input_buf, m_audio_input_len);
			else if (m_audio_src == AUDIO_SRC_FILE)
			{
				if (m_audio_format == AUDIO_FORMAT_MP3)
				{
					data_len = m_audio_file.read_frame_data((BYTE *)p_sample_buf, m_audio_file.m_out_bitrate / 8 / (1000 / m_mseconds_per_packet));
					write_rtp_senddata(p_sample_buf, data_len / sizeof (short));
				}
				else
				{
					data_len = m_audio_file.read_frame_data((BYTE *)p_sample_buf, m_samples_per_packet * sizeof (short));
					write_rtp_senddata(p_sample_buf, data_len / sizeof (short));
				}
			}
			else if (m_audio_src == AUDIO_SRC_MEMORY)
			{
				if (m_enable_rtp_send && m_enable_memory_fill_send)
				{	// freeswitch的rtp中,要想接收数据,必须要发送数据
					memset(p_sample_buf, 0, m_samples_per_packet * sizeof (short));
					write_rtp_senddata(p_sample_buf, m_samples_per_packet);
				}
			}
			m_audio_input_len = 0;
		}

		// 接收
		if (m_enable_rtp_recv)
		{
			have_move = data_len = 0;
			if (m_prtp_session_audio != NULL)
				data_len = rtp_session_recv_with_ts(m_prtp_session_audio, p_codec_buf, codec_buf_len, m_rtp_audio_ts, &have_move);//m_samples_per_packet * sizeof (short)
			if (data_len > 0)
			{
				if (m_audio_src == AUDIO_SRC_MEMORY && m_audio_format == AUDIO_FORMAT_MP3 && !m_enable_mp3decoder)
				{
					memcpy(p_sample_buf, p_codec_buf, min((int)sample_buf_len, data_len));
					data_len /= sizeof (short);
				}
				else
					data_len = decoder_data(p_sample_buf, sample_buf_len, p_codec_buf, data_len);
				if (data_len > 0)
				{
					ua_mutex_lock(&m_mutex_rtp_buf);
					for (i = 0; i < data_len; i++)
					{
						m_rtp_buf_arx[m_rtp_write_arx++] = p_sample_buf[i];//lhg can improve speed  :   memcpy
						if (m_rtp_write_arx >= m_rtp_buf_max)
							m_rtp_write_arx = 0;
					}
					ua_mutex_unlock(&m_mutex_rtp_buf);
					if (m_precvdata_callback != NULL)
						m_precvdata_callback(m_precvdata_callback_param);
				}
			}
		}
		// 发送
		if (m_enable_rtp_send)
		{
			if (m_rtp_read_atx <= m_rtp_write_atx)
				data_len = m_rtp_write_atx - m_rtp_read_atx;
			else
				data_len = m_rtp_buf_max - m_rtp_read_atx + m_rtp_write_atx;
			if (m_audio_format == AUDIO_FORMAT_MP3)
			{
				if (m_audio_src == AUDIO_SRC_FILE)
					frame_len = (int)(m_audio_file.m_out_bitrate / 8 / (1000 / m_mseconds_per_packet) / sizeof (short));
				else // AUDIO_SRC_SOUNDCARD or AUDIO_SRC_MEMORY
					frame_len = m_samples_per_packet;
			}
			else
				frame_len = m_samples_per_packet;
			if (data_len >= frame_len)
			{
				ua_mutex_lock(&m_mutex_rtp_buf);
				for (i = 0; i < frame_len; i++)
				{
					p_sample_buf[i] = m_rtp_buf_atx[m_rtp_read_atx++];  //lhg can improve speed  :  memcpy
					if (m_rtp_read_atx >= m_rtp_buf_max)
						m_rtp_read_atx = 0;
				}
				ua_mutex_unlock(&m_mutex_rtp_buf);
				if (m_file_atx != NULL)
					fwrite(p_sample_buf, 1, frame_len * sizeof (short), m_file_atx);
				data_len = encoder_data(p_codec_buf, p_sample_buf, frame_len);
				if (data_len > 0)
				{
					//memset(data_buf, 0, data_len);
					if (m_prtp_session_audio != NULL)
						rtp_session_send_with_ts(m_prtp_session_audio, p_codec_buf, data_len, m_rtp_audio_ts);
				}
			}
			else if (m_audio_src == AUDIO_SRC_MEMORY && m_rtp_audio_ts < 2000)
			{	// 配合虚拟终端使用时,虚拟终端不会先发送数据,所以先发送一段时间的数据
				data_len = frame_len;
				memset(p_codec_buf, 0, data_len);
				rtp_session_send_with_ts(m_prtp_session_audio, p_codec_buf, data_len, m_rtp_audio_ts);
			}
		}

		if (m_audio_format == AUDIO_FORMAT_MP3)
			m_rtp_audio_ts += 8000 * m_mseconds_per_packet / 1000;
		else
			m_rtp_audio_ts += m_samples_per_packet;
		if (p_sample_buf != NULL)
		{
			ua_free(p_sample_buf);
			p_sample_buf = NULL;
		}
		if (p_codec_buf != NULL)
		{
			ua_free(p_codec_buf);
			p_codec_buf = NULL;
		}
	}
}

// 定时器处理
#ifdef WIN32
void CALLBACK CAudioStream::on_timer_event(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    timer_data		*p_tdata = (timer_data *)dwUser;
    CAudioStream	*p_audio_stream = (CAudioStream *)p_tdata->p_param;

	//static int		tick = 0;
	//char			str_buf[64];

    switch (p_tdata->sival)
    {
    case TIMER_SIVAL_AUDIO:
		ua_mutex_lock(&m_mutex_as);
		//tick++;
		//if (tick >= 50)
		//{
		//	tick = 0;
		//	sprintf(str_buf, "\non_timer : %d", GetTickCount());
		//	OutputDebugString(str_buf);
		//}
		m_timer_rtp_data = FALSE;
		if (!m_list_as.IsEmpty())
			m_list_as.TraverseHead(running_rtp_data);
		if (!m_timer_rtp_data)
		{
			if (m_tdata_audio.timerid != NULL)
				ua_delete_timer_event(&m_tdata_audio, FALSE);
		}
		ua_mutex_unlock(&m_mutex_as);
        break;
    default:
        break;
    }
}

void CALLBACK CAudioStream::on_timer_queue(void *p_param, BOOL timeorwait)
{
    timer_data		*p_tdata = (timer_data *)p_param;
    CAudioStream	*p_audio_stream = (CAudioStream *)p_tdata->p_param;

    switch (p_tdata->sival)
    {
    case TIMER_MP3DECODE_SAMPLERATE:
		if (p_audio_stream != NULL)
		{
			if (p_audio_stream->m_mp3decode_samplerate != p_audio_stream->m_sample_rate)
				p_audio_stream->open_soundcard(p_audio_stream->m_mp3decode_samplerate);
 			if (p_audio_stream->m_tdata_mp3decode_samplerate.timerid != NULL)
				ua_delete_timer_queue(&(p_audio_stream->m_tdata_mp3decode_samplerate), FALSE);
		}
       break;
    default:
        break;
    }
}

#else
void CAudioStream::on_timer_event(union sigval sig_val)
{
    timer_data		*p_tdata = (timer_data *)sig_val.sival_ptr;
    CAudioStream	*p_audio_stream = (CAudioStream *)p_tdata->p_param;

    switch (p_tdata->sival)
    {
    case TIMER_SIVAL_AUDIO:
		ua_mutex_lock(&m_mutex_as);
		m_timer_rtp_data = FALSE;
		if (!m_list_as.IsEmpty())
			m_list_as.TraverseHead(running_rtp_data);
		if (!m_timer_rtp_data)
		{
			if (m_tdata_audio.timerid != NULL)
				ua_delete_timer(&m_tdata_audio, FALSE);
		}
		ua_mutex_unlock(&m_mutex_as);
        break;
    default:
        break;
    }
}

void CAudioStream::on_timer_queue(union sigval sig_val)
{
    timer_data		*p_tdata = (timer_data *)sig_val.sival_ptr;
    CAudioStream	*p_audio_stream = (CAudioStream *)p_tdata->p_param;

    switch (p_tdata->sival)
    {
    case TIMER_MP3DECODE_SAMPLERATE:
		if (p_audio_stream != NULL)
		{
			if (p_audio_stream->m_mp3decode_samplerate != p_audio_stream->m_sample_rate)
				p_audio_stream->open_soundcard(p_audio_stream->m_mp3decode_samplerate);
 			if (p_audio_stream->m_tdata_mp3decode_samplerate.timerid != NULL)
				ua_delete_timer(&(p_audio_stream->m_tdata_mp3decode_samplerate), FALSE);
		}
       break;
    default:
        break;
    }
}
#endif


// 线程回调函数(处理rtp音频数据)
BOOL CAudioStream::running_rtp_data(CAudioStream *&p_audio_stream)
{
	if (p_audio_stream != NULL && p_audio_stream->m_rtp_is_connect && !p_audio_stream->m_use_sound_card)
	{
		memset(p_audio_stream->m_audio_input_buf, 0, p_audio_stream->m_samples_per_packet * sizeof (short));
		p_audio_stream->m_audio_input_len = p_audio_stream->m_samples_per_packet;
		m_timer_rtp_data = TRUE;
		p_audio_stream->process_rtp_data();
	}
	return TRUE;
}

// 数据编码, samples为采样个数, 返回编码后字节数
int CAudioStream::encoder_data(BYTE *p_dst, short *p_src, int samples)
{
	int				encoder_len = 0;
	BE_ERR			be_err;

	switch (m_audio_format)
	{
	case AUDIO_FORMAT_PCMU:		CodecUlaw_Encoder(p_dst, p_src, samples * 2); encoder_len = samples; break;
	case AUDIO_FORMAT_PCMA:		CodecAlaw_Encoder(p_dst, p_src, samples * 2); encoder_len = samples; break;
	case AUDIO_FORMAT_MP3:
		if (m_audio_src != AUDIO_SRC_FILE)
		{
			if (m_mp3encoder != 0)
			{
				be_err = beEncodeChunk(m_mp3encoder, samples, p_src, p_dst, (DWORD *)&encoder_len);
				if (be_err != BE_ERR_SUCCESSFUL)
					printf_log(LOG_IS_DEBUG, "[CAudioStream::encoder_data(beEncodeChunk)](%d) : faild\n", be_err);
			}
		}
		else
		{
			memcpy(p_dst, p_src, samples * sizeof (short));
			encoder_len = samples * sizeof (short);
		}
		break;
	case AUDIO_FORMAT_L16:		memcpy(p_dst, p_src, samples * sizeof (short)); encoder_len = samples * sizeof (short); break;
	default:	break;
	}

	return encoder_len;
}

// 数据解码, len为字节数, 返回编码后采样个数
int CAudioStream::decoder_data(short *p_dst, int dst_samples, BYTE *p_src, int src_len)
{
	int				decoder_len = 0;
	int				process_sum, process_cur, samples;
	int				data_in_len, data_out_len;
	DWORD			samplerate_cur;
	
	switch (m_audio_format)
	{
	case AUDIO_FORMAT_PCMU:		CodecUlaw_Decoder(p_dst, p_src, src_len); decoder_len = src_len; break;
	case AUDIO_FORMAT_PCMA:		CodecAlaw_Decoder(p_dst, p_src, src_len); decoder_len = src_len; break;
	case AUDIO_FORMAT_MP3:
		if (m_audio_src != AUDIO_SRC_FILE)
		{
			data_in_len = src_len;
			data_out_len = 0;
			process_sum = 0;
			do
			{
				process_cur = mad_get_frame_size(&m_mp3decoder);
				if (process_cur > MAD_IN_LEN_MAX)
					process_cur = MAD_IN_LEN_MAX;
				if (process_cur == 0)
					process_cur = MAD_IN_LEN_MAX / 4;
				if (process_cur > data_in_len - process_sum)
					process_cur = data_in_len - process_sum;
				process_cur = mad_decode(&m_mp3decoder, &p_src[process_sum], process_cur);
				process_sum += process_cur;
				samples = mad_get_out_len(&m_mp3decoder);
				if (samples > 0)	// 解码出一帧
				{
					samplerate_cur = mad_get_samplerate(&m_mp3decoder);
					if (samplerate_cur != 0 && samplerate_cur != m_mp3decode_samplerate)
					{
						m_mp3decode_samplerate = samplerate_cur;
						if (m_tdata_mp3decode_samplerate.timerid != NULL)
							ua_delete_timer_queue(&m_tdata_mp3decode_samplerate, FALSE);
						ua_create_timer_queue(&m_tdata_mp3decode_samplerate, on_timer_queue, 100, 0, TIMER_MP3DECODE_SAMPLERATE);
						printf_log(LOG_IS_INFO, "[CAudioStream::decoder_data()] : sample rate %d\n", m_mp3decode_samplerate);
					}
					samples = mad_get_out_data(&m_mp3decoder, &p_dst[data_out_len], NULL, (dst_samples - data_out_len));
					data_out_len += samples;
				}
			} while (process_sum < data_in_len && process_cur > 0);

			decoder_len = data_out_len;
		}
		else
		{
			memcpy(p_dst, p_src, src_len);
			decoder_len = src_len / sizeof (short);
		}
		break;
	case AUDIO_FORMAT_L16:		memcpy(p_dst, p_src, src_len); decoder_len = src_len / sizeof (short); break;
	default:	break;
	}

	return decoder_len;
}

// 进行一个rtp连接
BOOL CAudioStream::rtp_connect(rtp_connect_param_t *p_param)
{
	BOOL			b_find_profile = FALSE;
	PayloadType		*p_payload_type = NULL;
	char			p_strerr[PATH_MAX] = {0};

	try
	{
		rtp_unconnect();
		if (p_param == NULL || p_param->p_local_ip == NULL || p_param->p_remote_ip == NULL || p_param->p_mime_type == NULL)
		{
			sprintf(p_strerr, "[CAudioStream::rtp_connect()] : parameter invalid\n");
			throw 10;
		}
		if (strcasecmp(p_param->p_mime_type, g_mime_type_pcmu) == 0)
		{
			p_param->payload_index = 0;
			b_find_profile = TRUE;
			m_audio_format = AUDIO_FORMAT_PCMU;
		}
		else if (strcasecmp(p_param->p_mime_type, g_mime_type_pcma) == 0)
		{
			b_find_profile = TRUE;
			p_param->payload_index = 8;
			m_audio_format = AUDIO_FORMAT_PCMA;
		}
		else if (strcasecmp(p_param->p_mime_type, g_mime_type_l16) == 0)
		{
			if (p_param->sample_rate == 8000)
			{
				b_find_profile = TRUE;
				p_payload_type = &g_payload_type_l16_8000;
				m_audio_format = AUDIO_FORMAT_L16;
			}
			else if (p_param->sample_rate == 22050)
			{
				b_find_profile = TRUE;
				p_payload_type = &g_payload_type_l16_22050;
				m_audio_format = AUDIO_FORMAT_L16;
			}
		}
		else if (strcasecmp(p_param->p_mime_type, g_mime_type_mp3) == 0)
		{
			b_find_profile = TRUE;
			p_payload_type = &g_payload_type_mp3;
			m_audio_format = AUDIO_FORMAT_MP3;
		}
		if (!b_find_profile)
		{
			sprintf(p_strerr, "[CAudioStream::rtp_connect()] : audio format(%s) is not supported\n", p_param->p_mime_type);
			throw 11;
		}
		m_prtp_profile = rtp_profile_clone_full(&av_profile);
		m_prtp_session_audio = rtp_session_new(RTP_SESSION_SENDRECV);
		m_prtp_event_audio = ortp_ev_queue_new();
		if (m_prtp_session_audio == NULL || m_prtp_event_audio == NULL || m_prtp_profile == NULL)
		{
			sprintf(p_strerr, "[CAudioStream::rtp_connect()] : rtp initialize failed\n");
			throw 12;
		}
		// 设置负载类型
		if (p_payload_type != NULL)
		{
			rtp_profile_set_payload(m_prtp_profile, p_param->payload_index, p_payload_type);
			rtp_profile_set_payload(m_prtp_profile, 106, p_payload_type);
		}
		rtp_session_set_profile(m_prtp_session_audio, m_prtp_profile);
		rtp_session_set_payload_type(m_prtp_session_audio, p_param->payload_index);
		rtp_session_register_event_queue(m_prtp_session_audio, m_prtp_event_audio);
		// 设置远程RTP客户端的的IP和监听端口(即本rtp数据包的发送目的地址)
		rtp_session_set_remote_addr(m_prtp_session_audio, p_param->p_remote_ip, p_param->remote_port);
		rtp_session_set_local_addr(m_prtp_session_audio, p_param->p_local_ip, p_param->local_port, p_param->local_port + 10000);

		if (ua_is_multicast(p_param->p_remote_ip))
		{
			int						err;
#ifdef WIN32
			// windows上要使用组播,必须使用WSASocket, 还后面WSA_FLAG_MULTIPOINT的标志
			struct sockaddr_in		addr;
			int						flag = 1;
			unsigned long			nonBlock = 1;

			close_socket(m_prtp_session_audio->rtp.socket);
			m_prtp_session_audio->rtp.socket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, (LPWSAPROTOCOL_INFO)NULL, 0, \
														WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF);
			if (m_prtp_session_audio->rtp.socket == INVALID_SOCKET)
			{
				sprintf(p_strerr, "[CAudioStream::rtp_connect(WSASocket)](%d) : Fail.\n", getSocketErrorCode());
				throw 13;
			}
			err = setsockopt(m_prtp_session_audio->rtp.socket, SOL_SOCKET, SO_REUSEADDR, (CHAR *)&flag, sizeof(flag));
			if (err == SOCKET_ERROR)
			{
				sprintf(p_strerr, "[CAudioStream::rtp_connect(setsockopt)](%d) : Fail.\n", getSocketErrorCode());
				throw 14;
			}
			addr.sin_family = PF_INET;
			addr.sin_addr.s_addr = inet_addr(p_param->p_local_ip);
			addr.sin_port = htons(p_param->local_port);
			memset(&(addr.sin_zero), 0, sizeof (addr.sin_zero));
			err = bind(m_prtp_session_audio->rtp.socket, (struct sockaddr FAR *)&addr, sizeof(struct sockaddr)); 
			if (err == SOCKET_ERROR)
			{
				sprintf(p_strerr, "[CAudioStream::rtp_connect(bind)](%d) : Fail.\n", getSocketErrorCode());
				throw 15;
			}
			ioctlsocket(m_prtp_session_audio->rtp.socket, FIONBIO , &nonBlock);

			addr.sin_family = PF_INET;
			addr.sin_addr.s_addr = inet_addr(p_param->p_remote_ip);
			addr.sin_port = htons(p_param->remote_port);
			memset(&(addr.sin_zero), 0, sizeof (addr.sin_zero));
			if (WSAJoinLeaf(m_prtp_session_audio->rtp.socket, (PSOCKADDR)&addr, sizeof (SOCKADDR), \
				NULL, NULL, NULL, NULL, JL_BOTH) == INVALID_SOCKET)
			{
				sprintf(p_strerr, "[CAudioStream::rtp_connect(WSAJoinLeaf)](%d) : Fail.\n", getSocketErrorCode());
				throw 16;
			}
#else
			struct ip_mreq mreq;
			mreq.imr_multiaddr.s_addr = inet_addr(p_param->p_remote_ip);
			mreq.imr_interface.s_addr = INADDR_ANY;
			err = setsockopt(m_prtp_session_audio->rtp.socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (SOCKET_OPTION_VALUE) &mreq, sizeof(mreq));
			if (err < 0)
			{
				sprintf(p_strerr, "[CAudioStream::rtp_connect(setsockopt)](%d) : Fail to join address group.\n", getSocketErrorCode());
				throw 13;
			}
#endif
			rtp_session_set_multicast_loopback(m_prtp_session_audio, TRUE);	// 必须要设置, 否则不能收到本机发送的数据
			rtp_session_set_multicast_ttl(m_prtp_session_audio, 64);
		}

		m_rtp_audio_ts = 0;
		m_rtp_read_arx = m_rtp_write_arx = m_rtp_read_atx = m_rtp_write_atx = 0;
		if (m_audio_format == AUDIO_FORMAT_MP3 && m_audio_src == AUDIO_SRC_SOUNDCARD)
			m_sample_rate = 44100;
		else
			m_sample_rate = p_param->sample_rate;
		if (p_param->mseconds_per_packet >= 10 && p_param->mseconds_per_packet <= MAX_MS_PER_FRAME)
			m_mseconds_per_packet = p_param->mseconds_per_packet;
		else
			m_mseconds_per_packet = MSECOND_PER_PACKET;
		m_samples_per_packet = m_sample_rate * m_mseconds_per_packet / 1000;
		if (p_param->b_rtp_buf_max)
			m_rtp_buf_max = RTP_AUDIO_LEN_MAX;
		else 
		{
			if (p_param->rtp_buf_ms >= MIN_MS_PER_FRAME)
			{
				m_rtp_buf_max = m_sample_rate * p_param->rtp_buf_ms / 1000;
				if (m_rtp_buf_max > RTP_AUDIO_LEN_MAX)
					m_rtp_buf_max = RTP_AUDIO_LEN_MAX;
			}
			else
				m_rtp_buf_max = RTP_AUDIO_LEN_MAX;
		}
		m_enable_rtp_send = p_param->enable_rtp_send;
		m_enable_rtp_recv = p_param->enable_rtp_recv;
		m_enable_memory_fill_send = p_param->enable_memory_fill_send;
		if (m_audio_format == AUDIO_FORMAT_MP3)
			m_audio_file.set_out_format(AUDIO_TYPE_MP3, m_sample_rate, m_mseconds_per_packet);
		else
			m_audio_file.set_out_format(AUDIO_TYPE_PCM, m_sample_rate, m_mseconds_per_packet);
		m_audio_file.reset_play();
		m_audio_file.play();
		m_audio_file.init_mp3_encoder(&m_mp3encoder, m_sample_rate, 1, m_mp3encode_bitrate, &m_mp3encode_samples, &m_mp3encode_out_size);

		if (m_record)
			create_record_file(p_param->p_user_name, p_param->p_callee_name);

		if (m_enable_sound_card)
			open_soundcard(m_sample_rate);
		if (!m_use_sound_card)
		{
			if (m_tdata_audio.timerid == NULL)
				ua_create_timer_event(&m_tdata_audio, on_timer_event, m_mseconds_per_packet, m_mseconds_per_packet, TIMER_SIVAL_AUDIO);
		}

		m_rtp_is_connect = TRUE;
		printf_log(LOG_IS_INFO, "[CAudioStream::rtp_connect()] : %s:%d - %s:%d\n", p_param->p_local_ip, p_param->local_port, p_param->p_remote_ip, p_param->remote_port);
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
		printf_log(LOG_IS_ERR, "[CAudioStream::rtp_connect()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CAudioStream::rtp_connect()] : error catch!\n");
    }

	return m_rtp_is_connect;
}

// RTP断开连接
void CAudioStream::rtp_unconnect(void)
{
	m_rtp_is_connect = FALSE;
	ua_mutex_lock(&m_mutex_unconnect);
	close_soundcard();
	if (m_prtp_session_audio != NULL && m_prtp_event_audio != NULL)
		rtp_session_unregister_event_queue(m_prtp_session_audio, m_prtp_event_audio);
	if (m_prtp_event_audio != NULL)
	{
		ortp_ev_queue_destroy(m_prtp_event_audio);
		m_prtp_event_audio = NULL;
	}
	if (m_prtp_session_audio != NULL)
	{
		rtp_session_destroy(m_prtp_session_audio);
		m_prtp_session_audio = NULL;
	}
	if (m_prtp_profile != NULL)
	{
		rtp_profile_destroy(m_prtp_profile);
		m_prtp_profile = NULL;
	}
	mad_destroy(&m_mp3decoder);
	mad_init(&m_mp3decoder);
	m_audio_file.stop();
	m_audio_format = AUDIO_FORMAT_PCMU;
	close_record_file();
	ua_mutex_unlock(&m_mutex_unconnect);
}

// 获取支持的媒体格式
char *CAudioStream::get_media_containing(char *p_containing, int len, WORD rtp_port, int payload_type, const char *p_mine_type, int sample_rate)
{
	int				i, str_len;

	if (p_containing != NULL)
	{
		if (payload_type != 0 && p_mine_type != NULL)
		{
			snprintf(p_containing, len,
					"m=audio %d RTP/AVP 0 8 101 %d\r\n"
					"a=rtpmap:0 PCMU/8000\r\n"
					"a=rtpmap:8 PCMA/8000\r\n"
					"a=rtpmap:101 telephone-event/8000\r\n"
					"a=fmtp:101 0-11\r\n", 
					rtp_port, payload_type);
			str_len = strlen(p_containing);
			snprintf(&p_containing[str_len], len - str_len,
					"a=rtpmap:%d %s/%d\r\n", 
					payload_type, 
					p_mine_type, 
					sample_rate);
			p_containing[len - 1] = '\0';
		}
		else
		{
			snprintf(p_containing, len,
					"m=audio %d RTP/AVP 0 8 101 %d %d %d\r\n"
					"a=rtpmap:0 PCMU/8000\r\n"
					"a=rtpmap:8 PCMA/8000\r\n"
					"a=rtpmap:101 telephone-event/8000\r\n"
					"a=fmtp:101 0-11\r\n", 
					rtp_port, PAYLOAD_TYPE_L16_22050, PAYLOAD_TYPE_L16_8000, PAYLOAD_TYPE_MP3);
			for (i = 0; i < PAYLOAD_COUNT; i++)
			{
				str_len = strlen(p_containing);
				snprintf(&p_containing[str_len], len - str_len,
						"a=rtpmap:%d %s/%d\r\n", 
						g_ppayload_type[i].no, 
						g_ppayload_type[i].p_payload->mime_type, 
						g_ppayload_type[i].p_payload->clock_rate);
			}
			p_containing[len - 1] = '\0';
		}
	}

	return p_containing;
}

// 关闭录音文件
void CAudioStream::close_record_file(void)
{
	WAVEFILEINFO	waveFileInfo;
	WAVEFORMATEX	waveFileFormat;
	WAVEFILEDATA	waveFileData;
	int				i, file_len;
	FILE			**pp_file[2] = {&m_file_arx, &m_file_atx};

	// 文件头信息
	waveFileInfo.dwRIFFID	= TAG_RIFF;
	waveFileInfo.dwRIFFSize	= 0;
	waveFileInfo.dwFileType	= TAG_WAVE;
	waveFileInfo.dwFmtID	= TAG_FMT;
	waveFileInfo.dwFmtSize	= sizeof(WAVEFORMATEX); 
	// 文件头格式
	waveFileFormat.wFormatTag		= WAVE_FORMAT_PCM;
	waveFileFormat.nChannels		= 1;
	waveFileFormat.nSamplesPerSec	= m_sample_rate;
	waveFileFormat.wBitsPerSample	= 16;
	waveFileFormat.nBlockAlign		= waveFileFormat.wBitsPerSample / 8;
	waveFileFormat.nAvgBytesPerSec	= waveFileFormat.nBlockAlign * waveFileFormat.nSamplesPerSec * waveFileFormat.nChannels;
	waveFileFormat.cbSize			= 0;
	// 文件数据区长度
	waveFileData.dwDATAID	= TAG_DATA;
	waveFileData.dwDATASize = 0;
	
	for (i = 0; i < 2; i++)
	{
		if ((*pp_file[i]) != NULL)
		{
			if (m_audio_format != AUDIO_FORMAT_MP3)
			{
				fseek((*pp_file[i]), 0, SEEK_END);
				file_len = ftell((*pp_file[i]));
				waveFileInfo.dwRIFFSize	= file_len - 8;
				waveFileData.dwDATASize = file_len - (sizeof (WAVEFILEINFO) + sizeof (WAVEFORMATEX) + sizeof (WAVEFILEDATA));
				fseek((*pp_file[i]), 0, SEEK_SET);
				fwrite(&waveFileInfo, 1, sizeof(WAVEFILEINFO), (*pp_file[i]));
				fwrite(&waveFileFormat, 1, sizeof(WAVEFORMATEX), (*pp_file[i]));
				fwrite(&waveFileData, 1, sizeof(WAVEFILEDATA), (*pp_file[i]));
			}
			fclose((*pp_file[i]));
			(*pp_file[i]) = NULL;
		}
	}
}

// 创建录音文件
void CAudioStream::create_record_file(const char *p_user_name, const char *p_callee_name)
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
			get_exe_path(str_buf, PATH_MAX);
			str_path_rx = str_buf;
			str_path_rx = str_path_rx.substr(0, str_path_rx.find_last_of(DIRECTORY_SPLIT_CHAR) + 1);

			// AUDIO_RECORD_FOLDER_NAME文件夹, 年份文件夹, 月份文件夹创建
			str_folder[0] = AUDIO_RECORD_FOLDER_NAME;
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
						printf_log(LOG_IS_ERR, "[CAudioStream::create_record_file(mkdir)](%d) : %s\n", errno, strerror(errno));
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
			if (m_audio_format == AUDIO_FORMAT_MP3)
			{
				str_path_tx += ".mp3";
				str_path_rx += ".mp3";
			}
			else
			{
				str_path_tx += ".wav";
				str_path_rx += ".wav";
			}

			// 用户自己的录音文件
			m_file_atx = fopen(str_path_tx.c_str(), "wb");
			if (m_file_atx == NULL)
			{
				sprintf(p_strerr, "[CAudioStream::create_record_file(fopen)](%d) : '%s'%s\n", errno, str_path_tx.c_str(), strerror(errno));
				throw 10;
			}
			m_file_arx = fopen(str_path_rx.c_str(), "wb");
			if (m_file_arx == NULL)
			{
				sprintf(p_strerr, "[CAudioStream::create_record_file(fopen)](%d) : '%s'%s\n", errno, str_path_rx.c_str(), strerror(errno));
				throw 11;
			}
			if (m_audio_format != AUDIO_FORMAT_MP3)
			{
				memset(str_buf, 0, PATH_MAX);
				fwrite(str_buf, 1, sizeof (WAVEFILEINFO) + sizeof (WAVEFORMATEX) + sizeof (WAVEFILEDATA), m_file_atx);
				fwrite(str_buf, 1, sizeof (WAVEFILEINFO) + sizeof (WAVEFORMATEX) + sizeof (WAVEFILEDATA), m_file_arx);
			}
		}
		catch (int throw_err)
		{
			switch (throw_err)
			{
			case 11:
				fclose(m_file_atx);
			case 10:
			default:
				break;
			}
			if (strlen(p_strerr) > 0)
				printf_log(LOG_IS_ERR, "%s", p_strerr);
		}
		catch (std::exception &e)
		{   
			printf_log(LOG_IS_ERR, "[CAudioStream::create_record_file()] : %s", e.what());
		}
		catch (...)
		{   
			printf_log(LOG_IS_ERR, "[CAudioStream::create_record_file()] : error catch!\n");
		}
	}
}


// 设置音频接收数据回调处理(主循环实时性不好)
void CAudioStream::set_recvdata_callback(void(*p_callback)(void *), void *p_param)
{
	m_precvdata_callback = p_callback;
	m_precvdata_callback_param = p_param;
}


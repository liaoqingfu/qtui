/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : AudioFile.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : 音频文件类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#include "ua_port.h"
#include <time.h>

#ifdef WIN32
#pragma comment(lib, "libmad.lib")
#pragma comment(lib, "liblame.lib")
#pragma comment(lib, "libresample.lib")
#endif

#include "AudioFile.h"
#include "log.h"
#include "defines.h"

CAudioFile::CAudioFile()
{
	m_play_mode = PLAY_MODE_ALL_LOOP;
	m_out_audio_type = AUDIO_TYPE_PCM;
	m_out_sample_rate = 22050;
	m_out_msecond_per_frame = 20;
	m_out_bitrate = 0;
	m_pout_buf = NULL;
	m_out_buf_size = m_out_buf_len = 0;
	init_audio_info(&m_audio_info_playing);
	ua_mutex_init(&m_mutex_file);
	m_file = NULL;
	m_data_read = m_data_size = 0;
	m_file_data_read = m_file_data_progress = 0;
	ua_mutex_init(&m_mutex_play_list);
	m_list_pos = -1;
	m_bmp3_decode = m_bmp3_encode = m_bresample = FALSE;
	m_play_status = PLAY_STATUS_STOP;
	mad_init(&m_mp3_decoder);
	m_mp3_encoder = 0;
	m_presampler = NULL;
	m_mp3_encode_samples = m_mp3_encode_out_size = 0;
	m_pplay_event_callback = NULL;
	m_pevent_callback_param = NULL;
}

CAudioFile::~CAudioFile()
{
	close_file();
	mad_destroy(&m_mp3_decoder);
	if (m_mp3_encoder != 0)
	{
		beCloseStream(m_mp3_encoder);
		m_mp3_encoder = 0;
	}
	if (m_presampler != NULL)
	{
		ResampleDestroy(m_presampler);
		m_presampler = NULL;
	}
	if (m_pout_buf != NULL)
	{
		ua_free(m_pout_buf);
		m_pout_buf = NULL;
		m_out_buf_size = m_out_buf_len = 0;
	}
	destroy_play_list();
	ua_mutex_destroy(&m_mutex_play_list);
	ua_mutex_destroy(&m_mutex_file);
}

// 关闭文件
void CAudioFile::close_file(void)
{
	ua_mutex_lock(&m_mutex_file);
	if (m_file != NULL)
	{
		fclose(m_file);
		m_file = NULL;
	}
	ua_mutex_unlock(&m_mutex_file);
	destroy_audio_info(&m_audio_info_playing);
	m_bmp3_decode = m_bmp3_encode = m_bresample = FALSE;
	m_file_data_read = m_file_data_progress = 0;
	m_data_read = m_data_size = 0;
}

// 打开文件
BOOL CAudioFile::open_file(void)
{
	BOOL			b_open = FALSE;
	audio_info_t	*p_audio = NULL;
	int				error = 0;

	close_file();
	if (m_list_pos >= 0 && m_list_pos < m_list_play.GetCount())
	{
		if (m_list_play.GetElem(m_list_pos + 1, p_audio) && p_audio->file_path != NULL)
		{
			ua_mutex_lock(&m_mutex_file);
			m_file = fopen(p_audio->file_path, "rb");
			if (m_file != NULL)
			{
				file_data_read(NULL, p_audio->data_offset);
				dup_audio_info(&m_audio_info_playing, p_audio);
				if (m_audio_info_playing.audio_type == AUDIO_TYPE_PCM)
				{	// WAV
					if (m_out_audio_type == AUDIO_TYPE_PCM)
					{
						m_out_bitrate = m_out_sample_rate * 16;
						if (m_audio_info_playing.sample_rate != m_out_sample_rate)
							m_bresample = TRUE;
					}
					else
					{
						m_out_bitrate = MP3_ENCODER_BITRATE;
						m_bmp3_encode = TRUE;
					}
				}
				else
				{	// MP3
					if (m_out_audio_type == AUDIO_TYPE_PCM)
					{
						m_out_bitrate = m_out_sample_rate * 16;
						m_bmp3_decode = TRUE;
						if (m_audio_info_playing.sample_rate != m_out_sample_rate)
							m_bresample = TRUE;
					}
					else
						m_out_bitrate = m_audio_info_playing.bitrate;
				}
				// 初始化输出缓冲
				if (m_pout_buf != NULL)
				{
					ua_free(m_pout_buf);
					m_pout_buf = NULL;
					m_out_buf_size = m_out_buf_len = 0;
				}
				if (m_bmp3_decode || m_bmp3_encode || m_bresample)
				{	// 数据交换区
					m_out_buf_size = (m_out_bitrate * m_out_msecond_per_frame / 1000 / 8) + (m_out_bitrate * 26 / 1000 / 8);
					m_pout_buf = (BYTE *)ua_malloc(m_out_buf_size);
				}
				// 初始化编解码器及重采样
				if (m_bmp3_decode)
				{
					mad_destroy(&m_mp3_decoder);
					mad_init(&m_mp3_decoder);
				}
				else if (m_bmp3_encode)
					init_mp3_encoder(&m_mp3_encoder, m_audio_info_playing.sample_rate, m_audio_info_playing.channels, MP3_ENCODER_BITRATE, &m_mp3_encode_samples, &m_mp3_encode_out_size);
				if (m_bresample)
				{
					if (m_presampler != NULL)
					{
						ResampleDestroy(m_presampler);
						m_presampler = NULL;
					}
					m_presampler = ResampleInit(m_audio_info_playing.channels, m_audio_info_playing.sample_rate, m_out_sample_rate, 0, &error);
				}
				b_open = TRUE;

				m_file_data_read = m_file_data_progress = 0;
				if (m_pplay_event_callback != NULL && p_audio->bitrate >= 8)
					m_pplay_event_callback(PLAY_EVENT_TYPE_OPEN_FILE, m_list_pos, p_audio->data_size / (p_audio->bitrate / 8), m_pevent_callback_param);
			}
			ua_mutex_unlock(&m_mutex_file);
		}
	}

	return b_open;
}

// 复制音频信息
BOOL CAudioFile::dup_audio_info(audio_info_t *p_audio_dst, audio_info_t *p_audio_src)
{
	BOOL			b_dup = FALSE;

	if (p_audio_dst != NULL && p_audio_src != NULL)
	{
		destroy_audio_info(p_audio_dst);
		*p_audio_dst = *p_audio_src;
		if (p_audio_src->file_path != NULL)
			p_audio_dst->file_path = string_dup(p_audio_src->file_path);
		b_dup = TRUE;
	}

	return b_dup;
}


// 开始播放(返回文件序号,0起始)
int CAudioFile::play(void)
{
	m_play_status = PLAY_STATUS_PLAYING;
	return m_list_pos;
}

// 停止播放(返回文件序号,0起始)
int CAudioFile::stop(void)
{
	m_play_status = PLAY_STATUS_STOP;
	close_file();
	return m_list_pos;
}

// 暂停播放(返回文件序号,0起始)
int CAudioFile::pause(void)
{
	m_play_status = PLAY_STATUS_PAUSE;
	return m_list_pos;
}

// 播放前一个文件(返回文件序号,0起始)
int CAudioFile::previous(void)
{
	if (m_list_pos > 0)
	{
		m_list_pos -= 1;
		open_file();
	}
	return m_list_pos;
}

// 播放下一个文件(返回文件序号,0起始)
int CAudioFile::next(void)
{
	if (m_list_pos + 1 < m_list_play.GetCount())
	{
		m_list_pos += 1;
		open_file();
	}
	return m_list_pos;
}

// 设置播放文件的序号(0起始)
int CAudioFile::set_play_index(int index)
{
	if (index >= 0 && index < m_list_play.GetCount())
	{
		m_list_pos = index;
		open_file();
	}
	return m_list_pos;
}

// 根据播放模式播放下一个文件(返回文件序号,0起始)
int CAudioFile::play_next(void)
{
	int				count = m_list_play.GetCount();
	time_t			now;

	if (count > 0)
	{
		switch (m_play_mode)
		{
		case PLAY_MODE_SINGLE_PLAY:
			m_list_pos = -1;
			break;
		case PLAY_MODE_SINGLE_LOOP:
			break;
		case PLAY_MODE_ALL_PLAY:
			m_list_pos++;
			if (m_list_pos >= m_list_play.GetCount())
				m_list_pos = -1;
			break;
		case PLAY_MODE_ALL_LOOP:
			m_list_pos++;
			if (m_list_pos >= m_list_play.GetCount())
				m_list_pos = 0;
			break;
		case PLAY_MODE_RANDOM_PLAY:
			srand((unsigned int)time(&now));
			m_list_pos = rand() % count;
			break;
		default:
			m_list_pos = -1;
			break;
		}
	}
	else
		m_list_pos = -1;

	open_file();

	return m_list_pos;
}

BOOL CAudioFile::add_file(const char *p_file_path)
{
	BOOL			b_add = FALSE;
	char			p_strerr[PATH_MAX] = {0};
	int				len;
	audio_info_t	*p_audio = NULL;
	int				audio_type = AUDIO_TYPE_UNKNOWN;

	try
    {
		// 入口参数检查
		if (p_file_path != NULL && strlen(p_file_path) == 0)
		{
            sprintf(p_strerr, "[CAudioFile::add_file()] : parameter invalid\n");
            throw 10;
		}
		len = strlen(p_file_path);
		if (len > 4)
		{
			if (strncasecmp(&p_file_path[len - 4], ".wav", 4) == 0)
				audio_type = AUDIO_TYPE_PCM;
			else if (strncasecmp(&p_file_path[len - 4], ".mp3", 4) == 0)
				audio_type = AUDIO_TYPE_MP3;
		}
		if (audio_type == AUDIO_TYPE_UNKNOWN)
		{
            sprintf(p_strerr, "[CAudioFile::add_file()] : '%s' unknown file type\n", p_file_path);
            throw 11;
		}
		// 文件数据检查
		p_audio = (audio_info_t *)ua_malloc(sizeof (audio_info_t));
		init_audio_info(p_audio);
		switch (audio_type)
		{
		case AUDIO_TYPE_PCM:
			b_add = get_wav_file_info(p_file_path, p_audio);
			break;
		case AUDIO_TYPE_MP3:
			b_add = get_mp3_file_info(p_file_path, p_audio);
			break;
		}
		if (b_add)
		{
			ua_mutex_lock(&m_mutex_play_list);
			m_list_play.AddTail(p_audio);
			ua_mutex_unlock(&m_mutex_play_list);
			p_audio = NULL;
		}
	}
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 11:
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "%s", p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[CAudioFile::add_file()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[CAudioFile::add_file()] : error catch!\n");
    }

	if (p_audio != NULL)
	{
		destroy_audio_info(p_audio);
		ua_free(p_audio);
	}

	return b_add;
}

// 获取WAV文件信息
BOOL CAudioFile::get_wav_file_info(const char *p_file_path, audio_info_t *p_audio)
{
	BOOL			b_get = FALSE;
	char			p_strerr[PATH_MAX] = {0};
	WAVEFILEINFO	file_info;
	WAVEFILEDATA	file_data;
	WAVEFORMATEX	file_format;
	FILE			*file;

	try
    {
		if (p_file_path == NULL || p_audio == NULL)
		{
            sprintf(p_strerr, "[CAudioFile::get_wav_file_info()] : parameter invalid\n");
            throw 10;
		}
		file = fopen(p_file_path, "rb");
		if (file == NULL)
		{
            sprintf(p_strerr, "[CAudioFile::get_wav_file_info(fopen:%s)](%d) : %s\n", p_file_path, errno, strerror(errno));
            throw 11;
		}
		memset(&file_format, 0, sizeof (WAVEFORMATEX));
		fseek(file, 0, SEEK_SET);
		fread(&file_info, 1, sizeof (WAVEFILEINFO), file);
		if (file_info.dwRIFFID != TAG_RIFF || file_info.dwFileType != TAG_WAVE)
		{
            sprintf(p_strerr, "[CAudioFile::get_wav_file_info()] : '%s' not a wav file\n", p_file_path);
            throw 12;
		}
		fread(&file_format, 1, file_info.dwFmtSize, file);
		if (file_format.wFormatTag != WAVE_FORMAT_PCM \
			|| file_format.wBitsPerSample != 16)
		{
            sprintf(p_strerr, "[CAudioFile::get_wav_file_info()] : '%s' unsupported format\n", p_file_path);
            throw 13;
		}
		fread(&file_data, 1, sizeof (WAVEFILEDATA), file);
		if (file_data.dwDATAID == TAG_FACT)
		{
			fseek(file, file_data.dwDATASize, SEEK_CUR);
			fread(&file_data, 1, sizeof (WAVEFILEDATA), file);
		}

		// 音频信息填写
		p_audio->audio_type = AUDIO_TYPE_PCM;
		p_audio->bitrate = file_format.nAvgBytesPerSec * 8;
		p_audio->sample_rate = file_format.nSamplesPerSec;
		p_audio->channels = file_format.nChannels;
		p_audio->data_offset = ftell(file);
		p_audio->data_size = file_data.dwDATASize;
		if (p_audio->file_path != NULL)
			ua_free(p_audio->file_path);
		p_audio->file_path = string_dup(p_file_path);
		
		b_get = TRUE;
	}
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 13:
        case 12:
        case 11:
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "%s", p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[CAudioFile::get_wav_file_info()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[CAudioFile::get_wav_file_info()] : error catch!\n");
    }
	if (file != NULL)
		fclose(file);

	return b_get;
}

// 获取MP3文件信息
BOOL CAudioFile::get_mp3_file_info(const char *p_file_path, audio_info_t *p_audio)
{
	BOOL			b_get = FALSE;
	char			p_strerr[PATH_MAX] = {0};
	int				frame_count, frame_match_count, bitrate_pre, bitrate_cur, bitrate_real;
	int				in_read, out_samples;
	int				decode_len, decode_total;
	short			*p_samples_left = NULL, *p_samples_right = NULL;
	BYTE			p_in_data[MAD_IN_LEN_MAX];
	FILE			*file;
	mad_decoder_t	mp3_decoder;

	try
    {
		mad_init(&mp3_decoder);		// 初始化
		if (p_file_path == NULL || p_audio == NULL)
		{
            sprintf(p_strerr, "[CAudioFile::get_mp3_file_info()] : parameter invalid\n");
            throw 10;
		}
		file = fopen(p_file_path, "rb");
		if (file == NULL)
		{
            sprintf(p_strerr, "[CAudioFile::get_mp3_file_info(fopen:%s)](%d) : %s\n", p_file_path, errno, strerror(errno));
            throw 11;
		}
		// 读取mp3数据流
		frame_count = frame_match_count = 0;
		bitrate_pre = bitrate_cur = bitrate_real = 0;
		in_read = fread(p_in_data, 1, MAD_IN_LEN_MAX, file);
		while (bitrate_real == 0 && in_read > 0)
		{
			decode_len = decode_total = 0;
			do
			{
				decode_len = mad_get_frame_size(&mp3_decoder);
				if (decode_len > MAD_IN_LEN_MAX)
					decode_len = MAD_IN_LEN_MAX;
				if (decode_len == 0)
					decode_len = MAD_IN_LEN_MAX / 4;
				if (decode_len > in_read - decode_total)
					decode_len = in_read - decode_total;
				decode_len = mad_decode(&mp3_decoder, &p_in_data[decode_total], decode_len);
				decode_total += decode_len;
				out_samples = mad_get_out_len(&mp3_decoder);
				if (out_samples > 0)
				{	// 解码出一帧
					out_samples = mad_get_out_buf(&mp3_decoder, &p_samples_left, &p_samples_right);	// 清空输出数据
					bitrate_cur = mp3_decoder.frame.header.bitrate;
					if (bitrate_cur == bitrate_pre)
					{
						frame_match_count++;
						if (frame_match_count >= 5)
							bitrate_real = bitrate_cur;
					}
					else
					{
						bitrate_pre = bitrate_cur;
						frame_match_count = 1;
					}
					frame_count++;
					if (frame_count >= 10 && bitrate_real == 0)	// 连续多帧帧率都在变化,有可能是变码流
						bitrate_real = -1;
				}
			} while (decode_total < in_read && decode_len > 0);

			in_read = fread(p_in_data, 1, MAD_IN_LEN_MAX, file);
		}
		if (bitrate_real <= 0)
		{
            sprintf(p_strerr, "[CAudioFile::get_mp3_file_info()] : %s is VBR(Variable Bit Rate)\n", p_file_path);
            throw 12;
		}

		// 音频信息填写
		p_audio->audio_type = AUDIO_TYPE_MP3;
		p_audio->bitrate = bitrate_real;
		p_audio->sample_rate = mad_get_samplerate(&mp3_decoder);
		p_audio->channels = mp3_decoder.synth.pcm.channels;
		p_audio->data_offset = 0;
		fseek(file, 0, SEEK_END);
		p_audio->data_size = ftell(file);
		if (p_audio->file_path != NULL)
			ua_free(p_audio->file_path);
		p_audio->file_path = string_dup(p_file_path);

		b_get = TRUE;
	}
    catch (int throw_err)
    {
        switch (throw_err)
        {
        case 11:
			close_file();
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "%s", p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[CAudioFile::open_file()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[CAudioFile::open_file()] : error catch!\n");
    }
	mad_destroy(&mp3_decoder);
	if (file != NULL)
		fclose(file);

	return b_get;
}

// 重设播放状态
void CAudioFile::reset_play(void)
{
	m_play_status = PLAY_STATUS_STOP;
	close_file();
	m_list_pos = 0;
}

// 读取一帧数据
int CAudioFile::read_frame_data(BYTE *p_buf, int max_len)
{
	int				read_len = 0;
	int				process_sum, process_cur, samples, out_size, error;
	BYTE			*p_data_in = NULL, *p_data_out = NULL, *p_data_temp = NULL;
	int				data_in_len, data_out_len;
	DWORD			data_buf_size = 48000 * 16 * max(m_out_msecond_per_frame, 26) / 1000 / 8;
	BE_ERR			be_err;
		
	if (m_play_status == PLAY_STATUS_PLAYING)
	{
		if (m_file == NULL)
			open_file();
		else if (feof(m_file) != 0)
			play_next();
		if (m_file != NULL)
		{
			ua_mutex_lock(&m_mutex_file);
			if (m_pout_buf == NULL)
			{	// 不用做二次处理
				out_size = m_out_bitrate * m_out_msecond_per_frame / 1000 / 8;
				if (out_size > max_len)
					out_size = max_len;
				read_len = file_data_read(p_buf, out_size);
				m_file_data_read += read_len;
				if (m_file_data_read > m_audio_info_playing.data_size)
				{	// WAV文件后面有一段无效数据
					if (read_len > m_file_data_read - m_audio_info_playing.data_size)
						read_len -= m_file_data_read - m_audio_info_playing.data_size;
					else
						read_len = 0;
				}
				if (m_pplay_event_callback != NULL && m_audio_info_playing.bitrate >= 8)
				{
					if ((m_file_data_read / (m_audio_info_playing.bitrate / 8)) != (m_file_data_progress / (m_audio_info_playing.bitrate / 8)))
					{
						m_pplay_event_callback(PLAY_EVENT_TYPE_PLAY_PROGRESS, m_file_data_read / (m_audio_info_playing.bitrate / 8), \
							m_audio_info_playing.data_size / (m_audio_info_playing.bitrate / 8), m_pevent_callback_param);
						m_file_data_progress = m_file_data_read;
					}
				}
			}
			else
			{	// 需要二次处理(MP3编码或MP3解码或重采样)
				p_data_in = (BYTE *)ua_malloc(data_buf_size);
				p_data_out = (BYTE *)ua_malloc(data_buf_size);
				out_size = m_out_bitrate * m_out_msecond_per_frame / 1000 / 8;
				if (out_size > max_len)
					out_size = max_len;
				while (m_out_buf_len < out_size)//m_out_buf_len < out_size
				{
					data_in_len = m_audio_info_playing.bitrate * m_out_msecond_per_frame / 1000 / 8;
					if (data_in_len > out_size)
						data_in_len = out_size;
					data_in_len = file_data_read(p_data_in, data_in_len);
					m_file_data_read += data_in_len;
					if (m_file_data_read > m_audio_info_playing.data_size)
					{	// WAV文件后面有一段无效数据
						if (read_len > m_file_data_read - m_audio_info_playing.data_size)
							read_len -= m_file_data_read - m_audio_info_playing.data_size;
						else
							read_len = 0;
					}
					if (m_pplay_event_callback != NULL && m_audio_info_playing.bitrate >= 8)
					{
						if ((m_file_data_read / (m_audio_info_playing.bitrate / 8)) != (m_file_data_progress / (m_audio_info_playing.bitrate / 8)))
						{
							m_pplay_event_callback(PLAY_EVENT_TYPE_PLAY_PROGRESS, m_file_data_read / (m_audio_info_playing.bitrate / 8), \
								m_audio_info_playing.data_size / (m_audio_info_playing.bitrate / 8), m_pevent_callback_param);
							m_file_data_progress = m_file_data_read;
						}
					}
					if (data_in_len > 0)
					{
						if (m_bmp3_decode)
						{	// MP3解码
							data_out_len = 0;
							process_sum = 0;
							do
							{
								process_cur = mad_get_frame_size(&m_mp3_decoder);
								if (process_cur > MAD_IN_LEN_MAX)
									process_cur = MAD_IN_LEN_MAX;
								if (process_cur == 0)
									process_cur = MAD_IN_LEN_MAX / 4;
								if (process_cur > data_in_len - process_sum)
									process_cur = data_in_len - process_sum;
								process_cur = mad_decode(&m_mp3_decoder, &p_data_in[process_sum], process_cur);
								process_sum += process_cur;
								samples = mad_get_out_len(&m_mp3_decoder);
								if (samples > 0)	// 解码出一帧
								{
									samples = mad_get_out_data(&m_mp3_decoder, (short *)&p_data_out[data_out_len], NULL, (data_buf_size - data_out_len) / sizeof (short));
									data_out_len += samples * sizeof (short);
								}
							} while (process_sum < data_in_len && process_cur > 0);
							p_data_temp = p_data_in;
							p_data_in = p_data_out;
							p_data_out = p_data_temp;
							data_in_len = data_out_len;
							data_out_len = 0;
						} 
						else if (m_bmp3_encode)
						{	// MP3编码
							data_out_len = 0;
							be_err = beEncodeChunk(m_mp3_encoder, data_in_len / sizeof (short), (PSHORT)p_data_in, p_data_out, (DWORD *)&data_out_len);
							if (be_err != BE_ERR_SUCCESSFUL)
								printf_log(LOG_IS_DEBUG, "[CAudioFile::read_frame_data(beEncodeChunk)](%d) : faild\n", be_err);
							p_data_temp = p_data_in;
							p_data_in = p_data_out;
							p_data_out = p_data_temp;
							data_in_len = data_out_len;
							data_out_len = 0;
						}
						if (data_in_len > (int)data_buf_size)
							printf_log(LOG_IS_ERR, "[CAudioFile::read_frame_data()] : data_buf overflow(%d/%d)\n", data_in_len, data_buf_size);
						if (m_bresample)
						{	// 重采样
							data_out_len = 0;
							process_sum = 0;
							do
							{
								process_cur = (data_in_len - process_sum) / sizeof (short);
								samples = data_buf_size  / sizeof (short);
								error = ResampleProcessInt(m_presampler, 0, (spx_int16_t *)&p_data_in[process_sum], (spx_uint32_t *)&process_cur, \
									(spx_int16_t *)&p_data_out[data_out_len], (spx_uint32_t *)&samples);
								if (error != RESAMPLER_ERR_SUCCESS)
								{
									printf_log(LOG_IS_DEBUG, "[CAudioFile::read_frame_data(ResampleProcessInt)](%d) : faild\n", error);
									break;
								}
								process_sum += process_cur * sizeof (short);
								data_out_len += samples * sizeof (short);
							} while (process_sum < data_in_len);
							p_data_temp = p_data_in;
							p_data_in = p_data_out;
							p_data_out = p_data_temp;
							data_in_len = data_out_len;
							data_out_len = 0;
						}
						if (data_in_len > (int)data_buf_size)
							printf_log(LOG_IS_ERR, "[CAudioFile::read_frame_data()] : data_buf overflow(%d/%d)\n", data_in_len, data_buf_size);
						if (data_in_len > 0)
						{
							if (m_out_buf_len + data_in_len <= m_out_buf_size)
							{
								memcpy(&m_pout_buf[m_out_buf_len], p_data_in, data_in_len);
								m_out_buf_len += data_in_len;
							}
							else
								printf_log(LOG_IS_ERR, "[CAudioFile::read_frame_data()] : m_pout_buf overflow(%d/%d)\n", m_out_buf_len + data_in_len, m_out_buf_size);
						}
					}
					else
						break;
				}
				// 是否有缓冲数据
				if (m_out_buf_len >= out_size)
				{
					memcpy(p_buf, m_pout_buf, out_size);
					m_out_buf_len -= out_size;
					if (m_out_buf_len > 0)
						memcpy(m_pout_buf, m_pout_buf + out_size, m_out_buf_len);
					read_len = out_size;
				}
			}
			ua_mutex_unlock(&m_mutex_file);
		}
	}
	else if (m_play_status == PLAY_STATUS_PAUSE)
	{
		memset(p_buf, 0, max_len);
		read_len = max_len;
	}

	if (p_data_in != NULL)
		ua_free(p_data_in);
	if (p_data_out != NULL)
		ua_free(p_data_out);

	return read_len;
}

// 初始化音频信息
void CAudioFile::init_audio_info(audio_info_t *p_audio)
{
	if (p_audio != NULL)
		memset(p_audio, 0, sizeof (audio_info_t));
}

// 销毁音频信息
void CAudioFile::destroy_audio_info(audio_info_t *p_audio)
{
	if (p_audio != NULL)
	{
		if (p_audio->file_path != NULL)
		{
			ua_free(p_audio->file_path);
			p_audio->file_path = NULL;
		}
		memset(p_audio, 0, sizeof (audio_info_t));
	}
}

// 销毁播放列表
void CAudioFile::destroy_play_list(void)
{
	audio_info_t		*p_audio = NULL;

	ua_mutex_lock(&m_mutex_play_list);
	while (m_list_play.GetCount() > 0)
	{
		m_list_play.RemoveHead(p_audio);
		destroy_audio_info(p_audio);
		ua_free(p_audio);
		p_audio = NULL;
	}
	ua_mutex_unlock(&m_mutex_play_list);
	m_list_pos = -1;
}

// 设置播放模式
void CAudioFile::set_play_mode(int play_mode)
{
	switch (play_mode)
	{
	case PLAY_MODE_RANDOM_PLAY:
	case PLAY_MODE_ALL_LOOP:
	case PLAY_MODE_ALL_PLAY:
	case PLAY_MODE_SINGLE_LOOP:
		m_play_mode = play_mode;
		break;
	case PLAY_MODE_SINGLE_PLAY:
	default:
		m_play_mode = PLAY_MODE_SINGLE_PLAY;
		break;
	}
}

// 获取播放模式
int CAudioFile::get_play_mode(void)
{
	return m_play_mode;
}

// 获取输出格式
void CAudioFile::set_out_format(int audio_type, int sample_rate, int msecond_per_frame)
{
	m_out_audio_type = audio_type;
	m_out_sample_rate = sample_rate;
	m_out_msecond_per_frame = msecond_per_frame;
}

// 获取输出格式
void CAudioFile::get_out_format(int *p_audio_type, int *p_sample_rate, int *p_msecond_per_frame)
{
	if (p_audio_type != NULL)
		*p_audio_type = m_out_audio_type;
	if (p_sample_rate != NULL)
		*p_sample_rate = m_out_sample_rate;
	if (p_msecond_per_frame != NULL)
		*p_msecond_per_frame = m_out_msecond_per_frame;
}

// 清除播放列表
void CAudioFile::remove_all_file(void)
{
	destroy_play_list();
}

// 从播放列表删除文件
BOOL CAudioFile::remove_file(const char *p_file_path)
{
	BOOL				b_remove = FALSE;
	audio_info_t		*p_audio = NULL;
	int					i, count;

	if (p_file_path != NULL)
	{
		ua_mutex_lock(&m_mutex_play_list);
		count = m_list_play.GetCount();
		for (i = 0; !b_remove && i < count; i++);
		{
			if (m_list_play.GetElem(i + 1, p_audio) && p_audio->file_path != NULL)
			{
				if (strcasecmp(p_audio->file_path, p_file_path) == 0)
				{
					if (m_list_play.Remove(i + 1, p_audio))
					{
						destroy_audio_info(p_audio);
						ua_free(p_audio);
						p_audio = NULL;
						b_remove = TRUE;
						if (m_list_pos >= i && m_list_pos > 0)
							m_list_pos--;
					}
				}
			}
		}
		ua_mutex_unlock(&m_mutex_play_list);
	}

	return b_remove;
}

// 初始化mp3编码器
BOOL CAudioFile::init_mp3_encoder(HBE_STREAM *p_mp3_encoder, int sample_rate, int channels, int bitrate, DWORD *p_mp3_encode_samples, DWORD *p_mp3_encode_out_size)
{
	BOOL			b_init = FALSE;
	BE_CONFIG		be_config = {0,};
	DWORD			samples = 0;

	if (*p_mp3_encoder != 0)
	{
		beCloseStream(*p_mp3_encoder);
		*p_mp3_encoder = 0;
	}

	be_config.dwConfig						= BE_CONFIG_LAME;		// use the LAME config structure
	be_config.format.LHV1.dwStructVersion	= 1;
	be_config.format.LHV1.dwStructSize		= sizeof(BE_CONFIG);		
	be_config.format.LHV1.dwSampleRate		= sample_rate;			// INPUT FREQUENCY
	be_config.format.LHV1.dwReSampleRate	= 0;					// DON"T RESAMPLE
	if (channels == 1)
		be_config.format.LHV1.nMode			= BE_MP3_MODE_MONO;		// OUTPUT IN STREO
	else
		be_config.format.LHV1.nMode			= BE_MP3_MODE_STEREO;	// OUTPUT IN STREO
	be_config.format.LHV1.dwBitrate			= bitrate / 1000;	// MINIMUM BIT RATE
	be_config.format.LHV1.nPreset			= LQP_R3MIX;			// QUALITY PRESET SETTING
	be_config.format.LHV1.dwMpegVersion		= MPEG1;				// MPEG VERSION (I or II)
	be_config.format.LHV1.dwPsyModel		= 0;					// USE DEFAULT PSYCHOACOUSTIC MODEL 
	be_config.format.LHV1.dwEmphasis		= 0;					// NO EMPHASIS TURNED ON
	be_config.format.LHV1.bOriginal			= TRUE;					// SET ORIGINAL FLAG
	be_config.format.LHV1.bWriteVBRHeader	= FALSE;				// Write INFO tag
	be_config.format.LHV1.bNoRes			= TRUE;					// No Bit resorvoir

	if (beInitStream(&be_config, p_mp3_encode_samples, p_mp3_encode_out_size, p_mp3_encoder) == BE_ERR_SUCCESSFUL)
		b_init = TRUE;

	return b_init;
}

// 设置播放事件回调函数
void CAudioFile::set_play_event_callback(void(*p_callback)(int, DWORD, DWORD, void *), void *p_param)
{
	m_pplay_event_callback = p_callback;
	m_pevent_callback_param = p_param;
}

// 读取文件数据(加快文件读写速度,每次读取AUDIO_FILE_BUF_SIZE)
int CAudioFile::file_data_read(BYTE *p_buf, int max_len)
{
	int		read = 0, cur_len = 0;

	if (m_file != NULL)
	{
		do
		{
			if (m_data_size > 0)
			{
				if (m_data_size > (max_len - read))
					cur_len = max_len - read;
				else
					cur_len = m_data_size;
				if (p_buf != NULL)
					memcpy(&p_buf[read], &m_data_buf[m_data_read], cur_len);
				read += cur_len;
				m_data_size -= cur_len;
				m_data_read += cur_len;
			}
			if (m_data_size == 0)
			{
				m_data_size = fread(m_data_buf, 1, AUDIO_FILE_BUF_SIZE, m_file);
				m_data_read = 0;
			}
		} while (m_data_size > 0 && read < max_len);
	}

	return read;
}

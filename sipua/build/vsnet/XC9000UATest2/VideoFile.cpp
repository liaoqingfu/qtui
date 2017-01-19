/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : VideoFile.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2017/01/05
* Description  : 视频文件类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/

#include "stdafx.h"

#include "ua_port.h"
#include "ua_global.h"
#include <time.h>

#include "VideoFile.h"
#include "log.h"
#include "defines.h"

CVideoFile::CVideoFile()
{
	m_play_mode = VIDEO_PLAY_MODE_ALL_LOOP;
	m_out_msecond_per_frame = 20;
	m_out_bitrate = 0;
	init_video_info(&m_video_info_playing);
	ua_mutex_init(&m_mutex_file);
	m_file = NULL;
	m_data_read = m_data_size = 0;
	m_file_data_read = m_file_data_progress = 0;
	ua_mutex_init(&m_mutex_play_list);
	m_list_pos = -1;
	m_play_status = VIDEO_PLAY_STATUS_STOP;
	m_pplay_event_callback = NULL;
	m_pevent_callback_param = NULL;
	m_h264_start_len = 0;
}

CVideoFile::~CVideoFile()
{
	close_file();
	destroy_play_list();
	ua_mutex_destroy(&m_mutex_play_list);
	ua_mutex_destroy(&m_mutex_file);
}

// 关闭文件
void CVideoFile::close_file(void)
{
	ua_mutex_lock(&m_mutex_file);
	if (m_file != NULL)
	{
		fclose(m_file);
		m_file = NULL;
	}
	ua_mutex_unlock(&m_mutex_file);
	destroy_video_info(&m_video_info_playing);
	m_file_data_read = m_file_data_progress = 0;
	m_data_read = m_data_size = 0;
	m_h264_start_len = 0;
}

// 打开文件
BOOL CVideoFile::open_file(void)
{
	BOOL			b_open = FALSE;
	video_info_t	*p_audio = NULL;
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
				if (p_audio->data_offset > 0)
					file_data_read(NULL, p_audio->data_offset);
				dup_video_info(&m_video_info_playing, p_audio);
				b_open = TRUE;

				m_file_data_read = m_file_data_progress = 0;
				if (m_pplay_event_callback != NULL && p_audio->bitrate >= 8)
					m_pplay_event_callback(VIDEO_PLAY_EVENT_OPEN_FILE, m_list_pos, p_audio->data_size / (p_audio->bitrate / 8), m_pevent_callback_param);
			}
			ua_mutex_unlock(&m_mutex_file);
		}
	}

	return b_open;
}

// 复制视频信息
BOOL CVideoFile::dup_video_info(video_info_t *p_video_dst, video_info_t *p_video_src)
{
	BOOL			b_dup = FALSE;

	if (p_video_dst != NULL && p_video_src != NULL)
	{
		destroy_video_info(p_video_dst);
		*p_video_dst = *p_video_src;
		if (p_video_src->file_path != NULL)
			p_video_dst->file_path = string_dup(p_video_src->file_path);
		b_dup = TRUE;
	}

	return b_dup;
}


// 开始播放(返回文件序号,0起始)
int CVideoFile::play(void)
{
	m_play_status = VIDEO_PLAY_STATUS_PLAYING;
	return m_list_pos;
}

// 停止播放(返回文件序号,0起始)
int CVideoFile::stop(void)
{
	m_play_status = VIDEO_PLAY_STATUS_STOP;
	close_file();
	return m_list_pos;
}

// 暂停播放(返回文件序号,0起始)
int CVideoFile::pause(void)
{
	m_play_status = VIDEO_PLAY_STATUS_PAUSE;
	return m_list_pos;
}

// 播放前一个文件(返回文件序号,0起始)
int CVideoFile::previous(void)
{
	if (m_list_pos > 0)
	{
		m_list_pos -= 1;
		open_file();
	}
	return m_list_pos;
}

// 播放下一个文件(返回文件序号,0起始)
int CVideoFile::next(void)
{
	if (m_list_pos + 1 < m_list_play.GetCount())
	{
		m_list_pos += 1;
		open_file();
	}
	return m_list_pos;
}

// 设置播放文件的序号(0起始)
int CVideoFile::set_play_index(int index)
{
	if (index >= 0 && index < m_list_play.GetCount())
	{
		m_list_pos = index;
		open_file();
	}
	return m_list_pos;
}

// 根据播放模式播放下一个文件(返回文件序号,0起始)
int CVideoFile::play_next(void)
{
	int				count = m_list_play.GetCount();
	time_t			now;

	if (count > 0)
	{
		switch (m_play_mode)
		{
		case VIDEO_PLAY_MODE_SINGLE_PLAY:
			m_list_pos = -1;
			break;
		case VIDEO_PLAY_MODE_SINGLE_LOOP:
			break;
		case VIDEO_PLAY_MODE_ALL_PLAY:
			m_list_pos++;
			if (m_list_pos >= m_list_play.GetCount())
				m_list_pos = -1;
			break;
		case VIDEO_PLAY_MODE_ALL_LOOP:
			m_list_pos++;
			if (m_list_pos >= m_list_play.GetCount())
				m_list_pos = 0;
			break;
		case VIDEO_PLAY_MODE_RANDOM_PLAY:
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

BOOL CVideoFile::add_file(const char *p_file_path)
{
	BOOL			b_add = FALSE;
	char			p_strerr[PATH_MAX] = {0};
	int				len;
	video_info_t	*p_audio = NULL;
	int				video_type = VIDEO_TYPE_UNKNOWN;

	try
    {
		// 入口参数检查
		if (p_file_path != NULL && strlen(p_file_path) == 0)
		{
            sprintf(p_strerr, "[CVideoFile::add_file()] : parameter invalid\n");
            throw 10;
		}
		len = strlen(p_file_path);
		if (len > 4)
		{
			if (strncasecmp(&p_file_path[len - 4], ".264", 4) == 0)
				video_type = VIDEO_TYPE_H264;
		}
		if (video_type == VIDEO_TYPE_UNKNOWN)
		{
            sprintf(p_strerr, "[CVideoFile::add_file()] : '%s' unknown file type\n", p_file_path);
            throw 11;
		}
		// 文件数据检查
		p_audio = (video_info_t *)ua_malloc(sizeof (video_info_t));
		init_video_info(p_audio);
		switch (video_type)
		{
		case VIDEO_TYPE_H264:
			b_add = get_h264_file_info(p_file_path, p_audio);
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
        case 10:
        default:
            break;
        }
        if (strlen(p_strerr) > 0)
            printf_log(LOG_IS_ERR, "%s", p_strerr);
    }
    catch (std::exception &e)
    {   
        printf_log(LOG_IS_ERR, "[CVideoFile::add_file()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[CVideoFile::add_file()] : error catch!\n");
    }

	if (p_audio != NULL)
	{
		destroy_video_info(p_audio);
		ua_free(p_audio);
	}

	return b_add;
}

// 获取H264文件信息
BOOL CVideoFile::get_h264_file_info(const char *p_file_path, video_info_t *p_audio)
{
	BOOL			b_get = FALSE;
	char			p_strerr[PATH_MAX] = {0};
	FILE			*file;

	try
    {
		if (p_file_path == NULL || p_audio == NULL)
		{
            sprintf(p_strerr, "[CVideoFile::get_h264_file_info()] : parameter invalid\n");
            throw 10;
		}
		file = fopen(p_file_path, "rb");
		if (file == NULL)
		{
            sprintf(p_strerr, "[CVideoFile::get_h264_file_info(fopen:%s)](%d) : %s\n", p_file_path, errno, strerror(errno));
            throw 11;
		}

		// 视频信息填写
		p_audio->video_type = VIDEO_TYPE_H264;
		p_audio->bitrate = 0;
		p_audio->sample_rate = 0;
		p_audio->channels = 0;
		p_audio->data_offset = 0;
		fseek(file, 0, SEEK_END);
		p_audio->data_size = ftell(file);
		fseek(file, 0, SEEK_CUR);
		if (p_audio->file_path != NULL)
			ua_free(p_audio->file_path);
		p_audio->file_path = string_dup(p_file_path);
		
		b_get = TRUE;
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
        printf_log(LOG_IS_ERR, "[CVideoFile::get_h264_file_info()] : %s", e.what());
    }
    catch (...)
    {   
        printf_log(LOG_IS_ERR, "[CVideoFile::get_h264_file_info()] : error catch!\n");
    }
	if (file != NULL)
		fclose(file);

	return b_get;
}

// 重设播放状态
void CVideoFile::reset_play(void)
{
	m_play_status = VIDEO_PLAY_STATUS_STOP;
	close_file();
	m_list_pos = 0;
}

// 读取一帧数据
int CVideoFile::read_frame_data(BYTE *p_buf, int max_len, BYTE *p_nalu)
{
	int				read_len = 0, read_cur;
	BOOL			b_start_code;
		
	if (m_play_status == VIDEO_PLAY_STATUS_PLAYING)
	{
		if (m_file == NULL)
			open_file();
		else if (feof(m_file) != 0)
			play_next();
		if (m_file != NULL && max_len > 4)
		{
			ua_mutex_lock(&m_mutex_file);

			b_start_code = FALSE;
			if (m_h264_start_len > 0)
			{
				p_buf[read_len++] = 0x00;
				p_buf[read_len++] = 0x00;
				if (m_h264_start_len == 4)
					p_buf[read_len++] = 0x00;
				p_buf[read_len++] = 0x01;
			}
			do
			{
				read_cur = file_data_read(&p_buf[read_len], 1);
				m_file_data_read += read_cur;
				read_len += read_cur;
				if (read_len >= 3 && p_buf[read_len - 3] == 0x00 && p_buf[read_len - 2] == 0x00)
				{
					if (p_buf[read_len - 1] == 0x01)
					{
						b_start_code = TRUE;
						read_len -= 3;
						if (p_nalu != NULL && m_h264_start_len > 0)
							*p_nalu = p_buf[m_h264_start_len];
						m_h264_start_len = 3;
					}
					else if (p_buf[read_len - 1] == 0x00)
					{
						read_cur = file_data_read(&p_buf[read_len], 1);
						m_file_data_read += read_cur;
						read_len += read_cur;
						if (read_cur == 1 && p_buf[read_len - 1] == 0x01)
						{
							b_start_code = TRUE;
							read_len -= 4;
							if (p_nalu != NULL && m_h264_start_len > 0)
								*p_nalu = p_buf[m_h264_start_len];
							m_h264_start_len = 4;
						}
					}
				}
			} while (!b_start_code && read_len < 20480 && read_len < max_len && read_cur > 0);

			if (m_pplay_event_callback != NULL && m_video_info_playing.bitrate >= 8)
			{
				if ((m_file_data_read / (m_video_info_playing.bitrate / 8)) != (m_file_data_progress / (m_video_info_playing.bitrate / 8)))
				{
					m_pplay_event_callback(VIDEO_PLAY_EVENT_PLAY_PROGRESS, m_file_data_read / (m_video_info_playing.bitrate / 8), \
						m_video_info_playing.data_size / (m_video_info_playing.bitrate / 8), m_pevent_callback_param);
					m_file_data_progress = m_file_data_read;
				}
			}

			ua_mutex_unlock(&m_mutex_file);
		}
	}
	else if (m_play_status == VIDEO_PLAY_STATUS_PAUSE)
	{
		memset(p_buf, 0, max_len);
		read_len = max_len;
	}

	return read_len;
}

// 初始化视频信息
void CVideoFile::init_video_info(video_info_t *p_audio)
{
	if (p_audio != NULL)
		memset(p_audio, 0, sizeof (video_info_t));
}

// 销毁视频信息
void CVideoFile::destroy_video_info(video_info_t *p_audio)
{
	if (p_audio != NULL)
	{
		if (p_audio->file_path != NULL)
		{
			ua_free(p_audio->file_path);
			p_audio->file_path = NULL;
		}
		memset(p_audio, 0, sizeof (video_info_t));
	}
}

// 销毁播放列表
void CVideoFile::destroy_play_list(void)
{
	video_info_t		*p_audio = NULL;

	ua_mutex_lock(&m_mutex_play_list);
	while (m_list_play.GetCount() > 0)
	{
		m_list_play.RemoveHead(p_audio);
		destroy_video_info(p_audio);
		ua_free(p_audio);
		p_audio = NULL;
	}
	ua_mutex_unlock(&m_mutex_play_list);
	m_list_pos = -1;
}

// 设置播放模式
void CVideoFile::set_play_mode(int play_mode)
{
	switch (play_mode)
	{
	case VIDEO_PLAY_MODE_RANDOM_PLAY:
	case VIDEO_PLAY_MODE_ALL_LOOP:
	case VIDEO_PLAY_MODE_ALL_PLAY:
	case VIDEO_PLAY_MODE_SINGLE_LOOP:
		m_play_mode = play_mode;
		break;
	case VIDEO_PLAY_MODE_SINGLE_PLAY:
	default:
		m_play_mode = VIDEO_PLAY_MODE_SINGLE_PLAY;
		break;
	}
}

// 获取播放模式
int CVideoFile::get_play_mode(void)
{
	return m_play_mode;
}

// 获取输出格式
void CVideoFile::get_out_format(int *p_video_type, int *p_sample_rate, int *p_msecond_per_frame)
{
	if (p_video_type != NULL)
		*p_video_type = 0;
	if (p_sample_rate != NULL)
		*p_sample_rate = 0;
	if (p_msecond_per_frame != NULL)
		*p_msecond_per_frame = m_out_msecond_per_frame;
}

// 清除播放列表
void CVideoFile::remove_all_file(void)
{
	destroy_play_list();
}

// 从播放列表删除文件
BOOL CVideoFile::remove_file(const char *p_file_path)
{
	BOOL				b_remove = FALSE;
	video_info_t		*p_audio = NULL;
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
						destroy_video_info(p_audio);
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

// 设置播放事件回调函数
void CVideoFile::set_play_event_callback(void(*p_callback)(int, DWORD, DWORD, void *), void *p_param)
{
	m_pplay_event_callback = p_callback;
	m_pevent_callback_param = p_param;
}

// 读取文件数据(加快文件读写速度,每次读取VIDEO_FILE_BUF_SIZE)
inline int CVideoFile::file_data_read(BYTE *p_buf, int max_len)
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
				m_data_size = fread(m_data_buf, 1, VIDEO_FILE_BUF_SIZE, m_file);
				m_data_read = 0;
			}
		} while (m_data_size > 0 && read < max_len);
	}

	return read;
}

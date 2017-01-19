/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : VideoFile.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2017/01/05
* Description  : 视频文件类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __VIDEOFILE_H__
#define __VIDEOFILE_H__

#include <fstream>
using namespace std; 

#include "types.h"
#include "LIST.H"

#define VIDEO_PLAY_MODE_SINGLE_PLAY		0	// 单曲播放
#define VIDEO_PLAY_MODE_SINGLE_LOOP		1	// 单曲循环
#define VIDEO_PLAY_MODE_ALL_PLAY		2	// 顺序播放
#define VIDEO_PLAY_MODE_ALL_LOOP		3	// 循环播放
#define VIDEO_PLAY_MODE_RANDOM_PLAY		4	// 随机播放

#define VIDEO_PLAY_STATUS_PLAYING		0	// 播放
#define VIDEO_PLAY_STATUS_PAUSE			1	// 暂停
#define VIDEO_PLAY_STATUS_STOP			2	// 停止

#define VIDEO_TYPE_UNKNOWN			0
#define VIDEO_TYPE_H264				1

#define VIDEO_FILE_BUF_SIZE			(1024 * 80)

typedef struct video_info
{
	int			video_type;			// 视频类型(PCM, MP3)
	int			bitrate;			// 位流
	int			sample_rate;		// 采样率
	int			channels;			// 1 : 单声道; 2 : 双声道;
	int			data_offset;		// WAV文件的数据
	long		data_size;			// 视频数据大小(字节数)
	char		*file_path;			// 文件路径
} video_info_t;

#define VIDEO_PLAY_EVENT_OPEN_FILE		0	// 打开文件
#define VIDEO_PLAY_EVENT_PLAY_PROGRESS	1	// 播放进度

class CVideoFile
{
public:
	CVideoFile();
	virtual ~CVideoFile();

	// 配置
	void set_play_mode(int play_mode);									// 设置播放模式
	int get_play_mode(void);											// 获取播放模式
	// 播放列表控制
	BOOL add_file(const char *p_file_path);								// 添加文件到播放列表
	void remove_all_file(void);											// 清除播放列表
	BOOL remove_file(const char *p_file_path);							// 从播放列表删除文件
	// 播放状态控制
	void reset_play(void);												// 重设播放状态
	int play(void);														// 开始播放(返回文件序号,0起始)
	int stop(void);														// 停止播放(返回文件序号,0起始)
	int pause(void);													// 暂停播放(返回文件序号,0起始)
	int previous(void);													// 播放前一个文件(返回文件序号,0起始)
	int next(void);														// 播放下一个文件(返回文件序号,0起始)
	int set_play_index(int index);										// 设置播放文件的序号(0起始)
	int get_play_status(void);											// 获取播放状态
	// 配置,读取数据
	int read_frame_data(BYTE *p_buf, int max_len, BYTE *p_nalu);		// 读取一帧数据

private:
	// 播放状态控制
    void set_play_event_callback(void(*p_callback)(int, DWORD, DWORD, void *), void *p_param);// 设置播放事件回调函数
	// 播放列表
	void destroy_play_list(void);										// 销毁播放列表
	BOOL get_h264_file_info(const char *p_file_path, video_info_t *p_audio);	// 获取H264文件信息
	BOOL open_file(void);												// 打开文件
	void close_file(void);												// 关闭文件
	int play_next(void);												// 根据播放模式播放下一个文件(返回文件序号,0起始)
	// 视频信息
	void init_video_info(video_info_t *p_audio);						// 初始化视频信息
	void destroy_video_info(video_info_t *p_audio);						// 销毁视频信息
	BOOL dup_video_info(video_info_t *p_video_dst, video_info_t *p_video_src);	// 复制视频信息
	// 配置,读取数据
	void get_out_format(int *p_video_type, int *p_sample_rate, int *p_msecond_per_frame);// 获取输出格式
	inline int file_data_read(BYTE *p_buf, int max_len);				// 读取文件数据(加快文件读写速度,每次读取VIDEO_FILE_BUF_SIZE)

public:

private:
	FILE					*m_file;				// 当前正在播放的文件
	BYTE					m_data_buf[VIDEO_FILE_BUF_SIZE];
	int						m_data_read;			// m_data_buf的读取位置
	int						m_data_size;			// m_data_buf的数据大小
	long					m_file_data_read;		// 已读取文件数据大小
	long					m_file_data_progress;	// 已读取文件数据进度
	int						m_h264_start_len;		// 开始标志长度
	ua_mutex_t				m_mutex_file;			// 文件指针互斥锁
	List<video_info_t *>	m_list_play;			// 播放列表
	ua_mutex_t				m_mutex_play_list;		// m_list_play的互斥锁,所有公共函数操作,都需要加锁
	int						m_list_pos;				// 当前播放视频在清单的位置(0起始)
	video_info_t			m_video_info_playing;	// 当前播放的文件信息
	BOOL					m_play_status;			// 播放状态
	void					(*m_pplay_event_callback)(int, DWORD, DWORD, void *);
	void					*m_pevent_callback_param;

	// 视频输入
	int						m_play_mode;			// 播放模式
	int						m_out_msecond_per_frame;// 输出每帧毫秒数
	int						m_out_bitrate;			// 输出位率
};

// 获取播放状态
inline int CVideoFile::get_play_status(void)
{
	return m_play_status;
}

#endif // __VIDEOFILE_H__

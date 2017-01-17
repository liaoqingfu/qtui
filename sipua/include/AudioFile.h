/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : AudioFile.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : 音频文件类程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __AUDIOFILE_H__
#define __AUDIOFILE_H__

#include <fstream>
using namespace std; 

#include "types.h"
#include "LIST.H"
#include "libmad/libmad_face.h"
#include "liblame/liblame_face.h"
#include "libresample/resample_face.h"

#define TAG_RIFF			0x46464952
#define TAG_WAVE			0x45564157 
#define TAG_FMT				0x20746D66
#define TAG_FACT			0x74636166
#define TAG_DATA			0x61746164

#define WAVE_FORMAT_PCM     1

typedef struct tagWAVEFILEINFO {
	DWORD dwRIFFID;		//"RIFF"
	DWORD dwRIFFSize;   //文件总长度 - 8
	DWORD dwFileType;	//"WAVE"
	DWORD dwFmtID;		//"fmt "
	DWORD dwFmtSize;	//格式信息长度
}WAVEFILEINFO;

typedef struct tagWAVEFILEFACT {
	DWORD dwFACTID;		//"fact"
	DWORD dwFACTSize;   //fact区长度
}WAVEFILEFACT;

typedef struct tagWAVEFILEDATA {
	DWORD dwDATAID;		//"DATA"
	DWORD dwDATASize;   //数据区长度
}WAVEFILEDATA;

#if !defined(WIN32)
typedef struct tWAVEFORMATEX
{
    WORD        wFormatTag;         /* format type */
    WORD        nChannels;          /* number of channels (i.e. mono, stereo...) */
    DWORD       nSamplesPerSec;     /* sample rate */
    DWORD       nAvgBytesPerSec;    /* for buffer estimation */
    WORD        nBlockAlign;        /* block size of data */
    WORD        wBitsPerSample;     /* number of bits per sample of mono data */
    WORD        cbSize;             /* the count in bytes of the size of */
                                    /* extra information (after cbSize) */
} WAVEFORMATEX, *PWAVEFORMATEX;
#endif

#define PLAY_MODE_SINGLE_PLAY		0	// 单曲播放
#define PLAY_MODE_SINGLE_LOOP		1	// 单曲循环
#define PLAY_MODE_ALL_PLAY			2	// 顺序播放
#define PLAY_MODE_ALL_LOOP			3	// 循环播放
#define PLAY_MODE_RANDOM_PLAY		4	// 随机播放

#define PLAY_STATUS_PLAYING			0	// 播放
#define PLAY_STATUS_PAUSE			1	// 暂停
#define PLAY_STATUS_STOP			2	// 停止

#define AUDIO_TYPE_UNKNOWN			0
#define AUDIO_TYPE_PCM				1
#define AUDIO_TYPE_MP3				2

#define MP3_ENCODER_BITRATE			128000

#define AUDIO_FILE_BUF_SIZE			(1024 * 8)

typedef struct audio_info
{
	int			audio_type;			// 音频类型(PCM, MP3)
	int			bitrate;			// 位流
	int			sample_rate;		// 采样率
	int			channels;			// 1 : 单声道; 2 : 双声道;
	int			data_offset;		// WAV文件的数据
	long		data_size;			// 音频数据大小(字节数)
	char		*file_path;			// 文件路径
} audio_info_t;

#define PLAY_EVENT_TYPE_OPEN_FILE		0	// 打开文件
#define PLAY_EVENT_TYPE_PLAY_PROGRESS	1	// 播放进度

class CAudioStream;
class CSipUA;
class CAudioFile
{
public:
	CAudioFile();
	virtual ~CAudioFile();

	friend class CAudioStream;
	friend class CSipUA;

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

private:
	// 播放状态控制
    void set_play_event_callback(void(*p_callback)(int, DWORD, DWORD, void *), void *p_param);// 设置播放事件回调函数
	// 播放列表
	void destroy_play_list(void);										// 销毁播放列表
	BOOL get_wav_file_info(const char *p_file_path, audio_info_t *p_audio);	// 获取WAV文件信息
	BOOL get_mp3_file_info(const char *p_file_path, audio_info_t *p_audio);	// 获取MP3文件信息
	BOOL open_file(void);												// 打开文件
	void close_file(void);												// 关闭文件
	BOOL init_mp3_encoder(HBE_STREAM *p_mp3_encoder, int sample_rate, int channels, int bitrate, \
				DWORD *p_mp3_encode_samples, DWORD *p_mp3_encode_out_size);	// 初始化mp3编码器
	int play_next(void);												// 根据播放模式播放下一个文件(返回文件序号,0起始)
	// 音频信息
	void init_audio_info(audio_info_t *p_audio);						// 初始化音频信息
	void destroy_audio_info(audio_info_t *p_audio);						// 销毁音频信息
	BOOL dup_audio_info(audio_info_t *p_audio_dst, audio_info_t *p_audio_src);	// 复制音频信息
	// 配置,读取数据
	void set_out_format(int audio_type, int sample_rate, int msecond_per_frame);// 设置输出格式
	void get_out_format(int *p_audio_type, int *p_sample_rate, int *p_msecond_per_frame);// 获取输出格式
	int read_frame_data(BYTE *p_buf, int max_len);						// 读取一帧数据
	int file_data_read(BYTE *p_buf, int max_len);						// 读取文件数据(加快文件读写速度,每次读取AUDIO_FILE_BUF_SIZE)

public:

private:
	FILE					*m_file;				// 当前正在播放的文件
	BYTE					m_data_buf[AUDIO_FILE_BUF_SIZE];
	int						m_data_read;			// m_data_buf的读取位置
	int						m_data_size;			// m_data_buf的数据大小
	long					m_file_data_read;		// 已读取文件数据大小
	long					m_file_data_progress;	// 已读取文件数据进度
	ua_mutex_t				m_mutex_file;			// 文件指针互斥锁
	List<audio_info_t *>	m_list_play;			// 播放列表
	ua_mutex_t				m_mutex_play_list;		// m_list_play的互斥锁,所有公共函数操作,都需要加锁
	int						m_list_pos;				// 当前播放音频在清单的位置(0起始)
	audio_info_t			m_audio_info_playing;	// 当前播放的文件信息
	mad_decoder_t			m_mp3_decoder;			// mp3解码器
	HBE_STREAM				m_mp3_encoder;			// mp3编码器
	DWORD					m_mp3_encode_samples;	// mp3编码的输入采样数量
	DWORD					m_mp3_encode_out_size;	// mp3编码的输出缓冲大小
	SpeexResamplerState		*m_presampler;			// 重采样器
	BOOL					m_bmp3_decode;			// 需要mp3解码
	BOOL					m_bmp3_encode;			// 需要mp3编码
	BOOL					m_bresample;			// 需要重采样
	BOOL					m_play_status;			// 播放状态
	void					(*m_pplay_event_callback)(int, DWORD, DWORD, void *);
	void					*m_pevent_callback_param;

	// 音频输入
	int						m_play_mode;			// 播放模式
	int						m_out_audio_type;		// 输出音频类型(PCM,MP3)
	int						m_out_sample_rate;		// 输出采样率
	int						m_out_msecond_per_frame;// 输出每帧毫秒数
	int						m_out_bitrate;			// 输出位率
	BYTE					*m_pout_buf;			// 输出缓冲
	int						m_out_buf_size;			// 输出缓冲大小
	int						m_out_buf_len;			// 输出缓冲有效数据长度
};

// 获取播放状态
inline int CAudioFile::get_play_status(void)
{
	return m_play_status;
}

#endif // __AUDIOFILE_H__

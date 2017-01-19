/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : VideoFile.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2017/01/05
* Description  : ��Ƶ�ļ������
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

#define VIDEO_PLAY_MODE_SINGLE_PLAY		0	// ��������
#define VIDEO_PLAY_MODE_SINGLE_LOOP		1	// ����ѭ��
#define VIDEO_PLAY_MODE_ALL_PLAY		2	// ˳�򲥷�
#define VIDEO_PLAY_MODE_ALL_LOOP		3	// ѭ������
#define VIDEO_PLAY_MODE_RANDOM_PLAY		4	// �������

#define VIDEO_PLAY_STATUS_PLAYING		0	// ����
#define VIDEO_PLAY_STATUS_PAUSE			1	// ��ͣ
#define VIDEO_PLAY_STATUS_STOP			2	// ֹͣ

#define VIDEO_TYPE_UNKNOWN			0
#define VIDEO_TYPE_H264				1

#define VIDEO_FILE_BUF_SIZE			(1024 * 80)

typedef struct video_info
{
	int			video_type;			// ��Ƶ����(PCM, MP3)
	int			bitrate;			// λ��
	int			sample_rate;		// ������
	int			channels;			// 1 : ������; 2 : ˫����;
	int			data_offset;		// WAV�ļ�������
	long		data_size;			// ��Ƶ���ݴ�С(�ֽ���)
	char		*file_path;			// �ļ�·��
} video_info_t;

#define VIDEO_PLAY_EVENT_OPEN_FILE		0	// ���ļ�
#define VIDEO_PLAY_EVENT_PLAY_PROGRESS	1	// ���Ž���

class CVideoFile
{
public:
	CVideoFile();
	virtual ~CVideoFile();

	// ����
	void set_play_mode(int play_mode);									// ���ò���ģʽ
	int get_play_mode(void);											// ��ȡ����ģʽ
	// �����б����
	BOOL add_file(const char *p_file_path);								// ����ļ��������б�
	void remove_all_file(void);											// ��������б�
	BOOL remove_file(const char *p_file_path);							// �Ӳ����б�ɾ���ļ�
	// ����״̬����
	void reset_play(void);												// ���貥��״̬
	int play(void);														// ��ʼ����(�����ļ����,0��ʼ)
	int stop(void);														// ֹͣ����(�����ļ����,0��ʼ)
	int pause(void);													// ��ͣ����(�����ļ����,0��ʼ)
	int previous(void);													// ����ǰһ���ļ�(�����ļ����,0��ʼ)
	int next(void);														// ������һ���ļ�(�����ļ����,0��ʼ)
	int set_play_index(int index);										// ���ò����ļ������(0��ʼ)
	int get_play_status(void);											// ��ȡ����״̬
	// ����,��ȡ����
	int read_frame_data(BYTE *p_buf, int max_len, BYTE *p_nalu);		// ��ȡһ֡����

private:
	// ����״̬����
    void set_play_event_callback(void(*p_callback)(int, DWORD, DWORD, void *), void *p_param);// ���ò����¼��ص�����
	// �����б�
	void destroy_play_list(void);										// ���ٲ����б�
	BOOL get_h264_file_info(const char *p_file_path, video_info_t *p_audio);	// ��ȡH264�ļ���Ϣ
	BOOL open_file(void);												// ���ļ�
	void close_file(void);												// �ر��ļ�
	int play_next(void);												// ���ݲ���ģʽ������һ���ļ�(�����ļ����,0��ʼ)
	// ��Ƶ��Ϣ
	void init_video_info(video_info_t *p_audio);						// ��ʼ����Ƶ��Ϣ
	void destroy_video_info(video_info_t *p_audio);						// ������Ƶ��Ϣ
	BOOL dup_video_info(video_info_t *p_video_dst, video_info_t *p_video_src);	// ������Ƶ��Ϣ
	// ����,��ȡ����
	void get_out_format(int *p_video_type, int *p_sample_rate, int *p_msecond_per_frame);// ��ȡ�����ʽ
	inline int file_data_read(BYTE *p_buf, int max_len);				// ��ȡ�ļ�����(�ӿ��ļ���д�ٶ�,ÿ�ζ�ȡVIDEO_FILE_BUF_SIZE)

public:

private:
	FILE					*m_file;				// ��ǰ���ڲ��ŵ��ļ�
	BYTE					m_data_buf[VIDEO_FILE_BUF_SIZE];
	int						m_data_read;			// m_data_buf�Ķ�ȡλ��
	int						m_data_size;			// m_data_buf�����ݴ�С
	long					m_file_data_read;		// �Ѷ�ȡ�ļ����ݴ�С
	long					m_file_data_progress;	// �Ѷ�ȡ�ļ����ݽ���
	int						m_h264_start_len;		// ��ʼ��־����
	ua_mutex_t				m_mutex_file;			// �ļ�ָ�뻥����
	List<video_info_t *>	m_list_play;			// �����б�
	ua_mutex_t				m_mutex_play_list;		// m_list_play�Ļ�����,���й�����������,����Ҫ����
	int						m_list_pos;				// ��ǰ������Ƶ���嵥��λ��(0��ʼ)
	video_info_t			m_video_info_playing;	// ��ǰ���ŵ��ļ���Ϣ
	BOOL					m_play_status;			// ����״̬
	void					(*m_pplay_event_callback)(int, DWORD, DWORD, void *);
	void					*m_pevent_callback_param;

	// ��Ƶ����
	int						m_play_mode;			// ����ģʽ
	int						m_out_msecond_per_frame;// ���ÿ֡������
	int						m_out_bitrate;			// ���λ��
};

// ��ȡ����״̬
inline int CVideoFile::get_play_status(void)
{
	return m_play_status;
}

#endif // __VIDEOFILE_H__

/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : AudioFile.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/04/13
* Description  : ��Ƶ�ļ������
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
	DWORD dwRIFFSize;   //�ļ��ܳ��� - 8
	DWORD dwFileType;	//"WAVE"
	DWORD dwFmtID;		//"fmt "
	DWORD dwFmtSize;	//��ʽ��Ϣ����
}WAVEFILEINFO;

typedef struct tagWAVEFILEFACT {
	DWORD dwFACTID;		//"fact"
	DWORD dwFACTSize;   //fact������
}WAVEFILEFACT;

typedef struct tagWAVEFILEDATA {
	DWORD dwDATAID;		//"DATA"
	DWORD dwDATASize;   //����������
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

#define PLAY_MODE_SINGLE_PLAY		0	// ��������
#define PLAY_MODE_SINGLE_LOOP		1	// ����ѭ��
#define PLAY_MODE_ALL_PLAY			2	// ˳�򲥷�
#define PLAY_MODE_ALL_LOOP			3	// ѭ������
#define PLAY_MODE_RANDOM_PLAY		4	// �������

#define PLAY_STATUS_PLAYING			0	// ����
#define PLAY_STATUS_PAUSE			1	// ��ͣ
#define PLAY_STATUS_STOP			2	// ֹͣ

#define AUDIO_TYPE_UNKNOWN			0
#define AUDIO_TYPE_PCM				1
#define AUDIO_TYPE_MP3				2

#define MP3_ENCODER_BITRATE			128000

#define AUDIO_FILE_BUF_SIZE			(1024 * 8)

typedef struct audio_info
{
	int			audio_type;			// ��Ƶ����(PCM, MP3)
	int			bitrate;			// λ��
	int			sample_rate;		// ������
	int			channels;			// 1 : ������; 2 : ˫����;
	int			data_offset;		// WAV�ļ�������
	long		data_size;			// ��Ƶ���ݴ�С(�ֽ���)
	char		*file_path;			// �ļ�·��
} audio_info_t;

#define PLAY_EVENT_TYPE_OPEN_FILE		0	// ���ļ�
#define PLAY_EVENT_TYPE_PLAY_PROGRESS	1	// ���Ž���

class CAudioStream;
class CSipUA;
class CAudioFile
{
public:
	CAudioFile();
	virtual ~CAudioFile();

	friend class CAudioStream;
	friend class CSipUA;

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

private:
	// ����״̬����
    void set_play_event_callback(void(*p_callback)(int, DWORD, DWORD, void *), void *p_param);// ���ò����¼��ص�����
	// �����б�
	void destroy_play_list(void);										// ���ٲ����б�
	BOOL get_wav_file_info(const char *p_file_path, audio_info_t *p_audio);	// ��ȡWAV�ļ���Ϣ
	BOOL get_mp3_file_info(const char *p_file_path, audio_info_t *p_audio);	// ��ȡMP3�ļ���Ϣ
	BOOL open_file(void);												// ���ļ�
	void close_file(void);												// �ر��ļ�
	BOOL init_mp3_encoder(HBE_STREAM *p_mp3_encoder, int sample_rate, int channels, int bitrate, \
				DWORD *p_mp3_encode_samples, DWORD *p_mp3_encode_out_size);	// ��ʼ��mp3������
	int play_next(void);												// ���ݲ���ģʽ������һ���ļ�(�����ļ����,0��ʼ)
	// ��Ƶ��Ϣ
	void init_audio_info(audio_info_t *p_audio);						// ��ʼ����Ƶ��Ϣ
	void destroy_audio_info(audio_info_t *p_audio);						// ������Ƶ��Ϣ
	BOOL dup_audio_info(audio_info_t *p_audio_dst, audio_info_t *p_audio_src);	// ������Ƶ��Ϣ
	// ����,��ȡ����
	void set_out_format(int audio_type, int sample_rate, int msecond_per_frame);// ���������ʽ
	void get_out_format(int *p_audio_type, int *p_sample_rate, int *p_msecond_per_frame);// ��ȡ�����ʽ
	int read_frame_data(BYTE *p_buf, int max_len);						// ��ȡһ֡����
	int file_data_read(BYTE *p_buf, int max_len);						// ��ȡ�ļ�����(�ӿ��ļ���д�ٶ�,ÿ�ζ�ȡAUDIO_FILE_BUF_SIZE)

public:

private:
	FILE					*m_file;				// ��ǰ���ڲ��ŵ��ļ�
	BYTE					m_data_buf[AUDIO_FILE_BUF_SIZE];
	int						m_data_read;			// m_data_buf�Ķ�ȡλ��
	int						m_data_size;			// m_data_buf�����ݴ�С
	long					m_file_data_read;		// �Ѷ�ȡ�ļ����ݴ�С
	long					m_file_data_progress;	// �Ѷ�ȡ�ļ����ݽ���
	ua_mutex_t				m_mutex_file;			// �ļ�ָ�뻥����
	List<audio_info_t *>	m_list_play;			// �����б�
	ua_mutex_t				m_mutex_play_list;		// m_list_play�Ļ�����,���й�����������,����Ҫ����
	int						m_list_pos;				// ��ǰ������Ƶ���嵥��λ��(0��ʼ)
	audio_info_t			m_audio_info_playing;	// ��ǰ���ŵ��ļ���Ϣ
	mad_decoder_t			m_mp3_decoder;			// mp3������
	HBE_STREAM				m_mp3_encoder;			// mp3������
	DWORD					m_mp3_encode_samples;	// mp3����������������
	DWORD					m_mp3_encode_out_size;	// mp3�������������С
	SpeexResamplerState		*m_presampler;			// �ز�����
	BOOL					m_bmp3_decode;			// ��Ҫmp3����
	BOOL					m_bmp3_encode;			// ��Ҫmp3����
	BOOL					m_bresample;			// ��Ҫ�ز���
	BOOL					m_play_status;			// ����״̬
	void					(*m_pplay_event_callback)(int, DWORD, DWORD, void *);
	void					*m_pevent_callback_param;

	// ��Ƶ����
	int						m_play_mode;			// ����ģʽ
	int						m_out_audio_type;		// �����Ƶ����(PCM,MP3)
	int						m_out_sample_rate;		// ���������
	int						m_out_msecond_per_frame;// ���ÿ֡������
	int						m_out_bitrate;			// ���λ��
	BYTE					*m_pout_buf;			// �������
	int						m_out_buf_size;			// ��������С
	int						m_out_buf_len;			// ���������Ч���ݳ���
};

// ��ȡ����״̬
inline int CAudioFile::get_play_status(void)
{
	return m_play_status;
}

#endif // __AUDIOFILE_H__

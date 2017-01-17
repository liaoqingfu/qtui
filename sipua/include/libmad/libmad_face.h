/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : libmad_face.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/09/10
* Description  : mad mp3����ӿ�
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __LIBMAD_FACE_H__
#define __LIBMAD_FACE_H__

//-------------------- WIN32 --------------------
#ifdef WIN32

#include <limits.h>
#include <windows.h>

#	define LIBMAD_EXPORTS
#	ifdef LIBMAD_EXPORTS
#		define LIBMAD_PUBLIC	__declspec(dllexport)
#	else 
#		define LIBMAD_PUBLIC	__declspec(dllimport)
#	endif

//-------------------- Linux --------------------
#else
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#define LIBMAD_PUBLIC

#endif // WIN32

//-------------------- WIN32 Linux ���� --------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "MadGlobal.h"
#include "stream.h"
#include "frame.h"
#include "synth.h"

#define MAD_IN_LEN_MAX		256
#define MAD_INBUF_SIZE      (MAD_IN_LEN_MAX*6)
typedef struct mad_decoder {
	int					out_pcm_pos;
	int					out_pcm_len;
	unsigned char		in_data_buf[MAD_INBUF_SIZE];
	int					in_data_len;
	struct mad_stream	stream;
	struct mad_frame	frame;
	struct mad_synth	synth;
} mad_decoder_t;

#ifdef __cplusplus
extern "C"
{
#endif

LIBMAD_PUBLIC void		mad_init(mad_decoder_t *p_decoder);								// mad MP3��������ʼ��
LIBMAD_PUBLIC void		mad_destroy(mad_decoder_t *p_decoder);							// mad MP3���������ٶ�̬�ڴ�
LIBMAD_PUBLIC int		mad_decode(mad_decoder_t *p_decoder, const unsigned char *p_in_buf, \
							int in_len);												// ���ݽ���,�����С���Ϊ256unsigned chars
LIBMAD_PUBLIC int		mad_get_out_len(mad_decoder_t *p_decoder);						// ��ȡ���������������
LIBMAD_PUBLIC int		mad_get_out_buf(mad_decoder_t *p_decoder, short **p_out_left, \
							short **p_out_right);										// ������ݻ���ĵ�ַ,�����㻺���С
LIBMAD_PUBLIC int		mad_get_out_data(mad_decoder_t *p_decoder, short *p_out_left, \
							short *p_out_right, int out_len);							// �����������
LIBMAD_PUBLIC int		mad_get_frame_size(mad_decoder_t *p_decoder);					// ��ȡ��ǰ�����֡��С
LIBMAD_PUBLIC int		mad_get_samplerate(mad_decoder_t *p_decoder);					// ��ȡ��ǰ����Ĳ���Ƶ��

#ifdef __cplusplus
}
#endif

#endif // __LIBMAD_FACE_H__

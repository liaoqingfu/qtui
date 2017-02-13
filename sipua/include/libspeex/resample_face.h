/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : libresample_face.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/10/14
* Description  : speax重采样接口
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __LIBRESAMPLE_FACE_H__
#define __LIBRESAMPLE_FACE_H__

//-------------------- WIN32 --------------------
#ifdef WIN32

#include <limits.h>
#include <windows.h>

#	define LIBRESAMPLE_EXPORTS
#	ifdef LIBRESAMPLE_EXPORTS
#		define LIBRESAMPLE_PUBLIC	__declspec(dllexport)
#	else 
#		define LIBRESAMPLE_PUBLIC	__declspec(dllimport)
#	endif

//-------------------- Linux --------------------
#else
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>

#define LIBRESAMPLE_PUBLIC

#endif // WIN32

//-------------------- WIN32 Linux 共用 --------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "speex_resampler.h"

#ifdef __cplusplus
extern "C"
{
#endif

LIBRESAMPLE_PUBLIC SpeexResamplerState * \
							ResampleInit(spx_uint32_t nb_channels, \
								spx_uint32_t in_rate, spx_uint32_t out_rate, int quality, int *err);	// 初始化
LIBRESAMPLE_PUBLIC int		ResampleProcessInt(SpeexResamplerState *st, spx_uint32_t channel_index, const spx_int16_t *in, \
								spx_uint32_t *in_len, spx_int16_t *out, spx_uint32_t *out_len);			// 重采样
LIBRESAMPLE_PUBLIC void		ResampleDestroy(SpeexResamplerState *st);									// 销毁

#ifdef __cplusplus
}
#endif

#endif // __LIBRESAMPLE_FACE_H__

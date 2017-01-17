/* 
******************************************************************************************************** 
* Copyright (C) 2014, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : CodecAlawUlaw.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2014/07/12
* Description  : 音频编解码: Alaw Ulaw
******************************************************************************************************** 
* $Id:$ 
******************************************************************************************************** 
*/
#ifndef __CODEC_ALAW_ULAW_H__
#define __CODEC_ALAW_ULAW_H__

#include "types.h"

//----------------- 1. 宏及结构本声明 -----------------

//----------------- 2. 变量声明 -----------------------

//----------------- 3. 函数声明 -----------------------
#ifdef __cplusplus
extern "C"
{
#endif

WORD    CodecAlaw_Decoder(void *pDst, void *pSrc, WORD wSrcSize);			// Alaw数据解码:8Bit->16Bit,返回字节数
WORD    CodecAlaw_Encoder(void *pDst, void *pSrc, WORD wSrcSize);			// Alaw数据压缩:16Bit->8Bit,返回字节数
WORD    CodecUlaw_Decoder(void *pDst, void *pSrc, WORD wSrcSize);			// Ulaw数据解码:8Bit->16Bit,返回字节数
WORD    CodecUlaw_Encoder(void *pDst, void *pSrc, WORD wSrcSize);			// Ulaw数据压缩:16Bit->8Bit,返回字节数
WORD	CodecAlaw_2Ulaw(void *pDst, void *pSrc, WORD wSrcSize);				// A-law to u-law conversion
WORD	CodecUlaw_2Alaw(void *pDst, void *pSrc, WORD wSrcSize);				// U-law to A-law conversion

#ifdef __cplusplus
}
#endif

#endif // __CODEC_ALAW_ULAW_H__




/* 
******************************************************************************************************** 
* Copyright (C) 2014, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : CodecAlawUlaw.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2014/07/12
* Description  : ��Ƶ�����: Alaw Ulaw
******************************************************************************************************** 
* $Id:$ 
******************************************************************************************************** 
*/
#ifndef __CODEC_ALAW_ULAW_H__
#define __CODEC_ALAW_ULAW_H__

#include "types.h"

//----------------- 1. �꼰�ṹ������ -----------------

//----------------- 2. �������� -----------------------

//----------------- 3. �������� -----------------------
#ifdef __cplusplus
extern "C"
{
#endif

WORD    CodecAlaw_Decoder(void *pDst, void *pSrc, WORD wSrcSize);			// Alaw���ݽ���:8Bit->16Bit,�����ֽ���
WORD    CodecAlaw_Encoder(void *pDst, void *pSrc, WORD wSrcSize);			// Alaw����ѹ��:16Bit->8Bit,�����ֽ���
WORD    CodecUlaw_Decoder(void *pDst, void *pSrc, WORD wSrcSize);			// Ulaw���ݽ���:8Bit->16Bit,�����ֽ���
WORD    CodecUlaw_Encoder(void *pDst, void *pSrc, WORD wSrcSize);			// Ulaw����ѹ��:16Bit->8Bit,�����ֽ���
WORD	CodecAlaw_2Ulaw(void *pDst, void *pSrc, WORD wSrcSize);				// A-law to u-law conversion
WORD	CodecUlaw_2Alaw(void *pDst, void *pSrc, WORD wSrcSize);				// U-law to A-law conversion

#ifdef __cplusplus
}
#endif

#endif // __CODEC_ALAW_ULAW_H__




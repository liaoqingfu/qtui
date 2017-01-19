/* 
******************************************************************************************************** 
* Copyright (C) 2014, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : CodecAlawUlaw.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2014/07/12
* Description  : 音频编解码: Alaw Ulaw
******************************************************************************************************** 
* $Id:$ 
******************************************************************************************************** 
*/
#include "ua_port.h"
//#include <windows.h>
//#include "types.h"
#include "CodecAlawUlaw.h"

//----------------- 1. 宏及结构本声明 -----------------
#define SIGN_BIT    (0x80)      // Sign bit for a A-law byte.
#define QUANT_MASK  (0xf)       // Quantization field mask.  
#define NSEGS       (8)         // Number of A-law segments. 
#define SEG_SHIFT   (4)         // Left shift for segment number.
#define SEG_MASK    (0x70)      // Segment field mask.

//----------------- 2. 变量定义 -----------------------
static const short g_pSegEnd[8] = {0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF};  

// copy from CCITT G.711 specifications 
const unsigned char g_pUlaw2Alaw[128] = { // u- to A-law conversions
	1,  1,  2,  2,  3,  3,  4,  4,  
	5,  5,  6,  6,  7,  7,  8,  8,  
	9,  10, 11, 12, 13, 14, 15, 16,  
	17, 18, 19, 20, 21, 22, 23, 24,  
	25, 27, 29, 31, 33, 34, 35, 36,  
	37, 38, 39, 40, 41, 42, 43, 44,  
	46, 48, 49, 50, 51, 52, 53, 54,  
	55, 56, 57, 58, 59, 60, 61, 62,  
	64, 65, 66, 67, 68, 69, 70, 71,  
	72, 73, 74, 75, 76, 77, 78, 79,  
	81, 82, 83, 84, 85, 86, 87, 88,  
	89, 90, 91, 92, 93, 94, 95, 96,  
	97, 98, 99, 100,101,102,103,104,  
	105,106,107,108,109,110,111,112,  
	113,114,115,116,117,118,119,120,  
	121,122,123,124,125,126,127,128  
}; 

const unsigned char g_pAlaw2Ulaw[128] = { // A- to u-law conversions
	1,  3,  5,  7,  9,  11, 13, 15,  
	16, 17, 18, 19, 20, 21, 22, 23,  
	24, 25, 26, 27, 28, 29, 30, 31,  
	32, 32, 33, 33, 34, 34, 35, 35,  
	36, 37, 38, 39, 40, 41, 42, 43,  
	44, 45, 46, 47, 48, 48, 49, 49,  
	50, 51, 52, 53, 54, 55, 56, 57,  
	58, 59, 60, 61, 62, 63, 64, 64,  
	65, 66, 67, 68, 69, 70, 71, 72,  
	73, 74, 75, 76, 77, 78, 79, 79,  
	80, 81, 82, 83, 84, 85, 86, 87,  
	88, 89, 90, 91, 92, 93, 94, 95,  
	96, 97, 98, 99, 100,101,102,103,  
	104,105,106,107,108,109,110,111,  
	112,113,114,115,116,117,118,119,  
	120,121,122,123,124,125,126,127  
};  

//----------------- 3. 函数声明 -----------------------

//----------------- 4. 函数定义 -----------------------
/********************************************************************* 
	* linear2alaw() - Convert a 16-bit linear PCM value to 8-bit A-law 
	*   
	* linear2alaw() accepts an 16-bit integer and encodes it as A-law data. 
	* 
	*  Linear Input Code       Compressed Code 
	*  -----------------       ------------------ 
	*  0000000wxyza            000wxyz 
	*  0000001wxyza            001wxyz 
	*  000001wxyzab            010wxyz 
	*  00001wxyzabc            011wxyz 
	*  0001wxyzabcd            100wxyz 
	*  001wxyzabcde            101wxyz 
	*  01wxyzabcdef            110wxyz 
	*  1wxyzabcdefg            111wxyz 
	* 
	* For further information see John C. Bellamy's Digital Telephony, 1982, 
	* John Wiley & Sons, pps 98-111 and 472-476. 
 *********************************************************************/
// Alaw数据压缩:16Bit->8Bit,返回字节数
WORD CodecAlaw_Encoder(void *pDst, void *pSrc, WORD wSrcSize)
{
	short			*pshSrc = (short *)pSrc;
	BYTE			*pucDst = (BYTE *)pDst;
	WORD			i, j;
	int				nMask, nPcmVal, nSeg;
	BYTE			ucVal;

	wSrcSize /= 2;
	for (i = 0; i < wSrcSize; i++)
	{
		nPcmVal = pshSrc[i];
		if (nPcmVal >= 0)
			nMask = 0xD5;		// sign (7th) bit = 1
		else
		{
			nMask = 0x55;		// sign bit = 0
			nPcmVal = -nPcmVal - 1;
		}
		// 查找压缩区间 Convert the scaled magnitude to segment number.
		nSeg = 8;
		for (j = 0; j < 8; j++)
		{
			if (nPcmVal <= g_pSegEnd[j])
			{
				nSeg = j;
				break;
			}
		}
		// Combine the sign, segment, and quantization bits.
		if (nSeg >= 8)		// out of range, return maximum value.
			*pucDst++ = (BYTE)(0x7F ^ nMask);
		else
		{
			ucVal = nSeg << SEG_SHIFT;
			if (nSeg < 2)
				ucVal |= (nPcmVal >> 4) & QUANT_MASK;
			else
				ucVal |= (nPcmVal >> (nSeg + 3)) & QUANT_MASK;
			*pucDst++ = ucVal ^ nMask;
		}
	}

	return wSrcSize;
}

// Alaw数据解码:8Bit->16Bit,返回字节数
WORD CodecAlaw_Decoder(void *pDst, void *pSrc, WORD wSrcSize)
{
	int			nTemp, nSeg;
	WORD		i;
	short		*pshDst = (short *)pDst;
	BYTE		*pucSrc = (BYTE *)pSrc;
	BYTE		ucVal;

	for (i = 0; i < wSrcSize; i++)
	{
		ucVal = *pucSrc++;
		ucVal ^= 0x55;
		nTemp = (ucVal & QUANT_MASK) << 4;
		nSeg = ((unsigned)ucVal & SEG_MASK) >> SEG_SHIFT;
		switch (nSeg)
		{
		case 0:		nTemp += 8; break;
		case 1:		nTemp += 0x108; break;
		default:	nTemp += 0x108; nTemp <<= nSeg - 1; break;
		}
		*pshDst++ = (short)((ucVal & SIGN_BIT) ? nTemp : -nTemp);
	}

	return wSrcSize * 2;
}

#define BIAS        (0x84)      // Bias for linear code.
/********************************************************************* 
	* linear2ulaw() - Convert a linear PCM value to u-law 
	* 
	* In order to simplify the encoding process, the original linear magnitude 
	* is biased by adding 33 which shifts the encoding range from (0 - 8158) to 
	* (33 - 8191). The result can be seen in the following encoding table: 
	* 
	*  Biased Linear Input Code    Compressed Code 
	*  ------------------------    --------------- 
	*  00000001wxyza               000wxyz 
	*  0000001wxyzab               001wxyz 
	*  000001wxyzabc               010wxyz 
	*  00001wxyzabcd               011wxyz 
	*  0001wxyzabcde               100wxyz 
	*  001wxyzabcdef               101wxyz 
	*  01wxyzabcdefg               110wxyz 
	*  1wxyzabcdefgh               111wxyz 
	* 
	* Each biased linear code has a leading 1 which identifies the segment 
	* number. The value of the segment number is equal to 7 minus the number 
	* of leading 0's. The quantization interval is directly available as the 
	* four bits wxyz.  * The trailing bits (a - h) are ignored. 
	* 
	* Ordinarily the complement of the resulting code word is used for 
	* transmission, and so the code word is complemented before it is returned. 
	* 
	* For further information see John C. Bellamy's Digital Telephony, 1982, 
	* John Wiley & Sons, pps 98-111 and 472-476. 
*********************************************************************/  
// Ulaw数据压缩:16Bit->8Bit,返回字节数
WORD CodecUlaw_Encoder(void *pDst, void *pSrc, WORD wSrcSize)
{
	short			*pshSrc = (short *)pSrc;
	BYTE			*pucDst = (BYTE *)pDst;
	WORD			i, j;
	int				nMask, nPcmVal, nSeg;
	BYTE			ucVal;
	
	wSrcSize /= 2;
	for (i = 0; i < wSrcSize; i++)
	{
		nPcmVal = pshSrc[i];
		// Get the sign and the magnitude of the value.
		if (nPcmVal < 0)
		{
			nPcmVal = BIAS - nPcmVal;
			nMask = 0x7F;
		}
		else
		{
			nPcmVal += BIAS;
			nMask = 0xFF;
		}
		// 查找压缩区间 Convert the scaled magnitude to segment number.
		nSeg = 8;
		for (j = 0; j < 8; j++)
		{
			if (nPcmVal <= g_pSegEnd[j])
			{
				nSeg = j;
				break;
			}
		}
		// Combine the sign, segment, and quantization bits.and complement the code word.
		if (nSeg >= 8)		// out of range, return maximum value.
			*pucDst++ = (BYTE)(0x7F ^ nMask);
		else
		{
			ucVal = (nSeg << 4) | ((nPcmVal >> (nSeg + 3)) & 0x0F);
			*pucDst++ = ucVal ^ nMask;
		}
	}
	
	return wSrcSize;
}

// Ulaw数据解码:8Bit->16Bit,返回字节数
WORD CodecUlaw_Decoder(void *pDst, void *pSrc, WORD wSrcSize)
{
	int			nTemp;
	WORD		i;
	short		*pshDst = (short *)pDst;
	BYTE		*pucSrc = (BYTE *)pSrc;
	BYTE		ucVal;
	
	for (i = 0; i < wSrcSize; i++)
	{
		ucVal = *pucSrc++;
		// Complement to obtain normal u-law value
		ucVal = ~ucVal;
		// Extract and bias the quantization bits. Then shift up by the segment number and subtract out the bias.
		nTemp = ((ucVal & QUANT_MASK) << 3) + BIAS;
		nTemp <<= ((unsigned)ucVal & SEG_MASK) >> SEG_SHIFT;
		*pshDst++ = (short)((ucVal & SIGN_BIT) ? (BIAS - nTemp) : (nTemp - BIAS));
	}
	
	return wSrcSize * 2;
}

// A-law to u-law conversion
WORD CodecAlaw_2Ulaw(void *pDst, void *pSrc, WORD wSrcSize)
{
	WORD		i;
	BYTE		*pucDst = (BYTE *)pDst;
	BYTE		*pucSrc = (BYTE *)pSrc;
	BYTE		ucVal;

	for (i = 0; i < wSrcSize; i++)
	{
		ucVal = *pucSrc++;
		*pucDst++ = ((ucVal & 0x80) ? (0xFF ^ g_pAlaw2Ulaw[ucVal ^ 0xD5]) \
			: (0x7F ^ g_pAlaw2Ulaw[ucVal ^ 0x55]));
	}

	return wSrcSize;
}

// U-law to A-law conversion
WORD CodecUlaw_2Alaw(void *pDst, void *pSrc, WORD wSrcSize)
{
	WORD		i;
	BYTE		*pucDst = (BYTE *)pDst;
	BYTE		*pucSrc = (BYTE *)pSrc;
	BYTE		ucVal;

	for (i = 0; i < wSrcSize; i++)
	{
		ucVal = *pucSrc++;
		*pucDst++ = ((ucVal & 0x80) ? (0xD5 ^ (g_pUlaw2Alaw[ucVal ^ 0xFF] - 1)) \
			: (0x55 ^ (g_pUlaw2Alaw[ucVal ^ 0x7F] - 1)));
	}

	return wSrcSize;
}


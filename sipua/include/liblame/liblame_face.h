/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : liblame_face.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2016/09/10
* Description  : mad mp3解码接口
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#ifndef __LIBLAME_FACE_H__
#define __LIBLAME_FACE_H__

//-------------------- WIN32 --------------------
#ifdef WIN32

#include <limits.h>
#include <windows.h>

#	define LIBLAME_EXPORTS
#	ifdef LIBLAME_EXPORTS
#		define LIBLAME_PUBLIC	__declspec(dllexport)
#	else 
#		define LIBLAME_PUBLIC	__declspec(dllimport)
#	endif

//-------------------- Linux --------------------
#else
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include "types.h"

#define LIBLAME_PUBLIC

#endif // WIN32

//-------------------- WIN32 Linux 共用 --------------------
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "lame.h"

#ifdef __GNUC__
#define ATTRIBUTE_PACKED	__attribute__((packed))
#else
#define ATTRIBUTE_PACKED
#pragma pack(push)
#pragma pack(1)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

//----------------- 1. 宏及结构本声明 -----------------
/* encoding formats */
#define		BE_CONFIG_MP3			0										
#define		BE_CONFIG_LAME			256		

/* type definitions */
typedef		unsigned long			HBE_STREAM;
typedef		HBE_STREAM				*PHBE_STREAM;
typedef		unsigned long			BE_ERR;

/* error codes */
#define		BE_ERR_SUCCESSFUL					0x00000000
#define		BE_ERR_INVALID_FORMAT				0x00000001
#define		BE_ERR_INVALID_FORMAT_PARAMETERS	0x00000002
#define		BE_ERR_NO_MORE_HANDLES				0x00000003
#define		BE_ERR_INVALID_HANDLE				0x00000004
#define		BE_ERR_BUFFER_TOO_SMALL				0x00000005

/* other constants */
#define		BE_MAX_HOMEPAGE			128

/* format specific variables */
#define		BE_MP3_MODE_STEREO		0
#define		BE_MP3_MODE_JSTEREO		1
#define		BE_MP3_MODE_DUALCHANNEL	2
#define		BE_MP3_MODE_MONO		3

#define		MPEG1	1
#define		MPEG2	0

#define CURRENT_STRUCT_VERSION 1
#define CURRENT_STRUCT_SIZE sizeof(BE_CONFIG)	// is currently 331 bytes

typedef enum
{
	VBR_METHOD_NONE			= -1,
	VBR_METHOD_DEFAULT		=  0,
	VBR_METHOD_OLD			=  1,
	VBR_METHOD_NEW			=  2,
	VBR_METHOD_MTRH			=  3,
	VBR_METHOD_ABR			=  4
} VBRMETHOD;

typedef enum 
{
	LQP_NOPRESET			=-1,

	// QUALITY PRESETS
	LQP_NORMAL_QUALITY		= 0,
	LQP_LOW_QUALITY			= 1,
	LQP_HIGH_QUALITY		= 2,
	LQP_VOICE_QUALITY		= 3,
	LQP_R3MIX				= 4,
	LQP_VERYHIGH_QUALITY	= 5,
	LQP_STANDARD			= 6,
	LQP_FAST_STANDARD		= 7,
	LQP_EXTREME				= 8,
	LQP_FAST_EXTREME		= 9,
	LQP_INSANE				= 10,
	LQP_ABR					= 11,
	LQP_CBR					= 12,
	LQP_MEDIUM				= 13,
	LQP_FAST_MEDIUM			= 14,

	// NEW PRESET VALUES
	LQP_PHONE	=1000,
	LQP_SW		=2000,
	LQP_AM		=3000,
	LQP_FM		=4000,
	LQP_VOICE	=5000,
	LQP_RADIO	=6000,
	LQP_TAPE	=7000,
	LQP_HIFI	=8000,
	LQP_CD		=9000,
	LQP_STUDIO	=10000

} LAME_QUALITY_PRESET;

typedef struct	{
	DWORD	dwConfig;			// BE_CONFIG_XXXXX
								// Currently only BE_CONFIG_MP3 is supported
	union	{
		struct	{
			DWORD	dwSampleRate;		// 48000, 44100 and 32000 allowed
			BYTE	byMode;			// BE_MP3_MODE_STEREO, BE_MP3_MODE_DUALCHANNEL, BE_MP3_MODE_MONO
			WORD	wBitrate;		// 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256 and 320 allowed
			BOOL	bPrivate;		
			BOOL	bCRC;
			BOOL	bCopyright;
			BOOL	bOriginal;

			} mp3;					// BE_CONFIG_MP3

			struct
			{
			// STRUCTURE INFORMATION
			DWORD			dwStructVersion;	
			DWORD			dwStructSize;

			// BASIC ENCODER SETTINGS
			DWORD			dwSampleRate;		// SAMPLERATE OF INPUT FILE
			DWORD			dwReSampleRate;		// DOWNSAMPLERATE, 0=ENCODER DECIDES  
			LONG			nMode;				// BE_MP3_MODE_STEREO, BE_MP3_MODE_DUALCHANNEL, BE_MP3_MODE_MONO
			DWORD			dwBitrate;			// CBR bitrate, VBR min bitrate
			DWORD			dwMaxBitrate;		// CBR ignored, VBR Max bitrate
			LONG			nPreset;			// Quality preset, use one of the settings of the LAME_QUALITY_PRESET enum
			DWORD			dwMpegVersion;		// FUTURE USE, MPEG-1 OR MPEG-2
			DWORD			dwPsyModel;			// FUTURE USE, SET TO 0
			DWORD			dwEmphasis;			// FUTURE USE, SET TO 0

			// BIT STREAM SETTINGS
			BOOL			bPrivate;			// Set Private Bit (TRUE/FALSE)
			BOOL			bCRC;				// Insert CRC (TRUE/FALSE)
			BOOL			bCopyright;			// Set Copyright Bit (TRUE/FALSE)
			BOOL			bOriginal;			// Set Original Bit (TRUE/FALSE)
			
			// VBR STUFF
			BOOL			bWriteVBRHeader;	// WRITE XING VBR HEADER (TRUE/FALSE)
			BOOL			bEnableVBR;			// USE VBR ENCODING (TRUE/FALSE)
			INT				nVBRQuality;		// VBR QUALITY 0..9
			DWORD			dwVbrAbr_bps;		// Use ABR in stead of nVBRQuality
			VBRMETHOD		nVbrMethod;
			BOOL			bNoRes;				// Disable Bit resorvoir (TRUE/FALSE)

			// MISC SETTINGS
			BOOL			bStrictIso;			// Use strict ISO encoding rules (TRUE/FALSE)
			WORD			nQuality;			// Quality Setting, HIGH BYTE should be NOT LOW byte, otherwhise quality=5

			// FUTURE USE, SET TO 0, align strucutre to 331 bytes
			BYTE			btReserved[255-4*sizeof(DWORD) - sizeof( WORD )];
			} LHV1;					// LAME header version 1
		struct	{
			DWORD	dwSampleRate;
			BYTE	byMode;
			WORD	wBitrate;
			BYTE	byEncodingMethod;
		} aac;
	} format;
} ATTRIBUTE_PACKED BE_CONFIG, *PBE_CONFIG;

typedef struct	{
	// BladeEnc DLL Version number
	BYTE	byDLLMajorVersion;
	BYTE	byDLLMinorVersion;

	// BladeEnc Engine Version Number
	BYTE	byMajorVersion;
	BYTE	byMinorVersion;

	// DLL Release date
	BYTE	byDay;
	BYTE	byMonth;
	WORD	wYear;

	// BladeEnc	Homepage URL
	CHAR	zHomepage[BE_MAX_HOMEPAGE + 1];	

	BYTE	byAlphaLevel;
	BYTE	byBetaLevel;
	BYTE	byMMXEnabled;

	BYTE	btReserved[125];
} ATTRIBUTE_PACKED BE_VERSION, *PBE_VERSION;

//----------------- 2. 变量声明 -----------------------

//----------------- 3. 函数声明 -----------------------
//LIBLAME_PUBLIC void		mad_init(mad_decoder_t *p_decoder);								// mad MP3解码器初始化
LIBLAME_PUBLIC BE_ERR	beInitStream(PBE_CONFIG pbeConfig, PDWORD dwSamples, PDWORD dwBufferSize, PHBE_STREAM phbeStream);
LIBLAME_PUBLIC BE_ERR	beEncodeChunk(HBE_STREAM hbeStream, DWORD nSamples, PSHORT pSamples, PBYTE pOutput, PDWORD pdwOutput);

// added for floating point audio  -- DSPguru, jd
LIBLAME_PUBLIC BE_ERR	beDeinitStream(HBE_STREAM hbeStream, PBYTE pOutput, PDWORD pdwOutput);
LIBLAME_PUBLIC BE_ERR	beCloseStream(HBE_STREAM hbeStream);
LIBLAME_PUBLIC VOID		beVersion(PBE_VERSION pbeVersion);
LIBLAME_PUBLIC BE_ERR	beWriteVBRHeader(LPCSTR lpszFileName);
LIBLAME_PUBLIC BE_ERR	beWriteInfoTag( HBE_STREAM hbeStream, LPCSTR lpszFileName );

#ifdef __cplusplus
}
#endif

#ifndef __GNUC__
#pragma pack(pop)
#endif

#endif // __LIBLAME_FACE_H__

/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : H264Pack.h
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2017/01/10
* Description  : H264 RTP分包及合包程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
//////////////////////////////////////////////////////////////////////////////////////////
// http://blog.csdn.net/dengzikun/article/details/5807694
// class CH264_RTP_PACK start

#ifndef __H264_PACK_H__
#define __H264_PACK_H__

#include "ua_port.h"

class CH264_RTP_PACK
{
	#define RTP_VERSION 2

	typedef struct NAL_msg_s 
	{
		bool eoFrame ;
		unsigned char type;		// NAL type
		unsigned char *start;	// pointer to first location in the send buffer
		unsigned char *end;	// pointer to last location in send buffer
		unsigned long size ;
	} NAL_MSG_t;

	typedef struct 
	{
		//LITTLE_ENDIAN
		unsigned short   cc:4;		/* CSRC count                 */
		unsigned short   x:1;		/* header extension flag      */
		unsigned short   p:1;		/* padding flag               */
		unsigned short   v:2;		/* packet type                */
		unsigned short   pt:7;		/* payload type               */
		unsigned short   m:1;		/* marker bit                 */

		unsigned short    seq;		/* sequence number            */
		unsigned long     ts;		/* timestamp                  */
		unsigned long     ssrc;		/* synchronization source     */
	} rtp_hdr_t;

	typedef struct tagRTP_INFO
	{
		NAL_MSG_t	nal;	    // NAL information
		rtp_hdr_t	rtp_hdr;    // RTP header is assembled here
		int hdr_len;			// length of RTP header

		unsigned char *pRTP;    // pointer to where RTP packet has beem assembled
		unsigned char *start;	// pointer to start of payload
		unsigned char *end;		// pointer to end of payload

		unsigned int s_bit;		// bit in the FU header
		unsigned int e_bit;		// bit in the FU header
		bool FU_flag;		// fragmented NAL Unit flag
	} RTP_INFO;

public:
	CH264_RTP_PACK(unsigned long H264SSRC, unsigned char H264PAYLOADTYPE=99, unsigned short MAXRTPPACKSIZE=1472);
	~CH264_RTP_PACK(void);

	//传入Set的数据必须是一个完整的NAL,起始码为0x00000001。
	//起始码之前至少预留10个字节，以避免内存COPY操作。
	//打包完成后，原缓冲区内的数据被破坏。
	bool Set ( unsigned char *NAL_Buf, unsigned long NAL_Size, unsigned long Time_Stamp, bool End_Of_Frame );

	//循环调用Get获取RTP包，直到返回值为NULL
	unsigned char* Get ( unsigned short *pPacketSize );

private:
	unsigned int StartCode( unsigned char *cp );

private:
	RTP_INFO m_RTP_Info ;
	bool m_bBeginNAL ;
	unsigned short m_MAXRTPPACKSIZE ;
};

// class CH264_RTP_PACK end
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// class CH264_RTP_UNPACK start

class CH264_RTP_UNPACK
{

#define RTP_VERSION 2
#define BUF_SIZE (1024 * 100)

	typedef struct 
	{
		//LITTLE_ENDIAN
		unsigned short   cc:4;		/* CSRC count                 */
		unsigned short   x:1;		/* header extension flag      */
		unsigned short   p:1;		/* padding flag               */
		unsigned short   v:2;		/* packet type                */
		unsigned short   pt:7;		/* payload type               */
		unsigned short   m:1;		/* marker bit                 */

		unsigned short    seq;		/* sequence number            */
		unsigned long     ts;		/* timestamp                  */
		unsigned long     ssrc;		/* synchronization source     */
	} rtp_hdr_t;
public:

	CH264_RTP_UNPACK (unsigned char H264PAYLOADTYPE = 99 );
	~CH264_RTP_UNPACK(void);

	//pBuf为H264 RTP视频数据包，nSize为RTP视频数据包字节长度，outSize为输出视频数据帧字节长度。
	//返回值为指向视频数据帧的指针。输入数据可能被破坏。
	BYTE* Parse_RTP_Packet ( BYTE *pBuf, unsigned short nSize, int *outSize );

	void SetLostPacket();

private:
	rtp_hdr_t m_RTP_Header ;

	BYTE *m_pBuf ;

	bool m_bSPSFound ;
	bool m_bWaitKeyFrame ;
	bool m_bAssemblingFrame ;
	bool m_bPrevFrameEnd ;
	BYTE *m_pStart ;
	BYTE *m_pEnd ;
	DWORD m_dwSize ;

	WORD m_wSeq ;

	BYTE m_H264PAYLOADTYPE ;
	DWORD m_ssrc ;
};

// class CH264_RTP_UNPACK end
//////////////////////////////////////////////////////////////////////////////////////////

#endif // __H264_PACK_H__

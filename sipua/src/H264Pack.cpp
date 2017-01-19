/* 
******************************************************************************************************** 
* Copyright (C) 2016, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : H264Pack.cpp
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2017/01/10
* Description  : H264 RTP分包及合包程序
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*///////////////////////////////////////////////////////////////////////////////////////////
// http://blog.csdn.net/dengzikun/article/details/5807694
// class CH264_RTP_PACK start
//#include "stdafx.h"

#include "H264Pack.h"

CH264_RTP_PACK::CH264_RTP_PACK(unsigned long H264SSRC, unsigned char H264PAYLOADTYPE, unsigned short MAXRTPPACKSIZE)
{
	m_MAXRTPPACKSIZE = MAXRTPPACKSIZE ;
	if ( m_MAXRTPPACKSIZE > 10000 )
	{
		m_MAXRTPPACKSIZE = 10000 ;
	}
	if ( m_MAXRTPPACKSIZE < 50 )
	{
		m_MAXRTPPACKSIZE = 50 ;
	}
		
	memset ( &m_RTP_Info, 0, sizeof(m_RTP_Info) ) ;

	m_RTP_Info.rtp_hdr.pt = H264PAYLOADTYPE ;
	m_RTP_Info.rtp_hdr.ssrc = H264SSRC ;
	m_RTP_Info.rtp_hdr.v = RTP_VERSION ;

	m_RTP_Info.rtp_hdr.seq = 0 ;
}

CH264_RTP_PACK::~CH264_RTP_PACK(void)
{
}

//传入Set的数据必须是一个完整的NAL,起始码为0x00000001。
//起始码之前至少预留10个字节，以避免内存COPY操作。
//打包完成后，原缓冲区内的数据被破坏。
bool CH264_RTP_PACK::Set ( unsigned char *NAL_Buf, unsigned long NAL_Size, unsigned long Time_Stamp, bool End_Of_Frame )
{
	unsigned long startcode = StartCode(NAL_Buf) ;
		
	if ( startcode != 0x01000000 )
	{
		return false ;
	}

	int type = NAL_Buf[4] & 0x1f ;
	if ( type < 1 || type > 12 )
	{
		return false ;
	}

	m_RTP_Info.nal.start = NAL_Buf ;
	m_RTP_Info.nal.size = NAL_Size ;
	m_RTP_Info.nal.eoFrame = End_Of_Frame ;
	m_RTP_Info.nal.type = m_RTP_Info.nal.start[4] ;
	m_RTP_Info.nal.end = m_RTP_Info.nal.start + m_RTP_Info.nal.size ;

	m_RTP_Info.rtp_hdr.ts = Time_Stamp ;

	m_RTP_Info.nal.start += 4 ;	// skip the syncword
									
	if ( (m_RTP_Info.nal.size + 7) > m_MAXRTPPACKSIZE )
	{
		m_RTP_Info.FU_flag = true ;
		m_RTP_Info.s_bit = 1 ;
		m_RTP_Info.e_bit = 0 ;

		m_RTP_Info.nal.start += 1 ;	// skip NAL header
	}
	else
	{
		m_RTP_Info.FU_flag = false ;
		m_RTP_Info.s_bit = m_RTP_Info.e_bit = 0 ;
	}
		
	m_RTP_Info.start = m_RTP_Info.end = m_RTP_Info.nal.start ;
	m_bBeginNAL = true ;

	return true ;
}

//循环调用Get获取RTP包，直到返回值为NULL
unsigned char* CH264_RTP_PACK::Get ( unsigned short *pPacketSize )
{
	if ( m_RTP_Info.end == m_RTP_Info.nal.end )
	{
		*pPacketSize = 0 ;
		return NULL ;
	}

	if ( m_bBeginNAL )
	{
		m_bBeginNAL = false ;
	}
	else
	{
		m_RTP_Info.start = m_RTP_Info.end;	// continue with the next RTP-FU packet
	}

	int bytesLeft = m_RTP_Info.nal.end - m_RTP_Info.start ;
	int maxSize = m_MAXRTPPACKSIZE - 12 ;	// sizeof(basic rtp header) == 12 bytes
	if ( m_RTP_Info.FU_flag )
		maxSize -= 2 ;

	if ( bytesLeft > maxSize )
	{
		m_RTP_Info.end = m_RTP_Info.start + maxSize ;	// limit RTP packetsize to 1472 bytes
	}
	else
	{
		m_RTP_Info.end = m_RTP_Info.start + bytesLeft ;
	}

	if ( m_RTP_Info.FU_flag )
	{	// multiple packet NAL slice
		if ( m_RTP_Info.end == m_RTP_Info.nal.end )
		{
			m_RTP_Info.e_bit = 1 ;
		}
	}

	m_RTP_Info.rtp_hdr.m =	m_RTP_Info.nal.eoFrame ? 1 : 0 ; // should be set at EofFrame
	if ( m_RTP_Info.FU_flag && !m_RTP_Info.e_bit )
	{
		m_RTP_Info.rtp_hdr.m = 0 ;
	}

	m_RTP_Info.rtp_hdr.seq++ ;

	unsigned char *cp = m_RTP_Info.start ;
	cp -= ( m_RTP_Info.FU_flag ? 14 : 12 ) ;
	m_RTP_Info.pRTP = cp ;
		
	unsigned char *cp2 = (unsigned char *)&m_RTP_Info.rtp_hdr ;
	cp[0] = cp2[0] ;
	cp[1] = cp2[1] ;

	cp[2] = ( m_RTP_Info.rtp_hdr.seq >> 8 ) & 0xff ;
	cp[3] = m_RTP_Info.rtp_hdr.seq & 0xff ;

	cp[4] = ( m_RTP_Info.rtp_hdr.ts >> 24 ) & 0xff ;
	cp[5] = ( m_RTP_Info.rtp_hdr.ts >> 16 ) & 0xff ;
	cp[6] = ( m_RTP_Info.rtp_hdr.ts >>  8 ) & 0xff ;
	cp[7] = m_RTP_Info.rtp_hdr.ts & 0xff ;

	cp[8] =  ( m_RTP_Info.rtp_hdr.ssrc >> 24 ) & 0xff ;
	cp[9] =  ( m_RTP_Info.rtp_hdr.ssrc >> 16 ) & 0xff ;
	cp[10] = ( m_RTP_Info.rtp_hdr.ssrc >>  8 ) & 0xff ;
	cp[11] = m_RTP_Info.rtp_hdr.ssrc & 0xff ;
	m_RTP_Info.hdr_len = 12 ;
	/*!
	* /n The FU indicator octet has the following format:
	* /n
	* /n      +---------------+
	* /n MSB  |0|1|2|3|4|5|6|7|  LSB
	* /n      +-+-+-+-+-+-+-+-+
	* /n      |F|NRI|  Type   |
	* /n      +---------------+
	* /n
	* /n The FU header has the following format:
	* /n
	* /n      +---------------+
	* /n      |0|1|2|3|4|5|6|7|
	* /n      +-+-+-+-+-+-+-+-+
	* /n      |S|E|R|  Type   |
	* /n      +---------------+
	*/
	if ( m_RTP_Info.FU_flag )
	{
		// FU indicator  F|NRI|Type
		cp[12] = ( m_RTP_Info.nal.type & 0xe0 ) | 28 ;	//Type is 28 for FU_A
		//FU header		S|E|R|Type
		cp[13] = ( m_RTP_Info.s_bit << 7 ) | ( m_RTP_Info.e_bit << 6 ) | ( m_RTP_Info.nal.type & 0x1f ) ; //R = 0, must be ignored by receiver

		m_RTP_Info.s_bit = m_RTP_Info.e_bit= 0 ;
		m_RTP_Info.hdr_len = 14 ;
	}
	m_RTP_Info.start = &cp[m_RTP_Info.hdr_len] ;	// new start of payload

	*pPacketSize = m_RTP_Info.hdr_len + ( m_RTP_Info.end - m_RTP_Info.start ) ;
	return m_RTP_Info.pRTP ;
}

unsigned int CH264_RTP_PACK::StartCode( unsigned char *cp )
{
	unsigned int d32 ;
	d32 = cp[3] ;
	d32 <<= 8 ;
	d32 |= cp[2] ;
	d32 <<= 8 ;
	d32 |= cp[1] ;
	d32 <<= 8 ;
	d32 |= cp[0] ;
	return d32 ;
}

// class CH264_RTP_PACK end
//////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////
// class CH264_RTP_UNPACK start

CH264_RTP_UNPACK::CH264_RTP_UNPACK (unsigned char H264PAYLOADTYPE)
	: m_bSPSFound(false)
	, m_bWaitKeyFrame(true)
	, m_bPrevFrameEnd(false)
	, m_bAssemblingFrame(false)
	, m_wSeq(1234)
	, m_ssrc(0)
{
	m_pBuf = new BYTE[BUF_SIZE] ;

	m_H264PAYLOADTYPE = H264PAYLOADTYPE ;
	m_pEnd = m_pBuf + BUF_SIZE ;
	m_pStart = m_pBuf ;
	m_dwSize = 0 ;
}

CH264_RTP_UNPACK::~CH264_RTP_UNPACK(void)
{
	delete [] m_pBuf ;
}

//pBuf为H264 RTP视频数据包，nSize为RTP视频数据包字节长度，outSize为输出视频数据帧字节长度。
//返回值为指向视频数据帧的指针。输入数据可能被破坏。
BYTE* CH264_RTP_UNPACK::Parse_RTP_Packet ( BYTE *pBuf, unsigned short nSize, int *outSize )
{
	if ( nSize <= 12 )
	{
		return NULL ;
	}

	BYTE *cp = (BYTE*)&m_RTP_Header ;
	memcpy(cp, pBuf, 12);
	/* // ortp做了网络数据到主机数据顺序转换
	cp[0] = pBuf[0] ;
	cp[1] = pBuf[1] ;
	m_RTP_Header.seq = pBuf[2] ;
	m_RTP_Header.seq <<= 8 ;
	m_RTP_Header.seq |= pBuf[3] ;

	m_RTP_Header.ts = pBuf[4] ;
	m_RTP_Header.ts <<= 8 ;
	m_RTP_Header.ts |= pBuf[5] ;
	m_RTP_Header.ts <<= 8 ;
	m_RTP_Header.ts |= pBuf[6] ;
	m_RTP_Header.ts <<= 8 ;
	m_RTP_Header.ts |= pBuf[7] ;

	m_RTP_Header.ssrc = pBuf[8] ;
	m_RTP_Header.ssrc <<= 8 ;
	m_RTP_Header.ssrc |= pBuf[9] ;
	m_RTP_Header.ssrc <<= 8 ;
	m_RTP_Header.ssrc |= pBuf[10] ;
	m_RTP_Header.ssrc <<= 8 ;
	m_RTP_Header.ssrc |= pBuf[11] ;
	*/
	BYTE *pPayload = pBuf + 12 ;
	LONG nPayloadSize = nSize - 12 ;

	// Check the RTP version number (it should be 2):
	if ( m_RTP_Header.v != RTP_VERSION )
	{
		return NULL ;
	}
	
	// Skip over any CSRC identifiers in the header:
	if ( m_RTP_Header.cc )
	{
		long cc = m_RTP_Header.cc * 4 ;
		if ( nPayloadSize < cc )
		{
			return NULL ;
		}

		nPayloadSize -= cc ;
		pPayload += cc ;
	}

	// Check for (& ignore) any RTP header extension
	if ( m_RTP_Header.x )
	{
		if ( nPayloadSize < 4 )
		{
			return NULL ;
		}

		nPayloadSize -= 4 ;
		pPayload += 2 ;
		long l = pPayload[0] ;
		l <<= 8 ;
		l |= pPayload[1] ;
		pPayload += 2 ;
		l *= 4 ;
		if ( nPayloadSize < l )
		{
			return NULL ;
		}
		nPayloadSize -= l ;
		pPayload += l ;
	}
		
	// Discard any padding bytes:
	if ( m_RTP_Header.p )
	{
		if ( nPayloadSize == 0 )
		{
			return NULL ;
		}
		long Padding = pPayload[nPayloadSize-1] ;
		if ( nPayloadSize < Padding )
		{
			return NULL ;
		}
		nPayloadSize -= Padding ;
	}

	// Check the Payload Type.
	// ortp里有检查,跳过
	//if ( m_RTP_Header.pt != m_H264PAYLOADTYPE )
	//{
	//	return NULL ;
	//}

	int PayloadType = pPayload[0] & 0x1f ;
	int NALType = PayloadType ;
	if ( NALType == 28 ) // FU_A
	{
		if ( nPayloadSize < 2 )
		{
			return NULL ;
		}

		NALType = pPayload[1] & 0x1f ;
	}

	if ( m_ssrc != m_RTP_Header.ssrc )
	{
		m_ssrc = m_RTP_Header.ssrc ;
		SetLostPacket () ;
	}
	
	if ( NALType == 0x07 ) // SPS
	{
		m_bSPSFound = true ;
	}

	if ( !m_bSPSFound )
	{
		return NULL ;
	}

	if ( NALType == 0x07 || NALType == 0x08) // SPS PPS
	{
		m_wSeq = m_RTP_Header.seq ;
		m_bPrevFrameEnd = true ;

		pPayload -= 4 ;
		*((DWORD*)(pPayload)) = 0x01000000 ;
		*outSize = nPayloadSize + 4 ;
		return pPayload ;
	}

	if ( m_bWaitKeyFrame )
	{
		if ( m_RTP_Header.m ) // frame end
		{
			m_bPrevFrameEnd = true ;
			if ( !m_bAssemblingFrame )
			{
				m_wSeq = m_RTP_Header.seq ;
				return NULL ;
			}
		}

		if ( !m_bPrevFrameEnd )
		{
			m_wSeq = m_RTP_Header.seq ;
			return NULL ;
		}
		else
		{
			if ( NALType != 0x05 ) // KEY FRAME
			{
				m_wSeq = m_RTP_Header.seq ;
				m_bPrevFrameEnd = false ;
				return NULL ;
			}
		}
	}
	///////////////////////////////////////////////////////////////		
	if ( m_RTP_Header.seq != (WORD)( m_wSeq + 1 ) ) // lost packet
	{
		m_wSeq = m_RTP_Header.seq ;
		SetLostPacket () ;			
		return NULL ;
	}
	else
	{
		// 码流正常

		m_wSeq = m_RTP_Header.seq ;
		m_bAssemblingFrame = true ;
			
		if ( PayloadType != 28 ) // whole NAL
		{
			*((DWORD*)(m_pStart)) = 0x01000000 ;
			m_pStart += 4 ;
			m_dwSize += 4 ;
		}
		else // FU_A
		{
			if ( pPayload[1] & 0x80) // FU_A start
			{
				*((DWORD*)(m_pStart)) = 0x01000000 ;
				m_pStart += 4 ;
				m_dwSize += 4 ;

				pPayload[1] = ( pPayload[0] & 0xE0 ) | NALType ;
					
				pPayload += 1 ;
				nPayloadSize -= 1 ;
			}
			else
			{
				pPayload += 2 ;
				nPayloadSize -= 2 ;
			}
		}

		if ( m_pStart + nPayloadSize < m_pEnd )
		{
			memcpy ( m_pStart, pPayload, nPayloadSize ) ;
			m_dwSize += nPayloadSize ;
			m_pStart += nPayloadSize ;
		}
		else // memory overflow
		{
			SetLostPacket () ;
			return NULL ;
		}

		if ( m_RTP_Header.m) // frame end || NALType == 5
		{
			*outSize = m_dwSize ;

			m_pStart = m_pBuf ;
			m_dwSize = 0 ;

			if ( NALType == 0x05 ) // KEY FRAME
			{
				m_bWaitKeyFrame = false ;
			}
			return m_pBuf ;
		}
		else
		{
			return NULL ;
		}
	}
}

void CH264_RTP_UNPACK::SetLostPacket()
{
	m_bSPSFound = false ;
	m_bWaitKeyFrame = true ;
	m_bPrevFrameEnd = false ;
	m_bAssemblingFrame = false ;
	m_pStart = m_pBuf ;
	m_dwSize = 0 ;
}

// class CH264_RTP_UNPACK end
//////////////////////////////////////////////////////////////////////////////////////////
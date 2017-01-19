#pragma once

# pragma warning (disable:4819)
extern "C"
{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/fifo.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/dict.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avstring.h"
#include "libavutil/imgutils.h"
#include "libavcodec/avcodec.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersrc.h"
#include "libavfilter/buffersink.h"
#include "libswscale/swscale.h"
}

#define H264_DATA_BUF_SIZE			(1024*10)

// CVideoDlg dialog

class CVideoDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVideoDlg)

public:
	CVideoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CVideoDlg();

// Dialog Data
	enum { IDD = IDD_VIDEODLG };

	void SetH264Data(BYTE *pBuf, int nLen);
	void Reset(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	int Decode(BYTE *pDataIn, int nInSize, BYTE *pDataOut, int nOutSize, int *nWidth, int *nHeight);

	DECLARE_MESSAGE_MAP()

private:
	struct AVCodecContext	*m_pCodecCtx;
	struct AVCodec			*m_pCodec;
	struct AVFrame			*m_pFrameSrc;
	struct AVFrame			*m_pFrameDst;
	CBitmap					m_bmpVideo;
	CDC						m_cdcVideo;
	struct SwsContext		*m_pImgConvertCtx;
	BYTE					*m_pRGBBuffer;
	BOOL					m_bFirstFrame;
	int						m_nWidth;
	int						m_nHeight;
	BYTE					m_pH264Data[H264_DATA_BUF_SIZE];
	int						m_nH264DataSize;
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};

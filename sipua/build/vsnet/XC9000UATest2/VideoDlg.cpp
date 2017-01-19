// VideoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XC9000UATest2.h"
#include "VideoDlg.h"
#include "afxdialogex.h"
#include "MemDC.h"

#include "ua_port.h"
#include "log.h"

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "avutil.lib")

# pragma warning (disable:4996)

// CVideoDlg dialog

IMPLEMENT_DYNAMIC(CVideoDlg, CDialogEx)

CVideoDlg::CVideoDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVideoDlg::IDD, pParent)
{
	char	p_strerr[PATH_MAX] = {0};

	m_pImgConvertCtx = NULL;
	m_pRGBBuffer = NULL;
	m_nWidth = m_nHeight = 0;
	m_bFirstFrame = TRUE;
	m_nH264DataSize = 0;
	try
	{
		av_register_all();
		m_pCodec = avcodec_find_decoder(AV_CODEC_ID_H264);
		if (m_pCodec == NULL)
		{
			sprintf(p_strerr, "[CVideoDlg::CVideoDlg(avcodec_find_decoder)] : can't find h264 decoder!\n");
			throw 10;
		}
		m_pCodecCtx = avcodec_alloc_context3(m_pCodec);
		if (m_pCodecCtx == NULL)
		{
			sprintf(p_strerr, "[CVideoDlg::CVideoDlg(avcodec_alloc_context3)] : return NULL!\n");
			throw 11;
		}
		m_pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
		m_pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
		if (avcodec_open2(m_pCodecCtx, m_pCodec, NULL) < 0)
		{
			sprintf(p_strerr, "[CVideoDlg::CVideoDlg(avcodec_open2)] : return error!\n");
			throw 12;
		}
		if(m_pCodecCtx->time_base.num > 1000 && m_pCodecCtx->time_base.den == 1)
			m_pCodecCtx->time_base.den = 1000;
		m_pFrameSrc = av_frame_alloc();
		if (m_pFrameSrc == NULL)
		{
			sprintf(p_strerr, "[CVideoDlg::CVideoDlg(av_frame_alloc)] : return NULL!\n");
			throw 13;
		}
		m_pFrameDst = av_frame_alloc();
		if (m_pFrameDst == NULL)
		{
			sprintf(p_strerr, "[CVideoDlg::CVideoDlg(av_frame_alloc)] : return NULL!\n");
			throw 14;
		}
	}
	catch (int throw_err)
	{
		switch (throw_err)
		{
        case 10:
		default:
			break;
		}
		if (strlen(p_strerr) > 0)
			printf_log(LOG_IS_ERR, "%s", p_strerr);
	}
	catch (std::exception &e)
    {   
		printf_log(LOG_IS_ERR, "[CVideoDlg::CVideoDlg()] : %s", e.what());
    }
	catch (...)
    {   
		printf_log(LOG_IS_ERR, "[CVideoDlg::CVideoDlg()] : error catch!\n");
    }
}

CVideoDlg::~CVideoDlg()
{
	if (m_pImgConvertCtx != NULL)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}
	if (m_pRGBBuffer != NULL)
	{
		av_free(m_pRGBBuffer);
		m_pRGBBuffer = NULL;
	}
	//if (m_cdcVideo.m_hDC != NULL)
	//	ReleaseDC(&m_cdcVideo);
	//m_cdcVideo.DeleteDC();
	m_bmpVideo.DeleteObject();
	if (m_pCodecCtx != NULL)
	{
		avcodec_close(m_pCodecCtx);
		av_free(m_pCodecCtx);
		m_pCodecCtx = NULL;
	}
	if (m_pFrameSrc != NULL)
	{
		av_frame_free(&m_pFrameSrc);
		m_pFrameSrc = NULL;
	}
	if (m_pFrameDst != NULL)
	{
		av_frame_free(&m_pFrameDst);
		m_pFrameDst = NULL;
	}
}

void CVideoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CVideoDlg, CDialogEx)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

// CVideoDlg message handlers
#define CIF_W		352
#define CIF_H		288
#define CIF_RGB		(CIF_W * CIF_H * 3)
void CVideoDlg::SetH264Data(BYTE *pBuf, int nLen)
{
	BYTE		pDataOut[CIF_RGB];
	int			nWidth = 0, nHeight = 0;

	if (m_hWnd != NULL)
	{
		if (Decode(pBuf, nLen, pDataOut, CIF_RGB, &nWidth, &nHeight) == 0)
		{
			BITMAPINFOHEADER	bmih;
			HBITMAP				hBitmap;
			BYTE				*pBits = NULL;

			bmih.biSize					= sizeof (BITMAPINFOHEADER) ;
			bmih.biWidth				= m_nWidth;
			bmih.biHeight				= m_nHeight;
			bmih.biPlanes				= 1 ;
			bmih.biBitCount				= 24 ;
			bmih.biCompression			= BI_RGB ;
			bmih.biSizeImage			= bmih.biWidth * bmih.biHeight * (bmih.biBitCount / 8);
			bmih.biXPelsPerMeter		= 0 ;
			bmih.biYPelsPerMeter		= 0 ;
			bmih.biClrUsed				= 0 ;
			bmih.biClrImportant			= 0 ;			
			hBitmap = CreateDIBSection (NULL, (BITMAPINFO *)&bmih, DIB_RGB_COLORS, (void **)&pBits, NULL, 0) ;
			if (hBitmap != NULL)
			{
				memcpy(pBits, m_pRGBBuffer, bmih.biSizeImage);
				m_bmpVideo.Detach();
				m_bmpVideo.Attach(hBitmap);
				Invalidate();
			}
		}
	}
}

static void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[32];
    int  y;
 
    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;
 
    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
 
    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
 
    // Close file
    fclose(pFile);
}

int CVideoDlg::Decode(BYTE *pDataIn, int nInSize, BYTE *pDataOut, int nOutSize, int *pnWidth, int *pnHeight)
{
	int			nResult = -1, y;
	AVPacket	sPacket;
	int			nGotPicture = 0;
	int			nComsumedSize = 0;
	static int	nSaveCount = 0;

	if (m_pCodecCtx != NULL && m_pFrameSrc != NULL && m_pFrameDst != NULL)
	{
		av_init_packet(&sPacket);
		if (m_nH264DataSize + nInSize > H264_DATA_BUF_SIZE)
			m_nH264DataSize = 0;
		if (m_nH264DataSize + nInSize <= H264_DATA_BUF_SIZE)
		{
			memcpy(&m_pH264Data[m_nH264DataSize], pDataIn, nInSize);
			m_nH264DataSize += nInSize;
		}
		sPacket.size = m_nH264DataSize;
		sPacket.data = m_pH264Data;
		
		while (sPacket.size > 0)
		{
			nComsumedSize = avcodec_decode_video2(m_pCodecCtx, m_pFrameSrc, &nGotPicture, &sPacket);
			if (nComsumedSize < 0)
				break;
			else
			{
				if (sPacket.size >= nComsumedSize)
				{
					sPacket.size -= nComsumedSize;
					sPacket.data += nComsumedSize;
				}
				else
					sPacket.size = 0;
			}
			if (nGotPicture != 0 \
				&& m_pCodecCtx->width == m_pFrameSrc->width \
				&& m_pCodecCtx->height == m_pFrameSrc->height)
			{
				if (m_bFirstFrame)
				{
					if (m_pImgConvertCtx != NULL)
					{
						sws_freeContext(m_pImgConvertCtx);
						m_pImgConvertCtx = NULL;
					}
					if (m_pRGBBuffer != NULL)
					{
						av_free(m_pRGBBuffer);
						m_pRGBBuffer = NULL;
					}
					m_nWidth = m_pCodecCtx->width;
					m_nHeight = m_pCodecCtx->height;
					CRect		rcWindow;
					GetWindowRect(&rcWindow);
					SetWindowPos(NULL, rcWindow.left, rcWindow.top, m_nWidth + 100, m_nHeight + 80, SWP_SHOWWINDOW);
					m_pImgConvertCtx = sws_getContext(m_nWidth, m_nHeight, m_pCodecCtx->pix_fmt, \
						m_nWidth, m_nHeight, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
					m_pRGBBuffer = (BYTE *)av_malloc(avpicture_get_size(AV_PIX_FMT_RGB24, m_nWidth, m_nHeight));
					if (m_pRGBBuffer != NULL)
						avpicture_fill((AVPicture *)m_pFrameDst, m_pRGBBuffer, AV_PIX_FMT_RGB24, m_nWidth, m_nHeight);
					m_bFirstFrame = FALSE;
				}
				*pnWidth = m_pCodecCtx->width;
				*pnHeight = m_pCodecCtx->height;
				if (m_pImgConvertCtx != NULL && m_pRGBBuffer != NULL)
				{
					sws_scale(m_pImgConvertCtx, m_pFrameSrc->data, m_pFrameSrc->linesize, 0, \
						m_pCodecCtx->height, m_pFrameDst->data, m_pFrameDst->linesize);
					nResult = 0;
					//if (nSaveCount++ < 5)
					//	SaveFrame(m_pFrameDst, m_nWidth, m_nHeight, nSaveCount);

					BYTE	*pTemp = (BYTE *)new char[m_pFrameDst->linesize[0]];
					for (y = 0; y < m_nHeight / 2; y++)
					{
						memcpy(pTemp, m_pFrameDst->data[0] + y * m_pFrameDst->linesize[0], m_pFrameDst->linesize[0]);
						memcpy(m_pFrameDst->data[0] + y * m_pFrameDst->linesize[0], m_pFrameDst->data[0] + (m_nHeight - 1 - y) * m_pFrameDst->linesize[0], m_pFrameDst->linesize[0]);
						memcpy(m_pFrameDst->data[0] + (m_nHeight - 1 - y) * m_pFrameDst->linesize[0], pTemp, m_pFrameDst->linesize[0]);
					}
					delete pTemp;
					pTemp = NULL;
				}
			}
			av_free_packet(&sPacket);
		}

		if (sPacket.size > 0 && m_pH264Data != sPacket.data)
			memcpy(m_pH264Data, sPacket.data, sPacket.size);
		m_nH264DataSize = sPacket.size;
	}

	return nResult;
}

void CVideoDlg::Reset(void)
{
	m_bFirstFrame = TRUE;
	if (m_pImgConvertCtx != NULL)
	{
		sws_freeContext(m_pImgConvertCtx);
		m_pImgConvertCtx = NULL;
	}
	if (m_pRGBBuffer != NULL)
	{
		av_free(m_pRGBBuffer);
		m_pRGBBuffer = NULL;
	}
	m_nWidth = m_nHeight = 0;
}

BOOL CVideoDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	if (m_bmpVideo.m_hObject != NULL && pDC != NULL)
	{
		CRect		rect;
		int			nWidth, nHeight;

		// »­±³¾°É«
		CMemDCEx	cdcBkgnd(pDC);
		GetClientRect(&rect);
		nWidth = rect.Width();
		nHeight = rect.Height();
		//cdcBkgnd.FillSolidRect(0, 0, nWidth, nHeight, RGB(0, 0, 0));
		// »­±³¾°Í¼
		CDC			cdcVideo;
		CBitmap		*pOldVideo = NULL;
		cdcVideo.CreateCompatibleDC(pDC);
		pOldVideo = cdcVideo.SelectObject(&m_bmpVideo);
		cdcBkgnd.BitBlt(0, 0, m_nWidth, m_nHeight, &cdcVideo, 0, 0, SRCCOPY);
		cdcVideo.SelectObject(pOldVideo);
	}
	return TRUE;
}

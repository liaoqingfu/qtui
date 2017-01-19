
// XC9000UATest2Dlg.h : 头文件
//

#pragma once

#include "ResizingDialog.h"
#include "SipUA.h"
#include "VideoDlg.h"
#include "VideoFile.h"

#define H264_RECV_BUF_SIZE  VIDEO_RTP_FRAME_LEN_MAX

// CXC9000UATest2Dlg 对话框
class CXC9000UATest2Dlg : public CResizingDialog
{
// 构造
public:
	CXC9000UATest2Dlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_XC9000UATEST2_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	void InitControl(void);
	void ReadConfig(void);
	void WriteConfig(void);
	static void LogCallback(int nLogLevel, char * pLog, void * pParam);
	static void sip_event_callback(eXosip_event_t *p_event, void *p_param);
	static void ua_event_callback(CSipUA *p_ua, void *p_param);
	static void video_recvdata_callback(void *p_param);
	void SetListSelectFile(int nIndex);
	void UpdatePlayTime(DWORD dwTimePlayed, DWORD dwTimeTotal);

public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnClickedBtnAddFile();
	afx_msg void OnClickedBtnClearLog();
	afx_msg void OnClickedBtnModifyVolume();
	afx_msg void OnClickedBtnMonitor();
	afx_msg void OnClickedBtnNextFile();
	afx_msg void OnClickedBtnPlayFile();
	afx_msg void OnClickedBtnPreviousFile();
	afx_msg void OnClickedBtnRegister();
	afx_msg void OnClickedBtnRemoveFile();
	afx_msg void OnClickedBtnStopFile();
	afx_msg void OnClickedBtnTalk();
	afx_msg void OnClickedRadioSourceFile();
	afx_msg void OnClickedRadioFormatMp3();
	afx_msg void OnRadioFormatPcm();
	afx_msg void OnRadioSourceSoundCard();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClickedBtnUnregister();
	afx_msg void OnClickedBtnAnswer();
	afx_msg void OnClickedBtnHangUp();
	afx_msg void OnDblclkListFile(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnClickedBtnAlarmOut();
	CSpinButtonCtrl m_spinAlarmOutNo;
	CComboBox m_comboAlarmOut;
	UINT m_nAlarmOutNo;
	UINT m_nFSPort;
	CEdit m_editLog;
	CIPAddressCtrl m_cFSIpAddr;
	CString m_strLog;
	CString m_strReceiveID;
	UINT m_nVolume;
	CListCtrl m_listFile;
	int m_nFileFormat;
	int m_nAudioSource;
	CSliderCtrl m_cPlayProgress;
	CString m_strTimePlayed;
	CString m_strTimeRest;
	CString m_strID;
	CSpinButtonCtrl m_spinVolume;

private:
	DWORD				m_dwFSIpAddr;
	CSipUA				m_cSipUA;
	CVideoDlg			m_dlgVideo;
	CVideoFile			m_cFileVideo;

public:
	afx_msg void OnDestroy();
};

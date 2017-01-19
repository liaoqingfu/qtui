
// XC9000UATest2Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "XC9000UATest2.h"
#include "XC9000UATest2Dlg.h"
#include "afxdialogex.h"
#include "Defines.h"
#include "log.h"
#include "socket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CXC9000UATest2Dlg 对话框
#define LIST_FILE_LABEL_COUNT		2
#define LIST_FILE_COLUMN_FILE		0
#define LIST_FILE_COLUMN_PATH		1
const UINT		g_pListFileColumnLabel[LIST_FILE_LABEL_COUNT] = {
	IDS_MAIN_LIST_USER_LABEL1,
	IDS_MAIN_LIST_USER_LABEL2,
};

const short g_pListFileColumnWidth[LIST_FILE_LABEL_COUNT] = {
	200, 800
};



CXC9000UATest2Dlg::CXC9000UATest2Dlg(CWnd* pParent /*=NULL*/)
	: CResizingDialog(CXC9000UATest2Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nFSPort = 5060;
	m_strLog = _T("");
	m_strReceiveID = _T("1044");
	m_nVolume = 9;
	m_nFileFormat = 0;
	m_nAudioSource = 0;
	m_strTimePlayed = _T("00:00:00");
	m_strTimeRest = _T("00:00:00");
	m_dwFSIpAddr = 0xC0A8BA0D;	// 0xC0A8BA0D : 192.168.186.13
	m_strID = _T("1015");
	m_nAlarmOutNo = 1;
}

void CXC9000UATest2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CResizingDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MAIN_EDIT_FS_PORT, m_nFSPort);
	DDV_MinMaxUInt(pDX, m_nFSPort, 1, 65535);
	DDX_Control(pDX, IDC_MAIN_EDIT_LOG, m_editLog);
	DDX_Control(pDX, IDC_MAIN_IPADDR_FS_IP, m_cFSIpAddr);
	DDX_Text(pDX, IDC_MAIN_EDIT_LOG, m_strLog);
	DDX_Text(pDX, IDC_MAIN_EDIT_RECEIVE_ID, m_strReceiveID);
	DDX_Text(pDX, IDC_MAIN_EDIT_VOLUME, m_nVolume);
	DDV_MinMaxUInt(pDX, m_nVolume, 0, 15);
	DDX_Control(pDX, IDC_MAIN_LIST_FILE, m_listFile);
	DDX_Radio(pDX, IDC_MAIN_RADIO_FORMAT_MP3, m_nFileFormat);
	DDX_Radio(pDX, IDC_MAIN_RADIO_SOURCE_FILE, m_nAudioSource);
	DDX_Control(pDX, IDC_MAIN_SLIDER_PROGRESS, m_cPlayProgress);
	DDX_Text(pDX, IDC_MAIN_STATIC_TIME_PLAYED, m_strTimePlayed);
	DDX_Text(pDX, IDC_MAIN_STATIC_TIME_REST, m_strTimeRest);
	DDX_Text(pDX, IDC_MAIN_EDIT_ID, m_strID);
	DDX_Control(pDX, IDC_MAIN_SPIN_VOLUME, m_spinVolume);
	DDX_Control(pDX, IDC_MAIN_SPIN_ALARM_OUT_NO, m_spinAlarmOutNo);
	DDX_Control(pDX, IDC_MAIN_COMBO_ALARM_OUT, m_comboAlarmOut);
	DDX_Text(pDX, IDC_MAIN_EDIT_ALARM_OUT_NO, m_nAlarmOutNo);
	DDV_MinMaxUInt(pDX, m_nAlarmOutNo, 1, 999);
}

BEGIN_MESSAGE_MAP(CXC9000UATest2Dlg, CResizingDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CXC9000UATest2Dlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CXC9000UATest2Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_MAIN_BTN_ADD_FILE, &CXC9000UATest2Dlg::OnClickedBtnAddFile)
	ON_BN_CLICKED(IDC_MAIN_BTN_CLEAR_LOG, &CXC9000UATest2Dlg::OnClickedBtnClearLog)
	ON_BN_CLICKED(IDC_MAIN_BTN_MODIFY_VOLUME, &CXC9000UATest2Dlg::OnClickedBtnModifyVolume)
	ON_BN_CLICKED(IDC_MAIN_BTN_MONITOR, &CXC9000UATest2Dlg::OnClickedBtnMonitor)
	ON_BN_CLICKED(IDC_MAIN_BTN_NEXT_FILE, &CXC9000UATest2Dlg::OnClickedBtnNextFile)
	ON_BN_CLICKED(IDC_MAIN_BTN_PLAY_FILE, &CXC9000UATest2Dlg::OnClickedBtnPlayFile)
	ON_BN_CLICKED(IDC_MAIN_BTN_PREVIOUS_FILE, &CXC9000UATest2Dlg::OnClickedBtnPreviousFile)
	ON_BN_CLICKED(IDC_MAIN_BTN_REGISTER, &CXC9000UATest2Dlg::OnClickedBtnRegister)
	ON_BN_CLICKED(IDC_MAIN_BTN_REMOVE_FILE, &CXC9000UATest2Dlg::OnClickedBtnRemoveFile)
	ON_BN_CLICKED(IDC_MAIN_BTN_STOP_FILE, &CXC9000UATest2Dlg::OnClickedBtnStopFile)
	ON_BN_CLICKED(IDC_MAIN_BTN_TALK, &CXC9000UATest2Dlg::OnClickedBtnTalk)
	ON_BN_CLICKED(IDC_MAIN_RADIO_SOURCE_FILE, &CXC9000UATest2Dlg::OnClickedRadioSourceFile)
	ON_BN_CLICKED(IDC_MAIN_RADIO_FORMAT_MP3, &CXC9000UATest2Dlg::OnClickedRadioFormatMp3)
	ON_COMMAND(IDC_MAIN_RADIO_FORMAT_PCM, &CXC9000UATest2Dlg::OnRadioFormatPcm)
	ON_COMMAND(IDC_MAIN_RADIO_SOURCE_SOUND_CARD, &CXC9000UATest2Dlg::OnRadioSourceSoundCard)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_MAIN_BTN_UNREGISTER, &CXC9000UATest2Dlg::OnClickedBtnUnregister)
	ON_BN_CLICKED(IDC_MAIN_BTN_ANSWER, &CXC9000UATest2Dlg::OnClickedBtnAnswer)
	ON_BN_CLICKED(IDC_MAIN_BTN_HANG_UP, &CXC9000UATest2Dlg::OnClickedBtnHangUp)
	ON_NOTIFY(NM_DBLCLK, IDC_MAIN_LIST_FILE, &CXC9000UATest2Dlg::OnDblclkListFile)
	ON_BN_CLICKED(IDC_MAIN_BTN_ALARM_OUT, &CXC9000UATest2Dlg::OnClickedBtnAlarmOut)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CXC9000UATest2Dlg 消息处理程序

BOOL CXC9000UATest2Dlg::OnInitDialog()
{
	CResizingDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitControl();

	m_dlgVideo.Create(IDD_VIDEODLG, this);
	CRect		rcMain, rcVideo;
	GetWindowRect(&rcMain);
	m_dlgVideo.GetWindowRect(&rcVideo);
	m_dlgVideo.SetWindowPos(NULL, rcMain.right - rcVideo.Width(), \
		rcMain.bottom - rcVideo.Height(), \
		rcVideo.Width(), rcVideo.Height(), SWP_HIDEWINDOW);;
	m_cFileVideo.add_file("r:/test.264");
	m_cFileVideo.play();
	m_cFileVideo.set_play_index(0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CXC9000UATest2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CResizingDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CXC9000UATest2Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CResizingDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CXC9000UATest2Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CXC9000UATest2Dlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	g_log.set_callback(NULL, NULL);
	m_cSipUA.set_sip_event_callback(NULL, NULL);
	m_cSipUA.set_ua_event_callback(NULL, NULL);
	m_cSipUA.set_video_recvdata_callback(NULL, NULL);
	m_cSipUA.task_end();
	WriteConfig();
	CResizingDialog::OnCancel();
}

void CXC9000UATest2Dlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//CResizingDialog::OnOK();
}

void CXC9000UATest2Dlg::InitControl(void)
{
	SetControlInfo(IDC_MAIN_LIST_FILE, RESIZE_BOTH);
	SetControlInfo(IDC_MAIN_STATIC_BROADCAST, RESIZE_BOTH);

	m_listFile.SetExtendedStyle(m_listFile.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	CString		strText;
	for (int i = 0; i < LIST_FILE_LABEL_COUNT; i++)
	{
		strText.LoadString(g_pListFileColumnLabel[i]);
		m_listFile.InsertColumn(i, strText.GetBuffer(0), LVCFMT_LEFT, g_pListFileColumnWidth[i], i);
		strText.ReleaseBuffer();
	}
	m_spinVolume.SetRange32(0, 15);
	m_spinAlarmOutNo.SetRange32(1, 999);
	m_comboAlarmOut.AddString(CString(LPCTSTR(IDS_MAIN_COMBO_ALARM_OUT_OFF)));
	m_comboAlarmOut.AddString(CString(LPCTSTR(IDS_MAIN_COMBO_ALARM_OUT_ON)));
	m_comboAlarmOut.SetCurSel(0);
	ReadConfig();
	g_log.set_callback(LogCallback, this);
	m_cSipUA.set_sip_event_callback(this->sip_event_callback, this);
	m_cSipUA.set_ua_event_callback(this->ua_event_callback, this);
	m_cSipUA.set_video_recvdata_callback(this->video_recvdata_callback, this);
	m_cSipUA.set_enable_rtp_video(TRUE);
	OnClickedBtnRegister();
	//_CrtSetBreakAlloc(4460);
}

#define MAX_FILE		50
void CXC9000UATest2Dlg::OnClickedBtnAddFile()
{
	// TODO: Add your control notification handler code here
	CString			sFilter = _T("audio file(*.mp3;*.wav)|*.mp3;*.wav||");
	CFileDialog		fileDialog(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_HIDEREADONLY, (LPCTSTR)sFilter, this);
	TCHAR			*pPathBuf = NULL;
	POSITION		pos = NULL;
	CString			strSrcFile;
	CString			strText;
	int				nCount, nItem, nFind;

	fileDialog.m_ofn.nMaxFile = MAX_FILE * MAX_PATH;
	pPathBuf = new TCHAR[fileDialog.m_ofn.nMaxFile];
	ZeroMemory(pPathBuf, sizeof (TCHAR) * fileDialog.m_ofn.nMaxFile);
	fileDialog.m_ofn.lpstrFile = pPathBuf;
	if (fileDialog.DoModal() == IDOK)
	{
		pos = fileDialog.GetStartPosition();
		while (pos != NULL)
		{
			strSrcFile = fileDialog.GetNextPathName(pos);
			nCount = m_listFile.GetItemCount();
			strText = strSrcFile;
			nFind = strText.ReverseFind('\\');
			if (nFind >= 0)
				strText = strText.Right(strText.GetLength() - (nFind + 1));
			nItem = m_listFile.InsertItem(nCount + 1, strText);
			m_listFile.SetItemText(nItem, LIST_FILE_COLUMN_PATH, strSrcFile);
		}
	}
	delete pPathBuf;
}

void CXC9000UATest2Dlg::OnClickedBtnClearLog()
{
	// TODO: Add your control notification handler code here
	m_strLog.Empty();
	UpdateData(FALSE);
}

void CXC9000UATest2Dlg::OnClickedBtnModifyVolume()
{
	// TODO: Add your control notification handler code here
	if (UpdateData())
	{
		TCHAR		pVolume[20];
		_stprintf(pVolume, "%d", m_nVolume);
		m_cSipUA.send_msg_volume(m_strReceiveID.GetBuffer(0), pVolume, pVolume, pVolume, pVolume);
		m_strReceiveID.ReleaseBuffer();
	}
}

void CXC9000UATest2Dlg::OnClickedBtnMonitor()
{
	// TODO: Add your control notification handler code here
	m_cSipUA.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
	if (UpdateData())
	{
		m_cSipUA.monitor(m_strReceiveID.GetBuffer(0), NULL, NULL);
		m_strReceiveID.ReleaseBuffer();
	}
}

void CXC9000UATest2Dlg::OnClickedBtnNextFile()
{
	// TODO: Add your control notification handler code here
	int			nSelect = m_cSipUA.m_audio_stream.m_audio_file.next();
	SetListSelectFile(nSelect);
}

void CXC9000UATest2Dlg::OnClickedBtnPlayFile()
{
	// TODO: Add your control notification handler code here
	TCHAR			pAudioType[10];
	int				nCount, i;
	CString			strPath;
	BOOL			bNext = TRUE;

	if (UpdateData())
	{
		if (m_cSipUA.m_task_type == SIP_TASK_TYPE_NULL)
		{
			if (m_nAudioSource == 0)
			{
				m_cSipUA.m_audio_stream.m_audio_file.remove_all_file();
				nCount = m_listFile.GetItemCount();
				if (nCount <= 0)
				{
					MessageBox(CString((LPCTSTR)IDS_MAIN_PROMPT_ADD_FILE), CString((LPCTSTR)IDS_MAIN_PROMPT));
					bNext = FALSE;
				}
				for (i = 0; i < nCount; i++)
				{
					strPath = m_listFile.GetItemText(i, LIST_FILE_COLUMN_PATH);
					m_cSipUA.m_audio_stream.m_audio_file.add_file(strPath.GetBuffer(0));
					strPath.ReleaseBuffer();
				}

				m_cSipUA.m_audio_stream.set_audio_src(AUDIO_SRC_FILE);
				if (m_nFileFormat == 0)
					_stprintf(pAudioType, _T("%s"), _T("MP3"));
				else
					_stprintf(pAudioType, _T("%s"), _T("WAV"));
			}
			else
			{
				m_cSipUA.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
				if (m_nFileFormat == 0)
					_stprintf(pAudioType, _T("%s"), _T("MP3"));
				else
					_stprintf(pAudioType, _T("%s"), _T("WAV"));
			}
			if (bNext)
			{
				if (m_cSipUA.broadcast(m_strReceiveID.GetBuffer(0), pAudioType, TRUE))
				{
					SetListSelectFile(0);
					GetDlgItem(IDC_MAIN_BTN_PLAY_FILE)->SetWindowText(CString((LPCTSTR)IDS_MAIN_PAUSE));
				}
				m_strReceiveID.ReleaseBuffer();
			}
		}
		else
		{
			if (m_cSipUA.m_task_type == SIP_TASK_TYPE_BC_OUTGOING && m_cSipUA.m_status == SIP_STATUS_BCING)
			{
				if (m_cSipUA.m_audio_stream.m_audio_file.get_play_status() == PLAY_STATUS_PLAYING)
				{
					m_cSipUA.m_audio_stream.m_audio_file.pause();
					GetDlgItem(IDC_MAIN_BTN_PLAY_FILE)->SetWindowText(CString((LPCTSTR)IDS_MAIN_PLAY));
				}
				else
				{
					m_cSipUA.m_audio_stream.m_audio_file.play();
					GetDlgItem(IDC_MAIN_BTN_PLAY_FILE)->SetWindowText(CString((LPCTSTR)IDS_MAIN_PAUSE));
				}
			}
		}
	}
}

void CXC9000UATest2Dlg::OnClickedBtnPreviousFile()
{
	// TODO: Add your control notification handler code here
	int			nSelect = m_cSipUA.m_audio_stream.m_audio_file.previous();
	SetListSelectFile(nSelect);
}

#define SIP_LOCAL_PORT			5060
#define SIP_SERVER_PASSWORD		_T("1234")
void CXC9000UATest2Dlg::OnClickedBtnRegister()
{
	// TODO: Add your control notification handler code here
	CSocketEx			socket_spon;
	CString				strIp;
	if (UpdateData())
	{
		m_cSipUA.set_local_addr(socket_spon.get_first_hostaddr(), SIP_LOCAL_PORT + atoi(m_strID.GetBuffer(0)));
		m_cFSIpAddr.GetAddress(m_dwFSIpAddr);
		strIp.Format(_T("%d.%d.%d.%d"), BYTE4(m_dwFSIpAddr), BYTE3(m_dwFSIpAddr), BYTE2(m_dwFSIpAddr), BYTE1(m_dwFSIpAddr));
		m_cSipUA.set_register_addr(strIp.GetBuffer(0), m_nFSPort);
		strIp.ReleaseBuffer();
		m_cSipUA.set_username_password(m_strID.GetBuffer(0), SIP_SERVER_PASSWORD);
		m_strID.ReleaseBuffer();
		m_cSipUA.m_audio_stream.set_enable_sound_card(TRUE);
		m_cSipUA.init();
		m_cSipUA.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
	}
}

void CXC9000UATest2Dlg::OnClickedBtnUnregister()
{
	// TODO: Add your control notification handler code here
	m_cSipUA.unregister_server();
}

void CXC9000UATest2Dlg::OnClickedBtnRemoveFile()
{
	// TODO: Add your control notification handler code here
	POSITION			pos = m_listFile.GetFirstSelectedItemPosition();
	int					nItem, nCount, i;
	CArray<int, int&>	arrSel;

	while (pos != NULL)
	{
		nItem = m_listFile.GetNextSelectedItem(pos);
		arrSel.Add(nItem);
	}
	nCount = arrSel.GetCount();
	for (i = nCount - 1; i >= 0; i--)
	{
		nItem = arrSel.GetAt(i);
		m_listFile.DeleteItem(nItem);
	}
}

void CXC9000UATest2Dlg::OnClickedBtnStopFile()
{
	// TODO: Add your control notification handler code here
	m_cSipUA.task_end();
	GetDlgItem(IDC_MAIN_BTN_PLAY_FILE)->SetWindowText(CString((LPCTSTR)IDS_MAIN_PLAY));
}

void CXC9000UATest2Dlg::OnClickedBtnTalk()
{
	// TODO: Add your control notification handler code here
	m_cSipUA.m_audio_stream.set_audio_src(AUDIO_SRC_SOUNDCARD);
	if (UpdateData())
	{
		m_cFileVideo.set_play_index(0);
		m_cSipUA.talk(m_strReceiveID.GetBuffer(0), NULL, NULL);
		m_strReceiveID.ReleaseBuffer();
		m_dlgVideo.ShowWindow(SW_SHOW);
		m_dlgVideo.ShowWindow(SW_SHOWNORMAL);
		m_dlgVideo.Reset();
	}
}

void CXC9000UATest2Dlg::OnClickedRadioSourceFile()
{
	// TODO: Add your control notification handler code here
}

void CXC9000UATest2Dlg::OnClickedRadioFormatMp3()
{
	// TODO: Add your control notification handler code here
}

void CXC9000UATest2Dlg::OnRadioFormatPcm()
{
	// TODO: Add your command handler code here
}

void CXC9000UATest2Dlg::OnRadioSourceSoundCard()
{
	// TODO: Add your command handler code here
}

void CXC9000UATest2Dlg::ReadConfig(void)
{
	CString				strPath = GetApplicationPath() + FILE_CONFIG;
	TCHAR				pStr[MAX_PATH];
	int					nFileCount, i, nItem, nFind;
	CString				strTemp, strKey;

	UINT		nShowCmd = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("showCmd"), SW_SHOWDEFAULT, strPath);
	if (nShowCmd == SW_SHOWMAXIMIZED)
		ShowWindow(SW_SHOWMAXIMIZED);
	else if (nShowCmd == SW_SHOWMINIMIZED)
		ShowWindow(SW_SHOWNORMAL);
	else
	{
		CRect		rcWnd;
		ShowWindow(SW_SHOW);
		GetWindowRect(&rcWnd);
		rcWnd.left = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("left"), rcWnd.left, strPath);
		rcWnd.right = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("right"), rcWnd.right, strPath);
		rcWnd.top = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("top"), rcWnd.top, strPath);
		rcWnd.bottom = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("bottom"), rcWnd.bottom, strPath);
		MoveWindow(&rcWnd);
	}

	m_dwFSIpAddr = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("m_dwFSIpAddr"), m_dwFSIpAddr, strPath);
	m_cFSIpAddr.SetAddress(m_dwFSIpAddr);
	m_nFSPort = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("m_nFSPort"), m_nFSPort, strPath);
	GetPrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_strID"), m_strID, pStr, MAX_PATH, strPath);
	m_strID = pStr;
	GetPrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_strReceiveID"), m_strReceiveID, pStr, MAX_PATH, strPath);
	m_strReceiveID = pStr;
	m_nVolume = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("m_nVolume"), m_nVolume, strPath);
	m_nAudioSource = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("m_nAudioSource"), m_nAudioSource, strPath);
	m_nFileFormat = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("m_nFileFormat"), m_nFileFormat, strPath);

	m_listFile.DeleteAllItems();
	nFileCount = GetPrivateProfileInt(_T("CONFIG_MAIN_WND"), _T("nFileCount"), 0, strPath);
	for (i = 0; i < nFileCount; i++)
	{
		strKey.Format(_T("ListFile%03d"), i + 1);
		GetPrivateProfileString(_T("CONFIG_MAIN_WND"), strKey, _T(""), pStr, MAX_PATH, strPath);
		strTemp = pStr;
		nFind = strTemp.ReverseFind('\\');
		if (nFind >= 0)
			strTemp = strTemp.Right(strTemp.GetLength() - (nFind + 1));
		nItem = m_listFile.InsertItem(i + 1, strTemp);
		strTemp = pStr;
		m_listFile.SetItemText(nItem, LIST_FILE_COLUMN_PATH, strTemp);
	}

	UpdateData(FALSE);
}

void CXC9000UATest2Dlg::WriteConfig(void)
{
	CString				strPath = GetApplicationPath() + FILE_CONFIG;
	CString				strTemp, strKey;
	int					nFileCount, i;
	WINDOWPLACEMENT		wndpl;
	CRect				rcWnd;

	UpdateData();

	GetWindowPlacement(&wndpl);
	strTemp.Format(_T("%d"), wndpl.showCmd);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("showCmd"), strTemp, strPath);
	GetWindowRect(&rcWnd);
	strTemp.Format(_T("%d"), rcWnd.left);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("left"), strTemp, strPath);
	strTemp.Format(_T("%d"), rcWnd.right);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("right"), strTemp, strPath);
	strTemp.Format(_T("%d"), rcWnd.top);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("top"), strTemp, strPath);
	strTemp.Format(_T("%d"), rcWnd.bottom);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("bottom"), strTemp, strPath);

	m_cFSIpAddr.GetAddress(m_dwFSIpAddr);
	strTemp.Format(_T("%d"), m_dwFSIpAddr);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_dwFSIpAddr"), strTemp, strPath);
	strTemp.Format(_T("%d"), m_nFSPort);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_nFSPort"), strTemp, strPath);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_strID"), m_strID, strPath);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_strReceiveID"), m_strReceiveID, strPath);
	strTemp.Format(_T("%d"), m_nVolume);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_nVolume"), strTemp, strPath);
	strTemp.Format(_T("%d"), m_nAudioSource);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_nAudioSource"), strTemp, strPath);
	strTemp.Format(_T("%d"), m_nFileFormat);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("m_nFileFormat"), strTemp, strPath);

	nFileCount = m_listFile.GetItemCount();
	strTemp.Format(_T("%d"), nFileCount);
	WritePrivateProfileString(_T("CONFIG_MAIN_WND"), _T("nFileCount"), strTemp, strPath);
	for (i = 0; i < nFileCount; i++)
	{
		strTemp = m_listFile.GetItemText(i, LIST_FILE_COLUMN_PATH);
		strKey.Format(_T("ListFile%03d"), i + 1);
		WritePrivateProfileString(_T("CONFIG_MAIN_WND"), strKey, strTemp, strPath);
	}
}

void CXC9000UATest2Dlg::OnSize(UINT nType, int cx, int cy)
{
	CResizingDialog::OnSize(nType, cx, cy);

	// TODO: Add your message handler code here
	Invalidate();
}

#define EDIT_LINE_COUNT					2000
void CXC9000UATest2Dlg::LogCallback(int nLogLevel, char * pLog, void * pParam)
{
	CXC9000UATest2Dlg		*pDlg = (CXC9000UATest2Dlg *)pParam;
	CString					strTemp;

	if (pDlg != NULL && pLog != NULL && pDlg->m_editLog.m_hWnd != NULL)
	{
		if (pDlg->m_editLog.GetLineCount() > EDIT_LINE_COUNT)
			pDlg->m_strLog.Delete(0, pDlg->m_strLog.GetLength() / 2);
		strTemp = pLog;
		strTemp.Replace(_T("\n"), _T("\r\n"));
		pDlg->m_strLog += strTemp;
		pDlg->m_editLog.SetWindowText(pDlg->m_strLog);
		pDlg->m_editLog.LineScroll(pDlg->m_editLog.GetLineCount());
	}
}


void CXC9000UATest2Dlg::OnClickedBtnAnswer()
{
	// TODO: Add your control notification handler code here
	m_cSipUA.answer();
}


void CXC9000UATest2Dlg::OnClickedBtnHangUp()
{
	// TODO: Add your control notification handler code here
	m_cSipUA.task_end();
}

void CXC9000UATest2Dlg::SetListSelectFile(int nIndex)
{
	int			i, nCount = m_listFile.GetItemCount();

	for (i = 0; i < nCount; i++)
	{
		if (i == nIndex)
		{
			m_listFile.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			m_listFile.SetSelectionMark(i);
		}
		else
			m_listFile.SetItemState(i, 0, LVIS_FOCUSED | LVIS_SELECTED);
	}
}

void CXC9000UATest2Dlg::sip_event_callback(eXosip_event_t *p_event, void *p_param)
{
	CXC9000UATest2Dlg		*pDlg = (CXC9000UATest2Dlg *)p_param;

	if (pDlg != NULL && p_event != NULL)
	{
		switch (p_event->type)
		{
		case EXOSIP_REGISTRATION_SUCCESS:
			pDlg->LogCallback(0, _T("[CXC9000UATest2Dlg::sip_event_callback()] : EXOSIP_REGISTRATION_SUCCESS\n"), pDlg);
			break;
		case EXOSIP_CALL_RINGING:
			pDlg->LogCallback(0, _T("[CXC9000UATest2Dlg::sip_event_callback()] : EXOSIP_CALL_RINGING\n"), pDlg);
			break;
		case EXOSIP_CALL_ANSWERED:
			pDlg->LogCallback(0, _T("[CXC9000UATest2Dlg::sip_event_callback()] : EXOSIP_CALL_ANSWERED\n"), pDlg);
			break;
		case EXOSIP_CALL_INVITE:
			pDlg->LogCallback(0, _T("[CXC9000UATest2Dlg::sip_event_callback()] : EXOSIP_CALL_INVITE\n"), pDlg);
			break;
		case EXOSIP_CALL_CLOSED:
			pDlg->LogCallback(0, _T("[CXC9000UATest2Dlg::sip_event_callback()] : EXOSIP_CALL_CLOSED\n"), pDlg);
			break;
		}
	}
}

void CXC9000UATest2Dlg::ua_event_callback(CSipUA *p_ua, void *p_param)
{
	CXC9000UATest2Dlg		*pDlg = (CXC9000UATest2Dlg *)p_param;
	custom_event_t			*pEvent = NULL;
	event_data_t			*pEventData = NULL;
	int						nValue1, nValue2;

	if (pDlg != NULL  && pDlg->m_hWnd != NULL)
	{
		pEvent = pDlg->m_cSipUA.m_event_list_ua.get_event();
		while (pEvent != NULL)
		{
			if (strcasecmp(pEvent->subclass, UA_SUBCLASS_OPEN_FILE) == 0)
			{
				pEventData = pEvent->data;
				while (pEventData != NULL)
				{
					if (pEventData->name != NULL && pEventData->value != NULL)
					{
						if (strcasecmp(pEventData->name, UA_NAME_FILE_INDEX) == 0)
						{
							nValue1 = _ttoi(pEventData->value);
							pDlg->SetListSelectFile(nValue1);
						}
						else if (strcasecmp(pEventData->name, UA_NAME_TOTAL_LEN) == 0)
						{
							nValue1 = _ttoi(pEventData->value);
							pDlg->UpdatePlayTime(0, nValue1);
						}
					}
					pEventData = pEventData->next;
				}
			}
			else if (strcasecmp(pEvent->subclass, UA_SUBCLASS_PLAY_PROGRESS) == 0)
			{
				nValue1 = nValue2 = -1;
				pEventData = pEvent->data;
				while (pEventData != NULL)
				{
					if (pEventData->name != NULL && pEventData->value != NULL)
					{
						if (strcasecmp(pEventData->name, UA_NAME_PLAY_POS) == 0)
							nValue1 = _ttoi(pEventData->value);
						else if (strcasecmp(pEventData->name, UA_NAME_TOTAL_LEN) == 0)
							nValue2 = _ttoi(pEventData->value);
					}
					pEventData = pEventData->next;
				}
				if (nValue1 != -1 && nValue2 != -1)
					pDlg->UpdatePlayTime(nValue1, nValue2);
				{
				}
			}
			pEvent = pDlg->m_cSipUA.m_event_list_ua.get_event();
		}
	}
}

// NAL   network abstraction layer   网络提取层
// forbidden_bit(1bit) + nal_reference_bit(2bit) + nal_unit_type(5bit)
void CXC9000UATest2Dlg::video_recvdata_callback(void *p_param)
{
	CXC9000UATest2Dlg		*pDlg = (CXC9000UATest2Dlg *)p_param;
	mblk_t					*pMblk = NULL;
	unsigned char			*pPayload = NULL;
	int						nPayloadSize = 0;
	BOOL					bVideoFrame = FALSE;
	BYTE					pH264RecvBuf[H264_RECV_BUF_SIZE];

	static int		tick = 0, len_max = 0;

	if (pDlg != NULL  && pDlg->m_hWnd != NULL)
	{
		nPayloadSize = pDlg->m_cSipUA.m_video_stream.read_rtp_recvdata(pH264RecvBuf, H264_RECV_BUF_SIZE);
		while (nPayloadSize > 0)
		{
			//pDlg->m_cSipUA.m_video_stream.write_rtp_senddata(pH264RecvBuf, nPayloadSize);
			pDlg->m_dlgVideo.SetH264Data(pH264RecvBuf, nPayloadSize);
			nPayloadSize = pDlg->m_cSipUA.m_video_stream.read_rtp_recvdata(pH264RecvBuf, H264_RECV_BUF_SIZE);
		}
		//do
		//{
		//	ucNALU = 0;
		//	memset(pRecvBuf, 0, sizeof (pRecvBuf));
		//	nLen = pDlg->m_cFileVideo.read_frame_data(&pRecvBuf[0], 20000, &ucNALU);
		//	if (nLen > 0)
		//	{
		//		pDlg->m_dlgVideo.SetH264Data(&pRecvBuf[0], nLen);
		//		bVideoFrame = TRUE;
		//	}
		//} while (nLen > 0 && !bVideoFrame);
	}
}

void CXC9000UATest2Dlg::UpdatePlayTime(DWORD dwTimePlayed, DWORD dwTimeTotal)
{
	DWORD			dwHour, dwMinute, dwSecond, dwTimeRest;
	CString			strTime;

	dwHour = dwTimePlayed / 3600;
	dwMinute = (dwTimePlayed % 3600) / 60;
	dwSecond = (dwTimePlayed % 3600) % 60;
	if (dwHour > 0)
		strTime.Format(_T("%02d:%02d:%02d"), dwHour, dwMinute, dwSecond);
	else
		strTime.Format(_T("%02d:%02d"), dwMinute, dwSecond);
	if (GetDlgItem(IDC_MAIN_STATIC_TIME_PLAYED ) != NULL && GetDlgItem(IDC_MAIN_STATIC_TIME_PLAYED)->m_hWnd != NULL)
		GetDlgItem(IDC_MAIN_STATIC_TIME_PLAYED)->SetWindowText(strTime);

	if (dwTimeTotal >= dwTimePlayed)
		dwTimeRest = dwTimeTotal - dwTimePlayed;
	else
		dwTimeRest = 0;
	dwHour = dwTimeRest / 3600;
	dwMinute = (dwTimeRest % 3600) / 60;
	dwSecond = (dwTimeRest % 3600) % 60;
	if (dwHour > 0)
		strTime.Format(_T("-%02d:%02d:%02d"), dwHour, dwMinute, dwSecond);
	else
		strTime.Format(_T("-%02d:%02d"), dwMinute, dwSecond);
	if (GetDlgItem(IDC_MAIN_STATIC_TIME_REST) != NULL && GetDlgItem(IDC_MAIN_STATIC_TIME_REST)->m_hWnd != NULL)
		GetDlgItem(IDC_MAIN_STATIC_TIME_REST)->SetWindowText(strTime);

	if (m_cPlayProgress.m_hWnd != NULL)
	{
		m_cPlayProgress.SetRange(0, dwTimeTotal);
		m_cPlayProgress.SetPos(dwTimePlayed);
	}
}


void CXC9000UATest2Dlg::OnDblclkListFile(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: Add your control notification handler code here
	NM_LISTVIEW		*pNMListView = (NM_LISTVIEW *)pNMHDR;
	int				nItem = pNMListView->iItem;

	if(nItem >= 0 && nItem < m_listFile.GetItemCount())//判断双击位置是否在有数据的列表项上面
	{
		m_cSipUA.m_audio_stream.m_audio_file.set_play_index(nItem);
	}

	*pResult = 0;
}

void CXC9000UATest2Dlg::OnClickedBtnAlarmOut()
{
	// TODO: Add your control notification handler code here
	if (UpdateData())
	{
		TCHAR		pOperation[] = _T("control");
		TCHAR		pAlarmNo[20];
		TCHAR		pStatus[10];
		_stprintf(pAlarmNo, "%d", m_nAlarmOutNo + 500);
		_stprintf(pStatus, "%d", m_comboAlarmOut.GetCurSel());
		m_cSipUA.send_msg_alarm_out(m_strReceiveID.GetBuffer(0), pOperation, pAlarmNo, pStatus);
		m_strReceiveID.ReleaseBuffer();
	}
}


void CXC9000UATest2Dlg::OnDestroy()
{
	CResizingDialog::OnDestroy();

	// TODO: Add your message handler code here
	m_dlgVideo.DestroyWindow();
}

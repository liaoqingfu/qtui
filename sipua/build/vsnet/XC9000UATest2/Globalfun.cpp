//globalfunction

#include "StdAfx.h"
#include "Globalfun.h"
#include "Defines.h"

// Get the path of application.
CString GetApplicationPath()
{
	TCHAR exeFullPath[MAX_PATH]; 
	GetModuleFileName( NULL, exeFullPath, MAX_PATH ); 
	CString strPathSetting = exeFullPath;
	int nFind = strPathSetting.ReverseFind('\\');
	if (nFind != -1)
		strPathSetting = strPathSetting.Left( nFind+1 );
	else
		strPathSetting = _T("");
	return strPathSetting;
}

// 从系统中取最后一次错误代码，并转换成字符串返回
LPTSTR GetLastErrorText(LPTSTR pBuf, DWORD dwSize) 
{
	DWORD		dwReturn;
	LPTSTR		pTemp = NULL;

	dwReturn = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
				NULL,
				GetLastError(),
				LANG_NEUTRAL,
				(LPTSTR)&pTemp,
				0,
				NULL);

	// supplied buffer is not long enough
	if(!dwReturn || ((long)dwSize < (long)dwReturn + 14))
		pBuf[0] = _T('\0');
	else
	{
		pTemp[lstrlen(pTemp)-2] = _T('\0');  // remove cr and newline character
		_stprintf_s(pBuf, 256, TEXT("%s (%ld)"), pTemp, GetLastError() );
	}

	if (pTemp)
		LocalFree((HLOCAL)pTemp);

	return pBuf;
}

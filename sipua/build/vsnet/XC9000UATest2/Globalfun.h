#ifndef _GLOBALFUN_H
#define _GLOBALFUN_H

CString		GetApplicationPath();									// Get the path of application
LPTSTR		GetLastErrorText(LPTSTR pBuf, DWORD dwSize);			// 从系统中取最后一次错误代码，并转换成字符串返回

#endif	//_GLOBALFUN_H
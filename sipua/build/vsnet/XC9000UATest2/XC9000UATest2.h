
// XC9000UATest2.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CXC9000UATest2App:
// �йش����ʵ�֣������ XC9000UATest2.cpp
//

class CXC9000UATest2App : public CWinApp
{
public:
	CXC9000UATest2App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CXC9000UATest2App theApp;

// wgw.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CwgwApp:
// �йش����ʵ�֣������ wgw.cpp
//

class CwgwApp : public CWinApp
{
public:
	CwgwApp();

// ��д
public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CwgwApp theApp;
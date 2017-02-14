
#pragma once

int beginServer();

#include <map>
#include <vector>

#include "common.h"

using namespace std;


class CTcpThread : public CWinThread
{
	DECLARE_DYNCREATE(CTcpThread)

protected:
	CTcpThread();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CTcpThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	/*
	bool open(CString name, unsigned speed);
	bool close();
	bool addInput(char *buf, unsigned len);
	*/
protected:
	DECLARE_MESSAGE_MAP()

	virtual int Run();

private:
	int m_port;
};
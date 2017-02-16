
#pragma once

int beginServer();

#include <map>
#include <vector>

#include "common.h"

using namespace std;

class TcpMsg
{
public:
	TcpMsg(CString iplist, void *data, unsigned len){m_iplist = iplist; m_data = data; m_len = len;};
	CString m_iplist;
	void *m_data;
	unsigned m_len;
};

class CTcpThread : public CWinThread
{
	DECLARE_DYNCREATE(CTcpThread)

protected:
	CTcpThread();           // 动态创建所使用的受保护的构造函数
	virtual ~CTcpThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	bool addInput(CString iplist, char *buf, unsigned len);
	/*
	bool open(CString name, unsigned speed);
	bool close();
	bool addInput(char *buf, unsigned len);
	*/
protected:
	DECLARE_MESSAGE_MAP()

	virtual int Run();

private:
	class msgBuffer m_msgBuffer;
	int m_port;

	vector<TcpMsg> m_msg;
	CEvent m_Event;
	CMutex m_mutex;
};
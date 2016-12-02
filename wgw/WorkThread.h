#pragma once

#include <map>
#include <vector>

#include "common.h"

using namespace std;

struct handType
{
	handType(){hand = INVALID_HANDLE_VALUE; isCH9326 = false;};
	HANDLE hand;
	bool isCH9326;


};

// CWorkThread

class CWorkThread : public CWinThread
{
	DECLARE_DYNCREATE(CWorkThread)

protected:
	CWorkThread();           // 动态创建所使用的受保护的构造函数
	virtual ~CWorkThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	bool open(CString name, unsigned speed);
	bool close();
	bool addInput(char *buf, unsigned len);
protected:
	DECLARE_MESSAGE_MAP()

	void getHand(struct handType &hand);
	void getHand(HANDLE &hand, bool &isCH9326);
	void getInput(vector<void *> &m_output);
	virtual int Run();
	int WriteCommBlock(HANDLE hand, bool isCH9326, void *outbuf);
	int ReadCommBlock(HANDLE COMFile, bool isCH9326);
	HANDLE ConnectComm(int nPort, unsigned speed);
	BOOL CloseComm(HANDLE COMFile);
	bool setHand(HANDLE hand, bool isCH9326);
	int COMOpen(CString name, unsigned speed);
private:
	HANDLE m_hMutex;

	vector<void *> m_output;
	struct handType m_hand;
	CMutex m_mutex;

	OVERLAPPED osRead;
	OVERLAPPED osWrite;
	OVERLAPPED ShareEvent;

	CEvent m_EventEmpty;
	CEvent m_handEvent;
	class msgBuffer m_msgBuffer;

	USHORT m_inportlen;
	USHORT m_outportlen;
};



#pragma once



// CWorkThread

class CWorkThread : public CWinThread
{
	DECLARE_DYNCREATE(CWorkThread)

protected:
	CWorkThread();           // ��̬������ʹ�õ��ܱ����Ĺ��캯��
	virtual ~CWorkThread();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
};



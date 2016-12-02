

#pragma once

#include "stdint.h"

#define MSG_BEGIN TEXT('\xf9')
#define MSG_TRANSFERRED TEXT('\xf8')

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

struct checkItem
{
	TCHAR id[16];
	uint64_t firstTime;
	uint64_t lastTime;
	int electricity;
	int nindex;
	checkItem(){
		nindex = 0;
		firstTime = 0;
		lastTime = 0;
		electricity = 0;
		memset(id, 0, sizeof(id));
	}
};

struct answerItem
{
	TCHAR id[16];
	TCHAR answer[16];
	uint64_t time;
	int electricity;
	int nindex;
	answerItem(){
		nindex = 0;
		time = 0;
		electricity = 0;
		memset(id, 0, sizeof(id));
		memset(answer, 0, sizeof(answer));
	}
};

struct readThreadData
{
	volatile long runing;
	volatile long exit;

	HWND hWnd;
	HANDLE hCom;
	CWinThread* thread;

	bool isCH9326;
	unsigned inportlen;
	unsigned outportlen;
};

unsigned chrVal(TCHAR c);
unsigned chrMask();

#define USB_VID 0x1A86
#define USB_PID 0xE010

class msgBuffer
{
public:
	msgBuffer(unsigned size = 2048);
	~msgBuffer();

	bool addInput(char *buf, unsigned len);
protected:
	unsigned getMsg();
	int getMsgOne(char *buf, unsigned len);
	char* getMsgStart(char *buf, unsigned len);
	int processMsg(char *start, unsigned size, unsigned len);
private:
	char *m_buf;
	unsigned m_size;
	unsigned m_len;
};

unsigned char HIDSpeed(unsigned speed);

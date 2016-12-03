

#include "stdafx.h"

unsigned char HIDSpeed(unsigned speed)
{
	//1=300(ucRate为1时对应波特率300),2=600,3=1200,4=2400,5=4800,6=9600(默认值),7=14400,
	//	8=19200,9=28800,10=38400,11=57600,12=76800,13=115200
	unsigned char	ucRate = 6;
	switch(speed){
		case 300:
			ucRate = 1;
			break;
		case 600:
			ucRate = 2;
			break;
		case 1200:
			ucRate = 3;
			break;
		case 2400:
			ucRate = 4;
			break;
		case 4800:
			ucRate = 5;
			break;
		case 9600:
			ucRate = 6;
			break;
		case 14400:
			ucRate = 7;
			break;
		case 19200:
			ucRate = 8;
			break;
		case 28800:
			ucRate = 9;
			break;
		case 38400:
			ucRate = 10;
			break;
		case 57600:
			ucRate = 11;
			break;
		case 76800:
			ucRate = 12;
			break;
		case 115200:
			ucRate = 13;
			break;
	}

	return ucRate;
}

unsigned chrMask()
{
	unsigned mask = (unsigned)(-1);
	switch(sizeof(TCHAR)){
		case 1:
			mask = 0xff;
			break;
		case 2:
			mask = 0xffff;
			break;
		case 4:
			mask = 0xffffffff;
			break;
	}

	return mask;
}

unsigned chrVal(TCHAR c)
{
	return c&chrMask();
}

msgBuffer::msgBuffer(unsigned size)
{
	m_size = size;
	m_len = 0;
	m_buf = (char*)malloc(m_size);
	ASSERT(m_buf);
}

msgBuffer::~msgBuffer()
{
	free(m_buf);
}

char* msgBuffer::getMsgStart(char *buf, unsigned len)
{
	for(unsigned i = 0; i < len; i++){
		if(buf[i] == MSG_BEGIN && i+2 < len)
			return buf+i;
	}

	return 0;
}

int msgBuffer::processMsg(char *start, unsigned size, unsigned len)
{
	if(len > size)
		return 0;

	void *p = malloc(len);
	if(!p)
		return len;

	//::PostMessage(GetSafeHwnd(), WM_USER_THREADEND, 0, 0);
	//CWnd *pMainWnd = AfxGetMainWnd();pMainWnd->m_hWnd
	//::PostMessage(data->hand, WM_USER_THREADEND, (WPARAM)p, len);
	memcpy(p, start, len);
	::PostMessage(AfxGetMainWnd()->m_hWnd, WM_USER_THREADEND, (WPARAM)p, len);
	return len;
}

int msgBuffer::getMsgOne(char *buf, unsigned len)
{
	CString debug;
	char* start = getMsgStart(buf, len);
	if(!start)
		return 0;

	switch(start[2]){
		case TEXT('\x2'):	//答题对
			return processMsg(start, len, 18);
		case TEXT('\x5'):	//考勤
			msgXData(debug, start, 15);
			return processMsg(start, len, 15);
		case TEXT('\x4'):	//开考勤
		case TEXT('\x6'):	//关考勤
		case TEXT('\x1'):	//开答题
		case TEXT('\x3'):	//关答题
			return processMsg(start, len, 3);
		case TEXT('\x7'):	//校时
			return processMsg(start, len, 6);
		default:
			return -1;
	}

	return -1;
}

unsigned msgBuffer::getMsg()
{
	int ret = 0;
	
	while(1){
		ASSERT(m_len >= ret);
		int err = getMsgOne(m_buf + ret, m_len - ret);
		if(0 > err){
			return m_len;
		}
		if(0 == err)
			return ret;

		ret += err;
	}
	
	return m_len;
}

bool msgBuffer::addInput(char *buf, unsigned size)
{
	if(m_len + size >= m_size){
		m_len = 0;
		return false;
	}

	memcpy(m_buf + m_len, buf, size);
	m_len += size;

	unsigned len = getMsg();
	if(!len)
		return true;

	ASSERT(m_len >= len);
	m_len -= len;
	memmove(m_buf, m_buf+len, m_len);
	return true;
}

void msgXData(CString &str, TCHAR *in, unsigned len)
{
	unsigned mask = chrMask();

	TCHAR buffer[2048];

	ASSERT(sizeof(buffer)/sizeof(buffer[0]) > len+1);
	unsigned i;
	for(i = 0; i < len; i++){
		_stprintf(buffer + 2*i, TEXT("%02x"), chrVal(in[i]));
	}

	str = CString(buffer) + TEXT("\r\n") + str;
}

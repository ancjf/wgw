// WorkThread.cpp : 实现文件
//

#include "stdafx.h"
#include "wgw.h"
#include "WorkThread.h"

#include "CH9326DBG.h"
#include "CH9326DLL.H"
// CWorkThread

IMPLEMENT_DYNCREATE(CWorkThread, CWinThread)

CWorkThread::CWorkThread()
{
	memset(&osRead, 0, sizeof(osRead));
	memset(&osWrite, 0, sizeof(osWrite));
	memset(&ShareEvent, 0, sizeof(ShareEvent));

	osRead.hEvent= CreateEvent( NULL, FALSE, FALSE, NULL ) ;
	if (osRead.hEvent == NULL) {
		AfxMessageBox(_T("建立事件失败!")) ;
	}

	osWrite.hEvent= CreateEvent( NULL, FALSE, FALSE, NULL ) ;
	if (osWrite.hEvent == NULL) {
		AfxMessageBox(_T("建立事件失败!")) ;
	}

	ShareEvent.hEvent= CreateEvent( NULL, FALSE, FALSE, NULL ) ;
	if (ShareEvent.hEvent == NULL) {
		AfxMessageBox(_T("建立事件失败!")) ;
	}
}

CWorkThread::~CWorkThread()
{
}

BOOL CWorkThread::InitInstance()
{
	// TODO: 在此执行任意逐线程初始化
	return TRUE;
}

int CWorkThread::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
		CloseHandle(osRead.hEvent);
	memset(&osRead, 0, sizeof(osRead));;
	CloseHandle(osWrite.hEvent);
	memset(&osWrite, 0, sizeof(osWrite));
	CloseHandle(ShareEvent.hEvent);
	memset(&ShareEvent, 0, sizeof(ShareEvent));

	return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(CWorkThread, CWinThread)
END_MESSAGE_MAP()


// CWorkThread 消息处理程序
bool CWorkThread::setHand(HANDLE hand, bool isCH9326)
{
	m_mutex.Lock();

	m_hand.hand = hand;
	m_hand.isCH9326 = isCH9326;
	m_handEvent.SetEvent();

	m_mutex.Unlock();
	return true;
}

void CWorkThread::getHand(struct handType &hand)
{
	m_mutex.Lock();

	hand = m_hand;

	m_mutex.Unlock();
}

void CWorkThread::getHand(HANDLE &hand, bool &isCH9326)
{
	m_mutex.Lock();

	hand = m_hand.hand;
	isCH9326 = m_hand.isCH9326;

	m_mutex.Unlock();
}

bool CWorkThread::addInput(char *buf, unsigned len)
{
	m_mutex.Lock();

	uint32_t *p = (uint32_t *)malloc(len + sizeof(uint32_t));
	if(!p){
		m_mutex.Unlock();
		return false;
	}

	p[0] = len;
	memcpy(p+1, buf, len);
	m_output.push_back((void*)p);
	m_EventEmpty.SetEvent();

	m_mutex.Unlock();
	return true;
}

void CWorkThread::getInput(vector<void *> &output)
{
	m_mutex.Lock();

	output = m_output;
	m_output.clear();

	m_mutex.Unlock();
}

int CWorkThread::ReadCommCH9326(HANDLE COMFile)
{
	char buf[1024];
	ULONG len = sizeof(buf);

	if(CH9326ReadThreadData(COMFile, buf, &len) && len)
		m_msgBuffer.addInput(buf, len);

	return 0;
}
int CWorkThread::ReadCommBlock(HANDLE COMFile)
{
	char buf[128];
	DWORD dwErrorFlags;
	COMSTAT ComStat ;
	DWORD dwLength;
	DWORD dwOutBytes = 0;
	ClearCommError( COMFile, &dwErrorFlags, &ComStat ) ;
	dwLength = ComStat.cbInQue;


	ReadFile( COMFile, buf/*lpszBlock*/, sizeof(buf), &dwOutBytes, &/*ShareEvent*/osRead);
	m_msgBuffer.addInput(buf, dwOutBytes);

	//m_nBuffLen = m_nBuffLen > MAX_BUFF ? MAX_BUFF : m_nBuffLen;
// 	if (WAIT_OBJECT_0 == WaitForSingleObject(osRead.hEvent, 200)) {
//  		ResetEvent(osRead.hEvent);
// 	else
// 		ResetEvent(ShareEvent.hEvent);
// 		m_nBuffLen/*dwLength*/ += dwLength;
// 
// 		if (m_nBuffLen > 50)
// 		{
// 			if (m_nBuffLen > 256)
// 			{
// 				m_nBuffLen = 256;
// 			}
// 			CStringA szTemp, szShow;
// 			for (DWORD i = 0; i < m_nBuffLen; ++i) {
// 				szTemp.Format((i + 1 != m_nBuffLen) ? _T("%02X ") :_T("%02X"), m_InPutBuff[i]);
// 				szShow += szTemp; szTemp.Empty();
// 			}
// 			g_pMainWnd->m_LsDataIn.InsertString(0, szShow);
// 
// 			memset(m_InPutBuff, 0, MAX_PATH);
// 			m_nBuffLen = 0;
// 		}
// 	}

	return ( dwLength ) ;
}

int CWorkThread::ReadComm(HANDLE COMFile, bool isCH9326)
{
	if(isCH9326)
		return ReadCommCH9326(COMFile);

	DWORD dwEvtMask = 0 ; 
	WaitCommEvent(COMFile, &dwEvtMask, &ShareEvent);//等待串口事件
	if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR) {
		ReadCommBlock(COMFile);
	}
}

int CWorkThread::WriteCommCh9326(HANDLE COMFile, char *data, unsigned len)
{
	bool ret;
	HANDLE hEventObject=CreateEvent(NULL,TRUE,TRUE, "");
	ret = CH9326WriteData(COMFile, data, len, hEventObject);

	CloseHandle(hEventObject);
	return ret;
}

int CWorkThread::WriteCommBlock(HANDLE COMFile, bool isCH9326, void *outbuf)
{
	uint32_t *pbuf = (uint32_t *)outbuf;
	PBYTE pOutData = (PBYTE)(pbuf+1);
	int nDataLen = pbuf[0];

	if (NULL == pOutData || nDataLen < 1)
		return FALSE;

	if(isCH9326)
		return WriteCommCh9326(COMFile, (char*)(pbuf+1), nDataLen);
	//while(nDataLen > 0){
	DWORD nLen = 0;
	if (! WriteFile(COMFile, (LPCVOID)pOutData, nDataLen, &nLen, &/*ShareEvent*/osWrite)) {

		if (WAIT_OBJECT_0 == WaitForSingleObject(osWrite.hEvent, 0xFFFFFF)) 
			ResetEvent(osWrite.hEvent);
		else
			ResetEvent(osWrite.hEvent);


		DWORD nError = GetLastError();
		if (997 != nError) {//997异步没完成
			char chError[256];memset(chError, 0, 256);
			int nBuffLen = 256;
		}
	}
	//}

	return TRUE;
}

int CWorkThread::Run()
{
	HANDLE hEventArr[2];

	hEventArr[0] = osRead.hEvent;
	hEventArr[1] = m_EventEmpty.m_hObject;
	vector<void *> output;
	HANDLE COMFile;
	bool isCH9326;
	vector<void *>::iterator itr;

	while(1){
		//优先事件数据中的索引小的事件, 如果小索引事件一直有信号, 则大索引事件将很难触发
		getHand(COMFile, isCH9326);
		if(COMFile == INVALID_HANDLE_VALUE){
			WaitForSingleObject(m_handEvent.m_hObject, 2000);
			continue;
		}

		DWORD nResutl = WaitForMultipleObjectsEx(2, hEventArr, FALSE, 10, TRUE/*INFINITE*/);
		if(0 == nResutl){
			getInput(output);
		}else if(1 == nResutl){
			getInput(output);
			itr = output.begin();
			while(itr != output.end()){
				if(WriteCommBlock(COMFile, isCH9326, *itr))
					output.erase(itr);

				itr = output.begin();
			}
		}

		ReadComm(COMFile, isCH9326);
	}

	//子线程结束
	CloseHandle( osRead.hEvent ) ;
	CloseHandle( osWrite.hEvent ) ;
	CloseHandle(ShareEvent.hEvent);

	return CWinThread::Run();
}

BOOL CWorkThread::CloseComm(HANDLE COMFile)
{
	//禁止串行端口所有事件
	SetCommMask(COMFile, 0) ;

	//清除数据终端就绪信号
	EscapeCommFunction( COMFile, CLRDTR ) ;

	//丢弃通信资源的输出或输入缓冲区字符并终止在通信资源上挂起的读、写操//场作 
	PurgeComm( COMFile, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	CloseHandle( COMFile );
	COMFile = NULL;
// 	CloseHandle(osRead.hEvent);
// 	memset(&osRead, 0, sizeof(osRead));;
// 	CloseHandle(osWrite.hEvent);
// 	memset(&osWrite, 0, sizeof(osWrite));
// 	CloseHandle(ShareEvent.hEvent);
// 	memset(&ShareEvent, 0, sizeof(ShareEvent));

	return TRUE;
}


HANDLE CWorkThread::ConnectComm(int nPort, unsigned speed)
{
//	BYTE bSet;
	DCB dcb ;
	BOOL fRetVal ;
	COMMTIMEOUTS CommTimeOuts;
	CString szCom; 
	szCom.Format(_T("\\\\.\\COM%d"), nPort);
	HANDLE COMFile = CreateFile(szCom.GetBuffer(50), GENERIC_READ | GENERIC_WRITE,//可读、可写
							FILE_SHARE_READ | FILE_SHARE_WRITE, // 不共享FILE_SHARE_READ,FILE_SHARE_WRITE,FILE_SHARE_DELETE
							NULL, // 无安全描
							OPEN_EXISTING, //打开已存在文件
							/*FILE_ATTRIBUTE_NORMAL | */FILE_FLAG_OVERLAPPED, // 文件属性
							NULL//一个有效的句柄，已存在的设备句柄A valid handle to a template file with the GENERIC_READ access right
							);

	int nError = GetLastError();
	if (INVALID_HANDLE_VALUE == COMFile/*COMFileTemp*/ ){
		return INVALID_HANDLE_VALUE;
	}
	// 设置缓冲区,输入/输出大小(字节数)
	SetupComm( /*COMFileTemp*/COMFile,4096,4096) ;
	// 指定监视事件_收到字符放入缓冲区
	SetCommMask(/*COMFileTemp*/COMFile, EV_RXCHAR ) ;


	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 2*CBR_9600/9600 ;
	CommTimeOuts.WriteTotalTimeoutConstant = 0 ;
	//给定串口读与操作限时
	SetCommTimeouts(/*COMFileTemp*/COMFile, &CommTimeOuts ) ;

	//设置串口参数:波特率=9600;停止位 1个;无校验;8位
	dcb.DCBlength = sizeof( DCB ) ;
	GetCommState( /*COMFileTemp*/COMFile, &dcb ) ;
	dcb.BaudRate = speed;
	dcb.StopBits =ONESTOPBIT;
	dcb.Parity = NOPARITY;
	dcb.ByteSize=8;
	dcb.fBinary=TRUE;//二进制通信, 非字符通信
	dcb.fOutxDsrFlow = 0 ;
	dcb.fDtrControl = DTR_CONTROL_ENABLE ;
	dcb.fOutxCtsFlow = 0 ;
	dcb.fRtsControl = RTS_CONTROL_ENABLE ;
	dcb.fInX = dcb.fOutX = 1 ;
	dcb.XonChar = 0X11 ;
	dcb.XoffChar = 0X13 ;
	dcb.XonLim = 100 ;
	dcb.XoffLim = 100 ;
	dcb.fParity = TRUE ;

	//根据设备控制块配置通信设备
	fRetVal = SetCommState(/*COMFileTemp*/COMFile, &dcb) ;

	if(!fRetVal){
		CloseHandle(COMFile);
		return INVALID_HANDLE_VALUE;
	}

	//刷清缓冲区
	PurgeComm( /*COMFileTemp*/COMFile, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	//指定串口执行扩展功能
	EscapeCommFunction( /*COMFileTemp*/COMFile, SETDTR ) ;



// 	//设置串口 “收到字符放入缓冲区”事件
// 	if (!SetCommMask(COMFile, EV_RXCHAR )) return ( FALSE );

	return COMFile;
}

int CWorkThread::COMOpen(CString name, unsigned speed)
{
	int index = name.Find(TEXT("com"));
	if(0 > index)
		index = name.Find(TEXT("COM"));
	if(0 > index)
		return 1;

	TCHAR *p = name.GetBuffer()+index+3;
	CString szCom;
	int n = _tstoi(p);

	HANDLE hHID = ConnectComm(n, speed);
	if(hHID==INVALID_HANDLE_VALUE){
		AfxMessageBox("打开com设备失败");
		return -1;
	}

	setHand(hHID, false);
	return 0;
}

bool CWorkThread::open(CString name, unsigned speed)
{
	int err = COMOpen(name, speed);
	if(0 >= err)
		return 0 == err;

	HANDLE hHID = CH9326OpenDevicePath((PCHAR)LPCTSTR (name.GetBuffer()));
	if(hHID == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("打开HID设备失败");
		return false;
	}

	USHORT VID,PID,VER;
	//获取厂商ID和设备ID
	if(!CH9326GetAttributes(hHID,&VID,&PID,&VER)){
		CH9326CloseDevice(hHID);
		return false;
	}

	char version[100]="";
	sprintf(version,"设备已连接,VID=%XPID=%X VER=%X ",VID,PID,VER);
		
	if(VID == USB_VID && PID == USB_PID){
		CH9326GetBufferLen(hHID,&m_inportlen,&m_outportlen);
		CH9326SetTimeOut(hHID,3000,3000);
		if(!CH9326SetRate(hHID, HIDSpeed(speed), 4, 1, 4, 48)){
			CH9326CloseDevice(hHID);
			AfxMessageBox("设置不成功!");
			return false;
		}
	}

	if(!CH9326InitThreadData(hHID)){
		CH9326CloseDevice(hHID);
		AfxMessageBox("初始化不成功!");
		return false;
	}

	setHand(hHID, true);
	return true;
}

bool CWorkThread::close()
{
	HANDLE COMFile;
	bool isCH9326;

	getHand(COMFile, isCH9326);
	if(COMFile == INVALID_HANDLE_VALUE)
		return false;

	if(isCH9326){
		CH9326CloseDevice(COMFile);
	}else{
		CloseComm(COMFile);
	}

	return true;
}

// hid.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cfgmgr32.h>
#include <time.h>

#include <vector>
#include "hid.h"
#include "WorkThread.h"

#include "CH9326DBG.h"
#include "CH9326DLL.H"

using namespace std;

extern "C" {  
#include "setupapi.h" 
#include "hidsdi.h" 
}



//#define USB_VID 0x093A
//#define USB_PID 0x2510

//#define USB_VID 0xFFFF
//#define USB_PID 0x0035

HANDLE OpenMyHIDDevice(int overlapped);
LPVOID lpMsgBuf;
/*用FormatMessage()得到由GetLastError()返回的出错编码所对应错误信息*/
#define error FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL )
/*---------------------------------------------*/

struct sendData
{
	HANDLE hand;
	unsigned timeout;
	char *data;
	unsigned len;
	bool isCH9326;
};

UINT  SendThreadFunction(LPVOID lpParameter)
{
	struct sendData *data  = (struct sendData *)lpParameter;

	if(data->isCH9326){

		CString debug;
		msgXData(debug, data->data, data->len);

		HANDLE hEventObject=CreateEvent(NULL,TRUE,TRUE, "");
		if(!CH9326WriteData(data->hand, data->data, data->len, hEventObject))
			AfxMessageBox("发送数据失败!");

		CloseHandle(hEventObject);
		//free(data);
	}else{
		DWORD dwReadSize = 0;
		bool ret = WriteFile(data->hand, data->data, data->len, &dwReadSize, NULL);

		int i = 0;
	}

	free(data);
	return 0;
}	


int HIDWrite(HANDLE hand, const char *buf, unsigned len, bool isCH9326)
{
	struct sendData *data = (struct sendData *)malloc(sizeof(struct sendData) + len);
	if(!data)
		return -1;

	data->data = (char*)(data+1);
	data->len = len;
	data->hand = hand;
	data->isCH9326 = isCH9326;
	memcpy(data->data, buf, len);

	return SendThreadFunction(data);
	AfxBeginThread(SendThreadFunction, data, THREAD_PRIORITY_NORMAL, 0, 0, NULL); 
	return 0;
}

struct readData
{
	char *buf;
	unsigned size;
	unsigned len;

	HWND hand;

	unsigned inportlen;
	unsigned outportlen;
};


bool s_ReadThread_start = false;

char* getMsgStart(char *buf, unsigned len)
{
	for(unsigned i = 0; i < len; i++){
		if(buf[i] == MSG_BEGIN && i+2 < len)
			return buf+i;
	}

	return 0;
}

int processMsg(char *start, unsigned size, unsigned len)
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

int getMsgOne(struct readData *data, char *buf, unsigned len)
{
	char* start = getMsgStart(buf, len);
	if(!start)
		return 0;

	switch(start[2]){
		case TEXT('\x2'):	//答题对
			return processMsg(start, len, 18);
		case TEXT('\x5'):	//考勤
			return processMsg(start, len, 14);
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

unsigned getMsg(struct readData *data)
{
	int ret = 0;
	
	while(1){
		ASSERT(data->len >= ret);
		int err = getMsgOne(data, data->buf + ret, data->len - ret);
		if(0 > err){
			return data->len;
		}
		if(0 == err)
			return ret;

		ret += err;
	}
	
	return data->len;
}

void addInput(struct readData *data, char *buf, long size)
{
	if(data->len + size >= data->size){
		data->len = 0;
		return;
	}

	memcpy(data->buf + data->len, buf, size);
	data->len += size;

	unsigned len = getMsg(data);
	if(!len)
		return;

	ASSERT(data->len >= len);
	data->len -= len;
	memmove(data->buf, data->buf+len, data->len);
}

UINT  ReadThreadFunction(LPVOID lpParameter)
{
	struct readThreadData *threaddata = (struct readThreadData *)lpParameter;

	HANDLE hCom = threaddata->hCom;
	bool isCH9326 = threaddata->isCH9326;
	if(isCH9326){
		/*
		if(!CH9326InitThreadData(hCom)){
			InterlockedExchange(&threaddata->runing, 0);
			return 0;
		}*/
	}

	char buf[1024];
	struct readData data;	
	data.size = 1024;
	data.buf = (char*)malloc(data.size+1);
	ASSERT(data.buf);
	data.len  = 0;
	data.hand = threaddata->hWnd;
	data.inportlen = threaddata->inportlen;
	if(data.inportlen > sizeof(buf))
		data.inportlen = sizeof(buf);
	data.outportlen = threaddata->outportlen;

	class msgBuffer msgbuffer;
	InterlockedExchange(&threaddata->runing, 1);
	CH9326ClearThreadData(hCom);
	while(!threaddata->exit){

		ULONG len = sizeof(buf);
		if(!len){
			break;
		}

		if(isCH9326){
			if(!CH9326ReadThreadData(hCom, buf, &len) ) {
				break;
			}
				
		}else{
			if (!ReadFile(hCom, buf, len, &len, NULL)){
				break;
			}
		}

		if(len){
			msgbuffer.addInput(buf, len);
		}
	}

	
	free(data.buf);
	InterlockedExchange(&threaddata->runing, 0);
	//AfxEndThread(0);
	return 0;
}

int HIDStartRead(struct readThreadData *threaddata)
{
	InterlockedExchange(&threaddata->exit, 0);
	InterlockedExchange(&threaddata->runing, 0);
	threaddata->thread = AfxBeginThread(ReadThreadFunction, threaddata, THREAD_PRIORITY_NORMAL, 0, 0, NULL); 
	return 0;
}

int HIDRead(HANDLE hComm, char *buf, unsigned size, unsigned timeout)
{
    DWORD dwRead;
    OVERLAPPED osReader = {0};
    // Create the overlapped event. Must be closed before exiting
    // to avoid a handle leak.
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osReader.hEvent == NULL) {
        // Error creating overlapped event; abort.
        return -1;
    }

    if (!ReadFile(hComm, buf, size, &dwRead, &osReader)) {
        // error;//错误查询
        if (GetLastError() != ERROR_IO_PENDING) { // read not delayed?
            // Error in communications; report it.
			return -1;
        }
    } else {
        // read completed immediately
        //HandleASuccessfulRead(buf, dwRead);
		return dwRead;
    }

	if(!timeout){
		CloseHandle(osReader.hEvent);
		return -1;
	}

	dwRead = -1;
    DWORD dwRes = WaitForSingleObject(osReader.hEvent, timeout);
    switch(dwRes) {
    // Read completed.
    case WAIT_OBJECT_0:
        if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)) {
            // Error in communications; report it.
        } else {
            // Read completed successfully.
            //HandleASuccessfulRead(lpBuf, dwRead);
			//return dwRead;
        }
        // Reset flag so that another opertion can be issued.
        break;
    case WAIT_TIMEOUT:
        // Operation isn't complete yet. fWaitingOnRead flag isn't
        // changed since I'll loop back around, and I don't want
        // to issue another read until the first one finishes.
        //
        // This is a good time to do some background work.
        break;
    default:
        // Error in the WaitForSingleObject; abort.
        // This indicates a problem with the OVERLAPPED structure's
        // event handle.
        break;
    }

	CloseHandle(osReader.hEvent);
    return dwRead;
}


int COMClose(struct readThreadData *data)
{
	if(data->hCom != INVALID_HANDLE_VALUE){
		SetCommMask(data->hCom, 0) ;

		//清除数据终端就绪信号
		EscapeCommFunction(data->hCom, CLRDTR ) ;

		//丢弃通信资源的输出或输入缓冲区字符并终止在通信资源上挂起的读、写操//场作 
		PurgeComm(data->hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

		CloseHandle(data->hCom);
		data->hCom = INVALID_HANDLE_VALUE;
	}
	return 0;
}

int HIDClose(struct readThreadData *data)
{
	CWorkThread *thread = (CWorkThread *)data->threadReal;
	if(thread->close())
		return 0;
	return -1;

	InterlockedExchange(&data->exit, 1);

	if(data->isCH9326){
		if(data->hCom != INVALID_HANDLE_VALUE){
			CH9326CloseDevice(data->hCom);
			data->hCom = INVALID_HANDLE_VALUE;
		}
	}else{
		COMClose(data);
	}

	for(int i = 0; i < 10000000; i++){
		Sleep(10);
		if(!InterlockedExchange(&data->runing, data->runing))
			break;
	}

	//if(data->runing)
		//TerminateThread(data->thread, 0);

	return 0;
}

int COMOpen(CString name, unsigned speed, struct readThreadData *data)
{
	int index = name.Find(TEXT("com"));
	if(0 > index)
		index = name.Find(TEXT("COM"));
	if(0 > index)
		return 1;

	TCHAR *p = name.GetBuffer()+index+3;
	CString szCom;
	int n = _tstoi(p);

	szCom.Format(_T("COM%d"), n);
	data->hCom = CreateFile(szCom.GetBuffer(50), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0,NULL);
	if(data->hCom == INVALID_HANDLE_VALUE){
		szCom.Format(_T("\\\\.\\COM%d"), n);

		data->hCom = CreateFile(szCom.GetBuffer(50), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0,NULL);
		if(data->hCom = INVALID_HANDLE_VALUE)
			return -1;
	}

	data->isCH9326 = false;

		// 设置缓冲区,输入/输出大小(字节数)
	SetupComm( /*COMFileTemp*/data->hCom,4096,4096) ;
	// 指定监视事件_收到字符放入缓冲区
	SetCommMask(/*COMFileTemp*/data->hCom, EV_RXCHAR ) ;

	COMMTIMEOUTS CommTimeOuts;
	CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF ;
	CommTimeOuts.ReadTotalTimeoutMultiplier = 0 ;
	CommTimeOuts.ReadTotalTimeoutConstant = 1000 ;
	CommTimeOuts.WriteTotalTimeoutMultiplier = 2*speed/9600 ;
	CommTimeOuts.WriteTotalTimeoutConstant = 0 ;
	//给定串口读与操作限时
	SetCommTimeouts(/*COMFileTemp*/data->hCom, &CommTimeOuts ) ;

	//设置串口参数:波特率=9600;停止位 1个;无校验;8位
	DCB dcb ;
	dcb.DCBlength = sizeof( DCB ) ;
	GetCommState( /*COMFileTemp*/data->hCom, &dcb ) ;
	dcb.BaudRate =speed;
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
	SetCommState(/*COMFileTemp*/data->hCom, &dcb ) ;

	PurgeComm( /*COMFileTemp*/data->hCom, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR ) ;

	//指定串口执行扩展功能
	EscapeCommFunction( /*COMFileTemp*/data->hCom, SETDTR ) ;

	data->inportlen = 128;
	data->outportlen = 128;
	HIDStartRead(data);
	return 0;
}

int HIDOpen(CString name, unsigned speed, struct readThreadData *data)
{/*
	data->inportlen = 32;
	data->outportlen = 32;
	data->hCom = (HANDLE)32;
	return HIDStartRead(data);*/
	CWorkThread *thread = (CWorkThread *)data->threadReal;
	if(thread->open(name, speed))
		return 0;
	return -1;

	int err = COMOpen(name, speed, data);
	if(0 >= err)
		return err;

	HANDLE hHID = INVALID_HANDLE_VALUE;

	hHID = CH9326OpenDevicePath((PCHAR)LPCTSTR (name));
	if(hHID==INVALID_HANDLE_VALUE)
	{
		AfxMessageBox("打开HID设备失败");
		return -1;
	}

	USHORT VID,PID,VER;
	//获取厂商ID和设备ID
	if(!CH9326GetAttributes(hHID,&VID,&PID,&VER)){
		CH9326CloseDevice(hHID);
		return -1;
	}

	char version[100]="";
	sprintf(version,"设备已连接,VID=%XPID=%X VER=%X ",VID,PID,VER);
		
	USHORT inportlen = 0, outportlen = 0;
	
	if(VID == USB_VID && PID == USB_PID){
		CH9326GetBufferLen(hHID,&inportlen,&outportlen);
		CH9326SetTimeOut(hHID,3000,3000);
		if(!CH9326SetRate(hHID, HIDSpeed(speed), 4, 1, 4, 48)){
			CH9326CloseDevice(hHID);
			AfxMessageBox("设置不成功!");
			return -1;
		}
	}

	if(!CH9326InitThreadData(hHID)){
		CH9326CloseDevice(hHID);
		AfxMessageBox("初始化不成功!");
		return -1;
	}

	data->inportlen = inportlen;
	data->outportlen = outportlen;
	data->hCom = hHID;
	data->isCH9326 = true;


	HIDStartRead(data);
	return 0;
	//return CreateFile(name.GetBuffer(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
}

void HIDSearch(vector<CString> &vstr)
{
    HANDLE hidHandle;
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO hDevInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return;
    }

    SP_DEVICE_INTERFACE_DATA devInfoData;
    devInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
    int deviceNo = 0;
    SetLastError(NO_ERROR);
    while (SetupDiEnumInterfaceDevice (hDevInfo, 0, &hidGuid, deviceNo, &devInfoData)) {

        ULONG requiredLength = 0;
        SetupDiGetInterfaceDeviceDetail(hDevInfo, &devInfoData, NULL, 0, &requiredLength, NULL);

        PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail = (SP_INTERFACE_DEVICE_DETAIL_DATA *) malloc (requiredLength);
        devDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

        if(!SetupDiGetInterfaceDeviceDetail(hDevInfo, &devInfoData, devDetail, requiredLength, NULL, NULL)) {
            free(devDetail);
            SetupDiDestroyDeviceInfoList(hDevInfo);
            return;
        }

		hidHandle = CreateFile(devDetail->DevicePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
        
        if (hidHandle == INVALID_HANDLE_VALUE) {
            free(devDetail);
			++deviceNo;
            continue;
        }

        printf("打开成功\n");
        _HIDD_ATTRIBUTES hidAttributes;
        if(!HidD_GetAttributes(hidHandle, &hidAttributes)) {
            CloseHandle(hidHandle);
            SetupDiDestroyDeviceInfoList(hDevInfo);
			free(devDetail);
            return;
        }

        if (USB_VID == hidAttributes.VendorID && USB_PID == hidAttributes.ProductID) {
            printf("找到设备!\n");
			vstr.push_back(devDetail->DevicePath);
        }

		free(devDetail);
        CloseHandle(hidHandle);
		++deviceNo;
    }
}


void HIDSampleFunc()
{
	vector<CString> vstr;
	HIDSearch(vstr);
	if (0 == vstr.size()){
		::MessageBox(0, TEXT("设备不存在"), TEXT("标题"), MB_OKCANCEL);
        return;
	}

	struct readThreadData threadData;
    if (HIDOpen(vstr[0], 9600, &threadData)){
		::MessageBox(0, TEXT("打开设备失败"), TEXT("标题"), MB_OKCANCEL);
        return;
	}

	char buf[128] = {0};
	char test[512] = {0};
	int err = 0;

    for(int i = 0; i < 24; i++) {

		err = HIDRead(threadData.hCom, buf, sizeof(buf), 100);//error;//

		if(err>0){
			printf("%x \n", err);
			//::MessageBox(0, TEXT("接收到数据"), TEXT("标题"), MB_OKCANCEL);
		
			for(int i = 0; i < err; i++) {
				sprintf(test+i*2, "%02x", buf[i]);
			}

			printf("结束\n");
		}
        
    }

	CloseHandle(threadData.hCom);
}

int _tmain_1(int argc, _TCHAR *argv[])
{
    HIDSampleFunc();
    // MessageBox(NULL,(LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
    // Free the buffer.
    // LocalFree( lpMsgBuf );
    system("PAUSE");
    return 0;
}

HANDLE OpenMyHIDDevice(int overlapped)
{
    HANDLE hidHandle;
    GUID hidGuid;
    HidD_GetHidGuid(&hidGuid);
    HDEVINFO hDevInfo = SetupDiGetClassDevs(
                            &hidGuid,
                            NULL,
                            NULL,
                            (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }
    SP_DEVICE_INTERFACE_DATA devInfoData;
    devInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
    int deviceNo = 0;
    SetLastError(NO_ERROR);
    while (GetLastError() != ERROR_NO_MORE_ITEMS) {
        if (SetupDiEnumInterfaceDevice (hDevInfo,
                                        0,
                                        &hidGuid,
                                        deviceNo,
                                        &devInfoData)) {
            ULONG requiredLength = 0;
            SetupDiGetInterfaceDeviceDetail(hDevInfo,
                                            &devInfoData,
                                            NULL,
                                            0,
                                            &requiredLength,
                                            NULL);
            PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail =
                (SP_INTERFACE_DEVICE_DETAIL_DATA *) malloc (requiredLength);
            devDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
            if(!SetupDiGetInterfaceDeviceDetail(hDevInfo,
                                                &devInfoData,
                                                devDetail,
                                                requiredLength,
                                                NULL,
                                                NULL)) {
                free(devDetail);
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return INVALID_HANDLE_VALUE;
            }
            /*----------------------该处为vid-pid-guid的比较后作出标志位------------*/
            char vidpidguid[30];
            //sprintf(vidpidguid, "%s", "\\\\?\\hid#vid_16c0&pid_05e1#"); //“\\”为转移字符，要改成这样
			sprintf(vidpidguid, "\\\\?\\hid#vid_%04x&pid_%04x#", USB_VID, USB_PID); //“\\”为转移字符，要改成这样
            int hidcheck = 0;
            for(int i = 0 ; i < 25; i++) {
                if(devDetail->DevicePath[i] == vidpidguid[i]) {
                    hidcheck++;
                }
                printf("%c", devDetail->DevicePath[i]);
            }
            printf("======%d\n", hidcheck);
            /*-------------------------------------------------------------------*/
            if (overlapped) {
                if(hidcheck == 25) { //vid-pid-guid的比较正确，则使用GENERIC_READ | GENERIC_WRITE访问
                    hidHandle = CreateFile(devDetail->DevicePath,
                                           GENERIC_READ | GENERIC_WRITE,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING,
                                           FILE_FLAG_OVERLAPPED,
                                           NULL);
                    printf("使用读写方式打开\n");
                } else {
                    //否则作为独占方式访问
                    hidHandle = CreateFile(devDetail->DevicePath,
                                           0,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING,
                                           FILE_FLAG_OVERLAPPED,
                                           NULL);
                    printf("系统独占方式打开\n");
                }
            } else {
                if(hidcheck == 25) { //vid-pid-guid的比较正确，则使用GENERIC_READ | GENERIC_WRITE访问
                    hidHandle = CreateFile(devDetail->DevicePath, //访问路径
                                           GENERIC_READ | GENERIC_WRITE, //访问方式必须为零，由于鼠标键盘HID是系统独占，所以会出错
                                           FILE_SHARE_READ | FILE_SHARE_WRITE, //共享模式
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING, //文件不存在是返回失败
                                           0, //若为FILE_FLAG_OVERLAPPED以重叠（异步）模式打开
                                           NULL
                                          );
                    printf("使用读写方式打开\n");
                } else {
                    //否则作为独占方式访问
                    hidHandle = CreateFile(devDetail->DevicePath, //访问路径
                                           0, //访问方式必须为零，由于鼠标键盘HID是系统独占，所以会出错
                                           FILE_SHARE_READ | FILE_SHARE_WRITE, //共享模式
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING, //文件不存在是返回失败
                                           0, //若为FILE_FLAG_OVERLAPPED以重叠（异步）模式打开
                                           NULL
                                          );
                    printf("系统独占方式打开\n");
                }
            }
            free(devDetail);
            if (hidHandle == INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(hDevInfo);
                //free(devDetail);
                return INVALID_HANDLE_VALUE;
            }
            printf("打开成功\n");
            _HIDD_ATTRIBUTES hidAttributes;
            if(!HidD_GetAttributes(hidHandle, &hidAttributes)) {
                CloseHandle(hidHandle);
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return INVALID_HANDLE_VALUE;
            }
            if (USB_VID == hidAttributes.VendorID
                && USB_PID == hidAttributes.ProductID) {
                printf("找到设备!\n");
                break;
            } else {
                CloseHandle(hidHandle);
                ++deviceNo;
            }
        }
    }

    SetupDiDestroyDeviceInfoList(hDevInfo);
    return hidHandle;
}


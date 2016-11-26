// hid.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cfgmgr32.h>
#include <time.h>

extern "C" {  
#include "setupapi.h" 
#include "hidsdi.h" 
}

#define USB_VID 0x1A86
#define USB_PID 0xE010


//#define USB_VID 0xFFFF
//#define USB_PID 0x0035

HANDLE OpenMyHIDDevice(int overlapped);
LPVOID lpMsgBuf;
/*用FormatMessage()得到由GetLastError()返回的出错编码所对应错误信息*/
#define error FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL )
/*---------------------------------------------*/
BYTE lpBuf[8];

bool write(HANDLE hComm)
{
    OVERLAPPED osWrite = {0};
    DWORD dwWritten;
    BOOL fRes;
    memset(lpBuf, 0, 8);//数组清零
    lpBuf[0] = 2;
    lpBuf[1] = 4;
    // Create this writes OVERLAPPED structure hEvent.
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL)
        // Error creating overlapped event handle.
        return false;
    // Issue write.
    if (!WriteFile(hComm, lpBuf, 8, &dwWritten, &osWrite)) {
        if (GetLastError() != ERROR_IO_PENDING) {
            // WriteFile failed, but it isn't delayed. Report error and abort.
            fRes = false;
        } else {
            // Write is pending.
#define WRITE_TIMEOUT 500 //write timeout
            DWORD dwRes = WaitForSingleObject(osWrite.hEvent, WRITE_TIMEOUT);
            switch(dwRes) {
            // Read completed.
            case WAIT_OBJECT_0:
                if (!GetOverlappedResult(hComm, &osWrite, &dwWritten, TRUE))
                    fRes = FALSE;
                else
                    // Write operation completed successfully.
                    fRes = true;
            case WAIT_TIMEOUT:
                // This is a good time to do some background work.
                break;
            default:
                // Error in the WaitForSingleObject; abort.
                // This indicates a problem with the OVERLAPPED structure's
                // event handle.
                break;
            }
        }
    } else
        // WriteFile completed immediately.
        fRes = true;
    CloseHandle(osWrite.hEvent);
    return fRes;
}


bool read(HANDLE hComm)
{
    DWORD dwRead;
    BOOL fWaitingOnRead = FALSE;
    OVERLAPPED osReader = {0};
    memset(lpBuf, 0, 8);//数组清零
    // Create the overlapped event. Must be closed before exiting
    // to avoid a handle leak.
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osReader.hEvent == NULL) {
        // Error creating overlapped event; abort.
        return false;
    }
    if (!fWaitingOnRead) {
        // Issue read operation.
        if (!ReadFile(hComm, lpBuf, 8, &dwRead, &osReader)) {
            // error;//错误查询
            if (GetLastError() != ERROR_IO_PENDING) { // read not delayed?
                // Error in communications; report it.
            } else {
                fWaitingOnRead = TRUE;
            }
        } else {
            // read completed immediately
            // HandleASuccessfulRead(lpBuf, dwRead);
        }
    }
    if (fWaitingOnRead) {
#define READ_TIMEOUT 500 // milliseconds
        DWORD dwRes = WaitForSingleObject(osReader.hEvent, READ_TIMEOUT);
        switch(dwRes) {
        // Read completed.
        case WAIT_OBJECT_0:
            if (!GetOverlappedResult(hComm, &osReader, &dwRead, FALSE)) {
                // Error in communications; report it.
            } else {
                // Read completed successfully.
                //HandleASuccessfulRead(lpBuf, dwRead);
            }
            // Reset flag so that another opertion can be issued.
            fWaitingOnRead = false;
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
    }
    return true;
}
void HIDSampleFunc()
{
    HANDLE hDev;
    hDev = OpenMyHIDDevice(1); //打开设备，使用重叠（异步）方式;
    printf("传递设备句柄:%x\n", hDev);
    if (hDev == INVALID_HANDLE_VALUE)
        return;
    /*//不使用异步方式的时候的代码
    // BYTE recvDataBuf[8];
    // BYTE reportBuf[8];
    // DWORD bytes;
    memset(reportBuf, 0, 8);//数组清零
    reportBuf[0] = 4; //输出报告的报告ID是4
    reportBuf[1] = 1;
    if (!WriteFile(hDev,// 设备句柄，即 CreateFile 的返回值
    reportBuf,// 存有待发送数据的 buffer
    8, // 待发送数据的长度
    &bytes, // 实际收到的数据的字节数
    0// 异步模式
    )) //写入数据到设备
    {

    return;}
    // Sleep(1500);
    printf("开始接收");
    ReadFile(hDev, recvDataBuf, 8, &bytes, NULL); //读取设备发给主机的数
    printf("结束");
    */
    for(int i = 0; i < 8; i++) {
        printf("开始写入\n");
        write(hDev);
        printf("开始接收\n");
        read(hDev);//error;//错误查询
        for(int i = 0; i < 8; i++) {
            printf("%x ", lpBuf[i]);
        }
        printf("结束\n");
    }
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
            sprintf(vidpidguid, "%s", "\\\\?\\hid#vid_16c0&pid_05e1#"); //“\\”为转移字符，要改成这样
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
                free(devDetail);
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



int _tmain_1(int argc, _TCHAR *argv[])
{
    HIDSampleFunc();
    // MessageBox(NULL,(LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
    // Free the buffer.
    // LocalFree( lpMsgBuf );
    system("PAUSE");
    return 0;
}


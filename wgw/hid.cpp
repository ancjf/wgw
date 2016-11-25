// hid.cpp : �������̨Ӧ�ó������ڵ㡣
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

//#define USB_VID 0x1A86
//#define USB_PID 0xE010


#define USB_VID 0xFFFF
#define USB_PID 0x0035

HANDLE OpenMyHIDDevice(int overlapped);
LPVOID lpMsgBuf;
/*��FormatMessage()�õ���GetLastError()���صĳ�����������Ӧ������Ϣ*/
#define error FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL )
/*---------------------------------------------*/
BYTE lpBuf[8];

bool write(HANDLE hComm)
{
    OVERLAPPED osWrite = {0};
    DWORD dwWritten;
    BOOL fRes;
    memset(lpBuf, 0, 8);//��������
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
    memset(lpBuf, 0, 8);//��������
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
            // error;//�����ѯ
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
    hDev = OpenMyHIDDevice(1); //���豸��ʹ���ص����첽����ʽ;
    printf("�����豸���:%x\n", hDev);
    if (hDev == INVALID_HANDLE_VALUE)
        return;
    /*//��ʹ���첽��ʽ��ʱ��Ĵ���
    // BYTE recvDataBuf[8];
    // BYTE reportBuf[8];
    // DWORD bytes;
    memset(reportBuf, 0, 8);//��������
    reportBuf[0] = 4; //�������ı���ID��4
    reportBuf[1] = 1;
    if (!WriteFile(hDev,// �豸������� CreateFile �ķ���ֵ
    reportBuf,// ���д��������ݵ� buffer
    8, // ���������ݵĳ���
    &bytes, // ʵ���յ������ݵ��ֽ���
    0// �첽ģʽ
    )) //д�����ݵ��豸
    {

    return;}
    // Sleep(1500);
    printf("��ʼ����");
    ReadFile(hDev, recvDataBuf, 8, &bytes, NULL); //��ȡ�豸������������
    printf("����");
    */
    for(int i = 0; i < 8; i++) {
        printf("��ʼд��\n");
        write(hDev);
        printf("��ʼ����\n");
        read(hDev);//error;//�����ѯ
        for(int i = 0; i < 8; i++) {
            printf("%x ", lpBuf[i]);
        }
        printf("����\n");
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
            /*----------------------�ô�Ϊvid-pid-guid�ıȽϺ�������־λ------------*/
            char vidpidguid[30];
            sprintf(vidpidguid, "%s", "\\\\?\\hid#vid_16c0&pid_05e1#"); //��\\��Ϊת���ַ���Ҫ�ĳ�����
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
                if(hidcheck == 25) { //vid-pid-guid�ıȽ���ȷ����ʹ��GENERIC_READ | GENERIC_WRITE����
                    hidHandle = CreateFile(devDetail->DevicePath,
                                           GENERIC_READ | GENERIC_WRITE,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING,
                                           FILE_FLAG_OVERLAPPED,
                                           NULL);
                    printf("ʹ�ö�д��ʽ��\n");
                } else {
                    //������Ϊ��ռ��ʽ����
                    hidHandle = CreateFile(devDetail->DevicePath,
                                           0,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING,
                                           FILE_FLAG_OVERLAPPED,
                                           NULL);
                    printf("ϵͳ��ռ��ʽ��\n");
                }
            } else {
                if(hidcheck == 25) { //vid-pid-guid�ıȽ���ȷ����ʹ��GENERIC_READ | GENERIC_WRITE����
                    hidHandle = CreateFile(devDetail->DevicePath, //����·��
                                           GENERIC_READ | GENERIC_WRITE, //���ʷ�ʽ����Ϊ�㣬����������HID��ϵͳ��ռ�����Ի����
                                           FILE_SHARE_READ | FILE_SHARE_WRITE, //����ģʽ
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING, //�ļ��������Ƿ���ʧ��
                                           0, //��ΪFILE_FLAG_OVERLAPPED���ص����첽��ģʽ��
                                           NULL
                                          );
                    printf("ʹ�ö�д��ʽ��\n");
                } else {
                    //������Ϊ��ռ��ʽ����
                    hidHandle = CreateFile(devDetail->DevicePath, //����·��
                                           0, //���ʷ�ʽ����Ϊ�㣬����������HID��ϵͳ��ռ�����Ի����
                                           FILE_SHARE_READ | FILE_SHARE_WRITE, //����ģʽ
                                           (LPSECURITY_ATTRIBUTES)NULL,
                                           OPEN_EXISTING, //�ļ��������Ƿ���ʧ��
                                           0, //��ΪFILE_FLAG_OVERLAPPED���ص����첽��ģʽ��
                                           NULL
                                          );
                    printf("ϵͳ��ռ��ʽ��\n");
                }
            }
            free(devDetail);
            if (hidHandle == INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(hDevInfo);
                free(devDetail);
                return INVALID_HANDLE_VALUE;
            }
            printf("�򿪳ɹ�\n");
            _HIDD_ATTRIBUTES hidAttributes;
            if(!HidD_GetAttributes(hidHandle, &hidAttributes)) {
                CloseHandle(hidHandle);
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return INVALID_HANDLE_VALUE;
            }
            if (USB_VID == hidAttributes.VendorID
                && USB_PID == hidAttributes.ProductID) {
                printf("�ҵ��豸!\n");
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

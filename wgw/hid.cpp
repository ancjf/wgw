// hid.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <cfgmgr32.h>
#include <time.h>

#include <vector>
#include "hid.h"

using namespace std;

extern "C" {  
#include "setupapi.h" 
#include "hidsdi.h" 
}

//#define USB_VID 0x1A86
//#define USB_PID 0xE010

#define USB_VID 0x093A
#define USB_PID 0x2510

//#define USB_VID 0xFFFF
//#define USB_PID 0x0035

HANDLE OpenMyHIDDevice(int overlapped);
LPVOID lpMsgBuf;
/*��FormatMessage()�õ���GetLastError()���صĳ����������Ӧ������Ϣ*/
#define error FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL )
/*---------------------------------------------*/
BYTE lpBuf[8];

int HIDWrite(HANDLE hComm, const char *buf, unsigned len, unsigned timeout)
{
	DWORD dwWritten;
	if(WriteFile(hComm, buf, len, &dwWritten, 0))
		return dwWritten;

	return -1;
}

int HIDRead(HANDLE hComm, char *buf, unsigned size, unsigned timeout)
{
    DWORD dwRead;
    OVERLAPPED osReader = {0};
    memset(lpBuf, 0, 8);//��������
    // Create the overlapped event. Must be closed before exiting
    // to avoid a handle leak.
    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osReader.hEvent == NULL) {
        // Error creating overlapped event; abort.
        return -1;
    }

    if (!ReadFile(hComm, buf, size, &dwRead, &osReader)) {
        // error;//�����ѯ
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

HANDLE HIDOpen(CString name)
{
	return CreateFile(name.GetBuffer(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
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

		hidHandle = CreateFile(devDetail->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
        
        if (hidHandle == INVALID_HANDLE_VALUE) {
            SetupDiDestroyDeviceInfoList(hDevInfo);
            free(devDetail);
            return;
        }

        printf("�򿪳ɹ�\n");
        _HIDD_ATTRIBUTES hidAttributes;
        if(!HidD_GetAttributes(hidHandle, &hidAttributes)) {
            CloseHandle(hidHandle);
            SetupDiDestroyDeviceInfoList(hDevInfo);
			free(devDetail);
            return;
        }

        if (USB_VID == hidAttributes.VendorID && USB_PID == hidAttributes.ProductID) {
            printf("�ҵ��豸!\n");
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
		::MessageBox(0, TEXT("�豸������"), TEXT("����"), MB_OKCANCEL);
        return;
	}

    HANDLE hDev = HIDOpen(vstr[0]); //���豸��ʹ���ص����첽����ʽ;
    printf("�����豸���:%x\n", hDev);
    if (hDev == INVALID_HANDLE_VALUE){
		::MessageBox(0, TEXT("���豸ʧ��"), TEXT("����"), MB_OKCANCEL);
        return;
	}

	char buf[128] = {0};
	char test[512] = {0};
	int err = 0;

    for(int i = 0; i < 24; i++) {

        err = HIDRead(hDev, buf, sizeof(buf), 100);//error;//

		if(err>0){
			printf("%x \n", err);
			//::MessageBox(0, TEXT("���յ�����"), TEXT("����"), MB_OKCANCEL);
		
			for(int i = 0; i < err; i++) {
				sprintf(test+i*2, "%02x", buf[i]);
			}

			printf("����\n");
		}
        
    }

	CloseHandle(hDev);
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
            /*----------------------�ô�Ϊvid-pid-guid�ıȽϺ�������־λ------------*/
            char vidpidguid[30];
            //sprintf(vidpidguid, "%s", "\\\\?\\hid#vid_16c0&pid_05e1#"); //��\\��Ϊת���ַ���Ҫ�ĳ�����
			sprintf(vidpidguid, "\\\\?\\hid#vid_%04x&pid_%04x#", USB_VID, USB_PID); //��\\��Ϊת���ַ���Ҫ�ĳ�����
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
                //free(devDetail);
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


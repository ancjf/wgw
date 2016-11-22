#define USB_VID 0xFC0

#define USB_PID 0x420

HANDLE OpenMyHIDDevice(int overlapped);

void HIDSampleFunc()
{
    HANDLE        hDev;
    BYTE        recvDataBuf[8];
    BYTE        reportBuf[8];
    DWORD        bytes;
    hDev = OpenMyHIDDevice(0); //打开设备，不使用重叠（异步）方式;
    if (hDev == INVALID_HANDLE_VALUE)
        return;
    reportBuf[0] = 4; //输出报告的报告ID是4
    memset(reportBuf, 0, 8);
    reportBuf[1] = 1;
    if (!WriteFile(hDev, reportBuf, 8, &bytes, NULL)) //写进数据到设备
        return;
    ReadFile(hDev, recvDatatBuf, 8, &bytes, NULL); //读取设备发给主机的数据
}

HANDLE OpenMyHIDDevice(int overlapped)
{
    HANDLE     hidHandle;
    GUID     hidGuid;
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
        if (SetupDiEnumInte ***ceDevice (hDevInfo,
                                         0,
                                         &hidGuid,
                                         deviceNo,
                                         &devInfoData)) {
            ULONG requiredLength = 0;
            SetupDiGetInte ***ceDeviceDetail(hDevInfo,
                                             &devInfoData,
                                             NULL,
                                             0,
                                             &requiredLength,
                                             NULL);
            PSP_INTERFACE_DEVICE_DETAIL_DATA devDetail =
                (SP_INTERFACE_DEVICE_DETAIL_DATA *) malloc (requiredLength);
            devDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
            if(!SetupDiGetInte ***ceDeviceDetail(hDevInfo,
                                                 &devInfoData,
                                                 devDetail,
                                                 requiredLength,
                                                 NULL,
                                                 NULL)) {
                free(devDetail);
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return INVALID_HANDLE_VALUE;
            }
            if (overlapped) {
                hidHandle = CreateFile(devDetail->DevicePath,
                                       GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_EXISTING,
                                       FILE_FLAG_OVERLAPPED,
                                       NULL);
            } else {
                hidHandle = CreateFile(devDetail->DevicePath,
                                       GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_EXISTING,
                                       0,
                                       NULL);
            }
            free(devDetail);
            if (hidHandle == INVALID_HANDLE_VALUE) {
                SetupDiDestroyDeviceInfoList(hDevInfo);
                free(devDetail);
                return INVALID_HANDLE_VALUE;
            }
            _HIDD_ATTRIBUTES hidAttributes;
            if(!HidD_GetAttributes(hidHandle, &hidAttributes)) {
                CloseHandle(hidHandle);
                SetupDiDestroyDeviceInfoList(hDevInfo);
                return INVALID_HANDLE_VALUE;
            }
            if (USB_VID == hidAttributes.VendorID
                && USB_PID == hidAttributes.ProductID) {
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



http://bbs.21ic.com/icview-455726-1-1.html

HID 数据传输上位机READFILE不到数据的问题

一个AVR和上位机的通讯实验，上位机VC2010
功能：上位机发送8位数据，HID设备返回相同数据
BUS HOUND 能检测出数据返回数据，可是READFILE（）读不出数据，这是为什么？
找了两天的资料，说什么的都有呀，头快爆炸了



固件程序代码

char usbDescriptorHidReport[33] = { // USB report descriptor
    0x06, 0x00, 0xFF, //USAGE_PAGE (Vendor Defined Page 1)
    0x09, 0x01, //USAGE (Vendor Usage 1)
    0xA1, 0x01, //COLLECTION (Application)
    0x19, 0x01, //(Vendor Usage 1)
    0x29, 0x08, //(Vendor Usage 1)
    0x85, 0x01, //报告ID(1)
    0x15, 0x00, //LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00, //LOGICAL_MAXIMUM (255)
    0x75, 0x08, //REPORT_SIZE (8)
    0x95, 0x07, //REPORT_COUNT (7)
    0x81, 0x02, //INPUT (Data,Var,Abs)
    0x19, 0x01, //(Vendor Usage 1)
    0x29, 0x08, //(Vendor Usage 1)
    0x85, 0x02, //报告ID(2)上位机VC-WriteFile函数缓冲区0位必须设置此ID数
    0x91, 0x02, //OUTPUT (Data,Var,Abs)
    0xC0 // END_COLLECTION // END_COLLECTION
};
static uchar rcbuf[8] = {0};
static uchar rpbuf[8] = {0};
void write()//AVRM16属低速设备，每次只能发8个字节
{
    if(usbInterruptIsReady()) {
        usbSetInterrupt(rcbuf, sizeof(rcbuf));
    }
    return;
}

uchar usbFunctionSetup(uchar data[8])
{
    //LCDprintf(1,(void *)data);
    /* 这里你可以根据data[8]里面的参数做一些判断，决定返回哪些数据 */
    return USB_NO_MSG; //USB_NO_MSG=0xff
}
/*
上位机通过函数要求读数据(UsbRequestType.EndpointIn)，
vusb Driver则自动会调用usbFunctionRead()函数来把数据
(rpbuf数组里的数据)返回给上位机，其中len参数就是的参
数bufferLen。
*/
uchar usbFunctionRead(uchar *data, uchar len)
{
    uchar i;
    for(i = 0; i < len; i++) {
        data[i] = rpbuf[i];
    }
    // LCDprintf(2,(void *)data);
    return len;
}
/*
如果上位机通过函数要求向单片机写数据(UsbRequestType.EndpointOut)，
vusb Driver自动会调用usbFunctionWrite()函数来接收上位机传来的数据，
并保存到rcbuf数组中。
*/
uchar usbFunctionWrite(uchar *data, uchar len)
{
    uchar i;
    for(i = 0; i < len; i++) {
        rcbuf[i] = data[i];
    }
    write();
    // LCDprintf(3,(void *)rcbuf);
    return len;
}

/* ------------------------------------------------------------------------- */
int main(void)
{
    DDRC = 0xFF;
    PORTC = 0xFF;
    init_devices();
    _delay_ms(50);
    LCD_init(); //LCD初始化
    LCD_clear(); //清屏
    clear_img();
    DDRC = 0xff;
    PORTC = 0xff;
    LCDprintf(1, "USBhid通讯实验");
    wdt_enable(WDTO_2S);
    // odDebugInit();
    usbInit();
    sei();
    // DBG1(0x00, 0, 0);
    for(;;) { /* main event loop */
        wdt_reset();
        usbPoll();
    }
    return 0;
}
复制代码


上位机VC代码

#define USB_VID 0x16c0
#define USB_PID 0x05e1
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



int _tmain(int argc, _TCHAR *argv[])
{
    HIDSampleFunc();
    // MessageBox(NULL,(LPCTSTR)lpMsgBuf, _T("Error"), MB_OK | MB_ICONINFORMATION );
    // Free the buffer.
    // LocalFree( lpMsgBuf );
    system("PAUSE");
    return 0;
}
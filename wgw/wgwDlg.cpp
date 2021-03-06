﻿
// wgwDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "wgw.h"
#include "wgwDlg.h"
#include "afxdialogex.h"
#include "EnumSerial.h"
#include "hid.h"
#include "CH9326DBG.h"
#include "CH9326DLL.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C" {  
#include "setupapi.h" 
#include "hidsdi.h" 
}

#pragma comment(lib, "setupapi.lib") 
#pragma comment(lib, "CH9326DLL.lib") 

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

int CwgwDlg::getMsgOne(TCHAR *in, unsigned len)
{
	switch(in[2]){
		case TEXT('\x2'):	//答题对
			setOpenAnswerEd(true);
			m_dlgAnswer.processMsgAnswer(in);
			//m_dlgOther.msgAppend(start, 6);
			//msgAppend(m_editOther, start, 18);
			//msgAppend(m_editAnswer, start, 18);
			return 0;
		case TEXT('\x5'):	//考勤
			setCheckOn(true);
			m_dlgCheck.processMsgCheck(in);
			//msgAppend(str, start, 13);
			return 0;
		case TEXT('\x4'):	//开考勤
		case TEXT('\x6'):	//关考勤
			setCheckOn(in[2] == TEXT('\x4'));
			//msgAppend(m_editAnswer, start, 3);
			return 0;
		case TEXT('\x1'):	//开答题
		case TEXT('\x3'):	//关答题
			setOpenAnswerEd(in[2] == TEXT('\x1'));
			//msgAppend(str, start, 3);
			return 0;
		case TEXT('\x7'):	//校时
			m_dlgOther.msgAppend(in, 6);
			return 0;
		default:
			return -1;
	}

	return -1;
}


void CwgwDlg::GetComLis(CComboBox * CCombox)
{
	CArray<SSerInfo,SSerInfo&> asi;

	// Populate the list of serial ports.
	EnumSerialPorts(asi,FALSE/*include all*/);
	CCombox->ResetContent();
	CCombox->SetDroppedWidth(500);
	//CCombox->SendMessage(CB_SETITEMHEIGHT,100,600);
	for (int ii=0; ii<asi.GetSize(); ii++) {
		CString strPortName = asi[ii].strPortName;
		CString strDevPath = asi[ii].strDevPath;
		CCombox->AddString(asi[ii].strFriendlyName);
	}
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CwgwDlg 对话框




CwgwDlg::CwgwDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CwgwDlg::IDD, pParent)
	, m_editOther(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_comStateStr = TEXT("串口未打开");
	m_commOpened = false;
	m_openAnswerEd = false;
	m_checkOn = false;
	m_dataLen = 0;

	memset(&threadData, 0, sizeof(threadData));
	threadData.hWnd = GetSafeHwnd();
	threadData.hCom = INVALID_HANDLE_VALUE;
}

void CwgwDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDIT_OTHER, m_editOther);
	//DDX_Control(pDX, IDC_LIST_CHECK, m_listCheck);
	//DDX_Control(pDX, IDC_LIST_ANSWER, m_listAnswer);
	DDX_Control(pDX, IDC_TAB1, m_tabctrl);
}

BEGIN_MESSAGE_MAP(CwgwDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPENANSWER, &CwgwDlg::OnBnClickedOpenanswer)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LINK, &CwgwDlg::OnBnClickedLink)
	ON_BN_CLICKED(IDC_OPEN, &CwgwDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_CLOSEANWER, &CwgwDlg::OnBnClickedCloseanwer)
	ON_BN_CLICKED(IDC_CHECKON, &CwgwDlg::OnBnClickedCheckon)
	ON_BN_CLICKED(IDC_CHECKOFF, &CwgwDlg::OnBnClickedCheckoff)
	ON_BN_CLICKED(IDC_TIMING, &CwgwDlg::OnBnClickedTiming)
	ON_EN_CHANGE(IDC_RFID, &CwgwDlg::OnEnChangeRfid)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CwgwDlg::OnTcnSelchangeTab1)
	ON_MESSAGE(WM_USER_THREADEND, OnUserThreadend)
	ON_MESSAGE(WM_USER_NEWLINK, OnUsernNewLink)
END_MESSAGE_MAP()


// CwgwDlg 消息处理程序

BOOL CwgwDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	TCHAR id[16];
	GetProfileString(TEXT("wgw"), TEXT("speed"), TEXT("01"), id, ARRAY_SIZE(id));
	CComboBox * CCombox = (CComboBox *)GetDlgItem(IDC_SPEED);


	CCombox->AddString(_T("300"));
	CCombox->AddString(_T("600"));
	CCombox->AddString(_T("1200"));
	CCombox->AddString(_T("2400"));
	CCombox->AddString(_T("4800"));
	CCombox->AddString(_T("9600"));
	CCombox->AddString(_T("14400"));
	CCombox->AddString(_T("19200"));
	CCombox->AddString(_T("28800"));
	CCombox->AddString(_T("38400"));
	CCombox->AddString(_T("57600"));
	CCombox->AddString(_T("76800"));
	CCombox->AddString(_T("115200"));
	
	CCombox->SelectString(0, _T("76800"));
	CCombox->SelectString(0, id);

	((CEdit*)GetDlgItem(IDC_RFID))->SetLimitText(2);
	((CEdit*)GetDlgItem(IDC_RFID))->SetWindowText(TEXT("01"));

	GetProfileString(TEXT("wgw"), TEXT("id"), TEXT("01"), id, ARRAY_SIZE(id));
	//WriteProfileString(TEXT("wgw"), TEXT("id"), id);
	//GetPrivateProfileString(TEXT("wgw"), TEXT("id"), TEXT("01"), id, ARRAY_SIZE(id), TEXT(".\\info.ini"));
	((CEdit*)GetDlgItem(IDC_RFID))->SetWindowText(id);
	
	OnBnClickedLink();
	updateState();
	/*
	m_listCheck.InsertColumn(0,TEXT("序号"),LVCFMT_LEFT,60);              //添加列标题！！！！  这里的80,40,90用以设置列的宽度。！！！LVCFMT_LEFT用来设置对齐方式！！！
    m_listCheck.InsertColumn(1,TEXT("ID号"),LVCFMT_LEFT,100);
    m_listCheck.InsertColumn(2,TEXT("第一次收到时间"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(3,TEXT("最后一次收到时间"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(4,TEXT("答题卡电量"),LVCFMT_LEFT,100);

	m_listAnswer.InsertColumn(0,TEXT("序号"),LVCFMT_LEFT,60);              //添加列标题！！！！  这里的80,40,90用以设置列的宽度。！！！LVCFMT_LEFT用来设置对齐方式！！！
    m_listAnswer.InsertColumn(1,TEXT("ID号"),LVCFMT_LEFT,100);
    m_listAnswer.InsertColumn(2,TEXT("答题信息"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(3,TEXT("答题时间"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(4,TEXT("答题卡电量"),LVCFMT_LEFT,100);*/
	// TODO: 在此添加额外的初始化代码
	TCITEM item;
	item.mask = TCIF_TEXT;
	item.pszText = TEXT("考勤 ");
	m_tabctrl.InsertItem(0,&item);
	item.pszText = TEXT("答题 ");
	m_tabctrl.InsertItem(1, &item);
	item.pszText = TEXT("其他 ");
	m_tabctrl.InsertItem(2, &item);

	CRect rect;
	m_tabctrl.GetClientRect(&rect);
	rect.top+=23;
	rect.bottom-=4;
	rect.left+=3;
	rect.right-=4;

	CWnd *p = GetDlgItem(IDC_TAB1);
	m_dlgCheck.Create(IDD_DLG_CHECK, GetDlgItem(IDC_TAB1));
	m_dlgAnswer.Create(IDD_DLG_ANSWER, GetDlgItem(IDC_TAB1));
	m_dlgOther.Create(IDD_DLG_OTHER, GetDlgItem(IDC_TAB1));

	m_dlgCheck.MoveWindow(&rect);
	m_dlgAnswer.MoveWindow(&rect);
	m_dlgOther.MoveWindow(&rect);

	m_dlgCheck.ShowWindow(SW_SHOW);
	m_dlgAnswer.ShowWindow(SW_HIDE);
	m_dlgOther.ShowWindow(SW_HIDE);

	m_tabctrl.SetCurSel(0);

	m_pCommThread = (CWorkThread *)AfxBeginThread(RUNTIME_CLASS(CWorkThread), 0, 0, CREATE_SUSPENDED);
	m_pCommThread->ResumeThread();

	m_pTcpThread = (CTcpThread *)AfxBeginThread(RUNTIME_CLASS(CTcpThread), 0, 0, CREATE_SUSPENDED);
	m_pTcpThread->ResumeThread();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CwgwDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CwgwDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CwgwDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CwgwDlg::comOpened()
{
	//return m_commOpened;

	//if(!m_ctrlComm.get_PortOpen()){
	if(!m_commOpened){
		MessageBox(TEXT("串口没打开"));
		updateState();
		return false;
	}

	return true;
}

void CwgwDlg::OnBnClickedOpenanswer()
{
	// TODO: 在此添加控件通知处理程序代码
	//MessageBox(TEXT("hello"));
	if(!comOpened()){
		return;
	}

	sendComMsg(&m_ctrlComm, TEXT("\x01"));
	//setOpenAnswerEd(true);
}

BEGIN_EVENTSINK_MAP(CwgwDlg, CDialogEx)
END_EVENTSINK_MAP()

void CwgwDlg::OnCommMscomm1()
{
	// TODO: 在此处添加消息处理程序代码
	if(m_ctrlComm.get_CommEvent()==2)  {   
		TCHAR str[1024]={0};   
		long k;    
		VARIANT InputData=m_ctrlComm.get_Input(); //读缓冲区   
		COleSafeArray fs;

		fs=InputData; //VARIANT型变À量转换为COleSafeArray型变量   
		int datasize = fs.GetOneDimSize();
		for(k=0;k<datasize;k++)    
			fs.GetElement(&k,str+k); //转换为BYTE型数组        
		//m_EditReceive+=str;      // 接收到编辑框里面   //

		//addInString(str, datasize);
		SetTimer(1,10,NULL);  //延时10ms    
		UpdateData(false);
	}
}


void CwgwDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if(m_commOpened){
		//m_ctrlComm.put_PortOpen(false);
	}
	// TODO: 在此处添加消息处理程序代码
}

void InitSelDev(CComboBox *CCombox)
{
	int deviceNo=0;
	CHAR buf[1024];
	HANDLE hHID;
	
	
	SetLastError(NO_ERROR);
	while(GetLastError()!=ERROR_NO_MORE_ITEMS)
	{
			memset(buf,0,sizeof(buf));
			if( CH9326GetDevicePath(deviceNo,buf,sizeof(buf)) )
			{
				//打开设备,并获得设备句柄
				 hHID=CreateFile(buf,
					GENERIC_READ|GENERIC_WRITE,
					FILE_SHARE_READ|FILE_SHARE_WRITE,
					NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
				
				if(hHID==INVALID_HANDLE_VALUE){
					deviceNo++;
					continue;
				}
				
#if !DEBUG_TEST
				_HIDD_ATTRIBUTES hidAttributes;
				if(!HidD_GetAttributes(hHID, &hidAttributes)) {
					CloseHandle(hHID);
					deviceNo++;
					continue;
				}

				if (USB_VID != hidAttributes.VendorID || USB_PID != hidAttributes.ProductID) {
					deviceNo++;
					continue;
				}
#endif
				CharUpperBuff(buf,strlen(buf));
				CCombox->InsertString(0,(const char *)buf);
				CloseHandle(hHID);
				deviceNo++;
				
			}
			
			break;
	}
	
}

void CwgwDlg::OnBnClickedLink()
{
	// TODO: 在此添加控件通知处理程序代码
	CComboBox * CCombox = (CComboBox *)GetDlgItem(IDC_COMLIST);
	CArray<SSerInfo,SSerInfo&> asi;

	TCHAR com[128];
	GetProfileString(TEXT("wgw"), TEXT("com"), TEXT(""), com, ARRAY_SIZE(com));

	// Populate the list of serial ports.
	EnumSerialPorts(asi,FALSE/*include all*/);
	CCombox->ResetContent();
	CCombox->SetDroppedWidth(520);
	//CCombox->SendMessage(CB_SETITEMHEIGHT,100,600);
	for (int ii=0; ii<asi.GetSize(); ii++) {
		const CString &strPortName = asi[ii].strPortName;
		const CString &strDevPath = asi[ii].strDevPath;
		const CString &strFriendlyName = asi[ii].strFriendlyName;
		CCombox->AddString(strFriendlyName);
	}

	InitSelDev(CCombox);
	CCombox->SetCurSel(0);
	CCombox->SelectString(0, com);
}


void CwgwDlg::OnBnClickedOpen()
{
	CString text;
	GetDlgItemText(IDC_OPEN, text);

	threadData.threadReal = m_pCommThread;
	if(text != TEXT("打开")){
		/*
		if(threadData.hCom != INVALID_HANDLE_VALUE){
			HIDClose(&threadData);
		}else{
			m_ctrlComm.put_PortOpen(false);
		}*/
		HIDClose(&threadData);

		m_commOpened = false;
		SetDlgItemText(IDC_OPEN, TEXT("打开"));
		m_comStateStr = TEXT("串口未打开");
		updateState();
		return;
	}

	// TODO: 在此添加控件通知处理程序代码
	/*
	if(m_ctrlComm.get_PortOpen()){
		m_ctrlComm.put_PortOpen(false);
		SetDlgItemText(IDC_OPEN, TEXT("打开"));
		m_comStateStr = TEXT("串口未打开");
		updateState();
		return;
	}
	*/

	CString speed;
	GetDlgItemText(IDC_SPEED, speed);

	m_commOpened = true;
	CString strPortName;
	((CComboBox*)GetDlgItem(IDC_COMLIST))->GetWindowText(strPortName);
	int index = strPortName.Find(TEXT("com"));
	if(0 > index)
		index = strPortName.Find(TEXT("COM"));
	if(HIDOpen(strPortName, _ttoi(speed.GetBuffer()), &threadData)){
		//HIDOpen(strPortName, _ttoi(speed.GetBuffer()), &threadData);
		/*
		MessageBox(TEXT("串口不合法"));
		m_comStateStr = TEXT("串口未打开");
		m_comStateStr.Format(TEXT("串口未打开:%s"), strPortName.GetBuffer());
		SetDlgItemText(IDC_OPEN, TEXT("打开"));
		updateState();
		*/

		m_commOpened = false;
		SetDlgItemText(IDC_OPEN, TEXT("打开"));
		m_comStateStr = TEXT("串口未打开");
		updateState();
		return;
	}else{
		m_commOpened = true;
		m_comStateStr.Format(TEXT("hid,speed:"));
		m_comStateStr += speed;
		SetDlgItemText(IDC_OPEN, TEXT("关闭"));
		updateState();
		return;
	}


	CString str1 = speed+TEXT(",n,8,1");
	TCHAR *p = strPortName.GetBuffer()+index+3;
	int n = _tstoi(p);

	m_ctrlComm.put_CommPort(n);
	m_ctrlComm.put_InputMode(1);

	m_ctrlComm.put_Settings(str1);
	m_ctrlComm.put_InputLen(1024);
	m_ctrlComm.put_RThreshold(1);
	m_ctrlComm.put_RTSEnable(1);
	m_ctrlComm.put_PortOpen(true);

	m_comStateStr;
	m_comStateStr.Format(TEXT("com%d,speed:"), n);
	m_comStateStr += speed;
	SetDlgItemText(IDC_OPEN, TEXT("关闭"));

	WriteProfileString(TEXT("wgw"), TEXT("com"), strPortName);
	WriteProfileString(TEXT("wgw"), TEXT("speed"), speed);

	updateState();
}


void CwgwDlg::OnBnClickedCloseanwer()
{
	// TODO: 在此添加控件通知处理程序代码
	if(!comOpened()){
		return;
	}

	sendComMsg(&m_ctrlComm, TEXT("\x03"));
	//setOpenAnswerEd(false);
}

void CwgwDlg::putOutput(CMscomm1 *com, const CString &msg)
{
	//TCHAR *buffer = msg.GetBuffer();
	for(int i = 0; i < msg.GetLength(); i++){
		LONGLONG v = chrVal(msg[i]);
		com->put_Output(COleVariant(v));
	}
}

void CwgwDlg::sendComMsg(CMscomm1 *com, const CString &msg)
{
	CString id;
	((CEdit*)GetDlgItem(IDC_RFID))->GetWindowText(id);
	int rfid = _tcstol(id.GetBuffer(), NULL, 16);

	CString out = TEXT("\x7f");
	out.AppendChar((TCHAR)(rfid));
	out = out + msg;

	int len = out.GetLength();;
	//putOutput(com, out);

	//HIDWrite(threadData.hCom, out.GetBuffer(), len, threadData.isCH9326);
	m_pCommThread->addInput(out.GetBuffer(), len);
	return;

	if(threadData.hCom == INVALID_HANDLE_VALUE)
		com->put_Output(COleVariant(out));
	else
		HIDWrite(threadData.hCom, out.GetBuffer(), len, 0);
}

void CwgwDlg::sendComMsg(CMscomm1 *com, const TCHAR *msg)
{
	return sendComMsg(com, CString(msg));
}

void CwgwDlg::OnBnClickedCheckon()
{
	// TODO: 考勤关
	if(!comOpened()){
		return;
	}

	sendComMsg(&m_ctrlComm, TEXT("\x04"));
	//setCheckOn(true);
}


void CwgwDlg::OnBnClickedCheckoff()
{
	// TODO: 考勤开
	if(!comOpened()){
		return;
	}

	sendComMsg(&m_ctrlComm, TEXT("\x06"));
	//setCheckOn(false);
}


void CwgwDlg::OnBnClickedTiming()
{
	// TODO: 校时
	if(!comOpened()){
		//HIDSampleFunc();
		return;
	}

	SYSTEMTIME st;
	CString strDate;
	CString strTime;
	GetLocalTime(&st);

	//strDate.Format(TEXT("%4d-%2d-%2d"),st.wYear,st.wMonth,st.wDay);
	//int value = (st.wHour&0xff) + ((st.wMinute<<8) & 0xff00) + (((st.wHour+st.wMinute)<<16) & 0xff0000);
	//strTime.Format(TEXT("%x"), value);

	strTime.Insert(0, (TCHAR)(st.wHour&0xff));
	strTime.Insert(1, (TCHAR)(st.wMinute&0xff));
	strTime.Insert(2, (TCHAR)((st.wHour+st.wMinute)&0xff));

	CString m_Editsend = TEXT("\x07") + strTime;
	sendComMsg(&m_ctrlComm, m_Editsend);
}

void CwgwDlg::updateState()
{
	CString id;
	((CEdit*)GetDlgItem(IDC_RFID))->GetWindowText(id);
	WriteProfileString(TEXT("wgw"), TEXT("id"), id);
	//WritePrivateProfileString(TEXT("wgw"), TEXT("id"), id, TEXT(".\\info.ini"));

	if(!m_commOpened){
		SetWindowText(m_comStateStr);
		return;
	}

	CString tmp = m_comStateStr + (m_openAnswerEd ? TEXT(",答题已开"):TEXT(",答题未开启")) + (m_checkOn ? TEXT(",考勤已开"):TEXT(",考勤未开启"));
	SetWindowText(tmp);
}

void CwgwDlg::setOpenAnswerEd(bool v)
{
	if(v != m_openAnswerEd){
		m_openAnswerEd = v;
		updateState();
	}
}

void CwgwDlg::setCheckOn(bool v)
{
	if(v != m_checkOn){
		m_checkOn = v;
		updateState();
	}
}


void CwgwDlg::OnEnChangeRfid()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码

}


void CwgwDlg::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	int CurSel = m_tabctrl.GetCurSel();
	switch(CurSel){
		case 0:
			m_dlgCheck.ShowWindow(SW_SHOW);
			m_dlgAnswer.ShowWindow(SW_HIDE);
			m_dlgOther.ShowWindow(SW_HIDE);
			break;
		case 1:
			m_dlgCheck.ShowWindow(SW_HIDE);
			m_dlgAnswer.ShowWindow(SW_SHOW);
			m_dlgOther.ShowWindow(SW_HIDE);
			break;
		case 2:
			m_dlgCheck.ShowWindow(SW_HIDE);
			m_dlgAnswer.ShowWindow(SW_HIDE);
			m_dlgOther.ShowWindow(SW_SHOW);
		break;
		default:
			break;
	}

	//m_tabctrl.SetCurSel(CurSel);
	*pResult = 0;
}

LRESULT CwgwDlg::OnUserThreadend(WPARAM wParam, LPARAM lParam) 
{
	char *msg = (char *)wParam;
	unsigned len = lParam;

	getMsgOne(msg, len);
	free(msg);
	return 0;
} 

LRESULT CwgwDlg::OnUsernNewLink(WPARAM wParam, LPARAM lParam) 
{
	CComboBox * CCombox = (CComboBox *)GetDlgItem(IDC_COMLIST);

	CCombox->AddString((char*)wParam);
	free((char*)wParam);
	return 0;
} 

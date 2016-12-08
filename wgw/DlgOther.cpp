// DlgOther.cpp : 实现文件
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgOther.h"
#include "afxdialogex.h"


// CDlgOther 对话框

IMPLEMENT_DYNAMIC(CDlgOther, CDialogEx)

CDlgOther::CDlgOther(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgOther::IDD, pParent)
	, m_editOther(_T(""))
{
}

CDlgOther::~CDlgOther()
{
}

void CDlgOther::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Text(pDX, IDC_EDIT_DLG_OTHER, m_editOther);
}


BEGIN_MESSAGE_MAP(CDlgOther, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT1, &CDlgOther::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CDlgOther 消息处理程序


BOOL CDlgOther::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();

	return TRUE;
}

void CDlgOther::msgAppend(TCHAR *in, unsigned len)
{
	unsigned mask = chrMask();

	TCHAR buffer[2048];

	ASSERT(sizeof(buffer)/sizeof(buffer[0]) > len+1);
	unsigned i;
	for(i = 0; i < len; i++){
		_stprintf(buffer + 2*i, TEXT("%02x"), chrVal(in[i]));
	}
	
	m_editOther = CString(buffer) + CString(TEXT("\r\n")) + m_editOther;

	SetWindowText(m_editOther);
	//UpdateData(false);
}


void CDlgOther::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	GetWindowText(m_editOther);
}

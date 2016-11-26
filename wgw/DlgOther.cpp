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
	DDX_Text(pDX, IDC_EDIT_DLG_OTHER, m_editOther);
}


BEGIN_MESSAGE_MAP(CDlgOther, CDialogEx)
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

	UpdateData(false);
}

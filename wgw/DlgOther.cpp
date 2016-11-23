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
{

}

CDlgOther::~CDlgOther()
{
}

void CDlgOther::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgOther, CDialogEx)
END_MESSAGE_MAP()


// CDlgOther 消息处理程序


BOOL CDlgOther::OnInitDialog(void)
{
	return TRUE;
}

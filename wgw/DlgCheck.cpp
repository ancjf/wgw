// DlgCheck.cpp : 实现文件
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgCheck.h"
#include "afxdialogex.h"


// CDlgCheck 对话框

IMPLEMENT_DYNAMIC(CDlgCheck, CDialogEx)

CDlgCheck::CDlgCheck(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgCheck::IDD, pParent)
{

}

CDlgCheck::~CDlgCheck()
{
}

void CDlgCheck::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DLG_CHECK, m_listCheck);
}


BEGIN_MESSAGE_MAP(CDlgCheck, CDialogEx)
END_MESSAGE_MAP()


// CDlgCheck 消息处理程序


BOOL CDlgCheck::OnInitDialog(void)
{	
	CDialogEx::OnInitDialog();

	m_listCheck.InsertColumn(0,TEXT("序号"),LVCFMT_LEFT,60);              //添加列标题！！！！  这里的80,40,90用以设置列的宽度。！！！LVCFMT_LEFT用来设置对齐方式！！！
    m_listCheck.InsertColumn(1,TEXT("ID号"),LVCFMT_LEFT,100);
    m_listCheck.InsertColumn(2,TEXT("第一次收到时间"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(3,TEXT("最后一次收到时间"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(4,TEXT("答题卡电量"),LVCFMT_LEFT,100);
	return TRUE;
}

// DlgAnswer.cpp : 实现文件
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgAnswer.h"
#include "afxdialogex.h"


// CDlgAnswer 对话框

IMPLEMENT_DYNAMIC(CDlgAnswer, CDialogEx)

CDlgAnswer::CDlgAnswer(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgAnswer::IDD, pParent)
{

}

CDlgAnswer::~CDlgAnswer()
{
}

void CDlgAnswer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DLG_ANSWER, m_listAnswer);
}


BEGIN_MESSAGE_MAP(CDlgAnswer, CDialogEx)
END_MESSAGE_MAP()


// CDlgAnswer 消息处理程序


BOOL CDlgAnswer::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();

	m_listAnswer.InsertColumn(0,TEXT("序号"),LVCFMT_LEFT,60);              //添加列标题！！！！  这里的80,40,90用以设置列的宽度。！！！LVCFMT_LEFT用来设置对齐方式！！！
    m_listAnswer.InsertColumn(1,TEXT("ID号"),LVCFMT_LEFT,100);
    m_listAnswer.InsertColumn(2,TEXT("答题信息"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(3,TEXT("答题时间"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(4,TEXT("答题卡电量"),LVCFMT_LEFT,100);

	return TRUE;
}

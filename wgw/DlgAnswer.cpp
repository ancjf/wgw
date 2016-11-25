// DlgAnswer.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgAnswer.h"
#include "afxdialogex.h"


// CDlgAnswer �Ի���

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


// CDlgAnswer ��Ϣ�������


BOOL CDlgAnswer::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();

	m_listAnswer.InsertColumn(0,TEXT("���"),LVCFMT_LEFT,60);              //����б��⣡������  �����80,40,90���������еĿ�ȡ�������LVCFMT_LEFT�������ö��뷽ʽ������
    m_listAnswer.InsertColumn(1,TEXT("ID��"),LVCFMT_LEFT,100);
    m_listAnswer.InsertColumn(2,TEXT("������Ϣ"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(3,TEXT("����ʱ��"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(4,TEXT("���⿨����"),LVCFMT_LEFT,100);

	return TRUE;
}

// DlgCheck.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgCheck.h"
#include "afxdialogex.h"


// CDlgCheck �Ի���

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


// CDlgCheck ��Ϣ�������


BOOL CDlgCheck::OnInitDialog(void)
{	
	CDialogEx::OnInitDialog();

	m_listCheck.InsertColumn(0,TEXT("���"),LVCFMT_LEFT,60);              //����б��⣡������  �����80,40,90���������еĿ�ȡ�������LVCFMT_LEFT�������ö��뷽ʽ������
    m_listCheck.InsertColumn(1,TEXT("ID��"),LVCFMT_LEFT,100);
    m_listCheck.InsertColumn(2,TEXT("��һ���յ�ʱ��"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(3,TEXT("���һ���յ�ʱ��"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(4,TEXT("���⿨����"),LVCFMT_LEFT,100);
	return TRUE;
}

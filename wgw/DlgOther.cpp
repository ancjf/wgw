// DlgOther.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgOther.h"
#include "afxdialogex.h"


// CDlgOther �Ի���

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


// CDlgOther ��Ϣ�������


BOOL CDlgOther::OnInitDialog(void)
{
	return TRUE;
}

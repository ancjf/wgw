#pragma once


// CDlgCheck �Ի���

class CDlgCheck : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCheck)

public:
	CDlgCheck(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgCheck();

// �Ի�������
	enum { IDD = IDD_DLG_CHECK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listCheck;
};

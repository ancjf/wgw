#pragma once


// CDlgAnswer �Ի���

class CDlgAnswer : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAnswer)

public:
	CDlgAnswer(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgAnswer();

// �Ի�������
	enum { IDD = IDD_DLG_ANSWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
	virtual BOOL OnInitDialog(void);

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listAnswer;
	
};

#pragma once


// CDlgOther �Ի���

class CDlgOther : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgOther)

public:
	CDlgOther(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CDlgOther();

// �Ի�������
	enum { IDD = IDD_DLG_OTHER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
public:
	BOOL OnInitDialog(void);
};

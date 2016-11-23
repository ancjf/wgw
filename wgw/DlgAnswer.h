#pragma once


// CDlgAnswer 对话框

class CDlgAnswer : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAnswer)

public:
	CDlgAnswer(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgAnswer();

// 对话框数据
	enum { IDD = IDD_DLG_ANSWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog(void);

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_listAnswer;
	
};

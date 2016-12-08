#pragma once

#include <map>
#include <vector>

using namespace std;
// CDlgOther 对话框

class CDlgOther : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgOther)

public:
	CDlgOther(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgOther();

// 对话框数据
	enum { IDD = IDD_DLG_OTHER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
private:
	BOOL OnInitDialog(void);
	CString m_editOther;
public:
	void CDlgOther::msgAppend(TCHAR *in, unsigned len);
	afx_msg void OnEnChangeEdit1();
};

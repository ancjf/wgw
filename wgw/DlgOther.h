#pragma once

#include <map>
#include <vector>

using namespace std;
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
private:
	BOOL OnInitDialog(void);
	CString m_editOther;
public:
	void CDlgOther::msgAppend(TCHAR *in, unsigned len);
	afx_msg void OnEnChangeEdit1();
};

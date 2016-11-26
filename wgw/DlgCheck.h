#pragma once

#include <map>
#include <vector>

using namespace std;
// CDlgCheck 对话框

#include "common.h"

class CDlgCheck : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCheck)

public:
	CDlgCheck(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgCheck();

// 对话框数据
	enum { IDD = IDD_DLG_CHECK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	int insertCheck(struct checkItem *item);
	int updateCheck(const struct checkItem *item);
	int processMsgCheck(TCHAR *in);
private:
	CListCtrl m_listCheck;
	map<CString, struct checkItem> m_checkMap;
};

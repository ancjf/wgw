#pragma once

#include <map>
#include <vector>

using namespace std;

#include "common.h"
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
private:
	vector<struct answerItem> m_answerVec;
	CListCtrl m_listAnswer;

public:
	int insertAnswer(struct answerItem &item);
	int updateAnswer(struct answerItem &item, int nindex);
	int updateAnswer();
	int processMsgAnswer(TCHAR *in);
	
};


// wgwDlg.h : 头文件
//

#pragma once
#include "mscomm1.h"
#include "stdint.h"
#include <map>
#include <vector>

using namespace std;

#define MSG_BEGIN TEXT('\xf9')
#define MSG_TRANSFERRED TEXT('\xf8')

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

struct checkItem
{
	TCHAR id[16];
	uint64_t firstTime;
	uint64_t lastTime;
	int electricity;
	int nindex;
	checkItem(){
		nindex = 0;
		firstTime = 0;
		lastTime = 0;
		electricity = 0;
		memset(id, 0, sizeof(id));
	}
};

struct answerItem
{
	TCHAR id[16];
	TCHAR answer[16];
	uint64_t time;
	int electricity;
	int nindex;
	answerItem(){
		nindex = 0;
		time = 0;
		electricity = 0;
		memset(id, 0, sizeof(id));
		memset(answer, 0, sizeof(answer));
	}
};
// CwgwDlg 对话框
class CwgwDlg : public CDialogEx
{
// 构造
public:
	CwgwDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_WGW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void sendComMsg(CMscomm1 *com, const TCHAR *msg);
	void sendComMsg(CMscomm1 *com, const CString &msg);
	void updateState();

	void setOpenAnswerEd(bool v);
	void setCheckOn(bool v);

	void GetComLis(CComboBox * CCombox);

	TCHAR *getMsgStart(TCHAR *in, unsigned len);
	void msgAppend(CString &str, TCHAR *in, unsigned len);
	int getMsgOne(TCHAR *in, unsigned len, CString &str);
	unsigned getMsg(TCHAR *in, unsigned len, CString &str);

	void putOutput(CMscomm1 *com, const CString &msg);
	int processMsgCheck(TCHAR *in);
	int updateCheck(const struct checkItem &item);
	int insertCheck(struct checkItem &item);

	int processMsgAnswer(TCHAR *in);
	int insertAnswer(struct answerItem &item);

	int updateAnswer(struct answerItem &item, int nindex);
	int updateAnswer();
public:
	afx_msg void OnBnClickedOpenanswer();
	DECLARE_EVENTSINK_MAP()
	void OnCommMscomm1();

private:
	CMscomm1 m_ctrlComm;
	bool m_commOpened;
public:
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedLink();
	afx_msg void OnBnClickedOpen();

	bool comOpened();
	void addInString(TCHAR *str, long size);

	afx_msg void OnBnClickedCloseanwer();

private:
	CString m_inString;
	CString m_comStateStr;
	bool m_openAnswerEd;
	bool m_checkOn;

	TCHAR m_buffer[1024];
	unsigned m_dataLen;

	map<CString, struct checkItem> m_checkMap;
	vector<struct answerItem> m_answerVec;
public:
	afx_msg void OnBnClickedCheckon();
	afx_msg void OnBnClickedCheckoff();
	afx_msg void OnBnClickedTiming();
	CString m_editOther;
	afx_msg void OnEnChangeRfid();
	CListCtrl m_listCheck;
	CListCtrl m_listAnswer;
};

// DlgAnswer.cpp : 实现文件
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgAnswer.h"
#include "afxdialogex.h"


// CDlgAnswer 对话框

IMPLEMENT_DYNAMIC(CDlgAnswer, CDialogEx)

CDlgAnswer::CDlgAnswer(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgAnswer::IDD, pParent)
{

}

CDlgAnswer::~CDlgAnswer()
{
}

void CDlgAnswer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DLG_ANSWER, m_listAnswer);
}


BEGIN_MESSAGE_MAP(CDlgAnswer, CDialogEx)
END_MESSAGE_MAP()


// CDlgAnswer 消息处理程序


BOOL CDlgAnswer::OnInitDialog(void)
{
	CDialogEx::OnInitDialog();

	m_listAnswer.InsertColumn(0,TEXT("序号"),LVCFMT_LEFT,60);              //添加列标题！！！！  这里的80,40,90用以设置列的宽度。！！！LVCFMT_LEFT用来设置对齐方式！！！
    m_listAnswer.InsertColumn(1,TEXT("ID号"),LVCFMT_LEFT,100);
    m_listAnswer.InsertColumn(2,TEXT("答题信息"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(3,TEXT("答题时间"),LVCFMT_LEFT,140);
    m_listAnswer.InsertColumn(4,TEXT("答题卡电量"),LVCFMT_LEFT,100);

	return TRUE;
}

int CDlgAnswer::insertAnswer(struct answerItem &item)
{
	LV_ITEM    lvitemAdd = {0};
	lvitemAdd.iItem = m_listAnswer.GetItemCount();
	if (m_listAnswer.InsertItem(&lvitemAdd) == -1)
		return -1;

	m_listAnswer.SetItemData(lvitemAdd.iItem, lvitemAdd.iItem);
	item.nindex = lvitemAdd.iItem;

	CString str;
	str.Format(TEXT("%d"), item.nindex);

	m_listAnswer.SetItemText(item.nindex, 0, str);
	m_listAnswer.SetItemText(item.nindex, 1, item.id);
	m_listAnswer.SetItemText(item.nindex, 2, item.answer);

	CTime tm = CTime(0) + CTimeSpan(item.time);
	str = tm.Format(TEXT("%Y-%m-%d:%X"));
	m_listAnswer.SetItemText(item.nindex, 3, str);

	str.Format(TEXT("%d"), item.electricity);
	m_listAnswer.SetItemText(item.nindex, 4, str);

	//m_listAnswer.SortItems(MyCompareProc,(DWORD_PTR)&m_listAnswer);
	m_answerVec.insert(m_answerVec.begin(), item);
	updateAnswer();
	return 0;
}

int CDlgAnswer::updateAnswer(struct answerItem &item, int nindex)
{
	CString str;
	str.Format(TEXT("%d"), item.nindex);

	m_listAnswer.SetItemText(nindex, 0, str);
	m_listAnswer.SetItemText(nindex, 1, item.id);
	m_listAnswer.SetItemText(nindex, 2, item.answer);

	CTime tm = CTime(0) + CTimeSpan(item.time);
	str = tm.Format(TEXT("%Y-%m-%d:%X"));
	m_listAnswer.SetItemText(nindex, 3, str);

	str.Format(TEXT("%d"), item.electricity);
	m_listAnswer.SetItemText(nindex, 4, str);

	return 0;
}

int CDlgAnswer::updateAnswer()
{
	int i = 0;
	vector<struct answerItem>::iterator it;
	for(it=m_answerVec.begin(); it!=m_answerVec.end();it++){
		updateAnswer(*it, i++);
	}

	return 0;
}

int CDlgAnswer::processMsgAnswer(TCHAR *in)
{
	struct answerItem item;
	in += 3;
	for(int i = 0; i<10; i++){
		item.id[i] = TEXT('0') + in[i];
		if(!_istdigit(item.id[i]))
			item.id[i] = TEXT('F');
	}

	item.electricity = in[10];

	if(in[11] == TEXT('\x1')){
		_tcscpy(item.answer, TEXT("yes"));
	}else if(in[11] == TEXT('\x2')){
		_tcscpy(item.answer, TEXT("no"));
	}else{
		for(int i = 0; i<4; i++){
			TCHAR chr = (((in[11+i]&0xf0)>>4)-10)+TEXT('A');
			if(!_istalpha(chr)){
				break;
			}
			item.answer[2*i] = chr;
			chr = ((in[11+i]&0xf)-10)+TEXT('A');
			if(!_istalpha(chr))
				break;
			item.answer[2*i+1] = chr;
		}
	}

	CTimeSpan span = CTime::GetCurrentTime() - CTime(0);
	item.time = span.GetTotalSeconds();
	return insertAnswer(item);
}


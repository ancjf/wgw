// DlgCheck.cpp : 实现文件
//

#include "stdafx.h"
#include "wgw.h"
#include "DlgCheck.h"
#include "afxdialogex.h"


// CDlgCheck 对话框

IMPLEMENT_DYNAMIC(CDlgCheck, CDialogEx)

CDlgCheck::CDlgCheck(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgCheck::IDD, pParent)
{

}

CDlgCheck::~CDlgCheck()
{
}

void CDlgCheck::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DLG_CHECK, m_listCheck);
}


BEGIN_MESSAGE_MAP(CDlgCheck, CDialogEx)
END_MESSAGE_MAP()


// CDlgCheck 消息处理程序


BOOL CDlgCheck::OnInitDialog(void)
{	
	CDialogEx::OnInitDialog();

	m_listCheck.InsertColumn(0,TEXT("序号"),LVCFMT_LEFT,60);              //添加列标题！！！！  这里的80,40,90用以设置列的宽度。！！！LVCFMT_LEFT用来设置对齐方式！！！
    m_listCheck.InsertColumn(1,TEXT("ID号"),LVCFMT_LEFT,100);
    m_listCheck.InsertColumn(2,TEXT("第一次收到时间"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(3,TEXT("最后一次收到时间"),LVCFMT_LEFT,140);
    m_listCheck.InsertColumn(4,TEXT("答题卡电量"),LVCFMT_LEFT,100);
	return TRUE;
}

int CDlgCheck::updateCheck(const struct checkItem *item)
{
	CTime tm = CTime(0) + CTimeSpan(item->firstTime);
	CString str = tm.Format(TEXT("%Y-%m-%d:%X"));
	m_listCheck.SetItemText(item->nindex, 2, str);

	tm = CTime(0) + CTimeSpan(item->lastTime);
	str = tm.Format(TEXT("%Y-%m-%d:%X"));
	m_listCheck.SetItemText(item->nindex, 3, str);

	str.Format(TEXT("%d"), item->electricity);
	m_listCheck.SetItemText(item->nindex, 4, str);
	
	return 0;
}

int CDlgCheck::insertCheck(struct checkItem *item)
{
	LV_ITEM    lvitemAdd = {0};
	lvitemAdd.iItem = m_listCheck.GetItemCount();
	if (m_listCheck.InsertItem(&lvitemAdd) == -1)
		return -1;

	item->nindex = lvitemAdd.iItem;

	CString str;
	str.Format(TEXT("%d"), item->nindex);

	m_listCheck.SetItemText(item->nindex, 0, str);
	m_listCheck.SetItemText(item->nindex, 1, item->id);
	m_checkMap[CString(item->id)] = *item;
	return updateCheck(item);
}

int CDlgCheck::processMsgCheck(TCHAR *in)
{
	struct checkItem item;
	in += 3;
	for(int i = 0; i<10; i++){
		item.id[i] = TEXT('0') + in[i];
		if(!_istdigit(item.id[i]))
			item.id[i] = TEXT('F');
	}

	item.id[10] = 0;
	item.electricity = in[10];

	map<CString, struct checkItem>::iterator iter;
	iter = m_checkMap.find(CString(item.id));
	if(iter != m_checkMap.end()){
		struct checkItem &c = iter->second;
		CTimeSpan span = CTime::GetCurrentTime() - CTime(0);
		if(!c.firstTime){
			c.firstTime = span.GetTotalSeconds();
		}else{
			c.lastTime = span.GetTotalSeconds();
		}

		return updateCheck(&c);
	}

	return insertCheck(&item);
}

#include "stdafx.h"

#include <LogList.h>


// CLogList

IMPLEMENT_DYNAMIC(CLogList, CListCtrl)

CLogList::CLogList()
{

}

CLogList::~CLogList()
{
}


BEGIN_MESSAGE_MAP(CLogList, CListCtrl)
END_MESSAGE_MAP()


void CLogList::InitCtrl(const vector<pair<CString, LONG>>& vctColumns)
{
	CListCtrl::SetView(LVS_REPORT);

	CListCtrl::ModifyStyle(LVS_EDITLABELS| LVS_NOSCROLL| LVS_NOCOLUMNHEADER
		, LVS_REPORT| LVS_ALIGNTOP| LVS_SINGLESEL| LVS_SHOWSELALWAYS);
	CListCtrl::SetExtendedStyle(LVS_EX_GRIDLINES| LVS_EX_FULLROWSELECT);

	UINT nColumn = 0;
	for (vector<pair<CString, LONG>>::const_iterator itColumn=vctColumns.begin()
		; itColumn!=vctColumns.end(); ++itColumn, ++nColumn)
	{
		(void)CListCtrl::InsertColumn(nColumn, itColumn->first, LVCFMT_LEFT, itColumn->second);
	}
}

void CLogList::AddLog(const vector<CString>& vctLogTexts)
{
	int nItem = CListCtrl::GetItemCount();

	(void)CListCtrl::InsertItem(nItem,NULL);

	UINT nSubItem = 0;
	for (vector<CString>::const_iterator itLogText=vctLogTexts.begin()
		; itLogText!=vctLogTexts.end(); ++itLogText, ++nSubItem)
	{
		(void)CListCtrl::SetItemText(nItem, nSubItem, *itLogText);
	}

	CListCtrl::EnsureVisible(nItem, TRUE);
}

void CLogList::ClearLogs()
{
	(void)DeleteAllItems();
}

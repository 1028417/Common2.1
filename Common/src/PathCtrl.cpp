
#include "StdAfx.h"

#include <PathCtrl.h>

//CPathList

void CPathList::PreSubclassWindow()
{
	__super::PreSubclassWindow();

	(void)ModifyStyle(LVS_ALIGNLEFT, LVS_AUTOARRANGE);
}

BOOL CPathList::InitCtrl(COLORREF crText, UINT uFontSize, const TD_ListColumn& lstColumns)
{
	__EnsureReturn(__super::InitFont(crText, uFontSize), FALSE);
	return __super::InitColumn(lstColumns.empty() ? m_lstColumns : lstColumns);
}

BOOL CPathList::InitCtrl(COLORREF crText, UINT uFontSize, const CSize& szImglst, const CSize *pszSmallImglst, const TD_IconVec& vecIcons)
{
	__EnsureReturn(this->InitCtrl(crText, uFontSize, m_lstColumns), FALSE);

	__EnsureReturn(__super::InitImglst(szImglst, pszSmallImglst, vecIcons), FALSE);

	return TRUE;
}

BOOL CPathList::InitCtrlEx(COLORREF crText, UINT uFontSize)
{
	__EnsureReturn(this->InitCtrl(crText, uFontSize, m_lstColumns), FALSE);

	HBITMAP hBitmap = g_ResModule.loadBitmap(IDB_PATHCTRL_NORMAL);
	__AssertReturn(hBitmap, FALSE);

	CBitmap Bitmap;
	Bitmap.Attach(hBitmap);

	__EnsureReturn(__super::InitImglst(Bitmap, &Bitmap), FALSE);

	(void)Bitmap.DeleteObject();

	return TRUE;
}

void CPathList::SetPath(CPathObject* pPath)
{
	if (pPath)
	{
		TD_PathList lstSubPaths;
		pPath->GetSubPath(lstSubPaths);

		this->SetRedraw(FALSE);
		DeleteAllItems();

		TD_ListObjectList lstObjects(lstSubPaths);
		__super::SetObjects(lstObjects);

		this->SetRedraw(TRUE);
	}
	else
	{
		DeleteAllItems();
	}
}

BOOL CPathList::IsFileItem(int nItem)
{
	__EnsureReturn(0 <= nItem, FALSE);

	CPathObject *pPath = (CPathObject*)GetItemObject(nItem);
	__EnsureReturn(pPath, FALSE);
		
	return !pPath->m_bDir;
}

void CPathList::GetAllPathObjects(TD_PathObjectList& lstPathObjects)
{
	TD_ListObjectList lstListObjects;
	__super::GetAllObjects(lstListObjects);

	lstPathObjects.add(lstListObjects);
}

void CPathList::GetAllPathObjects(TD_PathObjectList& lstPathObjects, bool bDir)
{
	TD_PathObjectList lstAllObjects;
	GetAllPathObjects(lstAllObjects);

	for (auto pPathObject : lstAllObjects)
	{
		if (pPathObject->m_bDir == bDir)
		{
			lstPathObjects.add(pPathObject);
		}
	}
}

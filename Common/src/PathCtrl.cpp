
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

	if (!lstColumns.empty())
	{
		__super::InitColumn(lstColumns);
	}

	return true;
}

BOOL CPathList::InitCtrl(COLORREF crText, UINT uFontSize, const CSize& szImglst, const CSize *pszSmallImglst, const TD_IconVec& vecIcons)
{
	__EnsureReturn(this->InitCtrl(crText, uFontSize), FALSE);

	__EnsureReturn(__super::InitImglst(szImglst, pszSmallImglst, vecIcons), FALSE);

	return TRUE;
}

bool CPathList::SetPath(CPathObject* pPath)
{
	TD_PathList lstSubPaths;
	if (NULL == pPath)
	{
		DeleteAllItems();
		return false;
	}
	
	pPath->GetSubPath(lstSubPaths);

	TD_ListObjectList lstObjects(lstSubPaths);

	CRedrawLockGuard RedrawLockGuard(*this);
	
	DeleteAllItems();
	
	__super::SetObjects(lstObjects);
	
	return true;
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

	lstAllObjects([&](CPathObject& PathObject) {
		if (PathObject.m_bDir == bDir)
		{
			lstPathObjects.add(PathObject);
		}
	});
}

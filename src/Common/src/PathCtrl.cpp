
#include "StdAfx.h"

#include "Common/PathCtrl.h"

//CPathList

void CPathList::PreSubclassWindow()
{
	__super::PreSubclassWindow();

	(void)ModifyStyle(LVS_ALIGNLEFT, LVS_AUTOARRANGE);
}

BOOL CPathList::InitCtrl(COLORREF crText, float fFontSize, const TD_ListColumn& lstColumns)
{
	__EnsureReturn(__super::InitFont(crText, fFontSize), FALSE);

	if (!lstColumns.empty())
	{
		__super::InitColumn(lstColumns);
	}

	return true;
}

BOOL CPathList::InitCtrl(COLORREF crText, float fFontSize, const CSize& szImglst, const CSize *pszSmallImglst, const TD_IconVec& vecIcons)
{
	__EnsureReturn(this->InitCtrl(crText, fFontSize), FALSE);

	__EnsureReturn(__super::InitImglst(szImglst, pszSmallImglst, vecIcons), FALSE);

	return TRUE;
}

void CPathList::SetPath(CPathObject& dir)
{
	TD_PathObjectList paPathObject(dir.dirs());
	paPathObject.add(dir.files());
	__super::SetObjects(TD_ListObjectList(paPathObject));
}

BOOL CPathList::IsFileItem(int nItem)
{
	__EnsureReturn(0 <= nItem, FALSE);

	CPathObject *pPath = (CPathObject*)GetItemObject(nItem);
	__EnsureReturn(pPath, FALSE);
		
	return !pPath->fileInfo().bDir;
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
		if (PathObject.fileInfo().bDir == bDir)
		{
			lstPathObjects.add(PathObject);
		}
	});
}

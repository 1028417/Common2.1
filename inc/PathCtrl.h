
#pragma once

#include <TreeCtrl.h>
#include <ListCtrl.h>

template <typename T>
class CDirTreeT : public T
{
	struct tagDirSortor
	{
		bool operator () (CPath *pPath1, CPath *pPath2)
		{
			return 0 >= StrCmpA(pPath1->m_strName.c_str(), pPath2->m_strName.c_str());
		}
	};
	
public:
	CDirTreeT()
		: m_pRootDir(NULL)
	{
	}

	virtual	~CDirTreeT(void)
	{
	}

private:
	CDirObject *m_pRootDir;
	 
public:
	void SetRootDir(CDirObject *pRootDir, BOOL bShowRoot)
	{
		(void)DeleteAllItems();

		m_pRootDir = pRootDir;
	
		if (bShowRoot)
		{
			(void)InsertObject(*m_pRootDir);
			InsertChilds(m_pRootDir);

			(void)__super::SelectItem(getTreeItem(m_pRootDir));
			(void)ExpandObject(*m_pRootDir);
		}
		else
		{
			InsertChilds(m_pRootDir);
			InsertChildsEx(m_pRootDir);
		}
	}

	void InsertChilds(CDirObject *pDirObject)
	{
		TD_PathList lstSubDirs;
		pDirObject->GetSubPath(lstSubDirs);
	
		for (auto pSubDir : lstSubDirs)
		{
			(void)InsertObject(*(CDirObject*)pSubDir, pDirObject);
		}
	}
	
	virtual BOOL handleNMNotify(NMHDR& NMHDR) override
	{
		if (TVN_ITEMEXPANDING == NMHDR.code)
		{
			LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(&NMHDR);
	
			if (TVE_EXPAND == pNMTreeView->action)
			{
				CWaitCursor WaitCursor;

				CDirObject* pDirObject = (CDirObject*)__super::GetItemObject(pNMTreeView->itemNew.hItem);
				__AssertReturn(pDirObject, FALSE);
				InsertChildsEx(pDirObject);

				//return TRUE;
			}
		}

		return FALSE;
	}

private:
	void InsertChildsEx(CDirObject *pDirObject)
	{
		TD_PathList lstSubDirs;
		pDirObject->GetSubPath(lstSubDirs);

		for (auto pSubDir : lstSubDirs)
		{
			InsertChilds((CDirObject*)pSubDir);
		}
	}
};

using CDirTree = CDirTreeT<CObjectTree>;
using CDirCheckTree = CDirTreeT<CObjectCheckTree>;


class __CommonExt CPathList: public CObjectList
{
public:
	CPathList(){}

private:
	TD_ListColumn m_lstColumns = {
		{_T("名称"), 400}
		, {_T("大小"), 80}
		, {_T("修改日期"), 150}
	};
	
private:
	virtual void PreSubclassWindow() override;

public:
	BOOL InitCtrl(COLORREF crText, UINT uFontSize, const TD_ListColumn& lstColumns = TD_ListColumn());

	BOOL InitCtrl(COLORREF crText, UINT uFontSize, const CSize& szImglst, const CSize *pszSmallImglst = NULL, const TD_IconVec& vecIcons = {});

	BOOL InitCtrlEx(COLORREF crText, UINT uFontSize = 0);

	bool SetPath(CPathObject* pPath);

	BOOL IsFileItem(int nItem);
	
	void GetAllPathObjects(TD_PathObjectList& lstPathObjects);

	void GetAllPathObjects(TD_PathObjectList& lstPathObjects, bool bDir);
};

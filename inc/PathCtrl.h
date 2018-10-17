
#pragma once

#include <fsutil.h>

#include <TreeCtrl.h>
#include <ListCtrl.h>

class CPathObject;
typedef ptrlist<CPathObject*> TD_PathObjectList;

class __CommonPrjExt CPathObject: public CPath, public CListObject
{
public:
	CPathObject()
	{
	}

	CPathObject(const wstring& strDir)
		: CPath(strDir)
	{
	}

	CPathObject(const wstring& strName, const TD_PathObjectList& lstSubPathObjects)
		: CPath(strName, TD_PathList(lstSubPathObjects))
	{
	}

	CPathObject(CFileFind &FindFile, CPath *pParentPath)
		: CPath(FindFile, pParentPath)
	{
	}

	virtual ~CPathObject()
	{
	}

protected:
	virtual CPath *NewSubPath(CFileFind &FindFile, CPath *pParentPath)
	{
		return new CPathObject(FindFile, pParentPath);
	}

public:
	CString GetFileModifyTime()
	{
		if (m_bDir)
		{
			return L"";
		}

		return CTime(m_modifyTime).Format(_T("%y-%m-%d %H:%M"));
	}

	void GenListItem(CObjectList& wndList, vector<wstring>& vecText, int& iImage) override
	{
		vecText.push_back(m_strName);
		
		vecText.push_back(to_wstring(m_uFileSize));
		
		vecText.push_back((LPCTSTR)GetFileModifyTime());
	}
};


class CDirObject;

typedef ptrlist<CDirObject*> TD_DirObjectList;

class CDirObject : public CPathObject, public CTreeObject
{
public:
	CDirObject(const wstring& strDir=L"")
		: CPathObject(strDir)
	{
	}
	
	CDirObject(const wstring& strName, const TD_DirObjectList& lstSubDirObjects)
		: CPathObject(strName, TD_PathList(lstSubDirObjects))
	{
	}

	CDirObject(CFileFind &FindFile, CPath *pParentPath)
		: CPathObject(FindFile, pParentPath)
	{
	}

	virtual ~CDirObject()
	{
	}

protected:
	virtual CPath *NewSubPath(CFileFind &FindFile, CPath *pParentPath)
	{
		__EnsureReturn(FindFile.IsDirectory(), NULL);

		return new CDirObject(FindFile, pParentPath);
	}

public:
	CString GetTreeText()
	{
		return m_strName.c_str();
	}

	void GetTreeChilds(TD_TreeObjectList& lstChilds)
	{
		TD_PathList lstSubPaths;
		this->GetSubPath(lstSubPaths);
		
		TD_DirObjectList lstDirObjects(lstSubPaths);
		lstChilds.Insert(lstDirObjects);
	}
};

template <typename T>
class CDirTree: public T
{
	struct tagDirSortor
	{
		bool operator () (CPath *pPath1, CPath *pPath2)
		{
			return 0 >= StrCmpA(pPath1->m_strName.c_str(), pPath2->m_strName.c_str());
		}
	};
	
public:
	CDirTree()
		: m_pRootDir(NULL)
	{
	}

	virtual	~CDirTree(void)
	{
	}

private:
	CDirObject *m_pRootDir;
	 
public:
	void SetRootDir(CDirObject *pRootDir, BOOL bShowRoot)
	{
		(void)DeleteAllItems();

		m_pRootDir = pRootDir;
		m_pRootDir->FindFile();
	
		if (bShowRoot)
		{
			(void)InsertObject(*m_pRootDir);
			InsertChilds(m_pRootDir);

			(void)__super::SelectItem(m_pRootDir->m_hTreeItem);
			(void)__super::Expand(m_pRootDir->m_hTreeItem, TVE_EXPAND);
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
	
		for (TD_PathList::iterator itSubDir = lstSubDirs.begin()
			; itSubDir != lstSubDirs.end(); ++itSubDir)
		{
			(void)InsertObject(*(CDirObject*)*itSubDir, pDirObject);
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

		for (TD_PathList::iterator itSubDir = lstSubDirs.begin()
			; itSubDir != lstSubDirs.end(); ++itSubDir)
		{
			InsertChilds((CDirObject*)*itSubDir);
		}
	}
};


class __CommonPrjExt CPathList: public CObjectList
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
	BOOL InitCtrl(COLORREF crText, UINT uFontSize, const TD_ListColumn &lstColumns = TD_ListColumn());

	BOOL InitCtrl(COLORREF crText, UINT uFontSize, const CSize& szImglst, const CSize *pszSmallImglst = NULL, const TD_IconVec& vecIcons = {});

	BOOL InitCtrlEx(COLORREF crText, UINT uFontSize = 0);

	void SetPath(CPathObject* pPath);

	BOOL IsFileItem(int nItem);
	
	void GetAllPathObjects(TD_PathObjectList& lstPathObjects);

	void GetAllPathObjects(TD_PathObjectList& lstPathObjects, bool bDir);
};

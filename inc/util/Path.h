
#pragma once

#include "fsutil.h"

//CPath
class CPath;
using TD_PathList = PtrArray<CPath>;

class CListObject;
using TD_ListObjectList = PtrArray<CListObject>;

class CTreeObject;
using TD_TreeObjectList = PtrArray<CTreeObject>;

class CPathObject;
using TD_PathObjectList = PtrArray<CPathObject>;

class CDirObject;
using TD_DirObjectList = PtrArray<CDirObject>;

class __UtilExt CPath
{
	friend struct tagPathSortor;

public:
	CPath()
	{
	}

	CPath(const wstring& strDir)
		: m_bDir(true)
		, m_strName(strDir)
	{
		util::rtrim(m_strName, __Slant);
		util::rtrim(m_strName, __BackSlant);
	}

	CPath(const wstring& strDir, const TD_PathList& lstSubPath)
		: m_bDir(true)
		, m_strName(strDir)
	{
		util::rtrim(m_strName, __Slant);
		util::rtrim(m_strName, __BackSlant);

		m_plstSubPath = new TD_PathList();
		m_plstSubPath->add(lstSubPath);
	}
	
	CPath(const tagFindData& findData, CPath *pParentPath)
		: m_bDir(findData.isDir())
		, m_strName(findData.getFileName())
		, m_uFileSize(findData.data.nFileSizeLow)
		, m_modifyTime(findData.data.ftLastWriteTime)
		, m_pParentPath(pParentPath)
	{
	}
	
	virtual ~CPath()
	{
		ClearSubPath();
	}

public:
	bool m_bDir = false;

	ULONGLONG m_uFileSize = 0;

	FILETIME m_modifyTime = { 0,0 };

	CPath *m_pParentPath = NULL;

protected:
	wstring m_strName;

	TD_PathList *m_plstSubPath = NULL;
	
	bool m_bExists = false;

protected:
	virtual CPath* NewSubPath(const tagFindData& findData, CPath *pParentPath) const
	{
		return new CPath(findData, pParentPath);
	}
	
private:
	TD_PathList& _findFile(const wstring& strFind = L"");

public:
	wstring GetName() const;

	void SetName(const wstring& strNewName);

	wstring GetPath() const;

	bool GetSubPath(TD_PathList& lstSubPath, const wstring& strFind=L"");
	
	bool GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile=NULL, const wstring& strFind = L"");
	
	CPath *GetSubPath(wstring strSubPath, bool bDir, const wstring& strFind = L"");
	
	void ClearSubPath();

	void RemoveSubPath(const TD_PathList& lstDeletePaths);

	UINT GetSubPathCount() const;

	bool HasFile() const;

	TD_PathList& CPath::assignSubPath(const SArray<tagFindData>& arrFindData);
};

class __UtilExt CListObject
{
public:
	virtual void GenListItem(bool bReport, vector<wstring>& vecText, int& iImage)
	{
	}

	virtual wstring GetRenameText() const
	{
		return L"";
	}

	virtual void OnListItemRename(const wstring& strNewName)
	{
	}
};

class __UtilExt CTreeObject
{
public:
	CTreeObject()
	{
		m_hTreeItem = NULL;
	}

public:
	void *m_hTreeItem;

	virtual wstring GetTreeText() const
	{
		return L"";
	}

	virtual int GetTreeImage()
	{
		return 0;
	}

	virtual void GetTreeChilds(TD_TreeObjectList& lstChilds)
	{
		//do nothing
	}
};

class __UtilExt CPathObject : public CPath, public CListObject
{
public:
	CPathObject()
	{
	}

	CPathObject(const wstring& strDir)
		: CPath(strDir)
	{
	}

	CPathObject(const wstring& strDir, const TD_PathObjectList& lstSubPathObjects)
		: CPath(strDir, TD_PathList(lstSubPathObjects))
	{
	}

	CPathObject(const tagFindData& findData, CPath *pParentPath)
		: CPath(findData, pParentPath)
	{
	}

	virtual ~CPathObject()
	{
	}

protected:
	virtual CPath *NewSubPath(const tagFindData& findData, CPath *pParentPath) const override
	{
		return new CPathObject(findData, pParentPath);
	}

public:
	void GenListItem(bool bReport, vector<wstring>& vecText, int& iImage) override
	{
		vecText.push_back(m_strName);

		vecText.push_back(to_wstring(m_uFileSize));

		//vecText.push_back((LPCTSTR)GetFileModifyTime());
	}

	//wstring GetFileModifyTime()
	//{
	//	if (m_bDir)
	//	{
	//		return L"";
	//	}

	//	return CTime(m_modifyTime).Format(_T("%y-%m-%d %H:%M"));
	//}
};

class __UtilExt CDirObject : public CPathObject, public CTreeObject
{
public:
	CDirObject(const wstring& strDir = L"")
		: CPathObject(strDir)
	{
	}

	CDirObject(const wstring& strName, const TD_DirObjectList& lstSubDirObjects)
		: CPathObject(strName, TD_PathObjectList(lstSubDirObjects))
	{
	}

	CDirObject(const tagFindData& findData, CPath *pParentPath)
		: CPathObject(findData, pParentPath)
	{
	}

	virtual ~CDirObject()
	{
	}

protected:
	virtual CPath *NewSubPath(const tagFindData& findData, CPath *pParentPath) const override
	{
		__EnsureReturn(findData.isDir(), NULL);

		return new CDirObject(findData, pParentPath);
	}

public:
	wstring GetTreeText() const override
	{
		return m_strName;
	}

	void GetTreeChilds(TD_TreeObjectList& lstChilds)
	{
		TD_PathList lstSubPaths;
		this->GetSubPath(lstSubPaths);

		TD_DirObjectList lstDirObjects(lstSubPaths);
		lstChilds.add(lstDirObjects);
	}
};

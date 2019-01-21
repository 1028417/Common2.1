
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

	CPath(const wstring& strName, bool bDir);

	CPath(const tagFindData& findData, CPath *pParentDir)
		: m_bDir(findData.isDir())
		, m_strName(findData.getFileName())
		, m_uFileSize(findData.data.nFileSizeLow)
		, m_modifyTime(findData.data.ftLastWriteTime)
		, m_pParentDir(pParentDir)
	{
	}
	
	virtual ~CPath()
	{
		Clear();
	}

public:
	bool m_bDir = false;

	ULONGLONG m_uFileSize = 0;

	FILETIME m_modifyTime = { 0,0 };

	CPath *m_pParentDir = NULL;

protected:
	wstring m_strName;

	TD_PathList *m_plstSubPath = NULL;
	
	bool m_bExists = false;

protected:
	virtual CPath* NewSubPath(const tagFindData& findData, CPath *pParentDir)
	{
		return new CPath(findData, pParentDir);
	}
	
private:
	TD_PathList& _findFile(const wstring& strFind = L"");

public:
	void SetDir(const wstring& strDir);

	void SetName(const wstring& strNewName)
	{
		m_strName = strNewName;
	}

	wstring GetName() const;

	wstring GetPath() const;

	bool GetSubPath(TD_PathList& lstSubPath, const wstring& strFind=L"");
	
	bool GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile=NULL, const wstring& strFind = L"");
	
	CPath *FindSubPath(wstring strSubPath, bool bDir);
	
	virtual void Clear();

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

	CPathObject(const wstring& strName, bool bDir)
		: CPath(strName, bDir)
	{
	}

	CPathObject(const tagFindData& findData, CPath *pParentDir)
		: CPath(findData, pParentDir)
	{
	}

	virtual ~CPathObject()
	{
	}

protected:
	virtual CPath *NewSubPath(const tagFindData& findData, CPath *pParentDir) override
	{
		return new CPathObject(findData, pParentDir);
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
		: CPathObject(strDir, true)
	{
	}

	CDirObject(const tagFindData& findData, CPath *pParentDir)
		: CPathObject(findData, pParentDir)
	{
	}

	virtual ~CDirObject()
	{
	}

protected:
	virtual CPath *NewSubPath(const tagFindData& findData, CPath *pParentDir) override
	{
		__EnsureReturn(findData.isDir(), NULL);

		return new CDirObject(findData, pParentDir);
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


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

class __UtilExt CPath : public tagFileInfo
{
	friend struct tagPathSortor;

public:
	CPath()
	{
	}

	CPath(const wstring& strName, bool bDir);

	CPath(const tagFileInfo& FileInfo, CPath& ParentDir)
		: tagFileInfo(FileInfo)
		, m_pParentDir(&ParentDir)
	{
	}
	
	virtual ~CPath()
	{
		Clear();
	}

private:
	bool m_bExists = false;

	TD_PathList *m_plstSubPath = NULL;

protected:	
	CPath *m_pParentDir = NULL;
	
protected:
	virtual bool onFindFile(TD_PathList& lstSubPath);

	virtual CPath* NewSubPath(const tagFileInfo& FileInfo, CPath& ParentDir)
	{
		return new CPath(FileInfo, ParentDir);
	}
	
private:
	TD_PathList& _findFile();

	void _GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile = NULL);

public:
	void SetDir(const wstring& strDir);

	void SetName(const wstring& strNewName)
	{
		m_strName = strNewName;
	}

	wstring GetName() const;

	inline bool IsDir() const
	{
		return m_bDir;
	}

	inline bool IsExists() const
	{
		return m_bExists;
	}

	wstring GetPath() const;

	wstring GetParentDir() const;
	
	UINT GetSubPathCount() const;
	CPath *GetSubPath(UINT uIdx) const;

	void GetSubPath(TD_PathList& lstSubPath)
	{
		lstSubPath.add(_findFile());
	}
	
	void GetSubPath(TD_PathList& lstSubDir, TD_PathList& lstSubFile)
	{
		_GetSubPath(&lstSubDir, &lstSubFile);
	}

	void GetSubDir(TD_PathList& lstSubDir)
	{
		_GetSubPath(&lstSubDir);
	}

	void GetSubFile(TD_PathList& lstSubFile)
	{
		_GetSubPath(NULL, &lstSubFile);
	}

	CPath *FindSubPath(wstring strSubPath, bool bDir);

	bool enumSubFile(const function<bool(CPath& dir, TD_PathList& lstSubFile)>& cb);

	virtual void Clear();

	void RemoveSubPath(set<CPath*> setDeletePaths);

	bool HasFile() const;
};

class __UtilExt CListObject
{
public:
	virtual void GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage)
	{
	}

	virtual bool GetRenameText(wstring& strRenameText) const
	{
		return true;
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

	CPathObject(const tagFileInfo& FileInfo, CPath& ParentDir)
		: CPath(FileInfo, ParentDir)
	{
	}

	virtual ~CPathObject()
	{
	}

protected:
	virtual CPath* NewSubPath(const tagFileInfo& FileInfo, CPath& ParentDir) override
	{
		return new CPathObject(FileInfo, ParentDir);
	}
};

class __UtilExt CDirObject : public CPathObject, public CTreeObject
{
public:
	CDirObject(const wstring& strDir = L"")
		: CPathObject(strDir, true)
	{
	}

	CDirObject(const tagFileInfo& FileInfo, CPath& ParentDir)
		: CPathObject(FileInfo, ParentDir)
	{
	}

	virtual ~CDirObject()
	{
	}

protected:
	virtual CPath* NewSubPath(const tagFileInfo& FileInfo, CPath& ParentDir) override
	{
		if (FileInfo.m_bDir)
		{
			return new CDirObject(FileInfo, ParentDir);
		}

		return NULL;
	}

public:
	wstring GetTreeText() const override
	{
		return CPath::GetName();
	}

	void GetTreeChilds(TD_TreeObjectList& lstChilds)
	{
		TD_PathList lstSubPaths;
		this->GetSubPath(lstSubPaths);

		TD_DirObjectList lstDirObjects(lstSubPaths);
		lstChilds.add(lstDirObjects);
	}
};

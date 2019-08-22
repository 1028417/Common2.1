
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
	CPath() {}

	CPath(const wstring& strName, bool bDir);

	CPath(const tagFileInfo& FileInfo)
        : m_fi(FileInfo)
	{
	}
	
	virtual ~CPath()
	{
		Clear();
	}

private:
    tagFileInfo m_fi;

	enum class E_FindFileStatus
	{
		FFS_None
		, FFS_Exists
		, FFS_NotExists
	};
	E_FindFileStatus m_eFindFileStatus = E_FindFileStatus::FFS_None;

	TD_PathList m_paSubDir;
	TD_PathList m_paSubFile;

protected:
    const tagFileInfo& m_FileInfo = m_fi;

private:
	template <typename CB>
	inline bool _findFile(const CB& cb)
	{
		if (!fsutil::findFile(this->GetPath(), cb))
		{
			m_eFindFileStatus = E_FindFileStatus::FFS_NotExists;
			return false;
		}

		m_eFindFileStatus = E_FindFileStatus::FFS_Exists;

		return true;
	}
			
	virtual CPath* NewSubPath(const tagFileInfo& FileInfo)
	{
		return new CPath(FileInfo);
	}

	void _sort(TD_PathList& paSubPath);

protected:
	void _findFile();

	virtual void _onFindFile(TD_PathList& paSubDir, TD_PathList& paSubFile);

	virtual int _sort(const CPath& lhs, const CPath& rhs) const;

	size_t count() const
	{
		return m_paSubDir.size() + m_paSubFile.size();
	}

public:
	const tagFileInfo& fileInfo() const
    {
        return m_FileInfo;
    }

    void SetDir(const wstring& strDir);

	void SetName(const wstring& strNewName)
	{
        m_fi.strName = strNewName;
	}

	wstring GetName() const;

	wstring GetPath() const;

    wstring oppPath() const;

    wstring parentPath() const;

	using CB_PathScan = function<bool(CPath& dir, TD_PathList& paSubFile)>;
	bool scan(const CB_PathScan& cb);

	const TD_PathList& dirs()
	{
		_findFile();
		return m_paSubDir;
	}

	const TD_PathList& files()
	{
		_findFile();
		return m_paSubFile;
	}

	void subPath(const function<void(CPath&)>& cb)
	{
		_findFile();

		m_paSubDir(cb);
		m_paSubFile(cb);
	}

	/*void CPath::subPath(UINT uIdx, const function<void(CPath&)>& cb)
	{
		_findFile();

		if (uIdx < m_paSubDir.size())
		{
			m_paSubDir.get(uIdx, cb);
		}
		else
		{
			m_paSubFile.get(uIdx - m_paSubDir.size(), cb);
		}
	}*/

	CPath *FindSubPath(wstring strSubPath, bool bDir);

	void RemoveSelf();

	void Remove(CPath *pSubPath);

	virtual void Clear();
};

class __UtilExt CListObject
{
public:
    virtual void GenListItem(bool bReportView, vector<wstring>& vecText, int& iImage)
	{
        (void)bReportView;
        (void)vecText;
        (void)iImage;
    }

	virtual bool GetRenameText(wstring& strRenameText) const
	{
        (void)strRenameText;
		return true;
	}

	virtual void OnListItemRename(const wstring& strNewName)
	{
        (void)strNewName;
	}
};

class __UtilExt CTreeObject
{
public:
	CTreeObject() {}

public:
	virtual wstring GetTreeText() const	{ return L""; }

	virtual int GetTreeImage() { return 0; }

	virtual bool hasCheckState() { return true; }

	virtual void GetTreeChilds(TD_TreeObjectList&) {}
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

	CPathObject(const tagFileInfo& FileInfo)
		: CPath(FileInfo)
	{
	}

	virtual ~CPathObject()
	{
	}

protected:
	virtual CPath* NewSubPath(const tagFileInfo& FileInfo) override
	{
		return new CPathObject(FileInfo);
	}
};

class __UtilExt CDirObject : public CPathObject, public CTreeObject
{
public:
	CDirObject(const wstring& strDir = L"")
		: CPathObject(strDir, true)
	{
	}

	CDirObject(const tagFileInfo& FileInfo)
		: CPathObject(FileInfo)
	{
	}

	virtual ~CDirObject()
	{
	}

protected:
	virtual CPath* NewSubPath(const tagFileInfo& FileInfo) override
	{
		if (FileInfo.bDir)
		{
			return new CDirObject(FileInfo);
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
		lstChilds.add(TD_DirObjectList(this->dirs()));
	}
};

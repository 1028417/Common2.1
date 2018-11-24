
#pragma once

//CPath
class CPath;
typedef ptrlist<CPath*> TD_PathList;

class __CommonExt CPath
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
	}

	CPath(const wstring& strDir, const TD_PathList& lstSubPath)
		: m_bDir(true)
		, m_strName(strDir)
	{
		m_plstSubPath = new TD_PathList();
		m_plstSubPath->Insert(lstSubPath);
	}
	
	CPath(CFileFind& fileFind, CPath *pParentPath)
		: m_bDir(fileFind.IsDirectory())
		, m_strName(fileFind.GetFileName())
		, m_uFileSize(fileFind.GetLength())
		, m_pParentPath(pParentPath)
	{
		//(void)fileFind.GetCreationTime(&m_createTime);
		(void)fileFind.GetLastWriteTime(&m_modifyTime);
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

protected:
	virtual CPath* NewSubPath(CFileFind &FindFile, CPath *pParentPath)
	{
		return new CPath(FindFile, pParentPath);
	}

public:
	wstring GetName();

	void SetName(const wstring& strNewName);

	wstring GetPath();

	UINT GetSubPathCount();

	bool GetSubPath(TD_PathList& lstSubPath);
	
	bool GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile=NULL);
	
	CPath *GetSubPath(wstring strSubPath, bool bDir);
	
	void ClearSubPath();

	void RemoveSubPath(const TD_PathList& lstDeletePaths);

	bool FindFile();

	bool HasFile();
};

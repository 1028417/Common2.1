
#pragma once

#include "fsutil.h"

//CPath
class CPath;
typedef ptrlist<CPath> TD_PathList;

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
	}

	CPath(const wstring& strDir, const TD_PathList& lstSubPath)
		: m_bDir(true)
		, m_strName(strDir)
	{
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

protected:
	virtual CPath* NewSubPath(const tagFindData& findData, CPath *pParentPath)
	{
		return new CPath(findData, pParentPath);
	}
	
public:
	wstring GetName();

	void SetName(const wstring& strNewName);

	wstring GetPath() const;

	UINT GetSubPathCount();

	bool GetSubPath(TD_PathList& lstSubPath);
	
	bool GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile=NULL);
	
	CPath *GetSubPath(wstring strSubPath, bool bDir);
	
	void ClearSubPath();

	void RemoveSubPath(const TD_PathList& lstDeletePaths);

	bool FindFile();

	bool HasFile();
};

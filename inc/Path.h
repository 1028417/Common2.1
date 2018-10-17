
#pragma once

//CPath
class CPath;
typedef ptrlist<CPath*> TD_PathList;

class __CommonPrjExt CPath
{
public:
	CPath()
	{
	}

	CPath(const wstring& strDir)
		: m_bDir(true)
		, m_strName(strDir)
	{
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

	CPath(const wstring& strName, const TD_PathList & lstSubPath)
		: m_strName(strName)
		, m_bDir(TRUE)
	{
		m_plstSubPath = new TD_PathList();
		m_plstSubPath->Insert(lstSubPath);
	}

	virtual ~CPath()
	{
		ClearSubPath();
	}

public:
	bool m_bDir = false;

	wstring m_strName;

	ULONGLONG m_uFileSize = 0;

	FILETIME m_modifyTime = { 0,0 };

	CPath *m_pParentPath = NULL;

protected:
	TD_PathList *m_plstSubPath = NULL;

protected:
	virtual CPath *NewSubPath(CFileFind &FindFile, CPath *pParentPath) = 0; 
	//{
	//	return new CPath(FindFile, pParentPath);
	//}

public:
	UINT GetChildCount()
	{
		if (NULL == m_plstSubPath)
		{
			return 0;
		}

		return m_plstSubPath->size();
	}

	wstring GetPath();

	bool GetSubPath(TD_PathList& lstSubPath);
	
	bool GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile=NULL);
	
	CPath *GetSubPath(wstring strSubPath, bool bDir);
	
	void ClearSubPath();

	void RemoveSubPath(const TD_PathList& lstDeletePaths);

	bool FindFile();

	bool HasFile();
};

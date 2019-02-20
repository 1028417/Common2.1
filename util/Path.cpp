
#include <util.h>

#include <Path.h>

CPath::CPath(const wstring& strName, bool bDir)
{
	if (bDir)
	{
		SetDir(strName);
	}
	else
	{
		m_strName = strName;
	}
}

void CPath::SetDir(const wstring& strDir)
{
	Clear();

	m_bDir = true;
	m_strName = strDir;

	util::rtrim(m_strName, fsutil::slant);
	util::rtrim(m_strName, fsutil::backSlant);
}

wstring CPath::GetName() const
{
	if (NULL != m_pParentDir)
	{
		return m_strName;
	}
	else
	{
		return fsutil::GetFileName(m_strName);
	}
}

wstring CPath::GetPath() const
{
	if (NULL != m_pParentDir)
	{
		return m_pParentDir->GetPath() + fsutil::backSlant + m_strName;
	}

	return m_strName;
}

wstring CPath::GetParentDir() const
{
	if (NULL != m_pParentDir)
	{
		return m_pParentDir->GetPath();
	}

	return L"";
}

TD_PathList& CPath::assignSubPath(const SArray<tagFindData>& arrFindData)
{
	m_plstSubPath = new TD_PathList();

	arrFindData([&](auto& findData) {
		CPath *pSubPath = NewSubPath(findData, this);
		if (pSubPath)
		{
			m_plstSubPath->add(pSubPath);
		}
	});

	return *m_plstSubPath;
}

TD_PathList& CPath::_findFile(const wstring& strFind)
{
	if (NULL != m_plstSubPath)
	{
		return *m_plstSubPath;
	}

	m_plstSubPath = new TD_PathList();

	m_bExists = fsutil_win::FindFile(this->GetPath() + L"\\" + (strFind.empty() ? L"*" : strFind)
		, [&](const tagFindData& findData) {
		CPath *pSubPath = NewSubPath(findData, this);
		if (pSubPath)
		{
			m_plstSubPath->add(pSubPath);
		}
		return true;
	});
	
	m_plstSubPath->qsort([](const CPath& lhs, const CPath& rhs) {
		if (lhs.m_bDir && !rhs.m_bDir)
		{
			return true;
		}

		if (lhs.m_bDir == rhs.m_bDir)
		{
			return util::StrCompareUseCNCollate(lhs.m_strName, rhs.m_strName) < 0;
		}

		return false;
	});
	
	return *m_plstSubPath;
}

bool CPath::GetSubPath(TD_PathList& lstSubPath, const wstring& strFind)
{
	__EnsureReturn(m_bDir, false);
	
	lstSubPath.add(_findFile(strFind));

	return m_bExists;
}

bool CPath::GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile, const wstring& strFind)
{
	__EnsureReturn(m_bDir, false);

	TD_PathList& lstSubPath = _findFile(strFind);

	lstSubPath([&](CPath& SubPath) {
		if (SubPath.m_bDir)
		{
			if (plstSubDir)
			{
				plstSubDir->add(SubPath);
			}
		}
		else
		{
			if (plstSubFile)
			{
				plstSubFile->add(SubPath);
			}
		}
	});
	
	return m_bExists;;
}

CPath *CPath::FindSubPath(wstring strSubPath, bool bDir)
{
	__EnsureReturn(m_bDir, NULL);

	list<wstring> lstSubDirs;
	while (!strSubPath.empty())
	{
		wstring strName;
		wstring strExtName;
		fsutil::GetFileName(strSubPath, &strName, &strExtName);

		strName += strExtName;
		if (strName.empty())
		{
			break;
		}
		
		lstSubDirs.push_back(strName);

		strSubPath = fsutil::GetParentDir(strSubPath);
	}
	
	CPath *pPath = this;

	while (!lstSubDirs.empty() && NULL != pPath)
	{
		wstring strName = lstSubDirs.back();
		lstSubDirs.pop_back();

		TD_PathList& lstSubPath = pPath->_findFile();

		pPath = NULL;

		lstSubPath([&](CPath& SubPath) {
			if (lstSubDirs.empty())
			{
				if (SubPath.m_bDir != bDir)
				{
					return true;
				}
			}
			else
			{
				if (!SubPath.m_bDir)
				{
					return true;
				}
			}

			if (util::StrCompareIgnoreCase(SubPath.m_strName, strName))
			{
				pPath = &SubPath;
				return false;
			}

			return true;
		});
	}
	
	return pPath;
}

void CPath::Clear()
{
	if (NULL != m_plstSubPath)
	{
		(*m_plstSubPath)([](CPath& SubPath) {
			delete &SubPath;
		});

		delete m_plstSubPath;
		m_plstSubPath = NULL;
	}
}

void CPath::RemoveSubPath(const TD_PathList& lstDeletePaths)
{
	__Ensure(m_plstSubPath);

	m_plstSubPath->del_ex([&](CPath& SubPath) {
		if (lstDeletePaths.includes(&SubPath))
		{
			delete &SubPath;
			return true;
		}

		return false;
	});
}

UINT CPath::GetSubPathCount() const
{
	if (NULL == m_plstSubPath)
	{
		return 0;
	}

	return m_plstSubPath->size();
}

bool CPath::HasFile() const
{
	__EnsureReturn(m_plstSubPath, FALSE);
	return m_plstSubPath->any([](CPath& SubPath) {
		return !SubPath.m_bDir;
	});
}

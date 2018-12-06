
#include <util.h>

#include <Path.h>

wstring CPath::GetName()
{
	if (NULL != m_pParentPath)
	{
		return m_strName;
	}
	else
	{
		return fsutil::GetFileName(m_strName);
	}
}

void CPath::SetName(const wstring& strNewName)
{
	m_strName = strNewName;
}

wstring CPath::GetPath() const
{
	if (NULL != m_pParentPath)
	{
		return m_pParentPath->GetPath() + __BackSlant + m_strName;
	}

	return m_strName;
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
	
	m_plstSubPath->qsort([](CPath& lhs, CPath& rhs) {
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

	for (auto& pSubPath : lstSubPath)
	{
		if (pSubPath->m_bDir)
		{
			if (plstSubDir)
			{
				plstSubDir->add(pSubPath);
			}
		}
		else
		{
			if (plstSubFile)
			{
				plstSubFile->add(pSubPath);
			}
		}
	}
	
	return m_bExists;;
}

CPath *CPath::GetSubPath(wstring strSubPath, bool bDir, const wstring& strFind)
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

		TD_PathList& lstSubPath = pPath->_findFile(strFind);

		pPath = NULL;

		for (auto pSubPath : lstSubPath)
		{
			if (lstSubDirs.empty())
			{
				if (pSubPath->m_bDir != bDir)
				{
					continue;
				}
			}
			else
			{
				if (!pSubPath->m_bDir)
				{
					continue;
				}
			}

			if (util::StrCompareIgnoreCase(pSubPath->m_strName, strName))
			{
				pPath = pSubPath;
				break;
			}
		}
	}

	return pPath;
}

void CPath::ClearSubPath()
{
	if (NULL != m_plstSubPath)
	{
		for (auto pSubPath : *m_plstSubPath)
		{
			delete pSubPath;
		}

		m_plstSubPath->clear();
		delete m_plstSubPath;
		m_plstSubPath = NULL;
	}
}

void CPath::RemoveSubPath(const TD_PathList& lstDeletePaths)
{
	__Ensure(m_plstSubPath);

	for (auto itSubPath = m_plstSubPath->begin()
		; itSubPath != m_plstSubPath->end(); )
	{
		if (util::ContainerFind(lstDeletePaths, *itSubPath))
		{
			delete *itSubPath;
			itSubPath = m_plstSubPath->erase(itSubPath);
			continue;
		}			

		itSubPath++;
	}
}

UINT CPath::GetSubPathCount()
{
	if (NULL == m_plstSubPath)
	{
		return 0;
	}

	return m_plstSubPath->size();
}

bool CPath::HasFile()
{
	__EnsureReturn(m_plstSubPath, FALSE);

	for (auto pSubPath : *m_plstSubPath)
	{
		if (!pSubPath->m_bDir)
		{
			return true;
		}
	}

	return false;
}

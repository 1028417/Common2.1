
#include "util.h"

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

void CPath::SetDir(const wstring& strDir, bool bFindFile)
{
	Clear();

	m_bDir = true;
	m_strName = wsutil::rtrim_r(strDir, __wcBackSlant);

    if (bFindFile)
    {
        _findFile();
    }
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
		return m_pParentDir->GetPath() + __wcBackSlant + m_strName;
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

void CPath::_findFile()
{
	if (!m_bFinded)
	{
		Clear();
		
		_onFindFile();

		m_bFinded = true;
	}
}

void CPath::_onFindFile()
{
	m_bDirExists = fsutil::findFile(this->GetPath(), [&](const tagFileInfo& FileInfo) {
		CPath *pSubPath = NewSubPath(FileInfo, *this);
		if (pSubPath)
		{
			m_lstSubPath.add(pSubPath);
		}
	});
		
	m_lstSubPath.qsort([](const CPath& lhs, const CPath& rhs) {
		if (lhs.m_bDir && !rhs.m_bDir)
		{
			return true;
		}

		if (lhs.m_bDir == rhs.m_bDir)
		{
			return wsutil::compareUseCNCollate(lhs.m_strName, rhs.m_strName) < 0;
		}

		return false;
	});
}

const TD_PathList& CPath::GetSubPath()
{
	_findFile();

	return m_lstSubPath;
}

void CPath::_GetSubPath(TD_PathList *plstSubDir, TD_PathList *plstSubFile)
{
	_findFile();

	m_lstSubPath([&](CPath& SubPath) {
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
}

CPath *CPath::FindSubPath(wstring strSubPath, bool bDir)
{
	__EnsureReturn(m_bDir, NULL);

	list<wstring> lstSubName;
	while (!strSubPath.empty())
	{
		wstring strSubName = fsutil::GetFileName(strSubPath);
		if (strSubName.empty())
		{
			break;
		}
		
		lstSubName.push_front(strSubName);

		strSubPath = fsutil::GetParentDir(strSubPath);
	}
	
	CPath *pPath = this;

	while (!lstSubName.empty() && NULL != pPath)
	{
		wstring strSubName = lstSubName.front();
		lstSubName.pop_front();

		auto& lstSubPath = pPath->GetSubPath();
		pPath = NULL;

		lstSubPath([&](CPath& SubPath) {
			if (lstSubName.empty())
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

			if (wsutil::matchIgnoreCase(SubPath.m_strName, strSubName))
			{
				pPath = &SubPath;
				return false;
			}

			return true;
		});
	}
	
	return pPath;
}

CPath *CPath::GetSubPath(UINT uIdx) const
{
	CPath *pSubPath = NULL;
	m_lstSubPath.get(uIdx, [&](CPath& SubPath) {
		pSubPath = &SubPath;
	});

	return pSubPath;
}

void CPath::Clear()
{
	m_bDirExists = false;

	m_lstSubPath([](CPath& SubPath) {
		delete &SubPath;
	});
	m_lstSubPath.clear();

	m_bFinded = false;
}

void CPath::RemoveSubPath(set<CPath*> setDeletePaths)
{
	m_lstSubPath.del_ex([&](CPath& SubPath) {
		if (0 == setDeletePaths.erase(&SubPath))
		{
			return E_DelConfirm::DC_No;
		}
		else
		{
			if (setDeletePaths.empty())
			{
				return E_DelConfirm::DC_YesAbort;
			}
			else
			{
				return E_DelConfirm::DC_Yes;
			}
		}
	});
}

bool CPath::enumSubFile(const function<bool(CPath& dir, TD_PathList& lstSubFile)>& cb)
{
	TD_PathList lstSubFile;
	GetSubFile(lstSubFile);
	if (lstSubFile)
	{
		if (!cb(*this, lstSubFile))
		{
			return false;
		}
	}

	TD_PathList lstSubDir;
	GetSubDir(lstSubDir);
	for (auto pSubDir : lstSubDir)
	{
		if (!pSubDir->enumSubFile(cb))
		{
			return false;
		}
	}

	return true;
}

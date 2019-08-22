
#include "util.h"

CPath::CPath(const wstring& strName, bool bDir)
{
    m_FileInfo.strName = strName;

	if (bDir)
    {
        wsutil::rtrim(m_FileInfo.strName, __wcFSSlant);

        m_FileInfo.bDir = true;
    }
}

void CPath::SetDir(const wstring& strPath)
{
	Clear();

    m_FileInfo.strName = strPath;
    wsutil::rtrim(m_FileInfo.strName, __wcFSSlant);

    m_FileInfo.bDir = true;
}

wstring CPath::GetName() const
{
	if (NULL != m_FileInfo.pParent)
	{
		return m_FileInfo.strName;
	}
	else
	{
		return fsutil::GetFileName(m_FileInfo.strName);
	}
}

wstring CPath::GetPath() const
{
	if (NULL != m_FileInfo.pParent)
	{
        return m_FileInfo.pParent->GetPath() + __wcFSSlant + m_FileInfo.strName;
	}

	return m_FileInfo.strName;
}

wstring CPath::GetParentDir() const
{
	if (NULL != m_FileInfo.pParent)
	{
		return m_FileInfo.pParent->GetPath();
	}

	return L"";
}

void CPath::_findFile()
{
	if (E_FindFileStatus::FFS_None == m_eFindFileStatus)
	{
		(void)_onFindFile(m_paSubDir, m_paSubFile);
    }
}

void CPath::_onFindFile(TD_PathList& paSubDir, TD_PathList& paSubFile)
{
	(void)_findFile([&](tagFileInfo& FileInfo) {
		FileInfo.pParent = this;
		CPath *pSubPath = NewSubPath(FileInfo);
		if (NULL == pSubPath)
		{
			return;
		}

		if (FileInfo.bDir)
		{
			paSubDir.add(pSubPath);
		}
		else
		{
			paSubFile.add(pSubPath);
		}
	});

	_sort(paSubDir);
	_sort(paSubFile);
}

inline static int _sort(const wstring& lhs, const wstring& rhs)
{
#if __android
	return wsutil::toQStr(lhs).compare(wsutil::toQStr(rhs), Qt::CaseSensitivity::CaseInsensitive);
	//.localeAwareCompare(wsutil::toQStr(rhs.GetName()));
#else
	return wsutil::collate(lhs, rhs);
#endif

	return 1;
}

int CPath::_sort(const CPath& lhs, const CPath& rhs) const
{
	return ::_sort(lhs.GetName(), rhs.GetName());
}

void CPath::_sort(TD_PathList& paSubPath)
{
	paSubPath.qsort([&](const CPath& lhs, const CPath& rhs) {
		return _sort(lhs, rhs) < 0;
	});
}

bool CPath::scan(const CB_PathScan& cb)
{
    mtutil::usleep(1);

	_findFile();
	if (!cb(*this, m_paSubFile))
	{
		return false;
	}

	for (auto pSubDir : m_paSubDir)
	{
		if (!pSubDir->scan(cb))
		{
			return false;
		}
	}

	return true;
}

CPath *CPath::FindSubPath(wstring strSubPath, bool bDir)
{
	__EnsureReturn(m_FileInfo.bDir, NULL);

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
		
		cauto& paSubPath = (!lstSubName.empty() || bDir) ? pPath->dirs() : pPath->files();
		paSubPath([&](CPath& SubPath) {
			if (wsutil::matchIgnoreCase(SubPath.GetName(), strSubName))
			{
				pPath = &SubPath;
				return false;
			}

			return true;
		});
	}
	
	return pPath;
}

void CPath::RemoveSelf()
{
	if (NULL != m_FileInfo.pParent)
	{
		m_FileInfo.pParent->Remove(this);
	}
}

void CPath::Remove(CPath *pSubPath)
{
	if (NULL == pSubPath)
	{
		return;
	}

	if (pSubPath->IsDir())
	{
		if (0 != m_paSubDir.del(pSubPath))
		{
			delete pSubPath;
		}
	}
	else
	{
		if (0 != m_paSubFile.del(pSubPath))
		{
			delete pSubPath;
		}
	}
}

void CPath::Clear()
{
	for (auto p : m_paSubDir)
	{
		delete p;
	}
	m_paSubDir.clear();

	for (auto p : m_paSubDir)
	{
		delete p;
	}
	m_paSubFile.clear();

	m_eFindFileStatus = E_FindFileStatus::FFS_None;
}

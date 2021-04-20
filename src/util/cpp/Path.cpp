
#include "util.h"

wstring XFile::fileName() const
{
    if (m_fileInfo.pParent)
	{
        return m_fileInfo.strName;
	}
	else
	{
        return fsutil::GetFileName(m_fileInfo.strName);
	}
}

wstring XFile::path() const
{
    if (m_fileInfo.pParent)
	{
        auto strAbsPath = m_fileInfo.pParent->path();
        if (!strAbsPath.empty())
        {
            strAbsPath.push_back(__wcPathSeparator);
            strAbsPath.append(m_fileInfo.strName);
			return strAbsPath;
		}
	}

    return m_fileInfo.strName;
}

const CPath* XFile::rootDir() const
{
    if (NULL == m_fileInfo.pParent)
	{
		return (const CPath*)this;
	}

    return m_fileInfo.pParent->rootDir();
}

void XFile::remove()
{
    if (NULL != m_fileInfo.pParent)
	{
        m_fileInfo.pParent->remove(this);
	}
}

void CPath::assign(const TD_PathList& paSubDir, const TD_XFileList& paSubFile)
{
    for (auto pDir : paSubDir)
    {
        pDir->m_fileInfo.pParent = this;
    }
    m_paSubDir.assign(paSubDir);

    for (auto pFile : paSubFile)
    {
        pFile->m_fileInfo.pParent = this;
    }
    m_paSubFile.assign(paSubFile);

    m_bFindFileStatus = true;
}

void CPath::assign(TD_PathList&& paSubDir, TD_XFileList&& paSubFile)
{
    for (auto pDir : paSubDir)
    {
        pDir->m_fileInfo.pParent = this;
    }
    m_paSubDir.swap(paSubDir);

    for (auto pFile : paSubFile)
    {
        pFile->m_fileInfo.pParent = this;
    }
    m_paSubFile.swap(paSubFile);

    m_bFindFileStatus = true;
}

void CPath::_findFile()
{
    if (!m_bFindFileStatus)
    {        
        m_bFindFileStatus = true;
		(void)_onFindFile(m_paSubDir, m_paSubFile);
    }
}

void CPath::_onFindFile(TD_PathList& paSubDir, TD_XFileList& paSubFile)
{
    (void)fsutil::findFile(this->path(), [&](tagFileInfo& fi) {
        fi.pParent = this;

        if (fi.bDir)
		{
            XFile *pSubDir = _newSubDir(fi);
			if (pSubDir)
			{
				paSubDir.add(pSubDir);
			}
		}
		else
		{
            XFile *pSubFile = _newSubFile(fi);
			if (pSubFile)
			{
				paSubFile.add(pSubFile);
			}
		}
    });

	paSubDir.qsort([&](const CPath& lhs, const CPath& rhs) {
		return _sort(lhs, rhs) < 0;
	});

    paSubFile.qsort([&](const XFile& lhs, const XFile& rhs) {
		return _sort(lhs, rhs) < 0;
	});
}

inline static int _sort(cwstr lhs, cwstr rhs)
{
#if __windows
    return strutil::collate(lhs, rhs);
#else
    //return strutil::collate_cn(lhs, rhs);
    return __WS2Q(lhs).compare(__WS2Q(rhs), Qt::CaseSensitivity::CaseInsensitive);
    //.localeAwareCompare(__WS2Q(rhs.GetName()));
#endif

	return 1;
}

int CPath::_sort(const XFile& lhs, const XFile& rhs) const
{
    return ::_sort(lhs.fileName(), rhs.fileName());
}

XFile *CPath::findSubPath(wstring strSubPath, bool bDir)
{
    __EnsureReturn(m_fi.bDir, NULL);

	list<wstring> lstSubName;
	while (!strSubPath.empty())
	{
        cauto strSubName = fsutil::GetFileName(strSubPath);
		if (strSubName.empty())
		{
			break;
		}
		
		lstSubName.push_front(strSubName);

		strSubPath = fsutil::GetParentDir(strSubPath);
	}
	
	CPath *pSubDir = this;
	while (!lstSubName.empty())
	{
        cauto strSubName = lstSubName.front();
        if (lstSubName.size()==1 && !bDir)
        {
            for (auto pFile : pSubDir->files())
            {
                if (strutil::matchIgnoreCase(pFile->fileName(), strSubName))
                {
                    return pFile;
                }
            }
            return NULL;
		}

        bool bFlag = false;
        for (auto pDir : pSubDir->dirs())
        {
            if (strutil::matchIgnoreCase(pDir->fileName(), strSubName))
            {
                pSubDir = pDir;
                bFlag = true;
                break;
            }
        }
        if (!bFlag)
        {
            return NULL;
        }

        lstSubName.pop_front();
	}
	
	return pSubDir;
}

void CPath::remove(XFile *pSubPath)
{
	if (pSubPath)
	{
		if (pSubPath->isDir())
		{
			if (0 != m_paSubDir.del((CPath*)pSubPath))
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
}

void CPath::clear()
{
    _onClear();

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

    m_bFindFileStatus = false;
}


#include "util.h"

size_t CTxtWriter::_write(const void *pData, size_t size) const
{
	if (NULL == m_lpFile)
	{
		return 0;
	}

	if (NULL == pData || 0 == size)
	{
		return 0;
	}

	size_t ret = fwrite(pData, size, 1, m_lpFile);

#ifdef _DEBUG
	(void)fflush(m_lpFile);
#endif

	return ret;
}

bool CTxtWriter::open(const wstring& strFile, bool bTrunc)
{
	wstring strMode(bTrunc ? L"w" : L"a");
	if (m_bUTF8)
	{
		strMode.append(L"b,ccs=UTF-8");
	}
	else
	{
		strMode.append(L"b");
	}

#ifdef __ANDROID__
	m_lpFile = fopen(wsutil::toStr(strFile).c_str(), wsutil::toStr(strMode).c_str());
#else
	__EnsureReturn(0 == _wfopen_s(&m_lpFile, strFile.c_str(), strMode.c_str()), false);
#endif
	__EnsureReturn(m_lpFile, false);

	return true;
}

size_t CTxtWriter::write(const wstring& strText) const
{
	if (m_bUTF8)
	{
		return _write(wsutil::toUTF8(strText));
	}
	else
	{
		return _write(strText);
	}
}

size_t CTxtWriter::writeln(const wstring& strText) const
{
	if (E_EOLFlag::eol_n == m_eEOLFlag)
	{
		return write(strText + L"\n");
	}
	else if (E_EOLFlag::eol_rn == m_eEOLFlag)
	{
		return write(strText + L"\r\n");
	}
	else
	{
		return write(strText + L"\r");
	}
}

size_t CTxtWriter::writeln(const string& strText) const
{
	if (E_EOLFlag::eol_n == m_eEOLFlag)
	{
		return write(strText + "\n");
	}
	else if (E_EOLFlag::eol_rn == m_eEOLFlag)
	{
		return write(strText + "\r\n");
	}
	else
	{
		return write(strText + "\r");
	}
}

bool CTxtWriter::close()
{
	if (NULL != m_lpFile)
	{
		if (-1 == fclose(m_lpFile))
		{
			return false;
		}

		m_lpFile = NULL;
	}

	return true;
}

bool CUnicodeTxtWriter::open(const wstring& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strFile);

	if (!CTxtWriter::open(strFile, bTrunc))
	{
		return false;
	}

	if (!bExists || bTrunc)
	{
		(void)CTxtWriter::_write(
#ifdef __ANDROID__
			__UnicodeHead_Big
#else
			__UnicodeHead_Lit
#endif
		);
	}

	return true;
}

bool CUTF8TxtWriter::open(const wstring& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strFile);

	if (!CTxtWriter::open(strFile, bTrunc))
	{
		return false;
	}

	if (m_bWithBom)
	{
		if (!bExists || bTrunc)
		{
			(void)CTxtWriter::write(__UTF8Bom);
		}
	}

	return true;
}

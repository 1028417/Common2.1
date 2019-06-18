
#include "util.h"

static const wchar_t g_wchEolN[] = { L'\n' };
static const wchar_t g_wchEolRN[] = { L'\r', L'\n' };

static const char g_chEolN[] = { '\n' };
static const char g_chEolRN[] = { '\r', '\n' };

const string CTxtWriter::__UnicodeHead_LittleEndian({ (char)0xff, (char)0xfe });
const string CTxtWriter::__UnicodeHead_BigEndian({ (char)0xfe, (char)0xff });

const string CTxtWriter::__UTF8Bom({ (char)0xef, (char)0xbb, (char)0xbf });

bool CTxtWriter::open(const wstring& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strFile);

	wstring strMode(bTrunc ? L"w" : L"a");
	if (_isUtf8())
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


	if (!bExists || bTrunc)
	{
		if (E_TxtEncodeType::TET_Unicode_BigEndian == m_eEncodeType)
		{
			string strHead = __UnicodeHead_LittleEndian;
			(void)_fwrite(strHead.c_str(), strHead.size());
		}
		else if (E_TxtEncodeType::TET_Unicode_BigEndian == m_eEncodeType)
		{
			string strHead = __UnicodeHead_BigEndian;
			(void)_fwrite(strHead.c_str(), strHead.size());
		}
		else if (E_TxtEncodeType::TET_Utf8_WithBom == m_eEncodeType)
		{
			string strBom = __UTF8Bom;
			(void)_fwrite(strBom.c_str(), strBom.size());
		}
	}
	
	return true;
}

size_t CTxtWriter::_fwrite(const void *pData, size_t size) const
{
	return fwrite(pData, size, 1, m_lpFile);
}

size_t CTxtWriter::_write(const char *pStr, size_t len, bool bEndLine) const
{
	if (NULL == m_lpFile)
	{
		return 0;
	}

	size_t size = 0;
	if (len != 0)
	{
		if (_isUnicode())
		{
			cauto& str = wsutil::fromStr(pStr);
			size = _fwrite(str.c_str(), str.size()*2);
		}
		else
		{
			size = _fwrite(pStr, len);
		}
	}

	if (bEndLine)
	{
		size += _writeEndLine();
	}

#ifdef _DEBUG
	(void)fflush(m_lpFile);
#endif

	return size;
}

size_t CTxtWriter::_write(const wchar_t *pStr, size_t len, bool bEndLine) const
{
	if (NULL == m_lpFile)
	{
		return 0;
	}

	size_t size = 0;
	if (len != 0)
	{
		if (E_TxtEncodeType::TET_Asc == m_eEncodeType)
		{
			cauto& str = wsutil::toStr(pStr);
			size = _fwrite(str.c_str(), str.size());
		}
		else if (_isUtf8())
		{
			cauto& str = wsutil::toUTF8(pStr);
			size = _fwrite(str.c_str(), str.size());
		}
		else
		{
			size = _fwrite(pStr, len * 2);
		}
	}

	if (bEndLine)
	{
		size += _writeEndLine();
	}

#ifdef _DEBUG
	(void)fflush(m_lpFile);
#endif

	return size;
}

size_t CTxtWriter::_writeEndLine() const
{
	if (_isUnicode())
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
			return _fwrite(g_wchEolN, sizeof g_wchEolN);
		}
		else
		{
			return _fwrite(g_wchEolRN, sizeof g_wchEolRN);
		}
	}
	else
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
			return _fwrite(g_chEolN, sizeof g_chEolN);
		}
		else
		{
			return _fwrite(g_chEolRN, sizeof g_chEolRN);
		}
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

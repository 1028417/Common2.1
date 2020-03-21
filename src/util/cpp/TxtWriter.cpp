
#include "util.h"

static const wchar_t g_pwcEolN[] = { L'\n' };
static const wchar_t g_pwcEolRN[] = { L'\r', L'\n' };

static const char g_pchEolN[] = { '\n' };
static const char g_pchEolRN[] = { '\r', '\n' };

static const char __UTF8Bom[] { (char)0xef, (char)0xbb, (char)0xbf };

static const char __UCS2Head_LittleEndian[] { (char)0xff, (char)0xfe };
static const char __UCS2Head_BigEndian[]{ (char)0xfe, (char)0xff };

bool CTxtWriter::open(const wstring& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strFile);

    __EnsureReturn(m_ofs.open(strFile, bTrunc), false);

    if (!bExists || bTrunc)
    {
        _writeHead();
    }
	
	return true;
}

bool CTxtWriter::open(const string& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strutil::toWstr(strFile));

    __EnsureReturn(m_ofs.open(strFile, bTrunc), false);

    if (!bExists || bTrunc)
    {
        _writeHead();
    }

	return true;
}

bool CTxtWriter::_writeHead()
{
    if (_isUcsLittleEndian())
    {
        return m_ofs.writex(__UCS2Head_LittleEndian);
    }
    else if (_isUcsBigEndian())
    {
        return m_ofs.writex(__UCS2Head_BigEndian);
    }
    else if (_isUtf8WithBom())
    {
        return m_ofs.writex(__UTF8Bom);
    }

    return true;
}

bool CTxtWriter::_write(const void *ptr, size_t len, bool bEndLine)
{
	if (!m_ofs.writex(ptr, len))
	{
		return false;
	}

	if (bEndLine)
	{
		if (!_writeEndLine())
		{
			return false;
		}
	}

#if __isdebug
   m_ofs.flush();
#endif

	return true;
}

bool CTxtWriter::_writeEndLine()
{
	if (_isUcsLittleEndian() || _isUcsBigEndian())
	{
        if (E_EOLType::eol_n == m_eEOLFlag)
		{
            return m_ofs.writex(g_pwcEolN);
		}
		else
		{
            return m_ofs.writex(g_pwcEolRN);
		}
	}
	else
    {
        if (E_EOLType::eol_n == m_eEOLFlag)
		{
            return m_ofs.writex(g_pchEolN);
		}
		else
		{
            return m_ofs.writex(g_pchEolRN);
		}
	}
}

static E_TxtHeadType _checkHead(const char *&lpData, size_t &len)
{
	size_t headLen = sizeof __UCS2Head_LittleEndian;
	if (len >= headLen)
	{
		if (0 == memcmp(lpData, __UCS2Head_LittleEndian, headLen))
		{
			lpData += headLen;
			len -= headLen;
			return E_TxtHeadType::THT_UCS2Head_LittleEndian;
		}
	}

	headLen = sizeof __UCS2Head_BigEndian;
	if (len >= headLen)
	{
		if (0 == memcmp(lpData, __UCS2Head_BigEndian, headLen))
		{
			lpData += headLen;
			len -= headLen;
			return E_TxtHeadType::THT_UCS2Head_BigEndian;
		}
	}

	headLen = sizeof __UTF8Bom;
	if (len >= headLen)
	{
		if (0 == memcmp(lpData, __UTF8Bom, headLen))
		{
			lpData += headLen;
			len -= headLen;
			return E_TxtHeadType::THT_UTF8Bom;
		}
	}

	return E_TxtHeadType::THT_None;
}

void CTxtReader::_readData(const char *lpData, size_t len, string& strText)
{
	m_eHeadType = _checkHead(lpData, len);
	if (E_TxtHeadType::THT_UCS2Head_LittleEndian == m_eHeadType)
    {
		strText.append(strutil::toStr((const wchar_t*)lpData, len / 2));
    }
	else if (E_TxtHeadType::THT_UCS2Head_BigEndian == m_eHeadType)
	{
		cauto str = strutil::transEndian((const wchar_t*)lpData, len / 2);
        strText.append(strutil::toStr(str));
	}
	else
	{
		strText.append(lpData, len);
	}
}

void CTxtReader::_readData(const char *lpData, size_t len, wstring& strText)
{
	m_eHeadType = _checkHead(lpData, len);
	if (E_TxtHeadType::THT_UCS2Head_LittleEndian == m_eHeadType)
	{
		strText.append((const wchar_t*)lpData, len / 2);
	}
	else if (E_TxtHeadType::THT_UCS2Head_BigEndian == m_eHeadType)
	{
		cauto str = strutil::transEndian((const wchar_t*)lpData, len / 2);
        strText.append(str);
	}
	else if (E_TxtHeadType::THT_UTF8Bom == m_eHeadType || strutil::checkUtf8(lpData, len))
	{
        strText.append(strutil::fromUtf8(lpData, len));
	}
	else
	{
        strText.append(strutil::toWstr(lpData, len));
	}
}


#include "util.h"

static const wchar_t g_pwcEolN[] = { L'\n' };
static const wchar_t g_pwcEolRN[] = { L'\r', L'\n' };

static const char g_pchEolN[] = { '\n' };
static const char g_pchEolRN[] = { '\r', '\n' };

static const char __UTF8Bom[] { (char)0xef, (char)0xbb, (char)0xbf };

static const char __UCS2Head_LittleEndian[] { (char)0xff, (char)0xfe };
static const char __UCS2Head_BigEndian[]{ (char)0xfe, (char)0xff };

bool CTxtWriter::_writeHead()
{
	if (E_TxtEncodeType::TET_UCS2_LittleEndian == m_eEncodeType)
    {
        return m_ofs.writex(__UCS2Head_LittleEndian, sizeof __UCS2Head_LittleEndian);
	}
	else if (E_TxtEncodeType::TET_UCS2_BigEndian == m_eEncodeType)
    {
        return m_ofs.writex(__UCS2Head_BigEndian, sizeof __UCS2Head_BigEndian);
	}
	else if (E_TxtEncodeType::TET_UTF8_WithBom == m_eEncodeType)
    {
        return m_ofs.writex(__UTF8Bom, sizeof __UTF8Bom);
	}

    return true;
}

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

bool CTxtWriter::_write(const char *pStr, size_t len, bool bEndLine)
{
	if (len != 0)
	{
		if (_isUnicode())
		{
			cauto str = strutil::toWstr(pStr);
            if (!m_ofs.writex(str.c_str(), str.size() * 2))
			{
				return false;
			}
		}
		else
		{
            if (!m_ofs.writex(pStr, len))
			{
				return false;
			}
		}
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

bool CTxtWriter::_write(const wchar_t *pStr, size_t len, bool bEndLine)
{
	size_t size = 0;
	if (len != 0)
	{
		if (E_TxtEncodeType::TET_Asc == m_eEncodeType)
		{
			cauto str = strutil::toStr(pStr);
            size = m_ofs.writex(str.c_str(), str.size());
		}
		else if (_isUtf8())
		{
			cauto str = strutil::toUtf8(pStr);
            size = m_ofs.writex(str.c_str(), str.size());
		}
		else
		{
            size = m_ofs.writex(pStr, len * 2);
		}
	}

	if (bEndLine)
	{
		size += _writeEndLine();
	}

#if __isdebug
    m_ofs.flush();
#endif

	return size;
}

bool CTxtWriter::_writeEndLine()
{
	if (_isUnicode())
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
            return m_ofs.writex(g_pwcEolN, sizeof g_pwcEolN);
		}
		else
		{
            return m_ofs.writex(g_pwcEolRN, sizeof g_pwcEolRN);
		}
	}
	else
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
            return m_ofs.writex(g_pchEolN, sizeof g_pchEolN);
		}
		else
		{
            return m_ofs.writex(g_pchEolRN, sizeof g_pchEolRN);
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

static wstring _to_UCS2_BigEndian(const char *lpData, size_t len)
{
	TD_CharBuffer buff(len);
	memcpy(buff, lpData, len);
	char *ptr = buff;
	char chr = 0;
	for (size_t uIdx = 0; uIdx < len/2; uIdx++)
	{
		chr = *ptr;
		*ptr = *(ptr+1);
		
		ptr++;
		*ptr = chr;
	}

	return wstring((wchar_t*)buff.ptr(), len / 2);
}

void CTxtReader::_readData(const char *lpData, size_t len, string& strText)
{
	m_eHeadType = _checkHead(lpData, len);
	if (E_TxtHeadType::THT_UCS2Head_LittleEndian == m_eHeadType)
    {
        strText = strutil::toStr((wchar_t*)lpData, len / 2);
    }
	else if (E_TxtHeadType::THT_UCS2Head_BigEndian == m_eHeadType)
	{
        strText = strutil::toStr(_to_UCS2_BigEndian(lpData, len));
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
		strText.append((wchar_t*)lpData, len / 2);
	}
	else if (E_TxtHeadType::THT_UCS2Head_BigEndian == m_eHeadType)
	{
        strText = _to_UCS2_BigEndian(lpData, len);
	}
	else if (E_TxtHeadType::THT_UTF8Bom == m_eHeadType || strutil::checkUtf8(lpData, len))
	{
        strText = strutil::fromUtf8(lpData, len);
	}
	else
	{
        strText = strutil::toWstr(lpData, len);
	}
}

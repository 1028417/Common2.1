
#include "util.h"

static const wchar_t g_pwcEolN = L'\n';
static const wchar_t g_pwcEolRN[] { L'\r', L'\n' };

static const char g_pchEolN = '\n';
static const char g_pchEolRN[] { '\r', '\n' };

static const char __UTF8Bom[] { (char)0xef, (char)0xbb, (char)0xbf };

static const char __UCS2Head_LittleEndian[] { (char)0xff, (char)0xfe };
static const char __UCS2Head_BigEndian[]{ (char)0xfe, (char)0xff };

bool CTxtWriter::open(cwstr strFile, bool bTrunc)
{
    bool bWriteHead = bTrunc || !fsutil::existFile(strFile);

    __EnsureReturn(m_ofs.open(strFile, bTrunc), false);

    if (bWriteHead)
    {
        _writeHead();
    }
	
	return true;
}

bool CTxtWriter::open(const string& strFile, bool bTrunc)
{
    bool bWriteHead = bTrunc || !fsutil::existFile(strFile);

    __EnsureReturn(m_ofs.open(strFile, bTrunc), false);

    if (bWriteHead)
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

bool ITxtWriter::_write(const char *pStr, size_t len, bool bEndLine)
{
    if (_isANSI() || _isUtf8())
    {
        return _write((const void*)pStr, len, bEndLine);
    }
    else
    {
        wstring str = strutil::fromStr(pStr, len);
        return _write(str.c_str(), str.size(), bEndLine);
    }
}

inline static void _transEndian(char16_t *pStr, size_t len)
{
    for (; len--; pStr++)
    {
        *pStr = (*pStr << 8) | (*pStr >> 8);
    }
}

bool ITxtWriter::_write(const wchar_t *pStr, size_t len, bool bEndLine)
{
    if (_isANSI())
    {
        cauto str = strutil::toGbk(pStr, len);
        return _write((const void*)str.c_str(), str.size(), bEndLine);
    }
    else if (_isUtf8())
    {
        cauto str = strutil::toUtf8(pStr, len);
        return _write((const void*)str.c_str(), str.size(), bEndLine);
    }
    else
    {
        if (_isUcsBigEndian())
        {
#if !__windows
            u16string str;
            if (sizeof(wchar_t) != sizeof(char16_t))
            {
                str = QString::fromWCharArray(pStr, len).toStdU16String();
            }
            else
            {
                str.assign((char16_t*)pStr, len);
            }
#else
            u16string str((char16_t*)pStr, len);
#endif
            _transEndian((char16_t*)str.c_str(), str.size());
            return _write((const void*)str.c_str(), str.size() *sizeof(char16_t), bEndLine);
        }
        else
        {
    #if !__windows
            if (sizeof(wchar_t) != sizeof(char16_t))
            {
                cauto str = QString::fromWCharArray(pStr, len).toStdU16String();
                return _write((const void*)str.c_str(), str.size() *sizeof(char16_t), bEndLine);
            }
    #endif
            return _write((const void*)pStr, len *sizeof(wchar_t), bEndLine);
        }
    }
}

#if !__winvc
bool ITxtWriter::_write(cqstr qstr, bool bEndLine)
{
    if (_isANSI())
    {
        cauto str = strutil::toGbk(qstr);
        return _write((const void*)str.c_str(), str.size(), bEndLine);
    }
    else if (_isUtf8())
    {
        cauto ba = qstr.toUtf8();
        return _write((const void*)ba.constData(), ba.length(), bEndLine);
    }
    else
    {
        wstring str = qstr.toStdWString();
        return _write(str.c_str(), str.size(), bEndLine);
    }
}
#endif

static E_TxtHeadType _checkHead(char *&lpData, size_t &len)
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

inline static void _praseUcs2(const char16_t *lpData, size_t len, string& str)
{
#if !__windows
    if (sizeof(char16_t) != sizeof(wchar_t))
    {
        str.append(QString::fromUtf16(lpData, len).toStdString());
        return;
    }
#endif
    str.append(strutil::toUtf8((wchar_t*)lpData, len));
}

E_TxtHeadType CTxtReader::_readData(char *lpData, size_t len, string& strText)
{
	E_TxtHeadType eHeadType = _checkHead(lpData, len);
	if (E_TxtHeadType::THT_UCS2Head_LittleEndian == eHeadType)
    {
        len /= sizeof(char16_t);
        _praseUcs2((char16_t*)lpData, len, strText);
    }
	else if (E_TxtHeadType::THT_UCS2Head_BigEndian == eHeadType)
    {
        len /= sizeof(char16_t);
        _transEndian((char16_t*)lpData, len);
        _praseUcs2((char16_t*)lpData, len, strText);
	}
	else
	{
		strText.append(lpData, len);
	}
	return eHeadType;
}

inline static void _praseUcs2(const char16_t *lpData, size_t len, wstring& str)
{
#if !__windows
    if (sizeof(char16_t) != sizeof(wchar_t))
    {
        str.append(QString::fromUtf16(lpData, len).toStdWString());
        return;
    }
#endif
    str.append((wchar_t*)lpData, len);
}

E_TxtHeadType CTxtReader::_readData(char *lpData, size_t len, wstring& strText)
{
	E_TxtHeadType eHeadType = _checkHead(lpData, len);
	if (E_TxtHeadType::THT_UCS2Head_LittleEndian == eHeadType)
	{
        len /= sizeof(char16_t);
        _praseUcs2((char16_t*)lpData, len, strText);
	}
	else if (E_TxtHeadType::THT_UCS2Head_BigEndian == eHeadType)
    {
        len /= sizeof(char16_t);
        _transEndian((char16_t*)lpData, len);
        _praseUcs2((char16_t*)lpData, len, strText);
	}
    else if (E_TxtHeadType::THT_UTF8Bom == eHeadType)
	{
        strText.append(strutil::fromUtf8(lpData, len));
	}
	else
    {
        strText.append(strutil::fromStr(lpData, len));
	}
	return eHeadType;
}

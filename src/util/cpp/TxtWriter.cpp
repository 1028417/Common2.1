
#include "util.h"

static const wchar_t g_pwcEolN[] = { L'\n' };
static const wchar_t g_pwcEolRN[] = { L'\r', L'\n' };

static const char g_pchEolN[] = { '\n' };
static const char g_pchEolRN[] = { '\r', '\n' };

const string CTxtWriter::__UnicodeHead_LittleEndian({ (char)0xff, (char)0xfe });
const string CTxtWriter::__UnicodeHead_BigEndian({ (char)0xfe, (char)0xff });

const string CTxtWriter::__UTF8Bom({ (char)0xef, (char)0xbb, (char)0xbf });

bool CTxtWriter::_writeHead()
{
	if (E_TxtEncodeType::TET_Unicode_LittleEndian == m_eEncodeType)
	{
        string strHead = __UnicodeHead_LittleEndian;
        return m_obs.writeex(strHead.c_str(), strHead.size());
	}
	else if (E_TxtEncodeType::TET_Unicode_BigEndian == m_eEncodeType)
	{
		string strHead = __UnicodeHead_BigEndian;
        return m_obs.writeex(strHead.c_str(), strHead.size());
	}
	else if (E_TxtEncodeType::TET_Utf8_WithBom == m_eEncodeType)
	{
		string strBom = __UTF8Bom;
        return m_obs.writeex(strBom.c_str(), strBom.size());
	}

    return true;
}

bool CTxtWriter::open(const wstring& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strFile);

    __EnsureReturn(m_obs.open(strFile, bTrunc), false);

    if (!bExists || bTrunc)
    {
        _writeHead();
    }
	
	return true;
}

bool CTxtWriter::open(const string& strFile, bool bTrunc)
{
	bool bExists = fsutil::existFile(strutil::toWstr(strFile));

    __EnsureReturn(m_obs.open(strFile, bTrunc), false);

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
			if (!m_obs.writeex(str.c_str(), str.size() * 2))
			{
				return false;
			}
		}
		else
		{
            if (!m_obs.writeex(pStr, len))
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
   m_obs.flush();
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
            size = m_obs.writeex(str.c_str(), str.size());
		}
		else if (_isUtf8())
		{
			cauto str = strutil::toUtf8(pStr);
            size = m_obs.writeex(str.c_str(), str.size());
		}
		else
		{
            size = m_obs.writeex(pStr, len * 2);
		}
	}

	if (bEndLine)
	{
		size += _writeEndLine();
	}

#if __isdebug
    m_obs.flush();
#endif

	return size;
}

bool CTxtWriter::_writeEndLine()
{
	if (_isUnicode())
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
            return m_obs.writeex(g_pwcEolN, sizeof g_pwcEolN);
		}
		else
		{
            return m_obs.writeex(g_pwcEolRN, sizeof g_pwcEolRN);
		}
	}
	else
	{
		if (E_EOLFlag::eol_n == m_eEOLFlag)
		{
            return m_obs.writeex(g_pchEolN, sizeof g_pchEolN);
		}
		else
		{
            return m_obs.writeex(g_pchEolRN, sizeof g_pchEolRN);
		}
	}
}

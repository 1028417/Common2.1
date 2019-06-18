
#pragma once

#ifdef __ANDROID__
#define __DefEOL E_EOLFlag::eol_n
#else
#define __DefEOL E_EOLFlag::eol_rn
#endif

enum class E_EOLFlag
{
	eol_n = 0,
	eol_rn
};

enum class E_TxtEncodeType
{
	TET_Asc = 0,

	TET_Unicode_LittleEndian,
	TET_Unicode_BigEndian,

	TET_Utf8_NoBom,
	TET_Utf8_WithBom
};

class __UtilExt CTxtWriter
{
public:
	static const string __UnicodeHead_LittleEndian;
	static const string __UnicodeHead_BigEndian;
	
	static const string __UTF8Bom;

	CTxtWriter(E_TxtEncodeType eEncodeType, E_EOLFlag eEOLFlag = __DefEOL) :
		m_eEncodeType(eEncodeType),
		m_eEOLFlag(eEOLFlag)
	{
	}

	~CTxtWriter()
	{
		close();
	}

private:
	E_TxtEncodeType m_eEncodeType;

	E_EOLFlag m_eEOLFlag;

	FILE *m_lpFile = NULL;

private:
	inline bool _isUnicode() const
	{
		return E_TxtEncodeType::TET_Unicode_LittleEndian == m_eEncodeType
			|| E_TxtEncodeType::TET_Unicode_BigEndian == m_eEncodeType;
	}

	inline bool _isUtf8() const
	{
		return E_TxtEncodeType::TET_Utf8_NoBom == m_eEncodeType
			|| E_TxtEncodeType::TET_Utf8_WithBom == m_eEncodeType;
	}

	inline size_t _fwrite(const void *pData, size_t size) const;

	size_t _write(const char *pStr, size_t len, bool bEndLine = false) const;

	size_t _write(const wchar_t *pStr, size_t len, bool bEndLine = false) const;

	inline size_t _writeEndLine() const;

public:
	virtual bool open(const wstring& strFile, bool bTrunc);

	size_t write(const wstring& strText) const
	{
		return _write(strText.c_str(), strText.size());
	}

	size_t writeln(const wstring& strText) const
	{
		return _write(strText.c_str(), strText.size(), true);
	}

	size_t write(const string& strText) const
	{
		return _write(strText.c_str(), strText.size());
	}

	size_t writeln(const string& strText) const
	{
		return _write(strText.c_str(), strText.size(), true);
	}

	size_t write(const wchar_t *pStr) const
	{
		if (NULL == pStr)
		{
			return 0;
		}

		return _write(pStr, wcslen(pStr));
	}

	size_t writeln(const wchar_t* pStr) const
	{
		if (NULL == pStr)
		{
			return 0;
		}

		return _write(pStr, wcslen(pStr), true);
	}

	size_t write(const char *pStr) const
	{
		if (NULL == pStr)
		{
			return 0;
		}

		return _write(pStr, strlen(pStr));
	}

	size_t writeln(const char *pStr) const
	{
		if (NULL == pStr)
		{
			return 0;
		}

		return _write(pStr, strlen(pStr), true);
	}

	template <typename T>
	size_t write(const T& num) const
	{
		if (E_TxtEncodeType::TET_Asc == m_eEncodeType)
		{
			return write(to_string(num));
		}
		else
        {
			return write(to_wstring(num));
		}
	}

	template <typename T>
	size_t writeln(const T& num) const
	{
		if (E_TxtEncodeType::TET_Asc == m_eEncodeType)
		{
			return writeln(to_string(num));
		}
		else
		{
			return writeln(to_wstring(num));
		}
	}

	template <typename T>
    const CTxtWriter& operator<<(const T& num) const
	{
		(void)write(num);
		return *this;
	}

	bool close();
};

#ifdef __ANDROID__
#define __DefUnicodeHeadOpt E_UnicodeHeadOpt::UHO_BigEndian
#else
#define __DefUnicodeHeadOpt E_UnicodeHeadOpt::UHO_LittleEndian
#endif

enum class E_UnicodeHeadOpt
{
	UHO_LittleEndian = 0,
	UHO_BigEndian,
};

class __UtilExt CUnicodeTxtWriter : public CTxtWriter
{
public:
	CUnicodeTxtWriter(E_UnicodeHeadOpt eHeadType = __DefUnicodeHeadOpt, E_EOLFlag eEOLFlag = __DefEOL) :
		CTxtWriter(E_UnicodeHeadOpt::UHO_LittleEndian == eHeadType ? E_TxtEncodeType::TET_Unicode_LittleEndian
			: E_TxtEncodeType::TET_Unicode_BigEndian, eEOLFlag)
	{
	}
};

#ifdef _MSC_VER
#define __DefUTF8BomOpt true
#else
#define __DefUTF8BomOpt false
#endif

class __UtilExt CUTF8TxtWriter : public CTxtWriter
{
public:
	CUTF8TxtWriter(bool bWithBom = __DefUTF8BomOpt, E_EOLFlag eEOLFlag = __DefEOL) :
		CTxtWriter(bWithBom ? E_TxtEncodeType::TET_Utf8_WithBom : E_TxtEncodeType::TET_Utf8_NoBom, eEOLFlag)
	{
	}
};


#pragma once

#define __UnicodeHead_Lit string({(char)0xff, (char)0xfe})
#define __UnicodeHead_Big string({(char)0xfe, (char)0xff})

#define __UTF8Bom string({(char)0xef, (char)0xbb, (char)0xbf})

#ifdef __ANDROID__
#define __DefEOL E_EOLFlag::eol_n
#else
#define __DefEOL E_EOLFlag::eol_rn
#endif

enum class E_EOLFlag
{
	eol_n = 0,
	eol_rn,
	eol_r
};

class ITxtWriter
{
public:
    virtual bool open(const wstring& strFile, bool bTrunc) = 0;

	virtual size_t write(const wstring& strText) const = 0;
	virtual size_t write(const string& strText) const = 0;
	
	virtual size_t writeln(const wstring& strText) const = 0;
	virtual size_t writeln(const string& strText) const = 0;

    virtual bool close() = 0;
};

class __UtilExt CTxtWriter : public ITxtWriter
{
public:
	CTxtWriter() {}

	CTxtWriter(E_EOLFlag eEOLFlag)
	{
		m_eEOLFlag = eEOLFlag;
	}

	CTxtWriter(bool bUTF8)
	{
		m_bUTF8 = bUTF8;
	}

	CTxtWriter(E_EOLFlag eEOLFlag, bool bUTF8)
	{
		m_eEOLFlag = eEOLFlag;

		m_bUTF8 = bUTF8;
	}

	~CTxtWriter()
	{
		close();
	}

private:
	E_EOLFlag m_eEOLFlag = __DefEOL;

	bool m_bUTF8 = false;

	FILE *m_lpFile = NULL;

protected:
	size_t _write(const void *pData, size_t size) const;

	inline size_t _write(const wstring& strText) const
	{
		return _write(strText.c_str(), strText.size() * sizeof(wchar_t));
	}

	inline size_t _write(const string& strText) const
	{
		return _write(strText.c_str(), strText.size());
	}

public:
	virtual bool open(const wstring& strFile, bool bTrunc) override;

	virtual size_t write(const wstring& strText) const override;

	virtual size_t write(const string& strText) const override
	{
		return _write(strText);
	}

	virtual size_t writeln(const wstring& strText) const override;

	virtual size_t writeln(const string& strText) const override;

	bool close() override;
};

class __UtilExt CUnicodeTxtWriter : public CTxtWriter
{
public:
	CUnicodeTxtWriter(E_EOLFlag eEOLFlag = __DefEOL)
		: CTxtWriter(eEOLFlag)
	{
	}

public:
	bool open(const wstring& strFile, bool bTrunc) override;

	size_t write(const wstring& strText) const override
	{
		return CTxtWriter::_write(strText);
	}

	size_t writeln(const wstring& strText) const override
	{
		return CTxtWriter::writeln(strText);
	}

	size_t write(const string& strText) const override
	{
		return write(wsutil::fromStr(strText));
	}

	size_t writeln(const string& strText) const override
	{
		return writeln(wsutil::fromStr(strText));
	}
};

class __UtilExt CUTF8TxtWriter : public CTxtWriter
{
public:
	CUTF8TxtWriter()
		: CTxtWriter(true)
	{
	}

	CUTF8TxtWriter(E_EOLFlag eEOLFlag)
		: CTxtWriter(eEOLFlag, true)
	{
	}

	CUTF8TxtWriter(bool bWithBom)
		: CTxtWriter(true)
	{
		m_bWithBom = bWithBom;
	}

	CUTF8TxtWriter(E_EOLFlag eEOLFlag, bool bWithBom)
		: CTxtWriter(eEOLFlag)
	{
		m_bWithBom = bWithBom;
	}

private:
	bool m_bWithBom = false;

public:
	bool open(const wstring& strFile, bool bTrunc) override;
};

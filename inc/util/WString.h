
#pragma once

class __UtilExt WString
{
private:
	wstring m_str;

public:
    WString() {}

    template <typename T>
    WString(const T& t)
    {
        *this << t;
    }

    WString(const WString& other)
        : m_str(other.m_str)
    {
    }
	WString& operator=(const WString& other)
	{
		m_str.assign(other.m_str);
		return *this;
	}

	WString(const wstring& str)
		: m_str(str)
	{
	}
	WString& operator=(const wstring& str)
	{
		m_str.assign(str);
		return *this;
	}

    WString(WString&& other)
    {
		m_str.swap(other.m_str);
    }
	WString& operator=(WString&& other)
	{
		m_str.swap(other.m_str);
		return *this;
	}

    WString(wstring&& str)
    {
		m_str.swap(str);
    }
    WString& operator=(wstring&& str)
    {
		m_str.swap(str);
        return *this;
    }

    template <typename T>
    WString& operator=(const T& t)
    {
		m_str.clear();
        *this << t;
        return *this;
    }
	
public:
#if !__winvc
    operator QString() const
    {
        return strutil::toQstr(m_str);
    }

    QString qstr() const
    {
        return strutil::toQstr(m_str);
    }
#endif
	
    WString& operator<<(const WString& other)
    {
		m_str.append(other.m_str);
        return *this;
    }

    WString& operator<<(const wchar_t *pStr)
    {
        if (pStr)
        {
			m_str.append(pStr);
        }
        return *this;
    }

	WString& operator<<(wchar_t wc)
	{
		m_str.append(1, wc);
		return *this;
	}

	WString& operator<<(char c)
	{
		return *this << (wchar_t)c;
	}

	const wstring* operator->() const
	{
		return &m_str;
	}
	wstring* operator->()
	{
		return &m_str;
	}

    WString& operator<<(const wstring& str)
    {
		m_str.append(str);
        return *this;
    }

    template <typename T>
    WString& operator<<(const T& t)
    {
		m_str.append(to_wstring(t));
        return *this;
    }

    template <typename T>
    WString& append(const T& t)
    {
        *this << t;
        return *this;
    }

    template <typename T>
    WString operator +(const T& t) const
    {
        WString ret(*this);
        ret << t;
        return ret;
    }

	const wstring& operator*() const
	{
		return m_str;
	}
	wstring& operator*()
	{
		return m_str;
	}

	operator const wstring&() const
	{
		return m_str;
	}
	operator wstring&()
	{
		return m_str;
	}

	operator const wchar_t*() const
	{
		return m_str.c_str();
	}
};
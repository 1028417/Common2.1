
#pragma once

#include <string>

#if !__winvc
#include <QString>

#define __W2Q(pStr, len) QString::fromWCharArray(pStr, len)
#define __WS2Q(wstr) QString::fromStdWString(wstr)

#define to_string(x) QString::number(x).toStdString()
#define to_wstring(x) QString::number(x).toStdWString()

#define sscanf_s sscanf
#endif

#define __npos string::npos
#define __wnpos wstring::npos

#define __substr strutil::substr

class __UtilExt strutil
{
private:
    inline static bool _checkLen(const wchar_t *pStr, int& len)
    {
        if (-1 == len)
        {
            len = wcslen(pStr);
        }

        return len > 0;
    }

    inline static bool _checkLen(const char *pStr, int& len)
    {
        if (-1 == len)
        {
            len = strlen(pStr);
        }

        return len > 0;
    }

public:
	static int collate(const wstring& lhs, const wstring& rhs);
	static int collate_cn(const wstring& lhs, const wstring& rhs);

	template <class S>
	static S substr(const S& str, size_t pos, size_t len = -1)
	{
		if (str.length() < pos)
		{
			return S();
		}

		return str.substr(pos, len);
	}

        static bool endWith(const wstring& str, const wstring& strEnd);
        static bool endWith(const string& str, const string& strEnd);

	template <class S, typename T = decltype(*S().c_str())>
	inline static void ltrim(S& str, T t = ' ')
	{
		auto pos = str.find_first_not_of(t);
		str.erase(0, pos);
	}

	template <class S, typename T = decltype(*S().c_str())>
	static S ltrim_r(const S& str, T t = ' ')
	{
		auto pos = str.find_first_not_of(t);
		if (S::npos == pos)
		{
			return S();
		}
		
		if (pos > 0)
		{
			return str.substr(pos);
		}
		
		return str;
	}

	template <class S, typename T = decltype(*S().c_str())>
	inline static void rtrim(S& str, T t = ' ')
	{
		auto pos = str.find_last_not_of(t);
		str.erase(pos+1);
	}
	template <class S, typename T = decltype(*S().c_str())>
	static S rtrim_r(const S& str, T t = ' ')
	{
		auto pos = str.find_last_not_of(t);
		return str.substr(0, pos+1);
	}

	template <class S, typename T = decltype(*S().c_str())>
	static void trim(S& str, T t = ' ')
	{
		ltrim(str, t);
		rtrim(str, t);
	}

	template <class S, typename T = decltype(*S().c_str())>
	static S trim_r(const S& str, T t = ' ')
	{
		auto strRet = ltrim_r(str, t);
		rtrim(strRet, t);
		return strRet;
	}

	static void split(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet);
	static void split(const string& strText, char wcSplitor, vector<string>& vecRet);

	static bool matchIgnoreCase(const wstring& str1, const wstring& str2, size_t maxlen = 0);

	static void lowerCase(wstring& str)
	{
		for (auto& chr : str)
		{
			chr = ::towlower(chr);
		}
	}

	static void lowerCase(string& str)
	{
		for (auto& chr : str)
		{
			chr = ::tolower(chr);
		}
	}

	template <class S>
	static S lowerCase_r(const S& str)
	{
		auto strRet = str;
		lowerCase(strRet);
		return strRet;
	}

	static void upperCase(wstring& str)
	{
		for (auto& chr : str)
		{
			chr = ::towupper(chr);
		}
	}

	static void upperCase(string& str)
	{
		for (auto& chr : str)
		{
			chr = ::toupper(chr);
		}
	}

	template <class S>
	static S upperCase_r(const S& str)
	{
		auto strRet = str;
		upperCase(strRet);
		return strRet;
	}

	static UINT replace(wstring& str, const wstring& strFind, const wchar_t *pszReplace = NULL);
	static UINT replace(string& str, const string& strFind, const char *pszReplace = NULL);

	static UINT replace(wstring& str, const wstring& strFind, const wstring& strReplace);
	static UINT replace(string& str, const string& strFind, const string& strReplace);

	template <class S, typename T = decltype(S().c_str())>
	static S replace_r(const S& str, const S& strFind, T pszReplace = NULL)
	{
		auto strRet = str;
		replace(strRet, strFind, pszReplace);
		return strRet;
	}

	template <class S>
	static S replace_r(const S& str, const S& strFind, const S& strReplace)
	{
		auto strRet = str;
		replace(strRet, strFind, strReplace);
		return strRet;
	}
	
	template <class S, typename T=decltype(S()[0])>
    static UINT replaceChar(S& str, T chrFind, T chrReplace)
	{
		UINT uRet = 0;
		for (auto& chr : str)
		{
			if (chrFind == chr)
			{
				chr = chrReplace;
				uRet++;
			}
		}

		return uRet;
	}
	template <class S, typename T = decltype(S()[0])>
    static S replaceChar_r(const S& str, T chrFind, T chrReplace)
    {
        auto strRet = str;
        replaceChar(strRet, chrFind, chrReplace);
        return strRet;
    }

	template <class S, typename T = decltype(S()[0])>
    static UINT replaceChars(S& str, const S& strFindChars, T chrReplace)
	{
		UINT uRet = 0;
		for (auto& chr : str)
		{
			if (S::npos != strFindChars.find(chr))
			{
				chr = chrReplace;
				uRet++;
			}
		}

		return uRet;
	}
	template <class S, typename T = decltype(S()[0])>
    static S replaceChars_r(const S& str, const S& strFindChars, T chrReplace)
    {
        auto strRet = str;
        replaceChars(strRet, strFindChars, chrReplace);
        return strRet;
    }

	template <class S, typename T = decltype(S()[0])>
	static UINT eraseChar(S& str, T chrFind)
	{
		UINT uRet = 0;
		for (auto itr = str.begin(); itr != str.end(); )
		{
			if (*itr == chrFind)
			{
				itr = str.erase(itr);
				uRet++;
			}
			else
			{
				++itr;
			}
		}

		return uRet;
	}
	template <class S, typename T = decltype(S()[0])>
    static S eraseChar_r(const S& str, T chrFind)
    {
        auto strRet = str;
        eraseChar(strRet, chrFind);
        return strRet;
    }

    template <class S>
    static UINT eraseChars(S& str, const S& strFindChars)
    {
		UINT uRet = 0;
        for (auto itr=str.begin(); itr!=str.end(); )
        {
            if (S::npos != strFindChars.find(*itr))
            {
                itr = str.erase(itr);
				uRet++;
            }
            else
            {
                ++itr;
            }
        }

		return uRet;
    }
    template <class S>
    static S eraseChars_r(const S& str, const S& strFindChars)
    {
        auto strRet = str;
        eraseChars(strRet, strFindChars);
        return strRet;
    }

	static string base64_encode(const char *pStr, size_t len, const char *pszBase = NULL, char chrTail = 0);
	static string base64_encode(const char *pStr, const char *pszBase = NULL, char chrTail = 0)
	{
		return base64_encode(pStr, (size_t)strlen(pStr), pszBase, chrTail);
	}
	static string base64_encode(const string& str, const char *pszBase = NULL, char chrTail = 0)
	{
		return base64_encode(str.c_str(), str.length(), pszBase, chrTail);
	}

	static string base64_decode(const char *pStr, size_t len, const char *pszBase = NULL, char chrTail = 0);
	static string base64_decode(const char *pStr, const char *pszBase = NULL, char chrTail = 0)
	{
		return base64_decode(pStr, (size_t)strlen(pStr), pszBase, chrTail);
	}
	static string base64_decode(const string& str, const char *pszBase = NULL, char chrTail = 0)
	{
		return base64_decode(str.c_str(), str.length(), pszBase, chrTail);
	}
	
	static bool checkUtf8(const char *pStr, int len = -1);
	static bool checkUtf8(const string& str)
	{
		return checkUtf8(str.c_str(), str.size());
	}

	static wstring fromUtf8(const char *pStr, int len = -1);
	static wstring fromUtf8(const string& str)
	{
		return fromUtf8(str.c_str(), str.size());
	}

	static string toUtf8(const wchar_t *pStr, int len = -1);
	static string toUtf8(const wstring& str)
	{
		return toUtf8(str.c_str(), str.size());
	}

    inline static wchar_t transEndian(wchar_t wch)
    {
        return (wch << 8) | (wch >> 8);
    }

    static wstring toWstr(const char *pStr, int len = -1);
    static wstring toWstr(const string& str)
    {
        return toWstr(str.c_str(), str.size());
    }

    static string toStr(const wchar_t *pStr, int len = -1);
    static string toStr(const wstring& str)
    {
        return toStr(str.c_str(), str.size());
    }

    static void transEndian(wstring& str);
    static wstring transEndian(const wchar_t *pStr, int len=-1)
    {
        wstring str(pStr, len);
        transEndian(str);
        return str;
    }

	static bool checkGbk(const char *pStr, int len = -1);
	static bool checkGbk(const string& str)
	{
		return checkGbk(str.c_str(), str.size());
	}

#if !__winvc
    static QString fromGbk(const char *pStr, int len = -1);
    static QString fromGbk(const string& str)
	{
		return fromGbk(str.c_str(), str.size());
	}

    static string toGbk(const QString& qs);

    static string toStr(const QString& qs)
    {
        return toStr(qs.toStdWString());
    }
#endif

	template <typename T>
	static wstring ContainerToStr(const T& container, const wstring& strSplitor)
	{
        wstringstream strmRet;
        for (typename T::const_iterator it = container.begin(); ; )
		{
            strmRet << *it;
			
			it++;
			__EnsureBreak(it != container.end());

            strmRet << strSplitor;
		}

        return strmRet.str();
	}
	
	struct tagCNSortor
	{
		bool operator()(const wstring& lhs, const wstring& rhs) const
		{
			return strutil::collate(lhs, rhs)<0;
		}
    };
};

#include "WString.h"

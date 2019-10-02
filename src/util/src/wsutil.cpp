﻿
#include "util.h"

#include <locale>

/*#if __windows
    static const char *CN_LOCALE_STRING = "Chinese_china";
    static const locale g_locale_CN(CN_LOCALE_STRING);
    //static const locale g_locale_CN("");
    static const collate<wchar_t>& g_collate_CN = use_facet<collate<wchar_t> >(g_locale_CN);
#else
    #include <QLocale>
    #include <QCollator>
    static const QLocale g_locale_CN(QLocale::Chinese, QLocale::China);
    static const QCollator& g_collate_CN = QCollator(g_locale_CN);
#endif*/

static class __init
{
public:
    __init()
    {
        setlocale(LC_COLLATE, "chs");
        setlocale(LC_CTYPE, "chs");
 /*
#if !__windows
        g_collate_CN.setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive)
#endif*/
    }
} init;

bool wsutil::checkWChar(const wstring& str)
{
    for (auto& wchr : str)
    {
        if (wchr > 255)
        {
            return true;
        }
    }

    return false;
}

void wsutil::trim(wstring& strText, wchar_t chr)
{
    strText.erase(0, strText.find_first_not_of(chr));
    strText.erase(strText.find_last_not_of(chr) + 1);
}

wstring wsutil::trim_r(const wstring& strText, wchar_t chr)
{
    wstring strRet(strText);
    trim(strRet, chr);
    return strRet;
}

void wsutil::ltrim(wstring& strText, wchar_t chr)
{
    strText.erase(0, strText.find_first_not_of(chr));
}

wstring wsutil::ltrim_r(const wstring& strText, wchar_t chr)
{
    wstring strRet(strText);
    ltrim(strRet, chr);
    return strRet;
}

void wsutil::rtrim(wstring& strText, wchar_t chr)
{
    strText.erase(strText.find_last_not_of(chr) + 1);
}

wstring wsutil::rtrim_r(const wstring& strText, wchar_t chr)
{
    wstring strRet(strText);
    rtrim(strRet, chr);
    return strRet;
}

void wsutil::split(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet, bool bTrim)
{
    auto fn = [&](const wstring& strSub) {
        if (bTrim && wcSplitor != L' ')
        {
            cauto& str = trim_r(strSub);
            if (!str.empty())
            {
                vecRet.push_back(str);
            }
        }
        else
        {
            vecRet.push_back(strSub);
        }
    };

    size_t pos = 0;
    while ((pos = strText.find_first_not_of(wcSplitor, pos)) != wstring::npos)
    {
        auto nextPos = strText.find(wcSplitor, pos);
        if (wstring::npos == nextPos)
        {
            fn(strText.substr(pos));
            break;
        }

        fn(strText.substr(pos, nextPos - pos));

        pos = nextPos + 1;
    }
}

int wsutil::collate(const wstring& lhs, const wstring& rhs)
{
    return wcscoll(lhs.c_str(), rhs.c_str());

#if __windows
    //return g_collate_CN.compare(lhs.c_str(), lhs.c_str() + lhs.size()
        //, rhs.c_str(), rhs.c_str() + rhs.size());
#else
    //return g_collate_CN.compare(toQStr(lhs), toQStr(rhs));
#endif
}

bool wsutil::matchIgnoreCase(const wstring& str1, const wstring& str2)
{
#if __windows
    return 0 == _wcsicmp(str1.c_str(), str2.c_str());
#else
    return 0 == toQStr(str1).compare(toQStr(str2), Qt::CaseSensitivity::CaseInsensitive);
#endif
}

void wsutil::lowerCase(wstring& str)
{
#if __windows
    (void)::_wcslwr_s((wchar_t*)str.c_str(), str.size() + 1);
#else
    str = toQStr(str).toLower().toStdWString();
#endif
}

wstring wsutil::lowerCase_r(const wstring& str)
{
    wstring strRet = str;
    lowerCase(strRet);
    return strRet;
}

void wsutil::upperCase(wstring& str)
{
#if __windows
    (void)::_wcsupr_s((wchar_t*)str.c_str(), str.size() + 1);
#else
    str = toQStr(str).toUpper().toStdWString();
#endif
}

wstring wsutil::upperCase_r(const wstring& str)
{
    wstring strRet = str;
    upperCase(strRet);
    return strRet;
}

void wsutil::replaceChar(wstring& str, wchar_t chrFind, wchar_t chrReplace)
{
    for (auto& chr : str)
    {
        if (chrFind == chr)
        {
            chr = chrReplace;
        }
    }
}

void wsutil::replaceChars(wstring& str, const wstring& strFindChars, wchar_t chrReplace)
{
    for (auto& chr : str)
    {
        if (wstring::npos != strFindChars.find(chr))
        {
            chr = chrReplace;
        }
    }
}

wstring wsutil::replaceChar_r(const wstring& str, wchar_t chrFind, wchar_t chrReplace)
{
    wstring strRet(str);
    replaceChar(strRet, chrFind, chrReplace);
    return strRet;
}

wstring wsutil::replaceChars_r(const wstring& str, const wstring& strFindChars, wchar_t chrReplace)
{
    wstring strRet(str);
    replaceChars(strRet, strFindChars, chrReplace);
    return strRet;
}

#if __winvc
#include <codecvt>
using utf8_convert = std::wstring_convert<std::codecvt_utf8<wchar_t>>;
static utf8_convert g_utf8Convert;
#endif

inline static wstring _fromUTF8(const char *pStr)
{
#if __winvc
    return g_utf8Convert.from_bytes(pStr);
#else
    return QString::fromUtf8(pStr).toStdWString();
#endif
}

wstring wsutil::fromUTF8(const string& str)
{
    if (str.empty())
    {
        return L"";
    }

    return _fromUTF8(str.c_str());
}

inline static string _toUTF8(const wchar_t *pStr)
{
#if __winvc
    return g_utf8Convert.to_bytes(pStr);
#else
    return wsutil::toQStr(pStr).toUtf8().constData();
#endif
}

string wsutil::toUTF8(const wstring& str)
{
    if (str.empty())
    {
        return "";
    }

    return _toUTF8(str.c_str());
}

string wsutil::toUTF8(const wchar_t *pStr)
{
    if (NULL == pStr)
    {
        return "";
    }

    return _toUTF8(pStr);
}

static bool _checkUTF8(const char *pStr)
{
    UINT nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
    unsigned char chr = *pStr;
    bool bAllAscii = true;
    for (UINT uIndex = 0; pStr[uIndex] != '\0'; ++uIndex) {
        chr = *(pStr + uIndex);
        //判断是否ASCII编码,如果不是,说明有可能是UTF8,ASCII用7位编码,最高位标记为0,0xxxxxxx
        if (nBytes == 0 && (chr & 0x80) != 0) {
            bAllAscii = false;
        }
        if (nBytes == 0) {
            //如果不是ASCII码,应该是多字节符,计算字节数
            if (chr >= 0x80) {
                if (chr >= 0xFC && chr <= 0xFD) {
                    nBytes = 6;
                }
                else if (chr >= 0xF8) {
                    nBytes = 5;
                }
                else if (chr >= 0xF0) {
                    nBytes = 4;
                }
                else if (chr >= 0xE0) {
                    nBytes = 3;
                }
                else if (chr >= 0xC0) {
                    nBytes = 2;
                }
                else {
                    return false;
                }
                nBytes--;
            }
        }
        else {
            //多字节符的非首字节,应为 10xxxxxx
            if ((chr & 0xC0) != 0x80) {
                return false;
            }
            //减到为零为止
            nBytes--;
        }
    }
    //违返UTF8编码规则
    if (nBytes != 0) {
        return false;
    }
    if (bAllAscii) { //如果全部都是ASCII, 也是UTF8
        return false;
    }

    return true;
}

#if __windows
static wstring _fromStr(const char *pStr, size_t len = 0)
{
    if (0 == len)
    {
        len = strlen(pStr);
        if (0 == len)
        {
            return L"";
        }
    }

    vector<wchar_t> vecBuff(len + 1);
    wchar_t *pBuff = &vecBuff.front();

    if (mbstowcs_s(NULL, pBuff, len+1, pStr, len))
    {
        return L"";
    }

    return pBuff;
}
#endif

wstring wsutil::fromStr(const string& str, bool bCheckUTF8)
{
    if (str.empty())
    {
        return L"";
    }

    if (bCheckUTF8 && _checkUTF8(str.c_str()))
    {
        return _fromUTF8(str.c_str());
    }

#if __windows
    return _fromStr(str.c_str());
#else
    return QString::fromStdString(str).toStdWString();
#endif
}

wstring wsutil::fromStr(const char *pStr, bool bCheckUTF8)
{
    if (NULL == pStr)
    {
        return L"";
    }

    if (bCheckUTF8 && _checkUTF8(pStr))
    {
        return _fromUTF8(pStr);
    }

#if __windows
    return _fromStr(pStr);
#else
    return QString::fromStdString(pStr).toStdWString();
#endif
}

#if __windows
static string _toStr(const wchar_t *pStr, size_t len = 0)
{
	if (0 == len)
	{
		len = wcslen(pStr);
		if (0 == len)
		{
			return "";
		}
	}

    vector<char> vecBuff(len+1);
    char *pBuff = &vecBuff.front();

    if (wcstombs_s(NULL, pBuff, len+1, pStr, len))
    {
        return "";
    }

    return pBuff;
}
#endif

string wsutil::toStr(const wstring& str)
{
    if (str.empty())
    {
        return "";
    }

#if __windows
    return _toStr(str.c_str(), str.size());
#else
    return QString::fromStdWString(str).toStdString();
#endif
}

string wsutil::toStr(const wchar_t *pStr)
{
    if (NULL == pStr)
    {
        return "";
    }

#if __windows
    return _toStr(pStr);
#else
    return QString::fromStdWString(pStr).toStdString();
#endif
}


#include "util.h"

#include <locale>

static struct __localeInit {
    __localeInit() {
        setlocale(LC_COLLATE, ""); // "chs");
        setlocale(LC_CTYPE, ""); // "chs");

        //std::locale::global(std::locale(""));
    }
} localeInit;

#if __winvc
    static const locale g_locale_CN("Chinese_china");
    static const collate<wchar_t>& g_collate_CN = use_facet<collate<wchar_t>>(g_locale_CN);

#else
#include <QLocale>
#include <QCollator>
    static const QLocale g_locale_CN(QLocale::Chinese, QLocale::China);
    static const QCollator& g_collate_CN = QCollator(g_locale_CN);
#endif

int strutil::collate(const wstring& lhs, const wstring& rhs)
{
	return wcscoll(lhs.c_str(), rhs.c_str());

    //return toQstr(lhs).localeAwareCompare(toQstr(rhs));
}

int strutil::collate_cn(const wstring& lhs, const wstring& rhs)
{
#if __winvc
    return g_collate_CN.compare(lhs.c_str(), lhs.c_str() + lhs.size()
        , rhs.c_str(), rhs.c_str() + rhs.size());
#else
    return g_collate_CN.compare(toQstr(lhs), toQstr(rhs));
#endif
}

wstring strutil::substr(const wstring& str, size_t pos, size_t len)
{
    if (str.length() < pos)
    {
        return L"";
    }

    return str.substr(pos, len);
}

string strutil::substr(const string& str, size_t pos, size_t len)
{
	if (str.length() < pos)
	{
		return "";
	}

	return str.substr(pos, len);
}

bool strutil::endWith(const wstring& str, const wstring& strEnd)
{
	int pos = (int)str.length() - strEnd.length();
	if (pos < 0)
	{
		return false;
	}

	return substr(str, pos) == strEnd;
}

bool strutil::endWith(const string& str, const string& strEnd)
{
	int pos = (int)str.length() - strEnd.length();
	if (pos < 0)
	{
		return false;
	}

	return substr(str, pos) == strEnd;
}

template <typename T, class C = basic_string<T, char_traits<T>, allocator<T>>>
inline static void _trim(C& strText, T chr)
{
    strText.erase(0, strText.find_first_not_of(chr));
    strText.erase(strText.find_last_not_of(chr) + 1);
}

void strutil::trim(wstring& strText, wchar_t chr)
{
    _trim(strText, chr);
}

void strutil::trim(string& strText, char chr)
{
    _trim(strText, chr);
}

wstring strutil::trim_r(const wstring& strText, wchar_t chr)
{
    wstring strRet(strText);
    _trim(strRet, chr);
    return strRet;
}

string strutil::trim_r(const string& strText, char chr)
{
    string strRet(strText);
    _trim(strRet, chr);
    return strRet;
}

void strutil::ltrim(wstring& strText, wchar_t chr)
{
    strText.erase(0, strText.find_first_not_of(chr));
}

wstring strutil::ltrim_r(const wstring& strText, wchar_t chr)
{
    wstring strRet(strText);
    ltrim(strRet, chr);
    return strRet;
}

void strutil::rtrim(wstring& strText, wchar_t chr)
{
    strText.erase(strText.find_last_not_of(chr) + 1);
}

wstring strutil::rtrim_r(const wstring& strText, wchar_t chr)
{
    wstring strRet(strText);
    rtrim(strRet, chr);
    return strRet;
}

template <typename T, class C = basic_string<T, char_traits<T>, allocator<T>>>
static void _split(const C& strText, T wcSplitor, vector<C>& vecRet, bool bTrim)
{
    auto fn = [&](const C& strSub) {
        if (bTrim && wcSplitor != L' ')
        {
            cauto str = strutil::trim_r(strSub);
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
    while ((pos = strText.find_first_not_of(wcSplitor, pos)) != C::npos)
    {
        auto nextPos = strText.find(wcSplitor, pos);
        if (C::npos == nextPos)
        {
            fn(strText.substr(pos));
            break;
        }

        fn(strText.substr(pos, nextPos - pos));

        pos = nextPos + 1;
    }
}

void strutil::split(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet, bool bTrim)
{
    _split(strText, wcSplitor, vecRet, bTrim);
}

void strutil::split(const string& strText, char wcSplitor, vector<string>& vecRet, bool bTrim)
{
    _split(strText, wcSplitor, vecRet, bTrim);
}

bool strutil::matchIgnoreCase(const wstring& str1, const wstring& str2)
{
#if __windows
    return 0 == _wcsicmp(str1.c_str(), str2.c_str());
#else
    return 0 == toQstr(str1).compare(toQstr(str2), Qt::CaseSensitivity::CaseInsensitive);
#endif
}

void strutil::lowerCase(wstring& str)
{
    for (auto& chr : str)
    {
        chr = ::towlower(chr);
    }
}

void strutil::lowerCase(string& str)
{
    for (auto& chr : str)
    {
        chr = ::tolower(chr);
    }
}

wstring strutil::lowerCase_r(const wstring& str)
{
    wstring strRet = str;
    lowerCase(strRet);
    return strRet;
}

string strutil::lowerCase_r(const string& str)
{
    string strRet = str;
    lowerCase(strRet);
    return strRet;
}

void strutil::upperCase(wstring& str)
{
    for (auto& chr : str)
    {
        chr = ::towupper(chr);
    }
}

void strutil::upperCase(string& str)
{
    for (auto& chr : str)
    {
        chr = ::toupper(chr);
    }
}

wstring strutil::upperCase_r(const wstring& str)
{
    wstring strRet = str;
    upperCase(strRet);
    return strRet;
}

string strutil::upperCase_r(const string& str)
{
    string strRet = str;
    upperCase(strRet);
    return strRet;
}

template <class T>
static void _replace(T& str, const T& strFind, const T& strReplace)
{
    auto findLen = strFind.length();
    if (0 == findLen)
    {
        return;
    }

    auto replaceLen = strReplace.length();

    for (size_t pos = 0; pos+findLen <= str.length(); )
    {
        pos = str.find(strFind.c_str(), pos, findLen);
        if (T::npos == pos)
        {
            break;
        }

        if (0 == replaceLen)
        {
            str.erase(pos, findLen);
        }
        else
        {
            str.replace(pos, findLen, strReplace);
            pos += replaceLen;
        }
    }
}

void strutil::replace(wstring& str, const wstring& strFind, const wstring& strReplace)
{
    _replace(str, strFind, strReplace);
}

void strutil::replace(string& str, const string& strFind, const string& strReplace)
{
    _replace(str, strFind, strReplace);
}

void strutil::replaceChar(wstring& str, wchar_t chrFind, wchar_t chrReplace)
{
    for (auto& chr : str)
    {
        if (chrFind == chr)
        {
            chr = chrReplace;
        }
    }
}

void strutil::replaceChars(wstring& str, const wstring& strFindChars, wchar_t chrReplace)
{
    for (auto& chr : str)
    {
        if (wstring::npos != strFindChars.find(chr))
        {
            chr = chrReplace;
        }
    }
}

wstring strutil::replaceChar_r(const wstring& str, wchar_t chrFind, wchar_t chrReplace)
{
    wstring strRet(str);
    replaceChar(strRet, chrFind, chrReplace);
    return strRet;
}

wstring strutil::replaceChars_r(const wstring& str, const wstring& strFindChars, wchar_t chrReplace)
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

bool strutil::checkUtf8(const char *pStr, size_t len)
{
	UINT nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
	unsigned char chr = *pStr;
	bool bAllAscii = true;
	for (size_t uIndex = 0; uIndex < len; ++uIndex)
	{
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

wstring strutil::fromUtf8(const char *pStr, size_t len)
{
#if __winvc
	return g_utf8Convert.from_bytes(pStr, pStr+len);
#else
    return QString::fromUtf8(pStr, len).toStdWString();
#endif
}

string strutil::toUtf8(const wchar_t *pStr, size_t len)
{
#if __winvc
    return g_utf8Convert.to_bytes(pStr, pStr+len);
#else
    return strutil::toQstr(wstring(pStr, len)).toUtf8().constData();
#endif
}

string strutil::toUtf8(const wstring& str)
{
#if __winvc
    return toUtf8(str.c_str(), str.size());
#else
    return strutil::toQstr(str).toUtf8().constData();
#endif
}

wstring strutil::toWstr(const char *pStr, size_t len)
{
#if __winvc
	if (0 == len)
	{
		return L"";
	}

	TBuffer<wchar_t> buff(len+1);
	if (mbstowcs_s(NULL, buff, len+1, pStr, len))
	{
		return L"";
	}

	return buff.ptr();

#else
    return QString::fromStdString(string(pStr, len)).toStdWString();
#endif
}

wstring strutil::toWstr(const string& str)
{
#if __winvc
    return toWstr(str.c_str(), str.size());
#else
    return QString::fromStdString(str).toStdWString();
#endif
}

string strutil::toStr(const wchar_t *pStr, size_t len)
{
#if __winvc
	if (0 == len)
	{
		return "";
	}

	TD_CharBuffer buff(len+1);
    if (wcstombs_s(NULL, buff, len+1, pStr, len))
    {
        return "";
    }

    return buff.ptr();

#else
    return QString::fromStdWString(wstring(pStr, len)).toStdString();
#endif
}

string strutil::toStr(const wstring& str)
{
#if __winvc
    return toStr(str.c_str(), str.size());
#else
    return QString::fromStdWString(str).toStdString();
#endif
}

static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789+/";

static inline bool is_base64(const char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

string strutil::base64_encode(const char *bytes_to_encode, unsigned int in_len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--)
    {
        char_array_3[i++] = *(bytes_to_encode++);
        if(i == 3)
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for(i = 0; (i <4) ; i++)
            {
                ret += base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    if(i)
    {
        for(j = i; j < 3; j++)
        {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for(j = 0; (j < i + 1); j++)
        {
            ret += base64_chars[char_array_4[j]];
        }

        while((i++ < 3))
        {
            ret += '=';
        }

    }
    return ret;
}

string strutil::base64_decode(const string& encoded_string)
{
    int in_len = (int) encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = (unsigned char)base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }
    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = (unsigned char)base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}


#include "util.h"

#include <locale>

static struct __localeInit {
    __localeInit() {
		setlocale(LC_COLLATE, "");
		setlocale(LC_CTYPE, "");

		//_wsetlocale(LC_COLLATE, L"chs");
		//_wsetlocale(LC_CTYPE, L"chs");

        //std::locale::global(std::locale(""));
    }
} localeInit;

#if __winvc
#include <codecvt>
	static std::wstring_convert<std::codecvt_utf8<wchar_t>> g_utf8Convert;

    static const locale g_locale_CN("Chinese_china");
    static const collate<wchar_t>& g_collate_CN = use_facet<collate<wchar_t>>(g_locale_CN);

#else
#include <QTextCodec>
    static QTextCodec *g_gbkCodec = QTextCodec::codecForName("GBK");
    //static QTextCodec *g_utf8Codec = QTextCodec::codecForName("UTF-8");

#include <QLocale>
#include <QCollator>
    static const QLocale g_locale_CN(QLocale::Chinese, QLocale::China);
    static const QCollator& g_collate_CN = QCollator(g_locale_CN);
#endif

int strutil::collate(const wstring& lhs, const wstring& rhs)
{
	return wcscoll(lhs.c_str(), rhs.c_str());

    //return __WS2Q(lhs).localeAwareCompare(__WS2Q(rhs));
}

int strutil::collate_cn(const wstring& lhs, const wstring& rhs)
{
#if __winvc
    return g_collate_CN.compare(lhs.c_str(), lhs.c_str() + lhs.size()
        , rhs.c_str(), rhs.c_str() + rhs.size());
#else
    return g_collate_CN.compare(__WS2Q(lhs), __WS2Q(rhs));
#endif
}

template <class S>
inline static bool _endWith(const S& str, const S& strEnd)
{
    int pos = (int)str.length() - strEnd.length();
    if (pos < 0)
    {
        return false;
    }

    return strutil::substr(str, pos) == strEnd;
}

bool strutil::endWith(const wstring& str, const wstring& strEnd)
{
    return _endWith(str, strEnd);
}

bool strutil::endWith(const string& str, const string& strEnd)
{
    return _endWith(str, strEnd);
}

template <typename T, class C = basic_string<T, char_traits<T>, allocator<T>>>
static void _split(const C& strText, T wcSplitor, vector<C>& vecRet)
{
    /*auto fn = [&](const C& strSub) {
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
    };*/

    size_t pos = 0;
    while ((pos = strText.find_first_not_of(wcSplitor, pos)) != C::npos)
    {
        auto nextPos = strText.find(wcSplitor, pos);
        if (C::npos == nextPos)
        {
			cauto strSub = strText.substr(pos);
            vecRet.push_back(strSub);  //fn(strSub);
            break;
        }

		cauto strSub = strText.substr(pos, nextPos - pos);
		vecRet.push_back(strSub);  //fn(strSub);

        pos = nextPos + 1;
    }
}

void strutil::split(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet)
{
    _split(strText, wcSplitor, vecRet);
}

void strutil::split(const string& strText, char wcSplitor, vector<string>& vecRet)
{
    _split(strText, wcSplitor, vecRet);
}

bool strutil::matchIgnoreCase(const wstring& str1, const wstring& str2, size_t maxlen)
{
    if (0 == maxlen)
    {
#if __windows
        return 0 == _wcsicmp(str1.c_str(), str2.c_str());
#else
        return 0 == __WS2Q(str1).compare(__WS2Q(str2), Qt::CaseSensitivity::CaseInsensitive);
#endif
    }
    else
    {
#if __windows
        return 0 == _wcsnicmp(str1.c_str(), str2.c_str(), maxlen);
#else
        size_t len = str1.size();
        cauto qstr1 = __W2Q(str1.c_str(), MIN(maxlen, len));

        len = str2.size();
        cauto qstr2 = QString::fromWCharArray(str2.c_str(), MIN(maxlen, len));

        return 0 == qstr1.compare(qstr2, Qt::CaseSensitivity::CaseInsensitive);
#endif
    }
}

template <class S, typename T=decltype(S().c_str())>
inline static UINT _replace(S& str, const S& strFind, T pszReplace = NULL, size_t replaceLen = 0)
{
    auto findLen = strFind.length();
    if (0 == findLen)
    {
        return 0;
    }
	auto pszFind = strFind.c_str();
    
	UINT uRet = 0;
    for (size_t pos = 0; pos+findLen <= str.length(); )
    {
        pos = str.find(pszFind, pos, findLen);
        if (S::npos == pos)
        {
            break;
        }

		uRet++;

		if (pszReplace && replaceLen > 0)
		{
			str.replace(pos, findLen, pszReplace, replaceLen);
			pos += replaceLen;
		}
		else
		{
			str.erase(pos, findLen);
        }
    }

	return uRet;
}

UINT strutil::replace(wstring& str, const wstring& strFind, const wchar_t *pszReplace)
{
	if (pszReplace)
	{
		return _replace(str, strFind, pszReplace, wcslen(pszReplace));
	}
	else
	{
		return _replace(str, strFind);
	}
}

UINT strutil::replace(string& str, const string& strFind, const char *pszReplace)
{
	if (pszReplace)
	{
		return _replace(str, strFind, pszReplace, strlen(pszReplace));
	}
	else
	{
		return _replace(str, strFind);
	}
}

UINT strutil::replace(wstring& str, const wstring& strFind, const wstring& strReplace)
{
	return _replace(str, strFind, strReplace.c_str(), strReplace.length());
}

UINT strutil::replace(string& str, const string& strFind, const string& strReplace)
{
	return _replace(str, strFind, strReplace.c_str(), strReplace.length());
}

bool strutil::checkGbk(unsigned char *pStr, int len)
{
    if (-1 == len)
    {
        len = strlen(pStr);
    }
    if (len <= 0)
    {
        return false;
    }

    bool bAllAscii = true;
    int nIdx = 0;
    while (nIdx < len)
    {
        auto chr = pStr[nIdx];

        if (chr <= 0x7f)
        {
            //编码小于等于127,只有一个字节的编码，兼容ASCII
            nIdx++;
            continue;
        }

        bAllAscii = false;

        //大于127的使用双字节编码
        if (chr >= 0x81 && chr <= 0xfe)
        {
            chr = pStr[nIdx+1];
            if (chr >= 0x40 && chr <= 0xfe && chr != 0xf7)
            {
                nIdx += 2;
                continue;
            }
        }

        return false;
    }

    if (bAllAscii)
    {
        return false;
    }

    return true;
}

bool strutil::checkUtf8(const char *pStr, int len)
{
	if (-1 == len)
	{
		len = strlen(pStr);
	}
	if (len <= 0)
	{
		return false;
	}
		
	UINT nBytes = 0;
	unsigned char chr = 0;
	bool bAllAscii = true;
    for (int nIdx = 0; nIdx < len; nIdx++) {
        chr = *(pStr + nIdx);
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

	if (bAllAscii) {
		return false;
	}

	return true;
}

/*	auto bytes = (const unsigned char*)pStr;
	auto end = bytes + len;
	unsigned int cp = 0;
	int num = 0;

	while (bytes < end) {
		if ((*bytes & 0x80) == 0x00) {
			// U+0000 to U+007F
			cp = (*bytes & 0x7F);
			num = 1;
		}
		else if ((*bytes & 0xE0) == 0xC0) {
			// U+0080 to U+07FF
			cp = (*bytes & 0x1F);
			num = 2;
		}
		else if ((*bytes & 0xF0) == 0xE0) {
			// U+0800 to U+FFFF
			cp = (*bytes & 0x0F);
			num = 3;
		}
		else if ((*bytes & 0xF8) == 0xF0) {
			// U+10000 to U+10FFFF
			cp = (*bytes & 0x07);
			num = 4;
		}
		else {
			return false;
		}

		bytes += 1;
		for (int i = 1; i < num; ++i) {
		if ((*bytes & 0xC0) != 0x80)
		{
			return false;
		}

		cp = (cp << 6) | (*bytes & 0x3F);
			bytes += 1;
		}

		if ((cp > 0x10FFFF) ||
			((cp >= 0xD800) && (cp <= 0xDFFF)) ||
			((cp <= 0x007F) && (num != 1)) ||
			((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
			((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
			((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
		{
			return false;
		}
	}

	return true;
}*/

wstring strutil::fromUtf8(const char *pStr, int len)
{
    if (-1 == len)
    {
        len = strlen(pStr);
    }
    if (len <= 0)
    {
        return L"";
    }

#if __winvc
	return g_utf8Convert.from_bytes(pStr, pStr+len);
#else
    //return g_utf8Codec->toUnicode(pStr, len).toStdWString();
    return __A2Q(pStr, len).toStdWString();
#endif
}

string strutil::toUtf8(const wchar_t *pStr, int len)
{
    if (-1 == len)
    {
        len = wcslen(pStr);
    }
    if (len <= 0)
    {
        return "";
    }

#if __winvc
    return g_utf8Convert.to_bytes(pStr, pStr+len);
#else
    return __W2Q(pStr, len).toUtf8().toStdString();
#endif
}

wstring strutil::toWstr(const char *pStr, int len)
{
    if (-1 == len)
    {
        len = strlen(pStr);
    }
    if (len <= 0)
    {
        return L"";
    }

#if __winvc
	wstring strRet(len, L'\0');
	int nRet = MultiByteToWideChar(CP_ACP, 0, pStr, len, (wchar_t*)strRet.c_str(), len);
	if (nRet <= 0)
	{
		return L"";
	}
	strRet.erase(nRet);
	return strRet;
	
	/*wstring strRet(len, L'\0');
	if (mbstowcs_s(NULL, (wchar_t*)strRet.c_str(), len+1, pStr, len))
	{
		return L"";
	}
	return strRet;*/

#else
    return __A2Q(pStr, len).toStdWString();
#endif
}

string strutil::toStr(const wchar_t *pStr, int len)
{
    if (-1 == len)
    {
        len = wcslen(pStr);
    }
    if (len <= 0)
    {
        return "";
    }

#if __winvc
	string strRet(len*2, '\0');
	int nRet = WideCharToMultiByte(CP_ACP, 0, pStr, len, (char*)strRet.c_str(), len*2, NULL, NULL);
	if (nRet <= 0)
	{
		return "";
	}
	strRet.erase(nRet);
	return strRet;

#else
    return __W2Q(pStr, len).toUtf8().toStdString();
#endif
}

void strutil::transEndian(wstring& str)
{
    for (auto& wch : str)
    {
        wch = transEndian(wch);
    }
}


static const std::string g_strBase64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789-_";

template <typename T, typename RET = basic_string<T, char_traits<T>, allocator<T>>>
RET _base64_encode(const char *pStr, size_t len, const string& strBase64Chars = g_strBase64Chars, char chrTail = 0)
{
	RET strRet;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

    while (len--)
	{
        char_array_3[i++] = *(pStr++);
		if (i == 3)
		{
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			for (i = 0; (i <4); i++)
			{
                strRet.push_back(strBase64Chars[char_array_4[i]]);
			}
			i = 0;
		}
	}
	if (i)
	{
		for (j = i; j < 3; j++)
		{
			char_array_3[j] = '\0';
		}

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
		{
            strRet.push_back(strBase64Chars[char_array_4[j]]);
		}

		if (chrTail != '\0')
		{
			while ((i++ < 3))
			{
                strRet.push_back(chrTail);
			}
		}
	}

	return strRet;
}

static string _base64_decode(const char *pStr, size_t len, const string& strBase64Chars = g_strBase64Chars, char chrTail = 0)
{
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (len-- && (pStr[in_] != chrTail) && strBase64Chars.find(pStr[in_]) != __npos) {
        char_array_4[i++] = pStr[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = (unsigned char)strBase64Chars.find(char_array_4[i]);

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
            char_array_4[j] = (unsigned char)strBase64Chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

string strutil::base64_encode(const char *pStr, size_t len, const char *pszBase, char chrTail)
{
	if (pszBase)
	{
		string strBase64Chars(pszBase, 64);
		return _base64_encode<char>(pStr, len, strBase64Chars, chrTail);
	}
	else
	{
		return _base64_encode<char>(pStr, len);
	}
}

string strutil::base64_decode(const char *pStr, size_t len, const char *pszBase, char chrTail)
{
	if (pszBase)
	{
		string strBase64Chars(pszBase, 64);
		return _base64_decode(pStr, len, strBase64Chars, chrTail);
	}
	else
	{
		return _base64_decode(pStr, len);
	}
}

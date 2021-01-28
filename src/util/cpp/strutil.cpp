
#include "util.h"

#define CP_GBK 936u

static const string g_s;
static const wstring g_ws;

/*static union {char c[4]; uint32_t l;} endian_test{{'l', '?', '?', 'b'}};
#define __endian (char(endian_test.l))
#define __lendian (__endian == 'l')
#define __bendian (__endian == 'b')*/
//int i=1; (*(char*)&i);

#if __winvc
#include <codecvt>
    //全asc的转utf8很多崩溃 static std::wstring_convert<std::codecvt_utf8<wchar_t>> g_utf8Convert;

	static const locale g_locale_CN("Chinese_china");
    static const collate<wchar_t>& g_collate_CN = use_facet<collate<wchar_t>>(g_locale_CN);

#else
#include <QTextCodec>
    static QTextCodec *g_gbkCodec = QTextCodec::codecForName("GBK");
    //static QTextCodec *g_utf8Codec = QTextCodec::codecForName("UTF-8");

#include <QLocale>
#include <QCollator>
    static const QLocale g_locale_CN(QLocale::Chinese, QLocale::China);
    static QCollator g_collate_CN(g_locale_CN);
#endif

static struct __localeInit {
    __localeInit() {
#if __windows
        if (NULL == setlocale(LC_CTYPE, ".936"))
        {
            setlocale(LC_CTYPE, "");
            setlocale(LC_COLLATE, "");
        }
        else
        {
            setlocale(LC_COLLATE, ".936");
        }

        //setlocale(LC_ALL, "Chinese_china");
        //std::locale::global(std::locale("Chinese_china"));
#else
        setlocale(LC_CTYPE, "");
        setlocale(LC_COLLATE, "");
#endif

#if !__winvc
        g_collate_CN.setCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
#endif
    }
} localeInit;

int strutil::collate(cwstr lhs, cwstr rhs)
{
	return wcscoll(lhs.c_str(), rhs.c_str());

    //return __WS2Q(lhs).localeAwareCompare(__WS2Q(rhs));
}

int strutil::collate_cn(cwstr lhs, cwstr rhs)
{
#if __winvc
    return g_collate_CN.compare(lhs.c_str(), lhs.c_str() + lhs.size()
        , rhs.c_str(), rhs.c_str() + rhs.size());
#else
    return g_collate_CN.compare(__WS2Q(lhs), __WS2Q(rhs));
#endif
}

UINT strutil::checkWordCount(cwstr str)
{
    UINT uCount= 0;
    for (cauto wch : str)
    {
        if (wch > 255)
        {
            uCount += 2;
        }
        else
        {
            uCount++;
        }
    }

    if (uCount%2 == 0)
    {
        uCount++;
    }

    return uCount/2;
}

template <class S>
inline static bool _endWith(const S& str, const S& strEnd)
{
    int pos = (int)str.size() - strEnd.size();
    if (pos < 0)
    {
        return false;
    }

    return strutil::substr(str, pos) == strEnd;
}

bool strutil::endWith(cwstr str, cwstr strEnd)
{
    return _endWith(str, strEnd);
}

bool strutil::endWith(const string& str, const string& strEnd)
{
    return _endWith(str, strEnd);
}

// TODO
/*template <class S>
static bool _matchIgnoreCase(const S& str1, const S& str2, size_t maxlen)
{
    if (0 == maxlen)
    {
        maxlen = (size_t)-1;
    }

    auto ptr1 = str1.c_str();
    auto ptr2 = str2.c_str();
    auto len1 = str1.size();
    auto len2 = str2.size();

    auto AaDiff = 'A'-'a';

    for (UINT uIdx = 0; ; uIdx++)
    {
        if (uIdx == maxlen)
        {
            return true;
        }
        if (uIdx >= len1 || uIdx >= len2)
        {
            break;
        }

        auto& chr1 = ptr1[uIdx];
        auto& chr2 = ptr2[uIdx];
        if (chr1 == chr2)
        {
            continue;
        }

        if (chr1 >= 'a' && chr1 <= 'z')
        {
            if (chr1 + AaDiff == chr2)
            {
                continue;
            }
        }
        else if (chr1 >= 'A' && chr1 <= 'Z')
        {
            if (chr1 - AaDiff == chr2)
            {
                continue;
            }
        }
        return false;
    }

    return len1 == len2;
}

template <typename C>
static bool _matchIgnoreCase(const C *pstr1, const C *pstr2, size_t maxlen)
{
}*/

bool strutil::matchIgnoreCase(cwstr str1, cwstr str2, size_t maxlen)
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
        cauto qs1 = __W2Q(str1.c_str(), MIN(maxlen, len));

        len = str2.size();
        cauto qs2 = __W2Q(str2.c_str(), MIN(maxlen, len));

        return 0 == qs1.compare(qs2, Qt::CaseSensitivity::CaseInsensitive);
#endif
    }
}

bool strutil::matchIgnoreCase(cwchr_p pstr1, cwchr_p pstr2, size_t maxlen)
{
    if (0 == maxlen)
    {
#if __windows
        return 0 == _wcsicmp(pstr1, pstr2);
#else
        return 0 == QString::fromWCharArray(pstr1).compare(QString::fromWCharArray(pstr2), Qt::CaseSensitivity::CaseInsensitive);
#endif
    }
    else
    {
#if __windows
        return 0 == _wcsnicmp(pstr1, pstr2, maxlen);
#else
        size_t len = _checkLen(pstr1);
        cauto qs1 = __W2Q(pstr1, MIN(maxlen, len));

        len = _checkLen(pstr2);
        cauto qs2 = __W2Q(pstr2, MIN(maxlen, len));

        return 0 == qs1.compare(qs2, Qt::CaseSensitivity::CaseInsensitive);
#endif
    }
}

template <class S, typename T=decltype(S().c_str())>
inline static UINT _replace(S& str, const S& strFind, T pszReplace = NULL, size_t replaceLen = 0)
{
    auto findLen = strFind.size();
    if (0 == findLen)
    {
        return 0;
    }
	auto pszFind = strFind.c_str();
    
	UINT uRet = 0;
    for (size_t pos = 0; pos+findLen <= str.size(); )
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

UINT strutil::replace(wstring& str, cwstr strFind, const wchar_t *pszReplace)
{
	if (pszReplace)
	{
		return _replace(str, strFind, pszReplace, _checkLen(pszReplace));
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
		return _replace(str, strFind, pszReplace, _checkLen(pszReplace));
	}
	else
	{
		return _replace(str, strFind);
	}
}

UINT strutil::replace(wstring& str, cwstr strFind, cwstr strReplace)
{
    return _replace(str, strFind, strReplace.c_str(), strReplace.size());
}

UINT strutil::replace(string& str, const string& strFind, const string& strReplace)
{
    return _replace(str, strFind, strReplace.c_str(), strReplace.size());
}

/* 好像有bug bool strutil::checkGbk(const char *pStr, int len)
{
	if (!_checkLen(pStr, len))
	{
		return false;
	}

    bool bAllAscii = true;
    int nIdx = 0;
    while (nIdx < len)
    {
        auto chr = (unsigned char)pStr[nIdx];

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
            chr = (unsigned char)pStr[nIdx+1];
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
}*/

bool strutil::checkUtf8(const char *pStr, int len)
{
    if (!_checkLen(pStr, len))
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
    if (!_checkLen(pStr, len))
    {
        return g_ws;
    }

#if __winvc
	return fromMbs(CP_GBK, pStr, len);
	//return g_utf8Convert.from_bytes(pStr, pStr+len);
#else
    //return g_utf8Codec->toUnicode(pStr, len).toStdWString();
    return QString::fromUtf8(pStr, len).toStdWString();
#endif
}

string strutil::toUtf8(const wchar_t *pStr, int len)
{
    if (!_checkLen(pStr, len))
    {
        return g_s;
    }

#if __winvc
	return toMbs(CP_UTF8, pStr, len);
    //return g_utf8Convert.to_bytes(pStr, pStr+len);
#else
    return __W2Q(pStr, len).toUtf8().toStdString();
#endif
}

wstring strutil::fromGbk(const char *pStr, int len)
{
	if (!_checkLen(pStr, len))
	{
		return g_ws;
	}

#if __windows
		return fromMbs(CP_GBK, pStr, len);
#else
		return g_gbkCodec->toUnicode(pStr, len).toStdWString();
#endif
}

#if __windows
string strutil::toMbs(UINT uCodePage, const wchar_t *pStr, int len)
{
	if (_checkLen(pStr, len))
	{
		auto size = WideCharToMultiByte(uCodePage, 0, pStr, len, NULL, 0, NULL, NULL);
		if (size > 0)
		{
			string strRet(size, '\0');
			size = WideCharToMultiByte(uCodePage, 0, pStr, len, (char*)strRet.c_str(), size, NULL, NULL);
			if (size > 0)
			{
				strRet.erase(size);
				return strRet;
			}
		}

		/*auto size = std::wcstombs(nullptr, pStr, len);
		if (size_t(-1) != size)
		{
			string str(size, '\0');
			size = std::wcstombs((char*)str.c_str(), pStr, len);
			if (size_t(-1) != size)
			{
				str.erase(size);
				return str;
			}
		}*/
    }
    return g_s;
}

wstring strutil::fromMbs(UINT uCodePage, const char *pStr, int len)
{
	if (_checkLen(pStr, len))
	{
		wstring strRet(len, L'\0');
		auto size = MultiByteToWideChar(uCodePage, 0, pStr, len, (wchar_t*)strRet.c_str(), len);
		if (size > 0)
		{
			strRet.erase(size);
			return strRet;
		}

	    /*auto size = len;//std::mbstowcs(nullptr, pStr, len);
		//if (size_t(-1) != size) {
			wstring str(size, '\0');
			size = std::mbstowcs((wchar_t*)str.c_str(), pStr, len);
			if (size_t(-1) != size)
			{
				str.erase(size);
				return str;
			}
		//}*/
	}
	return g_ws;
}

#endif

string strutil::toGbk(const wchar_t *pStr, int len)
{
#if __windows
	return toMbs(CP_GBK, pStr, len);
#else
	return g_gbkCodec->fromUnicode(__W2Q(pStr, len)).toStdString();
#endif
}

#if !__winvc
/*QString strutil::fromGbk(const char *pStr, int len)
{
    if (!_checkLen(pStr, len))
    {
        return "";
    }

    return g_gbkCodec->toUnicode(pStr, len);
}

QString strutil::fromGbk(const string& str)
{
    return fromGbk(str.c_str(), str.size());
}*/

string strutil::toGbk(cqstr qs)
{
    return g_gbkCodec->fromUnicode(qs).toStdString();
}
#endif

string strutil::toAsc(const wchar_t *pStr, int len)
{
    if (!_checkLen(pStr, len))
    {
        return g_s;
    }

    string str;
    str.resize(len);
    for (int nIdx = 0; nIdx < len; nIdx++)
    {
        str[nIdx] = (char)pStr[nIdx];
    }
    return str;
}

wstring strutil::fromAsc(const char *pStr, int len)
{
    if (!_checkLen(pStr, len))
    {
        return g_ws;
    }

    wstring str;
    str.resize(len);
    for (int nIdx = 0; nIdx < len; nIdx++)
    {
        str[nIdx] = pStr[nIdx];
    }
    return str;
}

wstring strutil::fromStr(const char *pStr, int len)
{
	if (strutil::checkUtf8(pStr, len))
	{
		return strutil::fromUtf8(pStr, len);
	}
	else
	{
		return strutil::fromGbk(pStr, len);
	}
}

static const string g_strBase64 =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "0123456789-_";

template <class S=string>
S _base64_encode(const char *pStr, size_t len, const char *pszBase = g_strBase64.c_str(), char chrTail = 0)
{
    S strRet;
    uint8_t i = 0;
    unsigned char char_array_3[3];
    unsigned char chr = 0;

    while (len--) {
        char_array_3[i++] = *(pStr++);
        if (i == 3) {
            chr = char_array_3[0] >> 2;
            strRet.push_back(pszBase[chr]);
            chr = ((char_array_3[0] & 0x03) << 4) + (char_array_3[1] >> 4);
            strRet.push_back(pszBase[chr]);
            chr = ((char_array_3[1] & 0x0f) << 2) + (char_array_3[2] >> 6);
            strRet.push_back(pszBase[chr]);
            chr = char_array_3[2] & 0x3f;
            strRet.push_back(pszBase[chr]);

			i = 0;
		}
	}
    if (i) {
        for (uint8_t j = i; j < 3; j++) char_array_3[j] = 0;

        unsigned char char_array_4[4];
        char_array_4[0] = char_array_3[0] >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + (char_array_3[1] >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + (char_array_3[2] >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;
        for (uint8_t j = 0; j < i + 1; j++) strRet.push_back(pszBase[char_array_4[j]]);

        if (chrTail != '\0') {
            while ((i++ < 3)) strRet.push_back(chrTail);
        }
	}

	return strRet;
}

template <class S=string>
static S _base64_decode(const char *pStr, size_t len, const string& strBase = g_strBase64, char chrTail = 0)
{
    S strRet;
    uint8_t i = 0;
    int in_ = 0;
    unsigned char char_array_4[4];
    unsigned char chr = 0;

    while (len-- && (pStr[in_] != chrTail) && strBase.find(pStr[in_]) != __npos) {
        char_array_4[i++] = pStr[in_]; in_++;
        if (i == 4) {
            for (i = 0; i <4; i++) char_array_4[i] = (unsigned char)strBase.find(char_array_4[i]);

            chr = (char_array_4[0] << 2) + (char_array_4[1] >> 4);
            strRet.push_back(chr);
            chr = (char_array_4[1] << 4) + (char_array_4[2] >> 2);
            strRet.push_back(chr);
            chr = (char_array_4[2] << 6) + char_array_4[3];
            strRet.push_back(chr);

            i = 0;
        }
    }
    if (i) {
        for (uint8_t j = i; j < 4; j++) char_array_4[j] = 0;
        for (uint8_t j = 0; j < 4; j++) char_array_4[j] = (unsigned char)strBase.find(char_array_4[j]);

        unsigned char char_array_3[3];
        char_array_3[0] = (char_array_4[0] << 2) + (char_array_4[1] >> 4);
        char_array_3[1] = (char_array_4[1] << 4) + (char_array_4[2] >> 2);
        char_array_3[2] = (char_array_4[2] << 6) + char_array_4[3];
        for (uint8_t j = 0; j < i - 1; j++) strRet.push_back(char_array_3[j]);
    }

    return strRet;
}

string strutil::base64_encode(const char *pStr, size_t len, const char *pszBase, char chrTail)
{
	if (pszBase)
    {
        return _base64_encode(pStr, len, pszBase, chrTail);
	}
	else
	{
        return _base64_encode(pStr, len);
	}
}

string strutil::base64_decode(const char *pStr, size_t len, const char *pszBase, char chrTail)
{
	if (pszBase)
	{
        string strBase(pszBase, 64);
        return _base64_decode(pStr, len, strBase, chrTail);
	}
	else
	{
		return _base64_decode(pStr, len);
	}
}

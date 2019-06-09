
#pragma once

#include <string>

#ifndef _MSC_VER
#include <QString>
#define __QStr(wstr) QString::fromStdWString(wstr)

#ifndef QT_NO_DEBUG
#define _DEBUG
#endif
#endif

class __UtilExt wsutil
{
public:
	static const wchar_t wcSpace = L' ';

	static bool checkWChar(const wstring& str);

	template <typename T>
	inline static wstring fromNum(const T& num)
	{
#ifdef __ANDROID__
		return QString::number(num).toStdWString();
#else
		return std::to_wstring(num);
#endif
	}

	static void trim(wstring& strText, wchar_t chr = ' ');
	static wstring trim_r(const wstring& strText, wchar_t chr = ' ');

	static void ltrim(wstring& strText, wchar_t chr = ' ');
	static wstring ltrim_r(const wstring& strText, wchar_t chr = ' ');

	static void rtrim(wstring& strText, wchar_t chr = ' ');
	static wstring rtrim_r(const wstring& strText, wchar_t chr = ' ');
	
	static void split(const wstring& strText, wchar_t wcSplitor, vector<wstring>& vecRet, bool bTrim=false);

	static int compareUseCNCollate(const wstring& lhs, const wstring& rhs);
	static int compareIgnoreCase(const wstring& str1, const wstring& str2, size_t size=0);
	static bool matchIgnoreCase(const wstring& str1, const wstring& str2);
	
	//static int findIgnoreCase(const wstring& str, const wstring& strToFind);

	static void lowerCase(wstring& str);
	static wstring lowerCase_r(const wstring& str);
	
	static void upperCase(wstring& str);
	static wstring upperCase_r(const wstring& str);

	static void replaceChar(wstring& str, wchar_t chrFind, wchar_t chrReplace);
	static void replaceChars(wstring& str, const wstring& strFindChars, wchar_t chrReplace);

	static wstring fromUTF8(const string& str);
	static string toUTF8(const wstring& str);

	static string toStr(const wstring& str);
	static wstring fromStr(const string& str, bool bCheckUTF8=false);

	template <typename T>
	static wstring ContainerToStr(const T& container, const wstring& strSplitor)
	{
		wstringstream ssResult;
        for (typename T::const_iterator it = container.begin(); ; )
		{
			ssResult << *it;
			
			it++;
			__EnsureBreak(it != container.end());

			ssResult << strSplitor;
		}

		return ssResult.str();
	}
	
	struct tagCNSortor
	{
		bool operator()(const wstring& lhs, const wstring& rhs) const
		{
			return wsutil::compareUseCNCollate(lhs, rhs)<0;
		}
	};
};
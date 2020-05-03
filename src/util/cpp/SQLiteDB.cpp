
#include "util.h"

#include "../sqlite/sqlite3.h"

//CSQLiteDBResult

CSQLiteDBResult::~CSQLiteDBResult()
{
	if (m_pData)
	{
		sqlite3_free_table(m_pData);
	}
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, wstring& strData)
{
	__EnsureReturn(uRow < m_uRowCount && uColumn < m_uColumnCount, false);
	__EnsureReturn(m_pData, false);

	char *lpData = m_pData[(uRow + 1) * m_uColumnCount + uColumn];
	if (NULL != lpData)
	{
		strData = strutil::fromUtf8(lpData);
	}

	return true;
}

inline bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, string& strData)
{
	__EnsureReturn(uRow < m_uRowCount && uColumn < m_uColumnCount, false);
	__EnsureReturn(m_pData, false);

	char *lpData = m_pData[(uRow + 1) * m_uColumnCount + uColumn];
	if (NULL != lpData)
	{
		strData = lpData;
	}

	return true;
}

inline bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, int& nValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), false);

	nValue = atoi(strData.c_str());

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, UINT& uValue)
{
	int nValue = 0;
	__EnsureReturn(GetData(uRow, uColumn, nValue), false);

	uValue = (UINT)nValue;

	return true;
}

inline bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, long& nValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), false);

	nValue = atol(strData.c_str());

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, unsigned long& nValue)
{
	long long t_nValue = 0;
	__EnsureReturn(GetData(uRow, uColumn, t_nValue), false);

	nValue = (unsigned long)t_nValue;

	return true;
}

inline bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, long long& nValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), false);

	nValue = atoll(strData.c_str());

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, unsigned long long& nValue)
{
	long long t_nValue = 0;
	__EnsureReturn(GetData(uRow, uColumn, t_nValue), false);

	nValue = (unsigned long long)t_nValue;

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, double& dbValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), false);

	dbValue = atof(strData.c_str());

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, bool& bValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), false);

	bValue = "1" == strData;

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<wstring>& arrData)
{
	__EnsureReturn(uRow < m_uRowCount, false);
	__EnsureReturn(m_pData, false);

	for (UINT uColumn = 0; uColumn < m_uColumnCount; uColumn++)
	{
		char *lpData = m_pData[(uRow + 1) * m_uColumnCount + uColumn];
		if (NULL != lpData)
		{
			arrData.add(strutil::fromUtf8(lpData));
		}
		else
		{
			arrData.add(L"");
		}
	}

	return true;
}

bool CSQLiteDBResult::_getData(UINT uRow, cfn_void_t<const string&> cb)
{
	__EnsureReturn(uRow < m_uRowCount, false);
	__EnsureReturn(m_pData, false);

	for (UINT uColumn = 0; uColumn < m_uColumnCount; uColumn++)
	{
		char *lpData = m_pData[(uRow + 1) * m_uColumnCount + uColumn];
		if (NULL != lpData)
		{
			cb(lpData);
		}
		else
		{
			cb("");
		}
	}

	return true;
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<string>& arrData)
{
	return _getData(uRow, [&](const string& strData) {
		arrData.add(strData);
	});
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<int>& arrValue)
{
	return _getData(uRow, [&](const string& strData) {
		arrValue.add(atoi(strData.c_str()));
	});
}

bool CSQLiteDBResult::GetData(UINT uRow, SArray<UINT>& arrValue)
{
	return _getData(uRow, [&](const string& strData) {
		arrValue.add((UINT)atoi(strData.c_str()));
	});
}
bool CSQLiteDBResult::GetData(UINT uRow, SArray<double>& arrValue)
{
	return _getData(uRow, [&](const string& strData) {
		arrValue.add(atof(strData.c_str()));
	});
}


//CSQLiteDB

bool CSQLiteDB::Connect(cwstr strFile)
{
    if (m_hDB)
    {
        return false;
    }

    m_strFile.clear();

#if __windows
    m_nRetCode = sqlite3_open16(strFile.c_str(), (sqlite3**)&m_hDB);
#else
    m_nRetCode = sqlite3_open(strutil::toUtf8(strFile).c_str(), (sqlite3**)&m_hDB);
#endif

    __EnsureReturn(SQLITE_OK == m_nRetCode, false);
	__EnsureReturn(m_hDB, false);

    m_strFile = strFile;

	return true;
}

bool CSQLiteDB::Disconnect()
{
	__EnsureReturn(m_hDB, false);

    m_nRetCode = sqlite3_close((sqlite3*)m_hDB);
    __EnsureReturn(SQLITE_OK == m_nRetCode, false);

	m_hDB = NULL;

	return true;
}

bool CSQLiteDB::Execute(const string& strSql)
{
	__EnsureReturn(m_hDB, false);

	char *pszError = NULL;
    m_nRetCode = sqlite3_exec((sqlite3*)m_hDB, strSql.c_str(), 0, 0, &pszError);
	if (pszError)
	{
		m_strError = pszError;
	}
	else
	{
		m_strError.clear();
	}

    return SQLITE_OK == m_nRetCode;
}

bool CSQLiteDB::Execute(cwstr strSql)
{
	return Execute(strutil::toUtf8(strSql));
}

IDBResult* CSQLiteDB::Query(const string& strSql)
{
	__EnsureReturn(m_hDB, NULL);

	char ** pData = NULL;

	int nColumnCount = 0;
	int nRowCount = 0;

	char *pszError = NULL;
    m_nRetCode = sqlite3_get_table((sqlite3*)m_hDB, strSql.c_str(), &pData
		, &nRowCount, &nColumnCount, &pszError);
	if (pszError)
	{
		m_strError = pszError;
	}
	else
	{
		m_strError.clear();
	}

    __EnsureReturn(SQLITE_OK == m_nRetCode && pData, NULL);
	
	return new CSQLiteDBResult((UINT)nColumnCount, (UINT)nRowCount, pData);;
}

IDBResult* CSQLiteDB::Query(cwstr strSql)
{
	return Query(strutil::toUtf8(strSql));
}

bool CSQLiteDB::BeginTrans()
{
	__EnsureReturn(!m_bInTrans, false);
	
	__EnsureReturn(Execute(L"begin Transaction"), false);

	m_bInTrans = true;

	return true;
}

bool CSQLiteDB::CommitTrans()
{
	if (m_bInTrans)
	{
		__EnsureReturn(Execute(L"commit Transaction"), false);

		m_bInTrans = false;
	}

	return true;
}

bool CSQLiteDB::RollbakTrans()
{
	if (m_bInTrans)
	{
		__EnsureReturn(Execute(L"rollback Transaction"), false);

		m_bInTrans = false;
	}

	return true;
}

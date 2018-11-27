
#include <util.h>

#include <SQLiteDB.h>

#include "sqlite/sqlite3.h"


//CSQLiteDBResult

CSQLiteDBResult::CSQLiteDBResult()
{
	m_uColumnCount = 0;
	m_uRowCount = 0;

	m_pData = NULL;
}

CSQLiteDBResult::~CSQLiteDBResult()
{
	__Ensure(this);

	if (m_pData)
	{
		sqlite3_free_table(m_pData);
	}

	delete this;
}

UINT CSQLiteDBResult::GetColumnCount()
{
	__EnsureReturn(this, 0);

	return m_uColumnCount;
}

UINT CSQLiteDBResult::GetRowCount()
{
	__EnsureReturn(this, 0);

	return m_uRowCount;
}

BOOL CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, int& nValue)
{
	string strValue;
	__EnsureReturn(GetData(uRow, uColumn, strValue), FALSE);

	nValue = atoi(strValue.c_str());

	return TRUE;
}

BOOL CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, double& dbValue)
{
	string strValue;
	__EnsureReturn(GetData(uRow, uColumn, strValue), FALSE);

	dbValue = atof(strValue.c_str());

	return TRUE;
}

BOOL CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, string& strValue)
{
	__EnsureReturn(this, NULL);
	__EnsureReturn(m_uRowCount && m_uColumnCount, NULL);
	__EnsureReturn(uRow < m_uRowCount && uColumn < m_uColumnCount, NULL);
	__EnsureReturn(m_pData, FALSE);

	char* pszValue = m_pData[(uRow + 1) * m_uColumnCount + uColumn];
	if (NULL != pszValue)
	{
		strValue = pszValue;
	}

	return TRUE;
}

BOOL CSQLiteDBResult::GetData(UINT uRow, UINT uColumn, wstring& strValue)
{
	string strData;
	__EnsureReturn(GetData(uRow, uColumn, strData), FALSE);
	strValue = util::StrToWStr(strData, CP_UTF8);
	return TRUE;
}


//CSQLiteDB

CSQLiteDB::CSQLiteDB(const string& strDBPath)
	: m_strDBPath(strDBPath)
{
}

CSQLiteDB::~CSQLiteDB()
{
	Disconnect();
}

int CSQLiteDB::GetStatus()
{
	return (NULL != m_hDB);
}

BOOL CSQLiteDB::Connect(const string& strPara)
{
	__EnsureReturn(!m_hDB, FALSE);

	string strDBPath = !strPara.empty()?strPara:m_strDBPath;
	
	__EnsureReturn(SQLITE_OK == sqlite3_open(strDBPath.c_str(), (sqlite3**)&m_hDB), FALSE);
	__EnsureReturn(m_hDB, FALSE);

	return TRUE;
}

BOOL CSQLiteDB::Disconnect()
{
	__EnsureReturn(m_hDB, FALSE);

	__EnsureReturn(SQLITE_OK == sqlite3_close((sqlite3*)m_hDB), FALSE);

	m_hDB = NULL;

	return TRUE;
}

BOOL CSQLiteDB::Execute(const wstring& strSql)
{
	__EnsureReturn(m_hDB, FALSE);
	
	char *pszError = NULL;
	if (SQLITE_OK != sqlite3_exec((sqlite3*)m_hDB, util::WStrToStr(strSql, CP_UTF8).c_str(), 0, 0, &pszError))
	{
		if (NULL != pszError)
		{
			m_strError = pszError;
		}
		
		return FALSE;
	}

	return TRUE;
}

IDBResult* CSQLiteDB::Query(const wstring& strSql)
{
	__EnsureReturn(m_hDB, NULL);

	char ** pData = NULL;

	int nColumnCount = 0;
	int nRowCount = 0;

	char *pszError = NULL;
	int nResult = sqlite3_get_table((sqlite3*)m_hDB, util::WStrToStr(strSql, CP_UTF8).c_str(), &pData
		, &nRowCount, &nColumnCount, &pszError);

	if (pszError)
	{
		m_strError = pszError;
	}

	__EnsureReturn(SQLITE_OK == nResult && pData, NULL);
	
	CSQLiteDBResult* pSQLiteDBResult = new CSQLiteDBResult;

	pSQLiteDBResult->m_uColumnCount = (UINT)nColumnCount;
	pSQLiteDBResult->m_uRowCount = (UINT)nRowCount;

	pSQLiteDBResult->m_pData = pData;

	return pSQLiteDBResult;
}

bool CSQLiteDB::BeginTrans()
{
	return Execute(L"begin Transaction");
}

bool CSQLiteDB::CommitTrans()
{
	return Execute(L"commit Transaction");
}

bool CSQLiteDB::RollbakTrans()
{
	return Execute(L"rollback Transaction");
}

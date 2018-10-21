
#include "stdafx.h"

#include <SQLiteDB.h>

#include "sqlite/sqlite3.h"


//CSQLiteDBResult

CSQLiteDBResult::CSQLiteDBResult()
{
	m_nColumnCount = 0;
	m_nRowCount = 0;

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
	__AssertReturn(this, 0);

	return m_nColumnCount;
}

UINT CSQLiteDBResult::GetRowCount()
{
	__AssertReturn(this, 0);

	return m_nRowCount;
}

BOOL CSQLiteDBResult::GetData(UINT nRow, UINT nColumn, int& nValue)
{
	string strValue;
	__EnsureReturn(GetData(nRow, nColumn, strValue), FALSE);

	nValue = atoi(strValue.c_str());

	return TRUE;
}

BOOL CSQLiteDBResult::GetData(UINT nRow, UINT nColumn, double& dbValue)
{
	string strValue;
	__EnsureReturn(GetData(nRow, nColumn, strValue), FALSE);

	dbValue = atof(strValue.c_str());

	return TRUE;
}

BOOL CSQLiteDBResult::GetData(UINT nRow, UINT nColumn, string& strValue)
{
	__AssertReturn(this, NULL);
	__AssertReturn(m_nRowCount && m_nColumnCount, NULL);
	__AssertReturn(nRow < m_nRowCount && nColumn < m_nColumnCount, NULL);
	__AssertReturn(m_pData, FALSE);

	char* pszValue = m_pData[(nRow + 1) * m_nColumnCount + nColumn];
	if (NULL != pszValue)
	{
		strValue = pszValue;
	}

	return TRUE;
}

BOOL CSQLiteDBResult::GetData(UINT nRow, UINT nColumn, wstring& strValue)
{
	string strData;
	__EnsureReturn(GetData(nRow, nColumn, strData), FALSE);
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
	__AssertReturn(!m_hDB, FALSE);

	string strDBPath = !strPara.empty()?strPara:m_strDBPath;
	
	__AssertReturn(SQLITE_OK == sqlite3_open(strDBPath.c_str(), (sqlite3**)&m_hDB), FALSE);
	__AssertReturn(m_hDB, FALSE);

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
	__AssertReturn(m_hDB, FALSE);
	
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
	__AssertReturn(m_hDB, NULL);

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

	__AssertReturn(SQLITE_OK == nResult && pData, NULL);
	
	CSQLiteDBResult* pSQLiteDBResult = new CSQLiteDBResult;

	pSQLiteDBResult->m_nColumnCount = (UINT)nColumnCount;
	pSQLiteDBResult->m_nRowCount = (UINT)nRowCount;

	pSQLiteDBResult->m_pData = pData;

	return pSQLiteDBResult;
}

bool CSQLiteDB::BeginTrans()
{
	return Execute(L"begin Transaction");
}

bool CSQLiteDB::RollbakTrans()
{
	return Execute(L"commit Transaction");
}

bool CSQLiteDB::CommitTrans()
{
	return Execute(L"rollback Transaction");
}

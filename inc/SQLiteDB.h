#pragma once

#include "IDB.h"

class CSQLiteDB;
class __CommonPrjExt CSQLiteDBResult : public IDBResult
{
friend CSQLiteDB;
public:
	CSQLiteDBResult();

	virtual ~CSQLiteDBResult();

private:
	UINT m_nColumnCount;
	UINT m_nRowCount;

	char ** m_pData;

public:
	UINT GetColumnCount();

	UINT GetRowCount();

	BOOL GetData(UINT nRow, UINT nColumn, int& nValue);

	BOOL GetData(UINT nRow, UINT nColumn, double& dbValue);

	BOOL GetData(UINT nRow, UINT nColumn, string& strValue);
	BOOL GetData(UINT nRow, UINT nColumn, wstring& strValue);
};


class __CommonPrjExt CSQLiteDB : public IDB
{
public:
	CSQLiteDB(const string& strDBPath="");

	virtual ~CSQLiteDB();

private:
	string m_strDBPath;
	
	HANDLE m_hDB = NULL;

	string m_strError;

public:
	int GetStatus() override;

	BOOL Connect(const string& strPara="") override;

	BOOL Disconnect() override;

	BOOL Execute(const wstring& strSql) override;

	IDBResult* Query(const wstring& strSql) override;

	bool BeginTrans() override;
	bool RollbakTrans() override;
	bool CommitTrans() override;
};

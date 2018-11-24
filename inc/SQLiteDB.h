#pragma once

#include "IDB.h"

class CSQLiteDB;
class __CommonExt CSQLiteDBResult : public IDBResult
{
friend CSQLiteDB;
public:
	CSQLiteDBResult();

	virtual ~CSQLiteDBResult();

private:
	UINT m_uColumnCount;
	UINT m_uRowCount;

	char ** m_pData;

public:
	UINT GetColumnCount();

	UINT GetRowCount();

	BOOL GetData(UINT uRow, UINT uColumn, int& nValue);

	BOOL GetData(UINT uRow, UINT uColumn, double& dbValue);

	BOOL GetData(UINT uRow, UINT uColumn, string& strValue);
	BOOL GetData(UINT uRow, UINT uColumn, wstring& strValue);
};


class __CommonExt CSQLiteDB : public IDB
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
	bool CommitTrans() override;
	bool RollbakTrans() override;
};

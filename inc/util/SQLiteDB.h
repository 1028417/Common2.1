#pragma once

#include "IDB.h"

class CSQLiteDB;
class __UtilExt CSQLiteDBResult : public IDBResult
{
friend CSQLiteDB;
public:
	CSQLiteDBResult() {}

	~CSQLiteDBResult();

private:
	UINT m_uColumnCount = 0;
	UINT m_uRowCount = 0;

	char ** m_pData = NULL;

public:
	const UINT& GetColumnCount() override
	{
		return m_uColumnCount;
	}

	const UINT& GetRowCount() override
	{
		return m_uRowCount;
	}

	bool GetData(UINT uRow, UINT uColumn, string& strData) override;
	bool GetData(UINT uRow, UINT uColumn, wstring& strData) override;
	bool GetData(UINT uRow, UINT uColumn, int& nValue) override;
	bool GetData(UINT uRow, UINT uColumn, double& dbValue) override;

	bool GetData(UINT uRow, SArray<string>& arrData) override;
	bool GetData(UINT uRow, SArray<wstring>& arrData) override;
	bool GetData(UINT uRow, SArray<int>& arrValue) override;
	bool GetData(UINT uRow, SArray<double>& arrValue) override;
	
private:
	bool _getData(UINT uRow, const function<void(const string&)>& cb);
};


class __UtilExt CSQLiteDB : public IDB
{
public:
	CSQLiteDB(const string& strDBPath="");

	~CSQLiteDB();

private:
	string m_strDBPath;
	
	void *m_hDB = NULL;

	string m_strError;
	
	bool m_bInTrans = false;

public:
	int GetStatus() override;

	string& GetLastError()
	{
		return m_strError;
	}

	bool Connect(const string& strPara="") override;

	bool Disconnect() override;

	bool Execute(const string& strSql) override;
	bool Execute(const wstring& strSql) override;

	IDBResult* Query(const string& strSql) override;
	IDBResult* Query(const wstring& strSql) override;

	bool BeginTrans() override;
	bool CommitTrans() override;
	bool RollbakTrans() override;
};

#pragma once

#include <string>
using namespace std;

interface IDBResult
{
	virtual UINT GetColumnCount() = 0;
	virtual UINT GetRowCount() = 0;

	virtual BOOL GetData(UINT nRow, UINT nColumn, int& nValue) = 0;
	virtual BOOL GetData(UINT nRow, UINT nColumn, double& dbValue) = 0;
	virtual BOOL GetData(UINT nRow, UINT nColumn, string& strValue) = 0;
	virtual BOOL GetData(UINT nRow, UINT nColumn, wstring& strValue) = 0;
};

interface IDB
{
	virtual int GetStatus() = 0;

	virtual BOOL Connect(const string& strPara) = 0;
	virtual BOOL Disconnect() = 0;

	virtual BOOL Execute(const wstring& strSql, string& strError) = 0;
	virtual IDBResult* Query(const wstring& strSql, string& strError) = 0;
};

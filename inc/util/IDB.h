#pragma once

#include <string>
using namespace std;

typedef unsigned int UINT;
typedef int BOOL;

#include <sstl/sstl.h>
using namespace NS_SSTL;

interface IDBResult
{
	virtual const UINT& GetColumnCount() = 0;
	virtual const UINT& GetRowCount() = 0;

	virtual BOOL GetData(UINT uRow, UINT uColumn, int& nValue) = 0;
	virtual BOOL GetData(UINT uRow, UINT uColumn, double& dbValue) = 0;
	virtual BOOL GetData(UINT uRow, UINT uColumn, string& strValue) = 0;
	virtual BOOL GetData(UINT uRow, UINT uColumn, wstring& strValue) = 0;
	
	virtual BOOL GetData(UINT uRow, SArray<string>& arrData) = 0;
	virtual BOOL GetData(UINT uRow, SArray<wstring>& arrData) = 0;
	virtual BOOL GetData(UINT uRow, SArray<int>& arrValue) = 0;
	virtual BOOL GetData(UINT uRow, SArray<double>& arrValue) = 0;
};

interface IDB
{
	virtual int GetStatus() = 0;

	virtual BOOL Connect(const string& strPara="") = 0;
	virtual BOOL Disconnect() = 0;

	virtual BOOL Execute(const wstring& strSql) = 0;
	virtual IDBResult* Query(const wstring& strSql) = 0;

	virtual bool BeginTrans() = 0;
	virtual bool RollbakTrans() = 0;
	virtual bool CommitTrans() = 0;
};

class __UtilExt CDBTransGuard
{
private:
	IDB *m_pDB = NULL;

	bool _BeginTrans(IDB& db)
	{
		try
		{
			return db.BeginTrans();
		}
		catch (...)
		{
			return false;
		}
	}

public:
	CDBTransGuard(IDB& db)
	{
		if (_BeginTrans(db))
		{
			m_pDB = &db;
		}
	}

	CDBTransGuard(IDB *pDB)
	{
		if (NULL != pDB)
		{
			if (_BeginTrans(*pDB))
			{
				m_pDB = pDB;
			}
		}
	}

	~CDBTransGuard()
	{
		(void)Commit();
	}

	bool Commit()
	{
		if (NULL == m_pDB)
		{
			return false;
		}

		auto pDB = m_pDB;
		m_pDB = NULL;

		try
		{
			return pDB->CommitTrans();
		}
		catch (...)
		{
			return false;
		}
	}

	bool Rollback()
	{
		if (NULL == m_pDB)
		{
			return false;
		}

		auto pDB = m_pDB;
		m_pDB = NULL;

		try
		{
			return pDB->RollbakTrans();
		}
		catch (...)
		{
			return false;
		}
	}
};

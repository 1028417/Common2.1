#pragma once

#include <string>
using namespace std;

interface IDBResult
{
	virtual UINT GetColumnCount() = 0;
	virtual UINT GetRowCount() = 0;

	virtual BOOL GetData(UINT uRow, UINT uColumn, int& nValue) = 0;
	virtual BOOL GetData(UINT uRow, UINT uColumn, double& dbValue) = 0;
	virtual BOOL GetData(UINT uRow, UINT uColumn, string& strValue) = 0;
	virtual BOOL GetData(UINT uRow, UINT uColumn, wstring& strValue) = 0;
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

class __CommonExt CDBTransGuide
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
	CDBTransGuide(IDB& db)
	{
		if (_BeginTrans(db))
		{
			m_pDB = &db;
		}
	}

	CDBTransGuide(IDB *pDB)
	{
		if (NULL != pDB)
		{
			if (_BeginTrans(*pDB))
			{
				m_pDB = pDB;
			}
		}
	}

	~CDBTransGuide()
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

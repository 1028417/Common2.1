
#pragma once

// CLogList

class __CommonExt CLogList : public CListCtrl
{
	DECLARE_DYNAMIC(CLogList)

public:
	CLogList();
	virtual ~CLogList();

protected:
	DECLARE_MESSAGE_MAP()

public:
	void InitCtrl(const vector<pair<CString, LONG>>& vctColumns);

	void AddLog(const vector<CString>& vctLogTexts);

	void ClearLogs();
};

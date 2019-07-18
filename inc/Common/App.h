
#pragma once

#include <gdiplus.h>
using namespace Gdiplus;

#include "ModuleApp.h"

using CB_Sync = fn_voidvoid;

class IView
{
public:
	IView() {}

	virtual ~IView() {}

public:
	virtual CMainWnd* init() = 0;

	virtual bool handleCommand(UINT uID)
	{
		return false;
	}

	virtual bool handleHotkey(const tagHotkeyInfo& HotkeyInfo)
	{
		return false;
	}

	virtual void close() {}
};

class IController
{
public:
	IController(){}

public:
	virtual bool init()
	{
		return true;
	}

	virtual bool start()
	{
		return true;
	}

	virtual bool handleCommand(UINT uID)
	{
		return false;
	}

	virtual bool handleHotkey(const tagHotkeyInfo& HotkeyInfo)
	{
		return false;
	}

	virtual LRESULT handleModuleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	}

	virtual void stop()
	{
	}
};

enum class E_DoEventsResult
{
	DER_None
	, DER_OK
	, DER_Quit
};

class __CommonExt CMainApp : public CModuleApp, public IController
{
public:
	static CMainApp* GetMainApp()
	{
		return (CMainApp*)AfxGetApp();
	}

	virtual IView& getView() = 0;

	virtual IController& getController()
	{
		return *this;
	}
	
private:
	typedef vector<CModuleApp*> ModuleVector;
	ModuleVector m_vctModules;

protected:
	virtual BOOL InitInstance() override;
	
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

private:
	static BOOL _RegGlobalHotkey(HWND hWnd, const tagHotkeyInfo &HotkeyInfo);
	
	bool _HandleHotkey(LPARAM lParam);
	bool _HandleHotkey(tagHotkeyInfo &HotkeyInfo);

	BOOL OnCommand(UINT uID);

public:
	void Quit();

	static BOOL AddModule(CModuleApp& Module);

	static bool removeMsg(UINT uMsg);

	static void async(const CB_Sync& cb, UINT uDelayTime=0);

	static void sync(const CB_Sync& cb, bool bBlock=true);

	template <typename T>
	static void sync(const function<void(T&)>& cb, T& para, bool bBlock = true)
	{
		sync([=]() {
			cb(para);
		}, bBlock);
	}
	
	void thread(const fn_voidvoid& cb);

	E_DoEventsResult DoEvents(bool bOnce=false);

	static int msgBox(const wstring& strMsg, const wstring& strTitle, UINT nType, CWnd *pWnd);
	
	static void showTipMsg(const wstring& strMsg, const wstring& strTitle, CWnd *pWnd = NULL)
	{
		(void)msgBox(strMsg, strTitle, MB_OK, pWnd);
	}

	static void showTipMsg(const wstring& strMsg, CWnd *pWnd = NULL)
	{
		showTipMsg(strMsg, L"��ʾ", pWnd);
	}

	static void showTipMsg(const wstring& strMsg, class CPage& wndPage);

	static bool showWarnMsg(const wstring& strMsg, const wstring& strTitle, CWnd *pWnd = NULL)
	{
		return IDYES == msgBox(strMsg, strTitle, MB_YESNO, pWnd);
	}

	static bool showWarnMsg(const wstring& strMsg, CWnd *pWnd = NULL)
	{
		return showWarnMsg(strMsg, L"����", pWnd);
	}

	static bool showWarnMsg(const wstring& strMsg, class CPage& wndPage);
	
	static const CRect& getWorkArea(bool bFullScreen);
	
	static bool getKeyState(UINT uKey)
	{
		return ::GetKeyState(uKey) < 0;
	}

	static BOOL RegHotkey(const tagHotkeyInfo &HotkeyInfo);

	static BOOL RegisterInterface(UINT uIndex, LPVOID lpInterface);
	static LPVOID GetInterface(UINT uIndex);

	static LRESULT SendModuleMessage(UINT uMsg, WPARAM wParam=0, LPARAM lParam=0);

	static LRESULT SendModuleMessage(UINT uMsg, LPVOID pPara)
	{
		return CMainApp::SendModuleMessage(uMsg, (WPARAM)pPara);
	}

	template <typename _T1, typename _T2>
	static LRESULT SendModuleMessage(UINT uMsg, _T1 para1, _T2 para2)
	{
		return CMainApp::SendModuleMessage(uMsg, (WPARAM)para1, (LPARAM)para2);
	}

	static void BroadcastModuleMessage(UINT uMsg, WPARAM wParam=0, LPARAM lParam=0);

	static void BroadcastModuleMessage(UINT uMsg, LPVOID pPara)
	{
		BroadcastModuleMessage(uMsg, (WPARAM)pPara);
	}

	template <typename _T1, typename _T2>
	static void BroadcastModuleMessage(UINT uMsg, _T1 para1, _T2 para2)
	{
		BroadcastModuleMessage(uMsg, (WPARAM)para1, (LPARAM)para2);
	}
};
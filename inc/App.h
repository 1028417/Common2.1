
#pragma once

#include <ModuleApp.h>

using CB_Timer = function<bool()>;

using CB_Async = fn_voidvoid;

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
	IController(IView& view)
		: m_view(view)
	{
	}

private:
	IView& m_view;

public:
	IView& getView()
	{
		return m_view;
	}

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

	virtual LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

class __CommonExt CMainApp : public CModuleApp
{
public:
	static CMainApp* GetMainApp()
	{
		return (CMainApp*)AfxGetApp();
	}

	virtual IView& getView() = 0;
	virtual IController& getController() = 0;

	wstring GetAppPath(const wstring& strPath)
	{
		return m_strAppPath + strPath;
	}

private:
	wstring m_strAppPath;

	typedef vector<CModuleApp*> ModuleVector;
	ModuleVector m_vctModules;

protected:
	virtual BOOL InitInstance() override;
	
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

private:
	bool _HandleHotkey(LPARAM lParam);
	bool _HandleHotkey(tagHotkeyInfo &HotkeyInfo);

	BOOL OnCommand(UINT uID);

public:
	void Quit();

	static BOOL AddModule(CModuleApp& Module);

	static E_DoEventsResult DoEvents(bool bOnce=false);

	static bool peekMsg(UINT uMsg, MSG msg, bool bPeekAll);

	static UINT_PTR setTimer(UINT uElapse, const CB_Timer& cb);
	static void killTimer(UINT_PTR idEvent);

	static void async(const CB_Async& cb, UINT uDelayTime=0);

	static void sync(const CB_Sync& cb);

	static bool getKeyState(UINT uKey)
	{
		return ::GetKeyState(uKey) < 0;
	}

	static BOOL RegHotkey(const tagHotkeyInfo &HotkeyInfo, bool bGlobal=false);

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

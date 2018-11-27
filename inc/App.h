
#pragma once

// Hotkey
enum class E_HotkeyFlag
{
	HKF_Null = 0

	, HKF_Alt = MOD_ALT
	, HKF_Control = MOD_CONTROL
	, HKF_Shift = MOD_SHIFT

	, HKF_AltControl = MOD_ALT | MOD_CONTROL
	, HKF_AltShift = MOD_ALT | MOD_SHIFT
	, HKF_ControlShift = MOD_CONTROL | MOD_SHIFT
	, HKF_AltControlShift = MOD_ALT | MOD_CONTROL | MOD_SHIFT
};

struct tagHotkeyInfo
{
	tagHotkeyInfo(UINT t_uKey, E_HotkeyFlag eHotkeyFlag = E_HotkeyFlag::HKF_Null, bool t_bGlobal=FALSE, UINT t_uIDMenuItem = 0)
		: uKey(t_uKey)
		, eFlag(eHotkeyFlag)
		, lParam(MAKELPARAM(eHotkeyFlag, uKey))
		, bGlobal(t_bGlobal)
		, uIDMenuItem(t_uIDMenuItem)
	{
		bHandling = false;
	}

	UINT uKey;
	E_HotkeyFlag eFlag;
	LPARAM lParam;

	bool bGlobal;

	UINT uIDMenuItem;

	bool bHandling;

	bool operator ==(const tagHotkeyInfo &HotkeyInfo)
	{
		return (lParam == HotkeyInfo.lParam && bGlobal == HotkeyInfo.bGlobal);
	}
};

class __CommonExt CResModule
{
public:
	CResModule(HINSTANCE hInstance=NULL)
	{
		_hInstance = hInstance;
	}

	CResModule(const string& strDllName)
	{
		_hInstance = GetModuleHandleA(__DllFile(strDllName).c_str());
	}

private:
	HINSTANCE _hInstance = NULL;

	virtual HINSTANCE GetHInstance()
	{
		return _hInstance;
	}

public:
	void ActivateResource();

	HICON loadIcon(UINT uID);

	HBITMAP loadBitmap(UINT uID);

	HMENU loadMenu(UINT uID);

	LPCDLGTEMPLATE loadDialog(UINT uID);
};

class CMainWnd;

// CModuleApp
class __CommonExt CModuleApp: public CWinApp, public CResModule
{
friend class CMainApp;

public:
	CModuleApp() {}

	virtual ~CModuleApp() {}

	HINSTANCE GetHInstance() override
	{
		return m_hInstance;
	}
	
protected:
	virtual BOOL InitInstance() override;

protected:
	virtual BOOL OnReady(CMainWnd& MainWnd) { return TRUE; }

	virtual BOOL HandleCommand(UINT uID) { return FALSE; }

	virtual BOOL HandleHotkey(const tagHotkeyInfo& HotkeyInfo) { return FALSE; }

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) { return 0; }

	virtual BOOL OnQuit() { return TRUE; }
};

struct tagMainWndInfo;


class IView
{
public:
	IView() {}

	virtual ~IView() {}

public:
	virtual CMainWnd* init() = 0;

	virtual bool show() = 0;

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

class __CommonExt CMainApp: public CModuleApp
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

	map<UINT, LPVOID> m_mapInterfaces;

	vector<tagHotkeyInfo> m_vctHotkeyInfos;

protected:
	virtual BOOL InitInstance() override;
	
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

private:
	bool HandleHotkey(LPARAM lParam, bool bGlobal);
	bool HandleHotkey(tagHotkeyInfo &HotkeyInfo);

	BOOL OnCommand(UINT uID);

public:
	BOOL Quit();

	static void DoEvents(bool bOnce=false);

	static BOOL AddModule(CModuleApp& Module);

	BOOL RegisterInterface(UINT uIndex, LPVOID lpInterface);
	LPVOID GetInterface(UINT uIndex);

	static BOOL RegHotkey(const tagHotkeyInfo &HotkeyInfo);

public:
	static LRESULT SendMessage(UINT uMsg, WPARAM wParam=0, LPARAM lParam=0);

	static LRESULT SendMessage(UINT uMsg, LPVOID pPara)
	{
		return CMainApp::SendMessage(uMsg, (WPARAM)pPara);
	}

	template <typename _T1, typename _T2>
	static LRESULT SendMessage(UINT uMsg, _T1 para1, _T2 para2)
	{
		return CMainApp::SendMessage(uMsg, (WPARAM)para1, (LPARAM)para2);
	}

	static void SendMessageEx(UINT uMsg, WPARAM wParam=0, LPARAM lParam=0);

	static void SendMessageEx(UINT uMsg, LPVOID pPara)
	{
		SendMessageEx(uMsg, (WPARAM)pPara);
	}

	template <typename _T1, typename _T2>
	static void SendMessageEx(UINT uMsg, _T1 para1, _T2 para2)
	{
		SendMessageEx(uMsg, (WPARAM)para1, (LPARAM)para2);
	}
};

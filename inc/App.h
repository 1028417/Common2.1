
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

class CResModule
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
	void ActivateResource()
	{
		HINSTANCE hInstance = GetHInstance();
		__Assert(hInstance);

		AfxSetResourceHandle(hInstance);
	}

	HICON loadIcon(UINT uID)
	{
		HINSTANCE hInstance = GetHInstance();
		__AssertReturn(hInstance, NULL);

		HICON hIcon = ::LoadIcon(hInstance, MAKEINTRESOURCE(uID));
		__AssertReturn(hIcon, NULL);

		return hIcon;
	}

	HBITMAP loadBitmap(UINT uID)
	{
		HINSTANCE hInstance = GetHInstance();
		__AssertReturn(hInstance, NULL);

		HBITMAP hBitmap = ::LoadBitmap(hInstance, MAKEINTRESOURCE(uID));
		__AssertReturn(hBitmap, NULL);
		
		return hBitmap;
	}

	HMENU loadMenu(UINT uID)
	{
		HINSTANCE hInstance = GetHInstance();
		__AssertReturn(hInstance, NULL);

		HMENU hMenu = ::LoadMenu(hInstance, MAKEINTRESOURCE(uID));
		__AssertReturn(hMenu, NULL);

		return hMenu;
	}

	LPCDLGTEMPLATE loadDialog(UINT uID)
	{
		HINSTANCE hInstance = GetHInstance();
		__AssertReturn(hInstance, NULL);

		HRSRC hRes = ::FindResource(hInstance, MAKEINTRESOURCE(uID), RT_DIALOG);
		__AssertReturn(hRes, NULL);
		
		HGLOBAL hGlobal = ::LoadResource(hInstance, hRes);
		__AssertReturn(hGlobal, NULL);
		
		LPCDLGTEMPLATE lpRes = (LPCDLGTEMPLATE)LockResource(hGlobal);
		__AssertReturn(lpRes, NULL);

		return lpRes;
	}
};

class CResGuide
{
public:
	CResGuide(CResModule& resModule)
	{
		m_hPreInstance = AfxGetResourceHandle();
		resModule.ActivateResource();
	}

	CResGuide(CResModule *pResModule)
	{
		if (NULL != pResModule)
		{
			m_hPreInstance = AfxGetResourceHandle();
			pResModule->ActivateResource();
		}
	}

	~CResGuide()
	{
		if (NULL != m_hPreInstance)
		{
			AfxSetResourceHandle(m_hPreInstance);
		}
	}

private:
	HINSTANCE m_hPreInstance = NULL;
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


class IController
{
public:
	IController() {}

	virtual ~IController() {}

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

	virtual LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return 0;
	}

	virtual void stop()
	{
	}
};

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

class __CommonExt CMainApp: public CModuleApp
{
public:
	CMainApp(IView& view, IController& Controller)
		: m_view(view)
		, m_Controller(Controller)
	{
	}

	virtual ~CMainApp() {}

	static CMainApp* GetMainApp()
	{
		return (CMainApp*)AfxGetApp();
	}

	wstring GetAppPath(const wstring& strPath)
	{
		return m_strAppPath + strPath;
	}

private:
	IView& m_view;
	IController& m_Controller;

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

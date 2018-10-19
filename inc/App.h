
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
	tagHotkeyInfo(UINT t_nKey, E_HotkeyFlag eHotkeyFlag = E_HotkeyFlag::HKF_Null, bool t_bGlobal=FALSE, UINT t_nIDMenuItem = 0)
		: nKey(t_nKey)
		, eFlag(eHotkeyFlag)
		, lParam(MAKELPARAM(eHotkeyFlag, nKey))
		, bGlobal(t_bGlobal)
		, nIDMenuItem(t_nIDMenuItem)
	{
		bHandling = false;
	}

	UINT nKey;
	E_HotkeyFlag eFlag;
	LPARAM lParam;

	bool bGlobal;

	UINT nIDMenuItem;

	bool bHandling;

	bool operator ==(const tagHotkeyInfo &HotkeyInfo)
	{
		return (lParam == HotkeyInfo.lParam && bGlobal == HotkeyInfo.bGlobal);
	}
};

class CMainWnd;

enum E_ResourceType
{
	RCT_Icon
	, RCT_Menu
};

class CResourceModule
{
public:
	CResourceModule(const string& strDllName="")
		: m_strDllName(strDllName)
	{
	}

private:
	string m_strDllName;

	virtual HINSTANCE GetHInstance()
	{
		return GetModuleHandleA(__DllFile(m_strDllName).c_str());
	}

public:
	void ActivateResource()
	{
		AfxSetResourceHandle(GetHInstance());
	}

	HANDLE GetResource(E_ResourceType eResourceType, UINT nID)
	{
		ActivateResource();

		switch (eResourceType)
		{
		case RCT_Icon:
			return ::LoadIcon(GetHInstance(), MAKEINTRESOURCE(nID));
		case RCT_Menu:
			return ::LoadMenu(GetHInstance(), MAKEINTRESOURCE(nID));
		default:
			;
		}

		return NULL;
	}
};

// CModuleApp
class __CommonPrjExt CModuleApp: public CWinApp, public CResourceModule
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

	virtual BOOL HandleCommand(UINT nID) { return FALSE; }

	virtual BOOL HandleHotkey(const tagHotkeyInfo& HotkeyInfo) { return FALSE; }

	virtual LRESULT HandleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam) { return 0; }

	virtual BOOL OnQuit() { return TRUE; }
};

class CResourceLock
{
public:
	CResourceLock(CResourceModule& resModule)
	{
		m_hPreInstance = AfxGetResourceHandle();
		resModule.ActivateResource();
	}

	~CResourceLock()
	{
		if (NULL != m_hPreInstance)
		{
			AfxSetResourceHandle(m_hPreInstance);
		}
	}

private:
	HINSTANCE m_hPreInstance = NULL;
};

struct tagMainWndInfo;

//CMainApp
typedef vector<CModuleApp*> ModuleVector;

interface IController
{
	virtual CMainWnd* run() = 0;

	virtual bool handleCommand(UINT nID) = 0;

	virtual bool handleHotkey(const tagHotkeyInfo& HotkeyInfo) = 0;

	virtual LRESULT handleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam) = 0;

	virtual void stop() = 0;
};

class __CommonPrjExt CMainApp: public CModuleApp
{
public:
	CMainApp() {}

	virtual ~CMainApp() {}

	static CMainApp* GetMainApp()
	{
		return (CMainApp*)AfxGetApp();
	}

	static CMainWnd* GetMainWnd()
	{
		return (CMainWnd*)AfxGetMainWnd();
	}

	wstring GetAppPath(const wstring& strPath)
	{
		return m_strAppPath + strPath;
	}

private:
	virtual IController& getController() = 0;

	wstring m_strAppPath;

	ModuleVector m_vctModules;

	map<UINT, LPVOID> m_mapInterfaces;

	vector<tagHotkeyInfo> m_vctHotkeyInfos;

protected:
	virtual BOOL InitInstance() override;
	
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

private:
	bool HandleHotkey(LPARAM lParam, bool bGlobal);
	bool HandleHotkey(tagHotkeyInfo &HotkeyInfo);

	BOOL OnCommand(UINT nID);

public:
	BOOL Quit();

	static void DoEvents(bool bOnce=false);

	static BOOL AddModule(CModuleApp& Module);

	BOOL RegisterInterface(UINT nIndex, LPVOID lpInterface);
	LPVOID GetInterface(UINT nIndex);

	static BOOL RegHotkey(const tagHotkeyInfo &HotkeyInfo);

public:
	static LRESULT SendMessage(UINT nMsg, WPARAM wParam=0, LPARAM lParam=0);

	static LRESULT SendMessage(UINT nMsg, LPVOID pPara)
	{
		return CMainApp::SendMessage(nMsg, (WPARAM)pPara);
	}

	template <typename _T1, typename _T2>
	static LRESULT SendMessage(UINT nMsg, _T1 para1, _T2 para2)
	{
		return CMainApp::SendMessage(nMsg, (WPARAM)para1, (LPARAM)para2);
	}

	static void SendMessageEx(UINT nMsg, WPARAM wParam=0, LPARAM lParam=0);

	static void SendMessageEx(UINT nMsg, LPVOID pPara)
	{
		SendMessageEx(nMsg, (WPARAM)pPara);
	}

	template <typename _T1, typename _T2>
	static void SendMessageEx(UINT nMsg, _T1 para1, _T2 para2)
	{
		SendMessageEx(nMsg, (WPARAM)para1, (LPARAM)para2);
	}
};

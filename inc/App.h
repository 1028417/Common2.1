
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

enum E_ResourceType
{
	RCT_Icon
	, RCT_Menu
};

class CMainWnd;

// IModuleApp
class __CommonPrjExt IModuleApp: public CWinApp
{
friend class CMainApp;

public:
	IModuleApp();

	virtual ~IModuleApp();

public:
	void ActivateResource();
	
	HANDLE GetResource(E_ResourceType eResourceType, UINT nID);
	
	virtual BOOL HandleCommand(UINT nID) { return FALSE; }

	virtual LRESULT HandleMessage(UINT nMsg, WPARAM wParam, LPARAM lParam) { return 0; }

protected:
	virtual BOOL OnReady(CMainWnd& MainWnd) { return TRUE; }

	virtual BOOL HandleHotkey(const tagHotkeyInfo& HotkeyInfo) { return FALSE; }

	virtual BOOL OnQuit() { return TRUE; }
};

class CModuleApp : public IModuleApp {};

class CMainWnd;

class CResourceLock
{
public:
	CResourceLock(IModuleApp& Module)
	{
		m_hPreInstance = AfxGetResourceHandle();
		Module.ActivateResource();
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
typedef vector<IModuleApp*> ModuleVector;

class __CommonPrjExt CMainApp: public IModuleApp
{
public:
	CMainApp() {}
	virtual ~CMainApp() {}

	static CMainWnd* GetMainWnd()
	{
		return _pMainWnd;
	}

private:
	static class CMainWnd *_pMainWnd;

	static ModuleVector m_vctModules;

	static map<UINT, LPVOID> m_mapInterfaces;

	static map<char, LPVOID> m_mapHotkeyInfos;

	static vector<tagHotkeyInfo> m_vctHotkeyInfos;

protected:
	virtual BOOL InitInstance() override;
	
	virtual CMainWnd* OnInitMainWnd(tagMainWndInfo& MainWndInfo)=0;
	
	virtual BOOL PreTranslateMessage(MSG* pMsg) override;

private:
	static bool HandleHotkey(LPARAM lParam, bool bGlobal);
	static bool HandleHotkey(tagHotkeyInfo &HotkeyInfo);

	static BOOL OnCommand(UINT nID);

public:
	static wstring GetAppPath();

	static BOOL Quit();

	static void DoEvents(bool bOnce=false);

	static BOOL AddModule(IModuleApp& Module);

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
		CMainApp::SendMessageEx(nMsg, (WPARAM)pPara);
	}

	template <typename _T1, typename _T2>
	static void SendMessageEx(UINT nMsg, _T1 para1, _T2 para2)
	{
		CMainApp::SendMessageEx(nMsg, (WPARAM)para1, (LPARAM)para2);
	}

	static BOOL RegisterInterface(UINT nIndex, LPVOID lpInterface);
	static LPVOID GetInterface(UINT nIndex);

	static BOOL RegHotkey(const tagHotkeyInfo &HotkeyInfo);
};

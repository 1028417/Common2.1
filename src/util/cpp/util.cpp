
#include "util.h"

E_Platform platform()
{
#if __windows
    return E_Platform::PF_Windows;
#elif __android
    return E_Platform::PF_Android;
#elif __ios
    return E_Platform::PF_IOS;
#else
    return E_Platform::PF_Mac;
#endif
}

#if !__winvc
#include <QSysInfo>
wstring platformType()
{
    return QSysInfo::productType().toStdWString();
}

wstring platformVersion()
{
    return QSysInfo::productVersion().toStdWString();
}
#endif

#if __windows
//#include <ShellScalingApi.h>
#define MDT_EFFECTIVE_DPI 0

static HRESULT WINAPI GetDpiForMonitor(
	_In_ HMONITOR hmonitor,
	_In_ int dpiType,
	_Out_ UINT *dpiX,
	_Out_ UINT *dpiY)
{
	HINSTANCE hInstWinSta = LoadLibraryW(L"SHCore.dll");

	if (hInstWinSta == nullptr) return E_NOINTERFACE;

	typedef HRESULT(WINAPI * PFN_GDFM)(HMONITOR, int, UINT*, UINT*);

	PFN_GDFM pGetDpiForMonitor = (PFN_GDFM)GetProcAddress(hInstWinSta, "GetDpiForMonitor");

	if (pGetDpiForMonitor == nullptr) return E_NOINTERFACE;

	return pGetDpiForMonitor(hmonitor, dpiType, dpiX, dpiY);
}

//#include <wingdi.h>
#define __DPIDefault 96.0f

float getDPIRate()
{
	UINT uDPIX = 0; // = QApplication::primaryScreen()->logicDotsPerInch();
	HRESULT hr = GetDpiForMonitor(MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST), MDT_EFFECTIVE_DPI, &uDPIX, NULL);
	if (S_OK == hr && uDPIX > 0)
	{
		return __DPIDefault / uDPIX;
	}

	HDC hDCDesk = GetDC(NULL);
	int nDPIX = GetDeviceCaps(hDCDesk, LOGPIXELSX);
	if (nDPIX > 0)
	{
		return __DPIDefault / nDPIX;
	}

	return 1;
}

// XMusicHost目前做法是在清单工具设置“每个监视器高 DPI 识别”
// 开启对话框Per-Monitor DPI Aware支持(至少Win10)
//BOOL EnablePerMonitorDialogScaling()
//{
//	typedef BOOL(WINAPI *PFN_EnablePerMonitorDialogScaling)();
//	PFN_EnablePerMonitorDialogScaling pEnablePerMonitorDialogScaling =
//		(PFN_EnablePerMonitorDialogScaling)GetProcAddress(GetModuleHandleA("user32.dll"), (LPCSTR)2577);
//
//	if (pEnablePerMonitorDialogScaling) return pEnablePerMonitorDialogScaling();
//
//	return FALSE;
//}

const RECT& getWorkArea(bool bFullScreen)
{
	static RECT rcWorkArea{ 0,0,0,0 };
	if (bFullScreen)
	{
		rcWorkArea.left = 0;
		rcWorkArea.top = 0;
		rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
		rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	else
	{
		(void)::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWorkArea, 0);
	}

	return rcWorkArea;
}
#endif

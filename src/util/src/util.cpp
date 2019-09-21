
#include "util.h"

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

#define __DPIDefault 96.0f

float getDPIRate()
{
	UINT uDPIX = 0;
	HRESULT hr = GetDpiForMonitor(MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTONEAREST), MDT_EFFECTIVE_DPI, &uDPIX, NULL);
	if (S_OK != hr)
	{
		HDC hDCDesk = GetDC(NULL);
		uDPIX = GetDeviceCaps(hDCDesk, LOGPIXELSX);
	}

	return __DPIDefault / uDPIX;
}
#endif
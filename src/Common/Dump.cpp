
#include "stdafx.h"

#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")

#include <util/util.h>

static inline void CreateMiniDump(PEXCEPTION_POINTERS pep, const wstring& strFileName)
{
	HANDLE hFile = CreateFileW(strFileName.c_str(), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;
		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pep;
		mdei.ClientPointers = NULL;

		MINIDUMP_TYPE dumpType = (MINIDUMP_TYPE)(MiniDumpNormal  //
			| MiniDumpWithFullMemory     // full debug info
			| MiniDumpWithHandleData
			| MiniDumpWithUnloadedModules
			//| MiniDumpWithIndirectlyReferencedMemory
			//| MiniDumpScanMemory
			| MiniDumpWithProcessThreadData
			| MiniDumpWithThreadInfo);

		//MINIDUMP_CALLBACK_INFORMATION mci;
		//mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
		//mci.CallbackParam       = 0;

		//::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);
		::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, dumpType, (pep != 0) ? &mdei : 0, NULL, NULL);

		CloseHandle(hFile);
	}
}

static LONG MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	EXCEPTION_RECORD& exceptionRecord = *pExceptionInfo->ExceptionRecord;
	char pszExceptionInfo[128];
	memset(pszExceptionInfo, 0, sizeof(pszExceptionInfo));
	sprintf_s(pszExceptionInfo, "ExceptionCode=%u, ExceptionFlags=%u, ExceptionAddress=%d, NumberParameters=%u"
		, exceptionRecord.ExceptionCode, exceptionRecord.ExceptionFlags, (int)exceptionRecord.ExceptionAddress, exceptionRecord.NumberParameters);
	
	wstring 	strDumpFile = timeutil::formatTime64(L"pc_crash_%Y%m%d_%H%M%S.dmp");
	CreateMiniDump(pExceptionInfo, strDumpFile);
	
	return EXCEPTION_EXECUTE_HANDLER;
}

// �˺���һ���ɹ����ã�֮��� SetUnhandledExceptionFilter �ĵ��ý���Ч
static void DisableSetUnhandledExceptionFilter()
{
	void* addr = (void*)GetProcAddress(LoadLibraryA("kernel32.dll"), "SetUnhandledExceptionFilter");

	if (addr)
	{
		unsigned char code[16];
		int size = 0;

		code[size++] = 0x33;
		code[size++] = 0xC0;
		code[size++] = 0xC2;
		code[size++] = 0x04;
		code[size++] = 0x00;

		DWORD dwOldFlag, dwTempFlag;
		VirtualProtect(addr, size, PAGE_EXECUTE_READWRITE, &dwOldFlag);

		WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);

		VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
	}
}

void InitMinDump()
{
	//ע���쳣��������
	SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)MyUnhandledExceptionFilter);

	//ʹSetUnhandledExceptionFilter
	DisableSetUnhandledExceptionFilter();
}





void CreateDumpFile(LPCSTR lpstrDumpFilePathName, EXCEPTION_POINTERS *pException)
{
	// ����Dump�ļ�
	HANDLE hDumpFile = CreateFileA(lpstrDumpFilePathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// Dump��Ϣ
	MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
	dumpInfo.ExceptionPointers = pException;
	dumpInfo.ThreadId = GetCurrentThreadId();
	dumpInfo.ClientPointers = TRUE;

	// д��Dump�ļ�����
	MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);

	CloseHandle(hDumpFile);
}

LONG WINAPI UnhandledExceptionFilterEx(struct _EXCEPTION_POINTERS *pException)
{
	char szMbsFile[MAX_PATH] = { 0 };
	::GetModuleFileNameA(NULL, szMbsFile, MAX_PATH);	
	char* pFind = strrchr(szMbsFile, '\\');
	if (pFind)
	{
		*(pFind + 1) = 0;
		strcat_s(szMbsFile, "CrashDumpFile.dmp");
		CreateDumpFile(szMbsFile, pException);
	}

	FatalAppExitA(-1, "Fatal Error");
	return EXCEPTION_CONTINUE_SEARCH;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}

BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
	if (hKernel32 == NULL)
		return FALSE;

	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL)
		return FALSE;

	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far

	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

	newJump[0] = 0xE9;  // JMP absolute
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
	return bRet;
}

void RunCrashHandler()
{
	SetUnhandledExceptionFilter(UnhandledExceptionFilterEx);
	PreventSetUnhandledExceptionFilter();
}
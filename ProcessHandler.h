#pragma once
#include <tchar.h>
#include <atlstr.h>
#include <psapi.h>
#include <tlhelp32.h>

class ProcessHandler {

	HANDLE hProcess;

public:
	
	DWORD processID;
	CString pname;

	ProcessHandler(CString _pname) {
		pname = _pname;
		
	}

	~ProcessHandler() {
		CloseHandle(hProcess);
	}

	HANDLE COpenProcess();

	DWORD GetProcessID(CString pname);

	DWORD GetProcessStats(PROCESS_MEMORY_COUNTERS_EX* pmc);

	BOOL isProcessRunning();

	VOID CloseProcessHandle();
};
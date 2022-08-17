#include "ProcessHandler.h"

HANDLE	ProcessHandler::COpenProcess() {

	processID = GetProcessID(pname);

	if (processID == 0) {
		return NULL;
	}

	hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
		FALSE,
		processID);

	if (hProcess == NULL)
		return NULL;
	return hProcess;
}

DWORD	ProcessHandler::GetProcessID(CString pname) {

	HANDLE hSnapshot;
	PROCESSENTRY32 pe;
	DWORD pid = 0;
	BOOL hResult;

	// snapshot of all processes in the system
	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) return 0;

	// initializing size: needed for using Process32First
	pe.dwSize = sizeof(PROCESSENTRY32);

	// info about first process encountered in a system snapshot
	hResult = Process32First(hSnapshot, &pe);

	// retrieve information about the processes
	// and exit if unsuccessful
	while (hResult) {
		// if we find the process: return process ID
		if (pname == pe.szExeFile) {
			pid = pe.th32ProcessID;
			break;
		}
		hResult = Process32Next(hSnapshot, &pe);
	}
	// closes an open handle (CreateToolhelp32Snapshot)
	CloseHandle(hSnapshot);
	return pid;
}

DWORD	ProcessHandler::GetProcessStats(PROCESS_MEMORY_COUNTERS_EX* pmc) {

	PROCESS_MEMORY_COUNTERS_EX pmcpvt;

	ZeroMemory(&pmcpvt, sizeof(PROCESS_MEMORY_COUNTERS_EX));

	if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmcpvt, sizeof(pmcpvt))) {
		pmc = &pmcpvt;
		return 0;
	}
	else {
		return GetLastError();
	}
}

BOOL	ProcessHandler::isProcessRunning() {

	DWORD pstatusCode;
	if (GetExitCodeProcess(hProcess, &pstatusCode)) {
		if (pstatusCode == STILL_ACTIVE) {
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
}

VOID ProcessHandler::CloseProcessHandle() {
	CloseHandle(hProcess);
}
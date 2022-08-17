#include <windows.h>
#include <tchar.h>
#include <atlstr.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <psapi.h>
#include <tlhelp32.h>
#include "ProcessHandler.h"

using namespace std;

std::ofstream debugLog;
CString out;

//HANDLE                  svcStopEvent = NULL;
LPWSTR					SVCNAME = (LPWSTR)L"ProcessMontiorService";
SERVICE_STATUS          svcStatus;
SERVICE_STATUS_HANDLE   svcStatusHandle;
CString					pname = "mspaint.exe";
BOOL					isServiceRunning = FALSE;
CString					logMessage = _T("");


VOID	WINAPI SvcCtrlHandler(DWORD);
VOID	WINAPI SvcMain(DWORD, LPTSTR*);
DWORD	WINAPI ServiceWorkerThread(LPVOID lpParam);

DWORD	FindProcessID(CString);
VOID	MainWorkerThread();
VOID	DebugLogger(CString);

int __cdecl _tmain(int argc, TCHAR* argv[])
{
	debugLog.open("D:\\LOG.txt", std::ios::out | std::ios::app);
	out.Format(_T("\t\t\t\t--------------------------------DEBUG LOG--------------------------------\n"));
	DebugLogger(out);

	SERVICE_TABLE_ENTRY DispatchTable[] =
	{
		{ SVCNAME, (LPSERVICE_MAIN_FUNCTION)SvcMain },
		{ NULL, NULL }
	};

	if (!StartServiceCtrlDispatcher(DispatchTable))
	{
		
		out.Format(_T("[ERROR]\t: StartServiceCtrlDispatcher FAILED. ERROR_CODE : %d \n"), GetLastError());
		DebugLogger(out);
	}
	else {
		out.Format(_T("[OK]\t: StartServiceCtrlDispatcher SUCCESS\n"));
		DebugLogger(out);
		
	}
	debugLog.close();
	return 0;
}

VOID	WINAPI SvcMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
	svcStatusHandle = RegisterServiceCtrlHandler(SVCNAME, SvcCtrlHandler);

	if (!svcStatusHandle)
	{
		out.Format(_T("[ERROR]\t: Error in RegisterServiceCtrlHandler : %d\n"), GetLastError());
		DebugLogger(out);
		return;
	}
	else {
		out.Format(_T("[OK]\t: RegisterServiceCtrlHandler Success.\n"));
		DebugLogger(out);
	}

	svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	svcStatus.dwControlsAccepted = 0;
	svcStatus.dwCurrentState = SERVICE_START_PENDING;
	svcStatus.dwWin32ExitCode = 0;
	svcStatus.dwServiceSpecificExitCode = 0;
	svcStatus.dwCheckPoint = 0;


	if (SetServiceStatus(svcStatusHandle, &svcStatus) == FALSE)
	{;
		out.Format(_T("[ERROR]\t: My Sample Service: ServiceMain: SetServiceStatus returned error : %d\n"), GetLastError());
		DebugLogger(out);
	}

	svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	svcStatus.dwCurrentState = SERVICE_RUNNING;
	svcStatus.dwWin32ExitCode = 0;
	svcStatus.dwCheckPoint = 0;

	if (SetServiceStatus(svcStatusHandle, &svcStatus) == FALSE)
	{
		out.Format(_T("[ERROR]\t: My Sample Service: ServiceMain: SetServiceStatus returned error : %d\n"), GetLastError());
		DebugLogger(out);
	}
	else {
		isServiceRunning = TRUE;
		out.Format(_T("[OK]\t: Service Started\n"));
		DebugLogger(out);
	}

	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	WaitForSingleObject(hThread, INFINITE);

	//CloseHandle(svcStopEvent);

	svcStatus.dwControlsAccepted = 0;
	svcStatus.dwCurrentState = SERVICE_STOPPED;
	svcStatus.dwWin32ExitCode = 0;
	svcStatus.dwCheckPoint = 3;

	if (SetServiceStatus(svcStatusHandle, &svcStatus) == FALSE)
	{

		out.Format(_T("[ERROR]\t: My Sample Service: ServiceMain: SetServiceStatus returned error : %d\n"), GetLastError());
		DebugLogger(out);

	}

}

VOID	WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl) {
	case SERVICE_CONTROL_STOP:
		
		if (svcStatus.dwCurrentState == SERVICE_STOPPED) {
			out.Format(_T("[ERROR]\t: Service Already Stopped."));
			DebugLogger(out);
		}
		//Perform tasks necessary to stop the service here
	
		isServiceRunning = FALSE;
		
		svcStatus.dwControlsAccepted = 0;
		svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
		svcStatus.dwWin32ExitCode = 0;
		svcStatus.dwCheckPoint = 4;

		if (SetServiceStatus(svcStatusHandle, &svcStatus) == FALSE) {
			out.Format(_T("[ERROR]\t: My Sample Service: ServiceMain: SetServiceStatus returned error : %d\n"), GetLastError());
			DebugLogger(out);
		}
		// This will signal the worker thread to start shutting down
		//SetEvent(svcStopEvent);
		break;

	default:
		break;
	}

}

DWORD	WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	thread workerThread = thread(MainWorkerThread);
	
	workerThread.join();

	return ERROR_SUCCESS;
}

VOID MainWorkerThread() {

	ProcessHandler ph = ProcessHandler(pname);
	HANDLE hProcess;
	BOOL isStatsRunning = FALSE;

	std::ofstream myfile;
	myfile.open("D:\\ServiceLog.txt", std::ios::out | std::ios::app);
	out.Format(_T("\t\t\t\t---------------------------STATS LOG-----------------------------\n"));
	myfile << CStringA(out);

	while (isServiceRunning) {
		hProcess = ph.COpenProcess();

		if (hProcess != NULL) {
			out.Format(_T("[OK]\t : Process Opened.\n"));
			DebugLogger(out);
			isStatsRunning = TRUE;
		}
		else {
			out.Format(_T("[LOG]\t : NO SUCH PROCESS or WAITING FOR PROCESS TO COME ALIVE.\n"));
			DebugLogger(out);
			Sleep(1000);
		}

		PROCESS_MEMORY_COUNTERS_EX pmc;
		DWORD pstatusCode;

		while (isStatsRunning) {

			if (!isServiceRunning) break;

			if (ph.isProcessRunning()) {
				if (ph.GetProcessStats(&pmc) == 0)
				{
					CString out;
					out.Format(_T("[Process Name] : %s [processID] : %d [Working Set Memory] : %lu [Private Working Set Memory] : %lu "), pname, ph.processID, pmc.WorkingSetSize, pmc.PrivateUsage);
					myfile << CStringA(out) << endl;
				}
			}
			else {
				out.Format(_T("[LOG]\t : PROCESS  STOPPED.\n"));
				DebugLogger(out);
				isStatsRunning = FALSE;
				ph.CloseProcessHandle();
			}
			Sleep(1000);
		}
	}

	myfile.close();

	
	return;
}

VOID DebugLogger(CString log) {
	debugLog << CStringA(log);
}
#include "StdAfx.h" 
#include <iostream>
#include "LuraSysHeartbeat.h"

fnStartProgram g_StartProgram;
fnFinalizeProgram g_FinalizeProgram;

LPSTR SVCNAME;
LPSTR SVCDISPNAME;

SERVICE_STATUS_HANDLE g_ServiceStatusHandle;
HANDLE g_StopEvent;
DWORD g_CurrentState = 0;
BOOL g_bSystemShutdown = FALSE;
BOOL g_bProgramWorking = FALSE;

void ReportErrorStatus(DWORD errorCode)
{
	g_CurrentState = SERVICE_STOPPED;
	SERVICE_STATUS serviceStatus = 
	{
		SERVICE_WIN32_OWN_PROCESS,
		g_CurrentState,
		0,
		ERROR_SERVICE_SPECIFIC_ERROR,
		errorCode,
		0,
		0,
	};
	SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
}

void ReportStatus(DWORD state)
{
	g_CurrentState = state;
	SERVICE_STATUS serviceStatus =
	{
		SERVICE_WIN32_OWN_PROCESS,
		g_CurrentState,
		state == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
		NO_ERROR,
		0,
		0,
		0
	};
	SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
}

// Handler for service control events.
DWORD WINAPI HandlerEx(DWORD control, DWORD eventType, void *eventData, void *context)
{
	switch (control)
	{
		// Entrie system is shutting down.
		// continue... with SERVICE_CONTROL_STOP section
		case SERVICE_CONTROL_SHUTDOWN:
		{
			g_bSystemShutdown = TRUE;
		}
		
		case SERVICE_CONTROL_STOP:
		{
			// Service is being stopped.
			ReportStatus(SERVICE_STOP_PENDING);
			SetEvent(g_StopEvent); 
		}break;
		
		default:
		{	// Ignoring all other events, but we must always report service status.
			ReportStatus(g_CurrentState); 
		}break;
	}
	return NO_ERROR;
}

//service to start or stop and you should not exceed that without a good reason.
void ReportProgressStatus(DWORD state, DWORD checkPoint, DWORD waitHint)
{
	g_CurrentState = state;
	SERVICE_STATUS serviceStatus = 
	{
		SERVICE_WIN32_OWN_PROCESS,
		g_CurrentState,
		state == SERVICE_START_PENDING ? 0 : SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN,
		NO_ERROR,
		0,
		checkPoint,
		waitHint
	};
	SetServiceStatus(g_ServiceStatusHandle, &serviceStatus);
}
 
// Main function to be executed as entire service code.
void WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	// Must be called at start.
	g_ServiceStatusHandle = RegisterServiceCtrlHandlerEx(SVCNAME, &HandlerEx, NULL);

	// Startup code.
	ReportStatus(SERVICE_START_PENDING);
	g_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	/* Here initialize service...Load configuration, acquire resources etc. */
	ReportStatus(SERVICE_RUNNING);

	// check if the program is runnig on every half a seconds
	while (WaitForSingleObject(g_StopEvent, 500) != WAIT_OBJECT_0)
	{
		if (!g_bProgramWorking)
		{
			g_bProgramWorking = true;
			if (!g_StartProgram())
			{
				cout << "fn g_StartProgram failed"; 
				break;
			}
		}
	}

	ReportStatus(SERVICE_STOP_PENDING);
	 
	/* 
	Here finalize service...
	Save all unsaved data etc., but do it quickly.
	If g_bSystemShutdown, you can skip freeing memory etc.
	*/ 

	if (!g_bSystemShutdown)
	{
		cout << "fn g_FinalizeProgram...";

		g_FinalizeProgram();
		
		cout << "Program recources finalized";
	}

	CloseHandle(g_StopEvent);
	ReportStatus(SERVICE_STOPPED);
	g_bProgramWorking = FALSE;
}

BOOL SvcInstall(LPCSTR lpszServiceName, LPCSTR lpszDisplayName)
{
	cout << "Installing service:" << lpszServiceName;

	SC_HANDLE schService;
	SC_HANDLE schSCManager; 
	TCHAR szExePath[MAX_PATH] = { 0 };

	HMODULE hModule = GetModuleHandle(NULL);
	if (!hModule)
	{
		cout << "Cannot install service. GetModuleHandle failed: "<< GetLastError();
		return FALSE;
	}

	if (!GetModuleFileName(hModule, szExePath, MAX_PATH))
	{
		cout << "Cannot install service. GetModuleFileName failed:" << GetLastError();
		return FALSE;
	}

	// Get a handle to the SCM database.  
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		cout << "OpenSCManager failed:" << GetLastError();
		return FALSE;
	}

	// Create the service 
	schService = CreateService(
		schSCManager,              // SCM database 
		lpszServiceName,           // name of service 
		lpszDisplayName,           // service name to display 
		SERVICE_ALL_ACCESS,        // desired access 
		SERVICE_WIN32_OWN_PROCESS, // service type 
		SERVICE_DEMAND_START,      // start type 
		SERVICE_ERROR_NORMAL,      // error control type 
		szExePath,                 // path to service's binary 
		NULL,                      // no load ordering group 
		NULL,                      // no tag identifier 
		NULL,                      // no dependencies 
		NULL,                      // LocalSystem account 
		NULL);                     // no password 

	if (schService == NULL)
	{
		cout << "CreateService failed:" << GetLastError();
		CloseServiceHandle(schSCManager);
		return FALSE;
	}
	else
	{
		cout << "Service installed successfully";
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);

	return TRUE;
}

CString LastErrorText(DWORD dwErr)
{
	TCHAR *pMsgBuf = NULL;
	DWORD dwMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPTSTR>(&pMsgBuf), 0, NULL);

	if (!dwMsgLen)
		return _T("FormatMessage fail");

	CString sMsg(pMsgBuf, dwMsgLen);
	LocalFree(pMsgBuf);
	return sMsg;
} 

INT_PTR GoLive(LPCSTR lpszServiceName, LPCSTR lpszDisplayName, fnStartProgram fnStartProgram, fnFinalizeProgram fnFinalizeProgram)
{
	if (!fnStartProgram || !fnFinalizeProgram)
	{
		cout << "invalid StartProgram//FinalizeProgram function pointers";
		return -2; // 
	}
	 
	LPCSTR SVCNAME		= lpszServiceName;
	LPCSTR SVCDISPNAME	= lpszDisplayName;
	g_StartProgram		= fnStartProgram;
	g_FinalizeProgram	= fnFinalizeProgram;

	SERVICE_TABLE_ENTRY recServiceTable[] =
	{
		{ _T(""), (LPSERVICE_MAIN_FUNCTION)&ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(recServiceTable))
	{
		cout << "StartServiceCtrlDispatcher";
		return 0; // no error
	}
	else
	{
		DWORD dwLastError = GetLastError();
		if (dwLastError == ERROR_FAILED_SERVICE_CONTROLLER_CONNECT)
		{
			cout << "Program not started as a service";
			return -1;
		}
		else
		{
			cout << "Other error:" << LastErrorText(dwLastError).GetBuffer(0);
			return -2;  
		}
	}
}

BOOL SvcAlive()
{
	return g_bProgramWorking;
}
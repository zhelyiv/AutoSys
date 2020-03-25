#include "StdAfx.h"
#include "SqlConnector.h"
#include "BaseActionManager.h"

// STD redirect https://msdn.microsoft.com/en-us/library/windows/desktop/ms682499(v=vs.85).aspx

typedef struct STD_DATA
{
	HANDLE m_hPipe;
	char m_szFilename[512]; 
} *PSTD_DATA;
 
DWORD WINAPI ReadStdOut(LPVOID lpParam)
{
	PSTD_DATA pData = (PSTD_DATA)lpParam;

	if (!pData)
		return 0;

	if (strlen(pData->m_szFilename) == 0)
	{
		CloseHandle(pData->m_hPipe);
		return 0;
	}

	HANDLE hFile = CreateFile( pData->m_szFilename
		, GENERIC_WRITE
		, 0
		, NULL
		, CREATE_NEW
		, FILE_ATTRIBUTE_NORMAL
		, NULL
	);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(pData->m_hPipe);
		return 1;
	}

	DWORD dwRead = 0;
	DWORD dwWritten = 0;
	const DWORD BUFSIZE = 4096;
	CHAR chBuf[BUFSIZE] = { 0 };
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	for (;;)
	{
		bSuccess = ReadFile(pData->m_hPipe, chBuf, BUFSIZE, &dwRead, NULL);
		if (!bSuccess || dwRead == 0)
			break;

		bSuccess = WriteFile(hFile, chBuf, dwRead, &dwWritten, NULL);
		if (!bSuccess)
			break;
	}

	CloseHandle(hFile);
	CloseHandle(pData->m_hPipe);
	return 0;
}

void CBaseActionManager::RunJob()
{
	m_job.status = JOB_STATUS_RUNNING;
	m_job.status_changed_dt_unix = time(0);

	CSqlConnector oDb;
	if (!oDb.Open(g_CONFIG.DatabaseFile))
	{
		// log oDb.GetErr()  
	}

	if (!oDb.SqlExec(CAutoSysJob::SqlSetStatus(JOB_STATUS_RUNNING, m_job.id).GetBuffer(0)))
	{
		// log oDb.GetErr()  
	}

	if (!Execute())
	{
		// oActionManager.LastError();
		// log
	}

	CalcNextRun(m_job);

	if (!oDb.SqlExec(m_job.SqlUpdate().GetBuffer(0)))
	{
		// log 
	}
}


void CBaseActionManager::CalcNextRun(CAutoSysJob& job)
{
	time_t tNow = time(0);

	CString strRuntime0 = TimeStr(tNow);
	CString strRuntime1 = TimeStr(job.next_run_dt_unix);

	job.status = JOB_STATUS_FINISHED;
	job.status_changed_dt_unix = tNow;
	memset(&job.status_comment, 0, sizeof(job.status_comment));

	switch (job.shedule)
	{
	case JOB_SCHEDULE_ONCE:
	{
		job.next_run_dt_unix = 0;
		job.status = JOB_STATUS_EXPIRED;
		StringCchCopy(job.status_comment, sizeof(job.status_comment), "expired: onetime job");
	}break;
	case JOB_SCHEDULE_HOURLY:
	{
		job.next_run_dt_unix = AddHours(1, tNow);
		job.status = JOB_STATUS_FINISHED;
	}break;
	case JOB_SCHEDULE_DAILY:
	{
		job.next_run_dt_unix = AddDays(1, tNow);
		job.status = JOB_STATUS_FINISHED;
	}break;
	case JOB_SCHEDULE_MONTHLY:
	{
		job.next_run_dt_unix = AddMonths(1, tNow);
		job.status = JOB_STATUS_FINISHED;
	}break;
	case JOB_SCHEDULE_YEARLY:
	{
	//  TODO 
	//	job.next_run_dt_unix = AddYear(1, tNow);
	//	job.status = JOB_STATUS_FINISHED;
	}break;

	default:
	{
		job.next_run_dt_unix = 0;
		job.status = JOB_STATUS_ONICE;
		StringCchCopy(job.status_comment, sizeof(job.status_comment)
			, StringFormat("onice auto: schedule type \"%c\" is not supported", job.shedule));
	}

	if (job.next_run_dt_unix >= job.expiry_dt_unix)
	{
		job.next_run_dt_unix = 0;
		job.status = JOB_STATUS_EXPIRED;
		StringCchCopy(job.status_comment, sizeof(job.status_comment), "expired");
	}
	}

	CString strRuntime2 = TimeStr(job.next_run_dt_unix);
}

BOOL CBaseActionManager::Execute()
{  
	CString strCommand = PrepareCommand();
	if (strCommand.IsEmpty())
		return FALSE;

	if ( !PipeCreate(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr)
		 || !PipeCreate(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr) )
	{
		ClosePipes();
		return FALSE;
	}
	  
	char* pszCommand = strCommand.GetBuffer(0);
	  
	PROCESS_INFORMATION outProcInfo = { 0 };
	STARTUPINFO startupInfo = { 0 };
	startupInfo.cb = sizeof(startupInfo);
	startupInfo.hStdOutput = g_hChildStd_OUT_Wr;
	startupInfo.hStdError = g_hChildStd_ERR_Wr;
	 
	if ( !CreateProcess(NULL
		, pszCommand
		, NULL
		, NULL
		, FALSE
		, NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW
		, NULL
		, NULL
		, &startupInfo
		, &outProcInfo) )
	{
		m_strError = GetLastErrorString(GetLastError());
		ClosePipes();
		return FALSE;
	}
  
	PSTD_DATA PDataArray[2];
	PDataArray[0] = (PSTD_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PSTD_DATA));
	PDataArray[1] = (PSTD_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PSTD_DATA));
	
	HANDLE hThreadArray[2] = { 0 };
	DWORD dwThreadIdArray[2] = { 0 };

	DWORD dwWaitProcRes = 0;
	DWORD dwWaitPipeRes = 0;

	if (outProcInfo.hProcess)
	{ 
		PDataArray[0]->m_hPipe = g_hChildStd_OUT_Rd;
		StringCchCopy(PDataArray[0]->m_szFilename, sizeof(PDataArray[0]->m_szFilename), m_job.stdout_file);

		PDataArray[1]->m_hPipe = g_hChildStd_ERR_Rd;
		StringCchCopy(PDataArray[1]->m_szFilename, sizeof(PDataArray[1]->m_szFilename), m_job.stderr_file);

		hThreadArray[0] = CreateThread(NULL, 0, ReadStdOut, PDataArray[0], 0, &dwThreadIdArray[0]);
		if (!hThreadArray[0]) 
			ClosePipeHandles(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr); 

		hThreadArray[1] = CreateThread(NULL, 0, ReadStdOut, PDataArray[1], 0, &dwThreadIdArray[1]);
		if (!hThreadArray[1])
			ClosePipeHandles(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr); 

		DWORD dwTimeout = GetTimeout() > 0 ? GetTimeout() * 1000 : INFINITE;
		
		// чакаме процеса да завърши
		dwWaitProcRes = WaitForSingleObject(outProcInfo.hProcess, dwTimeout);
		
		// чакаме ншките да завършат
		dwWaitPipeRes = WaitForMultipleObjects(2, hThreadArray, TRUE, dwTimeout);
	}
	else
	{
		m_strError = "Process was not created properly";
		ClosePipes();
		return FALSE;
	}

	DWORD dwExitCode = -1;

	switch (dwWaitProcRes)
	{
		case WAIT_OBJECT_0:
		{
			if (!GetExitCodeProcess(outProcInfo.hProcess, &dwExitCode))
			{
				SetComment("Unable to retrieve process exit code");
			}
			else
			{
				SetComment("The process finished successfuly");
			}
		}break;

		case WAIT_ABANDONED:
		{
			SetComment("Process event result retrieval: WAIT_ABANDONED");
		}break;

		case WAIT_TIMEOUT:
		{
			SetComment("Process event result retrieval: WAIT_TIMEOUT");
		}break;

		case WAIT_FAILED:
		{
			SetComment("Process event result retrieval: WAIT_FAILED");
		}break; 
	}
	 
	// деалокираме ресурсите на работния процес
	CloseHandle(outProcInfo.hProcess);
	CloseHandle(outProcInfo.hThread);

	// деалокираме нишките, четящи пайповете
	for (int i = 0; i< 2; i++)
	{
		CloseHandle(hThreadArray[i]);
		if (PDataArray[i] != NULL)
		{
			HeapFree(GetProcessHeap(), 0, PDataArray[i]);
			PDataArray[i] = NULL;  
		}
	}

	SetExitCode(dwExitCode);
	  
	return dwExitCode == -1;
}

CString CBaseActionManager::PrepareCommand()
{
	CString strCmdLine;
	CString strCommand = GetCommand();
	CString strParams = GetParams();

	if (strCommand.IsEmpty())
		return CString();

	switch (m_job.type)
	{
		case JOB_TYPE_CMD:
		{
			strCmdLine.Format("cmd.exe //C %s %s", strCommand, strParams);

		}break;
		//
		case JOB_TYPE_EXE:
		{
			if (_access(strCommand, 0) != 0)
			{ 
				m_strError.Format("File [%s] does not exist", strCommand);
				return CString();
			}
 
			strCmdLine.Format("%s %s", strCommand, strParams);

		}break;
		//
		case JOB_TYPE_SP1:
		{   
			if (_access(strCommand, 0) != 0)
			{
				m_strError.Format("File [%s] does not exist", strCommand);
				return CString();
			}

			if (_access(strCommand, 4) != 0)
			{
				m_strError.Format("File [%s] does not have read permissions", strCommand);
				return CString();
			}
			 
			strCmdLine.Format("powershell -executionPolicy bypass -file \"%s\" ", strCommand);  
			if (!strParams.IsEmpty())
				strCmdLine.AppendFormat("\"%s\" ", strParams);

		}break;
		//
		case JOB_TYPE_VBA:
		{
			if (_access(strCommand, 0) != 0)
			{
				m_strError.Format("File [%s] does not exist", strCommand);
				return CString();
			}

			if (_access(strCommand, 4) != 0)
			{
				m_strError.Format("File [%s] does not have read permissions", strCommand);
				return CString();
			}

			strCmdLine.Format("C:\\Windows\\System32\\cscript.exe %s %s", strCommand, strParams);

		}break;
		//
		default:
		{
			m_strError.Format("Type [%c] not supported", m_job.type);
			return CString();
		}
	}

	strCmdLine.Trim();
	return strCmdLine;
}
 
BOOL CBaseActionManager::PipeCreate(PHANDLE phRead, PHANDLE phWrite) 
{
	SECURITY_ATTRIBUTES saAttr; 
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(phRead, phWrite, &saAttr, 0))
	{
		m_strError = GetLastErrorString(GetLastError());
		return FALSE;
	}
	 
	SetHandleInformation(phRead, HANDLE_FLAG_INHERIT, 0);

	return TRUE;
}
   
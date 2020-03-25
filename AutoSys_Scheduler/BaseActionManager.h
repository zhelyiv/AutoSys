#pragma once 
#include "jobs_tbl.h"

class CBaseActionManager
{
public:
	CBaseActionManager(CAutoSysJob& job) : m_job(job) {}
	virtual ~CBaseActionManager() {}
	 
	void RunJob(); 
	CString LastError() { return m_strError; }

private:
	BOOL Execute();
	
	void CalcNextRun(CAutoSysJob& job);
	
	CString PrepareCommand();
	
	BOOL PipeCreate(PHANDLE phRead, PHANDLE phWrite);
	 
	void ClosePipes()
	{
		ClosePipeHandles(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr);
		ClosePipeHandles(&g_hChildStd_ERR_Rd, &g_hChildStd_ERR_Wr);
	}

	void ClosePipeHandles(PHANDLE phRead, PHANDLE phWrite)
	{
		CloseHandle(*phRead);
		CloseHandle(*phWrite);
	}
	 
	void SetExitCode(int nExitCode) { m_job.exit_code = nExitCode; }
	void SetComment(CString strComment) {StringCchCopy(m_job.comment, sizeof(m_job.comment), strComment); }
   
	CString GetCommand() { return m_job.command_line; }
	CString GetParams() { return m_job.command_params; }
	DWORD GetTimeout() { return (DWORD)m_job.timeout_seconds; }

private:
	CAutoSysJob& m_job;
	   
	HANDLE g_hChildStd_OUT_Rd;
	HANDLE g_hChildStd_OUT_Wr; 
	HANDLE g_hChildStd_ERR_Rd;
	HANDLE g_hChildStd_ERR_Wr; 

	CString m_strError;
};
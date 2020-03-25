#include "StdAfx.h"
#include "Logger.h"
#include "LuraSysThPool.h"

string strreplace(string word, string target, string replacement) 
{
	int len, loop = 0;
	string nword = "", let;
	len = word.length();
	len--;
	while (loop <= len) 
	{
		let = word.substr(loop, 1);
		if (let == target)
		{
			nword = nword + replacement;
		}
		else 
		{
			nword = nword + let;
		}
		loop++;
	}
	return nword;

}

std::string gettmsamp()
{ 
	std::time_t rawtime;
	std::tm* timeinfo;
	char szTimestamp[64] = {0};

	std::time(&rawtime);
	localtime_s(timeinfo,&rawtime);

	std::strftime(szTimestamp, sizeof(szTimestamp), "%Y-%m-%d-%H-%M-%S", timeinfo);
	 
	return szTimestamp;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum eLogLevel
{
	loglvl_info = 0 // info log - severity 0
	,loglvl_err = 1 // app err - severity 1
	,loglvl_sql = 2 // db err - severity 2
	,loglvl_critical = 3 // app critical err - severity 3
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LOGMSG
{
public:
	LOGMSG(const std::string& sText, eLogLevel loglvl) : m_sMsg(sText), m_loglvl(loglvl) 
	{
		m_dwThreadId = GetCurrentThreadId();
	}

	LOGMSG(CString strText, eLogLevel loglvl) : m_sMsg(strText.GetBuffer(strText.GetLength())), m_loglvl(loglvl) 
	{
		m_dwThreadId = GetCurrentThreadId();
	}

	~LOGMSG() {}

	DWORD m_dwThreadId;
	std::string m_sMsg;
	eLogLevel m_loglvl;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CLoggerThPool : public CLuraThPool<LOGMSG>
{
public:
	CLoggerThPool() : CLuraThPool<LOGMSG>(1) 
	{
		if (!InitializeCriticalSectionAndSpinCount(&m_csLogFile, 0x00000400))
		{
			printf("Unable to initialize critical section object");
		}
	}  
	
	~CLoggerThPool()
	{
		Stop();
		DeleteCriticalSection(&m_csLogFile);
	}

	BOOL SetProcessingDetails(const std::string& sLogDir, std::string sBaseFilename, int unMaxFileSizeKB)
	{
		m_sLogDir = sLogDir;
		m_sLogDir = strreplace(m_sLogDir, "\\", "/" );

		if (!sBaseFilename.empty()) 
			m_sBaseFilename = sBaseFilename; 
		else 
			m_sBaseFilename = "logfile_"; 
		 
		m_sLogFilename = m_sBaseFilename + gettmsamp();

		m_unMaxFileSizeKB = unMaxFileSizeKB; 

		if (CreateDirectory(sLogDir.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
		{
			printf("Directory '%s' ready", sLogDir.c_str()); 
		}
		else
		{
			printf("Failed to create directory '%s'", sLogDir.c_str());
			return FALSE;
		}

		return TRUE;
	}
	 
private:
	int Process(const LOGMSG& objLogLine)
	{  
		std::string logfile = m_sLogDir + "/" + m_sLogFilename + ".log";
		
		std::ifstream in(logfile, std::ifstream::ate | std::ifstream::binary);
		size_t length = in.tellg();
		if (objLogLine.m_sMsg.length() + length > m_unMaxFileSizeKB * 1000)
		{
			m_sLogFilename = m_sBaseFilename + gettmsamp();
			logfile = m_sLogDir + "/" + m_sLogFilename + ".log";
		}

		fstream fsLog;
		fsLog.open(logfile, std::fstream::in | std::fstream::out | std::fstream::app);
		 
		// If file does not exist, Create new file
		if (!fsLog)
		{
			fsLog.open(logfile, std::fstream::in | std::fstream::out | std::fstream::trunc);
			fsLog << "time" << ";" << "loglvl" << ";" << "thread" << ";" << "message" << endl; 
			fsLog.close(); 
		} 

		fsLog << gettmsamp() << ";" << objLogLine.m_loglvl << ";" << objLogLine.m_dwThreadId << ";" << objLogLine.m_sMsg << endl;
		fsLog.close();

		return 1;
	}

	void AfterProcess(int nProcessResult){}

private:
	std::string m_sLogDir;
	std::string m_sLogFilename;
	std::string m_sBaseFilename;
	unsigned int m_unMaxFileSizeKB;
	CRITICAL_SECTION m_csLogFile;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CLogger
{ 
public:
	CLogger() :m_bInitialized(FALSE)
	{
	}
	 
	~CLogger()
	{ 
		m_oLoggerThPool.Stop();  
	};
	 
	BOOL Initialize(std::string sLogDir, std::string sBaseFilename, unsigned int unMaxFileSizeKB)
	{
		m_bInitialized = TRUE;
		
		if (!m_oLoggerThPool.SetProcessingDetails(sLogDir, sBaseFilename, unMaxFileSizeKB))
		{
			m_bInitialized = FALSE;
		}
		
		return m_bInitialized;
	}

    void LogMsg(LOGMSG recLogMsg)
	{
		m_oLoggerThPool.Enqueue(recLogMsg, 0);
	}
 
private:
	BOOL m_bInitialized; 
	CLoggerThPool m_oLoggerThPool;
};
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLogger g_oLogger; // inacessible global object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL LOGGER_INIT(const char* pszDir, const char* pszFileNameTemplate, int nMaxKbFileSize)
{
	return g_oLogger.Initialize(pszDir, pszFileNameTemplate, nMaxKbFileSize);
}

void LOGGER_FREE()
{ 
	// be smart
}

extern void LAPPINFO(const char * format, ...)
{
	CString strText;
	va_list arglist;
	va_start(arglist, format);
	strText.FormatV(format, arglist);
	va_end(arglist); 

	g_oLogger.LogMsg(LOGMSG(strText, loglvl_info));
}

extern void LAPPERR(const char * format, ...)
{
	CString strText;
	va_list arglist;
	va_start(arglist, format);
	strText.FormatV(format, arglist);
	va_end(arglist); 

	g_oLogger.LogMsg(LOGMSG(strText, loglvl_err));
}

extern void LAPPSQL(const char * format, ...)
{
	CString strText;
	va_list arglist;
	va_start(arglist, format);
	strText.FormatV(format, arglist);
	va_end(arglist); 

	g_oLogger.LogMsg(LOGMSG(strText, loglvl_sql));
}

extern void LAPPCRITICAL(const char * format, ...)
{
	CString strText;
	va_list arglist;
	va_start(arglist, format);
	strText.FormatV(format, arglist);
	va_end(arglist); 

	g_oLogger.LogMsg(LOGMSG(strText, loglvl_critical));
}

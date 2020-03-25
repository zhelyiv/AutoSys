#include "StdAfx.h" 
#include "JobScheduler.h"
#include "AutoSys_Utils.h"	
#include "BaseActionManager.h"
  
#define READ_TH_SLEEP		 5*60	// 5min
#define CACHEMNG_TH_SLEEP	 1		// 1sec
 
////////////////////////////////////////////////////////////////////////////////
// CJobSchedulerThPool

int CJobSchedulerThPool::Process(CAutoSysJob& job)
{   
	CBaseActionManager oActionManager(job);
	oActionManager.RunJob();

	return TRUE; 
}
 
void CJobSchedulerThPool::AfterProcess(int nProcessResult)
{
	// status update or ?
}

////////////////////////////////////////////////////////////////////////////////
// CJobScheduler

CJobScheduler::CJobScheduler(void): m_bStop(false), m_bInitialized(FALSE)
{
}

CJobScheduler::~CJobScheduler(void)
{
	Stop();
	DeleteCriticalSection(&m_csJobCache);
}
 
BOOL CJobScheduler::Init()
{
	m_bInitialized = false;
	if (!InitializeCriticalSectionAndSpinCount(&m_csJobCache, 0x00000400))
	{
		m_err = GetLastErrorString(GetLastError());
		goto LBL_ERR;
	}
	if (!m_oThPool.Initialize())
	{
		m_err = m_oThPool.GetErr();
		goto LBL_ERR;
	}

	m_bInitialized = TRUE;
	return m_bInitialized;

LBL_ERR: 
	DeleteCriticalSection(&m_csJobCache);
	m_oThPool.Stop();
	return FALSE;

}

BOOL CJobScheduler::Start()
{
	BOOL bRes = TRUE;

	try
	{
		if (!m_oThPool.Start(false))
		{
			bRes = FALSE;
			goto LBL_ERR;
		}

		m_tSqlJobReader  = std::thread{ &CJobScheduler::ReadJob		, this };  
		m_tCacheManager  = std::thread{ &CJobScheduler::ManageCache	, this };
	}
	catch (...)
	{
		// log any errors
	}
LBL_ERR:
	return bRes;
}
  
void CJobScheduler::Stop()
{
	m_bStop = true;
	m_oThPool.Stop();
}
   
void CJobScheduler::LSleep(int nSeconds)
{ 
	time_t tWakeUp = AddSeconds(abs(nSeconds), time(0));
	while (time(0) <= tWakeUp)
	{ 
		if (m_bStop)
		{
			break;
		}
	}
}
  
void CJobScheduler::ReadJob()
{ 
	char* pszSqlWhere = StringFormat("status = 0 and days_of_week like '%s%'", DayOfWeek()).GetBuffer(0);
	char* pszSqlSelect = CAutoSysJob::SqlSelect(pszSqlWhere).GetBuffer(0);

	while (!m_bStop)
	{  
		CSqlConnector oDb;
		if (!oDb.Open(g_CONFIG.DatabaseFile))
		{ 
			// log oDb.GetErr()
			break;
		}
		 
		CSqlReader oSqlReader;
		if (!oDb.SqlSelect(pszSqlSelect, oSqlReader))
		{
			// log oDb.GetErr()
			break;
		}

		if (TryEnterCriticalSection(&m_csJobCache))
		{
			while (oSqlReader.Fetch())
			{ 
				m_vCache.push_back(CAutoSysJob(oSqlReader));
			}
			LeaveCriticalSection(&m_csJobCache);

			if (oSqlReader.FetchError())
			{
				// log
				break;
			}
		}
		 
		LSleep(READ_TH_SLEEP); 
	}
}

void CJobScheduler::ManageCache()
{
	while (!m_bStop)
	{ 
		if (TryEnterCriticalSection(&m_csJobCache))
		{
			std::vector<int> vCleanupIDs;
			for (size_t i = 0; i < m_vCache.size(); i++)
			{
				auto job = m_vCache[i];
				if (job.next_run_dt_unix <= time(0))
				{
					m_oThPool.Enqueue(job, job.priority);
					vCleanupIDs.push_back(job.id);
				}
			}

			if (vCleanupIDs.size() > 0)
			{
				auto lambdaCleanup = [&vCleanupIDs](const CAutoSysJob& job)->bool
				{
					return std::find(vCleanupIDs.begin(), vCleanupIDs.end(), job.id) != vCleanupIDs.end();
				};

				m_vCache.erase(remove_if(m_vCache.begin(), m_vCache.end(), lambdaCleanup), m_vCache.end());
			}
		}
		LeaveCriticalSection(&m_csJobCache);

		LSleep(CACHEMNG_TH_SLEEP);
	}
}
 
 
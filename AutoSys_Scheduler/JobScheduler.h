#pragma once
#include <atomic>
#include <map>
#include "LuraSysThPool.h"
#include "jobs_tbl.h"
#include "SqlConnector.h"
 
class CJobSchedulerThPool : public CLuraThPool<CAutoSysJob>
{
public:
	CJobSchedulerThPool() : CLuraThPool<CAutoSysJob>(1){}
	~CJobSchedulerThPool() {}

private:
	virtual int Process(CAutoSysJob& obj); 
	virtual void AfterProcess(int nProcessResult); 
};

class CJobScheduler
{
public:
	CJobScheduler(void);
	~CJobScheduler(void);

	BOOL Init();
	BOOL Start();
	void Stop();
	 
private: 
	void ReadJob();
	std::thread m_tSqlJobReader;  

	void ManageCache();
	std::thread m_tCacheManager;
	 
private:
	void LSleep(int nSeconds);

private:  
	std::string				 m_err;  
	std::atomic<bool>		 m_bStop;
	std::vector<CAutoSysJob> m_vCache;
	CJobSchedulerThPool		 m_oThPool; 
	CRITICAL_SECTION		 m_csJobCache;
	BOOL					 m_bInitialized;
};
 

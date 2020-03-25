#pragma once 
#include "AutoSys_Utils.h"
#include "SqlConnector.h"

/*
operation: a/e/d
status:active/expired
name:nzok importer
description:import of nzok files
priority:1
command:
params:
shedule: o / d / m / w
days_of_week:
start_YYYYDDMM:
start_HHMM:
expire_YYYYDDMM:
expire_HHMM:
expiry_delete: Y/N
stdout_file:
stderr_file:
mashine:
owner_default:
owner_pass:
*/

#define JOB_TYPE_CMD		   'c'
#define JOB_TYPE_EXE		   'e'
#define JOB_TYPE_SP1		   'p'
#define JOB_TYPE_VBA		   'v'
   
#define JOB_PRIORITY_LOW		1
#define JOB_PRIORITY_MEDIUM		2
#define JOB_PRIORITY_HIGH		3
#define JOB_PRIORITY_CRITICAL   4

#define JOB_STATUS_INITIAL		0
#define JOB_STATUS_TIMEOUT		1
#define JOB_STATUS_RUNNING		2 
#define JOB_STATUS_FINISHED		3  
#define JOB_STATUS_FAILED		4 
#define JOB_STATUS_ONICE		5
#define JOB_STATUS_EXPIRED		6 
#define JOB_STATUS_INVALID		7 

#define JOB_SCHEDULE_ONCE		'o'
#define JOB_SCHEDULE_HOURLY		'h'
#define JOB_SCHEDULE_DAILY		'd'
#define JOB_SCHEDULE_MONTHLY	'm'
#define JOB_SCHEDULE_YEARLY		'y'
  
struct jobs_tbl	
{
	jobs_tbl()
	{
		SecureZeroMemory(this, sizeof(*this)); 
	}

	int  id							;
	char name				[512]   ; 
	time_t creation_dt_unix			;
	char description		[1024]  ;
	char type			 			; // c,e,p,v
	int  priority		 			; // 1 - 4	
	int  status			 			; // 1 - 7
	time_t status_changed_dt_unix	;
	char status_comment		[128]   ;
	char command_line		[1024]  ;
	char command_params		[1024]  ;
	char shedule			 	    ; // o - once, h - hour, d - day, m - month, y - year
	char days_of_week		[32]	; // mo / tu / we / th / fr / sa / su
	time_t start_dt_unix		 	;
	time_t expiry_dt_unix		 	;
	time_t next_run_dt_unix 	 	;
	char stdout_file		[1024]  ;
	char stderr_file		[1024]  ;
	int  exit_code					;
	char comment			[128]	;
	int  timeout_seconds			;
};
 
class CAutoSysJob : public jobs_tbl
{
public:
	CAutoSysJob(CSqlReader& oSqlReader); 
	CAutoSysJob();
	~CAutoSysJob();

	BOOL ParseScript(CString strScript);
	
	CString SqlInsert();
	CString SqlUpdate();

	static CString SqlDelete(CString& strWhere);
	static CString SqlSetStatus(int nStatus, int id);
	static CString SqlSelect(char* pszWhere);

	BOOL m_bMarkForDel;
};

/*
struct jobs_tree_tbl
{
	jobs_tree_tbl() { SecureZeroMemory(this, sizeof(*this)); }
	
	sqlid			tree_id;
	sqlid			job_id;
	sqlid			prev_job_id; 
	sqlid			next_job_id_ifprev1;
	sqlid			next_job_id_ifprev0;

	time_t			creation_dt_unix;
	char			created_by[256];
};
*/
#include "stdafx.h"
#include "jobs_tbl.h"

CAutoSysJob::CAutoSysJob() :m_bMarkForDel(FALSE)
{ 
}

CAutoSysJob::~CAutoSysJob()
{
}

CAutoSysJob::CAutoSysJob(CSqlReader& oSqlReader)
{
	int nCol = 0;
	//
	id = oSqlReader.GetInt(nCol++);
	oSqlReader.GetString(nCol++, name, sizeof(name));
	creation_dt_unix = oSqlReader.GetChar(nCol++);
	oSqlReader.GetString(nCol++, description, sizeof(description));
	type = oSqlReader.GetChar(nCol++);
	priority = oSqlReader.GetInt(nCol++);
	status = oSqlReader.GetInt(nCol++);
	status_changed_dt_unix = oSqlReader.GetInt(nCol++);
	oSqlReader.GetString(nCol++, command_line, sizeof(status_comment));
	oSqlReader.GetString(nCol++, command_line, sizeof(command_line));
	oSqlReader.GetString(nCol++, command_params, sizeof(command_params));
	shedule = oSqlReader.GetChar(nCol++);
	oSqlReader.GetString(nCol++, days_of_week, sizeof(days_of_week)); 
	start_dt_unix = oSqlReader.GetInt(nCol++);
	expiry_dt_unix = oSqlReader.GetInt(nCol++);
	next_run_dt_unix = oSqlReader.GetInt(nCol++);
	oSqlReader.GetString(nCol++, stdout_file, sizeof(stdout_file));
	oSqlReader.GetString(nCol++, stderr_file, sizeof(stderr_file));
	exit_code = oSqlReader.GetInt(nCol++);
	oSqlReader.GetString(nCol++, comment, sizeof(comment));
	timeout_seconds = oSqlReader.GetInt(nCol++);
}
 
CString CAutoSysJob::SqlInsert()
{
	creation_dt_unix = time(0);
	status_changed_dt_unix = creation_dt_unix;

	return StringFormat
	(
		"insert into jobs			 " CLRF
		"(							 " CLRF
		"	name					 " CLRF
		"	, creation_dt_unix		 " CLRF
		"	, description			 " CLRF
		"	, type					 " CLRF
		"	, priority				 " CLRF
		"	, status				 " CLRF
		"	, status_changed_dt_unix " CLRF
		"	, status_comment		 " CLRF
		"	, command_line			 " CLRF
		"	, command_params		 " CLRF
		"	, shedule				 " CLRF
		"	, days_of_week			 " CLRF
		"	, start_dt_unix			 " CLRF
		"	, expiry_dt_unix		 " CLRF
		"	, next_run_dt_unix		 " CLRF
		"	, stdout_file			 " CLRF
		"	, stderr_file			 " CLRF
		"	, exit_code				 " CLRF
		"	, comment				 " CLRF
		"	, timeout_seconds		 " CLRF
		")							 " CLRF
		"values						 " CLRF
		"(							 " CLRF
		" %s'						 " CLRF
		" %ld						 " CLRF
		" '%s'						 " CLRF
		" '%c'						 " CLRF
		" %d						 " CLRF
		" %d						 " CLRF
		" %ld						 " CLRF
		" '%s'						 " CLRF
		" '%s'						 " CLRF
		" '%s'						 " CLRF
		" '%c'						 " CLRF
		" '%s'						 " CLRF
		" %ld						 " CLRF
		" %ld						 " CLRF
		" %ld						 " CLRF
		" '%s'						 " CLRF
		" '%s'						 " CLRF
		" %d						 " CLRF
		" '%s'						 " CLRF
		" %d						 " CLRF
		")							 " CLRF 
		, name
		, creation_dt_unix
		, description
		, type
		, priority
		, status
		, status_changed_dt_unix
		, status_comment
		, command_line
		, command_params
		, shedule
		, days_of_week
		, start_dt_unix
		, expiry_dt_unix
		, next_run_dt_unix
		, stdout_file
		, stderr_file
		, exit_code
		, comment
		, timeout_seconds 
	);
}

CString CAutoSysJob::SqlUpdate()
{
	return StringFormat
	( 
		" update jobs					   " CLRF
		" set name = '%s'				   " CLRF
		" , creation_dt_unix = %ld		   " CLRF
		" , description = '%s'			   " CLRF
		" , type = '%c'					   " CLRF
		" , priority = %d				   " CLRF
		" , status = %d					   " CLRF
		" , status_changed_dt_unix = %ld   " CLRF
		" , status_comment = '%s'		   " CLRF
		" , command_line = '%s'			   " CLRF
		" , command_params = '%s'		   " CLRF
		" , shedule = '%c'				   " CLRF
		" , days_of_week = '%s'			   " CLRF
		" , start_dt_unix = %ld			   " CLRF
		" , expiry_dt_unix = %ld		   " CLRF
		" , next_run_dt_unix = %ld		   " CLRF
		" , stdout_file = '%s'			   " CLRF
		" , stderr_file = '%s'			   " CLRF
		" , exit_code = %d				   " CLRF
		" , comment = '%s'				   " CLRF
		" , timeout_seconds = %d		   " CLRF
		" where							   " CLRF
		" id = %d						   " CLRF
		//
		, name
		, creation_dt_unix
		, description
		, type
		, priority
		, status
		, status_changed_dt_unix
		, status_comment
		, command_line
		, command_params
		, shedule
		, days_of_week
		, start_dt_unix
		, expiry_dt_unix
		, next_run_dt_unix
		, stdout_file
		, stderr_file
		, exit_code
		, comment
		, timeout_seconds
		// where
		, id
		 
	);
}

//static
CString CAutoSysJob::SqlDelete(CString& strWhere)
{ 
	return StringFormat("delete from jobs where %s", strWhere);
}
 
//static
CString CAutoSysJob::SqlSelect(char* pszWhere)
{
	CString strQuery("select * from jobs");

	if (pszWhere)
	{
		strQuery.AppendFormat(" where %s", pszWhere);
	}

	return strQuery;
}

//static
CString CAutoSysJob::SqlSetStatus(int nStatus, int id)
{
	return StringFormat("update jobs set status = %d where id = %d", nStatus, id);
}

// [<key>]:[<VALUE>]\r\n
BOOL CAutoSysJob::ParseScript(CString strScript)
{
	strScript.Replace("\r", "");
	strScript.Replace("\n", "");

	auto lines = SplitString(strScript, ";");
	 
	for (size_t i = 0; i < lines.size(); i++)
	{
		auto pair = SplitString(lines[i], ":");
		if (pair.size() < 2) 
			return FALSE;

		auto key = pair[0].Trim();
		auto val = pair[1].Trim();

		if (key == "")
			continue;

		if (key == "action")
		{
			if (val.MakeLower() == "del")
				m_bMarkForDel = true;
		} 
		else if (key == "id")
		{
			id = atoi(val);
		}
		else if (key == "name")
		{
			StringCchCopy(name, sizeof(name), val);
		}
		else if (key == "description")
		{
			StringCchCopy(description, sizeof(description), val);
		}
		else if (key == "type")
		{
			if(val.GetLength() > 0)
				type = val[0];
		}
		else if (key == "priority")
		{
			priority = atoi(val);
		}
		else if (key == "status")
		{
			status = atoi(val);
		}
		else if (key == "status_comment")
		{
			StringCchCopy(status_comment, sizeof(status_comment), val);
		}
		else if (key == "command_line")
		{
			StringCchCopy(command_line, sizeof(command_line), val);
		}
		else if (key == "command_params")
		{
			StringCchCopy(command_params, sizeof(command_params), val);
		}
		else if (key == "shedule")
		{
			if (val.GetLength() > 0)
				shedule = val[0]; 
		}
		else if (key == "days_of_week")
		{
			StringCchCopy(days_of_week, sizeof(days_of_week), val);
		}
		else if (key == "start_dt_unix")
		{ 
			time_t start_time = StringToTimeT(val);
			start_dt_unix = start_time;
			if (start_dt_unix <= 0)
			{
				status = JOB_STATUS_INVALID;
				StringCchCopy(status_comment, sizeof(status_comment), "Invalid start date");
			}
		}
		else if (key == "expiry_dt_unix")
		{ 
			time_t exp_time = StringToTimeT(val);
			expiry_dt_unix = exp_time;
		}
		else if (key == "stdout_file")
		{
			StringCchCopy(stdout_file, sizeof(stdout_file), val);
		}
		else if (key == "stderr_file")
		{
			StringCchCopy(stderr_file, sizeof(stderr_file), val);
		}
		else if (key == "comment")
		{
			StringCchCopy(comment, sizeof(comment), val);
		}
		else if (key == "timeout_seconds")
		{
			timeout_seconds = atoi(val);
		}
		else
		{
			// log invalid symbol
			return FALSE;
		}
	}

	if (strlen(name) == 0 && id == 0)
	{
		status = JOB_STATUS_INVALID;
		StringCchCopy(status_comment, sizeof(status_comment), "Invalid name/id");
	}

	return strlen(name) > 0 || id > 0;
}
#include "stdafx.h"
#include "SqlConnector.h"

char* SqliteErr(int rc)
{
	switch (rc)
	{
		case SQLITE_OK: return "Successful result";
		case SQLITE_ERROR: return "SQL error or missing database";
		case SQLITE_INTERNAL: return "NOT USED. Internal logic error in SQLite";
		case SQLITE_PERM: return "Access permission denied";
		case SQLITE_ABORT: return "Callback routine requested an abort";
		case SQLITE_BUSY: return "The database file is locked";
		case SQLITE_LOCKED: return "A table in the database is locked";
		case SQLITE_NOMEM: return "A malloc() failed";
		case SQLITE_READONLY: return "Attempt to write a readonly database";
		case SQLITE_INTERRUPT: return "Operation terminated by sqlite3_interrupt()";
		case SQLITE_IOERR: return "Some kind of disk I/O error occurred";
		case SQLITE_CORRUPT: return "The database disk image is malformed";
		case SQLITE_NOTFOUND: return "NOT USED. Table or record not found";
		case SQLITE_FULL: return "Insertion failed because database is full";
		case SQLITE_CANTOPEN: return "Unable to open the database file";
		case SQLITE_PROTOCOL: return "NOT USED. Database lock protocol error";
		case SQLITE_EMPTY: return "Database is empty";
		case SQLITE_SCHEMA: return "The database schema changed";
		case SQLITE_TOOBIG: return "String or BLOB exceeds size limit";
		case SQLITE_CONSTRAINT: return "Abort due to contraint violation";
		case SQLITE_MISMATCH: return "Data type mismatch";
		case SQLITE_MISUSE: return "Library used incorrectly";
		case SQLITE_NOLFS: return "Uses OS features not supported on host";
		case SQLITE_AUTH: return "Authorization denied";
		case SQLITE_FORMAT: return "Auxiliary database format error";
		case SQLITE_RANGE: return "2nd parameter to sqlite3_bind out of range";
		case SQLITE_NOTADB: return "File opened that is not a database file";
		case SQLITE_ROW: return "sqlite3_step() has another row ready";
		case SQLITE_DONE: return "sqlite3_step() has finished executing"; 
	} 

	return "No error";
}

CSqlConnector::CSqlConnector() : m_pdb(NULL)
{
}


CSqlConnector::~CSqlConnector()
{
	Close();
}

bool CSqlConnector::Open(char* pszDb)
{
	int rc = sqlite3_open(pszDb, &m_pdb);
	if (rc != SQLITE_OK && m_pdb)
	{
		m_err = sqlite3_errmsg(m_pdb);
		return false;
	}
	 
	return m_pdb != NULL;
}

void CSqlConnector::Close()
{
	if (m_pdb)
	{
		sqlite3_close(m_pdb);
		m_pdb = NULL;
	}
}

bool CSqlConnector::SqlExec(char* pszSql)
{
	if (!m_pdb)
	{
		m_err = "Database connection not yet established";
		return false;
	}

	char* pszErrMsg = NULL;
	int rc = sqlite3_exec(m_pdb, pszSql, NULL, 0, &pszErrMsg);
	
	if (rc != SQLITE_OK || pszErrMsg)
	{
		m_err = pszErrMsg;
		sqlite3_free(pszErrMsg);
		return false;
	}

	return true;
}

bool CSqlConnector::SqlSelect(char* pszSql, CSqlReader& oSqlReader)
{  
	int rc = sqlite3_prepare_v2(m_pdb, pszSql, -1, &oSqlReader.m_pStmt, 0);

	if (rc != SQLITE_OK)
	{
		m_err = sqlite3_errmsg(m_pdb);
		return false;
	} 
	  
	return true;
}

bool CSqlConnector::BeginTran()
{
	return SqlExec("BEGIN TRANSACTION;");
}

bool CSqlConnector::CommitTran()
{
	return SqlExec("END TRANSACTION;");
}

bool CSqlConnector::RollbackTran()
{
	return SqlExec("ROLLBACK TRANSACTION;");
}

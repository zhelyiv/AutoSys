#pragma once
#include "sqlite3.h"
 
char* SqliteErr(int rc);

class CSqlReader
{
public:  
	CSqlReader() : m_pStmt(NULL), m_rc(0), m_row(0) {}
	~CSqlReader() { Finalize(); }
	 
	bool Fetch()
	{
		if (!m_pStmt) return false;

		m_rc = sqlite3_step(m_pStmt);
		m_row++;

		return m_rc == SQLITE_ROW;
	}

	const char* GetErr() { return SqliteErr(m_rc); }
	void Finalize() { if (m_pStmt) sqlite3_finalize(m_pStmt); } 
	void Reset() { if (m_pStmt) sqlite3_reset(m_pStmt); }
	bool FetchError() { return m_rc != SQLITE_ROW && m_rc != SQLITE_DONE; } 
	  
	//types: SQLITE_INTEGER, SQLITE_FLOAT, SQLITE_TEXT, SQLITE_BLOB, SQLITE_NULL 
	int GetType(int nCol) { return sqlite3_column_type(m_pStmt, nCol); }
	const char* GetColName(int nCol) { return sqlite3_column_name(m_pStmt, nCol); }

	//get column values
	const void * GetBlob(int nCol) { return sqlite3_column_blob(m_pStmt, nCol); }
	int GetBytes(int nCol) { return	sqlite3_column_bytes(m_pStmt, nCol); }
	int GetBytes16(int nCol) { return sqlite3_column_bytes16(m_pStmt, nCol); }//utf8
	double GetDouble(int nCol) { return sqlite3_column_double(m_pStmt, nCol); }
	int GetInt(int nCol) { return sqlite3_column_int(m_pStmt, nCol); }
	__int64 GetInt64(int nCol) { return (__int64)sqlite3_column_int64(m_pStmt, nCol); }
	
	int GetString(int nCol, char* szDest, size_t nDestSize) 
	{
		const char *pValue = (const char *)sqlite3_column_text(m_pStmt, nCol);
		if(pValue)
			StringCchCopy(szDest, nDestSize, (LPCSTR)pValue);

		return strlen(szDest);
	} 

	char GetChar(int nCol)
	{
		char szBuff[2] = { 0 };
		GetString(nCol, szBuff, sizeof(szBuff)); 
		return szBuff[0];
	}

	const void* GetString16(int nCol) { return sqlite3_column_text16(m_pStmt, nCol); } //utf8
	CString GetCString(int nCol) { return CString(sqlite3_column_text(m_pStmt, nCol)); }
	sqlite3_value* GetColVal(int nCol) { return sqlite3_column_value(m_pStmt, nCol); }

public:
	int m_rc; // last result code
	int m_row; // processed row cnt
	sqlite3_stmt* m_pStmt;
};

class CSqlConnector
{
public:
	CSqlConnector();
	~CSqlConnector(); 

	bool Open(char* pszDb = NULL);
	void Close();
	bool SqlExec(char* pszSql);
	__int64 LastRowId() { return sqlite3_last_insert_rowid(m_pdb); }
	
	bool BeginTran();
	bool CommitTran();
	bool RollbackTran();
	  
	bool SqlSelect(char* pszSql, CSqlReader& oSqlReader); 
	std::string GetErr() { return m_err; }
	 
	std::string m_err;
	sqlite3* m_pdb;
};


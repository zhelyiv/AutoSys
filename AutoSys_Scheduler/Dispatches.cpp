#include "stdafx.h" 
#include "Dispatches.h"
#include "CmdDispatcher.h"
#include "jobs_tbl.h"
#include "BaseActionManager.h"

void send_xml_response(SOCKET clnsocket, int nStatusCode, char* pszComment)
{
	if (clnsocket == 0)
		return;

	char szBuff[512] = { 0 };
	StringCchCopy(szBuff, sizeof(szBuff), StringFormat("<sc>%d</sc>", nStatusCode));

	if (::send(clnsocket, szBuff, sizeof(szBuff), 0) == SOCKET_ERROR)
	{
		// log 
		return;
	}

	if (pszComment)
	{
		StringCchCopy(szBuff, sizeof(szBuff), StringFormat("<cm>%s</cm>", pszComment));

		if (::send(clnsocket, szBuff, sizeof(szBuff), 0) == SOCKET_ERROR)
		{
			// log 
			return;
		}
	}
}

int CmdGetCommandList(char* pszParams, SOCKET clnsocket) 
{   
	if (!pszParams)
	{
		//log
		return FALSE;
	}
	 
	char szBuff[4096] = { 0 };
	StringCchCopy(szBuff, sizeof(szBuff), "<r>");
	if (::send(clnsocket, szBuff, sizeof(szBuff), 0) == SOCKET_ERROR)
	{
		// log
		return FALSE;
	}
	 
	for (int i = 1; i < sizeof(g_CommandNames)/sizeof(g_CommandNames[0]); i++)
	{ 
		memset(&szBuff, 0, sizeof(szBuff));
		StringCchCopy(szBuff, sizeof(szBuff)
			, StringFormat("<cmd><id>%d</id><name>%s</name><tmpl>%s</tmpl></cmd>"
				, g_CommandNames[i].m_nCmd, g_CommandNames[i].m_pszName, g_CommandNames[i].m_pszCmdTemplate));
		 
		if(::send(clnsocket, szBuff, sizeof(szBuff), 0) == SOCKET_ERROR)
		{
			// log
			return FALSE;
		}
	}
	 
	// send resp code
	send_xml_response(clnsocket, XML_RESP_OK); 

	StringCchCopy(szBuff, sizeof(szBuff), "</r>");
	if (::send(clnsocket, szBuff, sizeof(szBuff), 0) == SOCKET_ERROR) 
	{
		// log
		return FALSE;
	}

	return TRUE;
}
  
int CmdExecuteScript(char* pszParams, SOCKET clnsocket)
{
	if (!pszParams)
		return FALSE;
	
	auto vScripts = SplitString(CString(pszParams), "~");
	 
	if (vScripts.size() == 0)
	{
		send_xml_response(clnsocket, XML_RESP_ERR, "invalid script");
		return FALSE;
	}

	CSqlConnector oDb;
	if (!oDb.Open(g_CONFIG.DatabaseFile))
	{
		// log oDb.GetErr()
		send_xml_response(clnsocket, XML_RESP_ERR, "Database connection error");
		return FALSE;
	}

	if (!oDb.BeginTran())
	{
		// log oDb.GetErr()
		send_xml_response(clnsocket, XML_RESP_ERR, "Database transaction cannot be started");
		return FALSE;
	}

	BOOL bParseResult = TRUE;
	
	for (size_t i = 0; i < vScripts.size(); i++)
	{
		CAutoSysJob oJob;
		bParseResult = oJob.ParseScript(XmlExtract(vScripts[i], "script"));
		if (!bParseResult)
		{
			//log
			break;
		}

		if (oJob.id > 0 && oJob.m_bMarkForDel == FALSE)
		{
			if (!oDb.SqlExec(oJob.SqlUpdate().GetBuffer(0)))
			{
				//log
				break;
			} 
		}
		else if (oJob.m_bMarkForDel == FALSE)
		{
			CString strWhere;
			if (oJob.id > 0)
			{
				strWhere.Format("id = %d", oJob.id);
			}
			else if (strlen(oJob.name) > 0)
			{
				strWhere.Format("name = '%d'", oJob.id);
			} 

			if (!oDb.SqlExec(CAutoSysJob::SqlDelete(strWhere).GetBuffer(0)))
			{
				//log
				break;
			} 
		}
		else
		{
			if (!oDb.SqlExec(oJob.SqlInsert().GetBuffer(0)))
			{
				//log
				break;
			}
		}
	}

	if (!bParseResult)
	{
		send_xml_response(clnsocket, XML_RESP_ERR, "Parse error occurred");
		return FALSE;

		if (!oDb.RollbackTran())
		{
			// log oDb.GetErr()
			return FALSE;
		}
	}
	else
	{
		if (!oDb.CommitTran())
		{
			// log oDb.GetErr()
			send_xml_response(clnsocket, XML_RESP_ERR, "Database error occurred");
			return FALSE; 
		}
	}

	return TRUE;
}

int CmdGetJobStatus(char* pszParams, SOCKET clnsocket)
{
	CString strIds = XmlExtract(pszParams, "ids").Trim();
	if (strIds.IsEmpty())
	{
		// log
		return FALSE;
	}

	char* pszSqlWhere = StringFormat("id in (%s)", strIds).GetBuffer(0);
	char* pszSqlSelect = CAutoSysJob::SqlSelect(pszSqlWhere).GetBuffer(0);

	CSqlConnector oDb;
	if (!oDb.Open(g_CONFIG.DatabaseFile))
	{
		// log oDb.GetErr()
		send_xml_response(clnsocket, XML_RESP_ERR, "Database connection error");
		return FALSE;
	}

	CSqlReader oSqlReader;
	if (!oDb.SqlSelect(pszSqlSelect, oSqlReader))
	{
		// log oDb.GetErr()
		send_xml_response(clnsocket, XML_RESP_ERR, "Database read data error");
		return FALSE;
	}

	while (oSqlReader.Fetch())
	{
		CAutoSysJob job(oSqlReader);
		// todo ::send
		// status details
	}

	if (oSqlReader.FetchError())
	{
		// log 
		send_xml_response(clnsocket, XML_RESP_ERR, "Database fetch data error");
		return FALSE;
	}

	send_xml_response(clnsocket, XML_RESP_OK);
	return TRUE; 
}

int CmdStartJob(char* pszParams, SOCKET clnsocket)
{
	if (!pszParams)
		return FALSE;

	CString strIds = XmlExtract(pszParams, "ids").Trim();
	if (strIds.IsEmpty())
	{
		// log
		return FALSE;
	} 

	char* pszSqlWhere = StringFormat("id in (%s)", strIds).GetBuffer(0);
	char* pszSqlSelect = CAutoSysJob::SqlSelect(pszSqlWhere).GetBuffer(0);
	 
	CSqlConnector oDb;
	if (!oDb.Open(g_CONFIG.DatabaseFile))
	{
		// log oDb.GetErr()
		send_xml_response(clnsocket, XML_RESP_ERR, "Database connection error"); 
		return FALSE; 
	}

	CSqlReader oSqlReader;
	if (!oDb.SqlSelect(pszSqlSelect, oSqlReader))
	{
		// log oDb.GetErr()
		send_xml_response(clnsocket, XML_RESP_ERR, "Database read data error");
		return FALSE;
	}

	while (oSqlReader.Fetch())
	{
		CAutoSysJob job(oSqlReader);
		CBaseActionManager oActionManager(job);
		oActionManager.RunJob();
	}

	if (oSqlReader.FetchError())
	{
		// log 
		send_xml_response(clnsocket, XML_RESP_ERR, "Database fetch data error");
		return FALSE;
	}
	 
	send_xml_response(clnsocket, XML_RESP_OK);
	return TRUE;
}
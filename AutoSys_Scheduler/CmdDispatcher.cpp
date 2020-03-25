#include "stdafx.h" 
#include "CmdDispatcher.h"
#include "Dispatches.h"
#include "CmdList.h"
 
//static 
int CmdDispatcher::Execute(CString& strXml, SOCKET clnsocket)
{   
	int nCommand = GetCommand(strXml);
	if (nCommand == APP_COMMANDS::cmd_empty)
		return FALSE;
	
	funcptr pFunc = GetDispatch(nCommand);
	
	if (!pFunc)
	{
		// log
		return FALSE;
	}
	
	CString strCommandName = GetCommandName(nCommand);
	if (!strCommandName.IsEmpty())
	{
		// log register command
		// Client [ip:port] - command [<N>][<S>]
	}
	  
	return pFunc(GetCommandParams(strXml).GetBuffer(0), clnsocket);
}

//static 
int CmdDispatcher::GetCommand(CString& strXml)
{ 
	// ex: <cmd>15</cmd>
	CString strValue = XmlExtract(strXml, "cmd"); 
	if (IsInteger(strValue))
	{
		return atoi(strValue.GetBuffer(0));
	}
	 
	return APP_COMMANDS::cmd_empty;
}

CString CmdDispatcher::GetCommandParams(CString& strXml)
{
	//ex:
	// <params>
	// 	   <id>3232123</id>
	// 	   <search>text</search>
	// </params> 

	return XmlExtract(strXml, "params");
}

//static 
CString CmdDispatcher::GetCommandName(int nCommad)
{
	for (int i = 0; i < sizeof(g_CommandNames) / sizeof(g_CommandNames[0]); i++)
	{
		if (g_CommandNames[i].m_nCmd == nCommad && g_CommandNames[i].m_pszName != NULL)
		{
			return g_CommandNames[i].m_pszName;
		}
	}

	return StringFormat("ANONYMOUS command %d", nCommad);
}

//static 
funcptr CmdDispatcher::GetDispatch(int nCommad)
{
	for (int i = 0; i < sizeof(g_Commands) / sizeof(g_Commands[0]); i++)
	{
		if (g_Commands[i].m_nCmd == nCommad)
		{
			return g_Commands[i].m_pFunc;
		}
	}

	return NULL;
}

#pragma once
#include "CmdList.h"

#define XML_RESP_ERR 0 
#define XML_RESP_OK  1  
 
void send_xml_response(SOCKET clnsocket, int nStatusCode, char* pszComment = NULL);

int CmdGetCommandList(char* pszParams, SOCKET clnsocket); 
int CmdExecuteScript(char* pszParams, SOCKET clnsocket);
int CmdGetJobStatus(char* pszParams, SOCKET clnsocket); 
int CmdStartJob(char* pszParams, SOCKET clnsocket);

//////////////////////////////////////////////////////////////////////////////////////
// command-dispatch mapping

CMD const g_Commands[] =
{
	 { APP_COMMANDS::cmd_empty				, NULL }
	,{ APP_COMMANDS::cmd_get_command_list	, CmdGetCommandList } 
	,{ APP_COMMANDS::cmd_execute			, CmdExecuteScript }
	,{ APP_COMMANDS::cmd_get_status			, CmdGetJobStatus }
	,{ APP_COMMANDS::cmd_start_job			, CmdStartJob }

};
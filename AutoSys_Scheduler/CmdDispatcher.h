#pragma once
#include "CmdHelpers.h"

class CmdDispatcher
{
public:
	CmdDispatcher() {}
	~CmdDispatcher() {}
	
	static int Execute(CString& strXml, SOCKET clnsocket); 

private:
	static int GetCommand(CString& strXml); 
	static CString GetCommandParams(CString& strXml);
	static CString GetCommandName(int nCommad);
	static funcptr GetDispatch(int nCommad);
};
 

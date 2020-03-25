#pragma once

typedef INT_PTR(*funcptr)(char*, SOCKET);

struct CMD
{
	CMD(INT_PTR nCmd, funcptr pFunc) :m_nCmd(nCmd), m_pFunc(pFunc) {}

	INT_PTR m_nCmd;
	funcptr m_pFunc;
};

struct CMD_DETAILS
{
	INT_PTR m_nCmd;
	char* m_pszName;
	char* m_pszCmdTemplate;
};
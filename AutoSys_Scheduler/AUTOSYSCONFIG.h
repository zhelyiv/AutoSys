#pragma once
 
struct AUTOSYSCONFIG
{
	AUTOSYSCONFIG() { SecureZeroMemory(this, sizeof(*this)); }
	 
	//char ServiceName[256];
	char ListenPort[8];
	char DatabaseFile[MAX_PATH];
	char LogDirectory[MAX_PATH];
	char InDirectory[MAX_PATH];
  
	void Reload(LPCSTR pszINI)
	{
		//GetPrivateProfileString("COMMON", "ServiceName", 0, ServiceName, sizeof(ServiceName), pszINI);
		GetPrivateProfileString("COMMON", "ListenPort", 0, ListenPort, sizeof(ListenPort), pszINI);
		GetPrivateProfileString("COMMON", "DatabaseFile", 0, DatabaseFile, sizeof(DatabaseFile), pszINI);
		GetPrivateProfileString("COMMON", "LogDirectory", 0, LogDirectory, sizeof(LogDirectory), pszINI);
		GetPrivateProfileString("COMMON", "InDirectory", 0, InDirectory, sizeof(InDirectory), pszINI);
	}
};
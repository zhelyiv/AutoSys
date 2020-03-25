#include "stdafx.h"  
#include "Main.h" 
#include "LuraSysHeartbeat.h" 
 
#ifdef _DEBUG 
	#define new DEBUG_NEW
#endif

//CWinApp theApp; 
   
// Main ////////////////////////////////////////////////////////////////////////////////////

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{  
	if (LoadHModule())
	{
		cout << "LoadHModule failed"<< endl;
		return 1;
	}  

	if (argc > 1 && lstrcmpi(argv[1], TEXT("install")) == 0)
	{
		SvcInstall(DEF_SERVICENAME, DEF_SERVICEDISPLAYNAME);
		return 0;
	}

	INT_PTR nResult = GoLive(DEF_SERVICENAME, DEF_SERVICEDISPLAYNAME, StartProgram, FinalizeProgram);
	if (!nResult)
	{
		 
	}
	else if(nResult == -1)
	{
		StartProgram();
		 
		cout << "Press any key for exit >> ";
		nResult = PauseResult(0);

		FinalizeProgram();
	}  
	else
	{
		cout << "Unknown error ocurred. Failed to start service:  " << DEF_SERVICENAME;
		nResult = 1;
	}
	  
	return nResult;
}  

//END MAIN /////////////////////////////////////////////////////////////////////////////////////
 
int PauseResult(int rc)
{
	char ch; cin >> ch;
	return rc;
} 
  
int LoadHModule()
{
	int nRetCode = 0;
	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{ 
			char szExeFilepath[MAX_PATH] = {0};
			if (GetModuleFileName(hModule, szExeFilepath, sizeof(szExeFilepath)) < 0)
			{
				_tprintf(_T("Error: GetModuleFileName failed\n"));
				nRetCode = 1;
			}

			g_sExeFilepath = szExeFilepath; 
			CString INI = szExeFilepath; INI.Replace(".exe", ".ini");
			g_sIniFilepath = INI.GetBuffer(INI.GetLength());

			if (_access(g_sIniFilepath.c_str(), 0) != 0)
			{
				_tprintf(_T("Error: the file '%s' does not exist\n"), g_sIniFilepath);
				nRetCode = 1;
			}  
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}
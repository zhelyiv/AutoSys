#pragma once 
#include "resource.h"
 
int LoadHModule();
int PauseResult(int rc);

std::string g_sExeFilepath;
std::string g_sIniFilepath;

#define DEF_SERVICENAME "LuraSys_Job_Broker"
#define DEF_SERVICEDISPLAYNAME "LuraSys� Job-Broker Service"

#include "SqlConnector.h" 
#include "JobScheduler.h" 
#include "TcpListener.h" 

CJobScheduler g_oSheduler;
CTcpListener g_�TcpServer; 
  
INT_PTR StartProgram(void)
{ 
	g_CONFIG.Reload(g_sIniFilepath.c_str()); 
	cout << "CONFIG file loaded" << endl;  

	if (LOGGER_INIT(g_CONFIG.LogDirectory, g_sExeFilepath.c_str()))
	{
	}

	if (g_�TcpServer.Init())
	{
		g_�TcpServer.StartListener();
		cout << "TcpListener started" << endl; 
	}
	else
	{
		cout << "TcpListener failed" << endl;
		return false;
	}

	if (g_oSheduler.Init())
	{
		//g_oSheduler.Start();
		cout << "Scheduler started" << endl; 
	} 
	else
	{
		cout << "Scheduler failed" << endl;
		return false;
	}
	 
	return TRUE;
}

void FinalizeProgram(void)
{
	cout << "Stop TcpListener..." << endl;
	g_�TcpServer.Stop();
	cout << "Stop Scheduler..." << endl;
	g_oSheduler.Stop();

	// todo next

	cout << "All Job-Broker components stopped !" << endl;
}

 
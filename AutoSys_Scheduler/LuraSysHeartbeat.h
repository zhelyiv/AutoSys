#pragma once
#include "winsvc.h" 

//////////////////////////////////////////////////////////////////////////////////////////////////
// The LuraSysHeartbeat Framework interface ////////////////////////////////////////////////////// 

//program worker template
typedef INT_PTR(*fnStartProgram)(void);

//program worker finalizer template
typedef void(*fnFinalizeProgram)(void);
  
//start service
INT_PTR GoLive(LPCSTR lpszServiceName, LPCSTR lpszDisplayName, fnStartProgram fnStartProgram, fnFinalizeProgram fnFinalizeProgram);

//check if the service is alive
BOOL SvcAlive();

//install service
BOOL SvcInstall(LPCSTR lpszServiceName, LPCSTR lpszDisplayName);


 
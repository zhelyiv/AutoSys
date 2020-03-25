// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include <afx.h>
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT







#include <iostream> 
using namespace std;
			 
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <exception>
#include "strsafe.h"
#include <thread>     
#include <ctime>
#include <algorithm>

#define CLRF "\r\n"
#define __FL__ __FUNCTION__ "("__STR1__(__LINE__)"): "   
 
#include "AutoSys_Utils.h"

class Exception : public exception
{
 public:
  Exception(const char* pszMessage)
	  :m_pszFunction(NULL), m_nLine(-1)
  { 
	  SetMessage(pszMessage);
  } 
  Exception(const char* pszMessage, const char* pszFunction, int nLine)
	  :m_pszFunction(pszFunction), m_nLine(nLine)
  { 
	  SetMessage(pszMessage);
  } 
  
  virtual const char* what() const throw() { return m_pszMessage; }
 
  CString GetMessgae(){ return m_pszMessage ? m_pszMessage : ""; }
  CString GetSource() {StringFormat("[%s][%d]", GetFunction(), GetLine()); }
  CString GetFunction(){ return m_pszFunction ? m_pszFunction : "empty"; }
  int GetLine(){ return m_nLine; }

 private:
  void SetMessage(const char* pszMessage)
  {	 
	  if( pszMessage )
	  {
		 int size = strlen(pszMessage) + 1;
		 m_pszMessage = new char[size];
		 StringCchCopy( m_pszMessage, size, pszMessage);
	  }
  }

  char* m_pszMessage;
  const char* m_pszFunction;
  const int m_nLine;
};

#include "logger.h"
 
 

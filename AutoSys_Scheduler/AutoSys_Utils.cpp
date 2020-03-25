#include "stdafx.h"
#include "AutoSys_Utils.h"
#include "rpc.h"
#pragma comment(lib, "Rpcrt4.lib")

AUTOSYSCONFIG g_CONFIG;
 
CString GenGUID()
{
	UUID uuid;
	unsigned char *pszUuid = 0;
	UuidCreateSequential(&uuid);
	UuidToString(&uuid, &pszUuid);
	CString strGUID = (char*)pszUuid;
	RpcStringFree(&pszUuid);

	return strGUID;
}
 
CString SqlEscape(CString str)
{
	str.Replace("'", "''");
	return str;
}

CString XmlEscape(CString& str)
{ 
	str.Replace( "\""	, "&quot;"	);
	str.Replace( "'"	, "&apos;"	);
	str.Replace( "<"	, "&lt;"	);
	str.Replace( ">"	, "&gt;"	);
	str.Replace( "&"	, "&amp;"	);
	 
	return str;
}

CString XmlExtract(const CString& strXml, const CString& strTag)
{
	CString strXmlTag = StringFormat("<%s>", strTag);
	int nStart = strXml.Find(strXmlTag);

	if (nStart != -1)
	{
		nStart += strXmlTag.GetLength();
		int nEnd = strXml.Find(StringFormat("</%s>", strTag));
		if (nEnd > nStart)
		{
			return strXml.Mid(nStart, nEnd - nStart);
		}
	}

	return CString();
}

CString StringFormat(const char * format, ...)
{
	CString strText;
	va_list arglist;
	va_start(arglist, format);
	strText.FormatV(format, arglist);
	va_end(arglist);
	return strText;
}

CString GetLastErrorString(DWORD dwErr)	  
{		 
    TCHAR *pMsgBuf = NULL;
    DWORD dwMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<LPTSTR>(&pMsgBuf), 0, NULL);

    if (!dwMsgLen)
        return _T("FormatMessage fail");

    CString sMsg(pMsgBuf, dwMsgLen);
    LocalFree(pMsgBuf);
    return sMsg;
}

std::string GetFileName(const string& strPath) 
{
	char sep = '/';		  
	#ifdef _WIN32
	   sep = '\\';
	#endif

   size_t i = strPath.rfind(sep, strPath.length());
   if (i != string::npos) 
      return(strPath.substr(i+1, strPath.length() - i));  

   return("");
}

std::string GetFileDir(const std::string& strPath)
{		
	char szBuffer[MAX_PATH] = {0};
	StringCchCopy(szBuffer, sizeof(szBuffer), strPath.c_str());

	char *pStr = strrchr(szBuffer, '\\');
    if(pStr) pStr[0] = 0;

	return szBuffer;
}

extern struct tm* TimeNow() 
{
	time_t tNow = time(0); 
	struct tm * pTimeNow = NULL;// = localtime(&tNow); 

	if (_localtime64_s(pTimeNow, &tNow))
		return NULL;

	return pTimeNow;
}
 
const std::string DateTime(time_t tt)
{
	struct tm tstruct; 
	localtime_s(&tstruct, &tt);
	  
	char szTime[80] = { 0 };
	strftime(szTime, sizeof(szTime), "%Y-%m-%d.%X", &tstruct);

	return szTime;
}


CString TimeStr(time_t tt)
{
	char buff[24] = {0}; 
	struct tm tstruct;
	localtime_s(&tstruct, &tt);
	strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", &tstruct);

	return buff;
}

CString DayOfWeek()
{
	const CString arrWeekDays[] = { "mo","tu","we","th","fr","sa","su" };
	  
	time_t ttime = time(0);
	struct tm tstruct;
	localtime_s(&tstruct, &ttime); 

	return arrWeekDays[tstruct.tm_wday];
}

time_t StringToTimeT(CString strYYYYDDMMHHMMSS) 
{ 
	if ((size_t)strYYYYDDMMHHMMSS.GetLength() < strlen("YYYYDDMMHHMMSS"))
		return -1;

	struct tm timeinfo;
	memset(&timeinfo, 0, sizeof(timeinfo));

	CString year = strYYYYDDMMHHMMSS.Mid(0,4);
	CString mday = strYYYYDDMMHHMMSS.Mid(4,2);
	CString mon  = strYYYYDDMMHHMMSS.Mid(6,2);
	CString hour = strYYYYDDMMHHMMSS.Mid(8,2);
	CString min  = strYYYYDDMMHHMMSS.Mid(10,2);
	CString sec  = strYYYYDDMMHHMMSS.Mid(12,2);
	 
	timeinfo.tm_mday	= atoi(mday);
	timeinfo.tm_mon		= atoi(mon);
	timeinfo.tm_year	= atoi(year) - 1900; 
	timeinfo.tm_hour	= atoi(hour);
	timeinfo.tm_min		= atoi(min);
	timeinfo.tm_sec		= atoi(sec);
	 
	return mktime(&timeinfo);
}

time_t AddSeconds(int ss, time_t ttime)
{ 
	struct tm tstruct;
	localtime_s(&tstruct, &ttime);
	tstruct.tm_sec += ss;
	return mktime(&tstruct);
}
  
time_t AddMinutes(int mm, time_t ttime)
{
	struct tm tstruct;
	localtime_s(&tstruct, &ttime);
	tstruct.tm_min += mm;
	return mktime(&tstruct);
}

time_t AddHours(int hh, time_t ttime)
{
	struct tm tstruct;
	localtime_s(&tstruct, &ttime);
	tstruct.tm_hour += hh;
	return mktime(&tstruct);
}

time_t AddDays(int DD, time_t ttime)
{
	struct tm tstruct;
	localtime_s(&tstruct, &ttime);
	tstruct.tm_mday += DD;
	return mktime(&tstruct);
}

time_t AddMonths(int MM, time_t ttime)
{
	struct tm tstruct;
	localtime_s(&tstruct, &ttime);
	tstruct.tm_mon += MM;
	return mktime(&tstruct);
}
  
BOOL IsInteger(CString& strText)
{
	if (strText.IsEmpty())
		return FALSE;

	char chFirst = strText[0];

	if (chFirst < 0)
		return FALSE;

	if ((!isdigit(chFirst)) && (chFirst != '-') && (chFirst != '+'))
		return FALSE;

	char * p = NULL;
	const char * pBuff = strText.GetBuffer(strText.GetLength());
	strtol(pBuff, &p, 10);

	return (*p == 0);
}
 
std::vector<CString> SplitString(CString& strText, INT_PTR nSize)
{
	INT_PTR nOriginalLen = strText.GetLength();

	std::vector<CString> vResult;
	if (nSize > nOriginalLen)
	{
		if (!strText.IsEmpty())
			vResult.push_back(strText);

		return vResult;
	}//if

	INT_PTR nStart = 0;
	INT_PTR nCount = nSize;
	INT_PTR nAccumulator = 0;

	CString strPeice = strText.Mid(nStart, nCount);
	vResult.push_back(strPeice);
	nAccumulator += strPeice.GetLength();

	while (TRUE)
	{
		nStart = nAccumulator;
		nCount = nOriginalLen - nAccumulator;

		if (nCount > nSize)
			nCount = nSize;

		strPeice = strText.Mid(nStart, nCount);
		vResult.push_back(strPeice);

		nAccumulator += strPeice.GetLength();

		if (nAccumulator >= nOriginalLen)
			break;
	}

	return vResult;
}

std::vector<CString> SplitString(CString& strText, CString strDelimChar)
{
	std::vector<CString> vResult;

	char* pszText = strText.GetBuffer(strText.GetLength());
	char* pszPiece = NULL;

	pszPiece = strtok(pszText, strDelimChar);

	if (pszPiece)
		vResult.push_back(pszPiece);

	while (pszPiece != NULL)
	{
		pszPiece = strtok(NULL, strDelimChar);

		if (pszPiece)
			vResult.push_back(pszPiece);
	}

	if (vResult.size() == 0)
		vResult.push_back(strText);

	return vResult;
}
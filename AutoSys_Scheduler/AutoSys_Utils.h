#pragma once
#include "AUTOSYSCONFIG.h"
extern AUTOSYSCONFIG g_CONFIG;

extern CString GenGUID();
extern CString SqlEscape(CString str);
extern CString XmlEscape(CString& str); 
extern CString XmlExtract(const CString& strXml, const CString& strTag);
extern CString StringFormat(const char * format, ...); 
extern CString GetLastErrorString(DWORD dwErr);							    
extern std::string GetFileName(const std::string& strPath);
extern std::string GetFileDir(const std::string& strPath);

extern struct tm* TimeNow();
const std::string DateTime(time_t tt); 
CString TimeStr(time_t tt);
CString DayOfWeek();
time_t StringToTimeT(CString strYYYYDDMMHHMMSS);

time_t AddSeconds(int ss, time_t ttime);
time_t AddMinutes(int mm, time_t ttime);
time_t AddHours(int hh, time_t ttime);
time_t AddDays(int DD, time_t ttime);
time_t AddMonths(int MM, time_t ttime); 

BOOL IsInteger(CString& strText);
 
std::vector<CString> SplitString(CString& strText, INT_PTR nSize);
std::vector<CString> SplitString(CString& strText, CString strDelimeter);

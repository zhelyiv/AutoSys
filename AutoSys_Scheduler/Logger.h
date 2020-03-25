#pragma once
 
extern BOOL LOGGER_INIT(const char* pszDir, const char* pszFileNameTemplate, int nMaxKbFileSize = 2056);
extern void LOGGER_FREE();
extern void LAPPINFO(const char * format, ...);
extern void LAPPERR(const char * format, ...);
extern void LAPPSQL(const char * format, ...);
extern void LAPPCRITICAL(const char * format, ...);

#pragma once 
#include "LuraSysThPool.h"  
 
class CTcpListener : public CLuraThPool<SOCKET>
{
public:
	CTcpListener(); 
	~CTcpListener();
	  
	bool Init();
	void StartListener();
	void StopListener();
	CString LastError() { return m_strError; }

private:
	void ConnGateThread(); 
	virtual int Process(SOCKET& socket);
	void RegisterClnConn(SOCKET socket, int& nPort, char* pszIP, int nIpSize);
	 
private: 
	bool m_bInit;
	std::thread m_tListener; 
	WSADATA m_wsaData;
	CString m_strError;
	SOCKET m_ListenSocket; 
};

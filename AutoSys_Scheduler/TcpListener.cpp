#include "stdafx.h" 
#include "TcpListener.h"
#include "CmdDispatcher.h"
#include <windows.h>
#include <ws2tcpip.h>  
#include <winsock2.h>

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#pragma comment (lib, "Ws2_32.lib") 

#define MAX_CLNTS		24
#define DEFAULT_BUFLEN	4096
  
CTcpListener::CTcpListener() : CLuraThPool<SOCKET>(1)
{
	m_bInit = false;
	m_ListenSocket = INVALID_SOCKET;
}

CTcpListener::~CTcpListener()
{
	if (m_bInit)
	{
		Stop();
		closesocket(m_ListenSocket);
		WSACleanup();
	}
}

bool CTcpListener::Init()
{
	if(!strlen(g_CONFIG.ListenPort))
	{
		m_strError = "No listen port specified";
		return false;
	}

	if (!Initialize())
	{
		m_strError.Format("Thread pool cannot be Initialize: %s", GetErr());
		return false;
	}

	int nResultCode = WSAStartup(MAKEWORD(2, 2), &m_wsaData);
	if (nResultCode != 0)
	{
		m_strError.Format("WSAStartup failed with error: %d", nResultCode);
		return false;
	}

	m_bInit = true;
	return m_bInit;
}

void CTcpListener::StartListener()
{
	if (m_bInit)
	{
		if (Start())
		{ 
			m_tListener = std::thread{ &CTcpListener::ConnGateThread, this };
		}
	}
}

void CTcpListener::StopListener()
{  
	Stop();
	m_bInit = false;
}

void CTcpListener::ConnGateThread()
{  
	int nResultCode = 0; 
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo aiHints = { 0 };
	struct addrinfo* pAiResult = NULL;
	   
	aiHints.ai_family = AF_INET;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = IPPROTO_TCP;
	aiHints.ai_flags = AI_PASSIVE;
	
	nResultCode = getaddrinfo(NULL, g_CONFIG.ListenPort, &aiHints, &pAiResult);
	if (nResultCode != 0)
	{
		m_strError.Format("getaddrinfo failed with error: %d\n", nResultCode);
		WSACleanup();
		return;
	}
	 
	 m_ListenSocket = socket(pAiResult->ai_family, pAiResult->ai_socktype, pAiResult->ai_protocol);

	if (m_ListenSocket == INVALID_SOCKET)
	{
		m_strError.Format("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(pAiResult);
		WSACleanup();
		return;
	}
	 
	nResultCode = ::bind(m_ListenSocket, pAiResult->ai_addr, (int)pAiResult->ai_addrlen);
	if (nResultCode == SOCKET_ERROR)
	{
		m_strError.Format("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(pAiResult);
		closesocket(m_ListenSocket);
		WSACleanup();
		return;
	}

	freeaddrinfo(pAiResult);

	nResultCode = listen(m_ListenSocket, MAX_CLNTS);
	if (nResultCode == SOCKET_ERROR)
	{
		m_strError.Format("listen failed with error: %d\n", WSAGetLastError());
		closesocket(m_ListenSocket);
		WSACleanup();
		return;
	}
	 
	while (!m_bStop)
	{  
		ClientSocket = ::accept(m_ListenSocket, NULL, NULL); 
		if (ClientSocket == INVALID_SOCKET || ClientSocket == SOCKET_ERROR)
		{
			m_strError.Format("accept failed with error: %d\n", WSAGetLastError());
			closesocket(m_ListenSocket);
			WSACleanup();
			break;
		}
		  
		Enqueue(ClientSocket, 0);  
	} 
	 
	closesocket(m_ListenSocket); 
	WSACleanup(); 
}

void CTcpListener::RegisterClnConn(SOCKET socket, int& nPort, char* pszIP, int nIpSize)
{  
	struct sockaddr_storage addr = {0};
	socklen_t addrlen = sizeof addr;

	::getpeername(socket, (struct sockaddr*)&addr, &addrlen);
	 
	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in *sin = (struct sockaddr_in *)&addr;
		nPort = ntohs(sin->sin_port);
		inet_ntop(AF_INET, &sin->sin_addr, pszIP, nIpSize);
	}
	else
	{	 
		struct sockaddr_in6 *sin = (struct sockaddr_in6 *)&addr;
		nPort = ntohs(sin->sin6_port);
		inet_ntop(AF_INET6, &sin->sin6_addr, pszIP, nIpSize);
	} 
}
 
int CTcpListener::Process(SOCKET& socket)
{   
	int port = 0;
	char szIP[INET6_ADDRSTRLEN] = { 0 };

	RegisterClnConn(socket, port, szIP, sizeof(szIP));

	CString strData;
	size_t nSizeRecv = 0;
	char szBuff[DEFAULT_BUFLEN];
	 
	for(;;)
	{
		memset(&szBuff, 0, sizeof(szBuff)); 
		nSizeRecv = ::recv(socket, szBuff, sizeof(szBuff)-1, 0);
		  
 		if (nSizeRecv > 0)
		{
			strData.Append(szBuff);   
			if (szBuff[DEFAULT_BUFLEN-1] == '\0')
			{
				break;
			}
		}
		else if (nSizeRecv == 0)
		{ 
			break;
		}
		else
		{
			// log err
			CString strErr;
			strErr.Format("recv failed with error: %d\n", WSAGetLastError()); 
			closesocket(socket); 
			return 1;
		}  
	}  
	 
	int nResultCode = CmdDispatcher::Execute(strData, socket);
	 
	// log nResultCode
 
	closesocket(socket);
	return 0;
}

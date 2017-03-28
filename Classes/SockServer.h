#pragma once
#ifndef _SOCK_SERVER_H_
#define _SOCK_SERVER_H_

#if defined(ANDROID)
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <time.h>
#include <android/log.h>
#include <pthread.h>
#endif //ANDROID

#if defined(WIN32)
#ifndef _WINSOCKAPI_
#include <WinSock2.h>
#endif //_WINSOCKAPI_
#pragma comment(lib,"ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 1
#endif //WIN32

#include <string>
#include <vector>
#include "Framework/Globals.h"

#if defined(NDEBUG) || defined(_DEBUG)
#if defined(ANDROID)
#define MAX_LEN 256
static void SocketLog(const char * pszFormat, ...)
{
	char buf[MAX_LEN];

	va_list args;
	va_start(args, pszFormat);
	vsnprintf(buf, MAX_LEN, pszFormat, args);
	va_end(args);

	__android_log_print(ANDROID_LOG_DEBUG, "PhysicsGame", "%s", buf);
}
#endif //ANDROId

#if defined(WIN32)
static void SocketLog(const char * pszFormat, ...)
{
	char szBuf[MAX_PATH];

	va_list ap;
	va_start(ap, pszFormat);
	vsnprintf_s(szBuf, MAX_PATH, MAX_PATH, pszFormat, ap);
	va_end(ap);

	WCHAR wszBuf[MAX_PATH] = {0};
	MultiByteToWideChar(CP_UTF8, 0, szBuf, -1, wszBuf, sizeof(wszBuf));
	OutputDebugStringW(wszBuf);
	OutputDebugStringA("\n");

	WideCharToMultiByte(CP_ACP, 0, wszBuf, sizeof(wszBuf), szBuf, sizeof(szBuf), NULL, FALSE);
	printf("%s\n", szBuf);
}
#endif //WIN32
#else
static void SocketLog(const char * pszFormat, ...)
{
	//NOT USED
}
#endif //NDEBUG || _DEBUG


using namespace std;

#if defined(WIN32)
static DWORD WINAPI ListenSockets(LPVOID lpParam);
#endif //Win32

#if defined(ANDROID)
static void* ListenSockets(void *ptr);
#endif //ANDROID

struct Socket_info
{
	int m_socket;
	struct sockaddr_in m_address;
};

//Forward Declarations
class SockPeer;

class SockServer
{
public:
	SockServer(void);
	~SockServer(void);

	bool						CreateServer(unsigned int port,unsigned int maxClients);
	void						WaitForClients();

	bool						GetStatus() { return m_status; }

	bool						CheckListForIP(struct sockaddr_in &sockAddr);

	//Don't Call this, This is Called Internally for Cross Platform Support
	void						ListenForConnections();

	void						Close();

	vector<SockPeer*>*			GetPeers() { return &m_peers; }

private:
	bool						m_status;
	bool						m_isSetup;
	string						m_ipAddress;
	unsigned int				m_port;

	int							m_listenSock;
	struct sockaddr_in			m_listenAddr;
	unsigned int				m_maxClients;

	vector<SockPeer*>			m_peers;
};

#endif
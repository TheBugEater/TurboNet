#ifndef __TURBO_NET_H__
#define __TURBO_NET_H__

#include "NullPtr.h"

#define SAFE_DELETE_POINTER(__VAR__) \
	if(__VAR__) \
{ \
	delete __VAR__; \
	__VAR__ = nullptr; \
} \

/////////////////////////////////////////////////////////////////////////////////
////////////ANDROID//////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
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
#if defined(_USEDLL)
#define SOCK_DLL	__declspec(dllexport)
#else
#define SOCK_DLL	__declspec(dllimport)
#endif
#endif //ANDROID

/////////////////////////////////////////////////////////////////////////////////
////////////WIN32////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined(WIN32)
#ifndef _WINSOCKAPI_
#include <WinSock2.h>
#include <stdio.h>
#endif //_WINSOCKAPI_
#pragma comment(lib,"ws2_32.lib")
#define _CRT_SECURE_NO_WARNINGS 1
#if defined(_USEDLL)
#define SOCK_DLL	__declspec(dllexport)
#else
#define SOCK_DLL	__declspec(dllimport)
#endif
#endif //WIN32

/////////////////////////////////////////////////////////////////////////////////
////////////XBOX/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
#if defined(_XBOX)
#include <Xtl.h>
#if defined(_USEDLL)
#define SOCK_DLL		
#else
#define SOCK_DLL		
#endif
static char* inet_ntoa(struct in_addr in)
{
	int firstBit = in.S_un.S_un_b.s_b1;
	int secondBit = in.S_un.S_un_b.s_b2;
	int thirdBit = in.S_un.S_un_b.s_b3;
	int fourthBit = in.S_un.S_un_b.s_b4;

	char ipAddr[256];
	sprintf(ipAddr,"%d.%d.%d.%d",firstBit,secondBit,thirdBit,fourthBit);

	return ipAddr;
}

struct XBoxHeader
{
	XNADDR							m_xnaIp;
	XNKID							m_xnkid;
	XNKEY							m_xnkey;
};
#endif

/////////////////////////////////////////////////////////////////////////////////
////////////Debug Log || All Platforms///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
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

#if defined(WIN32) || (_XBOX)
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

/////////////////////////////////////////////////////////////////////////////////////
////////////////////Thread Prototypes ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
#if defined(WIN32) || (_XBOX)
static DWORD WINAPI ListenSockets(LPVOID lpParam);
#endif //Win32

#if defined(ANDROID)
static void* ListenSockets(void *ptr);
#endif //ANDROID

struct Socket_info
{
	int					m_socket;
	struct	sockaddr_in m_address;
};

#endif
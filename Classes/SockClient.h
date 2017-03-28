#pragma once
#ifndef _SOCK_CLIENT_H_
#define _SOCK_CLIENT_H_

#include "SockServer.h"
#include "SockPeer.h"

#if defined(WIN32)
static DWORD WINAPI ConnectSocket(LPVOID lpParam);
#endif //Win32

#if defined(ANDROID)
static void* ConnectSocket(void *ptr);
#endif //ANDROID

class SockClient
{
public:
	SockClient(void);
	~SockClient(void);

	void ConnectToServer(unsigned int port);

	bool GetStatus() { return m_status; }

	SockPeer* GetPeer() { return m_pPeer; }

	///Dont Call this Function, It's used internally.
	void _ConnectSocket();

	void Close();

private:
	bool m_status;

	//string m_ipAddr;
	int m_port;

	SockPeer *m_pPeer;
};

#endif //_SOCK_CLIENT_H_
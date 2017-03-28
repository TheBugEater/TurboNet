#include "TurboClient.h"
#include <errno.h>
#include <time.h>

#if defined(ANDROID)
#include <netdb.h>
#endif //ANDROID

#if defined(WIN32) || (_XBOX)
DWORD WINAPI ConnectSocket(void* lpParam)
{
	TurboNet::TurboClient *pServer = (TurboNet::TurboClient*)lpParam;
	pServer->_ConnectSocket();

	return NULL;
}
#endif //WIN32

#if defined(ANDROID)
void* ConnectSocket(void *ptr)
{
	TurboNet::TurboClient *pServer = (TurboNet::TurboClient*)ptr;
	pServer->_ConnectSocket();

	return NULL;
}
#endif //ANDROID

namespace TurboNet
{
	TurboClient::TurboClient(void)
		: m_status(false)
		, m_pPeer(nullptr)
		, m_isThreadRunning(false)
	{
	}


	TurboClient::~TurboClient(void)
	{
	}

	void TurboClient::ConnectToServer(unsigned int port,int timeOut)
	{
		m_isThreadRunning = true;

		m_timeOut = timeOut;

		m_port = port;

#if defined(WIN32) || (_XBOX)
		CreateThread(NULL,0,ConnectSocket,this,0,NULL);
#endif

#if defined(ANDROID)
		pthread_t thread;
		pthread_create (&thread, NULL, &ConnectSocket, (void *) this);
#endif
	}

	void TurboClient::_ConnectSocket()
	{
		int m_connectSock;
		struct sockaddr_in m_connectAddr;

#if defined(_XBOX)
		if( XNetStartup( NULL ) != 0 )
		{
			SocketLog("XNetStartup Failed");
		}
#endif

#if defined(WIN32) || (_XBOX)

		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2),&wsa);

#endif //Win32 || _XBOX

		memset(&m_connectAddr,0,sizeof(sockaddr_in));

		m_connectSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

		//If socket fails
		if(m_connectSock < 0)
		{
			SocketLog("Socket Creation Failed");
			return;
		}

#ifndef _XBOX
		char szBuffer[1024];
		gethostname(szBuffer, sizeof(szBuffer));

		struct hostent *host = gethostbyname(szBuffer);

		//Check for the IP.
		in_addr *pInIp = nullptr;

		if(host == NULL)
		{
			pInIp->s_addr = INADDR_BROADCAST;
		}
		else
		{
			pInIp = ((struct in_addr *)(host->h_addr));
			SocketLog("IP : %s",inet_ntoa(*pInIp));
		}

		m_connectAddr.sin_addr.s_addr = pInIp->s_addr;
		m_connectAddr.sin_addr.s_addr |= htonl(0x1FF);
#else
		m_connectAddr.sin_addr.s_addr = INADDR_BROADCAST;
#endif

		m_connectAddr.sin_family = AF_INET;
		m_connectAddr.sin_port = htons(m_port);

		SocketLog("Connecting to IP : %s",inet_ntoa(m_connectAddr.sin_addr));

		int broadcast = 1;
		int result = 0;

		result = setsockopt(m_connectSock,SOL_SOCKET,SO_BROADCAST,(char*)&broadcast,sizeof(int));

		if(result)
		{
			SocketLog("setsockopt Failed : %s",strerror(errno));
			return;
		}

#if defined(ANDROID)
		int nonblock = 1;
		ioctl(m_connectSock,FIONBIO,&nonblock);
#endif
#if defined(WIN32) || (_XBOX)
		unsigned long nonblock = 1;
		ioctlsocket(m_connectSock,FIONBIO,&nonblock);
#endif

		bool status = false;

		Socket_info info;
		memset(&info,0,sizeof(Socket_info));

		info.m_socket = m_connectSock;
		info.m_address = m_connectAddr;

		m_pPeer = new TurboPeer;
		m_pPeer->CreatePeer(info);

		//While Connected to the Server
		while(!m_status)
		{

			string message = "[[TURBONET:SEEKINGCONNECTION]]";

			sendto(	m_connectSock,message.c_str(),
				message.size(),
				0,
				(const sockaddr*)&m_connectAddr,
				sizeof(m_connectAddr));

			char buffer[256];
			int sizeAddr = sizeof(sockaddr_in);

			//Give Sometime for the SendTo call, To Process.
#if defined(ANDROID)
			sleep(1);
#endif
#if defined(WIN32) || (_XBOX)
			Sleep(1);
#endif
			SOCKADDR_IN psaIn;

			int sockSize = sizeof(SOCKADDR_IN);
			int recvSize = recvfrom(m_connectSock,buffer,256,0,(sockaddr*)&psaIn,&sockSize);

			if(recvSize > 0)
			{

#if defined(_XBOX)
				buffer[recvSize] = '\0';
				XBoxBuff *pBuff = (XBoxBuff*)buffer;

				XNKID           xnkid;
				XNKEY           xnkey;
				IN_ADDR			addr;

				xnkid = pBuff->m_xnkid;
				xnkey = pBuff->m_xnkey;

				XNetRegisterKey( &xnkid, &xnkey );

				XNetXnAddrToInAddr(&pBuff->m_xnaIp,&xnkid,&addr);

				SAFE_DELETE_POINTER(m_pPeer);

				info.m_socket = m_connectSock;
				info.m_address.sin_addr.s_addr = addr.s_addr;
				info.m_address.sin_port = htons(m_port);
				info.m_address.sin_family = AF_INET;

				m_pPeer = new TurboPeer;
				m_pPeer->CreatePeer(info);
#else
				buffer[recvSize] = '\0';
				m_pPeer->GetData()->InitWithDataString(buffer);

				m_pPeer->GetData()->GetValue("Connection",status);
#endif
				m_status = true;
			}

			SocketLog("Client Recieve Error : %s",strerror(errno));

#if defined(ANDROID)
			sleep(100);
#endif
#if defined(WIN32) || (_XBOX)
			Sleep(100);
#endif


		}

		m_isThreadRunning = false;
	}

	void TurboClient::Update(float dt)
	{

		if(!m_status)
		{
			if(!m_isThreadRunning)
			{
				Close();
				ConnectToServer(m_port,m_timeOut);
			}
			
			return;
		}

		float timeOut = m_pPeer->UpdateAndGetTimeOut(dt);

		if(timeOut > m_timeOut)
		{
			m_pPeer->m_isInactive = true;

			//If User Chooses to End the Connection
			if(m_pPeer->m_closeSocket == true)
			{
				m_status = false;

#if defined(ANDROID)
				close(pPeer->m_socket.m_socket);
#endif //ANDROID

#if defined(WIN32) || (_XBOX)
				shutdown(m_pPeer->m_socket.m_socket,SD_BOTH);
				closesocket(m_pPeer->m_socket.m_socket);
#endif //WIN32

			}
		}
	}

	void TurboClient::Close()
	{
		if(m_pPeer)
		{
			m_pPeer->Close();
			SAFE_DELETE_POINTER(m_pPeer);
			m_status = false;
		}
	}
}

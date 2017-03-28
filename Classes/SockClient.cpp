#include "SockClient.h"
#include <errno.h>

#if defined(ANDROID)
#include <netdb.h>
#endif //ANDROID

#if defined(WIN32)
DWORD WINAPI ConnectSocket(LPVOID lpParam)
{
	SockClient *pServer = (SockClient*)lpParam;
	pServer->_ConnectSocket();

	return NULL;
}
#endif //WIN32

#if defined(ANDROID)
void* ConnectSocket(void *ptr)
{
	SockClient *pServer = (SockClient*)ptr;
	pServer->_ConnectSocket();

	return NULL;
}
#endif //ANDROID

SockClient::SockClient(void)
	: m_status(false)
	, m_pPeer(nullptr)
{
}


SockClient::~SockClient(void)
{
}

void SockClient::ConnectToServer(unsigned int port)
{
	m_port = port;

#if defined(WIN32)
	CreateThread(NULL,0,ConnectSocket,this,0,NULL);
#endif

#if defined(ANDROID)
	pthread_t thread;
	pthread_create (&thread, NULL, &ConnectSocket, (void *) this);
#endif
}

void SockClient::_ConnectSocket()
{
	int m_connectSock;
	struct sockaddr_in m_connectAddr;

#if defined(WIN32)

	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);

#endif //Win32

	memset(&m_connectAddr,0,sizeof(sockaddr_in));

	m_connectSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

	//If socket fails
	if(m_connectSock < 0)
	{
		SocketLog("Socket Creation Failed");
		return;
	}

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

	m_connectAddr.sin_family = AF_INET;
	m_connectAddr.sin_port = htons(m_port);
	m_connectAddr.sin_addr.s_addr = pInIp->s_addr;
	m_connectAddr.sin_addr.s_addr |= htonl(0x1FF);

#if defined(ANDROID)
	m_connectAddr.sin_addr.s_addr = inet_addr("192.168.43.255");
#endif

	SocketLog("IP : %s",inet_ntoa(m_connectAddr.sin_addr));

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
#if defined(WIN32)
	unsigned long nonblock = 1;
	ioctlsocket(m_connectSock,FIONBIO,&nonblock);
#endif

	bool status = false;

	Socket_info info;
	memset(&info,0,sizeof(Socket_info));

	info.m_socket = m_connectSock;
	info.m_address = m_connectAddr;

	m_pPeer = new SockPeer;
	m_pPeer->CreatePeer(info);

	//While Connected to the Server
	while(true)
	{
		string message = "Hello Server";

		sendto(m_connectSock,message.c_str(),message.size(),0,(struct sockaddr*)&m_connectAddr,sizeof(sockaddr_in));

		char buffer[256];
		int sizeAddr = sizeof(sockaddr_in);

		//Give Sometime for the SendTo call, To Process.
#if defined(ANDROID)
		sleep(1);
#endif
#if defined(WIN32)
		Sleep(1);
#endif

		int recvSize = recvfrom(m_connectSock,buffer,256,0,(struct sockaddr*)&m_connectAddr,&sizeAddr);

		if(recvSize > 0)
		{
			buffer[recvSize] = '\0';
			m_pPeer->GetData()->InitWithDataString(buffer);

			m_pPeer->GetData()->GetValue("Connection",status);

			if(status)
			{
				m_status = true;
				return;
			}
		}

		SocketLog("Client Recieve Error : %s",strerror(errno));

#if defined(ANDROID)
		sleep(100);
#endif
#if defined(WIN32)
		Sleep(100);
#endif

	}
}

void SockClient::Close()
{
	if(m_pPeer)
	{
		m_pPeer->Close();
		SAFE_DELETE_POINTER(m_pPeer);
	}
}
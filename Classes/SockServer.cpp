#include "SockServer.h"
#include "SockPeer.h"

#if defined(WIN32)
DWORD WINAPI ListenSockets(LPVOID lpParam)
{
	SockServer *pServer = (SockServer*)lpParam;
	pServer->ListenForConnections();

	return NULL;
}
#endif //WIN32

#if defined(ANDROID)
void* ListenSockets(void *ptr)
{
	SockServer *pServer = (SockServer*)ptr;
	pServer->ListenForConnections();

	return NULL;
}
#endif //ANDROID

SockServer::SockServer(void)
	: m_listenSock(0)
	, m_port(0)
	, m_isSetup(false)
	, m_status(false)
	, m_maxClients(0)
{
}


SockServer::~SockServer(void)
{
}

bool SockServer::CreateServer(unsigned int port,unsigned int maxClients)
{
	m_maxClients = maxClients;

#if defined(WIN32)

	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);

#endif //Win32

	memset(&m_listenAddr,0,sizeof(sockaddr_in));

	m_listenSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

	//If socket fails
	if(m_listenSock < 0)
	{
		SocketLog("Socket Creation Failed");
		return false;
	}

	m_listenAddr.sin_family = AF_INET;
	m_listenAddr.sin_port = htons(port);
	m_listenAddr.sin_addr.s_addr = INADDR_ANY;

	int broadcast = 1;

	setsockopt(m_listenSock,SOL_SOCKET,SO_REUSEADDR,(char*)&broadcast,sizeof(int));

	int result = ::bind(m_listenSock,(sockaddr*)&m_listenAddr,sizeof(sockaddr_in));

	//If bind fails
	if(result)
	{
		SocketLog("bind Failed : %s",strerror(errno));
		return false;
	}

	m_isSetup = true;
	return true;
}

void SockServer::WaitForClients()
{
	if(!m_isSetup)
	{
		return;
	}

#if defined(WIN32)
	CreateThread(NULL,0,ListenSockets,this,0,NULL);
#endif

#if defined(ANDROID)
	pthread_t thread;
	pthread_create (&thread, NULL, &ListenSockets, (void *) this);
#endif
}

void SockServer::ListenForConnections()
{
	int size = sizeof(sockaddr_in);

	while(m_maxClients)
	{
		struct sockaddr_in connectedAddr;
		memset(&connectedAddr,0,sizeof(sockaddr_in));

		int recievedSize = 0;
		char buffer[256];

		//If Server Recieves a Connection
		recievedSize = recvfrom(m_listenSock,buffer,256,0,(struct sockaddr*)&connectedAddr,&size);

		if(recievedSize > 0)
		{
			int connectedSock = 0;
			connectedSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

#if defined(ANDROID)
			int nonblock = 1;
			ioctl(connectedSock,FIONBIO,&nonblock);
#endif
#if defined(WIN32)
			unsigned long nonblock = 1;
			ioctlsocket(connectedSock,FIONBIO,&nonblock);
#endif
			Socket_info info;
			memset(&info,0,sizeof(Socket_info));

			info.m_socket = connectedSock;
			info.m_address = connectedAddr;

			m_status = true;

			SocketLog("Successfully Connected to IP : %s",inet_ntoa(connectedAddr.sin_addr));

			SockPeer *pPeer = new SockPeer;
			pPeer->CreatePeer(info);

			//Send Success note to Client
			pPeer->GetData()->AddValue("Connection",true);
			pPeer->Flush();

			if(CheckListForIP(connectedAddr))
			{
				delete pPeer;
				continue;
			}

			m_peers.push_back(pPeer);
			m_maxClients--;

		}
		else
			SocketLog("Recieve Error : %s",strerror(errno));

#if defined(ANDROID)
		sleep(1);
#endif
#if defined(WIN32)
		Sleep(1);
#endif

	}
}

bool SockServer::CheckListForIP(struct sockaddr_in &sockAddr)
{
	vector<SockPeer*>::iterator it;

	it = m_peers.begin();

	while(it != m_peers.end())
	{
		SockPeer *pPeer = (*it);

		Socket_info *pSocket = pPeer->GetSocketInfo();

		char *buffer1 = inet_ntoa(pSocket->m_address.sin_addr);
		char *buffer2 = inet_ntoa(sockAddr.sin_addr);

		if(strcmp(buffer1,buffer2) == 0)
		{
			return true;
		}

		it++;
	}

	return false;
}

void SockServer::Close()
{
	vector<SockPeer*>::iterator it;

	it = m_peers.begin();

	while(it != m_peers.end())
	{
		SockPeer *pPeer = (*it);

		pPeer->Close();
		SAFE_DELETE_POINTER(pPeer);

		it++;
	}

	m_peers.clear();

	m_status = false;
}
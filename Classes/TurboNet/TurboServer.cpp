#include "TurboServer.h"

#if defined(WIN32) || (_XBOX)
DWORD WINAPI ListenSockets(LPVOID lpParam)
{
	TurboNet::TurboServer *pServer = (TurboNet::TurboServer*)lpParam;
	pServer->ListenForConnections();

	return NULL;
}
#endif //WIN32

#if defined(ANDROID)
void* ListenSockets(void *ptr)
{
	TurboNet::TurboServer *pServer = (TurboNet::TurboServer*)ptr;
	pServer->ListenForConnections();

	return NULL;
}
#endif //ANDROID

namespace TurboNet
{
	TurboServer::TurboServer()
		: m_listenSock(0)
		, m_port(0)
		, m_isSetup(false)
		, m_maxClients(0)
		, m_lastClock(0)
		, m_isThreadRunning(false)
	{
	}

	TurboServer::~TurboServer()
	{

	}

	bool TurboServer::CreateServer(unsigned int port,unsigned int maxClients,int timeOut /* = 5 */)
	{
		int result = 0;

		m_timeOut = timeOut;
		m_maxClients = maxClients;
		m_port = port;

#if defined(_XBOX)
		if( XNetStartup( NULL ) != 0 )
		{
			SocketLog("XNetStartup Failed");
		}
#endif

#if defined(WIN32) || (_XBOX)

		WSADATA wsa;
		result = WSAStartup(MAKEWORD(2,2),&wsa);

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

		result = setsockopt(m_listenSock,SOL_SOCKET,SO_BROADCAST,(char*)&broadcast,sizeof(int));

		result = ::bind(m_listenSock,(sockaddr*)&m_listenAddr,sizeof(sockaddr_in));

		//If bind fails
		if(result)
		{
			SocketLog("bind Failed : %s",strerror(errno));
			return false;
		}

#if defined(_XBOX)

		DWORD dwRet;
		do
		{
			dwRet = XNetGetTitleXnAddr( &m_xnAddr );
		} while( dwRet == XNET_GET_XNADDR_PENDING );

#endif

		m_isSetup = true;
		return true;
	}

	void TurboServer::WaitForClients()
	{
		if(!m_isSetup)
		{
			return;
		}

#if defined(WIN32) || (_XBOX)
		CreateThread(NULL,0,ListenSockets,this,0,NULL);
#endif

#if defined(ANDROID)
		pthread_t thread;
		pthread_create (&thread, NULL, &ListenSockets, (void *) this);
#endif
	}

	//Will Always run on a separate thread
	void TurboServer::ListenForConnections()
	{
		m_isThreadRunning = true;
		int size = sizeof(sockaddr_in);

#if defined(_XBOX)

		XNKID           xnkid;
		XNKEY           xnkey;

		if( XNetCreateKey( &xnkid, &xnkey ) != 0 )
		{
			SocketLog("Failed to Create a Key");
		}

		XNetRegisterKey( &xnkid, &xnkey );

#endif
		//If there are no Max Clients wait for a Connection
		while(m_peers.size() < m_maxClients)
		{
			struct sockaddr_in connectedAddr;
			memset(&connectedAddr,0,sizeof(sockaddr_in));

			int recievedSize = 0;
			char buffer[256];

			connectedAddr = m_listenAddr;
			//If Server Recieves a Connection
			recievedSize = recvfrom(m_listenSock,buffer,256,0,(sockaddr*)&connectedAddr,&size);

			if(recievedSize > 0)
			{
				int connectedSock = 0;
				connectedSock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

#if defined(ANDROID)
				int nonblock = 1;
				ioctl(connectedSock,FIONBIO,&nonblock);
#endif
#if defined(WIN32) || (_XBOX)
				unsigned long nonblock = 1;
				ioctlsocket(connectedSock,FIONBIO,&nonblock);
#endif

				Socket_info info;
				memset(&info,0,sizeof(Socket_info));

				info.m_socket = connectedSock;
				info.m_address = connectedAddr;

				SocketLog("Successfully Connected to IP : %s",inet_ntoa(connectedAddr.sin_addr));

				TurboPeer *pPeer = new TurboPeer;
				pPeer->CreatePeer(info);

#if defined(_XBOX)
				XBoxBuff pBuff;
				memset(&pBuff,0,sizeof(XBoxBuff));

				pBuff.m_xnaIp = m_xnAddr;
				pBuff.m_xnkey = xnkey;
				pBuff.m_xnkid = xnkid;

				SOCKADDR_IN sa;

				sa.sin_family = AF_INET;
				sa.sin_addr.s_addr = INADDR_BROADCAST;
				sa.sin_port = htons(1000);
				sendto(m_listenSock,(char*)&pBuff,sizeof(XBoxBuff),0,(sockaddr*)&sa,sizeof(sockaddr_in));
#else
				//Send Success note to Client
				pPeer->GetData()->AddValue("Connection",true);
				pPeer->Flush();
#endif


				if(CheckListForIP(connectedAddr))
				{
					delete pPeer;
					continue;
				}

				m_peers.push_back(pPeer);
			}
			else
				SocketLog("Recieve Error : %s",strerror(errno));

#if defined(ANDROID)
			sleep(1);
#endif
#if defined(WIN32) || (_XBOX)
			Sleep(1);
#endif
		}

		m_isThreadRunning = false;
	}

	void TurboServer::Update(float dt)
	{
		//If Max Clients are not connected, Create a thread and Start waiting.
		if(!m_isThreadRunning && m_peers.size() < m_maxClients)
		{
			WaitForClients();
		}

		vector<TurboPeer*>::iterator it;
		it = m_peers.begin();

		while(it != m_peers.end())
		{
			TurboPeer *pPeer = (*it);

			float timeOut = pPeer->UpdateAndGetTimeOut(dt);

			if(timeOut > m_timeOut)
			{
				//Set Inactive to True
				pPeer->m_isInactive = true;

				//If User Chooses to End the Connection
				if(pPeer->m_closeSocket == true)
				{

#if defined(ANDROID)
					close(pPeer->m_socket.m_socket);
#endif //ANDROID

#if defined(WIN32) || (_XBOX)
					shutdown(pPeer->m_socket.m_socket,SD_BOTH);
					closesocket(pPeer->m_socket.m_socket);
#endif //WIN32

				}
			}

			it++;
		}
	}

	bool TurboServer::CheckListForIP(struct sockaddr_in &sockAddr)
	{
		vector<TurboPeer*>::iterator it;

		it = m_peers.begin();

		while(it != m_peers.end())
		{
			TurboPeer *pPeer = (*it);

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

	bool TurboServer::RemovePeer(TurboPeer* pPeer)
	{
		vector<TurboPeer*>::iterator it;

		it = m_peers.begin();

		while(it != m_peers.end())
		{
			TurboPeer *peer = (*it);

			if(pPeer == peer)
			{
				SAFE_DELETE_POINTER(pPeer);
				it = m_peers.erase(it);
				
				return true;
			}

			it++;
		}

		return false;
	}

	void TurboServer::CloseAll()
	{
		vector<TurboPeer*>::iterator it;

		it = m_peers.begin();

		while(it != m_peers.end())
		{
			TurboPeer *pPeer = (*it);

			pPeer->Close();
			SAFE_DELETE_POINTER(pPeer);

			it++;
		}

		m_peers.clear();
	}
}
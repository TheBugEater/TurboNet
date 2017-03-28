#include "SockPeer.h"


SockPeer::SockPeer(void)
	: m_pData(nullptr)
	, m_status(false)
	, m_timeOut(0)
{
	memset(&m_socket,0,sizeof(Socket_info));
}


SockPeer::~SockPeer(void)
{
}

bool SockPeer::CreatePeer(Socket_info socketInfo)
{
	m_pData = new NetworkData();
	m_socket = socketInfo;

	m_status = true;

	int buffSize = 16 * 1024;
	if (setsockopt(m_socket.m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&buffSize, sizeof(int)) == -1)
	{
		SocketLog("SockClient select Error : %s ", strerror(errno));
	}

	return true;
}

void SockPeer::Flush()
{
	string message = m_pData->GetDataString();

	fd_set writefds;
	FD_ZERO(&writefds);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_SET(m_socket.m_socket,&writefds);

	if(select(m_socket.m_socket + 1, NULL,&writefds, NULL, &tv) <= 0)
	{
		SocketLog("SockClient select Error : %s ", strerror(errno));
		return;
	}

	if(!FD_ISSET(m_socket.m_socket, &writefds))
	{
		SocketLog("SockClient FD_ISSET Error : %s ", strerror(errno));
		return;
	}

	int result = 0;

	/*int size = message.size();
	int currPos = 0;
	int nextChunkSize = 1024;

	while(true)
	{
		std::string substring;

		if(currPos > nextChunkSize + size)
			nextChunkSize = size - currPos;

		substring = string( message.substr(currPos, nextChunkSize) );

		result = sendto(m_socket.m_socket,substring.c_str(),substring.size(),0,(struct sockaddr*)&m_socket.m_address,sizeof(sockaddr_in));

		currPos += nextChunkSize;

		if(currPos >= size)
		{
			break;
		}

		if(result < 0)
		{
			break;
		}
	}*/
	
	result = sendto(m_socket.m_socket,message.c_str(),message.size(),0,(struct sockaddr*)&m_socket.m_address,sizeof(sockaddr_in));
	
	if(result < 0)
	{
		SocketLog("SockClient Send Error : %s ", strerror(errno));
	}
	else
	{
		//send(m_socket.m_socket,"\0",1,0);
	}
}

bool SockPeer::Read()
{
	string dataString;
	int size = 0;
	int tempSize = 1;

	fd_set readfds;
	FD_ZERO(&readfds);
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	FD_SET(m_socket.m_socket,&readfds);

	if(select(m_socket.m_socket + 1, &readfds, NULL, NULL, &tv) <= 0)
	{
		return false;
	}

	if(!FD_ISSET(m_socket.m_socket, &readfds))
	{
		return false;
	}

#if defined (WIN32)
	u_long count;
	ioctlsocket(m_socket.m_socket, FIONREAD, &count);
#endif	//WIN32

#if defined (ANDROID)
	int count = 0;
	ioctl(m_socket.m_socket, FIONREAD, &count);
#endif	//WIN32

	if(count <= 0)
		return false;

	char *data = new char[count+1];

	while(tempSize > 0)
	{
		int sizeAddr = sizeof(sockaddr_in);

		tempSize = recvfrom(m_socket.m_socket,data,count,0,(struct sockaddr*)&m_socket.m_address,&sizeAddr);

		if(tempSize > 0)
		{
			data[tempSize] = '\0';
			dataString.append(data);
			size += tempSize;
		}
	}

	SAFE_DELETE_POINTER(data);

	if(size <= 0)
	{
		SocketLog("SockClient Recv Error : %s ", strerror(errno));
		return false;
	}
	else
	{
		m_pData->InitWithDataString(dataString.c_str());
	}

	return true;
}

const char* SockPeer::GetPeerIP()
{
	const char *pIp = inet_ntoa(m_socket.m_address.sin_addr);

	return pIp;
}

bool SockPeer::Close()
{
#if defined(ANDROID)
	close(m_socket.m_socket);
#endif //ANDROID

#if defined(WIN32)
	shutdown(m_socket.m_socket,SD_BOTH);
	closesocket(m_socket.m_socket);
#endif //WIN32

	SAFE_DELETE_POINTER(m_pData);

	return true;
}
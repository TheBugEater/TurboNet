#ifndef __TURBO_SERVER_H__
#define __TURBO_SERVER_H__

#include "Framework/TurboNet.h"
#include "TurboPeer.h"
#include <string>

using namespace std;

namespace TurboNet
{
	class TURBO_DLL TurboServer
	{
	public:
		TurboServer();
		~TurboServer();

		bool							CreateServer(unsigned int port,unsigned int maxClients,int timeOut = 5);

		bool							GetStatus()		{ return m_peers.size() > 0; }

		//Don't Call this, This is Called Internally for Cross Platform Support
		void							ListenForConnections();

		void							CloseAll();

		vector<TurboPeer*>*				GetPeers()		{ return &m_peers; }
		bool							RemovePeer(TurboPeer* pPeer);

		void							Update(float dt);

	private:

		void							WaitForClients();
		bool							CheckListForIP(struct sockaddr_in &sockAddr);

		bool							m_isSetup;
		string							m_ipAddress;
		unsigned int					m_port;

		int								m_listenSock;
		struct sockaddr_in				m_listenAddr;
		unsigned int					m_maxClients;
		long							m_lastClock;

		int								m_timeOut;

		vector<TurboPeer*>				m_peers;

		bool							m_isThreadRunning;

#if defined(_XBOX)
		XNADDR							m_xnAddr;
#endif
	};
}

#endif //__TURBO_SERVER_H__
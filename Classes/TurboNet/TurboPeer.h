#ifndef __TURBO_PEER_H__
#define __TURBO_PEER_H__

#include "Framework/TurboNet.h"
#include "BytePack.h"
#include <string>

using namespace std;

namespace TurboNet
{
	class TURBO_DLL TurboPeer
	{
	public:
		TurboPeer();
		~TurboPeer();

		bool							CreatePeer(Socket_info socketInfo);

		void							Flush();
		bool							Read();

		bool							Close();

		BytePack*						GetData() const { return m_pData; }

		Socket_info*					GetSocketInfo() { return &m_socket; };

		const char*						GetPeerIP		(	);

		bool							IsInactive() { return m_isInactive;	}

	private:
		float							UpdateAndGetTimeOut	(float dt);

		friend	class					SockServer;
		friend	class					TurboClient;

		Socket_info						m_socket;
		BytePack*						m_pData;

		bool							m_status;
		float							m_timeOut;

		bool							m_closeSocket;
		bool							m_isInactive;

		friend class					TurboServer;
	};
}

#endif //__TURBO_PEER_H__
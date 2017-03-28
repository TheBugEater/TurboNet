#ifndef __TURBO_CLIENT_H__
#define __TURBO_CLIENT_H__

#include "Framework/TurboNet.h"
#include <string>
#include "TurboPeer.h"

using namespace std;

#if defined(WIN32) || (_XBOX)
static DWORD WINAPI ConnectSocket(void* lpParam);
#endif //Win32

#if defined(ANDROID)
static void* ConnectSocket(void *ptr);
#endif //ANDROID

namespace TurboNet
{
	class TURBO_DLL TurboClient
	{
	public:
		TurboClient(void);
		~TurboClient(void);

		void								ConnectToServer(unsigned int port,int timeOut);

		bool								GetStatus() { return m_status; }

		TurboPeer*							GetPeer() { return m_pPeer; }

		///Dont Call this Function, It's used internally.
		void								_ConnectSocket();

		void								Close();

		void								Update(float dt);
	private:
		
		bool								m_status;
		int									m_timeOut;

		bool								m_isThreadRunning;

		//string m_ipAddr;
		int									m_port;
		TurboPeer*							m_pPeer;
	};
}

#endif //__TURBO_CLIENT_H__
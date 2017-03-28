#pragma once
#ifndef __SOCK_PEER_H__
#define __SOCK_PEER_H__

#include "SockServer.h"
#include "NetworkData.h"

class SockPeer
{
public:
	SockPeer(void);
	~SockPeer(void);

	bool					CreatePeer(Socket_info socketInfo);

	void					Flush();
	bool					Read();

	bool					Close();

	NetworkData*			GetData() const { return m_pData; }

	Socket_info*			GetSocketInfo() { return &m_socket; };

	const char*				GetPeerIP		(	);

private:
	Socket_info				m_socket;
	NetworkData*			m_pData;

	bool					m_status;
	float					m_timeOut;
};

#endif
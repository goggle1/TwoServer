#ifndef __NFSPSERVER_H__
#define __NFSPSERVER_H__

#include "NfspPeer.h"
#include "TcpServer.h"

/*
NFSP: new funshion protocol, which is similiar to BT protocol.
*/
class NfspServer : public TcpServer
{
public:
	NfspServer();
	virtual ~NfspServer();
	virtual TcpSession*   	GetSession(int fd, struct sockaddr_in* addr, TaskThread* threadp);	
};

extern NfspServer*			g_nfsp_server;

#endif
#include <errno.h>
#include <unistd.h>

#include "NfspServer.h"

NfspServer::NfspServer()
{
}

NfspServer::~NfspServer()
{
}

TcpSession* NfspServer::GetSession(int fd, struct sockaddr_in * addr, TaskThread* threadp)
{
	NfspPeer* peerp = new NfspPeer(fd, addr);
	peerp->Init(threadp);
	return peerp;
}


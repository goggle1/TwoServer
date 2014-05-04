#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#include "NfspServer.h"

NfspServer::NfspServer()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

NfspServer::~NfspServer()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

TcpSession* NfspServer::GetSession(int fd, struct sockaddr_in * addr, TaskThread* threadp)
{
	NfspPeer* peerp = new NfspPeer(fd, addr);
	peerp->Init(threadp);
	return peerp;
}


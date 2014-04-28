#ifndef __NFSPPEER_H__
#define __NFSPPEER_H__

#include "TaskThread.h"
#include "TcpSession.h"

class NfspPeer : public TcpSession
{
public:
	NfspPeer(int fd, struct sockaddr_in* addr);
	virtual ~NfspPeer();
	int				DoRead();
	virtual	int		DoEvents(u_int32_t events, TaskThread* threadp);

protected:

};

#endif
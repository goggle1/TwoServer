#include <errno.h>
#include <unistd.h>

#include "NfspServer.h"

NfspServer::NfspServer()
{
}

NfspServer::~NfspServer()
{
}

int NfspServer::DoRead(TaskThread* threadp)
{
	int ret = 0;

	while(1)
	{
		struct sockaddr_in addr;
		socklen_t size = sizeof(addr);
		int a_fd = accept(m_SockFd, (struct sockaddr*)&addr, &size);
		if (a_fd == -1)
		{
			int acceptError = errno;
			//fprintf(stderr, "%s[%p][%p]: errno=%d, %s\n", __PRETTY_FUNCTION__, this, threadp, acceptError, strerror(acceptError));
			if (acceptError == EAGAIN)
			{	
				return 0;
			}				
			else if (acceptError == EMFILE || acceptError == ENFILE)
			{	
				exit (-1);	
			}
			else
			{
				// do nothing.
			}
		}
		else
		{
			NfspPeer* peerp = this->GetPeer(a_fd, &addr, threadp);
			if (peerp == NULL)
			{ 
				close(a_fd);				
			}			
		}
	}
	
	return ret;
}

NfspPeer* NfspServer::GetPeer(int fd, struct sockaddr_in * addr, TaskThread* threadp)
{
	NfspPeer* peerp = new NfspPeer(fd, addr);
	peerp->Init();
	return peerp;
}


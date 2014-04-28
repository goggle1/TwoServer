#include "NfspPeer.h"

NfspPeer::NfspPeer(int fd,struct sockaddr_in * addr) :
	TcpSession(fd, addr)
{
	
}

NfspPeer::~NfspPeer()
{
	
}

int NfspPeer::DoRead()
{
	int ret = 0;

	//todo:

	return ret;
}

int NfspPeer::DoEvents(u_int32_t events,TaskThread * threadp)
{
	int ret = 0;

	if(events & EPOLLERR)
	{
		delete this;
		return 0;
	}
	if(events & EPOLLHUP)
	{
		delete this;
		return 0;
	}
	if(events & EVENT_READ)
	{
		ret = DoRead();	
		if(ret < 0)
		{
			delete this;
			return 0;
		}
		else if(ret == 0)
		{
			// do nothing.
		}
		else
		{
			m_task_thread->m_EventsMaster.ModifyWatch(m_SockFd, EVENT_READ|EVENT_WRITE, this);
		}
	}

	return ret;
}

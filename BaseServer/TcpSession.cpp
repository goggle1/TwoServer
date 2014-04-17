#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"
#include "events.h"
#include "TaskThread.h"
#include "TcpSession.h"

TcpSession::TcpSession(int fd, struct sockaddr_in * addr)
{
	m_SockFd = fd;	
	memcpy(&m_SockAddr, addr, sizeof(struct sockaddr_in));
	
	m_StrReceived.Ptr = m_RequestBuffer;
	m_StrReceived.Len = 0;
	fprintf(stdout, "%s[%p]: fd=%d, 0x%08X:%u\n", __PRETTY_FUNCTION__, this, 
		m_SockFd, m_SockAddr.sin_addr.s_addr, m_SockAddr.sin_port);
}

TcpSession::~TcpSession()
{	
	fprintf(stdout, "%s[%p][%p]: fd=%d, 0x%08X:%u\n", __PRETTY_FUNCTION__, this, m_task_thread,
		m_SockFd, m_SockAddr.sin_addr.s_addr, m_SockAddr.sin_port);
	if(m_SockFd != -1)
	{
		close(m_SockFd);
		m_SockFd = -1;
	}
}

int TcpSession::Init(TaskThread* threadp)
{
	m_task_thread = threadp;
	fprintf(stdout, "%s[%p]: task_thread=%p\n", __FUNCTION__, this, m_task_thread);
	
	//set options on the socket
    //we are a server, always disable nagle algorithm
    int one = 1;
    int err = ::setsockopt(m_SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int));
    //AssertV(err == 0, OSThread::GetErrno());
    
    err = ::setsockopt(m_SockFd, SOL_SOCKET, SO_KEEPALIVE, (char*)&one, sizeof(int));
    //AssertV(err == 0, OSThread::GetErrno());

    int sndBufSize = 96L * 1024L;
    err = ::setsockopt(m_SockFd, SOL_SOCKET, SO_SNDBUF, (char*)&sndBufSize, sizeof(int));
    //AssertV(err == 0, OSThread::GetErrno());

	// InitNonBlocking
   	int flag = ::fcntl(m_SockFd, F_GETFL, 0);
    err = ::fcntl(m_SockFd, F_SETFL, flag | O_NONBLOCK);
        
	int ret = m_task_thread->m_EventsMaster.AddWatch(m_SockFd, EVENT_READ, this);
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: g_events_master->AddWatch [%d], return %d\n", __FUNCTION__, this, m_SockFd, ret);
		return -1;
	}
	
	return 0;
}

void TcpSession::Release()
{
	fprintf(stdout, "%s[%p]: \n", __PRETTY_FUNCTION__, this);
	int ret = 0;
	ret = m_task_thread->m_EventsMaster.DeleteWatch(m_SockFd);
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: g_events_master->DeleteWatch [%d], return %d\n", __FUNCTION__, this, m_SockFd, ret);
	}
	
}

int TcpSession::DoRead()
{
	char r_buffer[1024];
	size_t len = 1024;

	char s_buffer[1024];
	snprintf(s_buffer, 1024, "%s: I got it.\n", __PRETTY_FUNCTION__);
	size_t s_len = strlen(s_buffer);
	
	while(1)
	{
		ssize_t r_ret = recv(m_SockFd, r_buffer, len, 0);
		if(r_ret == 0)
		{
			fprintf(stdout, "%s: recv %ld, from fd=%d\n", __PRETTY_FUNCTION__, r_ret, m_SockFd);
			return -1;
		}
		else if(r_ret < 0)
		{
			int err = errno;
			fprintf(stderr, "%s: errno=%d, %s\n", __PRETTY_FUNCTION__, err, strerror(err));
			if(err == EAGAIN) // or other errno
			{
				return 0;
			}
			else
			{
				fprintf(stdout, "%s: recv %ld, close fd=%d\n", __PRETTY_FUNCTION__, r_ret, m_SockFd);
				return -1;
			}
		}
		r_buffer[r_ret] = '\0';
		fprintf(stdout, "%s: recv %ld,\n%s\n", __PRETTY_FUNCTION__, r_ret, r_buffer);
		
		ssize_t s_ret = send(m_SockFd, s_buffer, s_len, 0);	
		if(s_ret == -1)
		{
			int err = errno;
			fprintf(stderr, "%s: errno=%d, %s\n", __PRETTY_FUNCTION__, err, strerror(err));
			if(err == EAGAIN) // or other errno
			{
				return 0;
			}
			else
			{
				fprintf(stdout, "%s: send %ld, from fd=%d\n", __PRETTY_FUNCTION__, s_ret, m_SockFd);
				return -1;
			}
		}
		fprintf(stdout, "%s: send %ld, return %ld, %s\n", __PRETTY_FUNCTION__, s_len, s_ret, s_buffer);

		//g_timer_thread->SetTimeout(this, 5000);
		
	}
	
	return 0;
}


int TcpSession::DoTimeout()
{
	int ret = 0;

	fprintf(stdout, "%s[%p]: \n", __PRETTY_FUNCTION__, this);
	
	return -1;
}


int TcpSession::Run()
{	
	int ret = 0;
	
	while(1)
	{		
		u_int64_t events = 0;
		ret = DequeEvents(events);
		if(ret < 1)
		{
			return 0;
		}
		
		fprintf(stdout, "%s: events=0x%lx\n", __PRETTY_FUNCTION__, events);
		if(events & EVENT_READ)
		{
			ret = DoRead();			
		}
		else if(events & EVENT_CONTINUE)
		{
			//ret = DoContinue();			
		}
		else if(events & EVENT_TIMEOUT)
		{
			ret = DoTimeout();			
		}
		
		if(ret < 0)
		{
			return ret;
		}
	}
	
	return ret;
}


int TcpSession::DoEvents(u_int32_t events, TaskThread* threadp)
{
	// todo:
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "common.h"
#include "TaskThread.h"
#include "TcpSession.h"

TcpSession::TcpSession(int fd, struct sockaddr_in * addr) :
	m_StrResponse((char*)m_ResponseBuffer, 0),
    m_StrRemained(m_StrResponse)
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
	Close();
}

int TcpSession::Init(TaskThread* threadp)
{
	int ret = 0;
	
	m_task_thread = threadp;
	fprintf(stdout, "%s[%p]: task_thread=%p\n", __FUNCTION__, this, m_task_thread);
	
	//set options on the socket
    //we are a server, always disable nagle algorithm
    int one = 1;
    ret = ::setsockopt(m_SockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int));
    if(ret < 0)
    {
    	return -1;
    }
    
    ret = ::setsockopt(m_SockFd, SOL_SOCKET, SO_KEEPALIVE, (char*)&one, sizeof(int));
    if(ret < 0)
    {
    	return -2;
    }

    int sndBufSize = 96L * 1024L;
    ret = ::setsockopt(m_SockFd, SOL_SOCKET, SO_SNDBUF, (char*)&sndBufSize, sizeof(int));
    if(ret < 0)
    {
    	return -3;
    }

#if 1
    struct linger              linger;
    linger.l_onoff = 1;
    linger.l_linger = 0;
    ret = ::setsockopt(m_SockFd, SOL_SOCKET, SO_LINGER, (const void *) &linger, sizeof(struct linger));
    if(ret < 0)
    {
    	return -4;
    }

#endif
    
	// InitNonBlocking
   	int flag = ::fcntl(m_SockFd, F_GETFL, 0);
    ret = ::fcntl(m_SockFd, F_SETFL, flag | O_NONBLOCK);
    if(ret < 0)
    {
    	return -5;
    }
        
	ret = m_task_thread->m_EventsMaster.AddWatch(m_SockFd, EVENT_READ, this);
	if(ret < 0)
	{
		fprintf(stderr, "%s[%p]: g_events_master->AddWatch [%d], return %d\n", __FUNCTION__, this, m_SockFd, ret);
		return -6;
	}
	
	return 0;
}

void TcpSession::Close()
{
	if(m_SockFd != -1)
	{
		close(m_SockFd);
		m_SockFd = -1;
	}
}

// -1, error
// 0,  ok
int TcpSession::RecvData()
{	
	while(1)
	{
		char* 	recv_bufferp	= m_RequestBuffer + m_StrReceived.Len;
		ssize_t recv_buff_size	= REQUEST_BUFF_SIZE - m_StrReceived.Len;
		if(recv_buff_size <= 0)
		{
			fprintf(stderr, "%s[%p]: recv buffer full, recv_buff_size=%ld\n", __PRETTY_FUNCTION__, this, recv_buff_size);
			return -1;
		}
		
		ssize_t recv_ret = recv(m_SockFd, recv_bufferp, recv_buff_size, 0);
		if(recv_ret == 0)
		{
			fprintf(stdout, "%s[%p]: recv=%ld, from fd=%d\n", __PRETTY_FUNCTION__, this, recv_ret, m_SockFd);
			return -1;
		}
		else if(recv_ret < 0)
		{
			int err = errno;
			fprintf(stderr, "%s[%p]: errno=%d, %s\n", __PRETTY_FUNCTION__, this, err, strerror(err));
			if(err == EAGAIN) // or other errno
			{
				return 0;
			}
			else
			{
				fprintf(stderr, "%s[%p]: recv=%ld, from fd=%d\n", __PRETTY_FUNCTION__, this, recv_ret, m_SockFd);
				return -1;
			}
		}
		fprintf(stdout, "%s[%p]: size=%ld, recv=%ld\n", __PRETTY_FUNCTION__, this, recv_buff_size, recv_ret);
		//my_printf(stdout, recv_bufferp, recv_ret);
		m_StrReceived.Len += recv_ret;		
	}

	return 0;
	
}

// -1, error
// 0, ok
// 1, to be continue.
int TcpSession::SendData()
{  	
	int ret = 0;
	if(m_StrRemained.Len <= 0)
    {
    	return 0;
    }  	

	int should_send_len = m_StrRemained.Len;
	ret = send(m_SockFd, m_StrRemained.Ptr, should_send_len, 0);
    if(ret > 0)
    {        
    	fprintf(stdout, "%s[%p]: send %d return %d\n", 
            __PRETTY_FUNCTION__, this, should_send_len, ret);
        m_StrRemained.Ptr += ret;
        m_StrRemained.Len -= ret;
        ::memmove(m_ResponseBuffer, m_StrRemained.Ptr, m_StrRemained.Len);
        m_StrRemained.Ptr = m_ResponseBuffer; 

        if(m_StrRemained.Len <= 0)
	    {
	    	return 0;
	    }  	
	    else
	    {
	    	return SEND_TO_BE_CONTINUE;
	    }
    }
    else
    {
    	int err = errno;
        fprintf(stderr, "%s[%p]: send %d return %d, errno=%d, %s\n", 
            __PRETTY_FUNCTION__, this, should_send_len, ret, err, strerror(err));
        if(err == EAGAIN)
        {
        	return SEND_TO_BE_CONTINUE;
        }
        else // EPIPE, ECONNRESET
        {
        	close(m_SockFd);
        	m_SockFd = -1;
        	return -1;
        }
    }

    return 0;
}


bool TcpSession::IsFullRequest()
{
	if(m_StrReceived.Len <= 0)
	{
		return false;
	}

	m_StrRequest.Ptr = m_StrReceived.Ptr;
	m_StrRequest.Len = m_StrReceived.Len;	
	return true;
}


int TcpSession::DoRequest()
{
	int ret = 0;

	memcpy(m_ResponseBuffer, m_StrRequest.Ptr, m_StrRequest.Len);
	m_StrResponse.Ptr = m_ResponseBuffer;
	m_StrResponse.Len = m_StrRequest.Len;
	m_StrRemained.Ptr = m_StrResponse.Ptr;
	m_StrRemained.Len = m_StrResponse.Len;
	
	return ret;
}


void TcpSession::MoveOnRequest()
{
    StrPtrLen   strRemained;
    strRemained.Set(m_StrRequest.Ptr+m_StrRequest.Len, m_StrReceived.Len-m_StrRequest.Len);
        
    ::memmove(m_RequestBuffer, strRemained.Ptr, strRemained.Len);
    m_StrReceived.Set(m_RequestBuffer, strRemained.Len);
    m_StrRequest.Set(m_RequestBuffer, 0);
}

int TcpSession::DoRead()
{	
	int ret = 0;
	ret = RecvData();
	if(ret < 0)
	{
		return ret;
	}
		
	while(1)
	{
		if(IsFullRequest())
		{
			ret = DoRequest();
			if(ret < 0)
			{
				return ret;	
			}

			MoveOnRequest();

			ret = SendData();
			if(ret != 0)
			{
				return ret;
			}			
		}	
		else
		{
			break;
		}
	}
	
	return ret;
}


int TcpSession::DoEvents(u_int32_t events, TaskThread* threadp)
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
	if(events & EVENT_WRITE)
	{
		// todo:
	}
	
	return 0;
}

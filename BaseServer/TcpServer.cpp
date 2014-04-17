#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "events.h"
#include "TcpServer.h"

TcpServer::TcpServer()
{
	m_SockFd = -1;
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

TcpServer::~TcpServer()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

int TcpServer::DoRead()
{
	int ret = 0;

	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);

	while(1)
	{
		struct sockaddr_in addr;
		socklen_t size = sizeof(addr);
		int a_fd = accept(m_SockFd, (struct sockaddr*)&addr, &size);
		if (a_fd == -1)
		{
	        int acceptError = errno;
	        fprintf(stderr, "%s[%p]: errno=%d, %s\n", __PRETTY_FUNCTION__, this, acceptError, strerror(acceptError));
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
	    	TcpSession* sessionp = this->GetSession(a_fd, &addr);
		    if (sessionp == NULL)
		    { 
		        close(a_fd);		        
		    }		    
	    }
    }
	
	return ret;
}

int TcpServer::Run()
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
		
		fprintf(stdout, "%s[%p]: events=0x%lx\n", __PRETTY_FUNCTION__, this, events);
		if(events & EVENT_READ)
		{
			ret = DoRead(); 		
		}
		// else(other event)
		
		if(ret < 0)
		{
			return ret;
		}
	}
	
	return ret;
}


int TcpServer::Init(u_int32_t ip, u_int16_t port)
{
	int ret = 0;

	m_SockFd = ::socket(PF_INET, SOCK_STREAM, 0);
	if(m_SockFd < 0)
	{
		return -1;
	}

	int flag = ::fcntl(m_SockFd, F_GETFL, 0);
    ret = ::fcntl(m_SockFd, F_SETFL, flag | O_NONBLOCK);
    if(m_SockFd < 0)
	{
		return -2;
	}
    
    int one = 1;
    ret = ::setsockopt(m_SockFd, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(int));
    if(m_SockFd < 0)
	{
		return -3;
	}

	struct sockaddr_in  fLocalAddr;
    ::memset(&fLocalAddr, 0, sizeof(fLocalAddr));
    fLocalAddr.sin_family = AF_INET;
    fLocalAddr.sin_port = htons(port);
    fLocalAddr.sin_addr.s_addr = htonl(ip);    
    ret = ::bind(m_SockFd, (sockaddr *)&fLocalAddr, sizeof(fLocalAddr));
    

    int bufSize = 96*1024;
    ret = ::setsockopt(m_SockFd, SOL_SOCKET, SO_RCVBUF, (char*)&bufSize, sizeof(int));
    if(ret < 0)
	{
		return -4;
	}

    ret = ::listen(m_SockFd, 511);
    if(ret < 0)
	{
		return -5;
	}
	
	return ret;
}

TcpSession* TcpServer::GetSession(int fd, struct sockaddr_in * addr)
{
	TcpSession* sessionp = new TcpSession(fd, addr);
	sessionp->Init();
	return sessionp;
}

int TcpServer::DoEvents(u_int32_t events, TaskThread* threadp)
{
	// todo:
	return 0;
}


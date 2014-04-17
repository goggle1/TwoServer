#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "events.h"
#include "HttpServer.h"

HttpServer::HttpServer()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

HttpServer::~HttpServer()
{
	fprintf(stdout, "%s[%p]:\n", __PRETTY_FUNCTION__, this);
}

int HttpServer::DoRead(TaskThread* threadp)
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
			fprintf(stderr, "%s[%p][%p]: errno=%d, %s\n", __PRETTY_FUNCTION__, this, threadp, acceptError, strerror(acceptError));
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
			HttpSession* sessionp = this->GetSession(a_fd, &addr, threadp);
			if (sessionp == NULL)
			{ 
				close(a_fd);				
			}			
		}
	}
	
	return ret;
}


int HttpServer::Run()
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


HttpSession* HttpServer::GetSession(int fd, struct sockaddr_in * addr, TaskThread* threadp)
{
	HttpSession* sessionp = new HttpSession(fd, addr);
	sessionp->Init(threadp);
	return sessionp;
}

int 	HttpServer::DoEvents(u_int32_t events, TaskThread* threadp)
{
	int ret = 0;
	
	fprintf(stdout, "%s[%p]: events=0x%x, thread=%p\n", __PRETTY_FUNCTION__, this, events, threadp);
	if(events & EVENT_READ)
	{
		ret = DoRead(threadp); 		
	}
	
	return 0;
}

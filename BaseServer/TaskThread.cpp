#include <unistd.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/epoll.h>

#include "OS.h"

#include "TcpServer.h"
#include "HttpServer.h"
#include "NfspServer.h"

#include "TaskThread.h"

TaskThread::TaskThread()
{

}

TaskThread::~TaskThread()
{

}


int TaskThread::Init()
{
	int ret = 0;
	
	m_EventsMaster.Init();
	/*
	m_EventsMaster.AddWatch(g_tcp_server->GetFd(),  EVENT_READ, g_tcp_server);
	fprintf(stdout, "%s[%p]: AddWatch fd=%d\n", __PRETTY_FUNCTION__, this, g_tcp_server->GetFd());
	m_EventsMaster.AddWatch(g_http_server->GetFd(), EVENT_READ, g_http_server);
	fprintf(stdout, "%s[%p]: AddWatch fd=%d\n", __PRETTY_FUNCTION__, this, g_http_server->GetFd());
	*/
	return 0;
}

int TaskThread::Entry()
{
	prctl(PR_SET_NAME, "twoserver_work");
	
	struct epoll_event events[MAX_EVENTS];    
    while(1) 
    {    	
    	g_http_server->DoRead(this);
    	g_nfsp_server->DoRead(this);
    	
        int num = epoll_wait(m_EventsMaster.m_epoll_fd, events, MAX_EVENTS, WAIT_PERIOD);
        //fprintf(stdout, "%s[%p]: epoll_wait num=%d\n", __PRETTY_FUNCTION__, this, num);
		for(int index = 0; index < num; ++index) 
		{			 
			Task* taskp = (Task*)events[index].data.ptr;
			taskp->DoEvents(events[index].events, this);
		}		
	}
	
	return 0;
}


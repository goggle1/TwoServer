#include <unistd.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/epoll.h>

#include "OS.h"
#include "events.h"

#include "TcpServer.h"
#include "HttpServer.h"

#include "TaskThread.h"

TaskThread::TaskThread()
{
	dequeh_init(&m_task_queue);
}

TaskThread::~TaskThread()
{
	dequeh_release(&m_task_queue, task_release);
}


int TaskThread::Init()
{
	int ret = 0;
	
	m_EventsMaster.Init();
	m_EventsMaster.AddWatch(g_tcp_server->GetFd(),  EVENT_READ, g_tcp_server);
	fprintf(stdout, "%s[%p]: AddWatch fd=%d\n", __PRETTY_FUNCTION__, this, g_tcp_server->GetFd());
	m_EventsMaster.AddWatch(g_http_server->GetFd(), EVENT_READ, g_http_server);
	fprintf(stdout, "%s[%p]: AddWatch fd=%d\n", __PRETTY_FUNCTION__, this, g_http_server->GetFd());
	return 0;
}

int TaskThread::EnqueTask(Task * taskp)
{
	int ret = 0;
	{
        OSMutexLocker theLocker(&fMutex);
        ret = dequeh_append(&m_task_queue, taskp);
        if(ret < 0)
        {
    		return ret;
    	}
    }    
    	
    fCond.Signal();
	return ret;
}

Task* TaskThread::WaitForTask()
{
	while(1)
	{	
		int64_t theCurrentTime = OS::Milliseconds();
		
		OSHeapElem* elemp = fHeap.PeekMin();
        if (elemp != NULL)
        {	 
        	int64_t theTaskTime = elemp->GetValue();
        	if(theTaskTime <= theCurrentTime)
        	{
        		elemp = fHeap.ExtractMin();
        		Task* taskp = (Task*)elemp->GetEnclosingObject();     
        		taskp->EnqueEvents(EVENT_CONTINUE);
        		return taskp;
            }
        }

        //if there is an element waiting for a timeout, figure out how long we should wait.
        int64_t theTimeout = 0;
        if (elemp != NULL)
        { 
        	int64_t theTaskTime = elemp->GetValue();
            theTimeout = theTaskTime - theCurrentTime;
        }
        
        //
        // Make sure we can't go to sleep for some ridiculously short
        // period of time
        // Do not allow a timeout below 10 ms without first verifying reliable udp 1-2mbit live streams. 
        // Test with streamingserver.xml pref reliablUDP printfs enabled and look for packet loss and check client for  buffer ahead recovery.
	    if (theTimeout < 10) 
	    {
           theTimeout = 10;
        }
            
        //wait...        
        OSMutexLocker theLocker(&fMutex);
        if (m_task_queue.count == 0) 
        {
        	fCond.Wait(&fMutex, theTimeout);
        }    
        
		Task* taskp = (Task*)dequeh_remove_head(&m_task_queue);
		return taskp;
	}

	return NULL;
}

int TaskThread::Entry()
{
	prctl(PR_SET_NAME, "oneserver_work");
	
	struct epoll_event events[MAX_EVENTS];    
    while(1) 
    {
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


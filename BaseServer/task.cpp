#include <stdio.h>

#include "task.h"
#include "TaskThread.h"
#include "TaskThreadPool.h"

Task::Task() : 
	fTimerHeapElem(this),
	fIdleElem(this)
{	
	m_IsValid = true;
	m_task_thread = NULL;
	dequeh_init(&m_EventsQueue);
	//fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
}

Task::~Task()
{
	dequeh_release(&m_EventsQueue, NULL);
	//fprintf(stdout, "%s\n", __PRETTY_FUNCTION__);
}

int Task::Attach()
{
	// if no task thread, select one thread, then send signal to task thread.
	// otherwise, send signal to task thread.
	if(m_task_thread == NULL)
	{
		m_task_thread = g_task_thread_pool->SelectThread(THREAD_SHORT);
		m_task_thread->EnqueTask(this);
	}
	
	return 0;
}

void Task::Detach()
{
	OSMutexLocker theLocker(&fMutex);
	m_task_thread = NULL;
}

int Task::EnqueEvents(u_int64_t events)
{	
	fprintf(stdout, "%s[%p]: events=0x%lx\n", __PRETTY_FUNCTION__, this, events);
	OSMutexLocker theLocker(&fMutex);
	if(m_IsValid)
	{
	    dequeh_append(&m_EventsQueue, (void*)events);    
		this->Attach();
		return 0;
	}
	
	return -1;
}

int Task::DequeEvents(u_int64_t& events)
{
	OSMutexLocker theLocker(&fMutex);	
	void* elementp = dequeh_remove_head(&m_EventsQueue);
	if(elementp == NULL)
	{
		fprintf(stdout, "%s[%p]: events=[null], thread=%p\n", __PRETTY_FUNCTION__, this, m_task_thread);
		return 0;
	}
	
	events = (u_int64_t)elementp;
	fprintf(stdout, "%s[%p]: events=0x%lx\n", __PRETTY_FUNCTION__, this, events);
	return 1;		
}

void Task::Release()
{
	fprintf(stdout, "%s[%p]: \n", __PRETTY_FUNCTION__, this);
	// do nothing.
}

int Task::SetInvalid()
{
	fprintf(stdout, "%s[%p]: \n", __PRETTY_FUNCTION__, this);
	//OSMutexLocker theLocker(&fMutex);
	if(m_IsValid)
	{
		m_IsValid = false;
		Release();
		return 0;
	}
	
	return -1;
}



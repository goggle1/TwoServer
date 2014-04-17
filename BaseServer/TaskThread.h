#ifndef __TASK_THREAD_H__
#define __TASK_THREAD_H__

#include "events.h"
#include "OSCond.h"
#include "OSMutex.h"
#include "dequeH.h"
#include "task.h"
#include "thread.h"

class TaskThread : public Thread
{
public:
	TaskThread();
	~TaskThread();
	int			Init();
	int			EnqueTask(Task* taskp);
	Task*		WaitForTask();	
	virtual int Entry();

	Events		m_EventsMaster;
	
protected:			
	DEQUEH_T	m_task_queue;
	OSCond      fCond;
    OSMutex     fMutex;

	OSHeap              fHeap;	
};

#endif

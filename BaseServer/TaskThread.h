#ifndef __TASK_THREAD_H__
#define __TASK_THREAD_H__

#include "events.h"
#include "thread.h"

class TaskThread : public Thread
{
public:
	TaskThread();
	~TaskThread();
	int			Init();
	virtual int Entry();

	Events		m_EventsMaster;
	
protected:			

};

#endif

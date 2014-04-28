#ifndef __TASK_H__
#define __TASK_H__

#include "events.h"

class TaskThread;

class Task
{
public:
	Task();
	virtual ~Task();	
	int				GetFd() { return m_SockFd; };	
	virtual	int		DoEvents(u_int32_t events, TaskThread* threadp) = 0;	
	
protected:			
	int 			m_SockFd;
	TaskThread*		m_task_thread;
};

inline void task_release(void* elementp)
{
	Task* taskp = (Task*)elementp;
	delete taskp;
}

#endif
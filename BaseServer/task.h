#ifndef __TASK_H__
#define __TASK_H__

#include "OSCond.h"
#include "OSMutex.h"
#include "heap.h"
#include "dequeH.h"

class TaskThread;

class Task
{
public:
	Task();
	virtual ~Task();	
	int				GetFd() { return m_SockFd; };
	int 			EnqueEvents(u_int64_t events);
	int				DequeEvents(u_int64_t& events);
	int				SetInvalid();
	virtual int 	Run() = 0;
	// sub class do Release(), remove event source, remove timer.
	virtual	void	Release();
	virtual	int		DoEvents(u_int32_t events, TaskThread* threadp) = 0;	
protected:	
	int				Attach();
	void 			Detach();
	int 			m_SockFd;
	TaskThread*		m_task_thread;
	
	DEQUEH_T 	m_EventsQueue;	
    OSMutex     fMutex;
	
	//This could later be optimized by using a timing wheel instead of a heap,
    //and that way we wouldn't need both a heap elem and a queue elem here (just queue elem)
    OSHeapElem      fTimerHeapElem;

    OSHeapElem 		fIdleElem;

    bool 			m_IsValid;

	friend class TimerThread;
    friend class TaskThread;
};

inline void task_release(void* elementp)
{
	Task* taskp = (Task*)elementp;
	delete taskp;
}

#endif
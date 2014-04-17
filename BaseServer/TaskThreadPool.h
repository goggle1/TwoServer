#ifndef __TASKTHREADPOOL_H__
#define __TASKTHREADPOOL_H__

#include "TaskThread.h"

#define THREAD_SHORT	0
#define THREAD_LONG		1

class TaskThreadPool
{
public:
	TaskThreadPool();
	~TaskThreadPool();
	int 	Init();
	TaskThread*	SelectThread(int short_or_long);
protected:
	// these thread run short-time tasks.
	TaskThread**	m_threads_short; 
	// these thread run long-time tasks.
	TaskThread**	m_threads_long;

	// thread num
	int		m_num_short;
	//
	int		m_num_long;
	
	//position for select short thread, round-robin, or by load, or by weight
	int		m_position_short;
	//
	int		m_position_long;
};

extern TaskThreadPool* g_task_thread_pool;

#endif
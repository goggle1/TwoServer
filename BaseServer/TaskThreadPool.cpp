#include "TaskThreadPool.h"

TaskThreadPool::TaskThreadPool()
{
	m_threads_short		= NULL;
	m_threads_long 		= NULL;
	m_num_short			= 0;
	m_num_long			= 0;
	m_position_short 	= 0;
	m_position_long		= 0;
}

TaskThreadPool::~TaskThreadPool()
{
}

int TaskThreadPool::Init()
{
	// get cpu num = N.
	// start N short thread
	// start N long thread
	int cpu_num = 4;
	int index = 0;

	m_num_short = cpu_num;
	m_num_long	= cpu_num;

	m_threads_short = new TaskThread*[m_num_short];	
	for(index=0; index<m_num_short; index++)
	{
		TaskThread* threadp = new TaskThread();
		threadp->Init();
		threadp->Start();
		m_threads_short[index] = threadp;
	}

	m_threads_long = new TaskThread*[m_num_long];	
	for(index=0; index<m_num_long; index++)
	{
		TaskThread* threadp = new TaskThread();
		threadp->Init();
		threadp->Start();
		m_threads_long[index] = threadp;
	}
	
	return 0;
}

TaskThread* TaskThreadPool::SelectThread(int short_or_long)
{	
	if(short_or_long == THREAD_SHORT)
	{
		TaskThread* threadp = m_threads_short[m_position_short];
		m_position_short ++;
		if(m_position_short >= m_num_short)
		{
			m_position_short = 0;
		}
		return threadp;
	}
	else if(short_or_long == THREAD_LONG)
	{
		TaskThread* threadp = m_threads_long[m_position_long];
		m_position_long ++;	
		if(m_position_long >= m_num_long)
		{
			m_position_long = 0;
		}
		return threadp;
	}
	
	return NULL;
}

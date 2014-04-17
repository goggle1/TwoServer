#ifndef __THREAD_H__
#define __THREAD_H__

#include <pthread.h>

class Thread
{
public:
	Thread();
	~Thread();
	int	Start();
	virtual	int	Entry() = 0;
	
protected:
	static void*	_Entry(void* inThread);
	pthread_t		m_thread_id;	
};

#endif

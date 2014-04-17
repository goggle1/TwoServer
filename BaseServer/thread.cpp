#include "thread.h"

Thread::Thread()
{
}

Thread::~Thread()
{
}

int Thread::Start()
{
	int ret = 0;
	ret = pthread_create(&m_thread_id, NULL, _Entry, (void*)this);
	return ret;
}

void* Thread::_Entry(void *inThread)  //static
{
    Thread* theThread = (Thread*)inThread;
    theThread->Entry();
    return NULL;
}



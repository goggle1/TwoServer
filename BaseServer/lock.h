#ifndef __LOCK_H__
#define __LOCK_H__

#include <pthread.h>

class ReadLock
{
public:
	ReadLock(pthread_rwlock_t* lockp) 
	{ 
		m_lockp = lockp;
		pthread_rwlock_rdlock(m_lockp);		
	};
	virtual ~ReadLock()
	{
		pthread_rwlock_unlock(m_lockp);
	};

protected:
	pthread_rwlock_t* m_lockp;
};

class WriteLock
{
public:
	WriteLock(pthread_rwlock_t* lockp) 
	{ 
		m_lockp = lockp;
		pthread_rwlock_wrlock(m_lockp);
	};
	virtual ~WriteLock()
	{
		pthread_rwlock_unlock(m_lockp);
	};

protected:
	pthread_rwlock_t* m_lockp;
};

#endif

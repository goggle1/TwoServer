
#ifndef _OSMUTEX_H_
#define _OSMUTEX_H_

#include <stdlib.h>
#include <pthread.h>

class OSCond;

class OSMutex
{
    public:

        OSMutex();
        ~OSMutex();

        inline void	Lock();
        inline void Unlock();
        
        // Returns true on successful grab of the lock, false on failure
        inline bool TryLock();

    private:
        pthread_mutex_t fMutex;        

        friend class OSCond;
};

class   OSMutexLocker
{
    public:

        OSMutexLocker(OSMutex *inMutexP) : fMutex(inMutexP) { if (fMutex != NULL) fMutex->Lock(); }
        ~OSMutexLocker() {  if (fMutex != NULL) fMutex->Unlock(); }
        
        void Lock()         { if (fMutex != NULL) fMutex->Lock(); }
        void Unlock()       { if (fMutex != NULL) fMutex->Unlock(); }
        
    private:

        OSMutex*    fMutex;
};

void OSMutex::Lock()
{
    (void)pthread_mutex_lock(&fMutex);
}

void OSMutex::Unlock()
{
    pthread_mutex_unlock(&fMutex);
}

bool OSMutex::TryLock()
{
    int theErr = pthread_mutex_trylock(&fMutex);
    if (theErr != 0)
    {        
        return false;
    }
    
    return true;
}

#endif //_OSMUTEX_H_

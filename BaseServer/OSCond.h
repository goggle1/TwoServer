
#ifndef _OSCOND_H_
#define _OSCOND_H_

#include <sys/types.h>
#include <pthread.h>

#include "OSMutex.h"

class OSCond 
{
    public:

        OSCond();
        ~OSCond();
        
        inline void     Signal();
        inline void     Wait(OSMutex* inMutex, int32_t inTimeoutInMilSecs = 0);
        inline void     Broadcast();

    private:
        pthread_cond_t      fCondition;
        void                TimedWait(OSMutex* inMutex, int32_t inTimeoutInMilSecs);
};

inline void OSCond::Wait(OSMutex* inMutex, int32_t inTimeoutInMilSecs)
{
    this->TimedWait(inMutex, inTimeoutInMilSecs);
}

inline void OSCond::Signal()
{
    pthread_cond_signal(&fCondition);
}

inline void OSCond::Broadcast()
{
    pthread_cond_broadcast(&fCondition);
}

#endif //_OSCOND_H_

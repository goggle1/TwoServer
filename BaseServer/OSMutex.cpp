
#include <string.h>

#include "OSMutex.h"


static pthread_mutexattr_t  *sMutexAttr=NULL;
static pthread_once_t sMutexAttrInit = PTHREAD_ONCE_INIT;

static void MutexAttrInit()
{
    sMutexAttr = (pthread_mutexattr_t*)malloc(sizeof(pthread_mutexattr_t));
    ::memset(sMutexAttr, 0, sizeof(pthread_mutexattr_t));
    pthread_mutexattr_init(sMutexAttr);
}    

OSMutex::OSMutex()
{
    (void)pthread_once(&sMutexAttrInit, MutexAttrInit);
    (void)pthread_mutex_init(&fMutex, sMutexAttr); 
}

OSMutex::~OSMutex()
{
    pthread_mutex_destroy(&fMutex);
}




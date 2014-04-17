
#include <string.h>
#include <sys/time.h>

#include "OS.h"

int64_t OS::Milliseconds()
{
	struct timeval t;
    int theErr = ::gettimeofday(&t, NULL);    

    int64_t curTime;
    curTime = t.tv_sec;
    curTime *= 1000;                // sec -> msec
    curTime += t.tv_usec / 1000;    // usec -> msec

    return curTime;
}


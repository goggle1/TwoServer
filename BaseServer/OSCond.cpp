/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 1999-2008 Apple Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 *
 */
/*
    File:       OSCond.cpp

    Contains:   Implementation of OSCond class
    
    

*/

#include <sys/time.h>

#include "OSCond.h"
#include "OSMutex.h"

OSCond::OSCond()
{
	pthread_condattr_t cond_attr;
	pthread_condattr_init(&cond_attr);
	pthread_cond_init(&fCondition, &cond_attr);
}

OSCond::~OSCond()
{
    pthread_cond_destroy(&fCondition);
}


void OSCond::TimedWait(OSMutex* inMutex, int32_t inTimeoutInMilSecs)
{
    struct timespec ts;
    struct timeval tv;
    struct timezone tz;
    int sec, usec;
    
    if (inTimeoutInMilSecs == 0)
    {
        (void)pthread_cond_wait(&fCondition, &inMutex->fMutex);
    }
    else
    {
        gettimeofday(&tv, &tz);
        sec = inTimeoutInMilSecs / 1000;
        inTimeoutInMilSecs = inTimeoutInMilSecs - (sec * 1000);
        //Assert(inTimeoutInMilSecs < 1000);
        usec = inTimeoutInMilSecs * 1000;
        //Assert(tv.tv_usec < 1000000);
        ts.tv_sec = tv.tv_sec + sec;
        ts.tv_nsec = (tv.tv_usec + usec) * 1000;
        //Assert(ts.tv_nsec < 2000000000);
        if(ts.tv_nsec > 999999999)
        {
             ts.tv_sec++;
             ts.tv_nsec -= 1000000000;
        }
        (void)pthread_cond_timedwait(&fCondition, &inMutex->fMutex, &ts);
    }
    
}


/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include <string.h>
#include "os_stdlib.h"
#include "os_time.h"
#include "vortex_os.h"
#include "c_laptime.h"

#define GETMAX(a,b)      ((a) > (b) ? (a) : (b))
#define GETMIN(a,b)      ((a) < (b) ? (a) : (b))
#define GETLAP(t1,t2) ((t2) - (t1))

typedef os_uint64 c_hrtime;

struct c_laptime_s {
    const char *id;
    c_hrtime cur;
    c_hrtime min;
    c_hrtime max;
    c_hrtime tot;
    int count;
};

c_laptime
c_laptimeCreate(
    const char *id)
{
    c_laptime laptime;

    laptime = (c_laptime)os_malloc(sizeof(struct c_laptime_s));
    memset((void *)laptime,0,sizeof(struct c_laptime_s));

    laptime->id = id;
    laptime->cur = 0ll;
    laptime->min = 0x7fffffffffffffffll;
    laptime->max = 0ll;
    laptime->tot = 0ll;
    laptime->count = 0;

    return laptime;
}

void
c_laptimeDelete (
    c_laptime laptime)
{
    os_free(laptime);
}

void
c_laptimeReset (
    c_laptime laptime)
{
    laptime->cur = 0ll;
    laptime->min = 0x7fffffffffffffffll;
    laptime->max = 0ll;
    laptime->tot = 0ll;
    laptime->count = 0;
}

c_hrtime
c_gethrtime (
    void)
{
    os_time ctime;
    c_hrtime rtime;

    ctime = os_timeGetMonotonic();
    rtime = (c_hrtime)ctime.tv_sec * (c_hrtime)1000000000 + (c_hrtime)ctime.tv_nsec;
    return rtime;
}

void
c_laptimeStart(
    c_laptime laptime)
{
    laptime->cur = c_gethrtime();
}

void
c_laptimeStop(
    c_laptime laptime)
{
    c_hrtime last;
    c_hrtime lap;
    c_hrtime bound;

    last = c_gethrtime();
    lap = GETLAP(laptime->cur, last);
    bound = GETMIN(laptime->min, lap);
    laptime->min = bound;
    bound = GETMAX(laptime->max, lap);
    laptime->max = bound;
    laptime->tot += lap;
    laptime->count++;
}

void
c_laptimeReport(
    c_laptime laptime,
    const char *info)
{
    printf("==============================================================\n");
    printf("Lap info (%s): %s\n", laptime->id, info);
    printf("--------------------------------------------------------------\n");
    printf("Nr of laps        min (usec)       max (usec)      mean (usec)\n");
    printf("--------------------------------------------------------------\n");
    if (laptime->count > 0) {
        printf("%10d %16d %16d %16d\n",
            laptime->count,
            (int)(laptime->min / 1000ll),
            (int)(laptime->max / 1000ll),
            (int)(laptime->tot / 1000ll) / laptime->count); 
    } else {
        printf("         0               NA               NA               NA\n");
    }
    printf("==============================================================\n");
}


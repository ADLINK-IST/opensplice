/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include <string.h>
#include "os_stdlib.h"
#include "os_time.h"
#include "os.h"
#include "c_laptime.h"

#define GETMAX(a,b)      ((a) > (b) ? (a) : (b))
#define GETMIN(a,b)      ((a) < (b) ? (a) : (b))
#define GETLAP(t1,t2) ((t2) - (t1))

typedef long long c_hrtime;

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

    ctime = os_hrtimeGet();
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


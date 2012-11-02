/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/* Interface */
#include "nw_brake.h"

/* Implementation */
#include "os_heap.h"
#include "os_time.h"
#include "nw_report.h"

/* -------------------- helper class for slowing down the sender ------------ */

/* The brake slows down the thread. Every maxBurstSize bytes, it will check if
 * a sleep is needed. The maxBurstSize is derived from the maxThroughput and
 * the brakeFrequency. This means that a process can be slowed down a maximum
 * of brakeFrequency times per maxThroughput bytes. */

NW_STRUCT(nw_brake) {
    unsigned int maxBurstSize;
    unsigned int currentBurstSize;
    os_time timePerBurst;
    os_time thresholdTime;
    os_time minSleepTime;
    nw_bool checkNeeded;
};

nw_brake
nw_brakeNew(
    unsigned int maxThroughput, /* kb/sec */
    unsigned int maxBurstSize, /* kb */
    unsigned int minSleepMSecs)
{
    nw_brake result;
    os_time oneSecond = {1U,0U};
       
    result = (nw_brake)os_malloc(sizeof(*result));
    if (result != NULL) {
        result->maxBurstSize = 1000 * maxBurstSize;
        result->currentBurstSize = 0;
        result->timePerBurst = os_timeMulReal(oneSecond,
            (double)maxBurstSize/(double)maxThroughput);
        result->minSleepTime = os_timeMulReal(oneSecond,
            (double)minSleepMSecs/(double)1000.0);
        result->thresholdTime = os_timeGet();
        result->checkNeeded = FALSE;
    }
    return result;
}

void
nw_brakeAddUnits(
    nw_brake brake,
    unsigned int units)
{
    if (brake != NULL) {
        brake->currentBurstSize += units;
        if (brake->currentBurstSize > brake->maxBurstSize) {
            brake->checkNeeded = TRUE;
            do {
                brake->thresholdTime =
                    os_timeAdd(brake->thresholdTime, brake->timePerBurst);
                brake->currentBurstSize -= brake->maxBurstSize;
            } while (brake->currentBurstSize > brake->maxBurstSize);
        }
    }
}

void
nw_brakeSlowDown(
    nw_brake brake)
{
    os_time currentTime;
    os_time sleepTime;
    os_compare cmp;
    
    if ((brake != NULL) && (brake->checkNeeded)) {
        currentTime = os_timeGet();
        cmp = os_timeCompare(brake->thresholdTime, currentTime);
        if (cmp == OS_MORE) {
            sleepTime = os_timeSub(brake->thresholdTime, currentTime);
            /* Do not sleep below sleep-resolution */
            cmp = os_timeCompare(sleepTime, brake->minSleepTime);
            if (cmp == OS_MORE) {
                NW_TRACE_1(Send, 3, "Brake activated, sleeping for %d milliseconds",
                    1000U*sleepTime.tv_sec + sleepTime.tv_nsec/1000000U);
                os_nanoSleep(sleepTime);
            }
        } else {
            brake->thresholdTime = currentTime;
        }
        brake->checkNeeded = FALSE;
    }
}

void
nw_brakeFree(
    nw_brake brake)
{
    if (brake != NULL) {
        os_free(brake);
    }
}

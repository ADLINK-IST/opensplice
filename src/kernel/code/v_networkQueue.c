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
/* Interface */
#include "v__networkQueue.h"

/* Implementation */
#include "os_report.h"
#include "c_base.h"
#include "kernelModule.h"
#include "v_networkReaderEntry.h"
#include "v_networkReaderStatistics.h"
#include "v__statisticsInterface.h"
#include "v_entity.h"    /* for v_entity() */
#include "v_group.h"     /* for v_group()  */
#include "v__reader.h"    /* for v_reader() */
#include "v_readerQos.h" /* for v_readerQosNew() */
#include "v_entry.h"     /* for v_entry()  */
#include "v_state.h"
#include "v_subscriber.h"
#include "v_groupSet.h"
#include "v_topic.h"
#include "v_time.h"
#include "v_statistics.h"
#include "v_message.h"
#include "v__messageQos.h"

#define MIN_SLEEPTIME {0, 0}
#define MIN_SLEEPPERIOD 0

#define CONDWAIT_UNTIL(cvRef, mutexRef, until) \
    v_networkQueueCondWaitUntil(cvRef, mutexRef, until)

#define v_networkQueueSample(o) (C_CAST(o,v_networkQueueSample))
#define v_networkStatusMarker(o) (C_CAST(o,v_networkStatusMarker))

/* ------------------------------- v_networkQueue ------------------------------ */

#define v_networkQueueItem(o)   (C_CAST(o, v_networkQueueItem))

#define V_SEC_TO_MSEC(sec)   ((c_ulonglong)(sec)*1000)
#define V_MSEC_TO_SEC(msec)  ((c_ulong)((msec)/1000))
#define V_NSEC_TO_MSEC(nsec) ((c_ulonglong)((nsec)/1000000))
#define V_MSEC_TO_NSEC(nsec) ((c_ulong)((nsec)*1000000))

#define MSEC_TO_TIME(msec, time) \
    time.seconds = V_MSEC_TO_SEC(msec); \
    time.nanoseconds = V_MSEC_TO_NSEC(msec % 1000)

#define TIME_TO_MSEC(time, msec) \
    msec = (c_ulonglong)(V_SEC_TO_MSEC(time.seconds) + V_NSEC_TO_MSEC(time.nanoseconds))

static void
v_networkQueueUpdateNextWakeup(
    v_networkQueue queue,
    c_bool *hasChanged)
{
    c_time now;
    c_time newWakeup;
    c_ulonglong msecsTime;
    c_ulonglong msecsResult;
    c_ulonglong msecsLeftOver;
    
    *hasChanged = FALSE;
    if (queue->periodic) {
        now = v_timeGet();
        TIME_TO_MSEC(now, msecsTime);
        /* Do a ++ because we are doing a ceil and TIME_TO_MSEC is doing a trunc.
         * Only if time was an exact multiple of milliseconds, this approach is
         * not completely correct. But it saves us the hassle and this works fine */
        msecsTime++;
        msecsLeftOver = (msecsTime - queue->phaseMilliSeconds) % queue->msecsResolution;
        msecsResult = msecsTime - msecsLeftOver + queue->msecsResolution;
        MSEC_TO_TIME(msecsResult, newWakeup);
        if (c_timeCompare(newWakeup,queue->nextWakeup) == C_GT) {
            queue->nextWakeup = newWakeup;
            *hasChanged = TRUE;
        }
    }
}

#define dataExpired(queue) \
        (c_timeCompare(queue->nextWakeup, \
                       queue->firstStatusMarker->sendBefore) == C_GT)

#define v_networkQueueHasExpiringData(queue) \
        (queue->firstStatusMarker ? \
            (queue->periodic ? dataExpired(queue) : \
                               TRUE) : \
            FALSE)


v_networkQueue
v_networkQueueNew(
    c_base base,
    c_ulong queueSize,
    c_ulong priority,
    c_bool reliable,
    c_bool P2P,
    c_time resolution,
    v_networkQueueStatistics statistics)
{
    v_networkQueue result = NULL;
    c_type type;
    c_equality equality;
    c_bool hasChanged;
    c_time now;

    type = c_resolve(base, "kernelModule::v_networkQueue");
    assert(type);
    result = v_networkQueue(c_new(type));
    c_free(type);

    if (result) {
        /* Queue properties */
        result->maxMsgCount = queueSize;
        result->currentMsgCount = 0;
        /* Cached type */
        result->statusMarkerType = c_resolve(base, "kernelModule::v_networkStatusMarker");
        assert(result->statusMarkerType != NULL);
        result->sampleType = c_resolve(base, "kernelModule::v_networkQueueSample");
        assert(result->sampleType != NULL);
        /* Linked list of in-use marker items */
        result->firstStatusMarker = NULL;
        result->lastStatusMarker = NULL;
        /* Linked list of free marker and message items */
        result->freeStatusMarkers = NULL;
        result->freeSamples = NULL;
        /* Init cv stuff */
        c_mutexInit(&result->mutex, SHARED_MUTEX);
        c_condInit(&result->cv, &result->mutex, SHARED_COND);
        /* Currently no differentiation wrt qos */
        result->priority = priority;
        result->reliable = reliable;
        result->P2P = P2P;

        result->statistics = statistics;

        equality = c_timeCompare(C_TIME_ZERO, resolution);
        if (equality == C_EQ) {
            result->periodic = FALSE;
            result->resolution = C_TIME_INFINITE;
            result->msecsResolution = 0xFFFFFFFF;
            result->phaseMilliSeconds = 0;
            result->nextWakeup = C_TIME_INFINITE;
        } else {
            assert(equality == C_LT);
            result->periodic = TRUE;
            result->resolution = resolution;
            TIME_TO_MSEC(resolution, result->msecsResolution);        
            /* A semi-random phase to avoid wake-ups at the same time */
            now = v_timeGet();
            result->phaseMilliSeconds = ((c_ulong)(now.nanoseconds/1000000 * 1.618)) %
                result->msecsResolution;
            v_networkQueueUpdateNextWakeup(result, &hasChanged);
            assert(hasChanged);
        }
        result->threadWaiting = FALSE;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_networkQueueNew",0,
                  "Failed to allocate network queue.");
        assert(FALSE);
    }
    return result;
}    

c_bool
v_networkQueueWrite(
    v_networkQueue queue,
    v_message msg,
    v_networkReaderEntry entry,
    c_long sequenceNumber,
    v_gid sender,
    c_bool sendTo, /* for p2p writing */
    v_gid receiver)
{
    c_bool result = TRUE;
    c_bool wasEmpty;
    c_bool found;
    v_networkStatusMarker *currentMarkerPtr;
    v_networkStatusMarker currentMarker;
    v_networkStatusMarker marker;
    v_networkQueueSample newHolder;
    c_ulonglong msecsTime;
    c_ulonglong msecsResult;
    c_ulonglong msecsLeftOver;
    c_time sendBeforeNoTrunc;
    c_time sendBefore;
    c_time now;
    c_ulong priorityLookingFor;
    c_equality eq;
    c_bool sendNow = FALSE;
   
    V_MESSAGE_STAMP(msg,readerInsertTime); 

    c_mutexLock(&queue->mutex);
    sendBefore = C_TIME_ZERO;

    /* numberOfSamplesArrived statistics */
    v_networkQueueStatisticsAdd(numberOfSamplesArrived,queue->statistics);

    /* When the network queue is full then the message should be rejected.
     * The exception is an UNREGISTER message. The UNREGISTER message should
     * always be accepted because rejecting it may bring the system into an
     * inconsistent state (an instance cannot be NO_WRITERS for some readers
     * and still alive for the network reader). Because state changes as a
     * result of invalid message do not count towards resource limits this
     * is a valid exception. The DISPOSE message does not need to fall into
     * the same category because the group never acts on only the DISPOSE
     * messages. It is the transition to the NO_WRITERS state that potentially
     * triggers the purge mechanism.
     */
    if (!v_messageStateTest(msg,L_UNREGISTER)) {
        if (queue->currentMsgCount == queue->maxMsgCount) {
            c_mutexUnlock(&queue->mutex);
            /* numberOfSamplesRejected stat */
            v_networkQueueStatisticsAdd(numberOfSamplesRejected,queue->statistics);
            return FALSE;
        }
    }

    currentMarkerPtr = &(queue->firstStatusMarker);
    currentMarker = *currentMarkerPtr;

    if (queue->threadWaiting) {
        wasEmpty = !v_networkQueueHasExpiringData(queue);
    } else {
        wasEmpty = FALSE;
    }

    marker = NULL;
    found = FALSE;

    priorityLookingFor = v_messageQos_getTransportPriority(msg->qos);
    if (queue->periodic) {
        if (v_messageQos_isZeroLatency(msg->qos)) {
            sendBefore = C_TIME_ZERO;
            sendNow = TRUE;
        } else {
#ifdef _NAT_
            now = v_timeGet();
#else
            now = msg->allocTime;
#endif
            sendBeforeNoTrunc = c_timeAdd(now,
                                          v_messageQos_getLatencyPeriod(msg->qos));
            TIME_TO_MSEC(sendBeforeNoTrunc, msecsTime);
            msecsLeftOver = (c_ulonglong)((msecsTime - queue->phaseMilliSeconds) %
                                         queue->msecsResolution);
            msecsResult = (c_ulonglong)(msecsTime - msecsLeftOver);
            MSEC_TO_TIME(msecsResult, sendBefore);
        }
        while ((currentMarker != NULL) && (!found)) {
            eq = c_timeCompare(sendBefore, currentMarker->sendBefore);
            switch (eq) {
            case C_GT:
                currentMarkerPtr = &currentMarker->next;
                currentMarker = *currentMarkerPtr;
            break;
            case C_EQ:
                if (priorityLookingFor < currentMarker->priority) {
                    currentMarkerPtr = &currentMarker->next;
                    currentMarker = *currentMarkerPtr;
                } else {
                    found = TRUE;
                    if (priorityLookingFor == currentMarker->priority) {
                        marker = currentMarker;
                    }
                }
            break;
            case C_LT:
                found = TRUE;
            break;
            default:
                assert(FALSE);
            break;
            }
        }
    } else {
        if (currentMarker) {
        sendBefore = C_TIME_ZERO;
            if (c_timeIsZero(currentMarker->sendBefore)) {
                marker = currentMarker;
            }
        }
    }
    /* Insert after end of list */
    if (marker == NULL) {
        if (queue->freeStatusMarkers == NULL) {
            marker = v_networkStatusMarker(c_new(queue->statusMarkerType));
            if (marker == NULL) {
                OS_REPORT(OS_ERROR,
                          "v_networkQueueWrite",0,
                          "Failed to allocate v_networkStatusMarker object.");
                c_mutexUnlock(&queue->mutex);
                return FALSE;
            }
        } else {
            marker = queue->freeStatusMarkers;
            queue->freeStatusMarkers = marker->next;
        }

        marker->sendBefore = sendBefore;
        marker->priority = priorityLookingFor;
        marker->firstSample = NULL;
        marker->lastSample = NULL;
        marker->next = *currentMarkerPtr; /* no keep, transfer refCount */
        if (marker->next == NULL) {
            queue->lastStatusMarker = marker; /* no keep, not reference counted */
        }
        *currentMarkerPtr = marker; /* no keep, transfer refCount */
    }
    V_MESSAGE_STAMP(msg,readerLookupTime); 
    assert(marker != NULL);
    if (queue->freeSamples == NULL) {
        newHolder = c_new(queue->sampleType);
        if (newHolder == NULL) {
            OS_REPORT(OS_ERROR,
                      "v_networkQueueWrite",0,
                      "Failed to allocate v_networkQueueSample object.");
            result = FALSE;
            c_mutexUnlock(&queue->mutex);
            return result;
        }
    } else {
        newHolder = queue->freeSamples;
        queue->freeSamples = newHolder->next;
    }

    queue->currentMsgCount++;

    /* numberOfSamplesInserted & numberOfSamplesWaiting + stats*/
    v_networkQueueStatisticsAdd(numberOfSamplesInserted,queue->statistics);
    v_networkQueueStatisticsCounterInc(numberOfSamplesWaiting,queue->statistics);

    newHolder->message = c_keep(msg);
    newHolder->entry = c_keep(entry);
    newHolder->sequenceNumber = sequenceNumber;
    newHolder->sender = sender;
    newHolder->sendTo = sendTo;
    newHolder->receiver = receiver;

    if (marker->lastSample != NULL) {
        newHolder->next = v_networkQueueSample(marker->lastSample)->next; /* no keep, transfer refCount */
        v_networkQueueSample(marker->lastSample)->next = newHolder; /* no keep, transfer refCount */
    } else {
    newHolder->next = marker->firstSample; /* no keep, transfer refCount */
        marker->firstSample = newHolder; /* no keep, transfer refCount */
    }
    marker->lastSample = newHolder;


    /* Write done, wake up waiters if needed */
    if (wasEmpty && queue->threadWaiting) {
        if (sendNow || v_networkQueueHasExpiringData(queue)) {
            c_condBroadcast(&queue->cv);
        }
    }

    c_mutexUnlock(&queue->mutex);
    
    return result;
}


#undef V_SEC_TO_MSEC
#undef V_MSEC_TO_SEC
#undef V_NSEC_TO_MSEC
#undef V_MSEC_TO_NSEC


c_bool
v_networkQueueTakeFirst(
    v_networkQueue queue,
    v_message *message,
    v_networkReaderEntry *entry,
    c_ulong *sequenceNumber,
    v_gid *sender,
    c_bool *sendTo, /* for p2p writing */
    v_gid *receiver,
    c_time *sendBefore,
    c_ulong *priority,
    c_bool *more)
{
    c_bool result = FALSE;
    v_networkStatusMarker currentMarker;
    v_networkQueueSample sample;

    *more = FALSE;
    
    c_mutexLock(&queue->mutex);

    currentMarker = queue->firstStatusMarker;
    /* Note: the current design expects that this function has been preceded
     *       by a NetworkReaderWait. Therefore, the currentMarker should never
     *       be NULL. */

    if (currentMarker != NULL) {
        sample = currentMarker->firstSample;
        assert(sample != NULL);
        result = TRUE;
        
        V_MESSAGE_STAMP(sample->message,readerDataAvailableTime); 

        /* Copy values */
        *message = sample->message; /* no keep, transfer refCount */
        sample->message = NULL; /* clean reference because of  refCount transfer */
        *entry = sample->entry; /* no keep, transfer refCount */
        sample->entry = NULL; /* clean reference because of refCount transfer */
        *sequenceNumber = sample->sequenceNumber;
        *sender = sample->sender;
        *sendTo = sample->sendTo;
        *receiver = sample->receiver;
        *sendBefore = currentMarker->sendBefore;
        *priority = currentMarker->priority;
        
        /* Remove and free holder */
        queue->currentMsgCount--;

        /* numberOfSamplesTaken+ & numberOfSamplesWaiting- stats */
        v_networkQueueStatisticsAdd(numberOfSamplesTaken,queue->statistics);
        v_networkQueueStatisticsCounterDec(numberOfSamplesWaiting,queue->statistics);


        currentMarker->firstSample = sample->next; /* no keep, transfer refCount */
        sample->next = queue->freeSamples;
        queue->freeSamples = sample;
        if (currentMarker->firstSample == NULL) {
            queue->firstStatusMarker = currentMarker->next; /* no keep, transfer refCount */
            currentMarker->next = queue->freeStatusMarkers;
            queue->freeStatusMarkers = currentMarker;
            if (queue->firstStatusMarker == NULL) {
                queue->lastStatusMarker = NULL;
            }
        }
        *more = (queue->firstStatusMarker != NULL);
    } else {
        *message = NULL;
        *entry = NULL;
        *more = FALSE;
    }
    c_mutexUnlock(&queue->mutex);
    
    return result;
}

c_bool
v_networkQueueTakeAction(
    v_networkQueue queue,
    v_networkQueueAction action,
    c_voidp arg)
{
    v_networkStatusMarker currentMarker;
    v_networkQueueSample sample;
    c_bool proceed = TRUE;

    c_mutexLock(&queue->mutex);
    currentMarker = queue->firstStatusMarker;
    while ((currentMarker != NULL) && proceed) {
        sample = currentMarker->firstSample;
        assert(sample != NULL);

        if (sample != NULL) {
            proceed = action(sample, arg);
            queue->currentMsgCount--;
            /* numberOfSamplesTaken+ & numberOfSamplesWaiting- stats */
            v_networkQueueStatisticsAdd(numberOfSamplesTaken,queue->statistics);
            v_networkQueueStatisticsCounterDec(numberOfSamplesWaiting,queue->statistics);


            currentMarker->firstSample = sample->next; /* no keep, transfer refCount */
            sample->next = queue->freeSamples;
            queue->freeSamples = sample;
            if (currentMarker->firstSample == NULL) {
                currentMarker->lastSample = NULL;
                queue->firstStatusMarker = currentMarker->next; /* no keep, transfer refCount */
                currentMarker->next = queue->freeStatusMarkers;
                queue->freeStatusMarkers = currentMarker;
                if (queue->firstStatusMarker == NULL) {
                    queue->lastStatusMarker = NULL;
                }
            }
        }
        currentMarker = queue->firstStatusMarker;
    }
    c_mutexUnlock(&queue->mutex);
    proceed = action(NULL, arg);
    
    return proceed;
}

v_networkReaderWaitResult
v_networkQueueWait(
    v_networkQueue queue)
{
    v_networkReaderWaitResult result = V_WAITRESULT_NONE;
    c_syncResult syncResult;
    c_bool hasChanged;
    c_time interval;
    c_time minSleepTime = MIN_SLEEPTIME;
    c_equality eq;
    
    c_mutexLock(&queue->mutex);

    /* First update nextWakeup */
    v_networkQueueUpdateNextWakeup(queue, &hasChanged);
    if (hasChanged) {
        result |= V_WAITRESULT_TIMEOUT;
    }
    
    /* With the new nextWakeup, check if any data is expiring */
    if ((int)v_networkQueueHasExpiringData(queue)) {
        result |= V_WAITRESULT_MSGWAITING;
    }
    
    /* Also check if no request has been issued lately */
    if ((int)queue->triggered) {
        result |= V_WAITRESULT_TRIGGERED;
    }
    
    /* Now go to sleep if needed */
    while (result == V_WAITRESULT_NONE) {
        if (queue->periodic) {
            c_time org =  v_timeGet();
            interval = c_timeSub(queue->nextWakeup,org);
            eq = c_timeCompare(minSleepTime, interval);
            if (eq == C_LT) {
                queue->threadWaiting = TRUE;
                syncResult = c_condTimedWait(&queue->cv,
                                             &queue->mutex,
                                             interval);
                queue->threadWaiting = FALSE;
            } else {
                syncResult = SYNC_RESULT_TIMEOUT;
            }
            if (syncResult == SYNC_RESULT_TIMEOUT) {
                result |= V_WAITRESULT_TIMEOUT;
                queue->nextWakeup = c_timeAdd(queue->nextWakeup,
                                              queue->resolution);
            }
        } else {
            /* Wait infinitely if the queue is not periodic */
            queue->threadWaiting = TRUE;
            syncResult = c_condWait(&queue->cv, &queue->mutex);
            queue->threadWaiting = FALSE;
        }
        /* Test current status of queue */
        if ((int)queue->triggered) {
            result |= V_WAITRESULT_TRIGGERED;
        }
        if (v_networkQueueHasExpiringData(queue)) {
            result |= V_WAITRESULT_MSGWAITING;
        }
    }
    
    queue->triggered = 0;
    
    c_mutexUnlock(&queue->mutex);
    
    return result;
}    


void
v_networkQueueTrigger(
    v_networkQueue queue)
{
    c_mutexLock(&queue->mutex);
    queue->triggered = 1;
    if (queue->threadWaiting) {
        c_condBroadcast(&queue->cv);
    }
    c_mutexUnlock(&queue->mutex);
}



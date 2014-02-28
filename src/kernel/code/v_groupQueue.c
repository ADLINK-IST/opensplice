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

#include "v__groupQueue.h"
#include "v_groupQueue.h"
#include "v_groupStream.h"
#include "v__groupStream.h"
#include "v_groupQueueStatistics.h"
#include "v__statisticsInterface.h"
#include "v_reader.h"
#include "v_readerQos.h"
#include "v_observer.h"
#include "v_status.h"
#include "v_state.h"
#include "v_event.h"
#include "c_collection.h"
#include "os_report.h"
#include "v_message.h"
#include "v_statistics.h"
#include "v__statCat.h"

v_groupQueue
v_groupQueueNew(
    v_subscriber subscriber,
    const c_char* name,
    c_ulong maxSize,
    v_readerQos qos,
    c_iter expr)
{
    v_kernel kernel;
    v_groupQueue queue;
    v_groupQueueStatistics gqs;
    v_readerQos q;

    assert(C_TYPECHECK(subscriber,v_subscriber));

    kernel = v_objectKernel(subscriber);

    if (v_isEnabledStatistics(kernel, V_STATCAT_READER) ||
        v_isEnabledStatistics(kernel, V_STATCAT_DURABILITY)) {
        gqs = v_groupQueueStatisticsNew(kernel);
        if (gqs == NULL) {
            OS_REPORT_1(OS_ERROR,
                        "kernel::v_groupQueue::v_groupQueueNew", 0,
                        "Failed to create Statistics for GroupQueue (name=\"%s\").",
                        name);
            assert(FALSE);
            return NULL;
        }
    } else {
        gqs = NULL;
    }

    q = v_readerQosNew(kernel,qos);

    if (q != NULL) {
        queue = v_groupQueue(v_objectNew(kernel,K_GROUPQUEUE));
        v_groupQueueInit(queue, subscriber, name, maxSize, q, v_statistics(gqs), expr);
        c_free(q); /* ref now in v_reader(queue)->qos */
    } else {
        OS_REPORT(OS_ERROR,
                  "v_groupQueueNew", 0,
                  "v_groupQueue not created: inconsistent qos");
        queue = NULL;
        v_groupQueueStatisticsFree(gqs);
    }

    return queue;
}

void
v_groupQueueInit(
    v_groupQueue queue,
    v_subscriber subscriber,
    const c_char *name,
    c_ulong maxSize,
    v_readerQos qos,
    v_statistics gqs,
    c_iter expr)
{
    assert(C_TYPECHECK(queue, v_groupQueue));
    assert(C_TYPECHECK(subscriber, v_subscriber));

    queue->head    = NULL;
    queue->tail    = NULL;
    queue->marker  = NULL;
    queue->maxSize = maxSize;
    queue->size    = 0;
    queue->markerReached = FALSE;

    v_groupStreamInit(v_groupStream(queue), name, subscriber, qos, gqs, expr);
}

void
v_groupQueueDeinit(
    v_groupQueue queue)
{
    v_groupAction action;

    assert(C_TYPECHECK(queue, v_groupQueue));

    v_groupStreamDeinit(v_groupStream(queue));

    action = v_groupQueueTake(queue);

    while(action){
        c_free(action);
        action = v_groupQueueTake(queue);
    }
    queue->head = NULL;
    queue->tail = NULL;
}

void
v_groupQueueFree(
    v_groupQueue queue)
{
    assert(C_TYPECHECK(queue, v_groupQueue));

    v_groupStreamFree(v_groupStream(queue));
}

/* v_groupQueueTake takes until it encounters the marker */
void
v_groupQueueSetMarker(
    v_groupQueue queue)
{
    assert(C_TYPECHECK(queue,v_groupQueue));

    v_observerLock(v_observer(queue));

    queue->marker = queue->tail;
    queue->markerReached = FALSE;

    v_observerUnlock(v_observer(queue));
}

void
v_groupQueueResetMarker(
    v_groupQueue queue)
{
    assert(C_TYPECHECK(queue,v_groupQueue));

    v_observerLock(v_observer(queue));

    queue->marker = NULL;
    queue->markerReached = FALSE;

    v_observerUnlock(v_observer(queue));
}

v_groupAction
v_groupQueueRead(
    v_groupQueue queue)
{
    v_groupAction action;

    assert(C_TYPECHECK(queue,v_groupQueue));

    v_observerLock(v_observer(queue));

    if (queue->head) {
        action = c_keep(queue->head->action);
        v_statisticsULongValueInc(v_groupQueue, numberOfReads, queue);
    } else {
        action = NULL;
    }
    v_observerUnlock(v_observer(queue));

    return action;
}

v_groupAction
v_groupQueueTake(
    v_groupQueue queue)
{
    v_groupQueueSample sample;
    v_groupAction action;

    assert(C_TYPECHECK(queue,v_groupQueue));

    action = NULL;

    v_observerLock(v_observer(queue));

    if(queue->head){
    	if (!queue->markerReached) {
			sample = queue->head;
			action = c_keep(sample->action);

			if (queue->marker && (queue->marker == sample)) {
				queue->markerReached = TRUE;
			}

			queue->head = sample->next;
			sample->next = NULL;
			queue->size--;
			c_free(sample);

			if(queue->size == 0){
				queue->tail = NULL;
				v_statusReset(v_entity(queue)->status,V_EVENT_DATA_AVAILABLE);
			}
	        v_statisticsULongValueInc(v_groupQueue, numberOfTakes, queue);
	        v_statisticsFullCounterValueMin(v_groupQueue, numberOfSamples, queue);
    	}
    }

    v_observerUnlock(v_observer(queue));

    return action;
}

v_writeResult
v_groupQueueWrite(
    v_groupQueue queue,
    v_groupAction action)
{
    v_writeResult result;
    v_kernel kernel;
    v_groupQueueSample sample;

    assert(C_TYPECHECK(queue,v_groupQueue));
    assert(C_TYPECHECK(action,v_groupAction));

    v_observerLock(v_observer(queue));

    result = V_WRITE_SUCCESS;

    switch(action->kind){
    case V_GROUP_ACTION_REGISTER:             /*fallthrough on purpose.*/
    case V_GROUP_ACTION_UNREGISTER:           /*fallthrough on purpose.*/
        /*Do not handle register & unregister messages*/
        break;
    case V_GROUP_ACTION_WRITE:                /*fallthrough on purpose.*/
    case V_GROUP_ACTION_DISPOSE:              /*fallthrough on purpose.*/
    case V_GROUP_ACTION_LIFESPAN_EXPIRE:      /*fallthrough on purpose.*/
    case V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE: /*fallthrough on purpose.*/
    case V_GROUP_ACTION_DELETE_DATA:
        if((queue->size == queue->maxSize) && (queue->maxSize != 0)){
            result = V_WRITE_REJECTED;
            OS_REPORT(OS_WARNING,
                      "v_groupQueue", 0,
                      "The v_groupQueue is full, message rejected.");
        } else {
            kernel = v_objectKernel(queue);
            sample = c_new(v_kernelType(kernel, K_GROUPQUEUESAMPLE));
            if (sample) {
                sample->action = c_keep(action);
                sample->next   = NULL;

                if(queue->tail){
                    queue->tail->next = sample;
                    queue->tail = sample;
                } else {
                    queue->head = sample;
                    queue->tail = sample;
                }

                /* Floating marker, only set if marker is enabled. */
                if (queue->marker) {
                    queue->marker = sample;
                }

                queue->size++;
                v_groupStreamNotifyDataAvailable(v_groupStream(queue));

                v_statisticsULongValueInc(v_groupQueue, numberOfWrites, queue);
                v_statisticsFullCounterValuePlus(v_groupQueue, numberOfSamples, queue);
            } else {
                OS_REPORT(OS_ERROR,
                          "v_groupQueueWrite",0,
                          "Failed to allocate v_groupQueueSample object.");
                assert(FALSE);
            }
        }
    break;
    default:
        assert(FALSE);
        OS_REPORT_1(OS_ERROR,
                    "v_groupQueueWrite", 0,
                    "Cannot handle unknown write action: '%d'",
                    action->kind);
    break;
    }
    v_observerUnlock(v_observer(queue));

    return result;
}

c_ulong
v_groupQueueSize(
    v_groupQueue _this)
{
    c_ulong size;

    assert(C_TYPECHECK(_this,v_groupQueue));

    if(_this){
        v_observerLock(v_observer(_this));
        size = _this->size;
        v_observerUnlock(v_observer(_this));
    } else {
        size = 0;
    }
    return size;
}

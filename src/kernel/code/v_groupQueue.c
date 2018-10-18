/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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

#include "v__groupQueue.h"
#include "v_groupQueue.h"
#include "v_groupStream.h"
#include "v__groupStream.h"
#include "v_groupQueueStatistics.h"
#include "v_reader.h"
#include "v_readerQos.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v_status.h"
#include "v_state.h"
#include "v_event.h"
#include "c_collection.h"
#include "os_report.h"
#include "v_message.h"
#include "v_statistics.h"
#include "v_fullCounter.h"
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

    if (v_readerQosCheck(qos) != V_RESULT_OK) {
        return NULL;
    }

    if (v_isEnabledStatistics(kernel, V_STATCAT_READER) ||
        v_isEnabledStatistics(kernel, V_STATCAT_DURABILITY)) {
        gqs = v_groupQueueStatisticsNew(kernel);
        if (gqs == NULL) {
            OS_REPORT(OS_ERROR,
                        "kernel::v_groupQueue::v_groupQueueNew", V_RESULT_INTERNAL_ERROR,
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
                  "kernel::v_groupQueue::v_groupQueueNew", V_RESULT_OUT_OF_MEMORY,
                  "Failed to create qos for GroupQueue (name=\"%s\").",
                  name);
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
    v_kernel kernel;

    OS_UNUSED_ARG(gqs);

    assert(C_TYPECHECK(queue, v_groupQueue));
    assert(C_TYPECHECK(subscriber, v_subscriber));

    queue->head    = NULL;
    queue->tail    = NULL;
    queue->marker  = NULL;
    queue->maxSize = maxSize;
    queue->size    = 0;
    queue->markerReached = FALSE;

    kernel = v_objectKernel(queue);
    if (v_isEnabledStatistics(kernel, V_STATCAT_READER) ||
        v_isEnabledStatistics(kernel, V_STATCAT_DURABILITY)) {
        queue->statistics = v_groupQueueStatisticsNew(kernel);
        if (queue->statistics == NULL) {
            OS_REPORT(OS_ERROR,
                        "kernel::v_groupQueue::v_groupQueueInit", V_RESULT_INTERNAL_ERROR,
                        "Failed to create Statistics for GroupQueue (name=\"%s\").",
                        name);
            assert(FALSE);
        }
    } else {
        queue->statistics = NULL;
    }

    v_groupStreamInit(v_groupStream(queue), name, subscriber, qos, expr);
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

    OSPL_LOCK(queue);

    queue->marker = queue->tail;
    queue->markerReached = FALSE;

    OSPL_UNLOCK(queue);
}

void
v_groupQueueResetMarker(
    v_groupQueue queue)
{
    assert(C_TYPECHECK(queue,v_groupQueue));

    OSPL_LOCK(queue);

    queue->marker = NULL;
    queue->markerReached = FALSE;

    OSPL_UNLOCK(queue);
}

v_groupAction
v_groupQueueRead(
    v_groupQueue _this)
{
    v_groupAction action;

    assert(C_TYPECHECK(_this,v_groupQueue));

    OSPL_LOCK(_this);

    if (_this->head) {
        action = c_keep(_this->head->action);
        if (_this->statistics) {
            _this->statistics->numberOfReads++;
        }
    } else {
        action = NULL;
    }
    OSPL_UNLOCK(_this);

    return action;
}

v_groupAction
v_groupQueueTake(
    v_groupQueue _this)
{
    v_groupQueueSample sample;
    v_groupAction action;

    assert(C_TYPECHECK(_this,v_groupQueue));

    action = NULL;

    OSPL_LOCK(_this);

    if(_this->head){
        if (!_this->markerReached) {
            sample = _this->head;
            action = c_keep(sample->action);

            if (_this->marker && (_this->marker == sample)) {
                _this->markerReached = TRUE;
            }

            _this->head = sample->next;
            sample->next = NULL;
            _this->size--;
            c_free(sample);

            if(_this->size == 0){
                _this->tail = NULL;
                v_statusReset(v_entity(_this)->status,V_EVENT_DATA_AVAILABLE);
            }
            if (_this->statistics) {
                _this->statistics->numberOfTakes++;
                v_fullCounterValueDec(&_this->statistics->numberOfSamples);
            }
        }
    }

    OSPL_UNLOCK(_this);

    return action;
}

v_writeResult
v_groupQueueWrite(
    v_groupQueue _this,
    v_groupAction action)
{
    v_writeResult result;
    v_kernel kernel;
    v_groupQueueSample sample;

    assert(C_TYPECHECK(_this,v_groupQueue));
    assert(C_TYPECHECK(action,v_groupAction));

    OSPL_LOCK(_this);

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
    case V_GROUP_ACTION_DELETE_DATA:          /*fallthrough on purpose.*/
    case V_GROUP_ACTION_TRANSACTION_COMPLETE:
        if((_this->size == _this->maxSize) && (_this->maxSize != 0)){
            result = V_WRITE_REJECTED;
            OS_REPORT(OS_WARNING,
                      "v_groupQueue", V_RESULT_PRECONDITION_NOT_MET,
                      "The v_groupQueue is full, message rejected.");
        } else {
            kernel = v_objectKernel(_this);
            sample = c_new(v_kernelType(kernel, K_GROUPQUEUESAMPLE));
            sample->action = c_keep(action);
            sample->next   = NULL;

            if(_this->tail){
                _this->tail->next = sample;
                _this->tail = sample;
            } else {
                _this->head = sample;
                _this->tail = sample;
            }

            /* Floating marker, only set if marker is enabled. */
            if (_this->marker) {
                _this->marker = sample;
            }

            _this->size++;
            v_groupStreamNotifyDataAvailable(v_groupStream(_this));

            if (_this->statistics) {
                _this->statistics->numberOfTakes++;
                v_fullCounterValueDec(&_this->statistics->numberOfSamples);
            }
        }
    break;
    default:
        assert(FALSE);
        OS_REPORT(OS_CRITICAL,
                    "v_groupQueueWrite", V_RESULT_ILL_PARAM,
                    "Cannot handle unknown write action: '%d'",
                    action->kind);
    break;
    }
    OSPL_UNLOCK(_this);

    return result;
}

c_ulong
v_groupQueueSize(
    v_groupQueue _this)
{
    c_ulong size;

    assert(C_TYPECHECK(_this,v_groupQueue));

    if(_this){
        OSPL_LOCK(_this);
        size = _this->size;
        OSPL_UNLOCK(_this);
    } else {
        size = 0;
    }
    return size;
}

v_result
v_groupQueueEnable(
    _Inout_ v_groupQueue _this)
{
    return v_groupStreamEnable(v_groupStream(_this));
}

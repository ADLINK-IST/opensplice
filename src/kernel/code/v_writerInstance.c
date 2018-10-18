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
#include "v_writerInstance.h"
#include "v_state.h"
#include "v__writer.h"
#include "v_writerSample.h"
#include "c_collection.h"
#include "v_observer.h"
#include "v_public.h"
#include "v_writerCache.h"
#include "v__deadLineInstance.h"
#include "v__deadLineInstanceList.h"
#include "v_topic.h"
#include "v__kernel.h"
#include "v_maxValue.h"

#include "os_report.h"

#define CHECK_REFCOUNT(var, i)

void
v_writerInstanceSetKey (
    v_writerInstance instance,
    v_message message)
{
    v_writer writer;
    c_array instanceKeyList;
    c_array messageKeyList;
    c_value value;
    c_ulong i, nrOfKeys;

    assert(C_TYPECHECK(instance,v_writerInstance));
    assert(C_TYPECHECK(message,v_message));

    writer = v_writerInstanceWriter(instance);

    messageKeyList = v_topicMessageKeyList(v_writerTopic(writer));
    instanceKeyList = v_writerKeyList(writer);
    nrOfKeys = c_arraySize(messageKeyList);
    assert(nrOfKeys == c_arraySize(instanceKeyList));
    for (i=0;i<nrOfKeys;i++) {
        value = c_fieldValue(messageKeyList[i],message);
        c_fieldAssign(instanceKeyList[i],instance,value);
        c_valueFreeRef(value);
    }
    c_free(instanceKeyList);
}

v_writerInstance
v_writerInstanceNew(
    v_writer writer)
{
    v_writerInstance _this;

    assert(C_TYPECHECK(writer,v_writer));

    _this = v_writerInstance(c_new_s(writer->instanceType));
    if (_this) {
        v_object(_this)->kernel = v_objectKernel(writer);
        v_objectKind(_this) = K_WRITERINSTANCE;
        v_writerInstanceInit(_this,writer);
    } else {
        OS_REPORT(OS_FATAL, "v_writerInstanceNew", V_RESULT_OUT_OF_RESOURCES,
                  "Failed to allocate v_writerInstance object.");
    }
    return _this;
}

void
v_writerInstanceInit (
    v_writerInstance instance,
    v_writer writer)
{
    assert(C_TYPECHECK(instance,v_writerInstance));
    assert(C_TYPECHECK(writer,v_writer));

    v_deadLineInstanceInit(v_deadLineInstance(instance), v_entity(writer));

    instance->targetCache = v_writerCacheNew(v_objectKernel(writer), V_CACHE_TARGETS);
    instance->messageCount = 0;
    instance->deadlineCount = 0;
    v_instance(instance)->state = L_EMPTY;
    v_writerInstanceSetHead(instance,NULL);
    v_writerInstanceSetTail(instance,NULL);
}

void
v_writerInstanceFree(
    v_writerInstance instance)
{
    assert(C_TYPECHECK(instance,v_writerInstance));

    v_publicFree(v_public(instance));
    c_free(instance);
}

void
v_writerInstanceDeinit(
    v_writerInstance instance)
{
    v_writer writer;

    assert(C_TYPECHECK(instance,v_writerInstance));
    assert((v_writerInstanceTail(instance) == NULL) == v_writerInstanceTestState(instance,L_EMPTY));

    if (instance->targetCache) {
        v_writerCacheDeinit(instance->targetCache);
    }
    writer = v_writerInstanceWriter(instance);
    v_deadLineInstanceListRemoveInstance(writer->deadlineList, v_deadLineInstance(instance));
    v_deadLineInstanceDeinit(v_deadLineInstance(instance));
}

v_message
v_writerInstanceCreateMessage(
    v_writerInstance _this)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_value value;
    c_ulong i, nrOfKeys;
    v_writer writer;
    v_message message = NULL;

    if (_this != NULL) {
        writer = v_writerInstanceWriter(_this);
        message = v_topicMessageNew(v_writerTopic(writer));
        if (message != NULL) {
            messageKeyList = v_topicMessageKeyList(v_writerTopic(writer));
            instanceKeyList = v_writerKeyList(writer);
            assert(c_arraySize(messageKeyList) == c_arraySize(instanceKeyList));
            nrOfKeys = c_arraySize(messageKeyList);
            for (i=0;i<nrOfKeys;i++) {
                value = c_fieldValue(instanceKeyList[i],_this);
                c_fieldAssign(messageKeyList[i],message,value);
                c_valueFreeRef(value);
            }
            c_free(instanceKeyList);
        }
    }
    return message;
}

v_message
v_writerInstanceCreateMessage_s(
    v_writerInstance _this)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_value value;
    c_ulong i, nrOfKeys;
    v_writer writer;
    v_message message = NULL;

    if (_this != NULL) {
        writer = v_writerInstanceWriter(_this);
        message = v_topicMessageNew_s(v_writerTopic(writer));
        if (message != NULL) {
            messageKeyList = v_topicMessageKeyList(v_writerTopic(writer));
            instanceKeyList = v_writerKeyList(writer);
            assert(c_arraySize(messageKeyList) == c_arraySize(instanceKeyList));
            nrOfKeys = c_arraySize(messageKeyList);
            for (i=0;i<nrOfKeys;i++) {
                value = c_fieldValue(instanceKeyList[i],_this);
                c_fieldAssign(messageKeyList[i],message,value);
                c_valueFreeRef(value);
            }
            c_free(instanceKeyList);
        }
    }
    return message;
}


v_writerSample
v_writerInstanceInsert(
    v_writerInstance instance,
    v_writerSample sample)
{
    v_writer writer;
    v_writerSample last;
    v_writerSample cursor;
    v_writerSample result;
    v_message message, m;
    c_equality equality;
    c_bool hadResendsPending;

    assert(instance != NULL);
    assert(sample != NULL);
    assert(C_TYPECHECK(instance,v_writerInstance));
    assert(C_TYPECHECK(sample,v_writerSample));
    assert((v_writerInstanceTail(instance) == NULL) == v_writerInstanceTestState(instance,L_EMPTY));

    hadResendsPending = v__writerInstanceHasResendsPending(instance);
    writer = v_writerInstanceWriter(instance);
    cursor = v_writerInstanceHead(instance);
    last = v_writerInstanceTail(instance);

    /* Find insertion point in instance history.
     * Start at the newest sample and walk to the oldest.
     */
    message = v_writerSampleMessage(sample);
    if ((cursor) && (writer->qos->orderby.v.kind == V_ORDERBY_SOURCETIME)) {
        do {
            m = v_writerSampleMessage(cursor);
            equality = v_messageCompare(message, m);
            assert(equality != C_EQ);
            if ( equality == C_LT) {
                cursor = cursor->next;
            }
        } while ( cursor!= NULL && equality == C_LT);
        if (cursor == NULL && instance->messageCount == writer->depth && v_writerSampleTestState(sample,L_WRITE)) {
            /* the data (WRITE) sample is outdated reached the end of the full list, so abort insertion. */
            return NULL;
        }
    }

    /* only when WRITE message, take history depth into account */
    if (v_writerSampleTestState(sample,L_WRITE)) {
        assert(instance->messageCount <= writer->depth);
        if (instance->messageCount == writer->depth) {
            /* only remove oldest WRITE sample! */
            while ((last != NULL) && (!v_writerSampleTestState(last,L_WRITE))) {
                if (last != cursor) {
                    last = v_writerSample(last->prev);
                } else {
                    /* sample is older than the last data sample (WRITE) and therefore outdated. */
                    return NULL;
                }
            }
            /* remove last WRITE sample. */
            if (last != NULL) {
                if (cursor == last) {
                    cursor = last->next;
                }
                if (last->next != NULL) {
                    last->next->prev = v_writerSample(last->prev);
                } else {
                    instance->last = last->prev;
                }
                if (last->prev != NULL) {
                   /* do not "c_free(v_writerSample(last->prev)->next);",
                    * since last will be returned by this function.
                    */
                    v_writerSample(last->prev)->next = last->next;
                } else {
                    /* Avoiding writerInstanceSetHead because
                     * last->next's reference is transferred
                     */
                    v_writerInstanceTemplate(instance)->sample = last->next;
                }
                last->next = NULL;
                last->prev = NULL;

                if(v__writerNeedsInOrderResends(writer)){
                    v_writerResendItemRemove(writer, v_writerResendItem(last));
                }
            }

            /* Result will be the sample that has been pushed out. */
            CHECK_REFCOUNT(last, 1);
            result = last;
        } else {
            /* Success, return NULL because none has been rejected or
             * pushed out of the history
             */
            instance->messageCount++;
            v_checkMaxSamplesPerInstanceWarningLevel(v_objectKernel(instance), (c_ulong) instance->messageCount);
            result = NULL;
        }
    } else {
        result = NULL; /* no message will be pushed out of history. */
    }

    /* cursor is insertion point if instance not empty.
     * if cursor == NULL it is head if instance not empty or else is end of list.
     */
    if (instance->last == NULL) {
        v_writerInstanceSetTail(instance, sample);
        v_writerInstanceSetHead(instance, sample);
        sample->prev = NULL;
    } else {
        sample->next = cursor; /* Transfer refCount */
        if (cursor == v_writerInstanceHead(instance)) {
            v_writerInstanceSetHead(instance, sample);
            assert(cursor);
            sample->prev = NULL;
            cursor->prev = sample;
        } else if (cursor == NULL) {
            v_writerInstanceTail(instance)->next = c_keep(sample);
            sample->prev = v_writerInstanceTail(instance);
            v_writerInstanceSetTail(instance, sample);
        } else {
            v_writerSample(cursor->prev)->next = c_keep(sample);
            sample->prev = cursor->prev;
            cursor->prev = sample;
        }
        if (sample->next) {
            sample->next->prev = sample;
        }
    }
    sample->instance = instance;

    if (v_writerSampleTestState(sample, L_WRITE)) {
        v_writerInstanceResetState(instance, L_UNREGISTER);
    } else if(v_writerSampleTestState(sample ,L_UNREGISTER)) {
        v_writerInstanceSetState(instance, L_UNREGISTER);
    }
    v_writerInstanceResetState(instance, L_EMPTY);

    if(v__writerNeedsInOrderResends(writer)){
        v_writerResendItemInsert(writer, v_writerResendItem(sample));
    } else {
        if(!hadResendsPending && v__writerInstanceHasResendsPending(instance)) {
            (void) ospl_c_insert(v__writerResendInstances(writer), instance);
        }
    }

    assert((v_writerInstanceTail(instance) == NULL) == v_writerInstanceTestState(instance,L_EMPTY));
    assert(C_TYPECHECK(result,v_writerSample));

    if ((result == NULL) && (v_writerSampleTestState(sample, L_WRITE))) {
        if (writer->statistics) {
            writer->statistics->numberOfSamples++;
            v_maxValueSetValue(&writer->statistics->maxNumberOfSamplesPerInstance,
                               (c_ulong) instance->messageCount);
        }
    }

    return result;
}

v_writerSample
v_writerInstanceRemove (
    v_writerInstance instance,
    v_writerSample sample)
{
    v_writerSample result = NULL;
    v_writer writer;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_writerInstance));
    assert(C_TYPECHECK(sample,v_writerSample));
    assert((v_writerInstanceTail(instance) == NULL) == v_writerInstanceTestState(instance,L_EMPTY));

    if (v_writerInstanceTestState(instance,L_EMPTY)) {
        /* no samples in instance (except for the one for the key values) */
        return NULL;
    }

    writer = v_writerInstanceWriter(instance);

    if (sample != NULL) {
        if (sample->prev != NULL) {
            /* Do a keep here because this next is reference counted */
            v_writerSample(sample->prev)->next = c_keep(sample->next);
        } else {
            if (sample->next == NULL) {
                assert(sample == v_writerInstanceHead(instance));
                assert(sample == v_writerInstanceTail(instance));
                v_writerInstanceSetState(instance,L_EMPTY);
            }
            v_writerInstanceSetHead(instance,sample->next);
        }
        if (sample->next != NULL) {
            sample->next->prev = sample->prev;
        } else {
            v_writerInstanceSetTail(instance,sample->prev);
        }
        if (v_writerSampleTestState(sample, L_WRITE) && (instance->messageCount > 0))
        {
            if (writer->statistics) {
                writer->statistics->numberOfSamples--;
            }
            /* maxSamplesPerInstance does not have to be calculated */
            instance->messageCount--;
        }
        /* prevent that other samples in the list are removed! */
        c_free(sample->next);
        sample->next = NULL;

        if(v__writerNeedsInOrderResends(writer)){
            v_writerResendItemRemove(writer, v_writerResendItem(sample));
        }

        CHECK_REFCOUNT(sample, 1);
        result = sample;
    }

    assert((v_writerInstanceTail(instance) == NULL) == v_writerInstanceTestState(instance,L_EMPTY));
    assert(C_TYPECHECK(result,v_writerSample));

    return result;
}

/* Precondition: make sure that you have the history locked */

c_bool
v_writerInstanceWalk(
    v_writerInstance instance,
    v_writerInstanceWalkAction action,
    c_voidp arg)
{
    v_writerSample sample;
    c_bool proceed = TRUE;

    sample = v_writerInstanceHead(instance);
    while ((proceed) && (sample != NULL)) {
        proceed = action(sample,arg);
        sample = sample->next;
    }
    return proceed;
}

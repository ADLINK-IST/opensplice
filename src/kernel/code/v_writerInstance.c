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
#include "v_writerInstance.h"
#include "v_state.h"
#include "v__writer.h"
#include "v_writerSample.h"
#include "c_collection.h"
#include "v_observer.h"
#include "v_public.h"
#include "v_writerCache.h"
#include "v_instance.h"
#include "v_topic.h"
#include "v__statisticsInterface.h"
#include "v__kernel.h"

#include "os_report.h"

#define CHECK_REFCOUNT(var, i)

v_writerInstance
v_writerInstanceNew(
    v_writer writer,
    v_message message)
{
    v_writerInstance instance;

    assert(C_TYPECHECK(writer,v_writer));
    assert(C_TYPECHECK(message,v_message));

    if (writer->cachedInstance != NULL) {
        instance = writer->cachedInstance;
        writer->cachedInstance = NULL;
    } else {

        instance = v_writerInstance(c_new(writer->instanceType));
        if (instance) {
            v_object(instance)->kernel = v_objectKernel(writer);
            v_objectKind(instance) = K_WRITERINSTANCE;
            instance->writer = (c_voidp)writer;
            instance->targetCache = v_writerCacheNew(v_objectKernel(writer),
                                                        V_CACHE_TARGETS);
        } else {
            OS_REPORT(OS_ERROR,
                      "v_writerInstanceNew",0,
                      "Failed to allocate v_writerInstance object.");
            assert(FALSE);
        }
    }
    v_writerInstanceInit(instance,message);
    return instance;
}

void
v_writerInstanceInit (
    v_writerInstance instance,
    v_message message)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_value value;
    c_long i, nrOfKeys;

    assert(C_TYPECHECK(instance,v_writerInstance));
    assert(C_TYPECHECK(message,v_message));

    v_instanceInit(v_instance(instance));

    instance->sampleSequenceNumber = 1;
    instance->messageCount = 0;
    instance->state = 0;
    instance->deadlineCount = 0;
    instance->resend = FALSE;
    v_writerInstanceSetHead(instance,NULL);
    v_writerInstanceSetTail(instance,NULL);
    v_writerInstanceSetState(instance,L_EMPTY);

    messageKeyList = v_topicMessageKeyList(v_writerTopic(instance->writer));
    instanceKeyList = v_writerKeyList(instance->writer);
    nrOfKeys = c_arraySize(messageKeyList);
    assert(nrOfKeys == c_arraySize(instanceKeyList));
    for (i=0;i<nrOfKeys;i++) {
        value = c_fieldValue(messageKeyList[i],message);
        c_fieldAssign(instanceKeyList[i],instance,value);
        c_valueFreeRef(value);
    }
    c_free(instanceKeyList);

    if (v_messageStateTest(message,L_UNREGISTER)) {
        v_writerInstanceSetState(instance, L_UNREGISTER);
    }
}

void
v_writerInstanceFree(
    v_writerInstance instance)
{
    v_writerSample sample;

    assert(C_TYPECHECK(instance,v_writerInstance));

    if (c_refCount(instance) == 1) {
        sample = v_writerInstanceHead(instance);
        v_writerInstanceSetHead(instance,NULL);
        c_free(sample);
        if (v_writer(instance->writer)->cachedInstance == NULL) {
            v_writer(instance->writer)->cachedInstance = c_keep(instance);
        }

        v_writerCacheDeinit(instance->targetCache);
    }
    c_free(instance);
}

void
v_writerInstanceDeinit(
    v_writerInstance instance)
{
    assert(C_TYPECHECK(instance,v_writerInstance));
    assert((v_writerInstanceTail(instance) == NULL) ==
           v_writerInstanceTestState(instance,L_EMPTY));

    v_instanceDeinit(v_instance(instance));
}

v_message
v_writerInstanceCreateMessage(
    v_writerInstance _this)
{
    c_array instanceKeyList;
    c_array messageKeyList;
    c_value value;
    c_long i, nrOfKeys;
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

v_writerSample
v_writerInstanceInsert(
    v_writerInstance instance,
    v_writerSample sample)
{
    v_writer writer;
    v_writerSample last;
    v_writerSample result;
    v_writerSample firstRef;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_writerInstance));
    assert(C_TYPECHECK(sample,v_writerSample));
    assert((v_writerInstanceTail(instance) == NULL) ==
           v_writerInstanceTestState(instance,L_EMPTY));

    if (sample == NULL) {
        return NULL;
    }
    writer = v_writer(instance->writer);
    /* only when WRITE message, take history depth into account */
    if (v_writerSampleTestState(sample,L_WRITE)) {
        assert(instance->messageCount <= writer->depth);
        if (instance->messageCount == writer->depth) {
            /* only remove oldest WRITE sample! */
            last = v_writerInstanceTail(instance);
            while ((last != NULL) && (!v_writerSampleTestState(last,L_WRITE))) {
                last = v_writerSample(last->prev);
            }
            if (last != NULL) {
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
                     * last->next's reference is transferred */
                    v_writerInstanceTemplate(instance)->sample = last->next;
                }
                last->next = NULL;
                last->prev = NULL;
                CHECK_REFCOUNT(last, 1);
            }
            /* Result will be the sample that has been pushed out. */
            result = last;
        } else {
            /* Success, return NULL because none has been rejected or
             * pushed out of the history
             */
            instance->messageCount++;
            v_checkMaxSamplesPerInstanceWarningLevel(v_objectKernel(instance), instance->messageCount);
            result = NULL;
        }
    } else {
        result = NULL; /* no message will be pushed out of history. */
    }

    firstRef = v_writerInstanceHead(instance);

    if (instance->last == NULL) {
        v_writerInstanceSetTail(instance,sample);
    } else {
        CHECK_REFCOUNT(firstRef, 1);
        sample->next = firstRef; /* Transfer refCount */
        sample->next->prev = sample; /* prev is not refcounted */
    }
    sample->sequenceNumber = ++instance->sampleSequenceNumber;
    v_writerInstanceSetHead(instance,sample);
    if (v_writerSampleTestState(sample,L_UNREGISTER)) {
        v_writerInstanceSetState(instance,L_UNREGISTER);
    }
    if (v_writerSampleTestState(sample,L_WRITE)) {
        v_writerInstanceSetState(instance,L_WRITE);
    }
    sample->prev = NULL;
    v_writerInstanceResetState(instance, L_EMPTY);
    assert((v_writerInstanceTail(instance) == NULL) ==
           v_writerInstanceTestState(instance,L_EMPTY));
    assert(C_TYPECHECK(result,v_writerSample));

    if ((result == NULL) && (v_writerSampleTestState(sample, L_WRITE))) {
        v_statisticsULongValueInc(v_writer, numberOfSamples, writer);
        v_statisticsMaxValueSetValue(v_writer, maxNumberOfSamplesPerInstance,
                                     writer, instance->messageCount);
    }

    return result;
}

v_writerSample
v_writerInstanceRemove (
    v_writerInstance instance,
    v_writerSample sample)
{
    v_writerSample result = NULL;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_writerInstance));
    assert(C_TYPECHECK(sample,v_writerSample));
    assert((v_writerInstanceTail(instance) == NULL) ==
           v_writerInstanceTestState(instance,L_EMPTY));

    if (v_writerInstanceTestState(instance,L_EMPTY)) {
        /* no samples in instance (but the one for the key values */
        return NULL;
    }

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
        if (v_writerSampleTestState(sample, L_WRITE) &&
            (instance->messageCount > 0)) {
            v_statisticsULongValueDec(v_writer, numberOfSamples,
                                      v_writer(instance->writer));
            /* maxSamplesPerInstance does not have to be calculated */
            instance->messageCount--;
        }
        /* prevent that other samples in the list are removed! */
        c_free(sample->next);
        sample->next = NULL;
        result = sample;
    }

    assert((v_writerInstanceTail(instance) == NULL) ==
           v_writerInstanceTestState(instance,L_EMPTY));
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

v_writerSample
v_writerInstanceTakeAll(
     v_writerInstance instance)
{
    v_writerSample oldest;

    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_writerInstance));
    assert((v_writerInstanceTail(instance) == NULL) ==
           v_writerInstanceTestState(instance,L_EMPTY));

    if (v_writerInstanceTestState(instance,L_EMPTY)) {
        oldest = NULL;
    } else {
        oldest = c_keep(v_writerInstanceTail(instance));
        v_writerInstanceSetTail(instance,NULL);
        instance->messageCount = 0;
        v_statisticsULongSetValue(v_writer, numberOfSamples,
                                  v_writerInstanceWriter(instance), 0);
        v_writerInstanceSetState(instance,L_EMPTY);
        v_writerInstanceSetHead(instance,NULL);
    }

    assert((v_writerInstanceTail(instance) == NULL) ==
           v_writerInstanceTestState(instance,L_EMPTY));

    return oldest;
}

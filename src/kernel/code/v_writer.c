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
#include "v__writer.h"
#include "v__writerQos.h"
#include "v__publisher.h"
#include "v__topic.h"
#include "v__group.h"
#include "v__observer.h"
#include "v__kernel.h"
#include "v__leaseManager.h"
#include "v__deadLineInstanceList.h"
#include "v__lease.h"
#include "v__observable.h"
#include "v__status.h"
#include "v__statCat.h"
#include "v__builtin.h"
#include "v__participant.h"
#include "v__deliveryGuard.h"
#include "v__deliveryWaitList.h"
#include "v__dataReader.h"
#include "v__dataReaderSample.h"
#include "v__dataReaderInstance.h"
#include "v__entity.h"
#include "v__transaction.h"

#include "v_partition.h"
#include "v_groupSet.h"
#include "v_state.h"
#include "v_public.h"
#include "v_status.h"
#include "v_writerInstance.h"
#include "v_writerSample.h"
#include "v_writerCache.h"
#include "v__groupInstance.h"
#include "v_event.h"
#include "v_qos.h"
#include "v_policy.h"
#include "v__deadLineInstance.h"
#include "v_writerStatistics.h"
#include "v_message.h"
#include "v_messageQos.h"

#include "c_iterator.h"
#include "c_stringSupport.h"

#include "os_report.h"
#include "vortex_os.h"

#include "stdio.h"

/**************************************************************
 * Private functions
 **************************************************************/
const char*
v_writeResultString(
    v_writeResult result)
{
    const char *image;

#define V__CASE__(result) case result: image = #result; break;
    switch (result) {
        V__CASE__(V_WRITE_UNDEFINED);
        V__CASE__(V_WRITE_SUCCESS);
        V__CASE__(V_WRITE_SUCCESS_NOT_STORED);
        V__CASE__(V_WRITE_REGISTERED);
        V__CASE__(V_WRITE_UNREGISTERED);
        V__CASE__(V_WRITE_PRE_NOT_MET);
        V__CASE__(V_WRITE_ERROR);
        V__CASE__(V_WRITE_TIMEOUT);
        V__CASE__(V_WRITE_OUT_OF_RESOURCES);
        V__CASE__(V_WRITE_REJECTED);
        V__CASE__(V_WRITE_COUNT);
        default:
            image = "Internal error: no image for illegal result value";
            break;
    }
#undef V__CASE__

    return image;
}

static void
deadlineUpdate(
    v_writer writer,
    v_writerInstance instance,
    os_timeE timestamp)
{
    v_deadLineInstanceListUpdate(writer->deadlineList, v_deadLineInstance(instance), timestamp);
    instance->deadlineCount = 0;
}

/* Boolean expression yielding true if writer is synchronous. Abstracts away
 * from the implementation detail that w is synchronous is it has a delivery-
 * guard. This can be changed to an expression evaluating the qos of the writer
 * instead. */
#define v_writerIsSynchronous(w) (w->deliveryGuard)

static v_writerInstance
v_writerNewInstance(
    v_writer _this,
    v_message message)
{
    v_writerInstance instance;

    instance = v_writerInstanceNew(_this);
    if (instance) {
        v_writerInstanceSetKey(instance,message);
    }

    return instance;
}

static void
v_writerFreeInstance(
    v_writerInstance instance)
{
    /* the refcount is stable for a locked writer. */
    if (c_refCount(instance) == 2) {
        v_writerInstanceFree(instance);
    } else {
        c_free(instance);
    }
}

static void
v_writerGroupSetInit (
    struct v_writerGroupSet *set)
{
    set->firstGroup = NULL;
}

/* TODO: generate new-writer event */

static v_writerGroup
v_writerGroupSetAdd (
    v_writer w,
    v_group g)
{
    c_type type;
    v_writerGroup proxy;
    v_kernel kernel;
    struct v_writerGroupSet *set;

    set = &w->groupSet;
    kernel = v_objectKernel(g);
    type = v_kernelType(kernel,K_WRITERGROUP);
    proxy = c_new(type);

    if (proxy) {
        proxy->group = c_keep(g);
        proxy->next = set->firstGroup;
        proxy->targetCache = v_writerCacheNew(kernel, V_CACHE_CONNECTION);
        set->firstGroup = proxy;
    } else {
        OS_REPORT(OS_FATAL,
                  "v_writerGroupSetAdd",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate proxy.");
        assert(FALSE);
    }

    /* Notify the group about this writer being connected. */
    v_groupNotifyWriter(g,w);

    return c_keep(proxy);
}

static v_writerGroup
v_writerGroupSetRemove (
    struct v_writerGroupSet *set,
    v_group g)
{
    v_writerGroup *proxy;
    v_writerGroup foundProxy;

    foundProxy = NULL;
    proxy = &set->firstGroup;
    while (((*proxy) != NULL) && ((*proxy)->group != g)) {
        proxy = &(*proxy)->next;
    }
    if ((*proxy) != NULL) {
        foundProxy = *proxy;
        *proxy = (*proxy)->next;
        foundProxy->next = NULL;
    }
    return foundProxy;
}

typedef c_bool (*v_writerGroupSetWalkAction)(v_writerGroup group, c_voidp arg);

static c_bool
v_writerGroupSetWalk(
    struct v_writerGroupSet *s,
    v_writerGroupSetWalkAction action,
    c_voidp arg)
{
    v_writerGroup proxy;
    c_bool proceed = TRUE;

    proxy = s->firstGroup;
    while ((proceed) && (proxy != NULL)) {
        proceed = action(proxy,arg);
        proxy = proxy->next;
    }
    return proceed;
}

c_bool
v_writerGroupWalk(
    v_writer w,
    v_writerGroupAction action,
    c_voidp arg)
{
    c_bool proceed;

    v_observerLock(v_observer(w));
    proceed = v_writerGroupWalkUnlocked(w, action, arg);
    v_observerUnlock(v_observer(w));

    return proceed;
}

/* This function can only be called in case the observerLock of the writer has
 * been locked prior to calling this function.
 */
c_bool
v_writerGroupWalkUnlocked(
    v_writer w,
    v_writerGroupAction action,
    c_voidp arg)
{
    v_writerGroup proxy;
    c_bool proceed = TRUE;

    proxy = w->groupSet.firstGroup;
    while ((proceed) && (proxy != NULL)) {
        proceed = action(proxy->group,arg);
        proxy = proxy->next;
    }
    return proceed;
}

static void
initMsgQos(
    v_writer w)
{
    assert(w);
    assert(C_TYPECHECK(w,v_writer));

    c_free(w->msgQos); /* free existing value. */
    c_free(w->relQos); /* free existing value. */

    w->msgQos = v_messageQos_new(w);
    if (w->qos->reliability.v.kind == V_RELIABILITY_RELIABLE) {
        w->relQos = c_keep(w->msgQos);
    } else {
        w->relQos = v_messageQos_new(w);
    }
}

void
v_writerResendItemRemove(
    v_writer writer,
    v_writerResendItem ri)
{
    struct v_writerInOrderAdmin * admin;

    assert(writer);
    assert(C_TYPECHECK(writer,v_writer));
    assert(ri);
    assert(C_TYPECHECK(ri,v_writerResendItem));
    assert(v__writerNeedsInOrderResends(writer));
    assert(v__writerInOrderAdminOldest(writer));

    admin = v__writerInOrderAdmin(writer);

    if(ri->newer) {
        assert(v_writerResendItem(ri->newer)->older == ri);
        ri->newer->older = ri->older;
    } else {
        assert(admin->resendNewest == ri);
        admin->resendNewest = ri->older;
    }
    if(ri->older) {
        assert(v_writerResendItem(ri->older)->newer == ri);
        v_writerResendItem(ri->older)->newer = ri->newer; /* Transfer ref */
        ri->newer = NULL;
        c_free(ri);
    } else {
        assert(admin->resendOldest == ri);
        admin->resendOldest = ri->newer; /* Transfer ref */
        ri->newer = NULL;
        c_free(ri);
    }
}

void
v_writerResendItemInsert(
    v_writer writer,
    v_writerResendItem ri)
{
    struct v_writerInOrderAdmin * admin;

    assert(writer);
    assert(C_TYPECHECK(writer,v_writer));
    assert(ri);
    assert(C_TYPECHECK(ri,v_writerResendItem));
    assert(v__writerNeedsInOrderResends(writer));

    assert(ri->newer == NULL);
    assert(ri->older == NULL);

    admin = v__writerInOrderAdmin(writer);

    ri->older = admin->resendNewest;
    if(ri->older) {
        v_writerResendItem(ri->older)->newer = c_keep(ri);
    } else {
        assert(admin->resendOldest == NULL);
    }
    admin->resendNewest = ri;

    if(admin->resendOldest == NULL) {
        admin->resendOldest = c_keep(ri);
    }
}

static void
enqueueSampleForResend (
    v_writer writer,
    v_writerInstance instance,
    v_writerSample sample)
{
    v_writerSample removed;
    v_publisher publisher;
    c_bool hadResendsPending;

    hadResendsPending = v__writerHasResendsPending(writer);

    removed = v_writerInstanceInsert(instance, sample);
    if (removed != NULL) {
        assert(v_writerSampleTestState(removed, L_WRITE));
        c_free(removed);
    } else if(v_writerSampleTestState(sample, L_WRITE)){
        writer->count++;
        v_checkMaxSamplesWarningLevel(v_objectKernel(writer), writer->count);
    }

    if(!hadResendsPending){
        publisher = v_publisher(writer->publisher);
        if (!v_publisherIsSuspended(publisher)) {
            v_participantResendManagerAddWriter(v_publisherParticipant(publisher), writer);
        }
    }
}

/**
 * Implements blocking of writer on resource-limits. If w is a synchronous
 * writer, it will not block at all and just return V_WRITE_OUT_OF_RESOURCES
 * immediately (dds2810). Otherwise, if max-blocking time is not INFINITE
 * (!w->infWait) a wait will be done until until. In case max-blocking time is
 * INFINITE this method will wait indefinitely.
 * @param w The writer that needs to wait
 * @param until The time to wait maximally (only needs to be set to a valid time
 * if w is not synchronous or max-blocking time is not INFINITE.
 * @return the result of the wait:
 *  - V_WRITE_OUT_OF_RESOURCES iff v_writerIsSynchronous(w)
 *  - V_WRITE_TIMEOUT iff max-blocking time !INFINITE and a timeout occurred
 *  - V_WRITE_SUCCESS if no timeout occurred
 *  - V_WRITE_PRE_NOT_MET if the writer was deleted during the wait
 */
static v_writeResult
doWait (
    v_writer w,
    os_timeE until)
{
    os_duration relTimeOut;
    c_ulong flags;
    v_writeResult result;

    if(v_writerIsSynchronous(w)){
        /* In case the writer is synchronous, there will be no blocking on
         * resource limits. In this case the write will immediately return
         * with OUT_OF_RESOURCES. See dds2810 for more details. */
        result = V_WRITE_OUT_OF_RESOURCES;
        OS_REPORT(OS_ERROR, "v_writer::doWait", result,
                  "Out of resources: Synchronous DataWriter out of history resources");
    } else {
        if (w->infWait == FALSE) {
            relTimeOut = os_timeEDiff(until, os_timeEGet());
            if (relTimeOut > 0) {
                flags = v__observerTimedWait(v_observer(w), relTimeOut);
            } else {
                flags = V_EVENT_TIMEOUT;
            }
        } else {
            flags = v__observerWait(v_observer(w));
        }
        if (flags & V_EVENT_OBJECT_DESTROYED) {
            result = V_WRITE_PRE_NOT_MET;
            OS_REPORT(OS_ERROR, "v_writer::doWait", result,
                      "Precondition not met: DataWriter has already been deleted");
        } else {
            if (flags & V_EVENT_TIMEOUT) {
                result = V_WRITE_TIMEOUT;
            } else {
                result = V_WRITE_SUCCESS;
            }
        }
    }
    return result;
}

struct groupWriteArg {
    v_writerInstance instance;
    v_message message;
    v_writeResult result;
    v_resendScope resendScope;
    v_resendScope rejectScope;
};

static c_bool
groupWrite(
    v_writerGroup proxy,
    c_voidp arg)
{
    v_writeResult result;
    v_writerCacheItem item;
    struct groupWriteArg *a = (struct groupWriteArg *)arg;
    v_groupInstance instance;
    v_message message = v_message(a->message);

    assert(proxy != NULL);
    assert(C_TYPECHECK(proxy,v_writerGroup));

    instance = NULL;
    result = v_groupWrite(proxy->group, message, &instance, V_NETWORKID_LOCAL, &a->resendScope);
    if (instance != NULL) {
        item = v_writerCacheItemNew(proxy->targetCache,instance);
        v_writerCacheInsert(proxy->targetCache,item);
        v_writerCacheInsert(a->instance->targetCache,item);
        v_writerInstanceSetState(a->instance, L_REGISTER);
        c_free(instance);
        c_free(item);
    }

    if (result != V_WRITE_SUCCESS) {
        if ((result == V_WRITE_REJECTED) ||
            (a->result == V_WRITE_SUCCESS)) {
            a->result = result;
            a->rejectScope = a->resendScope;
        }
    }

    return TRUE;
}

static c_bool
groupWriteEOT(
    v_writerGroup proxy,
    c_voidp arg)
{
    v_writeResult result;
    struct groupWriteArg *a = (struct groupWriteArg *)arg;

    assert(proxy != NULL);
    assert(C_TYPECHECK(proxy,v_writerGroup));
    assert(a);
    assert(a->instance == NULL);

    result = v_groupWrite(proxy->group, a->message, NULL, V_NETWORKID_LOCAL, &a->resendScope);
    if (result != V_WRITE_SUCCESS) {
        if ((result == V_WRITE_REJECTED) ||
            (a->result == V_WRITE_SUCCESS)) {
            a->result = result;
            a->rejectScope = a->resendScope;
        }
    }

    return TRUE;
}

static c_bool
groupInstanceWrite (
    v_cacheNode node,
    c_voidp arg)
{
    v_writeResult result;
    struct groupWriteArg *a = (struct groupWriteArg *)arg;
    v_writerCacheItem item = v_writerCacheItem(node);

    if (item->instance) {
        result = v_groupInstanceWrite(item->instance, &item->instance, v_message(a->message), &a->resendScope);
        if (result != V_WRITE_SUCCESS) {
            if ((result == V_WRITE_REJECTED) || (a->result == V_WRITE_SUCCESS)) {
                a->result = result;
                a->rejectScope = a->resendScope;
            }
        }
    }

    return TRUE;
}

static c_bool
groupInstanceResend (
    v_cacheNode node,
    c_voidp arg)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerCacheItem item;
    v_resendScope scope;
    struct groupWriteArg *a = (struct groupWriteArg *)arg;
    c_voidp instancePtr;

    item = v_writerCacheItem(node);

    if (item->instance) {
        scope = a->resendScope;
        instancePtr = &(item->instance);
        result = v_groupInstanceResend(item->instance,
                                       instancePtr,
                                       v_message(a->message),
                                       &scope);
        a->rejectScope |= scope;

        if (result != V_WRITE_SUCCESS) {
            if ((result == V_WRITE_REJECTED) || (a->result == V_WRITE_SUCCESS)) {
                a->result = result;
            }
        }
    }
    return TRUE;
}

static c_bool
connectInstance(
    c_object o,
    c_voidp arg)
{
  /**
   * Locally we can never get a reject of a reader based on max_instance
   * resource limits, because the writer determines the number of instances,
   * the reader can never fix the problem that it has reached maximum of the
   * resource limits. In case of a reliable network we also will never receive
   * an in-transit state of a sample!
   * Only in case of an unreliable network it is possible we might need to
   * keep the register sample, since the sample has the state INTRANSIT.
   * In this case we should check resource limits, because we need to keep the
   * sample. However we accept that we exceed the resource limits in order to
   * implement true reliability.
   * The only way this can cause a problem is when an application changes the
   * QoS (partition policy in particular) with a high frequency, which is a
   * bad practice anyway.
   */
    v_writerInstance i = v_writerInstance(o);
    v_writerGroup proxy = (v_writerGroup)arg;
    v_writer w = v_writerInstanceWriter(i);
    v_message message;
    struct groupWriteArg grouparg;

    message = v_writerInstanceCreateMessage(i);
    v_nodeState(message) = L_REGISTER;
    message->writeTime = os_timeWGet();
    message->writerGID = v_publicGid(v_public(w));
    message->sequenceNumber = w->sequenceNumber++;
    message->writerInstanceGID = v_publicGid(v_public(i));
    message->qos = c_keep(w->relQos);

    grouparg.message = message;
    grouparg.instance = i;
    grouparg.result = V_WRITE_SUCCESS;
    grouparg.resendScope = V_RESEND_NONE;
    groupWrite(proxy, &grouparg);

    c_free(message);

    return TRUE;
}

struct writeGroupInstanceArg {
    v_message message;
    v_group group;
    v_cacheNode node;
};

static c_bool
writeGroupInstance(
    v_cacheNode node,
    c_voidp arg)
{
    v_writerCacheItem item = v_writerCacheItem(node);
    struct writeGroupInstanceArg *a = (struct writeGroupInstanceArg *)arg;
    c_bool result;
    v_groupInstance instance;
    v_resendScope resendScope = V_RESEND_NONE;

    /* Calling convention is that the caller presets a->node to NULL, so that
     * he cannot misinterpret the result when no match was found.
     */
    assert(a->node == NULL);


    instance = v_groupInstance(item->instance);
    if (instance && (v_groupInstanceOwner(instance) == a->group)) {
        /* This call is only used for L_DISPOSED and L_UNREGISTER messages,
         * which will never be rejected. */
        (void) v_groupWrite(v_groupInstanceOwner(instance),
                            a->message,
                            &instance,
                            V_NETWORKID_ANY,
                            &resendScope);
        a->node = node;
        result = FALSE;
    } else {
        result = TRUE;
    }

    return result;
}

static void
disconnectInstanceCommon(
    v_writerInstance i,
    v_writerGroup proxy,
    c_bool deleted)
{
  /**
   * The writer history is passed in order to ensure the disconnect get through
   * to the group. Since the group will never reject a DISPOSE or an UNREGISTER
   * this can be safely done. */
    v_writer w = v_writerInstanceWriter(i);
    v_message message;
    struct writeGroupInstanceArg grouparg;
    os_timeW now = os_timeWGet();

    if (deleted && w->qos->lifecycle.v.autodispose_unregistered_instances) {
        /* It is not correct to leave out the DISPOSE in case the state of the
         * writer-instance is DISPOSED, since there is no guarantee the state
         * of the instance in the group and the readers is the same (e.g.,
         * changed by another writer). */
        message = v_writerInstanceCreateMessage(i);
        v_nodeState(message) = L_DISPOSED;
        if (w->transactionStarted) {
            v_nodeState(message) |= L_TRANSACTION;
            v_nodeState(message) |= L_AUTO; /* Used to indicate that this messages needs to be injected on transaction aborts */
        }
        message->transactionId = w->transactionId;
        message->writeTime = now;
        message->writerGID = v_publicGid(v_public(w));
        message->sequenceNumber = w->sequenceNumber++;
        message->writerInstanceGID = v_publicGid(v_public(i));
        message->qos = c_keep(w->relQos);

        grouparg.message = message;
        grouparg.group = proxy->group;
        grouparg.node = NULL;
        (void) v_writerCacheWalk(i->targetCache, writeGroupInstance, &grouparg);
        c_free(message);

        v_writerInstanceSetState(i, L_DISPOSED);
    }

    message = v_writerInstanceCreateMessage(i);
    v_nodeState(message) = L_UNREGISTER;
    if (w->transactionStarted) {
        v_nodeState(message) |= L_TRANSACTION;
    }
    message->transactionId = w->transactionId;
    message->writeTime = now;
    message->writerGID = v_publicGid(v_public(w));
    message->sequenceNumber = w->sequenceNumber++;
    message->writerInstanceGID = v_publicGid(v_public(i));
    message->qos = c_keep(w->relQos);

    grouparg.message = message;
    grouparg.group = proxy->group;
    grouparg.node = NULL;

    /* The v_writerCacheWalk function stops iterating when a cacheNode for a
     * matching group has been found. In that case it aborts the walk with
     * return value FALSE. */
    if (!v_writerCacheWalk(i->targetCache, writeGroupInstance, &grouparg)) {
        /* When a match was found, the resulting node should be non-NULL. */
        assert(grouparg.node);

        /* Now remove the matching node from the pipeline. */
        v_cacheNodeRemove(grouparg.node, V_CACHE_ANY);

        /* If the pipeline has no more remaining nodes, then indicate the
         * absence of a pipeline by resetting the L_REGISTER flag and setting
         * the L_UNREGISTER flag.
         */
        if (v_cacheNode(i->targetCache)->targets.next == NULL) {
            /* No more targets left in pipeline. */
            v_writerInstanceResetState(i, L_REGISTER);
            v_writerInstanceSetState(i, L_UNREGISTER);
        }
    }
    c_free(message);
}

static c_bool
disconnectInstanceForReconnect(
    c_object o,
    c_voidp arg)
{
    v_writerInstance i = v_writerInstance(o);

    /* Only disconnect when there is actually a 'pipeline' */
    if(v_writerInstanceTestState(i, L_REGISTER)) {
        disconnectInstanceCommon(i,(v_writerGroup)arg,FALSE);
    }
    return TRUE;
}

static c_bool
disconnectInstance(
    c_object o,
    c_voidp arg)
{
    v_writerInstance i = v_writerInstance(o);

    /* Only disconnect when there is actually a 'pipeline' */
    if(v_writerInstanceTestState(i, L_REGISTER)) {
        disconnectInstanceCommon(i,(v_writerGroup)arg,TRUE);
    }
    return TRUE;
}

static v_writeResult
writerWrite(
    v_writer writer,
    v_writerInstance instance,
    v_message message)
{
    v_writeResult result;
    struct groupWriteArg grouparg;
    v_writerSample sample = NULL;

    assert(writer != NULL);
    assert(writer == v_writerInstanceWriter(instance));

    if (v_publisherIsSuspended(v_publisher(writer->publisher))) {
        sample = v_writerSampleNew(writer,message);
        if (sample) {
            enqueueSampleForResend(writer, instance, sample);
            c_free(sample);
        }
        result = V_WRITE_SUCCESS;
    } else {
        grouparg.message = message;
        grouparg.instance = instance;
        grouparg.result = V_WRITE_SUCCESS;
        grouparg.resendScope = V_RESEND_NONE;
        grouparg.rejectScope = V_RESEND_NONE;

        if (( v__writerNeedsInOrderResends(writer) && !v__writerHasResendsPending(writer)) ||
            (!v__writerNeedsInOrderResends(writer) && v_writerInstanceTestState(instance, L_EMPTY))) {
            if (v_writerInstanceTestState(instance, L_REGISTER)) {
                (void)v_writerCacheWalk(instance->targetCache, groupInstanceWrite, &grouparg);
            } else {
                v_writerGroupSetWalk(&writer->groupSet, groupWrite, &grouparg);
            }
            result = grouparg.result;
            if (result == V_WRITE_REJECTED) {
                sample = v_writerSampleNew(writer, message);
                assert(grouparg.rejectScope != V_RESEND_NONE);
            }
        } else {
            sample = v_writerSampleNew(writer, message);
            result = V_WRITE_REJECTED;
        }

        if(sample){
            v_writerSampleSetResendScope(sample, grouparg.rejectScope);
            enqueueSampleForResend(writer, instance, sample);
            c_free(sample);
        }
    }
    return result;
}

static v_writeResult
enqueueEotSampleForResend(
    v_writer _this,
    v_message message,
    v_resendScope scope)
{
    v_writerEotSample eot;
    c_bool hadPendingResends;
    v_publisher publisher;
    v_participant participant;

    hadPendingResends = v__writerHasResendsPending(_this);

    eot = c_new(v_kernelType(v_objectKernel(_this), K_WRITEREOTSAMPLE));
    v_writerResendItem(eot)->kind = V_RESENDITEM_WRITEREOTSAMPLE;
    v_writerResendItem(eot)->scope = scope;
    eot->message = c_keep(message);
    v_writerResendItemInsert(_this, v_writerResendItem(eot));
    _this->eotCount++;
    c_free(eot);

    if(!hadPendingResends){
        publisher = v_publisher(_this->publisher);
        if (!v_publisherIsSuspended(publisher)) {
            participant = v_publisherParticipant(publisher);
            v_participantResendManagerAddWriter(participant, _this);
        }
    }
    return V_WRITE_SUCCESS;
}


static v_writeResult
writerWriteEOT(
    v_writer _this,
    v_message message,
    v_resendScope scope)
{
    struct groupWriteArg grouparg;

    grouparg.instance = NULL;
    grouparg.message = message;
    grouparg.result = V_WRITE_SUCCESS;
    grouparg.resendScope = scope;
    grouparg.rejectScope = 0;

    if (v_publisherIsSuspended(v_publisher(_this->publisher))) {
        grouparg.result = enqueueEotSampleForResend(_this, message, scope);
    } else {
        assert(v__writerNeedsInOrderResends(_this) == TRUE);
        if (v__writerHasResendsPending(_this) == FALSE) {
            v_writerGroupSetWalk(&_this->groupSet, groupWriteEOT, &grouparg);
            if (grouparg.result == V_WRITE_REJECTED) {
                grouparg.result = enqueueEotSampleForResend(_this, message, grouparg.rejectScope);
            }
        } else {
            grouparg.result = enqueueEotSampleForResend(_this, message, V_RESEND_NONE);
        }
    }

    return grouparg.result;
}

static v_writeResult
instanceCheckResources(
    v_writerInstance _this,
    v_message message,
    os_timeE until)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writer writer;

    if (v_messageQos_isReliable(message->qos)) {
        writer = v_writerInstanceWriter(_this);
        if (writer->qos->history.v.kind == V_HISTORY_KEEPALL) {
            c_ulong blocked = 0;  /* Used for statistics */

            while ((_this->messageCount >= writer->depth) &&
                   (result == V_WRITE_SUCCESS)) {
                blocked++;
                if(blocked == 1){ /* We only count a blocked write once */
                    if (writer->statistics) {
                        writer->statistics->numberOfWritesBlockedBySamplesPerInstanceLimit++;
                    }
                }
                result = doWait(writer,until);
            }
        }
    }
    return result;
}

static c_equality
compareKeyValue(
    v_writerInstance _this,
    v_message message)
{
    v_writer writer;
    c_array instanceKeyList;
    c_array messageKeyList;
    c_ulong i, nrOfKeys;
    c_equality equality = C_EQ;

    writer = v_writerInstanceWriter(_this);
    messageKeyList = v_topicMessageKeyList(v_writerTopic(writer));
    instanceKeyList = v_writerKeyList(writer);
    assert(c_arraySize(messageKeyList) == c_arraySize(instanceKeyList));
    nrOfKeys = c_arraySize(messageKeyList);
    for (i=0;i<nrOfKeys && equality == C_EQ;i++) {
        equality = c_fieldCompare(messageKeyList[i],message,
                                  instanceKeyList[i],_this);
    }
    c_free(instanceKeyList);
    return equality;
}

static c_bool
writerInstanceAutoPurgeSuspended(
    c_object o,
    c_voidp arg)
{
    v_writerInstance instance = v_writerInstance(o);
    os_timeW *expiry = (os_timeW *)arg;
    v_writer writer = v_writerInstanceWriter(instance);
    v_writerSample sample, found;
    c_bool instanceHadResends;

    assert(expiry);

    instanceHadResends = !v_writerInstanceTestState(instance, L_EMPTY);

    /* Walk from the oldest sample to the newest, so we can stop processing
     * all samples as soon as we find a sample with a write time newer than
     * the expiry time.
     */
    sample = v_writerInstanceTail(instance);
    while ((sample != NULL) && (os_timeWCompare(v_writerSampleMessage(sample)->writeTime, *expiry) != OS_MORE)) {
        found = v_writerInstanceRemove(instance, sample);
        assert(found == sample);
        if(v_writerSampleTestState(found, L_WRITE)) {
            writer->count--;
        }
        c_free(found);
        sample = v_writerInstanceTail(instance);
    }

    if (instanceHadResends && v_writerInstanceTestState(instance, L_EMPTY) && !v__writerNeedsInOrderResends(writer)) {
        v_writerInstance found;

        found = c_remove(v__writerResendInstances(writer), instance, NULL, NULL);
        assert(found == instance);
        c_free(found);
    }

    return TRUE;
}

static void
autoPurgeSuspendedSamples(
    v_writer w)
{
    os_timeW expiry;

    assert(C_TYPECHECK(w,v_writer));

    if (v_publisherIsSuspended(v_publisher(w->publisher))) {
        if (!OS_DURATION_ISINFINITE(w->qos->lifecycle.v.autopurge_suspended_samples_delay)) {
            expiry = os_timeWSub(os_timeWGet(), w->qos->lifecycle.v.autopurge_suspended_samples_delay);
            c_tableWalk(w->instances, writerInstanceAutoPurgeSuspended, &expiry);
        }
    }
}

static void
assertLiveliness (
    v_writer w)
{
    v_kernel kernel;
    v_message builtinMsg;

    v_leaseRenew(w->livelinessLease, w->qos->liveliness.v.lease_duration);
    if (w->alive == FALSE) {
        kernel = v_objectKernel(w);
        w->alive = TRUE;
        if (kernel->builtin && kernel->builtin->kernelQos->builtin.v.enabled) {
            builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
            v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }
    }
}

static v_writeResult
writerDispose(
    v_writer w,
    v_message message,
    os_timeW timestamp,
    v_writerInstance instance,
    c_bool autodispose)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance found;
    v_writerQos qos;
    os_timeE until = OS_TIMEE_ZERO;
    const os_timeE nowEl = message->allocTime;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));

    if (w->statistics) {
        w->statistics->numberOfDisposes++;
    }

    v_nodeState(message) = L_DISPOSED;

    autoPurgeSuspendedSamples(w);

    if (OS_TIMEW_ISINVALID(timestamp)) {
        timestamp = os_timeWGet();
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->writerInstanceGID = v_publicGid(NULL);
    message->transactionId = w->transactionId;
    if (w->transactionStarted) {
        v_nodeState(message) |= L_TRANSACTION;
        if (autodispose) {
            v_nodeState(message) |= L_AUTO; /* Used to indicate that this messages needs to be injected on transaction aborts */
        }
    }
    message->qos = c_keep(w->relQos);

    if (instance == NULL) {
        instance = v_writerNewInstance(w,message);
        if (instance) {
            assert(c_refCount(instance) == 2);
            found = ospl_c_insert(w->instances,instance);
            if (found != instance) {
                result = instanceCheckResources(found,message,until);
            } else {
                assert(c_refCount(instance) == 3);
                qos = w->qos;
                if ((qos->resource.v.max_instances != V_LENGTH_UNLIMITED) &&
                        (c_count(w->instances) > (c_ulong) qos->resource.v.max_instances) &&
                        (result == V_WRITE_SUCCESS)) {
                    result = v_writerIsSynchronous(w) ? V_WRITE_OUT_OF_RESOURCES : V_WRITE_TIMEOUT;
                }
                if (result == V_WRITE_SUCCESS) {
                    assert(c_refCount(instance) == 3);
                    if (w->statistics) {
                        w->statistics->numberOfImplicitRegisters++;
                    }
                    /* The writer statistics are updated for the newly inserted
                     * instance (with its initial values). The previous state
                     * was nothing, so 0 is passed as the oldState. */
                    UPDATE_WRITER_STATISTICS(w, instance, 0);
                } else {
                    found = c_remove(w->instances,instance,NULL,NULL);
                    assert(found == instance);
                    c_free(found);
                    assert(c_refCount(instance) == 1);
                }
            }
            v_writerFreeInstance(instance);
            instance = found;
        } else {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "writerDispose", result,
                      "Out of resources: not enough memory available");
        }
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                result = instanceCheckResources(instance,message,until);
            } else {
                result = V_WRITE_PRE_NOT_MET;
                OS_REPORT(OS_ERROR,
                            "writerDispose", result,
                            "specified instance key value does not match "
                            "data key value for writer %s",
                            v_entityName2(w));
            }
        } else {
            result = V_WRITE_PRE_NOT_MET;
            OS_REPORT(OS_ERROR,
                        "writerDispose", result,
                        "specified instance does not belong to writer %s",
                        v_entityName2(w));
        }
    }

    if (result == V_WRITE_SUCCESS) {
        v_state oldState = v_writerInstanceState(instance);
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;
        deadlineUpdate(w, instance, nowEl);
        if (c_baseMakeMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH)) {
            result = writerWrite(w, instance, message);
            c_baseReleaseMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH);
            v_writerInstanceSetState(instance, L_DISPOSED);
            UPDATE_WRITER_STATISTICS(w, instance, oldState);
        } else {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "writerDispose", result,
                      "Out of resources: not enough memory available");
        }
    } else if(result == V_WRITE_TIMEOUT){
        if (w->statistics) {
            w->statistics->numberOfTimedOutWrites++;
        }
    }


    return result;
}

static v_writeResult
writerUnregister(
    v_writer w,
    v_message message,
    os_timeW timestamp,
    v_writerInstance instance)
{
    v_writeResult result;
    v_writerInstance found;
    v_writerSample sample;
    v_message dispose;
    os_timeE until = OS_TIMEE_ZERO;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));

    /* statistics update */
    if (w->statistics) {
        w->statistics->numberOfUnregisters++;
    }

    v_nodeState(message) |= L_UNREGISTER;

    autoPurgeSuspendedSamples(w);

    if (OS_TIMEW_ISINVALID(timestamp)) {
        timestamp = os_timeWGet();
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->qos = c_keep(w->relQos);

    if (instance == NULL) {
        instance = v_writerNewInstance(w,message);
        if (instance) {
            found = c_tableInsert(w->instances,instance);
            if (found != instance) {
                v_writerFreeInstance(instance);
                instance = found;
                v_deadLineInstanceListRemoveInstance(w->deadlineList,
                        v_deadLineInstance(instance));
                result = instanceCheckResources(instance,message,until);
            } else {
                found = c_remove(w->instances,instance,NULL,NULL);
                assert(found == instance);
                v_writerFreeInstance(found);
                v_writerFreeInstance(instance);
                result = V_WRITE_PRE_NOT_MET;
                OS_REPORT(OS_ERROR, "writerUnregister", result,
                        "Precondition not met: Unregister a non existing Instance");
            }
        } else {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "writerUnregister", result,
                      "Out of resources: not enough memory available");
        }
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                v_deadLineInstanceListRemoveInstance(w->deadlineList,
                                                     v_deadLineInstance(instance));
                result = instanceCheckResources(instance,message,until);
            } else {
                result = V_WRITE_PRE_NOT_MET;
                OS_REPORT(OS_ERROR,
                            "v_writer::writerUnregister", result,
                            "specified instance key value does not match "
                            "data key value for writer %s",
                            v_entityName2(w));
            }
        } else {
            result = V_WRITE_PRE_NOT_MET;
            OS_REPORT(OS_ERROR,
                        "v_writer::writerUnregister", result,
                        "specified instance does not belong to writer %s",
                        v_entityName2(w));
        }
    }

    /* In case of lifecycle.v.autodispose_unregistered_instances ideally
     * one combined message for unregister and dispose should be sent.
     * But because the current readers cannot handle combined messages
     * for now separate messages are sent. also see scdds63.
     */
    if (result == V_WRITE_SUCCESS) {
        if( (!v_writerInstanceTestState(instance, L_DISPOSED)) &&
            (w->qos->lifecycle.v.autodispose_unregistered_instances))
        {
            dispose = v_writerInstanceCreateMessage_s(instance);
            if (dispose) {
                result  = writerDispose(w, dispose, timestamp, instance, TRUE);
                c_free(dispose);
            } else {
                result = V_WRITE_OUT_OF_RESOURCES;
                OS_REPORT(OS_CRITICAL, "writerUnregister", result,
                          "Out of resources: not enough memory available");
            }
        }
    }
    if (result == V_WRITE_SUCCESS) {
        v_state oldState = v_writerInstanceState(instance);
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;
        if (( v__writerNeedsInOrderResends(w) && !v__writerHasResendsPending(w)) ||
            (!v__writerNeedsInOrderResends(w) && v_writerInstanceTestState(instance, L_EMPTY))) {
            /* An unregister-message is never rejected */
            if (c_baseMakeMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH)) {
                (void) writerWrite(w, instance, message);
                c_baseReleaseMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH);
                v_writerInstanceSetState(instance, L_UNREGISTER);
                if (v_writerInstanceTestState(instance, L_EMPTY)) {
                    found = c_remove(w->instances, instance, NULL, NULL);
                    /* Instance is removed from writer, so also subtract related
                     * statistics. */
                    assert(found == instance);
                    UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(w, instance);
                    v_writerCacheDeinit(instance->targetCache);
                    v_writerInstanceResetState(instance, L_REGISTER);
                    v_writerFreeInstance(found);
                } else {
                    UPDATE_WRITER_STATISTICS(w, instance, oldState);
                }
            } else {
                result = V_WRITE_OUT_OF_RESOURCES;
                OS_REPORT(OS_CRITICAL, "writerUnregister", result,
                          "Out of resources: not enough memory available");
            }
        } else {
            sample = v_writerSampleNew(w, message);
            if (sample) {
                enqueueSampleForResend(w, instance, sample);
                c_free(sample);
                UPDATE_WRITER_STATISTICS(w, instance, oldState);
            }
        }
    }

    return result;
}

void
v_writerAssertByPublisher(
    v_writer w)
{
    v_kernel kernel;
    v_message builtinMsg;
    c_bool writeBuiltinSample = FALSE;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    if (w->qos->liveliness.v.kind == V_LIVELINESS_PARTICIPANT) {
        v_observerLock(v_observer(w));
        kernel = v_objectKernel(w);
        if (w->alive == FALSE) {
            w->alive = TRUE;
            if (kernel->builtin && kernel->builtin->kernelQos->builtin.v.enabled) {
                writeBuiltinSample = TRUE;
            }
        }
        v_observerUnlock(v_observer(w));

        v_leaseRenew(w->livelinessLease, w->qos->liveliness.v.lease_duration);
        if (writeBuiltinSample) {
            builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin, w);
            v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }
    }
}


static void
publish(
    void *partition,
    void *writer)
{
    v_partition d = v_partition(partition);
    v_writer w = v_writer(writer);
    v_kernel kernel;
    v_writerGroup proxy;
    v_group g;

    kernel = v_objectKernel(w);
    g = v_groupSetCreate(kernel->groupSet,d,w->topic);

    proxy = v_writerGroupSetAdd(writer,g);
    c_tableWalk(w->instances, connectInstance, proxy);
    c_free(proxy);
}

static void
unpublish(
    void *partition,
    void *writer)
{
    v_partition d = v_partition(partition);
    v_writer w = v_writer(writer);
    v_kernel kernel;
    c_value params[2];
    c_iter list;
    v_group g;
    v_writerGroup proxy;

    params[0] = c_objectValue(d);
    params[1] = c_objectValue(w->topic);
    kernel = v_objectKernel(w);
    list = v_groupSetSelect(kernel->groupSet,
                            "partition = %0 and topic = %1",
                            params);
    assert(c_iterLength(list) <= 1);
    g = v_group(c_iterTakeFirst(list));
    if (g != NULL) {

        proxy = v_writerGroupSetRemove(&w->groupSet,g);
        assert(proxy != NULL);
        c_tableWalk(w->instances, disconnectInstance, proxy);
        v_writerCacheDeinit(proxy->targetCache);
        c_free(proxy);
        c_free(g);
    }
    c_iterFree(list);
}

static c_type
createWriterSampleType(
    v_topic topic)
{
    v_kernel kernel;
    c_type sampleType, baseType, foundType;
    c_metaObject o;
    c_base base;
    c_char *name;
    os_size_t length;
    int sres;

    kernel = v_objectKernel(topic);
    base = c_getBase(kernel);
    baseType = v_kernelType(kernel,K_WRITERSAMPLE);
    assert(baseType != NULL);

    sampleType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
        c_class(sampleType)->extends = c_keep(c_class(baseType));
        o = c_metaDeclare(c_metaObject(sampleType),"message",M_ATTRIBUTE);
        c_property(o)->type = c_keep(v_topicMessageType(topic));
        c_free(o);
    c_metaObject(sampleType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(sampleType));

#define SAMPLE_NAME   "v_writerSample<>"
#define SAMPLE_FORMAT "v_writerSample<%s>"
    /* Create a name and bind type to name */
    /* The sizeof contains \0 */
    length = sizeof(SAMPLE_NAME) + strlen(v_topicName(topic));
    name = os_malloc(length);
    sres = snprintf(name,length,SAMPLE_FORMAT,v_topicName(topic));
    assert(sres >= 0 && (os_size_t) sres == (length-1));
    OS_UNUSED_ARG(sres);
#undef SAMPLE_NAME
#undef SAMPLE_FORMAT

    foundType = c_type(c_metaBind(c_metaObject(base),
                                  name,
                                  c_metaObject(sampleType)));
    os_free(name);
    c_free(sampleType);

    return foundType;
}

static c_type
createWriterInstanceType(
    v_topic topic)
{
    v_kernel kernel;
    c_type instanceType, baseType, foundType;
    c_metaObject o;
    c_base base;
    c_char *name;
    os_size_t length;
    int sres;

    kernel = v_objectKernel(topic);
    base = c_getBase(kernel);
    baseType = v_kernelType(kernel,K_WRITERINSTANCETEMPLATE);
    assert(baseType != NULL);

    instanceType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(instanceType)->extends = c_keep(c_class(baseType));
    foundType = v_topicKeyType(topic);
    if (foundType != NULL) {
        o = c_metaDeclare(c_metaObject(instanceType),"key",M_ATTRIBUTE);
        c_property(o)->type = foundType; /* Transfer refcount */
        c_free(o);
    }
    c_metaObject(instanceType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(instanceType));

#define INSTANCE_NAME "v_writerInstance<v_writerSample<>>"
#define INSTANCE_FORMAT "v_writerInstance<v_writerSample<%s>>"
    /* Create a name and bind type to name */
    /* Ths sizeof contains \0 */
    length = sizeof(INSTANCE_NAME) + strlen(v_topicName(topic));
    name = os_malloc(length);
    sres = snprintf(name,length,INSTANCE_FORMAT,v_topicName(topic));
    assert(sres >= 0 && (os_size_t) sres == (length-1));
    OS_UNUSED_ARG(sres);
#undef INSTANCE_NAME
#undef INSTANCE_FORMAT

    foundType = c_type(c_metaBind(c_metaObject(base),
                                  name,
                                  c_metaObject(instanceType)));
    os_free(name);
    c_free(instanceType);

    return foundType;
}

static c_char *
createInstanceKeyExpr (
    v_topic topic)
{
    c_char fieldName[16];
    c_char *keyExpr;
    c_size i,nrOfKeys,totalSize;
    c_array keyList;

    assert(C_TYPECHECK(topic,v_topic));
    keyList = v_topicMessageKeyList(topic);
    nrOfKeys = c_arraySize(keyList);
    if (nrOfKeys>0) {
        totalSize = nrOfKeys * strlen("key.field0,");
        if (nrOfKeys > 9) {
            totalSize += (nrOfKeys-9);
            if (nrOfKeys > 99) {
                totalSize += (nrOfKeys-99);
            }
        }
        keyExpr = (char *)os_malloc(totalSize);
        keyExpr[0] = 0;
        for (i=0;i<nrOfKeys;i++) {
            os_sprintf(fieldName,"key.field%d",i);
            os_strcat(keyExpr,fieldName);
            if (i<(nrOfKeys-1)) { os_strcat(keyExpr,","); }
        }
    } else {
        keyExpr = NULL;
    }
    return keyExpr;
}


/**
 * Precondition: writer is locked
 * NOTE: writer lock can be released and locked again during this call
 */
static c_bool
writerSingleTransaction(
    v_writer w,
    c_ulong *publisherId,
    c_ulong *transactionId,
    c_array *tidList)
{
    c_bool result = FALSE;
    v_publisher p;
    struct v_tid *tid;

    assert(publisherId != NULL);
    assert(transactionId != NULL);
    assert(tidList != NULL);

    if ((w->publisher) &&
        (w->resend._d != V_PRESENTATION_INSTANCE) &&
        (w->coherent_access == TRUE) &&
        (w->transactionStarted == FALSE)) {
        p = c_keep(w->publisher);
        v_observerUnlock(v_observer(w));

        c_lockWrite(&p->lock);

        result = v__publisherCoherentTransactionSingleNoLock(p, publisherId, transactionId);

        v_observerLock(v_observer(w));
        c_lockUnlock(&p->lock);

        if (result == TRUE) {
            w->transactionStarted = TRUE;
            w->transactionId = w->sequenceNumber;
            if (publisherId != 0) {
                *tidList = c_arrayNew(v_kernelType(v_objectKernel(w), K_TID), 1);
                tid = (struct v_tid *)*tidList;
                tid->wgid = v_publicGid(v_public(w));
                tid->seqnr = w->transactionId;
            } else {
                *tidList = NULL;
            }
        }
        c_free(p);
    }

    return result;
}

static v_message
writerCreateEOT(
    v_writer w,
    os_timeW timestamp,
    c_ulong publisherId,
    c_ulong transactionId,
    c_array tidList)
{
    v_message message;

    message = c_new(v_kernelType(v_objectKernel(w), K_MESSAGEEOT));
    if (message) {
        v_stateSet(v_nodeState(message), L_TRANSACTION | L_ENDOFTRANSACTION);
        message->allocTime = os_timeEGet();
        if (OS_TIMEW_ISINVALID(timestamp)) {
            timestamp = os_timeWGet();
        }
        message->writeTime = timestamp;
        message->writerGID = v_publicGid(v_public(w));
        v_gidSetNil(message->writerInstanceGID);
        message->qos = c_keep(w->msgQos);
        message->sequenceNumber = w->sequenceNumber++;
        message->transactionId = w->transactionId;
        v_messageEOT(message)->publisherId = publisherId;
        v_messageEOT(message)->transactionId = transactionId;
        v_messageEOT(message)->tidList = c_keep(tidList);
    } else {
        OS_REPORT(OS_FATAL,OS_FUNCTION,V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate message.");
        assert(FALSE);
    }
    return message;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_writer
v_writerNew(
    v_publisher p,
    const c_char *name,
    v_topic topic,
    v_writerQos qos)
{
    v_kernel kernel;
    v_writer w = NULL;
    v_writerQos q;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(p);

    if (topic == NULL) {
        return NULL;
    }
    /* ES, dds1576: Before creating the datawriter we have to verify that write
     * access to the topic is allowed. We can accomplish this by checking the
     * access mode of the topic.
     */
    if(v_topicAccessMode(topic) == V_ACCESS_MODE_WRITE ||
       v_topicAccessMode(topic) == V_ACCESS_MODE_READ_WRITE) {
        if (v_writerQosCheck(qos) == V_RESULT_OK) {
            q = v_writerQosNew(kernel, qos);
            if (q != NULL) {
                w = v_writer(v_objectNew(kernel, K_WRITER));
                v_writerInit(w, p, name, topic, q);
                c_free(q); /* ref now in w->qos */
            } else {
                OS_REPORT(OS_ERROR, "v_writerNew", V_RESULT_INTERNAL_ERROR,
                            "Creation of writer <%s> failed. Cannot create writer QoS.",
                            name);
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_writerNew", V_RESULT_INTERNAL_ERROR,
                    "Creation of writer <%s> failed. Topic does not have write access rights.",
                    name);
    }

    return w;
}

void
v_writerInit(
    v_writer writer,
    v_publisher p,
    const c_char *name,
    v_topic topic,
    v_writerQos qos)
{
    v_kernel kernel;
    v_participant participant;
    c_char *keyExpr;
    os_duration deadline;
    os_duration unregister;
    v_publisherQos pubQos;

    assert(writer != NULL);
    assert(p != NULL);
    assert(C_TYPECHECK(writer, v_writer));
    assert(C_TYPECHECK(p,v_publisher));
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(writer);
    if (v_isEnabledStatistics(kernel, V_STATCAT_WRITER)) {
        writer->statistics = v_writerStatisticsNew(kernel);
    } else {
        writer->statistics = NULL;
    }
    v_entityInit(v_entity(writer), name, FALSE);

    writer->count                   = 0;
    writer->eotCount                = 0;
    writer->alive                   = TRUE;
    writer->depth                   = 0x7fffffff; /* MAX_INT */
    writer->topic                   = c_keep(topic);
    writer->qos                     = c_keep(qos);
    pubQos = v_publisherGetQos(p);
    /* Cache the immutable bit of the publisherQos for cheap access by the writer */
    writer->resend._d               = pubQos->presentation.v.access_scope;
    writer->ordered_access          = pubQos->presentation.v.ordered_access;
    writer->coherent_access         = pubQos->presentation.v.coherent_access;
    c_free(pubQos);
    writer->msgQos                  = NULL;
    writer->relQos                  = NULL;
    writer->msgQosType              = NULL;
    writer->publisher               = p;
    writer->deadlineList            = NULL;
    writer->sequenceNumber          = 1;
    writer->livelinessLease         = NULL;
    writer->transactionId           = 0;
    writer->transactionStarted      = FALSE;

    v_writerGroupSetInit(&writer->groupSet);
    writer->instanceType = createWriterInstanceType(topic);
    keyExpr = createInstanceKeyExpr(topic);
    writer->instances = c_tableNew(writer->instanceType, keyExpr);
    if(v__writerNeedsInOrderResends(writer)){
        struct v_writerInOrderAdmin * const admin = v__writerInOrderAdmin(writer);

        admin->resendOldest = NULL;
        admin->resendNewest = NULL;
    } else {
        v__writerResendInstances(writer) = c_tableNew(writer->instanceType, keyExpr);
    }
    os_free(keyExpr);

    writer->sampleType = createWriterSampleType(topic);
    writer->messageField = c_metaResolveProperty(writer->sampleType, "message");

    participant = v_participant(p->participant);
    assert(participant != NULL);

    deadline = qos->deadline.v.period;
    unregister = qos->lifecycle.v.autounregister_instance_delay;

    /* The auto unregister instance delay and deadline missed processing
     * is managed by means of one attribute: deadlineCountLimit.
     * The deadlineCountLimit is the number of successive deadline missed
     * before the auto unregister delay expires.
     * Special values are -1 (no auto unregister delay specified) and
     * 1 (no deadline specified).
     */
    if (unregister == OS_DURATION_INFINITE) {
        writer->deadlineCountLimit = -1;
    } else {
        if (deadline == OS_DURATION_INFINITE || deadline == 0) {
            writer->deadlineCountLimit = 1; /* no deadline */
        } else {
            writer->deadlineCountLimit = (int)(os_durationToReal(unregister) /
                                               os_durationToReal(deadline));
            /* if calculated is zero,
             * this means unregister delay is shorter than deadline delay.
             */
            writer->deadlineCountLimit++;
        }
    }
    if (writer->deadlineCountLimit == 1) {
        writer->deadlineList = v_deadLineInstanceListNew(
                                      c_getBase(c_object(writer)),
                                      participant->leaseManager,
                                      unregister,
                                      V_LEASEACTION_WRITER_DEADLINE_MISSED,
                                      v_public(writer));
    } else {
        writer->deadlineList = v_deadLineInstanceListNew(
                                      c_getBase(c_object(writer)),
                                      participant->leaseManager,
                                      deadline,
                                      V_LEASEACTION_WRITER_DEADLINE_MISSED,
                                      v_public(writer));
    }
}

v_result
v_writerEnable(
    v_writer writer)
{
    v_kernel kernel;
    v_participant participant;
    v_writerQos qos;
    v_result result = V_RESULT_ILL_PARAM;

    if (writer) {
        result = v_publisherAddWriter (writer->publisher, writer);

        if (result == V_RESULT_OK) {
            v_observerLock(v_observer(writer));

            qos = writer->qos;
            if (qos->history.v.kind == V_HISTORY_KEEPLAST) {
                if (qos->history.v.depth >= 0) {
                    writer->depth = qos->history.v.depth;
                }
            } else {
                if (qos->resource.v.max_samples_per_instance >= 0) {
                    writer->depth = qos->resource.v.max_samples_per_instance;
                }
            }
            writer->infWait = OS_DURATION_ISINFINITE(qos->reliability.v.max_blocking_time);

            assert(writer->publisher != NULL);
            participant = v_participant(v_publisher(writer->publisher)->participant);
            assert(participant != NULL);

            /* Register with the lease manager for periodic resending
             * This has to be done for all kinds of reliability because
             * dispose-messages always have to be sent reliably */
            /* The only condition is existence of writer->history */
            if (participant) {
                /* Add writer as observer of participant in case liveliness is
                 * BY PARTICIPANT
                 * This simplifies the liveliness assertion of the participant.
                 */
                if (qos->liveliness.v.kind == V_LIVELINESS_PARTICIPANT) {
                    v_observableAddObserver(v_observable(writer),
                                            v_observer(participant),
                                            NULL);
                    v_observerUnlock(v_observer(writer));
                    v_observerSetEvent(v_observer(participant),
                                       V_EVENT_LIVELINESS_ASSERT);
                    v_observerLock(v_observer(writer));
                }
            }

            kernel = v_objectKernel(writer);
            assert(kernel != NULL);

            /* Register lease for liveliness check, if duration not infinite.
             * When liveliness is AUTOMATIC, the liveliness is determined at
             * node level (i.e. splice daemon)
             */
            if (qos->liveliness.v.kind != V_LIVELINESS_AUTOMATIC) {
                if (!OS_DURATION_ISINFINITE(qos->liveliness.v.lease_duration)) {
                    writer->livelinessLease = v_leaseElapsedNew(kernel, qos->liveliness.v.lease_duration);
                    if(writer->livelinessLease) {
                        result = v_leaseManagerRegister(
                            kernel->livelinessLM,
                            writer->livelinessLease,
                            V_LEASEACTION_LIVELINESS_CHECK,
                            v_public(writer),
                            TRUE /* repeat lease if expired */);
                        if(result != V_RESULT_OK)
                        {
                            c_free(writer->livelinessLease);
                            writer->livelinessLease = NULL;
                            OS_REPORT(OS_CRITICAL, "v_writer", result,
                                "A fatal error was detected when trying to register writer's %p liveliness lease "
                                "to the liveliness lease manager of the kernel. The result code was %d.", (void*)writer, result);
                        }
                    }
                } /* else liveliness also determined by liveliness of node */
            }

            initMsgQos(writer);
            if (kernel->builtin && kernel->builtin->kernelQos->builtin.v.enabled) {
                v_message builtinMsg, builtinCMMsg;
                builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin, writer);
                builtinCMMsg = v_builtinCreateCMDataWriterInfo(kernel->builtin, writer);
                v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
                v_writeBuiltinTopic(kernel, V_CMDATAWRITERINFO_ID, builtinCMMsg);
                c_free(builtinMsg);
                c_free(builtinCMMsg);
            }

            v_observerUnlock(v_observer(writer));

            if (qos->reliability.v.synchronous) {
                writer->deliveryGuard = v_deliveryGuardNew(kernel->deliveryService,writer);
            } else {
                writer->deliveryGuard = NULL;
            }
        }
    }

    return result;
}

static c_bool
removeFromGroup (
    v_writerGroup g,
    c_voidp arg)
{
    v_writer w = v_writer(arg);

    c_tableWalk(w->instances, disconnectInstance, g);

    v_writerCacheDeinit(g->targetCache);
    return TRUE;
}

static c_bool
reconnectToGroup(
    v_writerGroup g,
    c_voidp arg)
{
    v_writer w = v_writer(arg);

    c_tableWalk(w->instances, disconnectInstanceForReconnect, g);
    v_writerCacheDeinit(g->targetCache);
    c_tableWalk(w->instances, connectInstance, g);
    return TRUE;
}

static c_bool
writeLocalEOT(
    v_writerGroup g,
    c_voidp arg)
{
    v_message msg = v_message(arg);
    v_resendScope resendScope = V_RESEND_NONE;

    /* This call is only used after L_DISPOSED and L_UNREGISTER messages,
     * which will never be rejected. */
    (void) v_groupWrite(g->group, msg, NULL, V_NETWORKID_ANY, &resendScope);

    return TRUE;
}

void
v_writerFree(
    v_writer w)
{
    v_kernel kernel;
    v_publisher p;
    v_message builtinMsg, builtinCMMsg;
    v_message unregisterMsg, unregisterCMMsg;
    v_writerInstance instance;
    c_ulong transactionId;
    c_ulong publisherId;
    c_array tidList = NULL;
    c_bool singleTransaction = FALSE;

    assert(C_TYPECHECK(w,v_writer));

    /* A writer cannot dynamically change its Publisher, so not writer lock
     * needed yet. We don't want to take the writer lock here, because that
     * would lock the writer first, and then the publisher, while other
     * functions like v_publisherAddWriter take the locks in reverse order.
     */
    p = v_publisher(w->publisher);
    v_publisherRemoveWriter(p,w);
    assert(p);

    /* Before starting to destroy the writer, make sure its ResendManager
     * has transmitted all pending samples. Because retransmissions will
     * require the writer lock, wait with acquiring the writer lock till
     * after successful completion of this call.
     */
    v_participantResendManagerRemoveWriterBlocking(v_participant(p->participant), w);

    v_observerLock(v_observer(w));
    singleTransaction = writerSingleTransaction(w, &publisherId, &transactionId, &tidList); /* NOTE: this function can re-lock the writer */

    kernel = v_objectKernel(w);

    /* First create message, only at the end dispose. Applications expect
       the disposed sample to be the last!
       In the free algorithm the writer is unpublished from the partitions,
       which also involves the production of the builtin topic.
    */
    builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
    builtinCMMsg = v_builtinCreateCMDataWriterInfo(kernel->builtin,w);
    unregisterMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
    unregisterCMMsg = v_builtinCreateCMDataWriterInfo(kernel->builtin,w);

    v_deadLineInstanceListFree(w->deadlineList);

    v_leaseManagerDeregister(kernel->livelinessLM, w->livelinessLease);

    v_writerGroupSetWalk(&w->groupSet,removeFromGroup,w);

    if (singleTransaction == TRUE) {
        v_message eotMsg;

        eotMsg = writerCreateEOT(w, OS_TIMEW_INVALID, publisherId, transactionId, tidList);
        v_writerGroupSetWalk(&w->groupSet, writeLocalEOT, eotMsg);

        c_free(tidList);
        c_free(eotMsg);
    }

    while ((instance = c_take(w->instances)) != NULL) {
        v_writerFreeInstance(instance);
    }

    w->publisher = NULL;

    v_observerUnlock(v_observer(w));

    if (w->deliveryGuard) {
        v_deliveryGuardFree(w->deliveryGuard);
        w->deliveryGuard = NULL;
    }

    if (w->deliveryGuard) {
        v_deliveryGuardFree(w->deliveryGuard);
        w->deliveryGuard = NULL;
    }

    if (kernel->qos->builtin.v.enabled || (c_tableCount(w->instances) > 0) ) {
        v_writeDisposeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
        v_writeDisposeBuiltinTopic(kernel, V_CMDATAWRITERINFO_ID, builtinCMMsg);
        v_unregisterBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, unregisterMsg);
        v_unregisterBuiltinTopic(kernel, V_CMDATAWRITERINFO_ID, unregisterCMMsg);
    }
    c_free(builtinMsg);
    c_free(builtinCMMsg);
    c_free(unregisterMsg);
    c_free(unregisterCMMsg);

    v_entityFree(v_entity(w));
}

void
v_writerDeinit(
    v_writer w)
{
    v_writerInstance instance;

    if (w == NULL) {
        return;
    }
    assert(C_TYPECHECK(w,v_writer));

    while ((instance = c_take(w->instances)) != NULL) {
        v_writerFreeInstance(instance);
    }

    v_entityDeinit(v_entity(w));
}

/**************************************************************
 * Protected functions
 **************************************************************/
c_bool
v_writerPublishGroup(
    v_writer writer,
    v_group group)
{
    v_writerGroup proxy;

    assert(C_TYPECHECK(writer, v_writer));
    assert(group != NULL);
    assert(C_TYPECHECK(group, v_group));

    if (group->topic == writer->topic) {
        v_observerLock(v_observer(writer));
        proxy = v_writerGroupSetAdd(writer,group);
        c_tableWalk(writer->instances, connectInstance, proxy);
        c_free(proxy);

        v_observerUnlock(v_observer(writer));
    }

    return TRUE;
}

typedef struct v_sampleWalkArg {
    c_iter resendSamples;
    c_iter releaseSamples;
} *v_sampleWalkArg;

void
v_writerNotifyIncompatibleQos(
    v_writer w,
    v_policyId id)
{
    c_bool handled;
    C_STRUCT(v_event) e;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));

    v_statusNotifyOfferedIncompatibleQos(v_entity(w)->status, id);

    e.kind = V_EVENT_OFFERED_INCOMPATIBLE_QOS;
    e.source = v_observable(w);
    e.data = NULL;
    handled = v_entityNotifyListener(v_entity(w), &e);
    v_observerUnlock(v_observer(w));
    if (!handled) {
        v_observableNotify(v_observable(w), &e);
    }
}


void
v_writerNotifyPublicationMatched (
    v_writer w,
    v_gid    readerGID,
    c_bool   dispose)
{
    c_bool handled;
    C_STRUCT(v_event) e;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));
    v_statusNotifyPublicationMatched(v_entity(w)->status, readerGID, dispose);

    e.kind = V_EVENT_PUBLICATION_MATCHED;
    e.source = v_observable(w);
    e.data = NULL;
    handled = v_entityNotifyListener(v_entity(w), &e);
    v_observerUnlock(v_observer(w));
    if (!handled) {
        v_observableNotify(v_observable(w), &e);
    }
}

void
v_writerNotifyChangedQos(
    v_writer w,
    v_writerNotifyChangedQosArg *arg)
{
    v_kernel kernel;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));
    if ((arg != NULL) &&
        ((arg->addedPartitions != NULL) || (arg->removedPartitions != NULL))) {
      /* partition policy has changed */
/**
 * Now the builtin topic is published, after all connections are updated.
 * Depending on the outcome of the RTPS protocol standardisation, this
 * solution is subject to change.
 */
        c_iterWalk(arg->addedPartitions, publish, w);
        c_iterWalk(arg->removedPartitions, unpublish, w);
    }
    kernel = v_objectKernel(w);
    if (kernel->builtin && kernel->builtin->kernelQos->builtin.v.enabled && v_entity(w)->enabled) {
        v_message builtinMsg, builtinCMMsg;
        builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
        builtinCMMsg = v_builtinCreateCMDataWriterInfo(kernel->builtin,w);
        v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
        v_writeBuiltinTopic(kernel, V_CMDATAWRITERINFO_ID, builtinCMMsg);
        c_free(builtinMsg);
        c_free(builtinCMMsg);
    }
    v_observerUnlock(v_observer(w));
}

v_writerQos
v_writerGetQos (
    v_writer _this)
{
    v_writerQos qos;

    assert(_this);
    assert(C_TYPECHECK(_this,v_writer));

    v_observerLock(v_observer(_this));
    qos = c_keep(_this->qos);
    v_observerUnlock(v_observer(_this));

    return qos;
}

v_result
v_writerSetQos(
    v_writer w,
    v_writerQos tmpl)
{
    v_result result;
    v_writerQos qos;
    v_qosChangeMask cm;
    v_message builtinMsg = NULL;
    v_message builtinCMMsg = NULL;
    v_kernel kernel;

    assert(C_TYPECHECK(w,v_writer));

    result = v_writerQosCheck(tmpl);
    if (result == V_RESULT_OK) {
        v_observerLock(v_observer(w));
        kernel = v_objectKernel(w);
        qos = v_writerQosNew(kernel, tmpl);
        if (!qos) {
            v_observerUnlock(v_observer(w));
            return V_RESULT_OUT_OF_MEMORY;
        }
        result = v_writerQosCompare(w->qos, qos, v_entityEnabled(v_entity(w)), &cm);
        if ((result == V_RESULT_OK) && (cm != 0)) {
            c_free(w->qos);
            w->qos = c_keep(qos);
            initMsgQos(w);
            if (cm & V_POLICY_BIT_DEADLINE) {
                v_deadLineInstanceListSetDuration(w->deadlineList, w->qos->deadline.v.period);
            }
            if (cm & (V_POLICY_BIT_DEADLINE | V_POLICY_BIT_LATENCY)) {
                v_writerGroupSetWalk(&w->groupSet, reconnectToGroup, (c_voidp)w);
            }
            if (kernel->builtin && kernel->builtin->kernelQos->builtin.v.enabled && v_entity(w)->enabled) {
                builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin, w);
                builtinCMMsg = v_builtinCreateCMDataWriterInfo(kernel->builtin,w);
            }
        }
        v_observerUnlock(v_observer(w));
        c_free(qos);
    }

    if (builtinMsg != NULL) {
        v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }
    if (builtinCMMsg != NULL) {
        v_writeBuiltinTopic(kernel, V_CMDATAWRITERINFO_ID, builtinCMMsg);
        c_free(builtinCMMsg);
    }

    return result;
}

void
v_writerNotifyLivelinessLost(
    v_writer w)
{
    c_bool handled;
    C_STRUCT(v_event) e;
    v_kernel kernel;
    v_message builtinMsg = NULL;

    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));
    kernel = v_objectKernel(w);
    w->alive = FALSE;
    /* suspend liveliness check */
    v_leaseRenew(w->livelinessLease, OS_DURATION_INFINITE);

    v_statusNotifyLivelinessLost(v_entity(w)->status);
    /* first liveliness lost event */
    e.kind = V_EVENT_LIVELINESS_LOST;
    e.source = v_observable(w);
    e.data = NULL;
    handled = v_entityNotifyListener(v_entity(w), &e);
    if (kernel->builtin && kernel->builtin->kernelQos->builtin.v.enabled) {
       builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
    }
    v_observerUnlock(v_observer(w));
    if (!handled) {
        v_observableNotify(v_observable(w), &e);
    }

    if (builtinMsg != NULL) {
        v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }
}

v_typeRepresentation
v__writerGetTypeRepresentation (
    v_writer _this)
{
    c_char *typeName;
    v_typeRepresentation found;

    typeName = c_metaScopedName(c_metaObject(v_topicDataType(v_writerTopic(_this))));
    found = v_participantLookupTypeRepresentation(v_writerParticipant(_this), typeName);
    os_free(typeName);

    return found;
}

/**************************************************************
 * Public functions
 **************************************************************/

v_writeResult
v_writerRegister(
    v_writer w,
    v_message message,
    os_timeW timestamp,
    v_writerInstance *inst)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance instance, found;

    assert(C_TYPECHECK(w,v_writer));
    assert(message != NULL);

    OS_UNUSED_ARG(timestamp);

    *inst = NULL;

    v_observerLock(v_observer(w));

    if (!w->publisher) {
        v_observerUnlock(v_observer(w));
        OS_REPORT(OS_ERROR, "v_writerRegister", V_WRITE_ERROR,"Writer is in process of deletion, link to publisher already deleted.");
        return V_WRITE_ERROR;
    }

    if (w->statistics) {
        w->statistics->numberOfRegisters++;
    }

    v_nodeState(message) = L_REGISTER;

    autoPurgeSuspendedSamples(w);

    instance = v_writerNewInstance(w,message);
    if (instance) {
        assert(c_refCount(instance) == 2);
        found = c_tableInsert(w->instances,instance);
        if (found != instance) {
            v_state oldState = v_writerInstanceState(found);
            UPDATE_WRITER_STATISTICS(w, found, oldState);

            assert(c_refCount(instance) == 2);
            result = V_WRITE_SUCCESS;
        } else {
            assert(c_refCount(instance) == 3);
            if ((w->qos->resource.v.max_instances != V_LENGTH_UNLIMITED) &&
                    (c_tableCount(w->instances) > (c_ulong) w->qos->resource.v.max_instances) &&
                    (result == V_WRITE_SUCCESS)) {
                result = v_writerIsSynchronous(w) ? V_WRITE_OUT_OF_RESOURCES : V_WRITE_TIMEOUT;
            }
            if (result != V_WRITE_SUCCESS) {
                found = c_remove(w->instances, instance, NULL, NULL);
                assert(found == instance);
                c_free(found);
            }
        }
        v_writerFreeInstance(instance);
    } else {
        result = V_WRITE_OUT_OF_RESOURCES;
        OS_REPORT(OS_CRITICAL, "v_writerRegister", result,
                "Out of resources: not enough memory available");
    }

    if ((result == V_WRITE_SUCCESS) &&
        (inst != NULL)) {
        *inst = c_keep(found);
    }
    v_observerUnlock(v_observer(w));

    return result;
}

v_writerInstance
v_writerLookupInstance(
    v_writer w,
    v_message keyTemplate)
{
    v_writerInstance instance, found = NULL;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(keyTemplate,v_message));

    v_observerLock(v_observer(w));

    instance = v_writerNewInstance(w, keyTemplate);
    if (instance) {
        found = c_find(w->instances, instance);
        v_writerFreeInstance(instance);
    } else {
        OS_REPORT(OS_CRITICAL, "v_writerLookupInstance", V_WRITE_OUT_OF_RESOURCES,
                  "Out of resources: not enough memory available");
    }

    v_observerUnlock(v_observer(w));

    return found;
}

v_writeResult
v_writerUnregister(
    v_writer w,
    v_message message,
    os_timeW timestamp,
    v_writerInstance instance)
{
    v_writeResult result;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));
    assert(message != NULL);

    v_observerLock(v_observer(w));
    if (!w->publisher) {
        v_observerUnlock(v_observer(w));
        OS_REPORT(OS_ERROR, "v_writerUnregister", V_WRITE_ERROR,
                  "Writer is in process of deletion, link to publisher already deleted.");
        return V_WRITE_ERROR;
    }

    if (w->transactionStarted) {
        v_nodeState(message) |= L_TRANSACTION;
    } else if ((w->resend._d != V_PRESENTATION_INSTANCE) &&
        (w->coherent_access == TRUE)) {
        w->transactionId = w->sequenceNumber;
    }
    message->transactionId = w->transactionId;
    result = writerUnregister(w, message, timestamp, instance);

    v_observerUnlock(v_observer(w));

    /* Rewrite internal return code which is for use only in the kernel */
    if ( result == V_WRITE_REJECTED ) {
       result = V_WRITE_SUCCESS;
    }
    return result;
}

v_writeResult
v_writerWrite(
    v_writer w,
    v_message message,
    os_timeW timestamp,
    v_writerInstance instance)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance found;
    v_writerQos qos;
    os_timeE until = OS_TIMEE_ZERO;
    c_ulong blocked; /* Used for statistics */
    enum v_livelinessKind livKind;
    C_STRUCT(v_event) event;
    v_deliveryWaitList waitlist;
    os_duration max_blocking_time = OS_DURATION_ZERO;
    const os_timeE nowEl = message->allocTime;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));

    V_MESSAGE_STAMP(message,writerCopyTime);

    v_observerLock(v_observer(w));
    if (!w->publisher) {
        v_observerUnlock(v_observer(w));
        OS_REPORT(OS_ERROR, "v_writerWrite", V_WRITE_ERROR,"Writer is in process of deletion, link to publisher already deleted.");
        return V_WRITE_ERROR;
    }

    if (w->statistics) {
        w->statistics->numberOfWrites++;
    }

    v_nodeState(message) = L_WRITE;

    autoPurgeSuspendedSamples(w);

    if (OS_TIMEW_ISINVALID(timestamp)) {
        timestamp = os_timeWGet();
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->writerInstanceGID = v_publicGid(NULL);
    if (w->transactionStarted) {
        v_nodeState(message) |= L_TRANSACTION;
    } else if ((w->resend._d != V_PRESENTATION_INSTANCE) &&
        (w->coherent_access == TRUE)) {
        w->transactionId = w->sequenceNumber;
    }
    message->transactionId = w->transactionId;

    qos = w->qos;
    if (!w->infWait) {
        until = os_timeEAdd(nowEl, qos->reliability.v.max_blocking_time);
    }
    blocked = 0;
    while ((qos->history.v.kind == V_HISTORY_KEEPALL ) && (qos->resource.v.max_samples != V_LENGTH_UNLIMITED) &&
           (w->count >= (c_ulong)qos->resource.v.max_samples)) {
        blocked++;
        if(blocked == 1){ /* We only count a blocked write once */
            if (w->statistics) {
                w->statistics->numberOfWritesBlockedBySamplesLimit++;
            }
        }
        result = doWait(w,until);
        if (result != V_WRITE_SUCCESS) {
            if(result == V_WRITE_TIMEOUT || result == V_WRITE_OUT_OF_RESOURCES) {
                /* Both results are a case of (immediate) timeout, so are counted
                 * in this statistic. */
                if (w->statistics) {
                    w->statistics->numberOfTimedOutWrites++;
                }
            }
            v_observerUnlock(v_observer(w));
            return result;
        }
    }
    message->qos = c_keep(w->msgQos);

    if (instance == NULL) {
        instance = v_writerNewInstance(w,message);
        if (instance) {
            assert(c_refCount(instance) == 2);

            found = c_tableInsert(w->instances,instance);
            if (found != instance) {
                result = instanceCheckResources(found,message,until);
            } else {
                assert(c_refCount(instance) == 3);
                if ((qos->resource.v.max_instances != V_LENGTH_UNLIMITED) &&
                        (c_tableCount(w->instances) > (c_ulong) qos->resource.v.max_instances))
                {
                    result = V_WRITE_OUT_OF_RESOURCES;
                    OS_REPORT(OS_ERROR, "v_writerWrite", result,
                            "Out of resources: hit max_instance limit (%d)",
                            qos->resource.v.max_instances);
                }
                if ((qos->resource.v.max_samples != V_LENGTH_UNLIMITED) &&
                        (w->count >= (c_ulong)qos->resource.v.max_samples))
                {
                    result = V_WRITE_OUT_OF_RESOURCES;
                    OS_REPORT(OS_ERROR, "v_writerWrite", result,
                            "Out of resources: hit max_samples limit (%d)",
                            qos->resource.v.max_samples);
                }
                if (result == V_WRITE_SUCCESS) {
                    assert(c_refCount(instance) == 3);
                    if (w->statistics) {
                        w->statistics->numberOfImplicitRegisters++;
                    }
                    /* The writer statistics are updated for the newly inserted
                     * instance (with its initial values). The previous state
                     * was nothing, so 0 is passed as the oldState. */
                    UPDATE_WRITER_STATISTICS(w, instance, 0);
                } else {
                    found = c_remove(w->instances,instance,NULL,NULL);
                    assert(found == instance);
                    c_free(found);
                    assert(c_refCount(instance) == 2);
                }
            }
            v_writerFreeInstance(instance);
            instance = found;
        } else {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "v_writerWrite", result,
                      "Out of resources: not enough memory available");
        }
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                result = instanceCheckResources(instance,message,until);
            } else {
                result = V_WRITE_PRE_NOT_MET;
                OS_REPORT(OS_ERROR,
                            "v_writerWrite", result,
                            "specified instance key value does not match "
                            "data key value for writer %s",
                            v_entityName2(w));
            }
        } else {
            result = V_WRITE_PRE_NOT_MET;
            OS_REPORT(OS_ERROR,
                        "v_writerWrite", result,
                        "specified instance does not belong to writer %s",
                        v_entityName2(w));
        }
    }

    V_MESSAGE_STAMP(message,writerLookupTime);

    waitlist = NULL;

    if (result == V_WRITE_SUCCESS) {
        v_state oldState = v_writerInstanceState(instance);
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;

        if (v_writerIsSynchronous(w)) {
            max_blocking_time = w->qos->reliability.v.max_blocking_time;
            /* Collect all currently known connected synchronous DataReaders */
            waitlist = v_deliveryWaitListNew(w->deliveryGuard,message);
            if (waitlist) {
                v_stateSet(v_nodeState(message), L_SYNCHRONOUS);
            } else {
                result = V_WRITE_OUT_OF_RESOURCES;
            }
        }

        if ((result == V_WRITE_SUCCESS) &&
             c_baseMakeMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH)) {
            result = writerWrite(w, instance, message);
            c_baseReleaseMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH);
#if 0
            if (result == V_WRITE_SUCCESS) {
                /* Successful delivered to all so no need to wait. */
                v_deliveryWaitListFree(waitlist);
                waitlist = NULL;
            }
#endif
            v_writerInstanceResetState(instance, L_DISPOSED);
            v_writerInstanceResetState(instance, L_UNREGISTER);
            deadlineUpdate(w, instance, nowEl);
            UPDATE_WRITER_STATISTICS(w, instance, oldState);
        } else {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "v_writerWrite", result,
                      "Out of resources: not enough memory available");
        }
    } else if (result == V_WRITE_TIMEOUT){
        if (w->statistics) {
            w->statistics->numberOfTimedOutWrites++;
        }
    }

    livKind = qos->liveliness.v.kind;
    assertLiveliness(w);

    v_observerUnlock(v_observer(w));

    if (livKind == V_LIVELINESS_PARTICIPANT) {
        event.kind = V_EVENT_LIVELINESS_ASSERT;
        event.source = v_observable(w);
        event.data = NULL;
        v_observableNotify(v_observable(w), &event);
    }

    if (waitlist) {
        /* The existance of a waitlist implies the writer is synchronous. */
        /* Apparently not all synchronous DataReaders have acknoledged delivery
         * so now call wait on the waitlist.
         */
        v_result r = v_deliveryWaitListWait(waitlist,max_blocking_time);
        switch(r) {
        case V_RESULT_OK:      result = V_WRITE_SUCCESS; break;
        case V_RESULT_TIMEOUT: result = V_WRITE_TIMEOUT; break;
        default:               result = V_WRITE_PRE_NOT_MET; break;
        }
        v_deliveryWaitListFree(waitlist);
    }

    /* Rewrite internal return code which is for use only in the kernel */
    if ( result == V_WRITE_REJECTED ) {
       result = V_WRITE_SUCCESS;
    }
    return result;
}

v_writeResult
v_writerDispose(
    v_writer _this,
    v_message message,
    os_timeW timestamp,
    v_writerInstance instance)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_deliveryWaitList waitlist;
    os_duration max_blocking_time = OS_DURATION_ZERO;

    assert(message != NULL);
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));
    assert(C_TYPECHECK(message,v_message));

    waitlist = NULL;
    v_observerLock(v_observer(_this));

    if (!_this->publisher) {
        v_observerUnlock(v_observer(_this));
        OS_REPORT(OS_ERROR, "v_writerDispose", V_WRITE_ERROR,"Writer is in process of deletion, link to publisher already deleted.");
        return V_WRITE_ERROR;
    }

    if (v_writerIsSynchronous(_this)) {
        max_blocking_time = _this->qos->reliability.v.max_blocking_time;
        waitlist = v_deliveryWaitListNew(_this->deliveryGuard,message);
        if (!waitlist) {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "v_writerWriteDispose", result,
                      "Out of resources: not enough memory available");
        }
    }

    if (result == V_WRITE_SUCCESS) {
        result = writerDispose(_this,message,timestamp,instance, FALSE);
        if (result == V_WRITE_SUCCESS) {
            v_deliveryWaitListFree(waitlist);
            waitlist = NULL;
        }
    }

    v_observerUnlock(v_observer(_this));

    if (waitlist) {
        /* This implies the writer is synchronous. */
        v_result r = v_deliveryWaitListWait(waitlist,max_blocking_time);
        switch(r) {
        case V_RESULT_OK:      result = V_WRITE_SUCCESS; break;
        case V_RESULT_TIMEOUT: result = V_WRITE_TIMEOUT; break;
        default:               result = V_WRITE_PRE_NOT_MET; break;
        }
        v_deliveryWaitListFree(waitlist);
    }

    /* Rewrite internal return code which is for use only in the kernel */
    if ( result == V_WRITE_REJECTED ) {
       result = V_WRITE_SUCCESS;
    }
    return result;
}

v_writeResult
v_writerWriteDispose(
    v_writer w,
    v_message message,
    os_timeW timestamp,
    v_writerInstance instance)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance found;
    v_writerQos qos;
    os_timeE until = OS_TIMEE_ZERO;
    enum v_livelinessKind livKind;
    C_STRUCT(v_event) event;
    v_deliveryWaitList waitlist;
    os_duration max_blocking_time = OS_DURATION_ZERO;
    const os_timeE nowEl = message->allocTime;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));
    assert(message != NULL);
    assert(C_TIME_GET_KIND(nowEl) == C_TIME_ELAPSED);

    v_observerLock(v_observer(w));

    if (!w->publisher) {
        v_observerUnlock(v_observer(w));
        OS_REPORT(OS_ERROR, "v_writerWriteDispose", V_WRITE_ERROR,"Writer is in process of deletion, link to publisher already deleted.");
        return V_WRITE_ERROR;
    }

    if (w->statistics) {
        w->statistics->numberOfWrites++;
        w->statistics->numberOfDisposes++;
    }

    v_nodeState(message) = L_WRITE | L_DISPOSED;

    autoPurgeSuspendedSamples(w);

    if (OS_TIMEW_ISINVALID(timestamp)) {
        timestamp = os_timeWGet();
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->writerInstanceGID = v_publicGid(NULL);
    if (w->transactionStarted) {
        v_nodeState(message) |= L_TRANSACTION;
    } else if ((w->resend._d != V_PRESENTATION_INSTANCE) &&
        (w->coherent_access == TRUE)) {
        w->transactionId = w->sequenceNumber;
    }
    message->transactionId = w->transactionId;

    qos = w->qos;
    if (!w->infWait) {
        until = os_timeEAdd(nowEl, qos->reliability.v.max_blocking_time);
    }
    while ((qos->history.v.kind == V_HISTORY_KEEPALL ) && (qos->resource.v.max_samples != V_LENGTH_UNLIMITED) &&
           (w->count >= (c_ulong)qos->resource.v.max_samples)) {
        result = doWait(w,until);
        if (result != V_WRITE_SUCCESS) {
            v_observerUnlock(v_observer(w));
            return result;
        }
    }
    message->qos = c_keep(w->relQos);

    if (instance == NULL) {
        instance = v_writerNewInstance(w,message);
        if (instance) {
            found = c_tableInsert(w->instances,instance);
            if (found != instance) {
                /* Noop */
            } else {
                if ((((qos->resource.v.max_instances != V_LENGTH_UNLIMITED) &&
                        (c_tableCount(w->instances) > (c_ulong) qos->resource.v.max_instances)) ||
                        ((qos->resource.v.max_samples != V_LENGTH_UNLIMITED) &&
                                (w->count >= (c_ulong)qos->resource.v.max_samples))) &&
                        (result == V_WRITE_SUCCESS)) {
                    result = v_writerIsSynchronous(w) ? V_WRITE_OUT_OF_RESOURCES : V_WRITE_TIMEOUT;
                }
                if (result == V_WRITE_SUCCESS) {
                    assert(c_refCount(instance) == 3);
                    if (w->statistics) {
                        w->statistics->numberOfImplicitRegisters++;
                    }
                    /* The writer statistics are updated for the newly inserted
                     * instance (with its initial values). The previous state
                     * was nothing, so 0 is passed as the oldState. */
                    UPDATE_WRITER_STATISTICS(w, instance, 0);
                } else {
                    found = c_remove(w->instances,instance,NULL,NULL);
                    assert(found == instance);
                    c_free(found);
                }
            }
            v_writerFreeInstance(instance);
            instance = found;
        } else {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "v_writerWriteDispose", result,
                      "Out of resources: not enough memory available");
        }
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                result = instanceCheckResources(instance,message,until);
            } else {
                result = V_WRITE_PRE_NOT_MET;
                OS_REPORT(OS_ERROR,
                            "v_writerWriteDispose", result,
                            "specified instance key value does not match "
                            "data key value for writer %s",
                            v_entityName2(w));
            }
        } else {
            result = V_WRITE_PRE_NOT_MET;
            OS_REPORT(OS_ERROR,
                        "v_writerWriteDispose", result,
                        "specified instance does not belong to writer %s",
                        v_entityName2(w));
        }
    }

    waitlist = NULL;


    if (result == V_WRITE_SUCCESS) {
        v_state oldState = v_writerInstanceState(instance);
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;
        deadlineUpdate(w, instance, nowEl);
        v_writerInstanceSetState(instance, L_DISPOSED);
        v_writerInstanceResetState(instance, L_UNREGISTER);

        if (v_writerIsSynchronous(w)) {
            max_blocking_time = w->qos->reliability.v.max_blocking_time;
            waitlist = v_deliveryWaitListNew(w->deliveryGuard,message);
            if (!waitlist) {
                result = V_WRITE_OUT_OF_RESOURCES;
                OS_REPORT(OS_CRITICAL, "v_writerWriteDispose", result,
                         "Out of resources: not enough memory available");
            }
        }

        if ((result == V_WRITE_SUCCESS) &&
                c_baseMakeMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH)) {
            result = writerWrite(w, instance, message);
            c_baseReleaseMemReservation(c_getBase(w), C_MM_RESERVATION_HIGH);
            if (result == V_WRITE_SUCCESS) {
                v_deliveryWaitListFree(waitlist);
                waitlist = NULL;
            }
            UPDATE_WRITER_STATISTICS(w, instance, oldState);
        } else {
            result = V_WRITE_OUT_OF_RESOURCES;
            OS_REPORT(OS_CRITICAL, "v_writerWriteDispose", result,
                      "Out of resources: not enough memory available");
        }
    }

    livKind = qos->liveliness.v.kind;
    assertLiveliness(w);

    v_observerUnlock(v_observer(w));

    if (livKind == V_LIVELINESS_PARTICIPANT) {
        event.kind = V_EVENT_LIVELINESS_ASSERT;
        event.source = v_observable(w);
        event.data = NULL;
        v_observableNotify(v_observable(w), &event);
    }

    if (waitlist) {
        /* This implies the writer is synchronous. */
        v_result r = v_deliveryWaitListWait(waitlist,max_blocking_time);
        switch(r) {
        case V_RESULT_OK:      result = V_WRITE_SUCCESS; break;
        case V_RESULT_TIMEOUT: result = V_WRITE_TIMEOUT; break;
        default:               result = V_WRITE_PRE_NOT_MET; break;
        }
        v_deliveryWaitListFree(waitlist);
    }

    /* Rewrite internal return code which is for use only in the kernel */
    if ( result == V_WRITE_REJECTED ) {
       result = V_WRITE_SUCCESS;
    }
    return result;
}

c_bool
v_writerPublish(
    v_writer w,
    v_partition d)
{
    assert(C_TYPECHECK(w,v_writer));
    assert(d != NULL);
    assert(C_TYPECHECK(d,v_partition));

    v_observerLock(v_observer(w));

    publish(d, w);

    v_observerUnlock(v_observer(w));

    return TRUE;
}

c_bool
v_writerUnPublish(
    v_writer w,
    v_partition d)
{
    assert(C_TYPECHECK(w,v_writer));
    assert(d != NULL);
    assert(C_TYPECHECK(d,v_partition));

    v_observerLock(v_observer(w));

    unpublish(d, w);

    v_observerUnlock(v_observer(w));

    return TRUE;
}

static v_result
waitForAcknowledgments(
    v_writer w,
    os_duration timeout)
{
    v_result result = V_RESULT_OK;
    os_timeM curTime, endTime;
    os_duration waitTime;
    c_ulong flags;

    assert(C_TYPECHECK(w,v_writer));


    if (OS_DURATION_ISINFINITE(timeout)) {
        while (v__writerHasResendsPending(w)) {
            flags = v__observerWait(v_observer(w));
            if(v__writerHasResendsPending(w) && (flags & V_EVENT_OBJECT_DESTROYED)){
                result = V_RESULT_ILL_PARAM;
                break;
            }
        }
    } else {
        waitTime = timeout;
        curTime = os_timeMGet();
        endTime = os_timeMAdd(curTime, waitTime);

        result = V_RESULT_TIMEOUT;

        while (v__writerHasResendsPending(w) && (os_timeMCompare(curTime, endTime) == OS_LESS)) {
            flags = v__observerTimedWait(v_observer(w), waitTime);

            if(v__writerHasResendsPending(w)) {
                if(flags & V_EVENT_OBJECT_DESTROYED){
                    result = V_RESULT_ILL_PARAM;
                    break;
                } else if (flags & V_EVENT_TIMEOUT) {
                    curTime = endTime;
                } else {
                    curTime = os_timeMGet();
                    waitTime = os_timeMDiff(endTime, curTime);
                }
            }
        }
    }

    if(!v__writerHasResendsPending(w)) {
        result = V_RESULT_OK;
    }

    return result;
}

v_result
v_writerWaitForAcknowledgments(
    v_writer w,
    os_duration timeout)
{
    v_result result;

    assert(C_TYPECHECK(w,v_writer));

    if(w){
        v_observerLock(v_observer(w));

        result = waitForAcknowledgments(w, timeout);

        v_observerUnlock(v_observer(w));
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}


typedef struct collectGarbageArg {
    c_iter emptyList;
} *collectGarbageArg;

static v_writeResult
writerResend(
    v_writer writer,
    v_writerInstance instance,
    v_writerSample sample,
    v_message message)
{
    v_writerSample removed;
    struct groupWriteArg grouparg;

    assert(writer == v_writerInstanceWriter(instance));
    assert(message == v_writerSampleMessage(sample));

    assert(os_timeECompare(v_writerPublisher(writer)->suspendTime, message->allocTime) == OS_MORE);

    if (writer->statistics) {
        writer->statistics->numberOfRetries++;
    }
    grouparg.instance = instance;
    grouparg.message = message;
    grouparg.result = V_WRITE_SUCCESS;
    grouparg.rejectScope = V_RESEND_NONE;

    if(v_writerSampleHasBeenSentBefore(sample)){
        grouparg.resendScope = v_writerResendItem(sample)->scope;
        if (v_writerInstanceTestState(instance, L_REGISTER)) {
            (void)v_writerCacheWalk(instance->targetCache, groupInstanceResend, &grouparg);
        } else {
            v_writerGroupSetWalk(&writer->groupSet, groupWrite, &grouparg);
        }
    } else {
        grouparg.resendScope = V_RESEND_NONE;
        if (v_writerInstanceTestState(instance, L_REGISTER)) {
            (void)v_writerCacheWalk(instance->targetCache, groupInstanceWrite, &grouparg);
        } else {
            v_writerGroupSetWalk(&writer->groupSet, groupWrite, &grouparg);
        }
    }
    if (grouparg.result == V_WRITE_REJECTED) {
        assert(grouparg.rejectScope != V_RESEND_NONE);
        v_writerSampleSetResendScope(sample, grouparg.rejectScope);
    } else {
        if (v_messageStateTest(message,L_UNREGISTER)) {
            v_writerInstanceSetState(instance, L_UNREGISTER);
            v_writerInstanceResetState(instance, L_REGISTER);
            v_writerCacheDeinit(instance->targetCache);
        } else {
            v_writerInstanceResetState(instance, L_UNREGISTER);
            v_writerInstanceSetState(instance, L_REGISTER);
        }

        removed = v_writerInstanceRemove(instance, sample);
        assert(removed == sample);
        if(v_writerSampleTestState(sample, L_WRITE)) {
            writer->count--;
        }
        c_free(removed);
    }

    return grouparg.result;
}

static v_writeResult
writerResendEot(
    v_writer writer,
    v_writerEotSample sample,
    v_message message)
{
    struct groupWriteArg grouparg;

    assert(message == v_writerEotSample(sample)->message);

    assert(os_timeECompare(v_writerPublisher(writer)->suspendTime, message->allocTime) == OS_MORE);

    if (writer->statistics) {
        writer->statistics->numberOfRetries++;
    }
    grouparg.instance = NULL;
    grouparg.message = message;
    grouparg.result = V_WRITE_SUCCESS;
    grouparg.rejectScope = V_RESEND_NONE;
    grouparg.resendScope = v_writerResendItem(sample)->scope;

    v_writerGroupSetWalk(&writer->groupSet, groupWriteEOT, &grouparg);

    if (grouparg.result == V_WRITE_REJECTED) {
        v_writerResendItem(sample)->scope = grouparg.rejectScope;
    } else {
        v_writerResendItemRemove(writer, v_writerResendItem(sample));
        writer->eotCount--;
    }

    return grouparg.result;
}

static v_writeResult
v__writerResendInOrder(
    v_writer writer)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerResendItem ri;
    v_writerInstance instance;
    v_writerSample sample;
    v_message message;

    while(result == V_WRITE_SUCCESS && ((ri = v__writerInOrderAdminOldest(writer)) != NULL)) {
        if(ri->kind == V_RESENDITEM_WRITEREOTSAMPLE) {
            sample = NULL;
            message = v_writerEotSample(ri)->message;
            instance = NULL;
        } else {
            assert(ri->kind == V_RESENDITEM_WRITERSAMPLE);
            sample = v_writerSample(ri);
            message = v_writerSampleMessage(sample);
            instance = sample->instance;
        }

        /**
         * When allocTime of the message is smaller than the suspendTime of the
         * publisher:
         * - the publisher is NOT suspended (suspendTime == C_TIME_INFINITE) or
         * - the publisher IS suspended after this message was written
         * In both cases the message should be resent.
         */
        if (os_timeECompare(v_writerPublisher(writer)->suspendTime, message->allocTime) == OS_MORE) {
            if(ri->kind == V_RESENDITEM_WRITEREOTSAMPLE) {
                result = writerResendEot(writer, v_writerEotSample(ri), message);
            } else {
                c_base base = c_getBase(writer);
                assert(ri->kind == V_RESENDITEM_WRITERSAMPLE);
                if (c_baseMakeMemReservation(base, C_MM_RESERVATION_HIGH)) {
                    result = writerResend(writer, instance, sample, message);
                    c_baseReleaseMemReservation(base, C_MM_RESERVATION_HIGH);
                } else {
                    result = V_WRITE_OUT_OF_RESOURCES;
                }

                if(v_writerInstanceTestState(instance, L_UNREGISTER | L_EMPTY)) {
                    v_writerInstance found = c_remove(writer->instances, instance, NULL, NULL);
                    assert(found == instance);
                    UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(writer, instance);
                    v_writerFreeInstance(found);
                }
            }
        } else {
            /* When all samples that were in the history from before the publisher
             * was suspended are (re)sent, it doesn't make sense to retry until
             * the publisher is resumed. */
            v_participantResendManagerRemoveWriter(v_writerParticipant(writer), writer);
            break;
        }
    }

    return result;
}

static c_bool
v__writerResendInstance(
    v_writerInstance instance,
    c_iter *emptyList)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerSample sample, prev;
    v_message message;
    v_writer writer;

    assert(instance);
    assert(C_TYPECHECK(instance, v_writerInstance));
    assert(!v_writerInstanceTestState(instance, L_EMPTY));

    assert(emptyList);

    writer = v_writerInstanceWriter(instance);

    assert(writer);
    assert(C_TYPECHECK(writer, v_writer));

    sample = v_writerSample(instance->last);
    while (sample && (result == V_WRITE_SUCCESS || result == V_WRITE_SUCCESS_NOT_STORED)) {
        prev = sample->prev;
        message = v_writerSampleMessage(sample);

        /**
         * When allocTime of the message is smaller than the suspendTime of the
         * publisher:
         * - the publisher is NOT suspended (suspendTime == C_TIME_INFINITE) or
         * - the publisher IS suspended after this message was written
         * In both cases the message should be resent.
         */
        if (os_timeECompare(v_writerPublisher(writer)->suspendTime, message->allocTime) == OS_MORE) {
            c_base base = c_getBase(writer);
            if (c_baseMakeMemReservation(base, C_MM_RESERVATION_HIGH)) {
                result = writerResend(writer, instance, sample, message);
                c_baseReleaseMemReservation(base, C_MM_RESERVATION_HIGH);
            } else {
                result = V_WRITE_OUT_OF_RESOURCES;
            }
        }
        sample = prev;
    }

    if (v_writerInstanceTestState(instance, L_EMPTY)) {
        /* If the instance has become empty it is inserted into an emptyList
         * that is returned to the callee. */
         *emptyList = c_iterInsert(*emptyList, c_keep(instance));
    }

    return (result != V_WRITE_OUT_OF_RESOURCES);
}

static c_bool
v__writerResendInstanceAction(
    c_object o,
    c_voidp arg /* c_iter * */)
{
    assert(o != NULL);
    assert(C_TYPECHECK(o, v_writerInstance));

    assert(arg);

    return v__writerResendInstance(v_writerInstance(o), (c_iter *)arg);
}

static v_writeResult
v__writerResendByInstance(
    v_writer writer)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance instance, found;
    c_iter emptyList = NULL;

    if (!c_tableWalk(v__writerResendInstances(writer), &v__writerResendInstanceAction, &emptyList)) {
        result = V_WRITE_OUT_OF_RESOURCES;
    }

    while ((instance = c_iterTakeFirst(emptyList)) != NULL) {
        found = c_remove(v__writerResendInstances(writer), instance, NULL, NULL);
        assert(found == instance);
        c_free(found);
        if (v_writerInstanceTestState(instance, L_UNREGISTER)) {
            found = c_remove(writer->instances, instance, NULL, NULL);
            assert(found == instance);
            UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(writer, instance);
            v_writerCacheDeinit(instance->targetCache);
            v_writerInstanceResetState(instance, L_REGISTER);
            v_writerFreeInstance(found);
        }
        v_writerFreeInstance(instance);
    }

    c_iterFree(emptyList);

    return result;
}

/* Used to resend messages by the resendmanager */
c_bool
v_writerResend(
    v_writer writer)
{
    v_writeResult result;
    c_ulong initialCount;

    assert(writer != NULL);
    assert(C_TYPECHECK(writer,v_writer));

    v_observerLock(v_observer(writer));

    initialCount = writer->count + writer->eotCount;

    if(v__writerNeedsInOrderResends(writer)) {
        result = v__writerResendInOrder(writer);
    } else {
        result = v__writerResendByInstance(writer);
    }

    if(!v__writerHasResendsPending(writer)) {
        /* If this writer has no more resends pending, it can be removed from
         * the resend-manager. Perhaps this shouldn't be done immediately, since
         * it is quite cheap for a writer to be registered with the resend-
         * manager when there is nothing to resend. */
        v_participantResendManagerRemoveWriter(v_writerParticipant(writer), writer);
    }

    if(initialCount > (writer->count + writer->eotCount)) {
        /* Some space was cleared in the history of one of this writer's instances.
         * Space is only cleared if data counting for the resource-limits has been
         * resent. EOT's and instance-state changes don't count. */
        v_observerNotify(v_observer(writer), NULL, NULL);
    }

    v_observerUnlock(v_observer(writer));

    return result != V_WRITE_OUT_OF_RESOURCES;
}

void
v_writerAssertLiveliness(
    v_writer w)
{
    enum v_livelinessKind livKind;
    C_STRUCT(v_event) event;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));
    livKind = w->qos->liveliness.v.kind;
    assertLiveliness(w);
    v_observerUnlock(v_observer(w));
    if (livKind == V_LIVELINESS_PARTICIPANT) {
        event.kind = V_EVENT_LIVELINESS_ASSERT;
        event.source = v_observable(w);
        event.data = NULL;
        v_observableNotify(v_observable(w), &event);
    }
}

v_result
v_writerGetLivelinessLostStatus(
    v_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entityStatus(v_entity(_this));
        result = action(&v_writerStatus(status)->livelinessLost, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_LIVELINESS_LOST);
        }
        v_writerStatus(status)->livelinessLost.totalChanged = 0;
        v_observerUnlock(v_observer(_this));
        c_free(status);
    }
    return result;
}

v_result
v_writerGetDeadlineMissedStatus(
    v_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_writerStatus(status)->deadlineMissed, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_OFFERED_DEADLINE_MISSED);
        }
        v_writerStatus(status)->deadlineMissed.totalChanged = 0;
        v_observerUnlock(v_observer(_this));
    }
    return result;
}

v_result
v_writerGetIncompatibleQosStatus(
    v_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;
    c_ulong i;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_writerStatus(status)->incompatibleQos, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_OFFERED_INCOMPATIBLE_QOS);
        }
        v_writerStatus(status)->incompatibleQos.totalChanged = 0;
        for (i=0; i<V_POLICY_ID_COUNT; i++) {
            v_writerStatus(status)->incompatibleQos.policyCount[i] = 0;
        }
        v_observerUnlock(v_observer(_this));
    }
    return result;
}

v_result
v_writerGetPublicationMatchedStatus(
    v_writer _this,
    c_bool reset,
    v_statusAction action,
    c_voidp arg)
{
    v_result result;
    v_status status;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_writerStatus(status)->publicationMatch, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_PUBLICATION_MATCHED);
        }
        v_writerStatus(status)->publicationMatch.totalChanged = 0;
        v_writerStatus(status)->publicationMatch.currentChanged = 0;
        v_observerUnlock(v_observer(_this));
    }
    return result;
}

v_result
v_writerCheckDeadlineMissed(
    v_writer w,
    os_timeE now)
{
    v_result result = V_RESULT_OK;
    c_bool handled;
    C_STRUCT(v_event) e;
    c_iter missed;
    v_writerInstance instance;
    v_message message;
    os_duration period;
    os_timeW unregisterTime;
    c_bool notify = FALSE;

    v_observerLock(v_observer(w));

    e.kind = V_EVENT_OFFERED_DEADLINE_MISSED;
    e.source = v_observable(w);
    e.data = NULL;

    /*
     * We are dealing with a potential automatic unregister under the
     * following conditions:
     * 1. the deadlineCountLimit equals 1
     * 2. the deadlineList is not empty AND the first instance in the deadline
     *    list has a deadlineCount equal to the deadlineCountLimit minus 1
     */

    if ((w->deadlineCountLimit == 1) ||
        ((!v_deadLineInstanceListEmpty(w->deadlineList)) &&
         (v_writerInstance(v_deadLineInstanceListHead(w->deadlineList))->deadlineCount == w->deadlineCountLimit - 1))) {
        period = w->qos->lifecycle.v.autounregister_instance_delay;
    } else {
        period = w->qos->deadline.v.period;
    }
    missed = v_deadLineInstanceListCheckDeadlineMissed(w->deadlineList, period, now);

    instance = v_writerInstance(c_iterTakeFirst(missed));
    if(instance){
        unregisterTime = os_timeWGet();
    }
    while ((instance != NULL) && (result == V_RESULT_OK)) {
        instance->deadlineCount++;

        /* The deadlineCountlimit drives the behavior of the auto_unregister policy of the writer. It piggybacks
         * on the deadline mechanism and uses the deadlineCountLimit to express the ratio between deadline period and
         * autounregister period.
         */
        if (instance->deadlineCount == w->deadlineCountLimit) { /* unregister */
            message = v_writerInstanceCreateMessage_s(instance);
            if (!message || writerUnregister(w, message, unregisterTime, instance) == V_WRITE_OUT_OF_RESOURCES) {

                result = V_RESULT_OUT_OF_MEMORY;
            }
            c_free(message);
        } else {
            v_statusNotifyOfferedDeadlineMissed(v_entity(w)->status,v_publicHandle(v_public(instance)));

            handled = v_entityNotifyListener(v_entity(w), &e);
            notify = !handled;
        }
        instance = v_writerInstance(c_iterTakeFirst(missed));
    }
    c_iterFree(missed);
    /* next wake-up time only needs to be changed if
     * 1. deadlineCountLimit > 1
     *      if ==1 then
     *         immediate unregister so period was already equal to
     *         autounregister_instance_delay
     *      if <1 then
     *         autounregister_instance_delay is disabled,
     *         so period is deadline.v.period.
     * 2. deadlineInstanceList is not empty
     * 3. first instance in deadlineList is the first to expire,
     *    so only period needs to be adapted if
     *    it almost reached the deadlineCountLimit.
     */
    if ((w->deadlineCountLimit > 1) &&
        (!v_deadLineInstanceListEmpty(w->deadlineList)) &&
        (v_writerInstance(v_deadLineInstanceListHead(w->deadlineList))->deadlineCount == w->deadlineCountLimit - 1)) {
        period = w->qos->lifecycle.v.autounregister_instance_delay - w->qos->deadline.v.period;
        v_deadLineInstanceListSetDuration(w->deadlineList, period);
    }
    if(notify) {
        v_observerUnlock(v_observer(w));
        v_observableNotify(v_observable(w), &e);
    }  else {
        v_observerUnlock(v_observer(w));
    }

    return result;
}

struct instanceActionArg {
    v_writerInstanceWalkAction action;
    c_voidp arg;
};

static c_bool
instanceRead(
    c_object o,
    c_voidp arg)
{
    struct instanceActionArg *a = (struct instanceActionArg *)arg;
    v_writerInstance instance = (v_writerInstance)o;

    v_writerInstanceWalk(instance,a->action,a->arg);
    return TRUE;
}

c_bool
v_writerRead (
    v_writer writer,
    c_action action,
    c_voidp arg)
{
    c_bool result = TRUE;
    struct instanceActionArg a;

    assert(C_TYPECHECK(writer,v_writer));
    assert(action != NULL);

    v_observerLock(v_observer(writer));
    a.action = (v_writerInstanceWalkAction)action;
    a.arg = arg;
    c_tableWalk(writer->instances, instanceRead, &a);
    v_observerUnlock(v_observer(writer));
    return result;
}

void
v_writerResumePublication(
    v_writer writer,
    const os_timeW *resumeTime)
{
    os_timeW expiry;

    assert(C_TYPECHECK(writer,v_writer));
    assert(resumeTime);
    assert(C_TIME_GET_KIND(*resumeTime) == C_TIME_REALTIME);

    v_observerLock(v_observer(writer));

    /* Auto-purge last remaining samples, since publisher was suspended but
     * now resumed.
     * Do not use the static function autoPurgeSuspendedSamples() as this
     * function will check whether the publisher is suspended, while it is
     * not anymore.
     */
    if (!OS_DURATION_ISINFINITE(writer->qos->lifecycle.v.autopurge_suspended_samples_delay)) {
        expiry = os_timeWSub(*resumeTime, writer->qos->lifecycle.v.autopurge_suspended_samples_delay);
        c_tableWalk(writer->instances, writerInstanceAutoPurgeSuspended, &expiry);
    }

    if(v__writerHasResendsPending(writer)){
        /* There is stuff to be resent, so add writer to resend-manager again.
         * It is possible that the writer was still registered with the resend-
         * manager (when there are still unsuspended samples in the history),
         * but that shouldn't matter. */
        v_participantResendManagerAddWriter(v_publisherParticipant(writer->publisher), writer);
    }

    v_observerUnlock(v_observer(writer));
}

void
v_writerCoherentBegin (
    v_writer _this,
    c_ulong *transactionId)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));
    assert(transactionId != NULL);

    v_observerLock(v_observer(_this));
    _this->transactionId = _this->sequenceNumber;
    _this->transactionStarted = TRUE;
    *transactionId = _this->transactionId;
    v_observerUnlock(v_observer(_this));
}

v_result
v_writerCoherentEnd (
    v_writer _this,
    c_ulong publisherId,
    c_ulong transactionId,
    c_array tidList)
{
    v_message message;
    v_result result = V_RESULT_INTERNAL_ERROR;
    v_writeResult wResult;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    v_observerLock(v_observer(_this));

    /* This function instantiates a new (dummy) message from scratch, one
     * that indicates that it no longer belongs to the coherent set, thus
     * effectively ending the coherent set.
     */
    message = c_new(v_kernelType(v_objectKernel(_this), K_MESSAGEEOT));

    /* The end-of-transaction marker is recognized by the fact that
     * it still has its L_TRANSACTION bit set, but that it also has
     * the L_ENDOFTRANSACTION bit set. */
    v_stateSet(v_nodeState(message), L_TRANSACTION | L_ENDOFTRANSACTION);
    message->allocTime = os_timeEGet();
    message->writeTime = os_timeWGet();
    message->writerGID = v_publicGid(v_public(_this));
    v_gidSetNil(message->writerInstanceGID);
    message->qos = c_keep(_this->msgQos);
    message->sequenceNumber = _this->sequenceNumber++;
    message->transactionId = _this->transactionId;
    v_messageEOT(message)->publisherId = publisherId;
    v_messageEOT(message)->transactionId = transactionId;
    v_messageEOT(message)->tidList = c_keep(tidList);
    wResult = writerWriteEOT(_this, message, V_RESEND_NONE);
    if (wResult == V_WRITE_SUCCESS || wResult == V_WRITE_REJECTED) {
        if(publisherId){
            /* If the publisherId is set, this writer is participating in a
             * group-coherent update. In that case, we need to flush all
             * writer-histories so that a newer transaction cannot be
             * delivered before this one. It is possible to do this more
             * fine-grained, but functionally this is correct. */
            result = waitForAcknowledgments(_this, OS_DURATION_INFINITE);
        } else {
            result = V_RESULT_OK;
        }
    } else {
        result = V_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_ERROR, "v_writerCoherentEnd", result,
                  "Received unexpected writeResult from writerWrite(): %d", wResult);
    }
    c_free (message);

    _this->transactionStarted = FALSE;
    v_observerUnlock(v_observer(_this));

    return result;
}

c_bool
v_writerContainsInstance(
    v_writer _this,
    v_writerInstance instance)
{
    v_writer instanceWriter;
    c_bool result = FALSE;

    assert(C_TYPECHECK(_this, v_writer));
    /* It is possible in the language binding to put a writerInstance or dataReaderInstance in the instance parameter.
     * In the kernel we simply assume it is always the correct type we need. This is not true. In a release build this causes
     * no problem because c_checkType is not called on the instance. In dev/debug mode this is called and causes the
     * v_writerInstanceWriter macro to dereference a null pointer in case of a wrong instance type.
     */
#ifdef _TYPECHECK_
    if(!C_TYPECHECK(instance, v_writerInstance)) {
        return result;
    }
#endif

    instanceWriter = v_writerInstanceWriter(instance);
    if (instanceWriter != NULL) {
        result = (instanceWriter == _this);
    } else {
        OS_REPORT(OS_ERROR, "v_writerContainsInstance", V_RESULT_INTERNAL_ERROR,
            "Invalid writerInstance: no attached DataWriter"
            "<_this = 0x%"PA_PRIxADDR" instance = 0x%"PA_PRIxADDR">", (os_address)_this, (os_address)instance);
        result = FALSE;
    }
    return result;
}

c_ulong
v_writerAllocSequenceNumber (
    v_writer _this)
{
  c_ulong seqNr;
  v_observerLock(v_observer(_this));
  seqNr = _this->sequenceNumber++;
  v_observerUnlock(v_observer(_this));
  return seqNr;
}

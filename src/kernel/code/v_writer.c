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
#include "v__statisticsInterface.h"
#include "v__builtin.h"
#include "v__participant.h"
#include "v__messageQos.h"
#include "v__deliveryGuard.h"
#include "v__deliveryWaitList.h"
#include "v__dataReader.h"
#include "v__dataReaderSample.h"
#include "v__dataReaderInstance.h"

#include "v_partition.h"
#include "v_groupSet.h"
#include "v_time.h"
#include "v_state.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_status.h"
#include "v_writerInstance.h"
#include "v_writerSample.h"
#include "v_writerCache.h"
#include "v__groupInstance.h"
#include "v_event.h"
#include "v_qos.h"
#include "v_policy.h"
#include "v_instance.h"
#include "v_statistics.h"
#include "v_writerStatistics.h"
#include "v_message.h"

#include "c_iterator.h"
#include "c_stringSupport.h"

#include "os_report.h"
#include "os.h"

#include "stdio.h"

#define _INTRANSIT_DECAY_COUNT_ (3)

/**************************************************************
 * Private functions
 **************************************************************/
static const char* v_writeResultStr[] = {
    "UNDEFINED",
    "SUCCESS",
    "SUCCESS_NOT_STORED",
    "REGISTERED",
    "UNREGISTERED",
    "PRE_NOT_MET",
    "ERROR",
    "TIMEOUT",
    "OUT_OF_RESOURCES",
    "REJECTED",
    "COUNT"};

const char*
v_writeResultString(
    v_writeResult result)
{
    assert (result <= V_WRITE_COUNT);

    return v_writeResultStr[result];
}

static void
deadlineUpdate(
    v_writer writer,
    v_writerInstance instance,
    c_time timestamp)
{
    v_deadLineInstanceListUpdate(writer->deadlineList, v_instance(instance), timestamp);
    instance->deadlineCount = 0;
}

/* Boolean expression yielding true if writer is synchronous. Abstracts away
 * from the implementation detail that w is synchronous is it has a delivery-
 * guard. This can be changed to an expression evaluating the qos of the writer
 * instead. */
#define v_writerIsSynchronous(w) (w->deliveryGuard)

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
    C_STRUCT(v_event) event;
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
        OS_REPORT(OS_ERROR,
                  "v_writerGroupSetAdd",0,
                  "Failed to allocate proxy.");
        assert(FALSE);
    }

    /* Connect writer event
     *
     * This event is introduced for durability to be able to act on a writer connecting to a group, which
     * signals that that group is being written to. For durability, this is a trigger to mark the
     * corresponding namespace with an infinite quality, which causes potential delayed alignment actions
     * of late-joining persistent sources to be discarded.
     */
    event.kind = V_EVENT_CONNECT_WRITER;
    event.source = v_publicHandle(v_public(w));
    event.userData = g;
    v_observableNotify(v_observable(kernel),&event);

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
    if (w->qos->reliability.kind == V_RELIABILITY_RELIABLE) {
        w->relQos = c_keep(w->msgQos);
    } else {
        w->relQos = v_messageQos_new(w);
    }
}

static void
keepSample (
    v_writer writer,
    v_writerInstance instance,
    v_writerSample sample)
{
    v_writerSample removed;
    v_writerInstance found;
    v_publisher publisher;
    v_participant participant;

    removed = v_writerInstanceInsert(instance, sample);
    if (removed != NULL) {
        c_free(removed);
    } else {
        writer->count++;
        v_checkMaxSamplesWarningLevel(v_objectKernel(writer), writer->count);
    }

    if (!instance->resend) {
        publisher = v_publisher(writer->publisher);
        if (!v_publisherIsSuspended(publisher)) {
            instance->resend = TRUE;
            found = c_tableInsert(writer->resendInstances, instance);
            assert(found == instance);
            /* notify participant */
            participant = v_publisherParticipant(publisher);
            v_participantResendManagerAddWriter(participant, writer);
        }
    }
}

/**
 * Implements blocking of writer on resource-limits. If w is a synchronous
 * writer, it will not block at all and just return V_WRITE_OUT_OF_RESOURCES
 * immediately (dds2810). Otherwise, if max-blocking time is not INFINITE
 * (!w->infWait) a wait will be done until until. In case max-blocking time is
 * INFINITE this method will wait indefinately.
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
    c_time until)
{
    c_time relTimeOut;
    c_ulong flags;
    v_writeResult result;

    if(v_writerIsSynchronous(w)){
        /* In case the writer is synchronous, there will be no blocking on
         * resource limits. In this case the write will immediately return
         * with OUT_OF_RESOURCES. See dds2810 for more details. */
        result = V_WRITE_OUT_OF_RESOURCES;
    } else {
        if (w->infWait == FALSE) {
            relTimeOut = c_timeSub(until, v_timeGet());
            if (c_timeCompare(relTimeOut,C_TIME_ZERO) == C_GT) {
                flags = v__observerTimedWait(v_observer(w), relTimeOut);
            } else {
                flags = V_EVENT_TIMEOUT;
            }
        } else {
            flags = v__observerWait(v_observer(w));
        }
        if (flags & V_EVENT_OBJECT_DESTROYED) {
            result = V_WRITE_PRE_NOT_MET;
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
groupInstanceWrite (
    v_cacheNode node,
    c_voidp arg)
{
    v_writeResult result;
    v_writerCacheItem item;
    struct groupWriteArg *a = (struct groupWriteArg *)arg;
    c_voidp instancePtr;

    item = v_writerCacheItem(node);
        if (item->instance) {
            instancePtr = &(item->instance);
        result = v_groupInstanceWrite(item->instance,instancePtr,v_message(a->message), &a->resendScope);
        if (result != V_WRITE_SUCCESS) {
            if ((result == V_WRITE_REJECTED) ||
                (a->result == V_WRITE_SUCCESS)) {
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
            if ((result == V_WRITE_REJECTED) ||
                (a->result == V_WRITE_SUCCESS)) {
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
    v_writer w = v_writer(i->writer);
    v_message message;
    struct groupWriteArg grouparg;

    message = v_writerInstanceCreateMessage(i);
    v_nodeState(message) = L_REGISTER;
    message->writeTime = v_timeGet();
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
    c_bool keep;
};

static c_bool
writeGroupInstance(
    v_cacheNode node,
    c_voidp arg)
{
    v_writerCacheItem item = v_writerCacheItem(node);
    struct writeGroupInstanceArg *a = (struct writeGroupInstanceArg *)arg;
    c_bool result;
    v_writeResult wr;
    v_groupInstance instance;
    v_resendScope resendScope = V_RESEND_NONE;

    instance = v_groupInstance(item->instance);
    if (instance) {
        if (v_groupInstanceOwner(instance) == a->group) {
            /* I will never get a rejected status, since datareaders don't
             * need to store the message.
             * I can get an INTRANSIT status, in case of an unreliable network.
             * Again we accept the fact that we will temporarily exceed resource
             * limits.
             */
            wr = v_groupWrite(instance->group,
                              a->message,
                              &instance,
                              V_NETWORKID_ANY,
                              &resendScope);
            assert(wr < V_WRITE_COUNT); /* TODO: remove variable or check it properly. */
            result = FALSE;
        } else {
            result = TRUE;
        }
    } else {
        result = TRUE;
    }
    return result;
}

static c_bool
disconnectInstance(
    c_object o,
    c_voidp arg)
{
  /**
   * Locally we can never get a reject of a reader based on max_instance
   * resource limits, because the writer determines the number of instances,
   * the reader can never fix the problem that it has reached maximum of the
   * resource limits.
   * In case of a reliable network we also will never receive an in-transit
   * state of a sample!
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
    v_writer w = v_writer(i->writer);
    v_message message;
    v_writerSample sample, found;
    struct writeGroupInstanceArg grouparg;
    c_time now = v_timeGet();

    if ((w->qos->lifecycle.autodispose_unregistered_instances == TRUE) &&
        (!v_stateTest(i->state, L_DISPOSED))) {
        /* Do not dispose instance more than once.
         * The instance may still be present in the writer history due to
         * reader rejects or networking intransit.
         */
        message = v_writerInstanceCreateMessage(i);
        v_nodeState(message) = L_DISPOSED;
        message->writeTime = now;
        message->writerGID = v_publicGid(v_public(w));
        message->sequenceNumber = w->sequenceNumber++;
        message->writerInstanceGID = v_publicGid(v_public(i));
        message->qos = c_keep(w->relQos);

        grouparg.message = message;
        grouparg.keep = FALSE;
        grouparg.group = proxy->group;
        v_writerCacheWalk(i->targetCache, writeGroupInstance, &grouparg);
        if (grouparg.keep) {
            sample = v_writerSampleNew(w, message);

            if (sample) {
                found = v_writerInstanceInsert(i, sample);
                v_writerSampleSetSentBefore(found, TRUE);
                c_free(found);
                c_free(sample);
            }
        }

        c_free(message);
    }

    if (!v_stateTest(i->state, L_UNREGISTER)) {
        /* Do not unregister instance more than once.
         * The instance may still be present in the writer history due to
         * reader rejects or networking intransit.
         */
        message = v_writerInstanceCreateMessage(i);
        v_nodeState(message) = L_UNREGISTER;
        message->writeTime = now;
        message->writerGID = v_publicGid(v_public(w));
        message->sequenceNumber = w->sequenceNumber++;
        message->writerInstanceGID = v_publicGid(v_public(i));
        message->qos = c_keep(w->relQos);

        grouparg.message = message;
        grouparg.keep = FALSE;
        grouparg.group = proxy->group;
        v_writerCacheWalk(i->targetCache, writeGroupInstance, &grouparg);
        if (grouparg.keep) {
            sample = v_writerSampleNew(w, message);

            if (sample) {
                found = v_writerInstanceInsert(i, sample);
                v_writerSampleSetSentBefore(found, TRUE);
                c_free(found);
                c_free(sample);
            }
        }

        c_free(message);
    }

    return TRUE;
}

static v_writeResult
writerResend(
    v_writerInstance instance,
    v_message message,
    c_bool implicit)
{
    v_writeResult result;
    struct groupWriteArg grouparg;
    v_writer writer = v_writer(instance->writer);
    v_writerSample sample;

    assert(writer != NULL);

    if (v_publisherIsSuspended(v_publisher(writer->publisher))) {
      /* The message is not written to the group(s), but instead kept in the
       * instance.
       * Keeping the message in the instance is done by pretending that at
       * least one reader has no resources left.
       * The writer will not resend any sample as long as the publisher is
       * suspended.
       */
        if (implicit) {
            v_stateSet(instance->state, L_SUSPENDED);
        }
        sample = v_writerSampleNew(writer,message);
        if (sample) {
            keepSample(writer, instance, sample);
            c_free(sample);
        }
        result = V_WRITE_SUCCESS;
    } else {
        grouparg.message = message;
        grouparg.instance = instance;
        grouparg.result = V_WRITE_SUCCESS;
        grouparg.resendScope = V_RESEND_NONE;
        grouparg.rejectScope = 0;

        if (implicit) {
            v_writerGroupSetWalk(&writer->groupSet,groupWrite,&grouparg);
        } else {
            v_writerCacheWalk(instance->targetCache,
                              groupInstanceWrite,&grouparg);
        }
        result = grouparg.result;
        if (result == V_WRITE_REJECTED) {
            sample = v_writerSampleNew(writer,message);
            if (sample) {
                v_writerSampleSetSentBefore(sample, TRUE);
                v_writerSampleResend(sample,grouparg.rejectScope);
                keepSample(writer,instance,sample);
                c_free(sample);
            }
        }
    }

    return result;
}

static v_writeResult
writerWrite(
    v_writerInstance instance,
    v_message message,
    c_bool implicit)
{
    v_writeResult result;
    struct groupWriteArg grouparg;
    v_writer writer = v_writer(instance->writer);
    v_writerSample sample;

    assert(writer != NULL);

    grouparg.message = message;
    grouparg.instance = instance;
    grouparg.result = V_WRITE_SUCCESS;
    grouparg.resendScope = V_RESEND_NONE;
    grouparg.rejectScope = 0;

    if (v_publisherIsSuspended(v_publisher(writer->publisher))) {
      /* The message is not written to the group(s), but instead kept in the
       * instance.
       * Keeping the message in the instance is done by pretending that at
       * least one reader has no resources left.
       * The writer will not resend any sample as long as the publisher is
       * suspended.
       */
        if (implicit) {
            v_stateSet(instance->state, L_SUSPENDED);
        }
        sample = v_writerSampleNew(writer,message);
        if (sample) {
            keepSample(writer, instance, sample);
            c_free(sample);
        }
        result = V_WRITE_SUCCESS;
    } else {
        if (v_writerInstanceTestState(instance, L_EMPTY)) {
            if (implicit) {
                v_writerGroupSetWalk(&writer->groupSet,groupWrite,&grouparg);
            } else {
                v_writerCacheWalk(instance->targetCache,
                                  groupInstanceWrite,&grouparg);
            }
            result = grouparg.result;
            if (result == V_WRITE_REJECTED) {
                sample = v_writerSampleNew(writer,message);

                if (sample) {
                    v_writerSampleSetSentBefore(sample, TRUE);
                    v_writerSampleResend(sample,grouparg.rejectScope);
                    keepSample(writer,instance,sample);
                    c_free(sample);
                }
            }
        } else {
            sample = v_writerSampleNew(writer,message);

            if (sample) {
                v_writerSampleResend(sample,grouparg.rejectScope);
                keepSample(writer,instance,sample);
                c_free(sample);
            }
            result = V_WRITE_REJECTED;
        }
    }

    return result;
}

static v_writeResult
instanceCheckResources(
    v_writerInstance _this,
    v_message message,
    c_time until)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writer writer;

    if (v_messageQos_isReliable(message->qos)) {
        writer = v_writerInstanceWriter(_this);
        if (writer->qos->history.kind == V_HISTORY_KEEPALL) {
            c_ulong blocked = 0;  /* Used for statistics */

            while ((_this->messageCount >= writer->depth) &&
                   (result == V_WRITE_SUCCESS)) {
                blocked++;
                if(blocked == 1){ /* We only count a blocked write once */
                    v_statisticsULongValueInc(v_writer,
                            numberOfWritesBlockedBySamplesPerInstanceLimit,
                            writer);
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
    c_long i, nrOfKeys;
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
    c_time *expiry = (c_time *)arg;
    v_writer writer = v_writer(instance->writer);
    v_writerSample sample, found;

    /* Walk from the oldest sample to the newest, so we can stop processing
     * all samples as soon as we find a sample with a write time newer than
     * the expiry time.
     */
    sample = v_writerInstanceTail(instance);
    while ((sample != NULL) &&
           (c_timeCompare(v_writerSampleMessage(sample)->writeTime, *expiry) != C_GT)) {
        found = v_writerInstanceRemove(instance, sample);
        assert(found == sample);
        c_free(found);
        writer->count--;
        sample = v_writerInstanceTail(instance);
    }
    return TRUE;
}

static void
autoPurgeSuspendedSamples(
    v_writer w)
{
    c_time expiry;

    assert(C_TYPECHECK(w,v_writer));

    if (v_publisherIsSuspended(v_publisher(w->publisher))) {
        if (c_timeCompare(w->qos->lifecycle.autopurge_suspended_samples_delay,
                          C_TIME_INFINITE) != C_EQ) {
            expiry = c_timeSub(v_timeGet(),
                               w->qos->lifecycle.autopurge_suspended_samples_delay);
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

    v_leaseRenew(w->livelinessLease, &(w->qos->liveliness.lease_duration));
    if (w->alive == FALSE) {
        kernel = v_objectKernel(w);
        w->alive = TRUE;
        if (kernel->builtin && kernel->builtin->kernelQos->builtin.enabled) {
            builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
            v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }
    }
}

static void
updateTransactionInformation(
     v_writer w,
     v_message message)
{
    /* If the operation is part of a coherent set of changes we need to keep
     * track of this message, in case it ends up being the last message sent
     * within the transaction.  In that case, it is cloned and resent in
     * v_writerCoherentEnd with the transaction information set
     */

    if (w->transactionId != 0) {
        if (w->transactionsLastMessage != NULL) {
            c_free (w->transactionsLastMessage);
        }
        w->transactionsLastMessage = c_keep (message);
    }
}

static v_writeResult
writerDispose(
    v_writer w,
    v_message message,
    c_time timestamp,
    v_writerInstance instance)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance found;
    v_writerQos qos;
    c_time until,now;
    c_bool implicit = FALSE;

    until = C_TIME_ZERO;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));
    assert(message != NULL);

    v_statisticsULongValueInc(v_writer, numberOfDisposes, w);

    /* only autpurge if publisher is suspended */
    autoPurgeSuspendedSamples(w);

    v_nodeState(message) = L_DISPOSED;

#ifdef _NAT_
    now = v_timeGet();
#else
    now = message->allocTime;
#endif

    if (c_timeIsInvalid(timestamp)) {
        timestamp = now;
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->writerInstanceGID = v_publicGid(NULL);
    message->transactionId = w->transactionId;
    if (w->transactionId != 0) {
        w->transactionCount++;
    }

    qos = w->qos;
    if (!w->infWait) {
        until = c_timeAdd(now, qos->reliability.max_blocking_time);
    }

    while ((qos->resource.max_samples != V_LENGTH_UNLIMITED) &&
           (w->count >= qos->resource.max_samples)) {
        result = doWait(w,until);
        if (result != V_WRITE_SUCCESS) {
            if(result == V_WRITE_TIMEOUT || result == V_WRITE_OUT_OF_RESOURCES) {
                /* Both results are a case of (immediate) timeout, so are counted
                 * in this statistic. */
                v_statisticsULongValueInc(v_writer, numberOfTimedOutWrites, w);
            }
            return result;
        }
    }
    message->qos = c_keep(w->relQos);

    if (instance == NULL) {
        instance = v_writerInstanceNew(w,message);
        assert(c_refCount(instance) == 1);
        found = c_insert(w->instances,instance);
        if (found != instance) {
            result = instanceCheckResources(found,message,until);
        } else {
            assert(c_refCount(instance) == 2);
            if ((qos->resource.max_instances != V_LENGTH_UNLIMITED) &&
                   (c_count(w->instances) > qos->resource.max_instances) &&
                   (result == V_WRITE_SUCCESS)) {
                result = v_writerIsSynchronous(w) ? V_WRITE_OUT_OF_RESOURCES : V_WRITE_TIMEOUT;
            }
            if (result == V_WRITE_SUCCESS) {
                v_publicInit(v_public(instance));
                assert(c_refCount(instance) == 3);
                implicit = TRUE;
                v_statisticsULongValueInc(v_writer, numberOfImplicitRegisters, w);
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
        v_writerInstanceFree(instance);
        instance = found;
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                result = instanceCheckResources(instance,message,until);
            } else {
                OS_REPORT_1(OS_API_INFO,
                            "writerDispose", 0,
                            "specified instance does not belong to writer %s",
                            v_entityName2(w));
                result = V_WRITE_PRE_NOT_MET;
            }
        } else {
            OS_REPORT_1(OS_API_INFO,
                        "writerDispose", 0,
                        "specified instance key value does not match "
                        "data key value for writer %s",
                        v_entityName2(w));
            result = V_WRITE_PRE_NOT_MET;
        }
    }

    if (result == V_WRITE_SUCCESS) {
        v_state oldState = instance->state;
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;
        deadlineUpdate(w, instance, now);
        v_stateSet(instance->state, L_DISPOSED);

        /* Now that all attributes of the message are populated, store the
         * message if required for operating as part of a transaction */
        updateTransactionInformation(w, message);

        result = writerWrite(instance,message,implicit);
        UPDATE_WRITER_STATISTICS(w, instance, oldState);
    } else if(result == V_WRITE_TIMEOUT){
        v_statisticsULongValueInc(v_writer, numberOfTimedOutWrites, w);
    }

    return result;
}

static v_writeResult
writerUnregister(
    v_writer w,
    v_message message,
    c_time timestamp,
    v_writerInstance instance)
{
    v_writeResult result;
    v_writerInstance found;
    v_writerSample sample;
    v_writerQos qos;
    v_message dispose;
    c_time until,now;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));
    assert(message != NULL);

    until = C_TIME_ZERO;
    result = V_WRITE_SUCCESS;

    v_statisticsULongValueInc(v_writer, numberOfUnregisters, w);
    /* statistics update */
    /* only autpurge if publisher is suspended */
    autoPurgeSuspendedSamples(w);

    v_nodeState(message) = L_UNREGISTER;

#ifdef _NAT_
    now = v_timeGet();
#else
    now = message->allocTime;
#endif

    if (c_timeIsInvalid(timestamp)) {
        timestamp = now;
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));

    qos = w->qos;
    if (!w->infWait) {
        until = c_timeAdd(now, qos->reliability.max_blocking_time);
    }
    while ((qos->resource.max_samples != V_LENGTH_UNLIMITED) &&
           (w->count >= qos->resource.max_samples)) {
        result = doWait(w,until);
        if (result != V_WRITE_SUCCESS) {
            return result;
        }
    }
    message->qos = c_keep(w->relQos);

    if (instance == NULL) {
        instance = v_writerInstanceNew(w,message);
        found = c_tableInsert(w->instances,instance);
        if (found != instance) {
            v_writerInstanceFree(instance);
            instance = found;
            v_deadLineInstanceListRemoveInstance(w->deadlineList,
                                                 v_instance(instance));
            result = instanceCheckResources(instance,message,until);
        } else {
            found = c_remove(w->instances,instance,NULL,NULL);
            assert(found == instance);
            v_writerInstanceFree(found);
            v_writerInstanceFree(instance);
            result = V_WRITE_PRE_NOT_MET;
        }
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                v_deadLineInstanceListRemoveInstance(w->deadlineList,
                                                     v_instance(instance));
                result = instanceCheckResources(instance,message,until);
            } else {
                OS_REPORT_1(OS_API_INFO,
                            "v_writer::writerUnregister", 0,
                            "specified instance does not belong to writer %s",
                            v_entityName2(w));
                result = V_WRITE_PRE_NOT_MET;
            }
        } else {
            OS_REPORT_1(OS_API_INFO,
                        "v_writer::writerUnregister", 0,
                        "specified instance key value does not match "
                        "data key value for writer %s",
                        v_entityName2(w));
            result = V_WRITE_PRE_NOT_MET;
        }
    }

    /* In case of lifecycle.autodispose_unregistered_instances ideally
     * one combined message for unregister and dispose should be sent.
     * But because the current readers cannot handle combined messages
     * for now separate messages are sent. also see scdds63.
     */
    if (result == V_WRITE_SUCCESS) {
        if( (!v_stateTest(instance->state, L_DISPOSED)) &&
            (w->qos->lifecycle.autodispose_unregistered_instances))
        {
            dispose = v_writerInstanceCreateMessage(instance);
            result  = writerDispose(w, dispose, timestamp, instance);
            c_free(dispose);
        }
    }
    if (result == V_WRITE_SUCCESS) {
        v_state oldState = instance->state;
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;
        if (v_writerInstanceTestState(instance, L_EMPTY)) {
            result = writerWrite(instance,message,FALSE);
            if (v_writerInstanceTestState(instance, L_EMPTY)) {
                found = c_remove(w->instances,instance,NULL,NULL);
                /* Instance is removed from writer, so also subtract related
                 * statistics. */
                assert(found == instance);
                UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(w, instance);
                v_writerCacheDeinit(instance->targetCache);
                v_publicFree(v_public(instance));
                v_writerInstanceFree(found);
            } else {
                v_writerInstanceUnregister(instance);
                UPDATE_WRITER_STATISTICS(w, instance, oldState);
            }
        } else {
            sample = v_writerSampleNew(w,message);
            if (sample) {
                v_writerSampleResend(sample,V_RESEND_ALL);
                keepSample(w,instance,sample);
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

    if (w->qos->liveliness.kind == V_LIVELINESS_PARTICIPANT) {
        v_observerLock(v_observer(w));
        kernel = v_objectKernel(w);
        if (w->alive == FALSE) {
            w->alive = TRUE;
            if (kernel->builtin && kernel->builtin->kernelQos->builtin.enabled) {
                writeBuiltinSample = TRUE;
            }
        }
        v_observerUnlock(v_observer(w));

        v_leaseRenew(w->livelinessLease, &(w->qos->liveliness.lease_duration));
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
    c_long length,sres;

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
    assert(sres == (length-1));
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
    c_long length,sres;

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
    assert(sres == (length-1));
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
    c_long i,nrOfKeys,totalSize;
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

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_writer
v_writerNew(
    v_publisher p,
    const c_char *name,
    v_topic topic,
    v_writerQos qos,
    c_bool enable)
{
    v_kernel kernel;
    v_writer w;
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
       v_topicAccessMode(topic) == V_ACCESS_MODE_READ_WRITE)
    {
        q = v_writerQosNew(kernel,qos);
        if (q != NULL) {
            w = v_writer(v_objectNew(kernel,K_WRITER));
            v_writerInit(w, p, name, topic, q, enable);
            c_free(q); /* ref now in w->qos */
        } else {
            OS_REPORT(OS_ERROR, "v_writerNew", 0,
                      "Writer not created: inconsistent qos");
            w = NULL;
        }
    } else
    {
        OS_REPORT_1(OS_ERROR, "v_writerNew", 0,
                    "Creation of writer <%s> failed. Topic "
                    "does not have write access rights.", name);
        w = NULL;
    }

    return w;
}

void
v_writerInit(
    v_writer writer,
    v_publisher p,
    const c_char *name,
    v_topic topic,
    v_writerQos qos,
    c_bool enable)
{
    v_kernel kernel;
    v_participant participant;
    c_type instanceType,sampleType;
    c_char *keyExpr;
    v_writerStatistics wStat;
    c_time deadline;
    c_time unregister;

    assert(writer != NULL);
    assert(p != NULL);
    assert(C_TYPECHECK(writer, v_writer));
    assert(C_TYPECHECK(p,v_publisher));
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(writer);
    if (v_isEnabledStatistics(kernel, V_STATCAT_WRITER)) {
        wStat = v_writerStatisticsNew(kernel);
    } else {
        wStat = NULL;
    }
    v_observerInit(v_observer(writer),name, v_statistics(wStat),enable);

    writer->count               = 0;
    writer->alive               = TRUE;
    writer->depth               = 0x7fffffff; /* MAX_INT */
    writer->topic               = c_keep(topic);
    writer->qos                 = c_keep(qos);
    writer->pubQos              = c_keep(v_publisherGetQosRef(p));
    writer->msgQos              = NULL;
    writer->relQos              = NULL;
    writer->msgQosType          = NULL;
    writer->publisher           = p;
    writer->deadlineList        = NULL;
    writer->sequenceNumber      = 0;
    writer->cachedInstance      = NULL;
    writer->livelinessLease     = NULL;
    writer->transactionId       = 0;

    v_writerGroupSetInit(&writer->groupSet);
    instanceType = createWriterInstanceType(topic);
    keyExpr = createInstanceKeyExpr(topic);
    writer->instances = c_tableNew(instanceType, keyExpr);
    writer->resendInstances = c_tableNew(instanceType, keyExpr);
    os_free(keyExpr);
    sampleType = createWriterSampleType(topic);
    writer->messageField = c_metaResolveProperty(sampleType,"message");
    writer->instanceType = c_keep (instanceType);
    writer->sampleType   = c_keep (sampleType);
    c_free(instanceType);
    c_free(sampleType);
    writer->transactionsLastMessage = NULL;

    participant = v_participant(p->participant);
    assert(participant != NULL);

    deadline = qos->deadline.period;
    unregister = qos->lifecycle.autounregister_instance_delay;

    /* The auto unregister instance delay and deadline missed processing
     * is managed by means of one attribute: deadlineCountLimit.
     * The deadlineCountLimit is the number of successive deadline missed
     * before the auto unregister delay expires.
     * Special values are -1 (no auto unregister delay specified) and
     * 1 (no deadline specified).
     */
    if ((c_timeCompare(unregister, C_TIME_INFINITE) == C_EQ))
    {
        writer->deadlineCountLimit = -1;
    } else {
        if ((c_timeCompare(deadline, C_TIME_INFINITE) == C_EQ) ||
            (c_timeCompare(deadline, C_TIME_ZERO) == C_EQ))
        {
            writer->deadlineCountLimit = 1; /* no deadline */
        } else {
            writer->deadlineCountLimit = (int)(c_timeToReal(unregister) /
                                               c_timeToReal(deadline));
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
    v_publisherAddWriter(p,writer);

    if (enable) {
        v_writerEnable(writer);
    }
}

v_result
v_writerEnable(
    v_writer writer)
{
    v_kernel kernel;
    v_message builtinMsg;
    v_participant participant;
    v_writerQos qos;
    v_result result = V_RESULT_ILL_PARAM;

    if (writer) {
        v_observerLock(v_observer(writer));
        result = V_RESULT_OK;
        qos = writer->qos;
        if (qos->history.kind == V_HISTORY_KEEPLAST) {
            if (qos->history.depth >= 0) {
                writer->depth = qos->history.depth;
            }
        } else {
            if (qos->resource.max_samples_per_instance >= 0) {
                writer->depth = qos->resource.max_samples_per_instance;
            }
        }
        writer->infWait = (c_timeCompare(qos->reliability.max_blocking_time,
                                         C_TIME_INFINITE) == C_EQ);

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
            if (qos->liveliness.kind == V_LIVELINESS_PARTICIPANT) {
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
        if (qos->liveliness.kind != V_LIVELINESS_AUTOMATIC) {
            if (c_timeCompare(qos->liveliness.lease_duration,
                              C_TIME_INFINITE) != C_EQ) {
                writer->livelinessLease = v_leaseNew(kernel, qos->liveliness.lease_duration);
                if(writer->livelinessLease)
                {
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
                        OS_REPORT_2(OS_ERROR, "v_writer", 0,
                            "A fatal error was detected when trying to register writer's %p liveliness lease "
                            "to the liveliness lease manager of the kernel. The result code was %d.", writer, result);
                    }
                }
            } /* else liveliness also determined by liveliness of node */
        }

        if (qos->reliability.synchronous) {
            writer->deliveryGuard = v_deliveryGuardNew(kernel->deliveryService,writer);
        } else {
            writer->deliveryGuard = NULL;
        }

        initMsgQos(writer);
        if (kernel->builtin && kernel->builtin->kernelQos->builtin.enabled) {
            builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin, writer);
            v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }
        v_observerUnlock(v_observer(writer));
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

    c_tableWalk(w->instances, disconnectInstance, g);
    v_writerCacheDeinit(g->targetCache);
    c_tableWalk(w->instances, connectInstance, g);
    return TRUE;
}

static c_bool
instanceFree(
    c_object o,
    c_voidp writer)
{
    v_writerInstance instance = v_writerInstance(o);

    UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(writer, instance);
    v_writerCacheDeinit(instance->targetCache);
    v_publicFree(o);

    return TRUE;
}

void
v_writerFree(
    v_writer w)
{
    v_kernel kernel;
    v_publisher p;
    v_message builtinMsg;
    v_message unregisterMsg;

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
    kernel = v_objectKernel(w);

    /* First create message, only at the end dispose. Applications expect
       the disposed sample to be the last!
       In the free algorithm the writer is unpublished from the partitions,
       which also involves the production of the builtin topic.
    */
    builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
    unregisterMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);

    if (w->deliveryGuard) {
        v_deliveryGuardFree(w->deliveryGuard);
        w->deliveryGuard = NULL;
    }
    v_deadLineInstanceListFree(w->deadlineList);

    v_leaseManagerDeregister(kernel->livelinessLM, w->livelinessLease);

    v_writerGroupSetWalk(&w->groupSet,removeFromGroup,w);

    (void) c_tableWalk(w->instances,instanceFree,w); /* Always returns TRUE. */

    v_observerUnlock(v_observer(w));

    if (kernel->qos->builtin.enabled || (c_tableCount(w->instances) > 0) ) {
        v_writeDisposeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
        v_unregisterBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, unregisterMsg);
    }
    c_free(builtinMsg);
    c_free(unregisterMsg);

    w->publisher = NULL;
    v_observerFree(v_observer(w));
}

void
v_writerDeinit(
    v_writer w)
{
    if (w == NULL) {
        return;
    }
    assert(C_TYPECHECK(w,v_writer));
    v_observerDeinit(v_observer(w));
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

c_bool
v_writerUnPublishGroup(
    v_writer writer,
    v_group group)
{
    v_writerGroup proxy;

    assert(C_TYPECHECK(writer, v_writer));
    assert(group != NULL);
    assert(C_TYPECHECK(group, v_group));

    v_observerLock(v_observer(writer));

    proxy = v_writerGroupSetRemove(&writer->groupSet, group);
    assert(proxy != NULL);
    c_tableWalk(writer->instances, disconnectInstance, proxy);
    v_writerCacheDeinit(proxy->targetCache);
    c_free(proxy);

    v_observerUnlock(v_observer(writer));

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
    c_bool changed;
    C_STRUCT(v_event) e;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));

    changed = v_statusNotifyIncompatibleQos(v_entity(w)->status, id);
    if (changed) {
        e.kind = V_EVENT_INCOMPATIBLE_QOS;
        e.source = v_publicHandle(v_public(w));
        e.userData = NULL;
        v_observerNotify(v_observer(w), &e, NULL);
        v_observerUnlock(v_observer(w));
        v_observableNotify(v_observable(w), &e);
    } else {
        v_observerUnlock(v_observer(w));
    }

}


void
v_writerNotifyPublicationMatched (
    v_writer w,
    v_gid    readerGID,
    c_bool   dispose)
{
    c_bool changed;
    C_STRUCT(v_event) e;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));

    changed = v_statusNotifyPublicationMatched(v_entity(w)->status, readerGID, dispose);
    if (changed) {
        e.kind = V_EVENT_TOPIC_MATCHED;
        e.source = v_publicHandle(v_public(w));
        e.userData = NULL;
        v_observerNotify(v_observer(w), &e, NULL);
        v_observerUnlock(v_observer(w));
        v_observableNotify(v_observable(w), &e);
    } else {
        v_observerUnlock(v_observer(w));
    }

}

void
v_writerNotifyChangedQos(
    v_writer w,
    v_writerNotifyChangedQosArg *arg)
{
    v_kernel kernel;
    v_message builtinMsg;

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
    if (kernel->builtin && kernel->builtin->kernelQos->builtin.enabled) {
        builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
        v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }
    v_observerUnlock(v_observer(w));
}


v_result
v_writerSetQos(
    v_writer w,
    v_writerQos qos)
{
    v_result result;
    v_qosChangeMask cm;
    v_message builtinMsg;
    v_kernel kernel;

    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));
    kernel = v_objectKernel(w);
    result = v_writerQosSet(w->qos, qos, v_entity(w)->enabled, &cm);
    if ((result == V_RESULT_OK) && (cm != 0)) {
        initMsgQos(w);
        if (cm & V_POLICY_BIT_DEADLINE) {
            v_deadLineInstanceListSetDuration(w->deadlineList, w->qos->deadline.period);
        }
        v_writerGroupSetWalk(&w->groupSet, reconnectToGroup, (c_voidp)w);
        if (kernel->builtin && kernel->builtin->kernelQos->builtin.enabled) {
            builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
            v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
            c_free(builtinMsg);
        }
        v_observerUnlock(v_observer(w));
    } else {
        v_observerUnlock(v_observer(w));
    }

    return result;
}

void
v_writerNotifyLivelinessLost(
    v_writer w)
{
    c_bool changed;
    C_STRUCT(v_event) e;
    v_kernel kernel;
    v_message builtinMsg;
    v_duration duration = C_TIME_INFINITE;

    assert(C_TYPECHECK(w,v_writer));

    v_observerLock(v_observer(w));
    changed = v_statusNotifyLivelinessLost(v_entity(w)->status);
    if (changed) { /* first liveliness lost event */
        e.kind = V_EVENT_LIVELINESS_LOST;
        e.source = v_publicHandle(v_public(w));
        e.userData = NULL;
        v_observerNotify(v_observer(w), &e, NULL);
        v_observableNotify(v_observable(w), &e);
    }
    w->alive = FALSE;
    kernel = v_objectKernel(w);

    v_observerUnlock(v_observer(w));
    /* suspend liveliness check */
    v_leaseRenew(w->livelinessLease, &(duration));

    if (kernel->builtin && kernel->builtin->kernelQos->builtin.enabled) {
        builtinMsg = v_builtinCreatePublicationInfo(kernel->builtin,w);
        v_writeBuiltinTopic(kernel, V_PUBLICATIONINFO_ID, builtinMsg);
        c_free(builtinMsg);
    }
}

/**************************************************************
 * Public functions
 **************************************************************/

v_writeResult
v_writerRegister(
    v_writer w,
    v_message message,
    c_time timestamp,
    v_writerInstance *inst)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance instance, found;
    v_writerQos qos;
    c_time until,now;

    assert(C_TYPECHECK(w,v_writer));
    assert(message != NULL);

    *inst = NULL;

    v_observerLock(v_observer(w));

    v_statisticsULongValueInc(v_writer, numberOfRegisters, w);
    /* only autpurge if publisher is suspended */
    autoPurgeSuspendedSamples(w);

    v_nodeState(message) = L_REGISTER;

#ifdef _NAT_
        now = v_timeGet();
#else
        now = message->allocTime;
#endif

    if (c_timeIsInvalid(timestamp)) {
        timestamp = now;
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->writerInstanceGID = v_publicGid(NULL);
    message->transactionId = w->transactionId;
    qos = w->qos;
    if (!w->infWait) {
        until = c_timeAdd(now, qos->reliability.max_blocking_time);
    }
    while ((qos->resource.max_samples != V_LENGTH_UNLIMITED) &&
           (w->count >= qos->resource.max_samples)) {
        result = doWait(w,until);
        if (result != V_WRITE_SUCCESS) {
            v_observerUnlock(v_observer(w));
            return result;
        }
    }
    message->qos = c_keep(w->relQos);

    instance = v_writerInstanceNew(w,message);
    assert(c_refCount(instance) == 1);
    found = c_tableInsert(w->instances,instance);
    if (found != instance) {
        v_state oldState = found->state;
        /* The existing instance may be unregistered, so clear the flag */
        v_stateClear(found->state, L_UNREGISTER);
        UPDATE_WRITER_STATISTICS(w, found, oldState);

        assert(c_refCount(instance) == 1);
        result = V_WRITE_SUCCESS;
    } else {
        assert(c_refCount(instance) == 2);
        if (!w->infWait) {
            until = c_timeAdd(message->writeTime,
                              w->qos->reliability.max_blocking_time);
        }
        if ((w->qos->resource.max_instances != V_LENGTH_UNLIMITED) &&
               (c_tableCount(w->instances) > w->qos->resource.max_instances) &&
               (result == V_WRITE_SUCCESS)) {
            result = v_writerIsSynchronous(w) ? V_WRITE_OUT_OF_RESOURCES : V_WRITE_TIMEOUT;
        }
        if (result == V_WRITE_SUCCESS) {
            v_publicInit(v_public(instance));
            deadlineUpdate(w, instance, now);
            message->writerInstanceGID = v_publicGid(v_public(instance));
            message->sequenceNumber = w->sequenceNumber++;
            result = writerWrite(instance,message,TRUE);
            /* The writer statistics are updated for the newly inserted
            * instance (with its initial values). The previous state
            * was nothing, so 0 is passed as the oldState. */
            UPDATE_WRITER_STATISTICS(w, instance, 0);
        } else {
            found = c_remove(w->instances, instance, NULL, NULL);
            assert(found == instance);
            c_free(found);
        }
    }
    v_writerInstanceFree(instance);
    if (((result == V_WRITE_SUCCESS) || (result == V_WRITE_REJECTED)) &&
        (inst != NULL)) {
        *inst = c_keep(found);
    }
    v_observerUnlock(v_observer(w));

    /* Rewrite internal return code which is for use only in the kernel */
    if ( result == V_WRITE_REJECTED ) {
       result = V_WRITE_SUCCESS;
    }
    return result;
}

v_writerInstance
v_writerLookupInstance(
    v_writer w,
    v_message keyTemplate)
{
    v_writerInstance instance, found;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(keyTemplate,v_message));

    v_observerLock(v_observer(w));

    instance = v_writerInstanceNew(w, keyTemplate);
    found = c_find(w->instances, instance);
    c_free(instance);

    v_observerUnlock(v_observer(w));

    return found;
}

v_writeResult
v_writerUnregister(
    v_writer w,
    v_message message,
    c_time timestamp,
    v_writerInstance instance)
{
    v_writeResult result;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));
    assert(message != NULL);

    v_observerLock(v_observer(w));
    if (w->transactionId == 0) {
        result = writerUnregister(w, message, timestamp, instance);
    } else {
        result = V_WRITE_PRE_NOT_MET;
    }
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
    c_time timestamp,
    v_writerInstance instance)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance found;
    v_writerQos qos;
    c_time until,now;
    c_bool implicit = FALSE;
    c_ulong blocked; /* Used for statistics */
    enum v_livelinessKind livKind;
    C_STRUCT(v_event) event;
    v_deliveryWaitList waitlist;

    until = C_TIME_ZERO;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));
    assert(message != NULL);

    V_MESSAGE_STAMP(message,writerCopyTime);

    v_observerLock(v_observer(w));

    v_statisticsULongValueInc(v_writer, numberOfWrites, w);
    /* only autopurge if publisher is suspended */
    autoPurgeSuspendedSamples(w);

    v_nodeState(message) = L_WRITE;

#ifdef _NAT_
        now = v_timeGet();
#else
        now = message->allocTime;
#endif

    if (c_timeIsInvalid(timestamp)) {
        timestamp = now;
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->writerInstanceGID = v_publicGid(NULL);
    message->transactionId = w->transactionId;
    if (w->transactionId != 0) {
        w->transactionCount++;
    }

    qos = w->qos;
    if (!w->infWait) {
        until = c_timeAdd(now, qos->reliability.max_blocking_time);
    }
    blocked = 0;
    while ((qos->resource.max_samples != V_LENGTH_UNLIMITED) &&
           (w->count >= qos->resource.max_samples)) {
        blocked++;
        if(blocked == 1){ /* We only count a blocked write once */
            v_statisticsULongValueInc(v_writer, numberOfWritesBlockedBySamplesLimit, w);
        }
        result = doWait(w,until);
        if (result != V_WRITE_SUCCESS) {
            if(result == V_WRITE_TIMEOUT || result == V_WRITE_OUT_OF_RESOURCES) {
                /* Both results are a case of (immediate) timeout, so are counted
                 * in this statistic. */
                v_statisticsULongValueInc(v_writer, numberOfTimedOutWrites, w);
            }
            v_observerUnlock(v_observer(w));
            return result;
        }
    }
    message->qos = c_keep(w->msgQos);

    if (instance == NULL) {
        instance = v_writerInstanceNew(w,message);
        assert(c_refCount(instance) == 1);

        found = c_tableInsert(w->instances,instance);
        if (found != instance) {
            result = instanceCheckResources(found,message,until);
        } else {
            assert(c_refCount(instance) == 2);
            if ((qos->resource.max_instances != V_LENGTH_UNLIMITED) &&
                (c_tableCount(w->instances) > qos->resource.max_instances) &&
                (result == V_WRITE_SUCCESS)) {
                result = v_writerIsSynchronous(w) ? V_WRITE_OUT_OF_RESOURCES : V_WRITE_TIMEOUT;
            }
            if (result == V_WRITE_SUCCESS) {
                v_publicInit(v_public(instance));
                assert(c_refCount(instance) == 3);
                implicit = TRUE;
                v_statisticsULongValueInc(v_writer, numberOfImplicitRegisters, w);
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
        v_writerInstanceFree(instance);
        instance = found;
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                result = instanceCheckResources(instance,message,until);
            } else {
                OS_REPORT_1(OS_API_INFO,
                            "v_writerWrite", 0,
                            "specified instance does not belong to writer %s",
                            v_entityName2(w));
                result = V_WRITE_PRE_NOT_MET;
            }
        } else {
            OS_REPORT_1(OS_API_INFO,
                        "v_writerWrite", 0,
                        "specified instance key value does not match "
                        "data key value for writer %s",
                        v_entityName2(w));
            result = V_WRITE_PRE_NOT_MET;
        }
    }

    V_MESSAGE_STAMP(message,writerLookupTime);

    waitlist = NULL;

    if (result == V_WRITE_SUCCESS) {
        v_state oldState = instance->state;
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;
        if (v_writerIsSynchronous(w)) {
            /* Collect all currently known connected synchronous DataReaders */
            waitlist = v_deliveryWaitListNew(w->deliveryGuard,message);
            if (waitlist) {
                v_stateSet(v_nodeState(message),L_SYNCHRONOUS);
            }
        }

        /* Now that all attributes of the message are populated, store the
         * message if required for operating as part of a transaction */
        updateTransactionInformation(w, message);

        result = writerWrite(instance,message,implicit);
#if 0
        if (result == V_WRITE_SUCCESS) {
            /* Successful delivered to all so no need to wait. */
            v_deliveryWaitListFree(waitlist);
            waitlist = NULL;
        }
#endif
        v_stateClear(instance->state, L_DISPOSED);
        deadlineUpdate(w, instance, now);
        UPDATE_WRITER_STATISTICS(w, instance, oldState);
    } else if (result == V_WRITE_TIMEOUT){
        v_statisticsULongValueInc(v_writer, numberOfTimedOutWrites, w);
    }

    livKind = qos->liveliness.kind;
    assertLiveliness(w);

    v_observerUnlock(v_observer(w));

    if (livKind == V_LIVELINESS_PARTICIPANT) {
        event.kind = V_EVENT_LIVELINESS_ASSERT;
        event.source = v_publicHandle(v_public(w));
        event.userData = NULL;
        v_observableNotify(v_observable(w), &event);
    }

    if (waitlist) {
        /* The existance of a waitlist implies the writer is synchronous. */
        /* Apparently not all synchronous DataReaders have acknoledged delivery
         * so now call wait on the waitlist.
         */
        v_writeResult r = v_deliveryWaitListWait(waitlist,w->qos->reliability.max_blocking_time);
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
    c_time timestamp,
    v_writerInstance instance)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_deliveryWaitList waitlist;

    assert(message != NULL);
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));
    assert(C_TYPECHECK(message,v_message));

    waitlist = NULL;
    v_observerLock(v_observer(_this));

    if (v_writerIsSynchronous(_this)) {
        waitlist = v_deliveryWaitListNew(_this->deliveryGuard,message);
    }
    result = writerDispose(_this,message,timestamp,instance);
    if (result == V_WRITE_SUCCESS) {
        v_deliveryWaitListFree(waitlist);
        waitlist = NULL;
    }
    v_observerUnlock(v_observer(_this));

    if (waitlist) {
        /* This implies the writer is synchronous. */
        v_writeResult r = v_deliveryWaitListWait(waitlist,_this->qos->reliability.max_blocking_time);
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
    c_time timestamp,
    v_writerInstance instance)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_writerInstance found;
    v_writerQos qos;
    c_time until,now;
    c_bool implicit = FALSE;
    enum v_livelinessKind livKind;
    C_STRUCT(v_event) event;
    v_deliveryWaitList waitlist;

    assert(C_TYPECHECK(w,v_writer));
    assert(C_TYPECHECK(message,v_message));
    assert(message != NULL);

    until = C_TIME_ZERO;

    v_observerLock(v_observer(w));
    v_statisticsULongValueInc(v_writer, numberOfWrites, w);
    v_statisticsULongValueInc(v_writer, numberOfDisposes, w);

    /* only autpurge if publisher is suspended */
    autoPurgeSuspendedSamples(w);

    v_nodeState(message) = L_WRITE | L_DISPOSED;

#ifdef _NAT_
        now = v_timeGet();
#else
        now = message->allocTime;
#endif

    if (c_timeIsInvalid(timestamp)) {
        timestamp = now;
    }

    message->writeTime = timestamp;
    message->writerGID = v_publicGid(v_public(w));
    message->writerInstanceGID = v_publicGid(NULL);
    message->transactionId = w->transactionId;
    if (w->transactionId != 0) {
        w->transactionCount++;
    }

    qos = w->qos;
    if (!w->infWait) {
        until = c_timeAdd(now, qos->reliability.max_blocking_time);
    }
    while ((qos->resource.max_samples != V_LENGTH_UNLIMITED) &&
           (w->count >= qos->resource.max_samples)) {
        result = doWait(w,until);
        if (result != V_WRITE_SUCCESS) {
            v_observerUnlock(v_observer(w));
            return result;
        }
    }
    message->qos = c_keep(w->relQos);

    if (instance == NULL) {
        instance = v_writerInstanceNew(w,message);
        found = c_tableInsert(w->instances,instance);
        if (found != instance) {
            /* Noop */
        } else {
            if ((qos->resource.max_instances != V_LENGTH_UNLIMITED) &&
                   (c_tableCount(w->instances) > qos->resource.max_instances) &&
                   (result == V_WRITE_SUCCESS)) {
                result = v_writerIsSynchronous(w) ? V_WRITE_OUT_OF_RESOURCES : V_WRITE_TIMEOUT;
            }
            if (result == V_WRITE_SUCCESS) {
                v_publicInit(v_public(instance));
                assert(c_refCount(instance) == 3);
                implicit = TRUE;
                v_statisticsULongValueInc(v_writer, numberOfImplicitRegisters, w);
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
        v_writerInstanceFree(instance);
        instance = found;
    } else {
        if (v_writerInstanceWriter(instance) == w) {
            if (compareKeyValue(instance,message) == C_EQ) {
                result = instanceCheckResources(instance,message,until);
            } else {
                OS_REPORT_1(OS_API_INFO,
                            "v_writerWriteDispose", 0,
                            "specified instance does not belong to writer %s",
                            v_entityName2(w));
                result = V_WRITE_PRE_NOT_MET;
            }
        } else {
            OS_REPORT_1(OS_API_INFO,
                        "v_writerWriteDispose", 0,
                        "specified instance key value does not match "
                        "data key value for writer %s",
                        v_entityName2(w));
            result = V_WRITE_PRE_NOT_MET;
        }
    }

    waitlist = NULL;

    if (result == V_WRITE_SUCCESS) {
        v_state oldState = instance->state;
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->sequenceNumber = w->sequenceNumber++;
        deadlineUpdate(w, instance, now);
        v_stateSet(instance->state, L_DISPOSED);
        if (v_writerIsSynchronous(w)) {
            waitlist = v_deliveryWaitListNew(w->deliveryGuard,message);
        }

        /* Now that all attributes of the message are populated, store the
         * message if required for operating as part of a transaction */
        updateTransactionInformation(w, message);

        result = writerWrite(instance,message,implicit);
        if (result == V_WRITE_SUCCESS) {
            v_deliveryWaitListFree(waitlist);
            waitlist = NULL;
        }
        UPDATE_WRITER_STATISTICS(w, instance, oldState);
    }

    livKind = qos->liveliness.kind;
    assertLiveliness(w);

    v_observerUnlock(v_observer(w));

    if (livKind == V_LIVELINESS_PARTICIPANT) {
        event.kind = V_EVENT_LIVELINESS_ASSERT;
        event.source = v_publicHandle(v_public(w));
        event.userData = NULL;
        v_observableNotify(v_observable(w), &event);
    }

    if (waitlist) {
        /* This implies the writer is synchronous. */
        v_writeResult r = v_deliveryWaitListWait(waitlist,w->qos->reliability.max_blocking_time);
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

v_result
v_writerWaitForAcknowledgments(
        v_writer w,
        v_duration timeout)
{
    v_result result;
    c_time curTime, endTime, waitTime;
    c_ulong flags;

    assert(C_TYPECHECK(w,v_writer));

    if(w){
        v_observerLock(v_observer(w));

        if(c_tableCount(w->resendInstances) > 0){
            if (c_timeIsInfinite(timeout)) {
                flags = v__observerWait(v_observer(w));
                if(c_tableCount(w->resendInstances) == 0){
                    result = V_RESULT_OK;
                } else {
                    result = V_RESULT_ILL_PARAM;
                }
            } else {
                curTime = v_timeGet();
                endTime = c_timeAdd(curTime, timeout);
                waitTime = timeout;
                result = V_RESULT_TIMEOUT;

                do{
                    flags = v__observerTimedWait(v_observer(w), waitTime);

                    if(c_tableCount(w->resendInstances) == 0){
                        result = V_RESULT_OK;
                    } else if (flags & V_EVENT_OBJECT_DESTROYED) {
                        result = V_RESULT_ILL_PARAM;
                        curTime = endTime;
                    } else if (flags & V_EVENT_TIMEOUT) {
                        curTime = endTime;
                    } else {
                        curTime = v_timeGet();
                        waitTime = c_timeSub(endTime, curTime);
                    }
                } while( (c_tableCount(w->resendInstances) > 0) &&
                         (c_timeCompare(curTime, endTime) == C_LT));
            }
        } else {
            result = V_RESULT_OK;
        }
        v_observerUnlock(v_observer(w));
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}


typedef struct collectGarbageArg {
    c_iter emptyList;
} *collectGarbageArg;


/* Used to resend messages by the resendmanager */
static c_bool
resendInstance(
    v_writerInstance instance,
    c_voidp arg)
{
    v_writerSample prev,sample,removed;
    v_message message;
    struct groupWriteArg grouparg;
    c_iter *emptyList = (c_iter *)arg;
    v_writer writer;
    c_bool proceed = TRUE;

    writer = v_writer(instance->writer);
    assert(!v_writerInstanceTestState(instance, L_EMPTY));
    if (!v_writerInstanceTestState(instance, L_EMPTY)) {
        sample = v_writerSample(instance->last);
        while ((sample != NULL) && proceed) {
            prev = v_writerSample(sample)->prev;
            message = v_writerSampleMessage(sample);
            /**
             * When writetime of the message is smaller than
             * the suspendTime of the publisher:
             * - the publisher is NOT suspended or
             * - the publisher IS suspended after this message was written
             * In both cases resend the message.
             */
            if (c_timeCompare(v_writerPublisher(writer)->suspendTime,
                              message->writeTime) == C_GT) {
                switch (v_writerSampleGetStatus(sample)) {
                case V_SAMPLE_RESEND:
                    v_writerSampleClear(sample);
                    v_statisticsULongValueInc(v_writer, numberOfRetries, writer);
                    grouparg.instance = instance;
                    grouparg.message = message;
                    grouparg.result = V_WRITE_SUCCESS;
                    grouparg.rejectScope = 0;

                    if(v_writerSampleHasBeenSentBefore(sample)){
                        grouparg.resendScope = sample->resendScope;
                        v_writerCacheWalk(instance->targetCache,
                                      groupInstanceResend,
                                      &grouparg);
                    } else {
                        grouparg.resendScope = V_RESEND_NONE;
                        v_writerCacheWalk(instance->targetCache,
                                      groupInstanceWrite,
                                      &grouparg);

                        v_writerSampleSetSentBefore(sample, TRUE);
                    }
                    if (grouparg.result == V_WRITE_REJECTED) {
                        v_writerSampleResend(sample,grouparg.rejectScope);
                        /* The sample could not be delivered because
                         * of the lack of resources at the receiver.
                         * Therefore it is of no use to try to resend
                         * any other samples so proceed is set to FALSE
                         * to abort resending for this writer.
                         */
                        proceed = FALSE;
                    }
                break;
                case V_SAMPLE_KEEP:
                    v_writerSampleRelease(sample);
                break;
                default:
                break;
                }
                if (V_SAMPLE_RELEASE == v_writerSampleGetStatus(sample)) {
                    removed = v_writerInstanceRemove(instance, sample);
                    assert(removed == sample);
                    writer->count--;
                    c_free(removed);
                }
            }
            sample = prev;
        }
        if (v_writerInstanceTestState(instance, L_EMPTY)) {
            /* If the instance is (has become) empty it is inserted into
             * an emptyList that is returned to the callee.
             * The callee (the writer) can use this information to remove
             * the instance from the set of cached 'resend' instances.
             */
            *emptyList = c_iterInsert(*emptyList, c_keep(instance));
        }
    }
    return proceed;
}

/* Used to resend messages by the resendmanager */
void
v_writerResend(
    v_writer writer)
{
    c_iter emptyList = NULL;
    v_writerInstance instance,found;
    int length;

    assert(writer != NULL);
    assert(C_TYPECHECK(writer,v_writer));

    v_observerLock(v_observer(writer));

    c_tableWalk(writer->resendInstances,(c_action)resendInstance,&emptyList);
    length = c_iterLength(emptyList);
    while ((instance = c_iterTakeFirst(emptyList)) != NULL) {
        found = c_remove(writer->resendInstances, instance, NULL, NULL);
        found->resend = FALSE;
        assert(found == instance);
        c_free(found);
        if (v_writerInstanceIsUnregistered(instance)) {
            found = c_remove(writer->instances,instance,NULL,NULL);
            assert(found == instance);
            UPDATE_WRITER_STATISTICS_REMOVE_INSTANCE(writer, instance);
            v_writerCacheDeinit(instance->targetCache);
            v_publicFree(v_public(instance));
            v_writerInstanceFree(found);
        }
        /*NK: Always free because it has been kept in the emptyList iterator!*/
        v_writerInstanceFree(instance);
    }
    /* Free the iterator here. If it is NULL, this statement is also valid */
    if (c_tableCount(writer->resendInstances) == 0) {
        v_participantResendManagerRemoveWriter(v_writerParticipant(writer),
                                               writer);
    }
    if (length) {
        v_observerNotify(v_observer(writer), NULL, NULL);
    }
    v_observerUnlock(v_observer(writer));
    c_iterFree(emptyList);
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
    livKind = w->qos->liveliness.kind;
    assertLiveliness(w);
    v_observerUnlock(v_observer(w));
    if (livKind == V_LIVELINESS_PARTICIPANT) {
        event.kind = V_EVENT_LIVELINESS_ASSERT;
        event.source = v_publicHandle(v_public(w));
        event.userData = NULL;
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
            v_statusReset(status, V_EVENT_DEADLINE_MISSED);
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

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    result = V_RESULT_PRECONDITION_NOT_MET;
    if (_this != NULL) {
        v_observerLock(v_observer(_this));
        status = v_entity(_this)->status;
        result = action(&v_writerStatus(status)->incompatibleQos, arg);
        if (reset) {
            v_statusReset(status, V_EVENT_INCOMPATIBLE_QOS);
        }
        v_writerStatus(status)->incompatibleQos.totalChanged = 0;
        v_observerUnlock(v_observer(_this));
    }
    return result;
}

v_result
v_writerGetTopicMatchStatus(
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
            v_statusReset(status, V_EVENT_TOPIC_MATCHED);
        }
        v_writerStatus(status)->publicationMatch.totalChanged = 0;
        v_writerStatus(status)->publicationMatch.currentChanged = 0;
        v_observerUnlock(v_observer(_this));
    }
    return result;
}

void
v_writerCheckDeadlineMissed(
    v_writer w,
    c_time now)
{
    c_bool changed = FALSE;
    C_STRUCT(v_event) e;
    c_iter missed;
    v_writerInstance instance;
    v_message message;
    v_duration period;
    v_observerLock(v_observer(w));

    /*
     * We are dealing with a potential automatic unregister under the
     * following conditions:
     * 1. the deadlineCountLimit equals 1
     * 2. the deadlineList is not empty AND the first instance in the deadline
     *    list has a deadlineCount equal to the deadlineCountLimit minus 1
     */

    if ((w->deadlineCountLimit == 1) ||
        ((!v_deadLineInstanceListEmpty(w->deadlineList)) &&
         (v_writerInstance(v_instance(w->deadlineList)->next)->deadlineCount == w->deadlineCountLimit - 1))) {
        period = w->qos->lifecycle.autounregister_instance_delay;
    } else {
        period = w->qos->deadline.period;
    }
    missed = v_deadLineInstanceListCheckDeadlineMissed(w->deadlineList, period, now);

    instance = v_writerInstance(c_iterTakeFirst(missed));
    while (instance != NULL) {
        instance->deadlineCount++;

        /* The deadlineCountlimit drives the behavior of the auto_unregister policy of the writer. It piggybacks
         * on the deadline mechanism and uses the deadlineCountLimit to express the ratio between deadline period and
         * autounregister period.
         */
        if (instance->deadlineCount == w->deadlineCountLimit) { /* unregister */
            message = v_writerInstanceCreateMessage(instance);
            writerUnregister(w, message, now, instance);
            c_free(message);
        } else {
            if (v_statusNotifyDeadlineMissed(v_entity(w)->status,
                                             v_publicHandle(v_public(instance))))
            {
                changed = TRUE;
            }
            /* do not use deadlineUpdate(w, instance);
             * since it will also reset instance->deadlineCount
                 v_deadLineInstanceListUpdate(w->deadlineList,
                                            v_instance(instance));
                 */
        }
        /* do not use deadlineUpdate(w, instance);
         * since it will also reset instance->deadlineCount */
        instance = v_writerInstance(c_iterTakeFirst(missed));
    }
    c_iterFree(missed);
    /* next wake-up time only needs to be changed iff
     * 1. deadlineCountLimit > 1
     *      if ==1 then
     *         immediate unregister so period was already equal to
     *         autounregister_instance_delay
     *      if <1 then
     *         autounregister_instance_delay is disabled,
     *         so period is deadline.period.
     * 2. deadlineInstanceList is not empty
     * 3. first instance in deadlineList is the first to expire,
     *    so only period needs to be adapted if
     *    it almost reached the deadlineCountLimit.
     */
    if ((w->deadlineCountLimit > 1) &&
        (!v_deadLineInstanceListEmpty(w->deadlineList)) &&
        (v_writerInstance(v_instance(w->deadlineList)->next)->deadlineCount == w->deadlineCountLimit - 1)) {
        period = c_timeSub(w->qos->lifecycle.autounregister_instance_delay,
                           w->qos->deadline.period);
        v_deadLineInstanceListSetDuration(w->deadlineList, period);
    }

    if (changed) {
        e.kind = V_EVENT_DEADLINE_MISSED;
        e.source = v_publicHandle(v_public(w));
        e.userData = NULL;
        v_observerNotify(v_observer(w), &e, NULL);
        v_observerUnlock(v_observer(w));
        v_observableNotify(v_observable(w), &e);
    } else {
        v_observerUnlock(v_observer(w));
    }

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

static c_bool
instanceResume(
    c_object o,
    c_voidp arg)
{
    v_writerInstance instance = v_writerInstance(o);
    v_writer w = v_writer(instance->writer);
    c_time *suspendTime = (c_time *)arg;
    v_message message;
    v_writerSample removed;
    v_writerSample s;
    v_writerSample prev;

/* When the instance state is suspended, the
 * instance pipeline must be constructed, even when the instance is empty!
 */
    if (v_stateTest(instance->state, L_SUSPENDED)) {
        v_stateClear(instance->state, L_SUSPENDED);
        /* create a register message, so the instance pipeline is constructed! */
        message = v_writerInstanceCreateMessage(instance);
        v_nodeState(message) = L_REGISTER;
        message->writeTime = v_timeGet();
        message->writerGID = v_publicGid(v_public(w));
        message->sequenceNumber = w->sequenceNumber++;
        message->writerInstanceGID = v_publicGid(v_public(instance));
        message->qos = c_keep(w->relQos);
        message->transactionId = 0;

        writerResend(instance, message, TRUE);
        c_free(message);
    }

    if (v_writerInstanceTestState(instance, L_EMPTY)) {
        return TRUE; /* just continue with next instance */
    }

    s = c_keep(v_writerInstance(instance)->last);
    while (s != NULL) {
        prev = v_writerSample(c_keep(s->prev));
        /* Check whether this sample was suspended, samples that were
         * not suspended are handled through the regular resend mechanism
         */
        if (c_timeCompare(v_writerSampleMessage(s)->writeTime,
                          *suspendTime) != C_LT) {
            removed = v_writerInstanceRemove(instance, s);
            assert(removed == s);
            c_free(removed);
            w->count--;
            message = v_writerSampleMessage(s);
            writerResend(instance, message, FALSE);
        }
        c_free(s);
        s = prev;
    }

    return TRUE; /* continue with next instance */
}

void
v_writerResumePublication(
    v_writer writer,
    c_time *suspendTime)
{
    c_time expiry;
    assert(C_TYPECHECK(writer,v_writer));

    v_observerLock(v_observer(writer));

    /* autpurge last remaining samples, since publisher was suspended but
     * now resumed.
     * Do not use the static function autoPurgeSuspendedSamples() as this
     * function will check whether the publisher is suspended, while it is
     * not anymore.
     */
    if (c_timeCompare(writer->qos->lifecycle.autopurge_suspended_samples_delay,
                      C_TIME_INFINITE) != C_EQ) {
        expiry = c_timeSub(v_timeGet(),
                           writer->qos->lifecycle.autopurge_suspended_samples_delay);
        c_tableWalk(writer->instances, writerInstanceAutoPurgeSuspended, &expiry);
    }

    c_tableWalk(writer->instances, instanceResume, (c_voidp)suspendTime);
    v_observerUnlock(v_observer(writer));
}

v_result
v_writerCoherentBegin (
    v_writer _this,
    c_ulong id)
{
    v_result result;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    v_observerLock(v_observer(_this));
    if ((_this->transactionId == 0) &&
        (_this->pubQos->presentation.coherent_access == TRUE))
    {
        _this->transactionId = id;
        _this->transactionCount = 0;
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    v_observerUnlock(v_observer(_this));
    return result;
}

v_result
v_writerCoherentEnd (
    v_writer _this)
{
    v_message message;
    v_writerInstance instance, dummy;
    v_result result;
    v_writeResult wResult;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_writer));

    v_observerLock(v_observer(_this));

    /* This function does not instantiate a message from scratch but instead
     * creates a clone of the last message sent.  This is so that the act
     * of completing a transaction actually (re)sends the last message.
     * The reader will then replace the previously sent message with this one,
     * indicating that the transaction is complete. See OSPL-1013.
     */

    if (_this->transactionId != 0) {
        if (_this->transactionsLastMessage) {
            dummy = v_writerInstanceNew(_this, _this->transactionsLastMessage);
            instance = c_find(_this->instances, dummy);
            c_free(dummy);
            assert(instance);

            c_cloneIn(v_topicMessageType(_this->topic), _this->transactionsLastMessage, (c_voidp*)&(message));
            c_free (_this->transactionsLastMessage);
            _this->transactionsLastMessage = NULL;
            if (message) {
                v_stateSet(v_nodeState(message), L_TRANSACTION);
                /* This message needs to describe the number of transactions delivered so far:
                 * The design for OSPL-1013 states that the current transactionId (32-bits)
                 * should also be used for storing the number of messages belonging to the
                 * transaction. In order not to limit the number of messages that can
                 * belong to a transaction too much, we propose that 24 bits are used
                 * for the number of transaction messages, and 8 bits for the unique
                 * transaction's id. (Note that the id is always evaluated in combination
                 * with the writer's GID, so 256 unique id's seems to be enough). That
                 * means that we propose to shift the number of transaction messages 8
                 * bits to the left and then add the transaction's unique id, which is
                 * always increased by one with a modulo of 256.
                 */
                message->transactionId = V_MESSAGE_SET_TRANSACTION_ID(_this->transactionId,_this->transactionCount);
                message->sequenceNumber = _this->sequenceNumber++;
                wResult = writerWrite(instance,message,FALSE);
                if (wResult == V_WRITE_SUCCESS || wResult == V_WRITE_REJECTED) {
                  result = V_RESULT_OK;
                } else {
                  OS_REPORT_1(OS_ERROR,
                              "v_writerCoherentEnd", 0,
                              "Received unexpected writeResult from writerWrite(): %d", wResult);
                  result = V_RESULT_PRECONDITION_NOT_MET;
                }
                c_free (message);
                result = V_RESULT_OK;
            } else {
                OS_REPORT(OS_ERROR,
                          "v_writerCoherentEnd", 0,
                          "Could not allocate resources for end transaction message");
                result = V_RESULT_PRECONDITION_NOT_MET;
            }
            c_free(instance);
        } else {
            result = V_RESULT_OK;
        }
        /* Even if the transactionsLastMessage is NULL (indicating that no samples
         * were sent during the transaction), the transaction should still be ended
         * so set transactionId to 0 */
        _this->transactionId = 0;
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
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
    assert(C_TYPECHECK(instance, v_writerInstance));

    instanceWriter = v_writerInstanceWriter(instance);
    if (instanceWriter != NULL) {
        result = (instanceWriter == _this);
    } else {
        OS_REPORT_2(OS_ERROR, "v_writerContainsInstance", 0,
            "Invalid writerInstance: no attached DataWriter"
            "<_this = 0x%x instance = 0x%x>", _this, instance);
        result = FALSE;
    }
    return result;
}

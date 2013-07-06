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
#include "os_abstract.h"
#include "os_heap.h"
#include "os_report.h"

#include "c_iterator.h"

#include "v__subscriber.h"
#include "v__subscriberQos.h"
#include "v_participant.h"
#include "v__partition.h"
#include "v_topic.h"
#include "v__partitionAdmin.h"
#include "v__reader.h"
#include "v__deliveryService.h"
#include "v_group.h"
#include "v__observer.h"
#include "v_observable.h"
#include "v_public.h"
#include "v_groupSet.h"
#include "v_dataReaderEntry.h"
#include "v_status.h"
#include "v_event.h"
#include "v__groupStream.h"
#include "v__groupQueue.h"
#include "v__networkReader.h"
#include "v__dataReader.h"
#include "v__policy.h"
#include "v__kernel.h"
#include "v_partition.h"


/* ----------------------------------- Private ------------------------------ */
static c_bool
qosChangedAction(
    c_object o,
    c_voidp arg)
{
    v_dataReader r;

    if (v_objectKind(o) == K_DATAREADER) {
        r = v_dataReader(o);
        v_dataReaderUpdateConnections(r,(v_dataReaderConnectionChanges *)arg);
        v_dataReaderNotifyChangedQos(r);
    }

    return TRUE;
}

/* ----------------------------------- Public ------------------------------- */

v_subscriber
v_subscriberNew(
    v_participant p,
    const c_char *name,
    v_subscriberQos qos,
    c_bool enable)
{
    v_kernel kernel;
    v_subscriber s;
    v_subscriberQos q;
    v_entity found;
    v_accessMode access;

    kernel = v_objectKernel(p);
    /* ES, dds1576: If a partition policy was provided then we need to verify
     * if the partition policy does not contain any partition expressions for
     * which read access is not allowed.
     * If read access is not allowed for one of the partitions listed in the
     * partition policy of the qos, then the subscriber will not be created at
     * all.
     */
    if(qos && qos->partition)
    {
        access = v_kernelPartitionAccessMode(kernel, qos->partition);
    } else
    {
        access = V_ACCESS_MODE_READ_WRITE;/* default */
    }
    if(access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_READ)
    {
        q = v_subscriberQosNew(kernel,qos);
        if (q != NULL) {
            s = v_subscriber(v_objectNew(kernel,K_SUBSCRIBER));
            v_observerInit(v_observer(s),name, NULL, enable);
            s->qos = q;
            c_mutexInit(&s->sharesMutex, SHARED_MUTEX);
            if (q->share.enable) {
                v_lockShares(kernel);
                found = v_addShareUnsafe(kernel,v_entity(s));
                if (found != v_entity(s)) {
                    /* Make sure to set the partition list to NULL, because
                     * v_publicFree will cause a crash in the v_subscriberDeinit
                     * otherwise.
                     */
                    s->partitions = NULL;
                    /*v_publicFree to free reference held by the handle server.*/
                    v_publicFree(v_public(s));
                    /*Now free the local reference as well.*/
                    c_free(s);
                    pa_increment(&(v_subscriber(found)->shareCount));
                    v_unlockShares(kernel);
                    return c_keep(found);
                }
                s->shares = c_tableNew(v_kernelType(kernel,K_READER),
                                       "qos.share.name");
            } else {
                s->shares = NULL;
            }
            s->shareCount  = 1;
            s->partitions  = v_partitionAdminNew(kernel);
            s->readers     = c_setNew(v_kernelType(kernel,K_READER));

            if (q->share.enable) {
                s->participant = kernel->builtin->participant;
            } else {
                s->participant = p;
            }

            c_lockInit(&s->lock,SHARED_LOCK);
            v_participantAdd(v_participant(s->participant),v_entity(s));

            if (q->share.enable) {
                v_unlockShares(kernel);
            }
            if (enable) {
                v_subscriberEnable(s);
            }
        } else {
            OS_REPORT(OS_ERROR,
                      "v_subscriberNew", 0,
                      "Subscriber not created: inconsistent qos");
            s = NULL;
        }
    } else
    {
        OS_REPORT(OS_ERROR,
              "v_subscriberNew", 0,
              "Subscriber not created: Access rights for one of the partitions listed in the partition list was not sufficient (i.e. read or readwrite).");
        s = NULL;
    }
    return s;
}

v_result
v_subscriberEnable (
    v_subscriber _this)
{
    v_kernel kernel;
    c_iter list;
    c_char *partitionName;
    v_result result = V_RESULT_ILL_PARAM;

    if (_this) {
        kernel = v_objectKernel(_this);

        v_observableAddObserver(v_observable(kernel->groupSet),
                                v_observer(_this), NULL);

        if (_this->qos->partition != NULL) {
            list = v_partitionPolicySplit(_this->qos->partition);
            while((partitionName = c_iterTakeFirst(list)) != NULL) {
                v_subscriberSubscribe(_this,partitionName);
                os_free(partitionName);
            }
            c_iterFree(list);
        }
        result = V_RESULT_OK;
    }
    return result;
}


void
v_subscriberFree(
    v_subscriber s)
{
    v_kernel kernel;
    v_participant p;
    v_reader o;
    v_entity found;
    c_long sc;

    kernel = v_objectKernel(s);

    sc = (c_long)pa_decrement(&(s->shareCount));
    if (sc > 0) return;

    if(sc == 0){
        v_observableRemoveObserver(v_observable(kernel->groupSet),v_observer(s), NULL);
        if (s->qos->share.enable) {
            found = v_removeShare(kernel,v_entity(s));
            assert(found == v_entity(s));
            c_free(found);
        }
        while ((o = c_take(s->readers)) != NULL) {
            switch (v_objectKind(o)) {
            case K_DATAREADER:
                v_dataReaderFree(v_dataReader(o));
            break;
            case K_DELIVERYSERVICE:
                v_deliveryServiceFree(v_deliveryService(o));
            break;
            case K_GROUPQUEUE:
                v_groupQueueFree(v_groupQueue(o));
            break;
            case K_NETWORKREADER:
                v_networkReaderFree(v_networkReader(o));
            break;
            default:
                OS_REPORT_1(OS_ERROR,
                            "v_subscriber", 0,
                            "Unknown reader %d",
                            v_objectKind(o));
                assert(FALSE);
            break;
            }
            c_free(o);
        }
        p = v_participant(s->participant);
        if (p != NULL) {
            v_participantRemove(p,v_entity(s));
            s->participant = NULL;
        }
        v_publicFree(v_public(s));
    } else {
        OS_REPORT_1(OS_ERROR,  "v_subscriberFree", 0,
                "subscriber already freed (shareCount is now %d).", sc);
        assert(sc == 0);
    }
}

void
v_subscriberDeinit(
    v_subscriber s)
{
    v_observerDeinit(v_observer(s));
}

static c_bool
collectPartitions(
    c_object o,
    c_voidp arg)
{
    v_partition d = (v_partition)o;
    c_iter iter = (c_iter)arg;

    iter = c_iterInsert(iter,c_keep(d));
    return TRUE;
}

void
v_subscriberAddReader(
    v_subscriber s,
    v_reader r)
{
    v_reader found;
    v_partition d;
    c_iter iter;

    assert(s != NULL);
    assert(r != NULL);

    iter = c_iterNew(NULL);
    c_lockWrite(&s->lock);
    v_partitionAdminWalk(s->partitions,collectPartitions,iter);
    while ((d = c_iterTakeFirst(iter)) != NULL) {
        v_readerSubscribe(r,d);
        c_free(d);
    }
    found = c_setInsert(s->readers,r);
    c_lockUnlock(&s->lock);
    if (found != r) {
        OS_REPORT_1(OS_ERROR,
                    "v_subscriberAddReader", 0,
                    "shared <%s> name already defined",
                    r->qos->share.name);
    }
    c_iterFree(iter);
}

void
v_subscriberRemoveReader(
    v_subscriber s,
    v_reader r)
{
    v_reader found;
    v_partition d;
    c_iter iter;

    assert(s != NULL);
    assert(r != NULL);

    iter = c_iterNew(NULL);

    c_lockWrite(&s->lock);
    found = c_remove(s->readers,r,NULL,NULL);
    v_partitionAdminWalk(s->partitions,collectPartitions,iter);
    while ((d = c_iterTakeFirst(iter)) != NULL) {
        /* ES, dds1576: The unsubscribe here is performed for all partitions,
         * however for the K_DATAREADER class not all partitions were
         * actually subscribed too. Instead of verifying if a partition was
         * subscribed too or not, the unsubscribe is just executed every time
         * as this is 'cheaper' then performing the access check again.
         * If a partition was not subscribed too then the unsubcribe will have no
         * effect.
         */
        v_readerUnSubscribe(r,d);
        c_free(d);
    }
    c_lockUnlock(&s->lock);
    c_iterFree(iter);
    c_free(found);
}


void
v_subscriberLockShares(
    v_subscriber _this)
{
    c_mutexLock(&_this->sharesMutex);
}

void
v_subscriberUnlockShares(
    v_subscriber _this)
{
    c_mutexUnlock(&_this->sharesMutex);
}

v_reader
v_subscriberAddShareUnsafe(
    v_subscriber _this,
    v_reader reader)
{
    v_reader found;

    found = c_tableInsert(_this->shares,reader);

    return found;
}

v_reader
v_subscriberRemoveShare(
    v_subscriber _this,
    v_reader reader)
{
    v_reader found;

    v_subscriberLockShares(_this);
    found = c_remove(_this->shares,reader,NULL,NULL);
    v_subscriberUnlockShares(_this);

    return found;
}


c_bool
v_subscriberCheckPartitionInterest(
    v_subscriber s,
    v_partition partition)
{
    return v_partitionAdminFitsInterest(s->partitions, partition);
}

void
v_subscriberSubscribe(
    v_subscriber s,
    const c_char *partitionExpr)
{
    v_partition d;
    v_dataReaderConnectionChanges arg;
    v_partitionPolicy old;

    assert(s != NULL);

    arg.removedPartitions = NULL;

    c_lockWrite(&s->lock);
    arg.addedPartitions = v_partitionAdminAdd(s->partitions, partitionExpr);
    old = s->qos->partition;
    s->qos->partition = v_partitionPolicyAdd(old, partitionExpr,
                                             c_getBase(c_object(s)));
    c_free(old);

    c_setWalk(s->readers, qosChangedAction, &arg);
    d = v_partition(c_iterTakeFirst(arg.addedPartitions));
    while (d != NULL) {
        c_free(d);
        d = v_partition(c_iterTakeFirst(arg.addedPartitions));
    }
    c_iterFree(arg.addedPartitions);
    c_lockUnlock(&s->lock);
}


void
v_subscriberUnSubscribe(
    v_subscriber s,
    const c_char *partitionExpr)
{
    v_partition d;
    v_dataReaderConnectionChanges arg;
    v_partitionPolicy old;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_subscriber));

    arg.addedPartitions = NULL;

    c_lockWrite(&s->lock);

    arg.removedPartitions = v_partitionAdminRemove(s->partitions, partitionExpr);
    old = s->qos->partition;
    s->qos->partition = v_partitionPolicyRemove(old, partitionExpr,
                                                c_getBase(c_object(s)));
    c_free(old);
    c_setWalk(s->readers, qosChangedAction, &arg);

    d = v_partition(c_iterTakeFirst(arg.removedPartitions));
    while (d != NULL) {
        c_free(d);
        d = v_partition(c_iterTakeFirst(arg.removedPartitions));
    }
    c_iterFree(arg.removedPartitions);

    c_lockUnlock(&s->lock);
}

c_iter
v_subscriberLookupReaders(
    v_subscriber s)
{
    c_iter list;

    assert(s != NULL);

    c_lockRead(&s->lock);
    list = ospl_c_select(s->readers,0);
    c_lockUnlock(&s->lock);
    return list;
}

struct lookupReaderByTopicArg {
    c_iter list;
    const c_char *topicName;
};

static c_bool
checkTopic (
    c_object o,
    c_voidp arg)
{
    v_dataReaderEntry entry;
    c_char *topicName = (c_char *)arg;

    if(v_object(o)->kind == K_DATAREADERENTRY){
        entry = v_dataReaderEntry(o);
        if (strcmp(v_topicName(entry->topic),topicName) == 0) {
            return FALSE;
        }
    }
    return TRUE;
}

static c_bool
lookupReaderByTopic(
    v_reader reader,
    c_voidp arg)
{
    struct lookupReaderByTopicArg *a = (struct lookupReaderByTopicArg *)arg;
    c_bool found = FALSE;

    if (a->topicName) {
        found = !v_readerWalkEntries(reader,
                                     checkTopic,
                                     (c_voidp)a->topicName);
    }
    if (found) {
        a->list = c_iterInsert(a->list, c_keep(reader));
    }
    return TRUE;
}

c_iter
v_subscriberLookupReadersByTopic(
    v_subscriber s,
    const c_char *topicName)
{
    struct lookupReaderByTopicArg arg;

    assert(s != NULL);

    arg.list = NULL;
    arg.topicName = topicName;

    c_lockRead(&s->lock);
    c_setWalk(s->readers, (c_action)lookupReaderByTopic, &arg);
    c_lockUnlock(&s->lock);

    return arg.list;
}

c_iter
v_subscriberLookupPartitions(
    v_subscriber s,
    const c_char *partitionExpr)
{
    c_iter list;

    assert(s != NULL);

    c_lockRead(&s->lock);
    list = v_partitionAdminLookup(s->partitions, partitionExpr);
    c_lockUnlock(&s->lock);

    return list;
}

v_subscriberQos
v_subscriberGetQos(
    v_subscriber s)
{
    v_subscriberQos qos;

    assert(s != NULL);

    c_lockRead(&s->lock);
    qos = v_subscriberQosNew(v_objectKernel(s), s->qos);
    c_lockUnlock(&s->lock);

    return qos;
}

v_result
v_subscriberSetQos(
    v_subscriber s,
    v_subscriberQos qos)
{
    v_result result;
    v_qosChangeMask cm;
    v_dataReaderConnectionChanges arg;
    v_partition d;
    v_accessMode access;

    assert(s != NULL);

    arg.addedPartitions = NULL;
    arg.removedPartitions = NULL;

    c_lockWrite(&s->lock);

    /* ES, dds1576: When the QoS is being set we have to verify that the partitions
     * listed in the qos do not have an invalid access mode set. For a subscriber
     * an invalid mode is characterized as 'write' mode. If the partitionAccess
     * check returns ok, then everything continue as normal. If the
     * partitionAccess check returns something else, then the setQos operation
     * is aborted. Please see the partitionAccess check operation to know when
     * a partition expression is not allowed.
     */
    if(qos && qos->partition)
    {
        access = v_kernelPartitionAccessMode(v_objectKernel(s), qos->partition);
    } else
    {
        access = V_ACCESS_MODE_READ_WRITE;/* default */
    }
    if(access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_READ)
    {
        result = v_subscriberQosSet(s->qos, qos, v_entity(s)->enabled, &cm);
        if ((result == V_RESULT_OK) && (cm != 0)) {
            c_lockUnlock(&s->lock); /* since creation builtin topic might lock subscriber again. */
            if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
                v_partitionAdminSet(s->partitions, s->qos->partition,
                                 &arg.addedPartitions, &arg.removedPartitions);
            }

            c_setWalk(s->readers, qosChangedAction, &arg);

            d = v_partition(c_iterTakeFirst(arg.addedPartitions));
            while (d != NULL) {
                c_free(d);
                d = v_partition(c_iterTakeFirst(arg.addedPartitions));
            }
            c_iterFree(arg.addedPartitions);
            d = v_partition(c_iterTakeFirst(arg.removedPartitions));
            while (d != NULL) {
                c_free(d);
                d = v_partition(c_iterTakeFirst(arg.removedPartitions));
            }
            c_iterFree(arg.removedPartitions);
        } else {
            c_lockUnlock(&s->lock);
        }
    } else
    {
        result = V_RESULT_PRECONDITION_NOT_MET;
        c_lockUnlock(&s->lock);
    }
    return result;
}

static c_bool
notifyGroupQueues(
    v_reader reader,
    v_group group)
{
    if(v_objectKind(reader) == K_GROUPQUEUE){
        v_groupStreamConnectNewGroups(v_groupStream(reader), group);
    }
    return TRUE;
}

void
v_subscriberNotify(
    v_subscriber _this,
    v_event e)
{
    /* This Notify method is part of the observer-observable pattern.
     * It is designed to be invoked when _this object as observer receives
     * an event from an observable object.
     * It must be possible to pass the event to the subclass of itself by
     * calling <subclass>Notify(_this, event, userData).
     * This implies that _this cannot be locked within any Notify method
     * to avoid deadlocks.
     * For consistency _this must be locked by v_observerLock(_this) before
     * calling this method.
     */
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(e);
}

void
v_subscriberConnectNewGroup(
    v_subscriber s,
    v_group g)
{
    c_bool connect;
    c_iter addedPartitions;
    v_partition d;

    c_lockWrite(&s->lock);
    connect = v_partitionAdminFitsInterest(s->partitions, g->partition);
    if (connect) {
        addedPartitions = v_partitionAdminAdd(s->partitions,
                                        v_partitionName(g->partition));
        d = v_partition(c_iterTakeFirst(addedPartitions));
        while (d != NULL) {
            c_free(d);

            d = v_partition(c_iterTakeFirst(addedPartitions));
        }
        c_iterFree(addedPartitions);
        c_setWalk(s->readers, (c_action)v_readerSubscribeGroup, g);
    } else {
        /*
         * Check if group fits interest. This extra steps are needed because
         * the groupActionStream does not create the groups that match the
         * subscriber qos partition expression on creation. It only needs to
         * connect to new groups once they are created. This is a different
         * approach then for a data reader.
         */
        /*
         * Because already existing partitions are created and added to the
         * subscriber of the groupActionStream at creation time, these
         * partitions can be resolved from the subscriber. This is necessary to
         * determine whether the groupActionStream should connect to the new
         * group or if it is already connected.
         */
        if (v_partitionAdminExists(s->partitions, v_partitionName(g->partition))) {
            c_setWalk(s->readers, (c_action)notifyGroupQueues, g);
        }
    }
    c_lockUnlock(&s->lock);
}

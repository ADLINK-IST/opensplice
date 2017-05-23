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
#include "v__builtin.h"
#include "v_group.h"
#include "v__observer.h"
#include "v_observable.h"
#include "v_listener.h"
#include "v_public.h"
#include "v_groupSet.h"
#include "v_dataReaderEntry.h"
#include "v_status.h"
#include "v_event.h"
#include "v__groupStream.h"
#include "v__groupQueue.h"
#include "v__networkReader.h"
#include "v__dataReader.h"
#include "v_dataReaderInstance.h"
#include "v__policy.h"
#include "v__kernel.h"
#include "v__entity.h"
#include "v_partition.h"
#include "os_atomics.h"
#include "v__orderedInstance.h"

/**
 * Subscriber locking
 *
 * The subscriber has two type of locking.
 *
 * - subscriber lock, the subscriber has a subscriber lock (v_subscriberLock and
 * v_subscriberUnlock) which protects access to the subscriber. When holding this
 * lock it's NOT allowed to call operations on the readers as the lock order is
 * readerLock -> subscriberLock.
 *
 * - access lock, this is a sort of read/write locking used when the presentation
 * qos GROUP with ordering or coherency is set. This lock is used to guarantee that
 * the state of the readers belonging to this subscriber is not changed after a
 * BeginAccess(read) is called. When this lock is acquired via LockAccess(write)
 * the state of the readers can be changed and it's allowed to take the readerLock.
 * BeginAccess and LockAccess are mutual exclusive.
 */

#if 0
#define _TRACE_EVENTS_ printf
#else
#define _TRACE_EVENTS_(...)
#endif

#define subscriberCondWait(_this) \
        c_condWait(&_this->cond, &_this->mutex);

#define subscriberCondBroadcast(_this) \
        c_condBroadcast(&_this->cond);

static void
subscriberLockAccessIgnoreAccessCount(
    v_subscriber _this);

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
    v_subscriber s = NULL;
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
    if(qos && qos->partition.v) {
        access = v_kernelPartitionAccessMode(kernel, qos->partition);
    } else{
        access = V_ACCESS_MODE_READ_WRITE;/* default */
    }

    if(access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_READ) {
        if (v_subscriberQosCheck(qos) == V_RESULT_OK) {
            q = v_subscriberQosNew(kernel, qos);
            if (q != NULL) {
                s = v_subscriber(v_objectNew(kernel,K_SUBSCRIBER));
                v_entityInit(v_entity(s), name, enable);
                s->qos = q;
                /* Presentation policy must be properly initialized to avoid
                   dirty reads. The presentation policy is overwritten once the
                   subscriber is enabled. */
                s->presentation = s->qos->presentation.v;
                s->accessCount = 0;
                s->accessBusy = FALSE;
                c_mutexInit(c_getBase(s), &s->mutex);
                c_condInit(c_getBase(s), &s->cond, &s->mutex);
                c_mutexInit(c_getBase(s), &s->sharesMutex);

                if (q->share.v.enable) {
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
                        v_subscriber(found)->shareCount++;
                        v_unlockShares(kernel);
                        return c_keep(found);
                    }
                    s->shares = c_tableNew(v_kernelType(kernel,K_READER),
                                           "qos.share.v.name");
                } else {
                    s->shares = NULL;
                }

                s->orderedInstance = NULL;
                s->shareCount = 1;
                s->partitions  = v_partitionAdminNew(kernel);
                s->readers     = c_setNew(v_kernelType(kernel,K_READER));
                s->transactionGroupAdmin = NULL;

                if (q->share.v.enable) {
                    s->participant = kernel->builtin->participant;
                } else {
                    s->participant = p;
                }

                v_participantAdd(v_participant(s->participant),v_object(s));

                if (q->share.v.enable) {
                    v_unlockShares(kernel);
                }
                if (enable) {
                    v_subscriberEnable(s);
                }
            } else {
                OS_REPORT(OS_ERROR, "v_subscriberNew", V_RESULT_INTERNAL_ERROR,
                          "Subscriber <%s> not created: failed to create subscriber QoS",
                          name);
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_subscriberNew", V_RESULT_PRECONDITION_NOT_MET,
              "Subscriber not created: Access rights for one of the partitions listed in the partition list was not sufficient (i.e. read or readwrite).");
    }
    return s;
}

v_result
v_subscriberEnable (
    v_subscriber _this)
{
    v_kernel kernel;
    v_message builtinCMMsg;
    c_iter list;
    c_char *partitionName;
    v_result result = V_RESULT_ILL_PARAM;

    if (_this) {
        /* The accessLock is only used in case of group access scope, but to
           to check if it should be locked, the qos is dereferenced without
           locking the subscriber. This is fine as the presentation qos is
           immutable, but the qos object itself can be reset. A read-only copy
           is saved to make sure no invalid memory will be read. Hereafter,
           only the read-only copy should be used. */
        _this->presentation = _this->qos->presentation.v;

        kernel = v_objectKernel(_this);

        v_observableAddObserver(v_observable(kernel->groupSet),
                                v_observer(_this), NULL);

        if (_this->qos->partition.v != NULL) {
            list = v_partitionPolicySplit(_this->qos->partition);
            while((partitionName = c_iterTakeFirst(list)) != NULL) {
                v_subscriberSubscribe(_this,partitionName);
                os_free(partitionName);
            }
            c_iterFree(list);
        }

        if (_this->presentation.access_scope == V_PRESENTATION_GROUP &&
            _this->presentation.coherent_access)
        {
            _this->transactionGroupAdmin = v_transactionGroupAdminNew(v_object(_this));
        }

        builtinCMMsg = v_builtinCreateCMSubscriberInfo(kernel->builtin, _this);
        v_writeBuiltinTopic(kernel, V_CMSUBSCRIBERINFO_ID, builtinCMMsg);
        c_free(builtinCMMsg);
        result = V_RESULT_OK;
    }
    return result;
}


void
v_subscriberFree(
    v_subscriber s)
{
    v_kernel kernel;
    v_message builtinCMMsg;
    v_message unregisterCMMsg;
    v_participant p;
    v_reader o;
    v_entity found;

    kernel = v_objectKernel(s);

    if (s->qos->share.v.enable) {
        v_lockShares(kernel);
        if (--s->shareCount == 0) {
            found = v_removeShareUnsafe(kernel,v_entity(s));
            assert(found == v_entity(s));
            c_free(found);
            v_unlockShares(kernel);
        } else {
            v_unlockShares(kernel);
            return;
        }
    }

    v_observableRemoveObserver(v_observable(kernel->groupSet),v_observer(s), NULL);
    builtinCMMsg = v_builtinCreateCMSubscriberInfo(kernel->builtin, s);
    unregisterCMMsg = v_builtinCreateCMSubscriberInfo(kernel->builtin, s);

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
            OS_REPORT(OS_CRITICAL,
                        "v_subscriber", V_RESULT_INTERNAL_ERROR,
                        "Unknown reader %d",
                        v_objectKind(o));
            assert(FALSE);
        break;
        }
        c_free(o);
    }
    p = v_participant(s->participant);
    if (p != NULL) {
        s->participant = NULL;
        v_participantRemove(p,v_object(s));
    }
    v_entityFree(v_entity(s));

    v_writeDisposeBuiltinTopic(kernel, V_CMSUBSCRIBERINFO_ID, builtinCMMsg);
    v_unregisterBuiltinTopic(kernel, V_CMSUBSCRIBERINFO_ID, unregisterCMMsg);
    v_orderedInstanceRemove (s->orderedInstance, v_entity(s));
    c_free(builtinCMMsg);
    c_free(unregisterCMMsg);
}

void
v_subscriberDeinit(
    v_subscriber s)
{
    v_entityDeinit(v_entity(s));
}

static c_bool
collectPartitions(
    c_object o,
    c_voidp arg)
{
    v_partition d = (v_partition)o;
    c_iter iter = (c_iter)arg;
    (void) c_iterInsert(iter,c_keep(d));
    return TRUE;
}

static c_bool
collectReaders(
    c_object object,
    c_voidp arg)
{
    c_iter *list = (c_iter *) arg;
    *list = c_iterInsert(*list, c_keep(object));
    return TRUE;
}

v_result
v_subscriberAddReader(
    v_subscriber s,
    v_reader r)
{
    v_result result = V_RESULT_OK;
    v_reader found = NULL;
    v_partition d;
    c_iter iter;
    v_kernel kernel;
    c_iter groupReaders = NULL;
    c_bool groupCoherent = FALSE;

    assert(s != NULL);
    assert(r != NULL);

    iter = c_iterNew(NULL);
    v_subscriberLock(s);
    kernel = v_objectKernel(s);
    if (s->presentation.ordered_access) {
        if (s->presentation.access_scope == V_PRESENTATION_GROUP) {
            if (s->orderedInstance == NULL) {
                s->orderedInstance = v_orderedInstanceNew (
                    v_entity(s), s->presentation.access_scope, r->qos->orderby.v.kind);
                if (s->orderedInstance == NULL) {
                    result = V_RESULT_OUT_OF_MEMORY;
                    OS_REPORT (OS_ERROR, OS_FUNCTION, result,
                        "Could not enabled reader, failed to create ordered instance");
                } else {
                    v_dataReader(r)->orderedInstance = c_keep (s->orderedInstance);
                }
            } else {
                if (r->qos->orderby.v.kind == s->orderedInstance->orderby) {
                    v_dataReader(r)->orderedInstance = c_keep (s->orderedInstance);
                } else {
                    result = V_RESULT_INCONSISTENT_QOS;
                    OS_REPORT (OS_ERROR, OS_FUNCTION, result,
                        "Could not enable reader, destination order inconsistent "
                        "with presentation and reader set on subscriber");
                }
            }
        } else {
            assert (s->orderedInstance == NULL);
            assert (v_dataReader(r)->orderedInstance == NULL);
            v_dataReader(r)->orderedInstance = v_orderedInstanceNew (
                v_entity(r), s->presentation.access_scope, r->qos->orderby.v.kind);
            if (v_dataReader(r)->orderedInstance == NULL) {
                result = V_RESULT_OUT_OF_MEMORY;
                OS_REPORT (OS_ERROR, OS_FUNCTION, result,
                    "Could not enable reader, failed to create ordered instance");
            }
        }
    }

    v_subscriberLockAccess(s);
    if (result == V_RESULT_OK) {
        if (s->presentation.access_scope == V_PRESENTATION_GROUP &&
            s->presentation.coherent_access) {
            if (s->accessCount > 0) { /* At least one begin_access one */
                OS_REPORT(OS_ERROR, "v_subscriberAddReader", V_RESULT_INTERNAL_ERROR,
                          "Reader <%s> could not be added to subscriber <%s>," OS_REPORT_NL
                          "modification not allowed with open begin_access",
                          v_entity(r)->name, v_entity(s)->name);
                result = V_RESULT_PRECONDITION_NOT_MET;
            } else {
                (void)c_setWalk(s->readers, (c_action)collectReaders, &groupReaders);
                v_transactionGroupAdminAddReader(s->transactionGroupAdmin, r);
                groupCoherent = TRUE;
            }
        }
    }

    if (result == V_RESULT_OK) {
        v_partitionAdminWalk(s->partitions,collectPartitions,iter);
        found = c_setInsert(s->readers,r);
        if (found != r) {
            result = V_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR,
                        "v_subscriberAddReader", V_RESULT_PRECONDITION_NOT_MET,
                        "shared <%s> name already defined",
                        r->qos->share.v.name);
        }
    }
    v_subscriberUnlock(s);

    if (groupCoherent) {
        /* Call kernel beginAccess so that the alignment data doesn't change */
        (void)v_kernelGroupTransactionBeginAccess(kernel);
    }

    while ((result == V_RESULT_OK) &&  ((d = c_iterTakeFirst(iter)) != NULL)) {
        result = v_readerSubscribe(r,d);
        c_free(d);
    }

    if (groupCoherent) {
        while ((found = c_iterTakeFirst(groupReaders)) != NULL) {
            v_readerGetHistoricalData(found);
            c_free(found);
        }
        v_kernelGroupTransactionEndAccess(kernel);
        v_transactionGroupAdminTrigger(s->transactionGroupAdmin, NULL);
    }
    v_subscriberLock(s);
    v_subscriberUnlockAccess(s);
    v_subscriberUnlock(s);

    c_iterFree(groupReaders);
    c_iterFree(iter);

    return result;
}

void
v_subscriberRemoveReader(
    v_subscriber s,
    v_reader r)
{
    v_reader found;
    v_partition d;
    c_iter iter;
    c_bool groupCoherent = FALSE;

    assert(s != NULL);
    assert(r != NULL);

    iter = c_iterNew(NULL);

    /* Prevent inappropriate  notifications to Subscriber caused by the reception
     * of unregister messages originating from the disconnection of the group.
     * For this reason we set the V_EVENT_PREPARE_DELETE flag, so that it is
     * clear that this reader should not bother to notify the Subscriber of
     * its state changes.
     */
    v_observerSetEvent(v_observer(r), V_EVENT_PREPARE_DELETE);

    v_subscriberLock(s);
    subscriberLockAccessIgnoreAccessCount(s);
    if (s->presentation.access_scope == V_PRESENTATION_GROUP &&
        s->presentation.coherent_access) {
        groupCoherent = TRUE;
        v_transactionGroupAdminRemoveReader(s->transactionGroupAdmin, r);
    }
    found = c_remove(s->readers,r,NULL,NULL);
    v_partitionAdminWalk(s->partitions,collectPartitions,iter);
    v_subscriberUnlock(s);
    if (groupCoherent) {
        v_transactionGroupAdminTrigger(s->transactionGroupAdmin, NULL);
    }
    v_subscriberLock(s);
    v_subscriberUnlockAccess(s);
    v_subscriberUnlock(s);
    while ((d = c_iterTakeFirst(iter)) != NULL) {
        (void)v_readerUnSubscribe(r,d);
        c_free(d);
    }
    c_iterFree(iter);
    c_free(found);
}

void
v_subscriberWalkReaders(
    v_subscriber _this,
    c_bool (*action)(v_reader reader, c_voidp arg),
    c_voidp arg)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_subscriber));

    v_subscriberLock(_this);
    (void)c_setWalk(_this->readers,(c_action)action,arg);
    v_subscriberUnlock(_this);
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
v_subscriberRemoveShareUnsafe(
    v_subscriber _this,
    v_reader reader)
{
    v_reader found;

    found = c_remove(_this->shares,reader,NULL,NULL);

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
    v_partitionPolicyI old;

    assert(s != NULL);

    arg.removedPartitions = NULL;

    v_subscriberLock(s);
    arg.addedPartitions = v_partitionAdminAdd(s->partitions, partitionExpr);
    old = s->qos->partition;
    s->qos->partition = v_partitionPolicyAdd(old, partitionExpr,
                                             c_getBase(c_object(s)));
    c_free(old.v);

    (void)c_setWalk(s->readers, qosChangedAction, &arg);
    d = v_partition(c_iterTakeFirst(arg.addedPartitions));
    while (d != NULL) {
        c_free(d);
        d = v_partition(c_iterTakeFirst(arg.addedPartitions));
    }
    c_iterFree(arg.addedPartitions);
    v_subscriberUnlock(s);
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
    assert(C_TYPECHECK(s,v_subscriber));

    arg.list = NULL;
    arg.topicName = topicName;

    v_subscriberLock(s);
    (void)c_setWalk(s->readers, (c_action)lookupReaderByTopic, &arg);
    v_subscriberUnlock(s);

    return arg.list;
}

c_iter
v_subscriberLookupPartitions(
    v_subscriber s,
    const c_char *partitionExpr)
{
    c_iter list;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_subscriber));

    v_subscriberLock(s);
    list = v_partitionAdminLookup(s->partitions, partitionExpr);
    v_subscriberUnlock(s);

    return list;
}

v_subscriberQos
v_subscriberGetQos(
    v_subscriber _this)
{
    v_subscriberQos qos;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_subscriber));

    v_subscriberLock(_this);
    qos = c_keep(_this->qos);
    v_subscriberUnlock(_this);

    return qos;
}

v_result
v_subscriberSetQos(
    v_subscriber s,
    v_subscriberQos tmpl)
{
    v_result result;
    v_kernel kernel;
    v_subscriberQos qos;
    v_qosChangeMask cm;
    v_dataReaderConnectionChanges arg;
    v_partition d;
    v_accessMode access;
    v_message builtinCMMsg;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_subscriber));

    arg.addedPartitions = NULL;
    arg.removedPartitions = NULL;

    result = v_subscriberQosCheck(tmpl);
    if (result == V_RESULT_OK) {
        v_subscriberLock(s);
        kernel = v_objectKernel(s);
        qos = v_subscriberQosNew(kernel, tmpl);
        if (!qos) {
            v_subscriberUnlock(s);
            return V_RESULT_OUT_OF_MEMORY;
        }

        /* ES, dds1576: When the QoS is being set we have to verify that the partitions
         * listed in the qos do not have an invalid access mode set. For a subscriber
         * an invalid mode is characterized as 'write' mode. If the partitionAccess
         * check returns ok, then everything continue as normal. If the
         * partitionAccess check returns something else, then the setQos operation
         * is aborted. Please see the partitionAccess check operation to know when
         * a partition expression is not allowed.
         */
        if (qos && qos->partition.v) {
            access = v_kernelPartitionAccessMode(kernel, qos->partition);
        } else {
            access = V_ACCESS_MODE_READ_WRITE;/* default */
        }
        if (access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_READ)
        {
            result = v_subscriberQosCompare(s->qos, qos, v_entityEnabled(v_entity(s)), &cm);
            if ((result == V_RESULT_OK) && (cm != 0)) {
                if (v_entity(s)->enabled) {

                    builtinCMMsg = v_builtinCreateCMSubscriberInfo (kernel->builtin, s);
                } else {
                    builtinCMMsg = NULL;
                }

                c_free(s->qos);
                s->qos = c_keep(qos);

                if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
                    v_partitionAdminSet(s->partitions,
                                        s->qos->partition,
                                        &arg.addedPartitions,
                                        &arg.removedPartitions);
                }

                (void)c_setWalk(s->readers, qosChangedAction, &arg);

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
                if (builtinCMMsg) {
                    v_writeBuiltinTopic(kernel, V_CMSUBSCRIBERINFO_ID, builtinCMMsg);
                    c_free(builtinCMMsg);
                }
            }
        } else {
            result = V_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR, "v_subscriberSetQos", result,
                      "Precondition not met: invalid Partition access mode (%d)",
                      access);
        }
        v_subscriberUnlock(s);
        c_free(qos);
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

c_bool
v_subscriberConnectNewGroup(
    v_subscriber s,
    v_group g)
{
    c_bool result = TRUE;
    c_bool connect;
    c_iter addedPartitions;
    v_partition d;

    v_subscriberLock(s);
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

        /*
         * Before doing a walk over the Readers, raise the accessBusy flag
         * and unlock the Subscriber. The accessBusy flag prevents the reader
         * list from being modified during the walk, and the Subscriber lock
         * must be released to avoid cross-locking, since a reader lock may be
         * claimed before a Subscriber lock, but not after a Subscriber lock.
         */
        v_subscriberLockAccess(s);
        v_subscriberUnlock(s);
        result = c_setWalk(s->readers, (c_action)v_readerSubscribeGroup, g);

        /* After the walk, reset the accessBusy flag again. */
        v_subscriberLock(s);
        v_subscriberUnlockAccess(s);
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
        /*
         * Before doing a walk over the Readers, raise the accessBusy flag
         * and unlock the Subscriber. The accessBusy flag prevents the reader
         * list from being modified during the walk, and the Subscriber lock
         * must be released to avoid cross-locking, since a reader lock may be
         * claimed before a Subscriber lock, but not after a Subscriber lock.
         */
        v_subscriberLockAccess(s);
        v_subscriberUnlock(s);
        if (v_partitionAdminExists(s->partitions, v_partitionName(g->partition))) {
            (void)c_setWalk(s->readers, (c_action)notifyGroupQueues, g);
        }

        /* After the walk, reset the accessBusy flag again. */
        v_subscriberLock(s);
        v_subscriberUnlockAccess(s);
    }
    v_subscriberUnlock(s);

    return result;
}

void
v_subscriberNotifyDataAvailable(
    v_subscriber _this,
    v_event e)
{
    v_status status;
    C_STRUCT(v_event) event;

    OS_UNUSED_ARG(e);

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_subscriber));

    status = v_entityStatus(v_entity(_this));
    v_statusNotifyDataOnReaders(status);
    c_free(status);

    _TRACE_EVENTS_("v_subscriberNotifyDataAvailable: Throw ON_DATA_ON_READERS event on K_SUBSCRIBER(0x%x)\n", _this);

    event.kind = V_EVENT_ON_DATA_ON_READERS;
    event.source = v_observable(_this);
    event.data = NULL;

    v_observableNotify(v_observable(_this), &event);
}

v_transactionGroupAdmin
v_subscriberLookupTransactionGroupAdmin(
    v_subscriber _this)
{
    assert(_this);
    assert(C_TYPECHECK(_this,v_subscriber));

    return _this->transactionGroupAdmin;
}

v_presentationKind
v_subscriberAccessScope(
    v_subscriber _this)
{
    assert (_this != NULL && C_TYPECHECK (_this, v_subscriber));

    return _this->presentation.access_scope;
}

static c_bool updateReaderPurgeList(c_object o, c_voidp arg)
{
    v_dataReader dr = v_dataReader(o);

    OS_UNUSED_ARG(arg);

    v_dataReaderUpdatePurgeLists(dr);
    return TRUE;
}

/**
 * This operation is called to notify the subscriber that the user is going
 * to access DataReaders.
 * This operation will also flush any pending transactions that have become complete
 * before access starts and just in time.
 */
v_result
v_subscriberBeginAccess(
    v_subscriber _this)
{
    v_result result = V_RESULT_OK;
    c_bool flush = FALSE;
    v_transactionGroupAdmin transactionGroupAdmin;

    v_subscriberLock(_this);
    while (_this->accessBusy) {
        subscriberCondWait(_this);
    }
    if (_this->accessCount == 0) {
        if (_this->transactionGroupAdmin) {
            flush = _this->accessBusy = TRUE;
#ifndef NDEBUG
            _this->accessOwner = os_threadIdToInteger(os_threadIdSelf());
#endif
            transactionGroupAdmin = c_keep(_this->transactionGroupAdmin);
        }
    }
    _this->accessCount++;
    (void)c_setWalk(_this->readers, updateReaderPurgeList, NULL);
    v_subscriberUnlock(_this);

    if (flush) {
        if (transactionGroupAdmin) {
            v_transactionGroupAdminFlush(transactionGroupAdmin);
            c_free(transactionGroupAdmin);
        }
        v_subscriberLock(_this);
        _this->accessBusy = FALSE;
#ifndef NDEBUG
        _this->accessOwner = 0;
#endif
        subscriberCondBroadcast(_this);
        v_subscriberUnlock(_this);
    }

    return result;
}

/**
 * This operation is called to notify the subscriber that the user is ready
 * accessing the DataReaders.
 * This operation will also flush any pending transactions that have become complete
 * between begin_acces and end_access, which where added to the pending list because
 * they couldn't flushed during this period.
 */
v_result
v_subscriberEndAccess(
    v_subscriber _this)
{
    c_bool flush = FALSE;
    v_result result = V_RESULT_OK;
    c_iter list = NULL;
    v_dataReader reader;
    v_transactionGroupAdmin transactionGroupAdmin;

    v_subscriberLock(_this);
    if (_this->accessCount > 0) {
        _this->accessCount--;
        flush = (_this->accessCount == 0);
        if (flush) {
            v_orderedInstanceReset (_this->orderedInstance);
            (void)c_walk(_this->readers, collectReaders, &list);

            _this->accessBusy = TRUE; /* must set to hold off others until after the flush. */
#ifndef NDEBUG
            _this->accessOwner = os_threadIdToInteger(os_threadIdSelf());
#endif
            transactionGroupAdmin = c_keep(_this->transactionGroupAdmin);
        }
    } else {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    v_subscriberUnlock(_this);
    if (flush) {
        while ((reader = c_iterTakeFirst(list)) != NULL) {
            v_dataReaderFlushPending(reader);
            c_free(reader);
        }

        if (transactionGroupAdmin) {
            v_transactionGroupAdminTrigger(transactionGroupAdmin, NULL);
            c_free(transactionGroupAdmin);
        }

        v_subscriberLock(_this);
        _this->accessBusy = FALSE;
#ifndef NDEBUG
        _this->accessOwner = 0;
#endif
        subscriberCondBroadcast(_this);
        v_subscriberUnlock(_this);
    }
    c_iterFree(list);

    return result;
}

static c_bool
v__subscriberRequireAccessLock(
    v_subscriber _this)
{
    c_bool lock = FALSE;

    if (_this->presentation.access_scope == V_PRESENTATION_GROUP) {
        lock = (_this->presentation.coherent_access ||
                _this->presentation.ordered_access);
    }

    return lock;
}

c_bool
v__subscriberRequireAccessLockCoherent(
    v_subscriber _this)
{
    c_bool lock = FALSE;

    if (_this->presentation.access_scope == V_PRESENTATION_GROUP) {
        lock = _this->presentation.coherent_access;
    }

    return lock;
}

c_bool
v__subscriberRequireAccessLockOrdered(
    v_subscriber _this)
{
    c_bool lock = FALSE;

    if (_this->presentation.access_scope == V_PRESENTATION_GROUP) {
        /* coherent_access lock out ranks ordered_access
         * Coherence locks the complete set no need to lock for single samples.
         */
        if (_this->presentation.coherent_access == FALSE) {
            lock = _this->presentation.ordered_access;
        }
    }

    return lock;
}

static v_result
testBeginAccess(
    v_subscriber _this)
{
    v_result result = V_RESULT_OK;
    if (v__subscriberRequireAccessLock (_this)) {
        if (_this->accessCount == 0) {
            result = V_RESULT_PRECONDITION_NOT_MET;
        }
    }
    return result;
}

/**
 * This operation is called by read and take operations to requests access to the
 * history cache. This operation will verify if beginAccess is called and if set
 * grant access, otherwise it will return precondition not met.
 */
v_result
v_subscriberGetDataReaders(
    v_subscriber _this,
    v_sampleMask mask,
    v_dataReaderAction action,
    c_voidp actionArg)
{
    v_result result = V_RESULT_OK;
    v_dataReader reader;
    c_iter list = NULL;

    assert(_this);
    assert(C_TYPECHECK(_this,v_subscriber));

    v_subscriberLock(_this);
    result = testBeginAccess(_this);
    if (result == V_RESULT_OK) {
        if ((_this->presentation.access_scope == V_PRESENTATION_GROUP) &&
            (_this->presentation.ordered_access == TRUE))
        {
            list = v_orderedInstanceGetDataReaders(_this->orderedInstance, mask);
        } else {
            (void)c_setWalk(_this->readers, (c_action)collectReaders, &list);
        }
    }
    v_subscriberUnlock(_this);

    while ((reader = v_dataReader(c_iterTakeFirst(list)))) {
        if (v_dataReaderHasMatchingSamples(reader, mask)) {
            action(reader, actionArg);
        }
        c_free(reader);
    }
    c_iterFree(list);

    return result;
}

v_result
v_subscriberTestBeginAccess(
    v_subscriber _this)
{
    v_result result = V_RESULT_OK;

    assert (_this != NULL && C_TYPECHECK (_this, v_subscriber));

    v_subscriberLock(_this);
    result = testBeginAccess(_this);
    v_subscriberUnlock(_this);
    return result;
}

/* subscriberLock required before calling this function */
static void
subscriberLockAccessIgnoreAccessCount(
    v_subscriber _this)
{
    while (_this->accessBusy) {
        subscriberCondWait(_this);
    }
    _this->accessBusy = TRUE;
#ifndef NDEBUG
    _this->accessOwner = os_threadIdToInteger(os_threadIdSelf());
#endif
}

/* v_subscriberLock required before calling this function */
void
v_subscriberLockAccess(
    v_subscriber _this)
{
    while (_this->accessCount > 0 || _this->accessBusy) {
        subscriberCondWait(_this);
    }
    _this->accessBusy = TRUE;
#ifndef NDEBUG
    _this->accessOwner = os_threadIdToInteger(os_threadIdSelf());
#endif
}

/* v_subscriberLock required before calling this function */
c_bool
v_subscriberTryLockAccess(
    v_subscriber _this)
{
    c_bool result = FALSE;

    if (_this->accessCount > 0 || _this->accessBusy) {
        /* Unable to get lock */
    } else {
       result = _this->accessBusy = TRUE;
#ifndef NDEBUG
       _this->accessOwner = os_threadIdToInteger(os_threadIdSelf());
#endif
    }

    return result;
}

/* v_subscriberLock required before calling this function */
void
v_subscriberUnlockAccess(
    v_subscriber _this)
{
    if (_this->accessBusy) {
        _this->accessBusy = FALSE;
#ifndef NDEBUG
        _this->accessOwner = 0;
#endif
        subscriberCondBroadcast(_this);
    }
}

static void
subscriberTriggerGroupCoherentNoLock(
    v_subscriber _this,
    v_reader owner)
{
    c_bool accessLock;

    assert(_this->presentation.access_scope == V_PRESENTATION_GROUP);
    assert(_this->presentation.coherent_access == TRUE);

    accessLock = v_subscriberTryLockAccess(_this);
    if (accessLock) {
        v_subscriberUnlock(_this);
        v_transactionGroupAdminTrigger(_this->transactionGroupAdmin, owner);
        v_subscriberLock(_this);
        v_subscriberUnlockAccess(_this);
    }
}

void
v_subscriberTriggerGroupCoherent(
    v_subscriber _this,
    v_reader owner)
{
    v_subscriberLock(_this);
    subscriberTriggerGroupCoherentNoLock(_this, owner);
    v_subscriberUnlock(_this);
}

void
v_subscriberNotifyGroupCoherentPublication(
    v_subscriber _this,
    v_message msg)
{
    v_subscriberLock(_this);
    if ((_this->presentation.access_scope == V_PRESENTATION_GROUP) &&
        (_this->presentation.coherent_access == TRUE))
    {
        c_bool dispose = FALSE;
        struct v_publicationInfo *info = v_builtinPublicationInfoData(msg);
        if (v_stateTest(v_nodeState(msg), L_DISPOSED)) {
            dispose = TRUE;
        }
/* Need this check to avoid crashes until OSPL-7992 is solved. */
if (_this->transactionGroupAdmin &&  _this->participant) {
        if (v_transactionGroupAdminNotifyGroupCoherentPublication(_this->transactionGroupAdmin, NULL, dispose, info) == TRUE) {
            subscriberTriggerGroupCoherentNoLock(_this, NULL);
        }
}
    }

    v_subscriberUnlock(_this);
}

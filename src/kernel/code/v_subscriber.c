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
#include "v__entry.h"
#include "v__deliveryService.h"
#include "v_deliveryServiceEntry.h"
#include "v__builtin.h"
#include "v_group.h"
#include "v__observer.h"
#include "v__observable.h"
#include "v_listener.h"
#include "v_public.h"
#include "v_groupSet.h"
#include "v_dataReaderEntry.h"
#include "v_status.h"
#include "v_event.h"
#include "v__groupStream.h"
#include "v__groupQueue.h"
#include "v__networkReader.h"
#include "v_networkReaderEntry.h"
#include "v__dataReader.h"
#include "v_dataReaderInstance.h"
#include "v__policy.h"
#include "v__kernel.h"
#include "v__entity.h"
#include "v_partition.h"
#include "os_atomics.h"
#include "v__orderedInstance.h"

#if 0
#define _TRACE_EVENTS_ printf
#else
#define _TRACE_EVENTS_(...)
#endif

/* ----------------------------------- Private ------------------------------ */

static void
updateConnections(
    c_object o,
    c_iterActionArg arg)
{
    v_kernel kernel;
    v_group g;
    v_partition p;
    c_iterIter it;
    v_dataReaderConnectionChanges *a = (v_dataReaderConnectionChanges *)arg;
    v_dataReaderEntry e = v_dataReaderEntry(o);;

    kernel = v_objectKernel(v_object(e));
    it = c_iterIterGet(a->addedPartitions);
    while ((p = c_iterNext(&it)) != NULL) {
        g = v_groupSetCreate(kernel->groupSet,p,e->topic);
        if (v_groupAddEntry(g, v_entry(e))) {
            v_entryAddGroup(v_entry(e), g);
        }
    }
    it = c_iterIterGet(a->removedPartitions);
    while ((p = c_iterNext(&it)) != NULL) {
        g = v_groupSetCreate(kernel->groupSet,p,e->topic);
        v_groupRemoveEntry(g, v_entry(e));
        v_entryRemoveGroup(v_entry(e), g);
    }
}

/* ----------------------------------- Public ------------------------------- */

_Check_return_
_Ret_maybenull_
v_subscriber
v_subscriberNew(
    _In_ v_participant p,
    _In_opt_z_ const c_char *name,
    _In_opt_ v_subscriberQos qos)
{
    v_kernel kernel;
    v_subscriber s = NULL;
    v_subscriberQos q;
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
                v_entityInit(v_entity(s), name);
                s->qos = q;
                s->accessCount = 0;
                c_condInit(c_getBase(s), &s->cond, &v_observable(s)->mutex);

                if (q->share.v.enable) {
                    v_subscriber found = v_kernelAddSharedSubscriber(kernel,s);
                    if (found != s) {
                        /* Make sure to set the partition list to NULL, because
                         * v_publicFree will cause a crash in the v_subscriberDeinit
                         * otherwise.
                         */
                        s->partitions = NULL;
                        /*v_publicFree to free reference held by the handle server.*/
                        v_publicFree(v_public(s));
                        /*Now free the local reference as well.*/
                        c_free(s);
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

static void
v__subscriberWalkReaders_nl(
    v_subscriber _this,
    c_action action,
    c_voidp arg);

static c_bool
collect(
    _Inout_ c_object object,
    _Inout_ c_voidp arg);


static c_bool
initGroupCoherentReader(
    _Inout_ c_object object, /* v_reader */
    _Inout_ c_voidp arg /* v_transactionGroupAdmin */)
{
    v_reader r;
    v_transactionGroupAdmin a;

    assert(C_TYPECHECK(object,v_dataReader));
    assert(C_TYPECHECK(arg,v_transactionGroupAdmin));

    r = v_reader(object);
    a = v_transactionGroupAdmin(arg);

    v_readerAddTransactionAdmin(r, a);
    v_transactionGroupAdminAddReader(a, r);

    return TRUE;
}

struct order_policy {
    v_result result;
    int count;
    v_orderbyKind kind;
};

static c_bool
getOrderPolicy(
    _Inout_ c_object object, /* v_reader */
    _Inout_ c_voidp arg /* order_policy */)
{
    c_bool proceed = TRUE;
    v_reader r = v_reader(object);
    struct order_policy *policy = (struct order_policy *)arg;

    assert(C_TYPECHECK(r,v_dataReader));

    if (policy->count == 0) {
        policy->kind = r->qos->orderby.v.kind;
    } else if (policy->kind != r->qos->orderby.v.kind) {
        policy->result = V_RESULT_INCONSISTENT_QOS;
        proceed = FALSE;
    }
    policy->count++;

    return proceed;
}

v_result
v__subscriberEnable (
    _Inout_ v_subscriber _this)
{
    v_kernel kernel;
    v_message builtinCMMsg;
    c_iter list;
    c_char *partitionName;
    c_bool groupCoherent = FALSE;
    c_iter readers = NULL;
    v_entity reader;

    OSPL_LOCK(_this);
    kernel = v_objectKernel(_this);
    if (_this->qos->presentation.v.ordered_access) {
        assert(_this->orderedInstance == NULL);
        if (_this->qos->presentation.v.access_scope == V_PRESENTATION_GROUP &&
            _this->qos->presentation.v.coherent_access &&
            _this->qos->entityFactory.v.autoenable_created_entities)
        {
            struct order_policy policy;
            policy.result = V_RESULT_OK;
            policy.count = 0;
            policy.kind = V_ORDERBY_RECEPTIONTIME;
            v__subscriberWalkReaders_nl(_this, &getOrderPolicy, &policy);
            if (policy.result != V_RESULT_OK) return policy.result;
            _this->orderby = policy.kind;
            _this->orderedInstance = v_orderedInstanceNew (v_entity(_this), _this->qos->presentation.v.access_scope, _this->orderby);
        }
    }

    if (_this->qos->partition.v != NULL) {
        list = v_partitionPolicySplit(_this->qos->partition);
        while((partitionName = c_iterTakeFirst(list)) != NULL) {
            v_partitionAdminFill(_this->partitions, partitionName);
            os_free(partitionName);
        }
        c_iterFree(list);
    }

    if (_this->qos->presentation.v.access_scope == V_PRESENTATION_GROUP && _this->qos->presentation.v.coherent_access)
    {
        groupCoherent = TRUE;
        _this->transactionGroupAdmin = v_transactionGroupAdminNew(v_object(_this));
        v__subscriberWalkReaders_nl(_this, &initGroupCoherentReader, _this->transactionGroupAdmin);
    }

    if(_this->qos->entityFactory.v.autoenable_created_entities || groupCoherent) {
        v__subscriberWalkReaders_nl(_this, &collect, &readers);
    }

    /* The following operation makes the subscriber sensitive to ON_DATA_ON_READER events. */
    v_observerSetEvent(v_observer(_this), V_EVENT_ON_DATA_ON_READERS);

    OSPL_UNLOCK(_this);

    /* Call kernel beginAccess so that the alignment data doesn't change.
     * This needs to be maintained until all readers are enabled and have
     * retrieved their historical data.
     */
    if (groupCoherent) {
        v_kernelGroupTransactionBeginAccess(kernel);
    }

    while((reader = c_iterTakeFirst(readers)) != NULL) {
        (void)v_entityEnable(reader);
        c_free(reader);
    }
    c_iterFree(readers);

    if (groupCoherent) {
        v_kernelGroupTransactionEndAccess(kernel);
    }

    builtinCMMsg = v_builtinCreateCMSubscriberInfo(kernel->builtin, _this);
    v_writeBuiltinTopic(kernel, V_CMSUBSCRIBERINFO_ID, builtinCMMsg);
    c_free(builtinCMMsg);

    return V_RESULT_OK;
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

    kernel = v_objectKernel(s);
    if (s->qos->share.v.enable) {
        if (v_kernelRemoveSharedSubscriber(kernel,s) > 0) {
            return;
        }
    }

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
collect(
    _Inout_ c_object object,
    _Inout_ c_voidp arg)
{
    c_iter *list = (c_iter *) arg;
    *list = c_iterInsert(*list, c_keep(object));
    return TRUE;
}

v_result
v_subscriberAddReader(
    _Inout_ v_subscriber s,
    _Inout_ v_reader r)
{
    v_result result;
    v_reader found;

    OSPL_LOCK(s);
    found = c_setInsert(s->readers, r);
    if (found != r) {
        result = V_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, OS_FUNCTION, result, "shared <%s> name already defined", r->qos->share.v.name);
        goto err;
    }

    if(!v__entityDisabled_nl(v_entity(s))){
        if (s->qos->presentation.v.access_scope == V_PRESENTATION_GROUP && s->qos->presentation.v.coherent_access) {
            /* No mutations to an already enabled subscriber are allowed. */
            result = V_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR, OS_FUNCTION, result,
                      "Reader <%s> could not be added to subscriber <%s>," OS_REPORT_NL
                      "adding a reader to a GROUP-coherent subscriber is not allowed after the subscriber has been enabled.",
                      v_entity(r)->name, v_entity(s)->name);
            goto err_immutable;
        }
    }

    OSPL_UNLOCK(s);
    return V_RESULT_OK;

err_immutable:
    found = c_remove(s->readers, r, NULL, NULL);
    c_free(found);
err:
    OSPL_UNLOCK(s);
    return result;
}

static void
connectPartition(
    c_object o,
    c_iterActionArg arg)
{
    v_kernel kernel;
    v_topic topic = NULL;
    v_group g;
    v_partition d = (v_partition)arg;
    v_entry e = v_entry(o);;

    kernel = v_objectKernel(v_object(e));
    switch (v_objectKind(v_entry(e)->reader)) {
    case K_DATAREADER:
        topic = v_dataReaderEntry(e)->topic;
    break;
    case K_DELIVERYSERVICE:
        topic = v_deliveryServiceEntry(e)->topic;
    break;
    default: assert(0); break;
    }
    g = v_groupSetCreate(kernel->groupSet,d,topic);
    if (v_groupAddEntry(g, v_entry(e))) {
        v_entryAddGroup(v_entry(e), g);
    }
}

v_result
v_subscriberEnableReader(
    _Inout_ v_subscriber s,
    _Inout_ v_reader r)
{
    v_result result = V_RESULT_OK;
    v_partition d;
    c_iter iter = NULL;

    OSPL_LOCK(s);
    if ((v_objectKind(v_entity(r)) == K_DATAREADER) &&
        (s->qos->presentation.v.ordered_access)) {
        if (s->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) {
            if (s->orderedInstance == NULL) {
                s->orderby = r->qos->orderby.v.kind;
                s->orderedInstance = v_orderedInstanceNew (v_entity(s), s->qos->presentation.v.access_scope, s->orderby);
                v_dataReader(r)->orderedInstance = c_keep (s->orderedInstance);
            } else if (s->orderby == r->qos->orderby.v.kind) {
                v_dataReader(r)->orderedInstance = c_keep (s->orderedInstance);
            } else {
                result = V_RESULT_PRECONDITION_NOT_MET;
            }
        } else {
            assert (s->orderedInstance == NULL);
            assert (v_dataReader(r)->orderedInstance == NULL);
            v_dataReader(r)->orderedInstance = v_orderedInstanceNew (v_entity(r),
                                                                     s->qos->presentation.v.access_scope,
                                                                     r->qos->orderby.v.kind);
        }
    }
    if (result == V_RESULT_OK) {
        c_iter entries;
        v_entry e;
        /* Subscribe reader to all partitions of the subscriber */
        v_partitionAdminWalk(s->partitions, &collect, &iter);
        OSPL_UNLOCK(s);

        /* Enable the reader: data may be delivered during the next step and events raised potentially
         * trigger listeners, which would be useless on a disabled reader */
        v_entity(r)->state = V_ENTITYSTATE_ENABLED;

        entries = v_readerCollectEntries(r);
        while ((d = c_iterTakeFirst(iter)) != NULL) {
            c_iterWalk(entries, connectPartition, d);
            c_free(d);
        }
        while ((e = c_iterTakeFirst(entries)) != NULL) {
            c_free(e);
        }
        c_iterFree(entries);
        c_iterFree(iter);
    } else {
        OSPL_UNLOCK(s);
    }
    return result;
}

static void
unsubscribePartition(
    c_object o,
    c_voidp arg)
{
    v_partition p = v_partition(arg);
    v_kind kind = v_objectKind(o);

    assert(C_TYPECHECK(o,v_entry));
    assert(C_TYPECHECK(p,v_partition));

    if (kind == K_NETWORKREADERENTRY) {
        if (v_group(v_networkReaderEntry(o)->group)->partition == p) {
            v_groupRemoveEntry(v_networkReaderEntry(o)->group, v_entry(o));
            v_entryRemoveGroup(v_entry(o), v_networkReaderEntry(o)->group);
        }
    } else {
        v_kernel kernel;
        v_group g;
        c_value params[2];
        c_iter list;

        params[0] = c_objectValue(p);
        switch (v_objectKind(o)) {
        case K_DATAREADERENTRY: params[1] = c_objectValue(v_dataReaderEntry(o)->topic); break;
        case K_DELIVERYSERVICEENTRY: params[1] = c_objectValue(v_deliveryServiceEntry(o)->topic); break;
        default: assert(0); break;
        }
        kernel = v_objectKernel(o);
        list = v_groupSetSelect(kernel->groupSet, "partition = %0 and topic = %1", params);
        while ((g = c_iterTakeFirst(list)) != NULL) {
            v_groupRemoveEntry(g,v_entry(o));
            v_entryRemoveGroup(v_entry(o), g);
            c_free(g);
        }
        c_iterFree(list);
    }
}

void
v_subscriberRemoveReader(
    v_subscriber s,
    v_reader r)
{
    v_reader found;
    v_partition d;
    c_iter iter = NULL;

    assert(s != NULL);
    assert(r != NULL);

    /* Prevent inappropriate  notifications to Subscriber caused by the reception
     * of unregister messages originating from the disconnection of the group.
     * For this reason we set the V_EVENT_PREPARE_DELETE flag, so that it is
     * clear that this reader should not bother to notify the Subscriber of
     * its state changes.
     */
    v_observerSetEvent(v_observer(r), V_EVENT_PREPARE_DELETE);

    OSPL_LOCK(s);
    if (s->transactionGroupAdmin) {
        v_transactionGroupAdminRemoveReader(s->transactionGroupAdmin, r);
    }
    found = c_remove(s->readers,r,NULL,NULL);
    if(v__entityEnabled_nl(v_entity(s))) {
        v_partitionAdminWalk(s->partitions, &collect, &iter);

        if ((s->transactionGroupAdmin) && (s->accessCount == 0)) {
            /* Removal of a reader could lead to new complete transactions */
            v_transactionGroupAdminFlushPending(s->transactionGroupAdmin, NULL);
        }
        OSPL_UNLOCK(s);
        switch(v_objectKind(r)) {
        case K_DATAREADER:
        case K_DELIVERYSERVICE:
        case K_NETWORKREADER:
        {
            c_iter entries;
            v_entry e;
            entries = v_readerCollectEntries(r);
            while ((d = c_iterTakeFirst(iter)) != NULL) {
                c_iterWalk(entries, unsubscribePartition, d);
                c_free(d);
            }
            while ((e = c_iterTakeFirst(entries)) != NULL) {
                c_free(e);
            }
            c_iterFree(entries);
        }
        break;
        case K_GROUPQUEUE:
            while ((d = c_iterTakeFirst(iter)) != NULL) {
                (void) v_groupStreamUnSubscribe(v_groupStream(r),d);
                c_free(d);
            }
        break;
        default:
            OS_REPORT(OS_CRITICAL,"v_subscriberRemoveReader failed",V_RESULT_ILL_PARAM,
                      "illegal reader kind (%d) specified", v_objectKind(r));
            assert(FALSE);
        break;
        }
        c_iterFree(iter);
    } else {
        OSPL_UNLOCK(s);
    }
    c_free(found);
}

static void
v__subscriberWalkReaders_nl(
    v_subscriber _this,
    c_action action,
    c_voidp arg)
{
    assert(C_TYPECHECK(_this,v_subscriber));

    (void)c_setWalk(_this->readers, action, arg);
}

void
v_subscriberWalkReaders(
    v_subscriber _this,
    c_action action,
    c_voidp arg)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_subscriber));

    OSPL_LOCK(_this);
    v__subscriberWalkReaders_nl(_this, action, arg);
    OSPL_UNLOCK(_this);
}

v_dataReader
v_subscriberAddShare(
    _Inout_ v_subscriber _this,
    _In_ v_dataReader reader)
{
    v_dataReader found;

    OSPL_LOCK(_this);
    found = c_tableInsert(_this->shares,reader);
    if (found != reader) {
        found->shareCount++;
    }
    OSPL_UNLOCK(_this);

    return found;
}

c_ulong
v_subscriberRemoveShare(
    v_subscriber _this,
    v_dataReader reader)
{
    c_ulong count;
    v_dataReader found;

    OSPL_LOCK(_this);
    count = --reader->shareCount;
    if (count == 0){
        found = c_remove(_this->shares,reader,NULL,NULL);
        c_free(found);
    }
    OSPL_UNLOCK(_this);

    return count;
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

    OSPL_LOCK(s);
    (void)c_setWalk(s->readers, (c_action)lookupReaderByTopic, &arg);
    OSPL_UNLOCK(s);

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

    OSPL_LOCK(s);
    list = v_partitionAdminLookup(s->partitions, partitionExpr);
    OSPL_UNLOCK(s);

    return list;
}

v_subscriberQos
v_subscriberGetQos(
    v_subscriber _this)
{
    v_subscriberQos qos;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_subscriber));

    OSPL_LOCK(_this);
    qos = c_keep(_this->qos);
    OSPL_UNLOCK(_this);

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
    v_message builtinCMMsg = NULL;

    assert(s != NULL);
    assert(C_TYPECHECK(s,v_subscriber));

    arg.addedPartitions = NULL;
    arg.removedPartitions = NULL;

    result = v_subscriberQosCheck(tmpl);
    if (result == V_RESULT_OK) {
        kernel = v_objectKernel(s);
        qos = v_subscriberQosNew(kernel, tmpl);
        if (!qos) {
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
            c_iter readers = NULL;
            OSPL_LOCK(s);
            result = v_subscriberQosCompare(s->qos, qos, !v__entityDisabled_nl(v_entity(s)), &cm);
            if ((result == V_RESULT_OK) && (cm != 0)) {
                if(v__entityEnabled_nl(v_entity(s))) {
                    builtinCMMsg = v_builtinCreateCMSubscriberInfo (kernel->builtin, s);
                }
                c_free(s->qos);
                s->qos = c_keep(qos);
                if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
                    v_partitionAdminUpdate(s->partitions,
                                           s->qos->partition,
                                           &arg.addedPartitions,
                                           &arg.removedPartitions);
                }
                v__subscriberWalkReaders_nl(s, &collect, &readers);
            }
            OSPL_UNLOCK(s);
            if ((result == V_RESULT_OK) && (cm != 0)) {
                v_reader r;
                while ((r = c_iterTakeFirst(readers)) != NULL) {
                    v_entry e;
                    c_iter entries = v_readerCollectEntries(r);
                    c_iterWalk(entries, updateConnections, &arg);
                    v_readerPublishBuiltinInfo(r);
                    while ((e = c_iterTakeFirst(entries)) != NULL) {
                        c_free(e);
                    }
                    c_iterFree(entries);
                }
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
        c_free(qos);
    }

    return result;
}

c_bool
v_subscriberConnectNewGroup(
    v_subscriber s,
    v_group g)
{
    c_bool result = TRUE;
    c_bool connect = TRUE;
    c_iter readers = NULL;
    v_reader r;

    OSPL_LOCK(s);
    v__subscriberWalkReaders_nl(s, &collect, &readers);
    connect = v_partitionAdminAdd(s->partitions, g->partition);
    OSPL_UNLOCK(s);

    if (connect) {
        while ((r = c_iterTakeFirst(readers)) != NULL) {
            if(v_objectKind(r) == K_GROUPQUEUE){
                v_groupStreamConnectNewGroups(v_groupStream(r), g);
            } else {
                c_iter entries;
                v_entry e;
                c_bool match;
                entries = v_readerCollectEntries(r);
                while ((e = c_iterTakeFirst(entries)) != NULL) {
                    switch (v_objectKind(e)) {
                    case K_DATAREADERENTRY:
                        match = (v_dataReaderEntry(e)->topic == g->topic);
                    break;
                    case K_DELIVERYSERVICEENTRY:
                        match = (v_deliveryServiceEntry(e)->topic == g->topic);
                    break;
                    case K_NETWORKREADERENTRY:
                        match = TRUE;
                    break;
                    default: match = FALSE; /* assert(0); */ break;
                    }
                    if (match) {
                        if (v_groupAddEntry(g, v_entry(e))) {
                            v_entryAddGroup(v_entry(e), g);
                        }
                    }
                    c_free(e);
                }
                c_iterFree(entries);
            }
            c_free(r);
        }
    } else {
        /* Check if group fits interest. This extra steps are needed because
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
        while ((r = c_iterTakeFirst(readers)) != NULL) {
            if(v_objectKind(r) == K_GROUPQUEUE){
                v_groupStreamConnectNewGroups(v_groupStream(r), g);
            }
            c_free(r);
        }
    }
    c_iterFree(readers);
    return result;
}

void
v_subscriberNotifyDataAvailable(
    v_subscriber _this,
    v_event e)
{
    C_STRUCT(v_event) event;

    OS_UNUSED_ARG(e);

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_subscriber));

    OSPL_ASSERT_LOCK(_this);

    _TRACE_EVENTS_("v_subscriberNotifyDataAvailable: Throw ON_DATA_ON_READERS event on K_SUBSCRIBER(0x%x)\n", _this);

    /* Create and throw on data on readers event. */
    event.kind = V_EVENT_ON_DATA_ON_READERS;
    event.source = v_observable(_this);
    event.data = NULL;
    OSPL_TRIGGER_EVENT(_this, &event, NULL);
    OSPL_THROW_EVENT(_this, &event);
}

void
v_subscriberNotify(
    v_subscriber _this,
    v_event event,
    c_voidp userData)
{
    v_status status;
    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_subscriber));

    OS_UNUSED_ARG(userData);
    OS_UNUSED_ARG(event);

    status = v_entityStatus(v_entity(_this));
    v_statusNotifyDataOnReaders(status);
    c_free(status);
}

static c_bool readerBeginAccess(c_object o, c_voidp arg)
{
    v_dataReader dr = v_dataReader(o);

    OS_UNUSED_ARG(arg);

    v_dataReaderBeginAccess(dr);
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

    OSPL_LOCK(_this);
    if(!v__entityEnabled_nl(v_entity(_this))) {
        OSPL_UNLOCK(_this);
        return V_RESULT_NOT_ENABLED;
    }
    if (++_this->accessCount == 1) {
        (void)c_setWalk(_this->readers, readerBeginAccess, NULL);
        if (_this->transactionGroupAdmin) {
            v_transactionGroupAdminFlush(_this->transactionGroupAdmin);
        }
    }
    OSPL_UNLOCK(_this);

    return result;
}

static c_bool
readerEndAccess(
    _Inout_ c_object object,
    _Inout_ c_voidp arg)
{
    OS_UNUSED_ARG(arg);
    v_dataReaderEndAccess(v_dataReader(object));
    return TRUE;
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
    v_result result = V_RESULT_OK;

    OSPL_LOCK(_this);
    if(!v__entityEnabled_nl(v_entity(_this))) {
        OSPL_UNLOCK(_this);
        OS_REPORT(OS_WARNING, "v_subscriberEndAccess", V_RESULT_NOT_ENABLED,
                  "Precondition not met: Subscriber not enabled");
        return V_RESULT_NOT_ENABLED;
    }
    if (_this->accessCount > 0) {
        if (--_this->accessCount == 0) {
            v_orderedInstanceReset (_this->orderedInstance);
            (void)c_walk(_this->readers, readerEndAccess, NULL);
            if (_this->transactionGroupAdmin) {
                v_transactionGroupAdminFlush(_this->transactionGroupAdmin);
            }
        }
    } else {
        OS_REPORT(OS_ERROR, "v_subscriberEndAccess", V_RESULT_NOT_ENABLED,
                  "Precondition not met: No Begin Access");
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    OSPL_UNLOCK(_this);

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

    OSPL_LOCK(_this);
    if (_this->accessCount == 0 &&
        (_this->qos->presentation.v.coherent_access ||
         _this->qos->presentation.v.ordered_access))
    {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    if (result == V_RESULT_OK) {
        if ((_this->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) &&
            (_this->qos->presentation.v.ordered_access == TRUE))
        {
            list = v_orderedInstanceGetDataReaders(_this->orderedInstance, mask);
        } else {
            (void)c_setWalk(_this->readers, &collect, &list);
        }
    }
    OSPL_UNLOCK(_this);

    while ((reader = v_dataReader(c_iterTakeFirst(list)))) {
        if (v_dataReaderHasMatchingSamples(reader, mask)) {
            action(reader, actionArg);
        }
        c_free(reader);
    }
    c_iterFree(list);

    return result;
}

void
v_subscriberGroupTransactionFlush(
    v_subscriber _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_subscriber));

    OSPL_LOCK(_this);
    if (v__entityEnabled_nl(v_entity(_this)) && v_subscriberQosIsGroupCoherent(_this->qos)) {
        if (_this->accessCount == 0) {
            v_transactionGroupAdminFlushPending(_this->transactionGroupAdmin, NULL);
        }
    }
    OSPL_UNLOCK(_this);
}

void
v_subscriberNotifyGroupCoherentPublication(
    v_subscriber _this,
    v_message msg)
{
    OSPL_LOCK(_this);
    if ((_this->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) &&
        (_this->qos->presentation.v.coherent_access == TRUE))
    {
        c_bool dispose = FALSE;
        struct v_publicationInfo *info = v_builtinPublicationInfoData(msg);
        if (v_stateTest(v_nodeState(msg), L_DISPOSED)) {
            dispose = TRUE;
        }
        /* Need this check to avoid crashes until OSPL-7992 is solved. */
        if (_this->transactionGroupAdmin &&  _this->participant) {
            if (v_transactionGroupAdminNotifyPublication(_this->transactionGroupAdmin, NULL, dispose, info) == TRUE) {
                /* Receiving a DCPSPublication could lead to a complete group transaction */
                if (_this->accessCount == 0) {
                    v_transactionGroupAdminFlush(_this->transactionGroupAdmin);
                }
            }
        }
    }
    OSPL_UNLOCK(_this);
}

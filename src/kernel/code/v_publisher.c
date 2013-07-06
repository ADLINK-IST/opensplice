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

#include "v__publisher.h"
#include "v__publisherQos.h"
#include "v_participant.h"
#include "v__partition.h"
#include "v__partitionAdmin.h"
#include "v__writer.h"
#include "v__kernel.h"
#include "v_group.h"
#include "v_observable.h"
#include "v__observer.h"
#include "v_public.h"
#include "v__policy.h"
#include "v_event.h"
#include "v_proxy.h"
#include "v_time.h"
#include "v_partition.h"

#include "q_expr.h"

#include "os_report.h"
#include "os_heap.h"


/**************************************************************
 * Private functions
 **************************************************************/

static c_bool
qosChangedAction(
    c_object o,
    c_voidp arg)
{
    v_writer w = v_writer(o);

    v_writerNotifyChangedQos(w, (v_writerNotifyChangedQosArg *)arg);

    return TRUE;
}

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_publisher
v_publisherNew(
    v_participant participant,
    const c_char *name,
    v_publisherQos qos,
    c_bool enable)
{
    v_kernel kernel;
    v_publisher p;
    v_publisherQos q;
    v_accessMode access;

    assert(C_TYPECHECK(participant,v_participant));

    kernel = v_objectKernel(participant);
    /* ES, dds1576: If a partition policy was provided then we need to verify
     * if the partition policy does not contain any partition expressions for
     * which write access is not allowed.
     * If write access is not allowed for one of the partitions listed in the
     * partition policy of the qos, then the publisher will not be created at
     * all.
     */
    if(qos && qos->partition)
    {
        access = v_kernelPartitionAccessMode(kernel, qos->partition);
    } else
    {
        access = V_ACCESS_MODE_READ_WRITE;/* default */
    }
    if(access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_WRITE)
    {
        q = v_publisherQosNew(kernel,qos);
        if (q != NULL) {
            p = v_publisher(v_objectNew(kernel,K_PUBLISHER));
            v_observerInit(v_observer(p),name,NULL,enable);
            p->partitions  = v_partitionAdminNew(kernel);
            p->writers     = c_setNew(v_kernelType(kernel,K_WRITER));
            p->qos         = q;
            p->suspendTime = C_TIME_INFINITE;
            p->participant = participant;
            p->transactionId = 0;
            c_lockInit(&p->lock,SHARED_LOCK);
            v_participantAdd(participant,v_entity(p));
            assert(c_refCount(p) == 3);  /* as handle, by participant and the local variable p */
            if (enable) {
                v_publisherEnable(p);
            }
        } else {
            OS_REPORT(OS_ERROR, "v_publisherNew", 0,
                      "Publisher not created: inconsistent qos");
            p = NULL;
        }
    } else
    {
        OS_REPORT(OS_ERROR,
              "v_publisherNew", 0,
              "Publisher not created: Access rights for one of the partitions listed in the partition list was not sufficient (i.e. write or readwrite).");
        p = NULL;
    }
    return p;
}

v_result
v_publisherEnable(
    v_publisher _this)
{
    v_kernel kernel;
    c_iter list;
    c_char *partitionName;
    v_result result = V_RESULT_ILL_PARAM;

    if (_this) {
        kernel = v_objectKernel(_this);
        v_observableAddObserver(v_observable(kernel->groupSet), v_observer(_this), NULL);

        if (_this->qos->partition != NULL) {
            list = v_partitionPolicySplit(_this->qos->partition);
            while((partitionName = c_iterTakeFirst(list)) != NULL) {
                v_publisherPublish(_this,partitionName);
                os_free(partitionName);
            }
            c_iterFree(list);
        }
        result = V_RESULT_OK;
    }
    return result;
}

void
v_publisherFree(
    v_publisher p)
{
    v_writer o;
    v_participant participant;
    v_kernel kernel;

    assert(C_TYPECHECK(p,v_publisher));

    kernel = v_objectKernel(p);
    v_observableRemoveObserver(v_observable(kernel->groupSet),v_observer(p), NULL);

    while ((o = c_take(p->writers)) != NULL) {
        /* remove writer from all partitions */
        v_writerFree(o);
        c_free(o);
    }
    participant = v_participant(p->participant);
    if (participant != NULL) {
        v_participantRemove(participant,v_entity(p));
        p->participant = NULL;
    }
    v_observerFree(v_observer(p));
}

void
v_publisherDeinit(
    v_publisher p)
{
    assert(C_TYPECHECK(p,v_publisher));

    v_observerDeinit(v_observer(p));
}

/**************************************************************
 * Protected functions
 **************************************************************/
v_publisherQos
v_publisherGetQosRef(
    v_publisher p)
{
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    return p->qos;  /* the reference is read only! */
}


v_result
v_publisherSetQos(
    v_publisher p,
    v_publisherQos qos)
{
    v_result result;
    v_qosChangeMask cm;
    v_writerNotifyChangedQosArg arg;
    v_partition d;
    v_accessMode access;
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    arg.addedPartitions = NULL;
    arg.removedPartitions = NULL;

    c_lockWrite(&p->lock);

    /* ES, dds1576: When the QoS is being set we have to verify that the partitions
     * listed in the qos do not have an invalid access mode set. For a publisher
     * an invalid mode is characterized as 'read' mode. If the partitionAccess
     * check returns ok, then everything continue as normal. If the
     * partitionAccess check returns something else, then the setQos operation
     * is aborted. Please see the partitionAccess check operation to know when
     * a partition expression is not allowed.
     */
    if(qos && qos->partition)
    {
        access = v_kernelPartitionAccessMode(v_objectKernel(p), qos->partition);
    } else
    {
        access = V_ACCESS_MODE_READ_WRITE;/* default */
    }
    if(access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_WRITE)
    {
        result = v_publisherQosSet( p->qos, qos, v_entity(p)->enabled, &cm);
        if ((result == V_RESULT_OK) && (cm != 0)) {
            if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
                v_partitionAdminSet(p->partitions, p->qos->partition, &arg.addedPartitions, &arg.removedPartitions);
            }
            c_walk(p->writers, qosChangedAction, &arg);

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
        }
    } else
    {
        result = V_RESULT_PRECONDITION_NOT_MET;
    }
    c_lockUnlock(&p->lock);

    return result;
}


static c_bool
assertLivelinessWriter(
    c_object o,
    c_voidp arg)
{
    v_entity e = v_entity(o);
    v_handle handle = *(v_handle *)arg;

    if (v_objectKind(e) == K_WRITER) {
        if (arg == NULL) { /* assert ALL writers */
            v_writerAssertByPublisher(v_writer(e));
        } else { /* assert all writers except writer starting the event! */
            if (v_handleIsEqual(v_publicHandle(v_public(e)), handle) == FALSE) {
                v_writerAssertByPublisher(v_writer(e));
            }
        }
    }
    return TRUE;
}

void
v_publisherAssertLiveliness(
    v_publisher p,
    v_event e)
{
    if (e->kind == V_EVENT_LIVELINESS_ASSERT) {
        c_lockRead(&p->lock);
        c_walk(p->writers, assertLivelinessWriter, (c_voidp)&e->source);
        c_lockUnlock(&p->lock);
    }
}

void
v_publisherConnectNewGroup(
    v_publisher p,
    v_group g)
{
    c_bool connect;
    c_iter addedPartitions;
    v_partition d;

    /* ES, dds1576: Only process this group event if the access rights to
     * the partition listed in the group is write or read_write.
     */
    if(v_groupPartitionAccessMode(g) == V_ACCESS_MODE_READ_WRITE ||
       v_groupPartitionAccessMode(g) == V_ACCESS_MODE_WRITE)
    {
        c_lockWrite(&p->lock);
        connect = v_partitionAdminFitsInterest(p->partitions, g->partition);

        if (connect) {
            addedPartitions = v_partitionAdminAdd(p->partitions,
                                            v_entityName(g->partition));
            d = v_partition(c_iterTakeFirst(addedPartitions));
            while (d != NULL) {
                c_free(d);
                d = v_partition(c_iterTakeFirst(addedPartitions));
            }
            c_iterFree(addedPartitions);

            c_walk(p->writers, (c_action)v_writerPublishGroup, g);
        }/*else do not connect */
        c_lockUnlock(&p->lock);
    }
}

/**************************************************************
 * Public functions
 **************************************************************/
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
v_publisherAddWriter(
    v_publisher p,
    v_writer w)
{
    v_partition d;
    c_iter iter;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));
    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    iter = c_iterNew(NULL);
    c_lockWrite(&p->lock);
    v_partitionAdminWalk(p->partitions, collectPartitions, iter);
    while ((d = c_iterTakeFirst(iter)) != NULL) {
        v_writerPublish(w,d);
        c_free(d);
    }
    v_writerCoherentBegin(w,v_publisherTransactionId(p));
    c_setInsert(p->writers,w);
    c_lockUnlock(&p->lock);
    c_iterFree(iter);
}

void
v_publisherRemoveWriter(
    v_publisher p,
    v_writer w)
{
    v_writer found;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));
    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    c_lockWrite(&p->lock);
#if 0
    /* following statement enables deletion of writers within
     * a transaction and still consider written samples to be
     * part of the transaction.
     * Not seen as correct behavior (yet).
     */
    v_writerCoherentEnd(w);
#endif
    found = c_remove(p->writers,w,NULL,NULL);
    c_lockUnlock(&p->lock);
    c_free(found);
}

c_bool
v_publisherCheckPartitionInterest(
    v_publisher p,
    v_partition partition)
{
    return v_partitionAdminFitsInterest(p->partitions, partition);
}

void
v_publisherPublish(
    v_publisher p,
    const c_char *partitionExpr)
{
    v_partition d;
    v_writerNotifyChangedQosArg arg;
    v_partitionPolicy old;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    arg.removedPartitions = NULL;

    c_lockWrite(&p->lock);
    arg.addedPartitions = v_partitionAdminAdd(p->partitions, partitionExpr);

    old = p->qos->partition;
    p->qos->partition = v_partitionPolicyAdd(old,
                                             partitionExpr,
                                             c_getBase(c_object(p)));
    c_free(old);

    c_walk(p->writers, qosChangedAction, &arg);

    d = v_partition(c_iterTakeFirst(arg.addedPartitions));
    while (d != NULL) {
        c_free(d);
        d = v_partition(c_iterTakeFirst(arg.addedPartitions));
    }
    c_iterFree(arg.addedPartitions);

    c_lockUnlock(&p->lock);
}

void
v_publisherUnPublish(
    v_publisher p,
    const c_char *partitionExpr)
{
    v_partition d;
    v_writerNotifyChangedQosArg arg;
    v_partitionPolicy old;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    arg.addedPartitions = NULL;

    c_lockWrite(&p->lock);
    arg.removedPartitions = v_partitionAdminRemove(p->partitions, partitionExpr);

    old = p->qos->partition;
    p->qos->partition = v_partitionPolicyRemove(old,
                                                partitionExpr,
                                                c_getBase(c_object(p)));
    c_free(old);

    c_walk(p->writers, qosChangedAction, &arg);

    d = v_partition(c_iterTakeFirst(arg.removedPartitions));
    while (d != NULL) {
        c_free(d);
        d = v_partition(c_iterTakeFirst(arg.removedPartitions));
    }
    c_iterFree(arg.removedPartitions);

    c_lockUnlock(&p->lock);
}


c_iter
v_publisherLookupWriters(
    v_publisher p,
    const c_char *topicExpr)
{
    c_iter list;
    c_collection q;
    q_expr expr;
    c_value params[1];

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    expr = (q_expr)q_parse("topic.name like %0");
    params[0] = c_stringValue((c_char *)topicExpr);
    q = c_queryNew(p->writers,expr,params);
    q_dispose(expr);

    c_lockRead(&p->lock);
    list = ospl_c_select(q,0);
    c_lockUnlock(&p->lock);
    c_free(q);
    return list;
}

c_iter
v_publisherLookupPartitions(
    v_publisher p,
    const c_char *partitionExpr)
{
    c_iter list;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    c_lockRead(&p->lock);
    list = v_partitionAdminLookup(p->partitions, partitionExpr);
    c_lockUnlock(&p->lock);

    return list;
}

void
v_publisherSuspend (
    v_publisher p)
{
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    c_lockWrite(&p->lock);
    if (c_timeCompare(p->suspendTime, C_TIME_INFINITE) == C_EQ) {
        p->suspendTime = v_timeGet();
    } /* else publisher was already suspended, so no-op */
    c_lockUnlock(&p->lock);
}

c_bool
v_publisherResume (
    v_publisher p)
{
    c_iter writers;
    v_writer w;
    c_time suspendTime;
    c_bool resumed = FALSE;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    c_lockWrite(&p->lock);
    if (c_timeCompare(p->suspendTime, C_TIME_INFINITE) != C_EQ) {
        suspendTime = p->suspendTime;
        p->suspendTime = C_TIME_INFINITE;
        writers = ospl_c_select(p->writers, 0);
        c_lockUnlock(&p->lock);

        w = v_writer(c_iterTakeFirst(writers));
        while (w != NULL) {
            v_writerResumePublication(w, &suspendTime);
            c_free(w);
            w = v_writer(c_iterTakeFirst(writers));
        }
        c_iterFree(writers);
        resumed = TRUE;
    } else {
        c_lockUnlock(&p->lock);
    }

    return resumed;
}

static c_bool
beginTransaction (
    c_object o,
    c_voidp arg)
{
    v_writer w = v_writer(o);
    c_ulong id = *(c_ulong *)arg;

    v_writerCoherentBegin(w,id);
    return TRUE;
}

void
v_publisherCoherentBegin (
    v_publisher p)
{
    v_kernel kernel;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    c_lockWrite(&p->lock);
    if (p->transactionId == 0) {
        kernel = v_objectKernel(p);
        p->transactionId = v_kernelGetTransactionId(kernel);
        c_walk(p->writers, beginTransaction, (c_voidp)&p->transactionId);
    }
    c_lockUnlock(&p->lock);
}

static c_bool
endTransaction (
    c_object o,
    c_voidp arg)
{
    v_writer w = v_writer(o);

    OS_UNUSED_ARG(arg);
    v_writerCoherentEnd(w);

    return TRUE;
}

void
v_publisherCoherentEnd (
    v_publisher p)
{
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    c_lockWrite(&p->lock);
    if (p->transactionId != 0) {
        p->transactionId = 0;
        c_walk(p->writers, endTransaction, NULL);
    }
    c_lockUnlock(&p->lock);
}

v_publisherQos
v_publisherGetQos(
    v_publisher p)
{
    v_publisherQos qos;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    c_lockRead(&p->lock);
    qos = v_publisherQosNew(v_objectKernel(p), p->qos);
    c_lockUnlock(&p->lock);

    return qos;
}

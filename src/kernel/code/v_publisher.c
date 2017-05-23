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

#include "v__publisher.h"
#include "v__publisherQos.h"
#include "v_participant.h"
#include "v__partition.h"
#include "v__partitionAdmin.h"
#include "v__writer.h"
#include "v__kernel.h"
#include "v__entity.h"
#include "v__builtin.h"
#include "v_group.h"
#include "v_observable.h"
#include "v__observer.h"
#include "v_public.h"
#include "v__policy.h"
#include "v_event.h"
#include "v_proxy.h"
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
    v_publisher p = NULL;
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
    if(qos && qos->partition.v) {
        access = v_kernelPartitionAccessMode(kernel, qos->partition);
    } else {
        access = V_ACCESS_MODE_READ_WRITE;/* default */
    }

    if(access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_WRITE) {
        if (v_publisherQosCheck(qos) == V_RESULT_OK) {
            q = v_publisherQosNew(kernel,qos);
            if (q != NULL) {
                p = v_publisher(v_objectNew(kernel,K_PUBLISHER));
                v_entityInit(v_entity(p),name,enable);
                p->partitions  = v_partitionAdminNew(kernel);
                p->writers     = c_setNew(v_kernelType(kernel,K_WRITER));
                p->qos         = q;
                p->suspendTime = OS_TIMEE_INFINITE;
                p->participant = participant;
                p->coherentNestingLevel = 0;
                p->transactionId = 0;
                p->tidList = NULL;
                p->orderbyFixed = FALSE;
                p->orderby = V_ORDERBY_RECEPTIONTIME;
                c_lockInit(c_getBase(p), &p->lock);
                v_participantAdd(participant,v_object(p));
                assert(c_refCount(p) == 3);  /* as handle, by participant and the local variable p */
                if (enable) {
                    v_publisherEnable(p);
                }
            } else {
                OS_REPORT(OS_ERROR, "v_publisherNew", V_RESULT_INTERNAL_ERROR,
                          "Publisher <%s> not created: cannot create publisher QoS", name);
            }
        }
    } else {
        OS_REPORT(OS_ERROR,
              "v_publisherNew", V_RESULT_PRECONDITION_NOT_MET,
              "Publisher not created: Access rights for one of the partitions listed in the partition list was not sufficient (i.e. write or readwrite).");
    }
    return p;
}

v_result
v_publisherEnable(
    v_publisher _this)
{
    v_kernel kernel;
    v_message builtinCMMsg;
    c_iter list;
    c_char *partitionName;
    v_result result = V_RESULT_ILL_PARAM;

    if (_this) {
        kernel = v_objectKernel(_this);
        v_observableAddObserver(v_observable(kernel->groupSet), v_observer(_this), NULL);

        if (_this->qos->partition.v != NULL) {
            list = v_partitionPolicySplit(_this->qos->partition);
            while((partitionName = c_iterTakeFirst(list)) != NULL) {
                v_publisherPublish(_this,partitionName);
                os_free(partitionName);
            }
            c_iterFree(list);
        }
        builtinCMMsg = v_builtinCreateCMPublisherInfo(kernel->builtin, _this);
        v_writeBuiltinTopic(kernel, V_CMPUBLISHERINFO_ID, builtinCMMsg);
        c_free(builtinCMMsg);
        result = V_RESULT_OK;
    }
    return result;
}

void
v_publisherFree(
    v_publisher p)
{
    v_writer o;
    v_message builtinCMMsg;
    v_message unregisterCMMsg;
    v_participant participant;
    v_kernel kernel;

    assert(C_TYPECHECK(p,v_publisher));

    kernel = v_objectKernel(p);
    builtinCMMsg = v_builtinCreateCMPublisherInfo(kernel->builtin, p);
    unregisterCMMsg = v_builtinCreateCMPublisherInfo(kernel->builtin, p);
    v_observableRemoveObserver(v_observable(kernel->groupSet),v_observer(p), NULL);

    while ((o = c_take(p->writers)) != NULL) {
        /* remove writer from all partitions */
        v_writerFree(o);
        c_free(o);
    }
    participant = v_participant(p->participant);
    if (participant != NULL) {
        v_participantRemove(participant,v_object(p));
        p->participant = NULL;
    }
    v_entityFree(v_entity(p));

    v_writeDisposeBuiltinTopic(kernel, V_CMPUBLISHERINFO_ID, builtinCMMsg);
    v_unregisterBuiltinTopic(kernel, V_CMPUBLISHERINFO_ID, unregisterCMMsg);
    c_free(builtinCMMsg);
    c_free(unregisterCMMsg);
}

void
v_publisherDeinit(
    v_publisher p)
{
    assert(C_TYPECHECK(p,v_publisher));

    v_entityDeinit(v_entity(p));
}

/**************************************************************
 * Protected functions
 **************************************************************/

v_publisherQos
v_publisherGetQos(
    v_publisher _this)
{
    v_publisherQos qos;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_publisher));

    c_lockRead(&_this->lock);
    qos = c_keep(_this->qos);
    c_lockUnlock(&_this->lock);

    return qos;
}

v_result
v_publisherSetQos(
    v_publisher p,
    v_publisherQos tmpl)
{
    v_result result;
    v_kernel kernel;
    v_publisherQos qos;
    v_qosChangeMask cm;
    v_writerNotifyChangedQosArg arg;
    v_partition d;
    v_accessMode access;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    arg.addedPartitions = NULL;
    arg.removedPartitions = NULL;

    result = v_publisherQosCheck(tmpl);
    if (result == V_RESULT_OK) {
        c_lockWrite(&p->lock);
        /* For now we don't allow changing the publisher qos during a coherent update.
         * we might want to improve this by only disallowing changes that break
         * connectivity or eventually allow them and correctly handle the impact.
         */
        if (p->coherentNestingLevel == 0) {
            kernel = v_objectKernel(p);
            qos = v_publisherQosNew(kernel, tmpl);
            if (!qos) {
                c_lockUnlock(&p->lock);
                return V_RESULT_OUT_OF_MEMORY;
            }

            /* ES, dds1576: When the QoS is being set we have to verify that the partitions
             * listed in the qos do not have an invalid access mode set. For a publisher
             * an invalid mode is characterized as 'read' mode. If the partitionAccess
             * check returns ok, then everything continue as normal. If the
             * partitionAccess check returns something else, then the setQos operation
             * is aborted. Please see the partitionAccess check operation to know when
             * a partition expression is not allowed.
             */
            if(qos->partition.v) {
                access = v_kernelPartitionAccessMode(kernel, qos->partition);
            } else {
                access = V_ACCESS_MODE_READ_WRITE;/* default */
            }
            if (access == V_ACCESS_MODE_READ_WRITE || access == V_ACCESS_MODE_WRITE)
            {
                result = v_publisherQosCompare(p->qos, qos, v_entityEnabled(v_entity(p)), &cm);
                if ((result == V_RESULT_OK) && (cm != 0)) {
                    v_message builtinCMMsg;
                    c_free(p->qos);
                    p->qos = c_keep(qos);

                    if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
                        v_partitionAdminSet(p->partitions,
                                            p->qos->partition,
                                            &arg.addedPartitions,
                                            &arg.removedPartitions);
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
                    if (v_entity(p)->enabled) {
                        builtinCMMsg = v_builtinCreateCMPublisherInfo (kernel->builtin, p);
                        v_writeBuiltinTopic(kernel, V_CMPUBLISHERINFO_ID, builtinCMMsg);
                        c_free(builtinCMMsg);
                    }
                }
            } else {
                result = V_RESULT_PRECONDITION_NOT_MET;
            }
            c_free(qos);
        } else {
            result = V_RESULT_PRECONDITION_NOT_MET;
        }
        c_lockUnlock(&p->lock);
    }
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

c_bool
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
        (void)c_lockWrite(&p->lock);
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

    return TRUE;
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
    (void) c_iterInsert(iter,c_keep(d));
    return TRUE;
}

v_result
v_publisherAddWriter(
    v_publisher p,
    v_writer w)
{
    v_result result = V_RESULT_OK;
    v_partition d;
    c_iter iter;
    c_ulong transactionId;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));
    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    iter = c_iterNew(NULL);
    (void)c_lockWrite(&p->lock);

    if (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP &&
        p->qos->presentation.v.ordered_access == TRUE)
    {
        if (p->orderbyFixed == FALSE) {
            p->orderbyFixed = TRUE;
            p->orderby = w->qos->orderby.v.kind;
        } else if (w->qos->orderby.v.kind != p->orderby) {
            result = V_RESULT_INCONSISTENT_QOS;
            OS_REPORT (OS_ERROR, OS_FUNCTION, result,
                "Could not enable writer, destination order inconsistent with presentation and writer set on publisher");
        }
    }

    if (result == V_RESULT_OK) {
        if (p->coherentNestingLevel > 0) {
            v_writerCoherentBegin(w, &transactionId);
            if (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) {
                /*
                 * A group coherent update is in progress so extend the existing
                 * tidList with new writer transactionId.
                 */
                c_ulong oldSize = c_arraySize(p->tidList);
                c_array tidList = p->tidList;
                p->tidList = c_arrayNew(v_kernelType(v_objectKernel(p), K_TID), oldSize+1);
                memcpy(p->tidList, tidList, sizeof(struct v_tid)*oldSize);

                ((struct v_tid *)p->tidList)[oldSize].wgid = v_publicGid(v_public(w));
                ((struct v_tid *)p->tidList)[oldSize].seqnr = transactionId;
            }
        }

        v_partitionAdminWalk(p->partitions, collectPartitions, iter);
        while ((d = c_iterTakeFirst(iter)) != NULL) {
            v_writerPublish(w,d);
            c_free(d);
        }
        c_setInsert(p->writers,w);
    }

    c_lockUnlock(&p->lock);
    c_iterFree(iter);

    return result;
}
#define v_publisherId(_this) v_public(_this)->handle.index

v_result
v_publisherRemoveWriter(
    v_publisher _this,
    v_writer w)
{
    v_writer found;
    v_result result;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this, v_publisher));
    assert(w != NULL);
    assert(C_TYPECHECK(w, v_writer));

    (void)c_lockWrite(&_this->lock);
    found = c_remove(_this->writers, w, NULL, NULL);
    c_lockUnlock(&_this->lock);

    if (found) {
        c_free(found);
        result = V_RESULT_OK;
    } else {
        result = V_RESULT_ALREADY_DELETED;
    }

    return result;
}

void
v_publisherWalkWriters(
    v_publisher _this,
    v_writerAction action,
    c_voidp arg)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_publisher));

    (void)c_lockWrite(&_this->lock);
    (void)c_setWalk(_this->writers,(c_action)action,arg);
    (void)c_lockUnlock(&_this->lock);
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
    v_partitionPolicyI old;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    arg.removedPartitions = NULL;

    (void)c_lockWrite(&p->lock);
    arg.addedPartitions = v_partitionAdminAdd(p->partitions, partitionExpr);

    old = p->qos->partition;
    p->qos->partition = v_partitionPolicyAdd(old, partitionExpr, c_getBase(c_object(p)));
    c_free(old.v);

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
    v_partitionPolicyI old;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    arg.addedPartitions = NULL;

    (void)c_lockWrite(&p->lock);
    arg.removedPartitions = v_partitionAdminRemove(p->partitions, partitionExpr);

    old = p->qos->partition;
    p->qos->partition = v_partitionPolicyRemove(old, partitionExpr, c_getBase(c_object(p)));
    c_free(old.v);

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

    (void)c_lockWrite(&p->lock);
    if (!v_publisherIsSuspended(p)) {
        p->suspendTime = os_timeEGet(); /* Compared with allocTime */
    } /* else publisher was already suspended, so no-op */
    c_lockUnlock(&p->lock);
}

c_bool
v_publisherResume (
    v_publisher p)
{
    c_iter writers;
    v_writer w;

    c_bool resumed = FALSE;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    (void)c_lockWrite(&p->lock);
    if (v_publisherIsSuspended(p)) {
        const os_timeW resumeTime = os_timeWGet();
        p->suspendTime = OS_TIMEE_INFINITE;
        writers = ospl_c_select(p->writers, 0);
        c_lockUnlock(&p->lock);

        while ((w = v_writer(c_iterTakeFirst(writers))) != NULL) {
            v_writerResumePublication(w, &resumeTime);
            c_free(w);
        }
        c_iterFree(writers);
        resumed = TRUE;
    } else {
        c_lockUnlock(&p->lock);
    }

    return resumed;
}

struct beginTransactionInfo
{
    v_result result;
    c_array tidList;
    c_ulong tidCount;
};

static c_bool
beginTransaction (
    c_object o,
    c_voidp arg)
{
    v_writer w = v_writer(o);
    struct beginTransactionInfo *info = (struct beginTransactionInfo *)arg;
    struct v_tid *tidList = (struct v_tid *)info->tidList;
    c_ulong transactionId;

    v_writerCoherentBegin(w, &transactionId);
    if (tidList) {
        struct v_tid *tid = &tidList[info->tidCount++];

        tid->wgid = v_publicGid(v_public(w));
        tid->seqnr = transactionId;
    }
    return TRUE;
}

c_bool
v__publisherCoherentTransactionSingleNoLock(
    v_publisher p,
    c_ulong *publisherId,
    c_ulong *transactionId)
{
    c_bool result = FALSE;
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));
    assert(publisherId != NULL);
    assert(transactionId != NULL);

    if (p->qos->presentation.v.access_scope != V_PRESENTATION_INSTANCE && p->qos->presentation.v.coherent_access == TRUE) {
        if (p->coherentNestingLevel == 0) {
            *publisherId = (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) ? v_publisherId(p) : 0;
            *transactionId = p->transactionId++;
            result = TRUE;
        }
    }

    return result;
}

v_result
v_publisherCoherentBegin (
    v_publisher p)
{
    v_result result = V_RESULT_OK;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    (void)c_lockWrite(&p->lock);
    if (++(p->coherentNestingLevel) == 1) {
        if (p->qos->presentation.v.access_scope != V_PRESENTATION_INSTANCE && p->qos->presentation.v.coherent_access == TRUE){
            struct beginTransactionInfo info;

            assert(p->tidList == NULL);
            if (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) {
                /* Create a list of participation group coherent writer transactions. */
                p->tidList = c_arrayNew(v_kernelType(v_objectKernel(p), K_TID),
                                        c_count(p->writers));
            }
            info.result = result;
            info.tidCount = 0;
            info.tidList = p->tidList;
            c_walk(p->writers, beginTransaction, &info);
            result = info.result;
        }
    }
    c_lockUnlock(&p->lock);

    return result;
}

struct endTransactionInfo
{
    v_result result;
    c_array tidList;
    c_ulong publisherId;
    c_ulong transactionId;
};

static c_bool
endTransaction (
    c_object o,
    c_voidp arg)
{
    v_writer w = v_writer(o);
    struct endTransactionInfo *eotInfo = (struct endTransactionInfo *)arg;

    eotInfo->result = v_writerCoherentEnd(w, eotInfo->publisherId, eotInfo->transactionId, eotInfo->tidList);

    return (eotInfo->result == V_RESULT_OK);
}

v_result
v_publisherCoherentEnd (
    v_publisher p)
{
    v_result result = V_RESULT_OK;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    (void)c_lockWrite(&p->lock);
    if (p->coherentNestingLevel == 0) {
        result = V_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, "v_publisherCoherentEnd", V_RESULT_PRECONDITION_NOT_MET,
                  "Invoked CoherentEnd without a preceding CoherentBegin");

    } else {
        if ((--(p->coherentNestingLevel) == 0) &&
            (p->qos->presentation.v.access_scope != V_PRESENTATION_INSTANCE) && p->qos->presentation.v.coherent_access == TRUE)
        {
            struct endTransactionInfo eotInfo;
            eotInfo.result = result;
            eotInfo.publisherId = 0;
            eotInfo.transactionId = p->transactionId++;
            eotInfo.tidList = NULL;
            if (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) {
                eotInfo.publisherId = v_publisherId(p);
                eotInfo.tidList = p->tidList;
                p->tidList = NULL;
            }
            if (eotInfo.result == V_RESULT_OK) {
                c_walk(p->writers, endTransaction, &eotInfo);
            }
            result = eotInfo.result;
            c_free(eotInfo.tidList);
        }
    }

    c_lockUnlock(&p->lock);

    return result;
}

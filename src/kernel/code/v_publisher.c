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
#include "v__observer.h"
#include "v__observable.h"
#include "v_public.h"
#include "v__policy.h"
#include "v_event.h"
#include "v_proxy.h"
#include "v_partition.h"
#include "v__nameSpace.h"
#include "c_stringSupport.h"

#include "q_expr.h"

#include "os_report.h"
#include "os_heap.h"

/**
 * \brief This operation checks if writer(s) are in correct nameSpace for
 *        group coherence.
 *
 * This operation is intended for group coherent publishers and checks if
 * writer(s) is/are in a single nameSpace and if they are all part of the
 * same nameSpace
 *
 * \precondition publisher lock is obtained
 *
 * \param p The publisher to perform this operation on.
 * \param writer When not NULL the single nameSpace check is only
 *        performed on this writer.
 *
 * \return TRUE when tested writer(s) is/are all in single nameSpace and
 *         all writers are in the same nameSpace.
 *         Otherwise FALSE is returned and an error is reported.
 */
static c_bool
publisherGroupCoherenceNameSpaceCheck(
    v_publisher p,
    v_writer writer)
{
    c_bool result = TRUE;
    v_kernel kernel;
    c_iter nameSpaces;
    c_iter partitions = NULL;
    struct v_nameSpace *ns;
    char *pName;
    struct c_collectionIter it;
    v_writer w;
    char *n2, *n1 = NULL;
    v_writerQos qos;

    assert(p);
    assert(C_TYPECHECK(p,v_publisher));
    assert(C_TYPECHECK(writer,v_writer));

    kernel = v_objectKernel(p);
    nameSpaces = v__nameSpaceCollect(kernel);
    partitions = v_partitionPolicySplit(p->qos->partition);
    if (partitions == NULL) {
        partitions = c_iterAppend(partitions, os_strdup(""));
    }

    if (writer) {
        /* Check only the supplied writer if its in a single nameSpace */
        qos = v_writerGetQos(writer);
        if (qos->durability.v.kind != V_DURABILITY_VOLATILE) {
            if (v__writerInSingleNameSpace(writer, partitions, nameSpaces) == FALSE) {
                result = FALSE;
            }
        }
        c_free(qos);
    } else {
        /* Check if all writers are in a single nameSpace */
        for (w = c_collectionIterFirst(p->writers, &it); w; w = c_collectionIterNext(&it)) {
            qos = v_writerGetQos(w);
            if (qos->durability.v.kind != V_DURABILITY_VOLATILE) {
                if (v__writerInSingleNameSpace(w, partitions, nameSpaces) == FALSE) {
                    result = FALSE;
                }
            }
            c_free(qos);
        }
    }

    /* Check if writers are in the same nameSpace */
    if (result == TRUE) {
        if (writer) {
            /* Use the supplied writer to check for same nameSpace */
            /* result == TRUE so writer is non volatile
             */
            n1 = v__writerGetNameSpaceNames(writer, partitions, nameSpaces);
        }
        for (w = c_collectionIterFirst(p->writers, &it); w && result == TRUE; w = c_collectionIterNext(&it)) {
            qos = v_writerGetQos(w);
            if (qos->durability.v.kind != V_DURABILITY_VOLATILE) {
                if (writer == NULL) {
                    writer = w; /* use first non volatile writer for same check */
                    n1 = v__writerGetNameSpaceNames(writer, partitions, nameSpaces);
                } else {
                    n2 = v__writerGetNameSpaceNames(w, partitions, nameSpaces);
                    if (n1 == NULL || n2 == NULL || strcmp(n1, n2) != 0) {
                        OS_REPORT (OS_ERROR, OS_FUNCTION, V_RESULT_UNDEFINED,
                                   "Not all writer for group coherent %s are in the same nameSpace", v_entityName(p));
                        result = FALSE;
                    }
                    os_free(n2);
                    n2 = NULL;
                }
            }
            c_free(qos);
        }
        os_free(n1);
    }

    while ((pName = c_iterTakeFirst(partitions)) != NULL) {
        os_free(pName);
    }
    c_iterFree(partitions);
    while ((ns = c_iterTakeFirst(nameSpaces)) != NULL) {
        v__nameSpaceFree(ns);
    }
    c_iterFree(nameSpaces);

    return result;
}

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
                v_entityInit(v_entity(p),name);
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
                v_participantAdd(participant,v_object(p));
                assert(c_refCount(p) == 3);  /* as handle, by participant and the local variable p */
                if (enable) {
                    (void)v_entityEnable(v_entity(p));
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
    v_result result = V_RESULT_ILL_PARAM;
    v_kernel kernel;

    if (_this) {
        v_message builtinCMMsg = NULL;
        kernel = v_objectKernel(_this);
        result = V_RESULT_OK;

        OSPL_LOCK(_this);
        if ((_this->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) &&
            (_this->qos->presentation.v.coherent_access == TRUE) &&
            (publisherGroupCoherenceNameSpaceCheck(_this, NULL) == FALSE)) {
#ifdef ENABLE_GROUP_COHERENT_NAMESPACE_CHECK_FAILURE
                result = V_RESULT_PRECONDITION_NOT_MET;
                OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_PRECONDITION_NOT_MET,
                          "Publisher enable failed: writer nameSpace error");
#endif
        }
        if (result == V_RESULT_OK) {
            if (_this->qos->partition.v != NULL) {
                c_char *partitionName;
                c_iter list = v_partitionPolicySplit(_this->qos->partition);
                while((partitionName = c_iterTakeFirst(list)) != NULL) {
                    v_partitionAdminFill(_this->partitions, partitionName);
                    os_free(partitionName);
                }
                c_iterFree(list);
            }
            builtinCMMsg = v_builtinCreateCMPublisherInfo(kernel->builtin, _this);
        }
        OSPL_UNLOCK(_this);
        if (builtinCMMsg) {
            v_writeBuiltinTopic(kernel, V_CMPUBLISHERINFO_ID, builtinCMMsg);
            c_free(builtinCMMsg);
        }
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

v_publisherQos
v_publisherGetQos(
    v_publisher _this)
{
    v_publisherQos qos;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_publisher));

    OSPL_LOCK(_this);
    qos = c_keep(_this->qos);
    OSPL_UNLOCK(_this);

    return qos;
}

v_result
v_publisherSetQos(
    v_publisher p,
    v_publisherQos tmpl)
{
    v_result result;
    v_kernel kernel;
    v_publisherQos qos, prevQos;
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
        kernel = v_objectKernel(p);
        qos = v_publisherQosNew(kernel, tmpl);
        if (!qos) {
            return V_RESULT_OUT_OF_MEMORY;
        }
        /* For now we don't allow changing the publisher qos during a coherent update.
         * we might want to improve this by only disallowing changes that break
         * connectivity or eventually allow them and correctly handle the impact.
         */
        if (p->coherentNestingLevel == 0) {

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
                v_message builtinCMMsg = NULL;
                c_iter writers = NULL;
                v_writer writer;
                OSPL_LOCK(p);
                result = v_publisherQosCompare(p->qos, qos, v__entityEnabled_nl(v_entity(p)), &cm);
                if ((result == V_RESULT_OK) && (cm != 0)) {
                    prevQos = p->qos;
                    p->qos = c_keep(qos);

                    if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
                        /* Only the changing of the partition Qos policy of the publisher can
                         * change the namespace(s) a writer belongs too. */
                        if ((p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) &&
                            (p->qos->presentation.v.coherent_access == TRUE) &&
                            (publisherGroupCoherenceNameSpaceCheck(p, NULL) == FALSE)) {
                            /* reset the qos */
#ifdef ENABLE_GROUP_COHERENT_NAMESPACE_CHECK_FAILURE
                            c_free(p->qos);
                            p->qos = c_keep(prevQos);
                            result = V_RESULT_PRECONDITION_NOT_MET;
                            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_PRECONDITION_NOT_MET,
                                      "Publisher qos change failed: writer nameSpace error");
#endif
                        }
                    }
                    c_free(prevQos);

                    if (result == V_RESULT_OK) {
                        if (cm & V_POLICY_BIT_PARTITION) { /* partition policy has changed! */
                            v_partitionAdminUpdate(p->partitions,
                                                   p->qos->partition,
                                                   &arg.addedPartitions,
                                                   &arg.removedPartitions);
                        }
                        writers = ospl_c_select(p->writers, 0);
                    }
                    if (v__entityEnabled_nl(v_entity(p))) {
                        builtinCMMsg = v_builtinCreateCMPublisherInfo (kernel->builtin, p);
                    }
                }
                OSPL_UNLOCK(p);
                while ((writer = c_iterTakeFirst(writers)) != NULL) {
                    v_writerNotifyChangedQos(writer, &arg);
                    c_free(writer);
                }
                c_iterFree(writers);
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
                    v_writeBuiltinTopic(kernel, V_CMPUBLISHERINFO_ID, builtinCMMsg);
                    c_free(builtinCMMsg);
                }
            } else {
                result = V_RESULT_PRECONDITION_NOT_MET;
            }
        } else {
            result = V_RESULT_PRECONDITION_NOT_MET;
        }
        c_free(qos);
    }
    return result;
}


static c_bool
assertLivelinessWriter(
    c_object o,
    c_voidp arg)
{
    v_event e = (v_event)arg;
    v_writer w = v_writer(o);

    if (e == NULL) { /* assert ALL writers */
        v_writerAssertByPublisher(w);
    } else { /* assert all writers except writer starting the event! */
        if (w != v_writer(e->source)) {
            v_writerAssertByPublisher(w);
        }
    }
    return TRUE;
}

void
v_publisherAssertLiveliness(
    v_publisher p,
    v_event e)
{
    if (e == NULL || e->kind == V_EVENT_LIVELINESS_ASSERT) {
        OSPL_LOCK(p);
        c_walk(p->writers, assertLivelinessWriter, (c_voidp)e);
        OSPL_UNLOCK(p);
    }
}

c_bool
v_publisherConnectNewGroup(
    v_publisher p,
    v_group g)
{
    c_iter writers = NULL;
    v_writer writer = NULL;
    /* ES, dds1576: Only process this group event if the access rights to
     * the partition listed in the group is write or read_write.
     */
    if(v_groupPartitionAccessMode(g) == V_ACCESS_MODE_READ_WRITE ||
       v_groupPartitionAccessMode(g) == V_ACCESS_MODE_WRITE)
    {
        OSPL_LOCK(p);
        if (v_partitionAdminAdd(p->partitions, g->partition)) {
            writers = ospl_c_select(p->writers, 0);
        }/*else do not connect */
        OSPL_UNLOCK(p);
        while ((writer = c_iterTakeFirst(writers)) != NULL) {
            v_writerPublishGroup(writer, g);
            c_free(writer);
        }
        c_iterFree(writers);
    }

    return TRUE;
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

v_result
v_publisherAddWriter(
    v_publisher p,
    v_writer w)
{
    v_result result = V_RESULT_OK;
    v_partition d;
    c_iter partitions = NULL;
    c_ulong transactionId;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));
    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    OSPL_LOCK(p);

    if ((p->qos->presentation.v.coherent_access == TRUE) &&
        (p->qos->presentation.v.access_scope != V_PRESENTATION_INSTANCE) &&
        (w->qos->history.v.kind == V_HISTORY_KEEPLAST)) {
        result = V_RESULT_PRECONDITION_NOT_MET;
        OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_PRECONDITION_NOT_MET,
                  "Could not enable writer, KEEP_LAST history QoS is not supported in combination with coherent access");
    }

    if ((result == V_RESULT_OK) &&
        (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) &&
        (p->qos->presentation.v.coherent_access == TRUE)) {
        if (publisherGroupCoherenceNameSpaceCheck(p, w) == FALSE) {
#ifdef ENABLE_GROUP_COHERENT_NAMESPACE_CHECK_FAILURE
            result = V_RESULT_PRECONDITION_NOT_MET;
            OS_REPORT(OS_ERROR, OS_FUNCTION, V_RESULT_PRECONDITION_NOT_MET,
                      "Failed to add writer to publisher: writer nameSpace error");
#endif
        }
    }

    if ((result == V_RESULT_OK) &&
        (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) &&
        (p->qos->presentation.v.ordered_access == TRUE))
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
        if (p->coherentNestingLevel > 0 && p->qos->presentation.v.coherent_access) {
            v_writerCoherentBegin(w, &transactionId);
            if (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) {
                /* A group coherent update is in progress so extend the existing
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
        c_setInsert(p->writers,w);

        partitions = c_iterNew(NULL);
        v_partitionAdminWalk(p->partitions, collectPartitions, partitions);
    }
    OSPL_UNLOCK(p);
    if (result == V_RESULT_OK) {
        while ((d = c_iterTakeFirst(partitions)) != NULL) {
            v_writerPublish(w,d);
            c_free(d);
        }
        c_iterFree(partitions);
    }

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

    OSPL_LOCK(_this);
    found = c_remove(_this->writers, w, NULL, NULL);
    OSPL_UNLOCK(_this);

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

    OSPL_LOCK(_this);
    (void)c_setWalk(_this->writers,(c_action)action,arg);
    OSPL_UNLOCK(_this);
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

    OSPL_LOCK(p);
    list = ospl_c_select(q,0);
    OSPL_UNLOCK(p);
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

    OSPL_LOCK(p);
    list = v_partitionAdminLookup(p->partitions, partitionExpr);
    OSPL_UNLOCK(p);

    return list;
}

void
v_publisherSuspend (
    v_publisher p)
{
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    OSPL_LOCK(p);
    if (!v_publisherIsSuspended(p)) {
        p->suspendTime = os_timeEGet(); /* Compared with allocTime */
    } /* else publisher was already suspended, so no-op */
    OSPL_UNLOCK(p);
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

    OSPL_LOCK(p);
    if (v_publisherIsSuspended(p)) {
        const os_timeW resumeTime = os_timeWGet();
        p->suspendTime = OS_TIMEE_INFINITE;
        writers = ospl_c_select(p->writers, 0);
        OSPL_UNLOCK(p);

        while ((w = v_writer(c_iterTakeFirst(writers))) != NULL) {
            v_writerResumePublication(w, &resumeTime);
            c_free(w);
        }
        c_iterFree(writers);
        resumed = TRUE;
    } else {
        OSPL_UNLOCK(p);
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
v_publisherStartTransaction(
    v_publisher p,
    c_ulong *publisherId,
    c_ulong *transactionId)
{
    c_bool result = FALSE;
    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));
    assert(publisherId != NULL);
    assert(transactionId != NULL);

    OSPL_LOCK(p);
    if (p->qos->presentation.v.access_scope != V_PRESENTATION_INSTANCE && p->qos->presentation.v.coherent_access == TRUE) {
        if (p->coherentNestingLevel == 0) {
            *publisherId = (p->qos->presentation.v.access_scope == V_PRESENTATION_GROUP) ? v_publisherId(p) : 0;
            *transactionId = p->transactionId++;
            result = TRUE;
        }
    }
    OSPL_UNLOCK(p);
    return result;
}

v_result
v_publisherCoherentBegin (
    v_publisher p)
{
    v_result result = V_RESULT_OK;

    assert(p != NULL);
    assert(C_TYPECHECK(p,v_publisher));

    OSPL_LOCK(p);
    if (++(p->coherentNestingLevel) == 1) {
        if (p->qos->presentation.v.access_scope != V_PRESENTATION_INSTANCE &&
            p->qos->presentation.v.coherent_access == TRUE)
        {
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
    OSPL_UNLOCK(p);

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

    OSPL_LOCK(p);
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
    OSPL_UNLOCK(p);

    return result;
}

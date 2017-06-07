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
/**
 * ----------------------- Heartbeats -----------------------
 * Heartbeats are implemented using the publish/subscriber mechanism.
 * The heartbeat contains a unique identification (identifying the other
 * splicedaemon) and a period telling the consumer when the next heartbeat
 * can be expected.
 * This period can change from heartbeat to heartbeat by the producer (this
 * allows the producer to lower the frequency when it is too busy for example).
 * Periodically writing the heartbeat is handled by the lease manager of the
 * splicedaemon (every participant has a leasemanager).
 * Checking heartbeats is also handled by the splicedeamon's lease manager.
 * When a heartbeat arrives the first time, we do not need to act immediately,
 * since all connections are also established arbitrarily.
 * This heartbeat is taken into account the next time all heartbeats are
 * checked, which is periodically.
 * A heartbeat does not need to be disposed, since the actions to be taken
 * are identical to missing a heartbeat.
 *
 * -------------------- Builtin-in topics -------------------
 * The information on builtin-in subscriptions and publications must also be
 * published! This is needed to ensure that the instance resources for the
 * builtin-in reader are freed as soon as there are no more writers!
 * Otherwise these resources were not freed in case a splicedaemon dies.
 */

#include "v__spliced.h"

#include "vortex_os.h"
#include "os_report.h"
#include "os_stdlib.h"
#include "os_process.h"
#include "c_stringSupport.h"

#include "sd_serializerXMLTypeinfo.h"

#include "v__builtin.h"
#include "v_participant.h"
#include "v_service.h"
#include "v__kernel.h"
#include "v_public.h"
#include "v_entity.h"
#include "v_observable.h"
#include "v_event.h"
#include "v_dataReaderSample.h"
#include "v__subscriber.h"
#include "v__dataReader.h"
#include "v_dataReaderInstance.h"
#include "v_dataReaderEntry.h"
#include "v_query.h"
#include "v_publisher.h"
#include "v__writer.h"
#include "v_policy.h"
#include "v__lease.h"
#include "v__leaseManager.h"
#include "v_topicQos.h"
#include "v__topic.h"
#include "v__topicImpl.h"
#include "v_state.h"
#include "v__group.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v__waitset.h"
#include "v_messageQos.h"
#include "v__deliveryService.h"
#include "v_writerQos.h"
#include "v__typeRepresentation.h"
#include "v_durabilityClient.h"

#define HB_VIEW_NAME   "heartbeat view"
#define HB_VIEW_EXPR   "select * from " V_HEARTBEATINFO_NAME " where id.systemId != %0;"
#define HB_READER_NAME V_HEARTBEATINFO_NAME "Reader"

#define HB_DEFAULT_SEC  (20)
#define HB_DEFAULT_NSEC (0)
#define HB_DEFAULT_RENEWAL_SEC  (1)
#define HB_DEFAULT_RENEWAL_NSEC (0)

#define C_AND_M_READER_NAME V_C_AND_M_COMMAND_NAME "Reader"

#define CONFORM_STATE_TO_LIVELINESS_STATE(conform, alive) \
    (((conform) == CONFORM_STATE_UNKNOWN) ? V_STATUSLIVELINESS_UNKNOWN : \
     ((conform) == CONFORM_STATE_NO_MATCH) ? V_STATUSLIVELINESS_DELETED : \
     ((conform) == CONFORM_STATE_MATCH_NOT_COMPATIBLE) ? V_STATUSLIVELINESS_DELETED : \
     ((conform) == CONFORM_STATE_OK) ? (((alive) == TRUE) ? V_STATUSLIVELINESS_ALIVE : V_STATUSLIVELINESS_NOTALIVE) : V_STATUSLIVELINESS_UNKNOWN)

typedef enum {
    CONFORM_STATE_UNKNOWN,
    CONFORM_STATE_NO_MATCH,
    CONFORM_STATE_MATCH_NOT_COMPATIBLE,
    CONFORM_STATE_OK
} conformState;

static void
doCleanupPublication(
    v_spliced spliced,
    struct v_publicationInfo *oInfo,
    os_timeW cleanTime,
    c_bool isValid,
    c_bool isLocal,
    c_bool isImplicit);

/**************************************************************
 * Private functions
 **************************************************************/
static v_actionResult
readerTakeAction(
    c_object s,
    c_voidp arg)
{
    v_actionResult result = 0;
    c_iter *samples = (c_iter *)arg;

    if ((s != NULL) && v_dataReaderSampleStateTest(s, L_VALIDDATA)) {
        /* use append, since the oldest message is given first to this copy routine! */
        *samples = c_iterAppend(*samples, c_keep(s));
    } /* else last sample is read */

    v_actionResultSet(result, V_PROCEED);
    return result;
}

#if 0
static c_bool
lookupAction(
    c_object o,
    c_voidp arg)
{
    c_iter *messages = (c_iter *)arg;

    if (o != NULL) {
        *messages = c_iterAppend(*messages, c_keep(o));
    }

    return TRUE;
}
#endif

static v_actionResult
takeOne(
    c_object s,
    c_voidp arg)
{
    v_readerSample *sample = (v_readerSample *)arg;
    v_actionResult result = 0;

    if (s != NULL) {
        v_actionResultSet(result, V_PROCEED);
        if (v_dataReaderSampleStateTest(s, L_VALIDDATA) ||
                v_dataReaderSampleInstanceStateTest(s, L_DISPOSED)) {
            /* Keep both the sample and its instance.
             * If the instance was disposed, taking the sample like we do
             * here will remove all instance administration. We need the
             * instance administration at a later stage though, so we need
             * to keep both the sample and its instance.
             */
            *sample = c_keep(s);
            c_keep(v_dataReaderSampleInstance(s));
            v_actionResultClear(result, V_PROCEED);
        }
    }

    return result;
}

static v_dataReader gidClaimDataReader(v_gid id, v_kernel kernel)
{
    v_dataReader r = NULL;
    v_public p = v_gidClaim(id, kernel);
    if (p != NULL) {
        if (c_instanceOf(p, "v_dataReader")) {
            r = v_dataReader(p);
        } else {
            v_gidRelease(id, kernel);
        }
    }
    return r;
}

struct matchArg {
    c_bool  matched;
    c_array partition;
};

static c_bool
partitionNotMatched(
    v_entity o,
    c_voidp arg)
{
    struct matchArg *matchArg = (struct matchArg *)arg;
    c_value match;
    c_value partitionName;
    c_value expr;
    c_ulong i;

    /* Don't re-evaluate if a matching partition has already been found */
    if (!matchArg->matched && v_objectKind(o) == K_DOMAIN) {
        match.kind = V_BOOLEAN;
        match.is.Boolean = FALSE;
        i = 0;
        while ((match.is.Boolean == FALSE) && (i < c_arraySize(matchArg->partition))) {
            partitionName = c_stringValue(v_entityName(o));
            expr = c_stringValue(matchArg->partition[i]);
            match = c_valueStringMatch(expr, partitionName);
            i++;
        }
        matchArg->matched = match.is.Boolean;
    }

    return TRUE;
}

static c_bool
readerWriterMatch(
    struct v_subscriptionInfo *rInfo,
    v_dataReader r,
    struct v_publicationInfo *oInfo,
    v_writer w)
{
    struct matchArg matchArg;
    v_gid gid;
    v_public publ;

    /**
     * If (w != NULL) then it is a local writer.
     */
    if ((w == NULL) && (r == NULL)) {
        /* nothing to check.... */
        matchArg.matched = FALSE;
    } else {
        if (w != NULL) {
            /* for every partition of w, check if it matches
               with the requested partition.
            */
            matchArg.matched = FALSE;
            matchArg.partition = rInfo->partition.name;

            /*Claim publisher first before walking over relations.*/
            gid = v_publicGid(v_public(w->publisher));
            publ = v_gidClaim(gid, v_objectKernel(w));

            if (publ) {
                v_entityWalkEntities(v_entity(publ), partitionNotMatched, &matchArg);
                v_gidRelease(gid, v_objectKernel(w));
            }
        } else { /* w == NULL && r != NULL */
            /* for every partition of r check if it matches
               with the offered partition.
            */
            matchArg.matched = FALSE;
            matchArg.partition = oInfo->partition.name;

            /*Claim subscriber first before walking over relations*/
            gid = v_publicGid(v_public(v_reader(r)->subscriber));
            publ = v_gidClaim(gid, v_objectKernel(r));

            if (publ) {
                v_entityWalkEntities(v_entity(publ), partitionNotMatched, &matchArg);
                v_gidRelease(gid, v_objectKernel(r));
            }
        }
    }
    return matchArg.matched;
}

#if 0
/* spliced->builtinDataMutex must be locked before calling this function */
static c_iter
lookupMatchingReadersByTopic(
    v_spliced spliced,
    struct v_publicationInfo *oInfo)
{
    c_iter requestedMessages;
    q_expr qExpr;
    c_value params[1];
    c_query q;
    c_bool proceed;

    if (spliced->builtinData[V_SUBSCRIPTIONINFO_ID] == NULL) {
        return NULL; /* this builtin topic is disabled */
    }

    requestedMessages = NULL;
    qExpr = (q_expr)q_parse("userData.topic_name like %0");
    params[0] = c_stringValue(oInfo->topic_name);
    q = c_queryNew((c_collection)spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                   qExpr, params);
    q_dispose(qExpr);
    proceed = c_readAction(q, (c_action)lookupAction, (c_voidp)&requestedMessages);
    assert(proceed == TRUE);
    OS_UNUSED_ARG(proceed);
    c_free(q);

    return requestedMessages;
}
#else

typedef struct getMatchingReaderArg_s {
    c_iter iter;
    c_char *topicName;
    v_kernel kernel;
} *getMatchingReaderArg;

static c_bool
getMatchingReader(
    c_object o,
    c_voidp arg)
{
    getMatchingReaderArg a = (getMatchingReaderArg)arg;
    struct v_subscriptionInfo *info = v_builtinSubscriptionInfoData(o);

    if (o != NULL) {
        if (strcmp(a->topicName, info->topic_name) == 0) {
            a->iter = c_iterAppend(a->iter, c_keep(o));
        }
    }
    return TRUE;
}

/* spliced->builtinDataMutex must be locked before calling this function */
static c_iter
lookupMatchingReadersByTopic(
    v_spliced spliced,
    struct v_publicationInfo *oInfo)
{
    struct getMatchingReaderArg_s arg;

    arg.iter = NULL;
    arg.topicName = oInfo->topic_name;
    arg.kernel = v_objectKernel(spliced);

    (void)c_walk((c_collection)spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                 (c_action)getMatchingReader,
                 (c_voidp)&arg);

    return arg.iter;
}
#endif

#if 0
/* spliced->builtinDataMutex must be locked before calling this function */
static c_iter
lookupMatchingWritersByTopic(
    v_spliced spliced,
    struct v_subscriptionInfo *rInfo)
{
    c_iter offeredMessages;
    q_expr qExpr;
    c_value params[1];
    c_query q;
    c_bool proceed;

    offeredMessages = NULL;
    qExpr = (q_expr)q_parse("userData.topic_name like %0");
    params[0] = c_stringValue(rInfo->topic_name);
    q = c_queryNew((c_collection)spliced->builtinData[V_PUBLICATIONINFO_ID],
                   qExpr, params);
    q_dispose(qExpr);
    proceed = c_readAction(q, (c_action)lookupAction, (c_voidp)&offeredMessages);
    assert(proceed == TRUE);
    c_free(q);

    return offeredMessages;
}
#else

typedef struct getMatchingWriterArg_s {
    c_iter iter;
    c_char *topicName;
    v_kernel kernel;
} *getMatchingWriterArg;

static c_bool
getMatchingWriter(
    c_object o,
    c_voidp arg)
{
    if (o != NULL) {
        getMatchingWriterArg a = (getMatchingWriterArg)arg;
        struct v_publicationInfo *info = (struct v_publicationInfo *) ((v_message) o + 1);
        if (strcmp(a->topicName, info->topic_name) == 0) {
            a->iter = c_iterAppend(a->iter, c_keep(o));
        }
    }
    return TRUE;
}

/* spliced->builtinDataMutex must be locked before calling this function */
static c_iter
lookupMatchingWritersByTopic(
    v_spliced spliced,
    struct v_subscriptionInfo *rInfo)
{
    struct getMatchingWriterArg_s arg;

    arg.iter = NULL;
    arg.topicName = rInfo->topic_name;
    arg.kernel = v_objectKernel(spliced);

    c_walk((c_collection)spliced->builtinData[V_PUBLICATIONINFO_ID],
           (c_action)getMatchingWriter,
           (c_voidp)&arg);

    return arg.iter;
}
#endif
static c_bool
checkOfferedRequested(
    struct v_publicationInfo *oInfo,
    struct v_subscriptionInfo *rInfo,
    v_policyId *compatible)
{
    v_policyId id;
    c_bool result;

    assert(oInfo != NULL);
    assert(rInfo != NULL);
    assert(compatible != NULL);

    for (id = 0; id < V_POLICY_ID_COUNT; id++) {
        compatible[id] = TRUE;
    }
    result = TRUE;

    if (!v_reliabilityPolicyCompatible(oInfo->reliability,
                                       rInfo->reliability)) {
        compatible[V_RELIABILITYPOLICY_ID] = FALSE;
        result = FALSE;
    }

    if (!v_durabilityPolicyCompatible(oInfo->durability,
                                      rInfo->durability)) {
        compatible[V_DURABILITYPOLICY_ID] = FALSE;
        result = FALSE;
    }

    if (!v_presentationPolicyCompatible(oInfo->presentation,
                                        rInfo->presentation)) {
        compatible[V_PRESENTATIONPOLICY_ID] = FALSE;
        result = FALSE;
    }

    if (!v_latencyPolicyCompatible(oInfo->latency_budget,
                                   rInfo->latency_budget)) {
        compatible[V_LATENCYPOLICY_ID] = FALSE;
        result = FALSE;
    }

    if (!v_orderbyPolicyCompatible(oInfo->destination_order,
                                   rInfo->destination_order)) {
        compatible[V_ORDERBYPOLICY_ID] = FALSE;
        result = FALSE;
    }

    if (!v_deadlinePolicyCompatible(oInfo->deadline,
                                    rInfo->deadline)) {
        compatible[V_DEADLINEPOLICY_ID] = FALSE;
        result = FALSE;
    }

    if (!v_livelinessPolicyCompatible(oInfo->liveliness,
                                      rInfo->liveliness)) {
        compatible[V_LIVELINESSPOLICY_ID] = FALSE;
        result = FALSE;
    }

    if (!v_ownershipPolicyCompatible(oInfo->ownership,
                                     rInfo->ownership)) {
        compatible[V_OWNERSHIPPOLICY_ID] = FALSE;
        result = FALSE;
    }

    return result;
}

static c_bool
checkTopicConsistency(
    v_spliced spliced,
    v_topic topic,
    struct v_topicInfo *info)
{
    c_bool consistent;
    v_kernel kernel;
    c_char *type_name;
    v_topicQos qos1, qos2;

    assert(C_TYPECHECK(spliced,v_spliced));
    assert(C_TYPECHECK(topic,v_topic));
    assert(info != NULL);

    kernel = v_objectKernel(spliced);

    /* Now check if the local topic is consistent with the remote
     * topic (in info)
     */
    if (v_topicCrcOfName(topic) == info->key.systemId) {
        if (v_topicCrcOfTypeName(topic) != info->key.localId) {
            /* inconsistent, so no further checking needed .... */
            consistent = FALSE;
            type_name = c_metaScopedName(c_metaObject(v_topicDataType(topic)));
            OS_REPORT(OS_INFO, "v_spliced", V_RESULT_ILL_PARAM,
                        "Inconsistent topic %s on types: %s(%d), "
                        "but %s(%d) defined",
                        info->name,
                        info->type_name, info->key.localId,
                        type_name, v_topicCrcOfTypeName(topic));
            os_free(type_name);
        } else {
            /* check topicQos */
            qos1 = v_topicQosFromTopicInfo(c_getBase(kernel), info);
            qos2 = v_topicGetQos(topic);
            if (qos1 && qos2) {
                c_bool valid;
                consistent = TRUE;
                valid = v_durabilityPolicyIEqual(qos1->durability, qos2->durability);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Durability' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_durabilityServicePolicyIEqual(qos1->durabilityService, qos2->durabilityService);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'DurabilityService' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_deadlinePolicyIEqual(qos1->deadline, qos2->deadline);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Deadline' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_latencyPolicyIEqual(qos1->latency, qos2->latency);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Latency' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_livelinessPolicyIEqual(qos1->liveliness, qos2->liveliness);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Liveliness' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_reliabilityPolicyIEqual(qos1->reliability, qos2->reliability);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Reliability' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_orderbyPolicyIEqual(qos1->orderby, qos2->orderby);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'OrderBy' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_historyPolicyIEqual(qos1->history, qos2->history);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'History' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_resourcePolicyIEqual(qos1->resource, qos2->resource);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Resource' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_transportPolicyIEqual(qos1->transport, qos2->transport);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Transport' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_lifespanPolicyIEqual(qos1->lifespan, qos2->lifespan);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Lifespan' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_ownershipPolicyIEqual(qos1->ownership, qos2->ownership);
                if (!valid) {
                    OS_REPORT(OS_WARNING, "v_spliced", V_RESULT_INCONSISTENT_QOS,
                                "Detected Unmatching QoS Policy: 'Ownership' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
            } else {
                consistent = FALSE;
            }
            c_free(qos1);
            c_free(qos2);
        }
    } else {
        consistent = FALSE;
        OS_REPORT(OS_INFO, "v_spliced", V_RESULT_ILL_PARAM,
                    "Inconsistent topic on name: local %s(%d) remote %s(%d)",
                    v_topicName(topic), v_topicCrcOfName(topic),
                    info->name, info->key.systemId);
    }
    if (consistent == FALSE) {
        v_observerLock(v_observer(topic));
        v_topicNotifyInconsistentTopic(topic);
        v_observerUnlock(v_observer(topic));
    }
    return consistent;
}

static c_bool
matchPartition(
    c_array name1,
    c_array name2)
{
    c_ulong i, j;
    c_value val;
    c_value str1, str2;

    val.kind = V_BOOLEAN;
    val.is.Boolean = FALSE;

    for (i = 0; val.is.Boolean == FALSE && i < c_arraySize(name1); i++) {
        str1 = c_stringValue(name1[i]);
        for (j = 0; val.is.Boolean == FALSE && j < c_arraySize(name2); j++) {
            str2 = c_stringValue(name2[j]);
            val = c_valueStringMatch(str1, str2);
            if (!val.is.Boolean) {
                val = c_valueStringMatch(str2, str1);
            }
        }
    }
    return val.is.Boolean;
}

static conformState
determineState(
    struct v_publicationInfo *oInfo,
    struct v_subscriptionInfo *rInfo)
{
    conformState state = CONFORM_STATE_UNKNOWN;

#define COMPATIBLE_QOS(offered, requested) \
    (v_reliabilityPolicyCompatible((offered)->reliability, (requested)->reliability) && \
     v_durabilityPolicyCompatible((offered)->durability, (requested)->durability) && \
     v_presentationPolicyCompatible((offered)->presentation, (requested)->presentation) && \
     v_latencyPolicyCompatible((offered)->latency_budget, (requested)->latency_budget) && \
     v_orderbyPolicyCompatible((offered)->destination_order, (requested)->destination_order) && \
     v_deadlinePolicyCompatible((offered)->deadline, (requested)->deadline) && \
     v_livelinessPolicyCompatible((offered)->liveliness, (requested)->liveliness) && \
     v_ownershipPolicyCompatible((offered)->ownership, (requested)->ownership))

    if ((oInfo != NULL) && (rInfo != NULL)) {
        if (matchPartition(oInfo->partition.name, rInfo->partition.name) == TRUE) {
            if (COMPATIBLE_QOS(oInfo, rInfo) == TRUE) {
                state = CONFORM_STATE_OK;
            } else {
                state = CONFORM_STATE_MATCH_NOT_COMPATIBLE;
            }
        } else {
            state = CONFORM_STATE_NO_MATCH;
        }
    }
#undef COMPATIBLE_QOS

    return state;
}

static void
notifyIncompatibleQos(
    struct v_publicationInfo *oInfo,
    struct v_subscriptionInfo *rInfo,
    v_writer w,
    v_dataReader r)
{
    v_policyId policyCompatible[V_POLICY_ID_COUNT];
    v_policyId id;

    if (checkOfferedRequested(oInfo, rInfo, policyCompatible) == FALSE) {
        for (id = 0; id < V_POLICY_ID_COUNT; id++) {
            if (policyCompatible[id] == FALSE) {
                if (w != NULL) {
                    v_writerNotifyIncompatibleQos(w, id);
                }
                if (r != NULL) {
                    v_dataReaderNotifyIncompatibleQos(r, id, oInfo->key);
                }
            }
        }
    }
}

/* returns 0, when no DCPSSubscriptionInfo sample was available.
 * Otherwise 1 is returned
 * See Wiki Design\DesignDocs\Features\Discovery for more info
 */
static int
v_splicedProcessSubscriptionInfo(
    v_spliced spliced)
{
    int result = 0;
    c_iter offeredMessages;
    v_dataReaderSample rSample;
    v_message msg;
    v_message offeredMsg;
    v_message oldMsg;
    v_message found;

    struct v_subscriptionInfo *rInfo;
    struct v_subscriptionInfo *oldInfo;
    struct v_publicationInfo *oInfo;
    v_kernel kernel;
    v_writer w;
    v_dataReader r;
    enum v_statusLiveliness newLivState;
    enum v_statusLiveliness oldLivState;
    conformState oldState;
    conformState newState;
    c_bool cont = TRUE;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    /* Try to take one subscriptionInfo message from the built-in DataRaeder.
     */
    rSample = NULL;
    if (spliced->readers[V_SUBSCRIPTIONINFO_ID] != NULL) {
        (void)v_dataReaderTake(spliced->readers[V_SUBSCRIPTIONINFO_ID], V_MASK_ANY,
                               takeOne,
                               &rSample, OS_DURATION_ZERO);
    }
    /* If a subscriptionInfo message exists then process it.
     */
    if (rSample != NULL) {
        result = 1;

        msg = v_dataReaderSampleMessage(rSample);
        rInfo = (struct v_subscriptionInfo *) (msg + 1);

        if (v_stateTest(v_readerSample(rSample)->sampleState, L_DISPOSED)) {

            c_mutexLock(&spliced->builtinDataMutex);

            /* The subscription is disposed so the following
             * code will remove and free the registration.
             */
            oldMsg = c_remove(spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                              msg, NULL, NULL);

            if (oldMsg == NULL) {
                /* The instance is not yet know no need to
                 * take action.
                 */
                c_mutexUnlock(&spliced->builtinDataMutex);
            } else {
                /* Notify the nodal delivery service about any synchronous
                 * DataReader topology changes.
                 */
                 oldInfo = v_builtinSubscriptionInfoData(oldMsg);
                 if (oldInfo->reliability.synchronous) {
                     v_deliveryServiceUnregister(kernel->deliveryService, oldMsg);
                 }

                 /* Notify matching writers that a matching subscription disappeared */
                 offeredMessages = lookupMatchingWritersByTopic(spliced, oldInfo);

                 c_mutexUnlock(&spliced->builtinDataMutex);

                 offeredMsg = c_iterTakeFirst(offeredMessages);
                 while (offeredMsg != NULL) {
                     oInfo = v_builtinPublicationInfoData(offeredMsg);

                     /* Determine old state.
                      * Not interested in new state, disposing.
                      */
                     oldState = determineState(oInfo, oldInfo);
                     if (oldState == CONFORM_STATE_OK) {
                         w = v_writer(v_gidClaim(oInfo->key, kernel));
                         if (w != NULL) {
                             v_writerNotifyPublicationMatched(w, oldInfo->key, TRUE);
                             v_gidRelease(oInfo->key, kernel);
                         }
                     }

                     c_free(offeredMsg);
                     offeredMsg = c_iterTakeFirst(offeredMessages);
                 }
                 c_iterFree(offeredMessages);
            }
        } else {
            /* Notify the nodal delivery service about any synchronous
             * DataReader topology changes.
             */
            if (rInfo->reliability.synchronous) {
                v_deliveryServiceRegister(kernel->deliveryService,msg);
            }

            c_mutexLock(&spliced->builtinDataMutex);

            oldMsg = c_replace(spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                               msg, NULL, NULL);

            if (oldMsg == NULL) {
                /* A new registration.
                 */
                oldInfo = NULL;
            } else if (oldMsg->sequenceNumber >= msg->sequenceNumber) {
                /* Sample already received, ignore duplicate.
                 */
                found = c_replace(spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                                oldMsg, NULL, NULL);
                c_free(found);
                cont = FALSE;
            } else {
                /* An update of a registration.
                 */
                oldInfo = v_builtinSubscriptionInfoData(oldMsg);
            }

            if (cont != TRUE) {
                c_mutexUnlock(&spliced->builtinDataMutex);
            } else {
                r = gidClaimDataReader(rInfo->key, kernel);
                /* Check if there are matching DataWriters i.e. with
                 * compatible qos policies.
                 */

                /* Read all publicationInfo, with the same topic name, this way
                 * all remote publications are also taken into account.
                 */
                offeredMessages = lookupMatchingWritersByTopic(spliced, rInfo);

                c_mutexUnlock(&spliced->builtinDataMutex);

                offeredMsg = c_iterTakeFirst(offeredMessages);
                while (offeredMsg != NULL) {
                    oInfo = v_builtinPublicationInfoData(offeredMsg);

                    oldState = determineState(oInfo, oldInfo);
                    newState = determineState(oInfo, rInfo);

                    if ((oldState != CONFORM_STATE_OK) &&
                        (newState == CONFORM_STATE_OK)) {
                        w = v_writer(v_gidClaim(oInfo->key, kernel));
                        if (w != NULL) {
                            /* notify the writer that a new subscription matched */
                            v_writerNotifyPublicationMatched(w, rInfo->key, FALSE);
                            v_gidRelease(oInfo->key, kernel);
                        }
                        if (r != NULL) {
                            /* notify the new reader that an existing publication matches */
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, FALSE, oInfo, FALSE);
                        }
                    } else if ((oldState == CONFORM_STATE_OK) &&
                               (newState != CONFORM_STATE_OK)) {
                        w = v_writer(v_gidClaim(oInfo->key, kernel));
                        if (w != NULL) {
                            /* notify the writer that a new subscription no longer matched */
                            v_writerNotifyPublicationMatched(w, rInfo->key, TRUE);
                            v_gidRelease(oInfo->key, kernel);
                        }
                        if (r != NULL) {
                            /* notify the new reader that an existing publication no longer matches */
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, TRUE, oInfo, FALSE);
                        }
                    }
                    if (newState == CONFORM_STATE_MATCH_NOT_COMPATIBLE) {
                        w = v_writer(v_gidClaim(oInfo->key, kernel));
                        notifyIncompatibleQos(oInfo, rInfo, w, r);
                        if (w != NULL) {
                            v_gidRelease(oInfo->key, kernel);
                        }
                    }

                    if (r != NULL) {
                        oldLivState = CONFORM_STATE_TO_LIVELINESS_STATE(oldState, oInfo->alive);
                        newLivState = CONFORM_STATE_TO_LIVELINESS_STATE(newState, oInfo->alive);
                        v_dataReaderNotifyLivelinessChanged(r, oInfo->key,
                                                            oldLivState,
                                                            newLivState,
                                                            offeredMsg);
                    }

                    c_free(offeredMsg);
                    offeredMsg = c_iterTakeFirst(offeredMessages);
                }
                c_iterFree(offeredMessages);

                if (r != NULL) {
                    v_gidRelease(rInfo->key, kernel);
                }
            }
        }

        c_free(v_dataReaderSampleInstance(rSample));
        c_free(rSample); /* msg is freed here */
        c_free(oldMsg);
        rSample = NULL;

    }

    return result;
}

/* returns 0, when no DCPSPublicationInfo sample was available.
 * Otherwise 1 is returned
 * See Wiki Design\DesignDocs\Features\Discovery for more info
 */
static int
v_splicedProcessPublicationInfo
(
    v_spliced spliced)
{
    int result = 0;
    v_dataReaderSample oSample;
    c_iter requestedMessages = NULL;
    v_message msg;
    v_message reqMsg;
    v_message oldMsg;
    v_message found;
    struct v_subscriptionInfo *rInfo;
    struct v_publicationInfo *oInfo;
    struct v_publicationInfo *oldInfo = NULL;
    v_kernel kernel;
    v_writer w;
    v_dataReader r;
    enum v_statusLiveliness newLivState;
    enum v_statusLiveliness oldLivState;
    conformState oldState;
    conformState newState;
    c_bool isLocal = TRUE;
    struct v_owner ownership;
    c_bool cont = TRUE;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    oSample = NULL;
    (void)v_dataReaderTake(spliced->readers[V_PUBLICATIONINFO_ID], V_MASK_ANY,
                           takeOne, &oSample, OS_DURATION_ZERO);

    /* For every publication check if the qos of a matching reader is
     * compatible.
     */
    if (oSample != NULL) {
        result = 1;

        /* Read all subscriptionInfo, with the same topic name, this way
           all remote subscriptions are also taken into account.
         */
        msg = v_dataReaderSampleMessage(oSample);
        oInfo = (struct v_publicationInfo *) (msg + 1);

        if (v_dataReaderSampleInstanceStateTest(oSample, L_DISPOSED)) {
            c_bool validSample = v_dataReaderSampleStateTest(oSample, L_VALIDDATA);
            c_bool isImplicit = v_messageStateTest(msg, L_IMPLICIT);

            c_mutexLock(&spliced->builtinDataMutex);
            oldMsg = c_remove(spliced->builtinData[V_PUBLICATIONINFO_ID],
                              msg, NULL, NULL);
            /* If there is no predecessor but the sample is valid, then interpret the sample itself.
             * (This is for example the case when the builtin topics have been disabled, in which
             * case only the dispose message will be transmitted as a VOLATILE sample.)
             */
            if (oldMsg == NULL &&
                validSample &&
                !kernel->builtin->kernelQos->builtin.v.enabled) {
                oldMsg = msg;
            }
            if (oldMsg != NULL) {
                oldInfo = v_builtinPublicationInfoData(oldMsg);

                /* the resulting readers are not matched on partition so the readers that do not belong
                 * to the required partition need to be filtered out later on.
                 * This is because of the partition field being an internal sequence so no expression
                 * can be done on it*/
                requestedMessages = lookupMatchingReadersByTopic(spliced, oldInfo);
            }
            c_mutexUnlock(&spliced->builtinDataMutex);

            if (oldInfo == NULL) {
                /* There is no old message and the dispose in received as an
                 * invalid-sample.  So we cannot select matching readers because the
                 * partition-name in the publication-info isn't present (non-key field).
                 * Local writers never get here because the kernel uses writeDispose.
                 *
                 * DDSI publishes built-in topics using a local writer and takes
                 * responsibility for cleaning up (because it has all the information
                 * readily available and getting the L_IMPLICIT bits right otherwise is
                 * hard), so when the message was written by remote writer, it must be a
                 * "traditional" built-in topic.
                 */
                if (msg->writerGID.systemId != kernel->GID.systemId) {
                    doCleanupPublication(spliced, oInfo, msg->writeTime, validSample, FALSE, isImplicit);
                }

            } else {
                if(oInfo->key.systemId != kernel->GID.systemId) {
                   /* If the message is disposed, the writer is no longer alive,
                    * so cleanup all resources taken by this writer. But only if this
                    * writer is a remote writer!
                    * DO NOT USE local time, but use the production time of the
                    * builtin topic! This is needed, since the time might not be
                    * aligned properly!
                    */
                    isLocal = FALSE;
                }

                if (isLocal || msg->writerGID.systemId != kernel->GID.systemId) {
                    doCleanupPublication(spliced, oldInfo, msg->writeTime, validSample, isLocal, isImplicit);
                }

                while ((reqMsg = c_iterTakeFirst(requestedMessages)) != NULL)
                {
                    rInfo = v_builtinSubscriptionInfoData(reqMsg);

                    oldState = determineState(oldInfo, rInfo);

                    r = gidClaimDataReader(rInfo->key, kernel);

                    if (oldState == CONFORM_STATE_OK) {
                        if (r != NULL) {
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, TRUE, oInfo, isImplicit);
                        }
                    }
                    oldLivState = CONFORM_STATE_TO_LIVELINESS_STATE(oldState, oldInfo->alive);
                    newLivState = V_STATUSLIVELINESS_DELETED;

                    if (r != NULL) {
                        v_dataReaderNotifyLivelinessChanged(r, oInfo->key,
                        oldLivState, newLivState,
                        msg);
                        v_gidRelease(rInfo->key, kernel);
                    }

                    c_free(reqMsg);
                }
                c_iterFree(requestedMessages);
            }
            v_kernelNotifyGroupCoherentPublication(kernel, msg);
        } else {
            v_kernelNotifyGroupCoherentPublication(kernel, msg);
            c_mutexLock(&spliced->builtinDataMutex);

            oldMsg = c_replace(spliced->builtinData[V_PUBLICATIONINFO_ID], msg, NULL, NULL);

            /* Do not free oldMsg yet!
             * We need it to determine the previous liveliness state
             * of the writer.
             */

            if (oldMsg == NULL) {
                /* A new registration.
                 */
                oldInfo = NULL;
            } else if (oldMsg->sequenceNumber >= msg->sequenceNumber) {
                /* Sample already received, ignore duplicate.
                 */
                found = c_replace(spliced->builtinData[V_PUBLICATIONINFO_ID],
                                 oldMsg, NULL, NULL);
                c_free(found);
                cont = FALSE;
            } else {
                /* An update of a registration.
                 */

                oldInfo = v_builtinPublicationInfoData(oldMsg);
            }
            if (cont == TRUE) {
                assert(v_dataReaderSampleStateTest(oSample, L_VALIDDATA));
                w = v_writer(v_gidClaim(oInfo->key, kernel));
                /* the resulting readers are not matched on partition so the readers that do not belong
                 * to the required partition need to be filtered out later on.
                 * This is because of the partition field being an internal sequence so no expression
                 * can be done on it*/
                requestedMessages = lookupMatchingReadersByTopic(spliced, oInfo);
            }
            c_mutexUnlock(&spliced->builtinDataMutex);

            if (cont == TRUE) {
                while ((reqMsg = c_iterTakeFirst(requestedMessages)) != NULL)
                {
                    rInfo = v_builtinSubscriptionInfoData(reqMsg);

                    oldState = determineState(oldInfo, rInfo);
                    newState = determineState(oInfo, rInfo);

                    r = gidClaimDataReader(rInfo->key, kernel);

                    if ((oldState != CONFORM_STATE_OK) &&
                        (newState == CONFORM_STATE_OK)) {
                        if (r != NULL) {
                            /* Notify the reader that a new publication matched */
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, FALSE, oInfo, FALSE);
                        }
                        if (w != NULL) {
                            /* Notify the new writer that an existing subscription matches */
                            v_writerNotifyPublicationMatched(w, rInfo->key, FALSE);
                        }
                    } else if ((oldState == CONFORM_STATE_OK) &&
                               (newState == CONFORM_STATE_OK)) {
                        if (r != NULL) {
                            /* oldInfo can't be NULL here.
                             */
                            if (oldInfo->ownership_strength.value != oInfo->ownership_strength.value) {
                                /* Notify reader ownership strength might have changed for it's instances. */
                                ownership.exclusive = (oInfo->ownership.kind == V_OWNERSHIP_EXCLUSIVE ? TRUE : FALSE);
                                ownership.gid = oInfo->key;
                                ownership.strength = oInfo->ownership_strength.value;
                                v_dataReaderNotifyOwnershipStrengthChanged (r, &ownership);
                            }
                        }
                    } else if ((oldState == CONFORM_STATE_OK) &&
                               (newState != CONFORM_STATE_OK)) {
                        if (r != NULL) {
                            /* Notify the reader that a publication no longer matches */
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, TRUE, oInfo, FALSE);
                        }
                        if (w != NULL) {
                            /* Notify the new writer that an existing subscription matches */
                            v_writerNotifyPublicationMatched(w, rInfo->key, TRUE);
                        }
                    }
                    if (newState == CONFORM_STATE_MATCH_NOT_COMPATIBLE) {
                        notifyIncompatibleQos(oInfo, rInfo, w, r);
                    }

                    if (r != NULL) {
                        oldLivState = CONFORM_STATE_TO_LIVELINESS_STATE(oldState, oldInfo->alive);
                        newLivState = CONFORM_STATE_TO_LIVELINESS_STATE(newState, oInfo->alive);
                        v_dataReaderNotifyLivelinessChanged(r, oInfo->key,
                                                            oldLivState, newLivState,
                                                            msg);
                        v_gidRelease(rInfo->key, kernel);
                    }

                    c_free(reqMsg);
                }
                c_iterFree(requestedMessages);
                if (w != NULL) {
                    v_gidRelease(oInfo->key, kernel);
                }
            }
        }
        if (oldMsg != msg) {
            c_free(oldMsg);
        }

        c_free(v_dataReaderSampleInstance(oSample));
        c_free(oSample);
    }
    return result;
}

/* returns 0, when no DCPSTopicInfo sample was available.
 * Otherwise 1 is returned */
static int
v_splicedProcessTopicInfo(
    v_spliced spliced)
{
    c_iter samples;
    c_bool proceed;
    v_dataReaderSample s;
    v_message m;
    v_message oldMsg;
    struct v_topicInfo *info;
    c_iter topics;
    v_topic aTopic;
    v_kernel kernel;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    samples = NULL;
    (void)v_dataReaderTake(spliced->readers[V_TOPICINFO_ID], V_MASK_ANY,
                               readerTakeAction, &samples, OS_DURATION_ZERO);
    proceed = FALSE;

    if (samples) {
        c_mutexLock(&spliced->builtinDataMutex);

        kernel = v_objectKernel(spliced);
        s = v_dataReaderSample(c_iterTakeFirst(samples));
        while (s != NULL) {
            proceed = TRUE;
            m = v_dataReaderSampleMessage(s);
            info = (struct v_topicInfo *) (m + 1);

            if (v_stateTest(v_readerSample(s)->sampleState, L_DISPOSED)) {
                oldMsg = c_remove(spliced->builtinData[V_TOPICINFO_ID],
                                  m, NULL, NULL);
            } else {
                oldMsg = c_replace(spliced->builtinData[V_TOPICINFO_ID],
                                   m, NULL, NULL);
            }
            if (oldMsg != NULL) {
                c_free(oldMsg);
            }

            topics = v_resolveTopics(kernel, info->name);
            assert(c_iterLength(topics) <= 1);
            aTopic = v_topic(c_iterTakeFirst(topics));
            if (aTopic == NULL) {
                /* No topic with that name locally defined, so define it now! */
                v_topicImplNewFromTopicInfo(v_objectKernel(spliced), info, FALSE);
            } else {
                do {
                    checkTopicConsistency(spliced, aTopic, info);
                    c_free(aTopic);
                    aTopic = v_topic(c_iterTakeFirst(topics));
                } while (aTopic != NULL);
            }
            c_iterFree(topics);
            c_free(s);
            s = v_dataReaderSample(c_iterTakeFirst(samples));
        }

        c_mutexUnlock(&spliced->builtinDataMutex);
    }
    assert(c_iterLength(samples) == 0);
    c_iterFree(samples);

    if (proceed) {
        return 1;
    } else {
        return 0;
    }
}


/* returns 0, when no DCPSTypeInfo sample was available.
 * Otherwise 1 is returned */
static int
v_splicedProcessTypeInfo(
    v_spliced spliced)
{
    c_iter samples;
    c_bool proceed;
    v_dataReaderSample s;
    v_message m;
    struct v_typeInfo *info;
    v_typeRepresentation tr;
    v_kernel kernel;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    samples = NULL;
    (void)v_dataReaderTake(spliced->readers[V_TYPEINFO_ID], V_MASK_ANY,
                               readerTakeAction, &samples, OS_DURATION_ZERO);
    proceed = FALSE;

    if (samples) {
        c_mutexLock(&spliced->builtinDataMutex);

        kernel = v_objectKernel(spliced);
        s = v_dataReaderSample(c_iterTakeFirst(samples));
        while (s != NULL) {
            proceed = TRUE;
            m = v_dataReaderSampleMessage(s);
            info = v_builtinTypeInfoData(m);

            if (v_stateTest(v_readerSample(s)->sampleState, L_VALIDDATA)) {
                tr = v__typeRepresentationNew(kernel, info, FALSE);
                c_free(tr);
            }

            c_free(s);
            s = v_dataReaderSample(c_iterTakeFirst(samples));
        }
        c_mutexUnlock(&spliced->builtinDataMutex);
    }
    assert(c_iterLength(samples) == 0);
    c_iterFree(samples);

    if (proceed) {
        return 1;
    } else {
        return 0;
    }
}


struct matchHeartbeatHelper {
        c_ulong systemId; /* systemId from key of participantInfo sample */
        c_bool found;
};

static v_actionResult
matchHeartbeatAction(
    c_object o,
    c_voidp arg)
{
    v_actionResult result = 0;
    struct matchHeartbeatHelper *helper;
    struct v_heartbeatInfo *hbInfo;

    assert(arg);
    helper = (struct matchHeartbeatHelper*)arg;

    if (o) {
        hbInfo = v_builtinHeartbeatInfoData(v_dataReaderSampleMessage(o));
        if (helper->systemId != v_gidSystemId(hbInfo->id)) {
            v_actionResultSet(result, V_PROCEED); /* Proceed when no match is found */
        } else { /* else a match is found, so do not proceed */
            helper->found = TRUE;
        }
        /* Ensure this routine does not influence sample state at all */
        v_actionResultSet(result, V_SKIP);
    }
    return result;
}

int
v_splicedProcessParticipantInfo(
    v_spliced spliced)
{
    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    if (spliced->readers[V_PARTICIPANTINFO_ID] != NULL) {
        struct matchHeartbeatHelper helper;
        v_dataReaderSample sample;
        c_iter samples = NULL;
        v_kernel kernel = v_objectKernel(spliced);

        (void)v_dataReaderTake(spliced->readers[V_PARTICIPANTINFO_ID], V_MASK_ANY, readerTakeAction, &samples, OS_DURATION_ZERO);
        sample = v_dataReaderSample(c_iterTakeFirst(samples));
        while (sample != NULL) {
            /* Only process this instance if it isn't seen before */
            if (v_dataReaderSampleStateTest(sample, L_NEW)) {
                v_message msg = v_dataReaderSampleMessage(sample);
                struct v_participantInfo *pInfo = (struct v_participantInfo *) (msg + 1);
                c_ulong systemId = v_gidSystemId(pInfo->key);

                /* Lookup heartbeat with matching systemId */
                assert(spliced->readers[V_HEARTBEATINFO_ID]);
                helper.systemId = systemId;
                helper.found = FALSE;
                (void)v_dataReaderRead(spliced->readers[V_HEARTBEATINFO_ID], V_MASK_ANY,
                                       matchHeartbeatAction, &helper, OS_DURATION_ZERO);
                if (!helper.found) {
                    /* fake heartbeat to ensure eventual cleanup if the DCPSParticipant
                       arrived via durability and the node has disappeared without us ever
                       receiving a heartbeat */
                    v_writer writer = v_builtinWriterLookup(kernel->builtin,
                                                            V_HEARTBEATINFO_ID);
                    (void)v_builtinWriteHeartbeat(
                        writer, systemId, os_timeWGet(), v_durationToOsDuration(spliced->hb.period), L_WRITE);
                }
            }
            c_free(sample);
            sample = v_dataReaderSample(c_iterTakeFirst(samples));
        }
        c_iterFree(samples);
    }

    return 0; /* is just a garbage collector, so no worries */
}

v_result
v_splicedGetMatchedSubscriptions(
    v_spliced spliced,
    v_writer w,
    v_subscriptionInfo_action action,
    c_voidp arg)
{
    v_kernel  kernel;
    v_result result;
    c_iter requestedMessages = NULL;
    v_message msg;
    v_message reqMsg;
    struct v_subscriptionInfo *rInfo;
    struct v_publicationInfo *oInfo;

    v_dataReader r;
    v_policyId compatible[V_POLICY_ID_COUNT];

    result = V_RESULT_OK;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    if (w != NULL) {
        /* Create a template message */
        msg = v_builtinCreatePublicationInfo(kernel->builtin, w);
        if (msg != NULL) {
            oInfo = (struct v_publicationInfo *) (msg + 1);
            v_gidClaim(oInfo->key, kernel);

            if (oInfo != NULL) {
                /* get topic-matching subscriptions */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingReadersByTopic(spliced, oInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL && result == V_RESULT_OK) {
                    rInfo = v_builtinSubscriptionInfoData(reqMsg);
                    r = gidClaimDataReader(rInfo->key, kernel);

                    /* if QoS are matching then add the subscription to return list */
                    if (readerWriterMatch(rInfo, r, oInfo, w) == TRUE) {
                        if (checkOfferedRequested(oInfo, rInfo, compatible) == TRUE) {
                            result = action(rInfo, arg);
                        }
                    }
                    if(r != NULL)
                    {
                        v_gidRelease(rInfo->key, kernel);
                    }
                    c_free(reqMsg);
                    reqMsg = c_iterTakeFirst(requestedMessages);
                }
                c_iterFree(requestedMessages);

                c_mutexUnlock(&spliced->builtinDataMutex);
            }
            v_gidRelease(oInfo->key, kernel);
            c_free(msg);
        }
    }
    return result;
}

v_result
v_splicedGetMatchedSubscriptionData(
    v_spliced spliced,
    v_writer w,
    v_gid subscription,
    v_subscriptionInfo_action action,
    c_voidp arg)
{
    v_kernel  kernel;
    v_result result;
    c_iter requestedMessages;
    v_message msg;
    v_message reqMsg;
    struct v_subscriptionInfo *rInfo;
    struct v_publicationInfo *oInfo;

    v_dataReader r;
    v_policyId compatible[V_POLICY_ID_COUNT];

    result = V_RESULT_ILL_PARAM;

    assert(w != NULL);
    assert(C_TYPECHECK(w,v_writer));

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    if (w != NULL) {
        /* Create a template message */
        msg = v_builtinCreatePublicationInfo(kernel->builtin, w);
        if (msg != NULL) {
            oInfo = (struct v_publicationInfo *) (msg + 1);
            v_gidClaim(oInfo->key, kernel);

            if (oInfo != NULL) {
                /* get topic-matching subscriptions */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingReadersByTopic(spliced, oInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL) {
                    rInfo = v_builtinSubscriptionInfoData(reqMsg);
                    r = gidClaimDataReader(rInfo->key, kernel);

                    if (readerWriterMatch(rInfo, r, oInfo, w) == TRUE) {
                        if (checkOfferedRequested(oInfo, rInfo, compatible) == TRUE) {
                            /*if(v_gidCompare(rInfo->key, subscription) == C_EQ)*/
                            if (rInfo->key.systemId == subscription.systemId
                                    && rInfo->key.localId == subscription.localId) {
                                result = action(rInfo, arg);
                            }
                        }
                    }
                    if(r != NULL)
                    {
                        v_gidRelease(rInfo->key, kernel);
                    }
                    c_free(reqMsg);
                    reqMsg = c_iterTakeFirst(requestedMessages);
                }
                c_iterFree(requestedMessages);

                c_mutexUnlock(&spliced->builtinDataMutex);
            }
            v_gidRelease(oInfo->key, kernel);
            c_free(msg);
        }
    }
    return result;
}

v_result
v_splicedGetMatchedPublications(
    v_spliced spliced,
    v_dataReader r,
    v_publicationInfo_action action,
    c_voidp arg)
{
    v_kernel  kernel;
    v_result result;
    c_iter requestedMessages;
    v_message msg;
    v_message reqMsg;
    struct v_subscriptionInfo *rInfo;
    struct v_publicationInfo *oInfo;

    v_writer w;
    v_policyId compatible[V_POLICY_ID_COUNT];

    result = V_RESULT_OK;

    assert(r != NULL);
    assert(C_TYPECHECK(r,v_reader));

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    if (r != NULL) {
        /* Create a template message */
        msg = v_builtinCreateSubscriptionInfo(kernel->builtin, v_reader(r));
        if (msg != NULL) {
            rInfo = (struct v_subscriptionInfo *) (msg + 1);
            v_gidClaim(rInfo->key, kernel);

            if (rInfo != NULL) {
                /* get topic-matching publications */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingWritersByTopic(spliced, rInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL && result == V_RESULT_OK) {
                    oInfo = (struct v_publicationInfo *) (reqMsg + 1);
                    w = v_writer(v_gidClaim(oInfo->key, kernel));
                    /* if QoS are matching then add the publication to return list */
                    if (readerWriterMatch(rInfo, r, oInfo, w) == TRUE) {
                        if (checkOfferedRequested(oInfo, rInfo, compatible) == TRUE) {
                            result = action(oInfo, arg);
                        }
                    }
                    if(w != NULL)
                    {
                        v_gidRelease(oInfo->key, kernel);
                    }
                    c_free(reqMsg);
                    reqMsg = c_iterTakeFirst(requestedMessages);
                }
                c_iterFree(requestedMessages);

                c_mutexUnlock(&spliced->builtinDataMutex);
            }
            v_gidRelease(rInfo->key, kernel);
            c_free(msg);
        }
    }
    return result;
}

v_result
v_splicedGetMatchedPublicationData(
    v_spliced spliced,
    v_dataReader r,
    v_gid publication,
    v_publicationInfo_action action,
    c_voidp arg)
{
    v_kernel  kernel;
    v_result  result;
    c_iter requestedMessages;
    v_message msg;
    v_message reqMsg;
    struct v_subscriptionInfo *rInfo;
    struct v_publicationInfo *oInfo;
    v_writer w;
    v_policyId compatible[V_POLICY_ID_COUNT];

    result = V_RESULT_ILL_PARAM;

    assert(r != NULL);
    assert(C_TYPECHECK(r,v_dataReader));

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    if (r != NULL) {
        /* Create a template message */
        msg = v_builtinCreateSubscriptionInfo(kernel->builtin, v_reader(r));
        if (msg != NULL) {
            rInfo = (struct v_subscriptionInfo *) (msg + 1);
            v_gidClaim(rInfo->key, kernel);

            if (rInfo != NULL) {
                /* get topic-matching publications */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingWritersByTopic(spliced, rInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL) {
                    oInfo = (struct v_publicationInfo *) (reqMsg + 1);
                    w = v_writer(v_gidClaim(oInfo->key, kernel));

                    if (readerWriterMatch(rInfo, r, oInfo, w) == TRUE) {
                        if (checkOfferedRequested(oInfo, rInfo, compatible) == TRUE) {
                            /*if(v_gidCompare(oInfo->key, publication) == C_EQ)*/
                            if (oInfo->key.systemId == publication.systemId
                                    && oInfo->key.localId == publication.localId) {
                                result = action(oInfo, arg);
                            }
                        }
                    }
                    if(w != NULL)
                    {
                        v_gidRelease(oInfo->key, kernel);
                    }
                    c_free(reqMsg);
                    reqMsg = c_iterTakeFirst(requestedMessages);
                }
                c_iterFree(requestedMessages);

                c_mutexUnlock(&spliced->builtinDataMutex);
            }
            v_gidRelease(rInfo->key, kernel);
            c_free(msg);
        }
    }
    return result;
}

static c_char *
messageKeyExpr(
    v_topic topic)
{
    c_string fieldName;
    c_char *keyExpr;
    c_ulong nrOfKeys;
    os_size_t totalSize;
    c_iter keyNames;

    assert(C_TYPECHECK(topic,v_topic));

    keyExpr = v_topicKeyExpr(topic);

#define PREFIX "userData."

    keyNames = c_splitString(keyExpr,", \t");
    nrOfKeys = c_iterLength(keyNames);
    totalSize = strlen(keyExpr) + (nrOfKeys * strlen(PREFIX)) + 1;
    if (nrOfKeys>0) {
        keyExpr = (char *)os_malloc(totalSize);
        keyExpr[0] = 0;
        fieldName = c_iterTakeFirst(keyNames);
        while (fieldName != NULL) {
            os_strcat(keyExpr,"userData.");
            os_strcat(keyExpr,fieldName);
            os_free(fieldName);
            fieldName = c_iterTakeFirst(keyNames);
            if (fieldName) {
                os_strcat(keyExpr,",");
            }
        }
        c_iterFree(keyNames);
    } else {
        keyExpr = NULL;
    }

#undef PREFIX

    return keyExpr;
}

#define _INIT_BUILTIN_DATA_(_id, _name) \
    topic = v_builtinTopicLookup(kernel->builtin, _id); \
    assert(topic); \
    str = messageKeyExpr(topic); \
    spliced->builtinData[_id] = c_tableNew(v_topicMessageType(topic), str); \
    assert(spliced->builtinData[_id]); \
    os_free(str);\
    expr = q_parse("select * from " _name);\
    spliced->readers[_id] = v_dataReaderNew(spliced->builtinSubscriber,\
                                   _name "Reader", \
                                   expr, NULL, rQos, TRUE);\
    q_dispose(expr); \
    v_observableAddObserver(v_observable(spliced->readers[_id]),\
                            v_observer(spliced->ws), NULL)

static void
v_splicedManageKernel(
    v_spliced spliced)
{
    v_kernel kernel;
    c_char *str;
    v_topic topic;
    q_expr expr;
    c_value params[1];
    v_subscriberQos sQos;
    v_readerQos rQos;
    c_bool builtinEnabled = TRUE;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    spliced->ws = v_waitsetNew(v_participant(spliced));
    v_observerSetEventMask(v_observer(spliced->ws),
        V_EVENT_OBJECT_DESTROYED |
        V_EVENT_DATA_AVAILABLE   |
        V_EVENT_TRIGGER          |
        V_EVENT_TERMINATE);

    c_mutexLock(&spliced->builtinDataMutex);

    sQos = v_subscriberQosNew(kernel, NULL);
    sQos->presentation.v.access_scope = V_PRESENTATION_TOPIC;
    c_free(sQos->partition.v);
    sQos->partition.v = c_stringNew(c_getBase(c_object(kernel)), V_BUILTIN_PARTITION);
    sQos->entityFactory.v.autoenable_created_entities = TRUE;
    spliced->builtinSubscriber = v_subscriberNew(v_participant(spliced),
                                                 "Builtin subscriber", sQos, TRUE);
    v_subscriberQosFree(sQos);

    rQos = v_readerQosNew(kernel, NULL);
    rQos->durability.v.kind = V_DURABILITY_TRANSIENT;
    rQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
    rQos->history.v.kind = V_HISTORY_KEEPALL;
    rQos->history.v.depth = V_LENGTH_UNLIMITED;

    _INIT_BUILTIN_DATA_(V_TOPICINFO_ID, V_TOPICINFO_NAME);
    _INIT_BUILTIN_DATA_(V_TYPEINFO_ID,  V_TYPEINFO_NAME);

    if (kernel->qos->builtin.v.enabled) {
        _INIT_BUILTIN_DATA_(V_SUBSCRIPTIONINFO_ID, V_SUBSCRIPTIONINFO_NAME);
        _INIT_BUILTIN_DATA_(V_PUBLICATIONINFO_ID, V_PUBLICATIONINFO_NAME);
    } else {
        rQos->durability.v.kind = V_DURABILITY_VOLATILE;
        _INIT_BUILTIN_DATA_(V_PUBLICATIONINFO_ID, V_PUBLICATIONINFO_NAME);
    }

    kernel->deliveryService = v_deliveryServiceNew(
                                      spliced->builtinSubscriber,
                                      "deliveryService");

    spliced->readers[V_DELIVERYINFO_ID] = c_keep(kernel->deliveryService);

    c_mutexUnlock(&spliced->builtinDataMutex);

    /* setup stuff for heartbeat protocol */
    /* Heartbeat period is first set to default, since configuration is not
     * known yet. As soon as configuration is known by splicedaemon, the
     * period is set when starting the heartbeat.
     */
    spliced->hb.id = v_publicGid(v_public(spliced));

    /* For backwards compatibility, make sure this matches with old versions.
     * Note this still leaves a gap (V6.5 - V6.5.0p5), because localId was bumped
     * to 83 and was still part of the keylist. But it restores compatibility
     * with all < V6.5 versions.
     * In >= V6.5.0p5 localId was removed from the keylist so no longer relevant.
     */
    spliced->hb.id.localId = 82;

    spliced->hb.period.seconds = HB_DEFAULT_SEC;
    spliced->hb.period.nanoseconds = HB_DEFAULT_NSEC;
    spliced->hbCheck = NULL;
    spliced->hbUpdate = NULL;

    expr = q_parse("select * from " V_HEARTBEATINFO_NAME " where id.systemId != %0;");
    params[0] = c_ulongValue(v_gidSystemId(spliced->hb.id));

    spliced->readers[V_HEARTBEATINFO_ID] =
                v_dataReaderNew(spliced->builtinSubscriber,
                                V_HEARTBEATINFO_NAME "Reader",
                                expr, params,
                                NULL, TRUE);
    q_dispose(expr);
    v_observableAddObserver(v_observable(spliced->readers[V_HEARTBEATINFO_ID]), v_observer(spliced->ws), NULL);
    topic = v_builtinTopicLookup(kernel->builtin, V_HEARTBEATINFO_ID);
    str = messageKeyExpr(topic);
    spliced->missedHB = c_tableNew(v_topicMessageType(topic), str);
    os_free(str);

    /* Setup ParticipantInfo reader if builtin topics are enabled*/
    builtinEnabled = v_objectKernel(spliced)->qos->builtin.v.enabled;
    if (builtinEnabled) {
        expr = q_parse("select * from " V_PARTICIPANTINFO_NAME " where _key.systemId != %0;");
        spliced->readers[V_PARTICIPANTINFO_ID] =
                    v_dataReaderNew(spliced->builtinSubscriber,
                                    V_PARTICIPANTINFO_NAME "Reader",
                                    expr, params,
                                    rQos, TRUE);
        v_observableAddObserver(v_observable(spliced->readers[V_PARTICIPANTINFO_ID]), v_observer(spliced->ws), NULL);
        q_dispose(expr);
    }
    v_readerQosFree(rQos);
    /* Setup CandM Command reader */
    expr = q_parse("select * from " V_C_AND_M_COMMAND_NAME);
    rQos = v_readerQosNew(kernel, NULL);
    rQos->reliability.v.kind = V_RELIABILITY_RELIABLE;
    rQos->history.v.kind = V_HISTORY_KEEPALL;
    rQos->history.v.depth = V_LENGTH_UNLIMITED;
    spliced->readers[V_C_AND_M_COMMAND_ID] =
                v_dataReaderNew(spliced->builtinSubscriber,
                                C_AND_M_READER_NAME,
                                expr, params,
                                rQos, TRUE);
    v_readerQosFree(rQos);
    q_dispose(expr);

}
#undef _INIT_BUILTIN_DATA_

/**************************************************************
 * constructor/destructor
 **************************************************************/
v_spliced
v_splicedNew(
    v_kernel kernel,
    c_bool enable)
{
    v_spliced spliced;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    spliced = v_spliced(v_objectNew(kernel, K_SPLICED));
    v_splicedInit(spliced, enable);

    return spliced;
}

void
v_splicedInit(
    v_spliced spliced,
    c_bool enable)
{
    v_kernel kernel;
    v_participantQos q;
    char* hostName;
    os_result osr;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);
    q = v_participantQosNew(kernel, NULL);

#define MAX_HOST_NAME_LENGTH (64)
    hostName = (char*)(os_malloc(MAX_HOST_NAME_LENGTH+1));
    memset(hostName, 0, MAX_HOST_NAME_LENGTH+1);
    osr = os_gethostname(hostName, MAX_HOST_NAME_LENGTH);

    if(osr == os_resultSuccess){
        q->userData.v.size = (c_long)strlen(hostName);
        q->userData.v.value = c_arrayNew(c_octet_t(c_getBase(kernel)), (c_ulong)q->userData.v.size);
        memcpy(q->userData.v.value, hostName, (c_ulong)q->userData.v.size);
    } else {
        q->userData.v.size = 0;
    }
    os_free(hostName);
#undef MAX_HOST_NAME_LENGTH

    v_serviceInit(v_service(spliced), V_SPLICED_NAME, NULL, V_SERVICETYPE_SPLICED, q, enable);
    c_free(q);
    /* replace my leaseManager (created in v_serviceInit())
     * with the kernel leaseManager */
    c_free(v_participant(spliced)->leaseManager);
    v_participant(spliced)->leaseManager = c_keep(kernel->livelinessLM);

    c_mutexInit(c_getBase(spliced), &spliced->mtx);
    c_mutexInit(c_getBase(spliced), &spliced->cAndMCommandMutex);
    c_mutexInit(c_getBase(spliced), &spliced->builtinDataMutex);

    spliced->readers[V_PARTICIPANTINFO_ID] = NULL;
    spliced->readers[V_TOPICINFO_ID] = NULL;
    spliced->readers[V_TYPEINFO_ID] = NULL;
    spliced->readers[V_PUBLICATIONINFO_ID] = NULL;
    spliced->readers[V_SUBSCRIPTIONINFO_ID] = NULL;
    spliced->readers[V_C_AND_M_COMMAND_ID] = NULL;

    spliced->builtinData[V_PARTICIPANTINFO_ID] = NULL;
    spliced->builtinData[V_TOPICINFO_ID] = NULL;
    spliced->builtinData[V_TYPEINFO_ID] = NULL;
    spliced->builtinData[V_PUBLICATIONINFO_ID] = NULL;
    spliced->builtinData[V_SUBSCRIPTIONINFO_ID] = NULL;
    spliced->builtinData[V_C_AND_M_COMMAND_ID] = NULL;

    spliced->durabilityClient = NULL;
    spliced->hbManager = NULL;
    spliced->ws = NULL;
    spliced->quit = FALSE;
    spliced->cAndMCommandWaitSet = v_waitsetNew(kernel->builtin->participant);
    spliced->cAndMCommandDispatcherQuit = FALSE;

    v_durabilityClientLoadTypes(spliced);

    v_splicedManageKernel(spliced);
}

void
v_splicedFree(
    v_spliced spliced)
{
    v_kernel kernel;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);
    /* set builtin writer to NULL, to prevent
       using those writers while publishing builtin topic information
    */
    v_builtinWritersDisable(kernel->builtin);

    /* Stop heartbeats */
    v_serviceFree(v_service(spliced));
    kernel->splicedRunning = FALSE;
}

void
v_splicedDeinit(
    v_spliced spliced)
{
    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    v_waitsetFree(spliced->cAndMCommandWaitSet);

    /* prevent that the kernel lease manager is destroyed */
    c_free(v_participant(spliced)->leaseManager);
    v_participant(spliced)->leaseManager = NULL;

    v_leaseManagerFree(spliced->hbManager);

    v_serviceDeinit(v_service(spliced));
}

/**************************************************************
 * Protected functions
 **************************************************************/

/**************************************************************
 * Protected functions
 **************************************************************/
void
v_splicedHeartbeat(
    v_spliced spliced)
{
    v_message msg;
    struct v_heartbeatInfo *hb;
    v_kernel kernel;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);
    msg = v_topicMessageNew(v_builtinTopicLookup(kernel->builtin, V_HEARTBEATINFO_ID));
    if (msg) {
        hb = (struct v_heartbeatInfo *) (msg + 1);
        *hb = spliced->hb;
        v_writerWrite(v_builtinWriterLookup(kernel->builtin, V_HEARTBEATINFO_ID), msg, os_timeWGet(),NULL);
        c_free(msg);
    }
}

static void disposeHeartbeat(v_spliced spliced, v_dataReaderSample s)
{
    v_writer writer = v_builtinWriterLookup(v_objectKernel(spliced)->builtin, V_HEARTBEATINFO_ID);
    v_message smsg = v_dataReaderSampleMessage(s);
    const struct v_heartbeatInfo *hb = v_builtinHeartbeatInfoData(smsg);
    (void)v_builtinWriteHeartbeat(
        writer, hb->id.systemId, smsg->writeTime, v_durationToOsDuration(spliced->hb.period), L_DISPOSED | L_UNREGISTER | L_IMPLICIT);
}

struct readTakeActionArg {
    c_iter result;
    int isdisposed;
};

struct processHeartbeatReadActionArg {
    v_spliced spliced;
    int resched;
};

static int readTakeActionAccept(int isdisposed, v_dataReaderSample s)
{
    if (isdisposed) {
        return v_dataReaderSampleInstanceStateTest(s, L_DISPOSED);
    } else {
        return (!v_dataReaderSampleInstanceStateTest(s, L_DISPOSED));
    }
    return 0;
}

static v_actionResult readTakeAction(c_object o, c_voidp varg)
{
    v_dataReaderSample s = v_dataReaderSample(o);
    struct readTakeActionArg *arg = varg;
    v_actionResult result = 0;
    v_actionResultSet(result, V_PROCEED);
    if (s != NULL) {
        if (readTakeActionAccept(arg->isdisposed, s)) {
            arg->result = c_iterAppend(arg->result, c_keep(s));
        } else {
            v_actionResultSet(result, V_SKIP);
        }
    }
    return result;
}

static int isFiniteHeartbeat(v_dataReaderSample s)
{
    v_message smsg = v_dataReaderSampleMessage(s);
    const struct v_heartbeatInfo *hb = (const struct v_heartbeatInfo *) (smsg + 1);
    return v_dataReaderSampleStateTest(s, L_VALIDDATA) && !OS_DURATION_ISINFINITE(v_durationToOsDuration(hb->period));
}

static v_actionResult processHeartbeatReadAction(c_object o, c_voidp varg)
{
    v_dataReaderSample s = v_dataReaderSample(o);
    struct processHeartbeatReadActionArg *arg = varg;
    v_actionResult result = 0;
    if (s != NULL) {
        if (v_dataReaderSampleInstanceStateTest(s, L_STATECHANGED | L_NOWRITERS)) {
            arg->resched = 1;
        } else if (v_dataReaderSampleStateTestNot(s, L_READ | L_LAZYREAD) && isFiniteHeartbeat(s)) {
            arg->resched = 1;
        } else {
            v_actionResultSet(result, V_PROCEED);
        }
    }
    return result;
}

static int v_splicedProcessHeartbeat (v_spliced spliced)
{
    /* Trigger the heartbeat lease action so that the actual processing is single
       threaded by the thread at the correct priority. */
    struct processHeartbeatReadActionArg arg;
    arg.spliced = spliced;
    arg.resched = 0;
    v_dataReaderRead(spliced->readers[V_HEARTBEATINFO_ID], V_MASK_ANY, processHeartbeatReadAction, &arg, OS_DURATION_ZERO);
    if (arg.resched) {
        v_leaseRenew(spliced->hbCheck, 0);
    }
    return arg.resched;
}

void
v_splicedCheckHeartbeats(
    v_spliced spliced)
{
    const os_timeE tnow = os_timeEGet ();
    os_duration nextPeriod = OS_DURATION_INFINITE;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    if (spliced->readers[V_HEARTBEATINFO_ID] != NULL) {
        struct readTakeActionArg rtarg;
        v_dataReaderSample s;

        /* Read updates first, ignoring disposes. NOT_ALIVE_NO_WRITERS instances
           will be disposed (so will be taken below), writes result in checking the
           heartbeat. */
        rtarg.result = NULL;
        rtarg.isdisposed = 0;
        v_dataReaderRead(spliced->readers[V_HEARTBEATINFO_ID], V_MASK_ANY, readTakeAction, &rtarg, OS_DURATION_ZERO);
        while ((s = c_iterTakeFirst(rtarg.result)) != NULL) {
            if (v_dataReaderSampleInstanceStateTest(s, L_NOWRITERS)) {
                disposeHeartbeat(spliced, s);
            } else if (v_dataReaderSampleStateTest(s, L_VALIDDATA)) {
                v_message msg = v_dataReaderSampleMessage(s);
                const struct v_heartbeatInfo *hb = (const struct v_heartbeatInfo *) (msg + 1);
                os_duration hb_period = v_durationToOsDuration(hb->period);
                assert(v_gidEqual(hb->id, v_publicGid(v_public(spliced))) == FALSE);
                if (OS_DURATION_ISINFINITE(hb_period)) {
                    /* Unregister a fake heartbeats just in case we wrote one when
                       processing a DCPSParticipant sample */
                    v_writer writer = v_builtinWriterLookup(
                        v_objectKernel(spliced)->builtin, V_HEARTBEATINFO_ID);
                    (void)v_builtinWriteHeartbeat(
                        writer, hb->id.systemId, msg->writeTime, hb_period, L_UNREGISTER);
                } else if (os_durationCompare(os_timeEDiff(tnow, msg->allocTime), hb_period) == OS_MORE) {
                    /* Expired heartbeat, dispose to let the next part of the
                       processing hand it off to the garbage collector */
                    disposeHeartbeat(spliced, s);
                } else if (os_durationCompare(hb_period, nextPeriod) == OS_LESS) {
                    /* This node's heartbeat is set to expire first. */
                    nextPeriod = hb_period;
                }
            }
            c_free(s);
        }
        c_iterFree(rtarg.result);

        /* For all heartbeats that we missed or were disposed, get the garbage collector
           to eventually clean up all associated data (dispose built-in topics for remote
           entities, (auto dispose and) unregister samples for remote writers, &c.) */
        rtarg.result = NULL;
        rtarg.isdisposed = 1;
        (void)v_dataReaderTake(spliced->readers[V_HEARTBEATINFO_ID], V_MASK_ANY, readTakeAction, &rtarg, OS_DURATION_ZERO);
        c_mutexLock(&spliced->mtx);
        while ((s = c_iterTakeFirst(rtarg.result)) != NULL) {
            if(v_dataReaderSampleStateTest(s, L_VALIDDATA)) {
                ospl_c_insert(spliced->missedHB, c_keep(v_dataReaderSampleMessage(s)));
            }
            c_free(s);
        }
        c_mutexUnlock(&spliced->mtx);
        c_iterFree(rtarg.result);
    }

    v_leaseRenew(spliced->hbCheck, nextPeriod);
}

/**************************************************************
 * Public functions
 **************************************************************/
void
v_splicedKernelManager(
    v_spliced spliced)
{
    int dataProcessed;
    c_bool builtinEnabled;

    builtinEnabled = v_objectKernel(spliced)->qos->builtin.v.enabled;

    while (!spliced->quit) {
        dataProcessed = v_splicedProcessHeartbeat(spliced);
        dataProcessed += v_splicedProcessTopicInfo(spliced);
        dataProcessed += v_splicedProcessTypeInfo(spliced);
        dataProcessed += v_splicedProcessPublicationInfo(spliced);
        if (builtinEnabled) {
            dataProcessed += v_splicedProcessSubscriptionInfo(spliced);
            dataProcessed += v_splicedProcessParticipantInfo(spliced);
        }
        if (dataProcessed == 0) {
            (void)v_waitsetWait(spliced->ws, NULL, NULL, OS_DURATION_INFINITE);
        }
    }
}

void
v_splicedBuiltinResendManager(
    v_spliced spliced)
{
    v_kernel k;

    k = v_objectKernel(spliced);
    v_participantResendManagerMain(k->builtin->participant);
}

void
v_splicedCAndMCommandDispatcherQuit(
    v_spliced spliced)
{
   assert(C_TYPECHECK(spliced,v_spliced));
   c_mutexLock(&spliced->cAndMCommandMutex);
   spliced->cAndMCommandDispatcherQuit = TRUE;
   /* Wakeup the C&M command dispatcher thread to take notice and terminate.
    * Waitset mask must be set sensitive to V_EVENT_TRIGGER. */
   v_waitsetTrigger(spliced->cAndMCommandWaitSet, NULL);
   c_mutexUnlock(&spliced->cAndMCommandMutex);
}

static void v_splicedTakeCandMCommand(v_spliced spliced);

static void
dispatchCandMCommandAction(
    v_waitsetEvent e,
    c_voidp arg)
{
    v_spliced spliced = (v_spliced)arg;
    v_splicedTakeCandMCommand(spliced);
    OS_UNUSED_ARG(e);
}

void
v_splicedBuiltinCAndMCommandDispatcher(
    v_spliced spliced)
{
    v_result res;
    c_bool boolRes;
    v_dataReader reader;

    c_mutexLock(&spliced->cAndMCommandMutex);
    reader = spliced->readers[V_C_AND_M_COMMAND_ID];
    boolRes = v_waitsetAttach(spliced->cAndMCommandWaitSet, (v_observable)reader, NULL);

    /* The V_EVENT_TRIGGER interest is used stop the C&M command dispatcher thread. */
    v_observerSetEventMask((v_observer)spliced->cAndMCommandWaitSet, V_EVENT_DATA_AVAILABLE | V_EVENT_TRIGGER);
    assert( boolRes );
    OS_UNUSED_ARG(boolRes);

    while (!spliced->cAndMCommandDispatcherQuit)
    {
        c_mutexUnlock(&spliced->cAndMCommandMutex);
        res = v_waitsetWait(spliced->cAndMCommandWaitSet, dispatchCandMCommandAction, spliced, OS_DURATION_INFINITE);
        assert(res == V_RESULT_OK || res == V_RESULT_DETACHING);
        OS_UNUSED_ARG(res);
        c_mutexLock(&spliced->cAndMCommandMutex);
    }
    c_mutexUnlock(&spliced->cAndMCommandMutex);

    (void) v_waitsetDetach (spliced->cAndMCommandWaitSet, v_observable(reader));
}

void
v_splicedPrepareTermination(
    v_spliced spliced)
{
    C_STRUCT(v_event) term;
    v_observer o;
    v_kernel k;

    k = v_objectKernel(spliced);
    v_participantResendManagerQuit(k->builtin->participant);

    term.kind = V_EVENT_TRIGGER;
    term.source = v_observable(spliced);
    term.data = NULL;
    c_mutexLock(&spliced->mtx);
    spliced->quit = TRUE;
    c_mutexUnlock(&spliced->mtx);

    o = v_observer(spliced->ws);
    v_observerLock(o);
    v_observerNotify(o, &term, NULL);
    v_observerUnlock(o);
}

static void
doCleanupPublication(
    v_spliced spliced,
    struct v_publicationInfo *oInfo,
    os_timeW cleanTime,
    c_bool isValid,
    c_bool isLocal,
    c_bool isImplicit)
{
    v_kernel kernel;
    c_ulong i, len;
    v_group group;
    c_iter groups;
    c_bool cleanup;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));
    assert(oInfo != NULL);

    kernel = v_objectKernel(spliced);

    if (isValid) {
        /* Publication-info is from a valid sample: now for every partition
         * in the partition info find the group and notify the group that all
         * instances of the writer must be unregistered
         */
        len = c_arraySize(oInfo->partition.name);
        cleanup = TRUE;

        /* Do not clean up for built-in data-writers of DCPSTopic and
         * DCPSType. Built-in writers only write into a single partition: the
         * built-in partition.
         */
        if((len == 1) && (strcmp(oInfo->partition.name[0], V_BUILTIN_PARTITION) == 0)) {
            /* Partition matches, so let's check topic. */
            if((strcmp(oInfo->topic_name, V_TOPICINFO_NAME) == 0) ||
               (strcmp(oInfo->topic_name, V_TYPEINFO_NAME) == 0)){
                /* Topic matches, so don't clean up. */
                cleanup = FALSE;
            }
        }

        if(cleanup == TRUE){
            for (i = 0; i < len; i++)
            {
                groups = v_groupSetLookup(kernel->groupSet, oInfo->partition.name[i], oInfo->topic_name);
                while ((group = c_iterTakeFirst(groups)) != NULL) {
                    v_groupDisconnectWriter(group, oInfo, cleanTime, isLocal, isImplicit);
                    c_free(group);
                }
                c_iterFree(groups);
            }
        }
    } else {
        /* Publication-info is from invalid sample so only key-fields may be interpreted.
         * Select all groups to check if any instances of the writer must be unregistered
         */
        groups = v_groupSetSelectAll(kernel->groupSet);
        while ((group = c_iterTakeFirst(groups)) != NULL) {
            cleanup = TRUE;

            /* Do not clean up for built-in data-writers of DCPSTopic and
             * DCPSType. Built-in writers only write into a single partition: the
             * built-in partition.
             */
            if(strcmp(V_BUILTIN_PARTITION, v_partitionName(v_groupPartition(group))) == 0) {
                /* Partition matches, so let's check topic. */
                if((strcmp(V_TOPICINFO_NAME, v_topicName(v_groupTopic(group))) == 0) ||
                   (strcmp(V_TYPEINFO_NAME,  v_topicName(v_groupTopic(group))) == 0)){
                    /* Topic matches, so don't clean up. */
                    cleanup = FALSE;
                }
            }
            if(cleanup == TRUE){
                v_groupDisconnectWriter(group, oInfo, cleanTime, isLocal, isImplicit);
            }
            c_free(group);
        }
        c_iterFree(groups);
    }
}

#define GC_DELAY_SEC  (0)
#define GC_DELAY_NSEC (20000000) /* 20ms */

static void
v_splicedGarbageCollectorMissedHB (
    v_spliced spliced,
    c_ulong systemId,
    os_timeW cleanTime)
{
    const os_duration delay = OS_DURATION_INIT(GC_DELAY_SEC, GC_DELAY_NSEC);
    v_kernel kernel = v_objectKernel (spliced);
    v_result vresult;
    v_group g;
    c_bool pubComplete;

    /* Wait for alignment of DCPSPublication topic */
    vresult = v_readerWaitForHistoricalData(v_reader(spliced->readers[V_PUBLICATIONINFO_ID]), OS_DURATION_ZERO);
    pubComplete = (vresult == V_RESULT_OK);

    OS_REPORT(OS_INFO, "v_spliced", vresult,
        "Missed heartbeat for node %d. (DCPSPublication is %s)",
        systemId, pubComplete ? "complete" : "not complete");

    if (kernel->builtin->kernelQos->builtin.v.enabled && pubComplete) {
        /* In case the builtin topics are enabled, we only need to remove the
         * DCPSPublication samples that originate from the disconnected node.
         * The v_splicedProcessPublicationInfo() function will then take it
         * from there.
         */
        g = v_groupSetGet(kernel->groupSet, V_BUILTIN_PARTITION, V_PUBLICATIONINFO_NAME);
        /* A group does not necessarily have to exist locally! */
        if (g != NULL) {
            /* Disconnect all the node's registrations from this group. Note
             * that the heartbeat's period is an effectively cleanup time.
             */
            v_groupDisconnectNode(g, systemId, cleanTime);
            v__kernelProtectWaitEnter(NULL, NULL);
            os_sleep(delay);
            v__kernelProtectWaitExit();
            c_free(g);
            g = NULL;
        }
    } else {
        /* In case the builtin topics are disabled, we need to walk over all
         * available groups and remove everything that originates from the
         * disconnected node. This is also the case if DCPSPublication
         * is not complete.
         */
        c_iter groups = v_groupSetSelectAll(kernel->groupSet);
        g = v_group(c_iterTakeFirst(groups));
        while ((g != NULL) && (!spliced->quit))
        {
            v_groupDisconnectNode(g, systemId, cleanTime);
            v__kernelProtectWaitEnter(NULL, NULL);
            os_sleep(delay);
            v__kernelProtectWaitExit();
            c_free(g);
            g = v_group(c_iterTakeFirst(groups));
        }
        c_iterFree (groups);
    }
}

void
v_splicedGarbageCollector(
v_spliced spliced)
{
    c_iter groups;
    v_message missedHBMsg;
    v_kernel kernel;
    c_ulong length;
    v_group g;
    os_duration delay = OS_DURATION_INIT(GC_DELAY_SEC, GC_DELAY_NSEC);

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    /* wait until kernelmanager has initialized */
    while (!spliced->missedHB) {
        os_sleep(delay);
    }
    /*Continue for as long as the spliced doesn't need to terminate*/
    while (!spliced->quit) {
        /*Check if a heartbeat has been missed*/
        c_mutexLock(&spliced->mtx);
        missedHBMsg = c_take(spliced->missedHB);
        c_mutexUnlock(&spliced->mtx);

        /* If a heartbeat has been missed, walk over all groups and clean up
         * data from writers on the removed node.
         */
        if (missedHBMsg) {
            /* Extract systemId and a best guess to the expiry time from the heartbeat */
            const struct v_heartbeatInfo *hb = (const struct v_heartbeatInfo *) (missedHBMsg + 1);
            c_ulong systemId = hb->id.systemId;
            os_timeW cleanTime;
            if (OS_DURATION_ISINFINITE(v_durationToOsDuration(hb->period))) {
                /* Can't trust the time stamp in msg->writeTime: it is that of the
                   original write, not of the unregister */
                cleanTime = os_timeWGet ();
            } else {
                cleanTime = os_timeWAdd (missedHBMsg->writeTime, v_durationToOsDuration(hb->period));
            }
            c_free (missedHBMsg);
            v_splicedGarbageCollectorMissedHB (spliced, systemId, cleanTime);
        } else {
            /*If no heartbeat is missed, do some garbage collection.*/
            groups = v_groupSetSelectAll(kernel->groupSet);
            g = v_group(c_iterTakeFirst(groups));
            while ((g != NULL) && (!spliced->quit)) {
                v_groupUpdatePurgeList(g);
                c_free(g);

                /* Missed heartbeat handling takes precedence over the updating
                 * of purgeLists. If a heartbeat has been missed, stop
                 * updating purgeLists.
                 */
                c_mutexLock(&spliced->mtx);
                length = c_tableCount(spliced->missedHB);
                c_mutexUnlock(&spliced->mtx);

                /* A heartbeat has been missed, stop updating purgeLists now
                 * but don't forget to free the iter of groups.
                 */
                if(length != 0) {
                    g = NULL;
                } else {
                    v__kernelProtectWaitEnter(NULL, NULL);
                    os_sleep(delay);
                    v__kernelProtectWaitExit();
                    g = v_group(c_iterTakeFirst(groups));
                }
            }
            /* Make sure each group is freed if the while loops above are
             * interrupted.
             */
            if(g != NULL) {
                c_free(g);
            }
            g = v_group(c_iterTakeFirst(groups));

            while(g != NULL) {
                c_free(g);
                g = v_group(c_iterTakeFirst(groups));
            }
            c_iterFree(groups);
        }
    }
}

v_result
v_splicedDurabilityClientSetup(
    v_spliced spliced,
    c_iter durablePolicies,
    const char* partitionRequest,
    const char* partitionDataGlobal,
    const char* partitionDataPrivate)
{
    v_result result = V_RESULT_OK;

    assert(C_TYPECHECK(spliced,v_spliced));

    if (!spliced->durabilityClient) {
        spliced->durabilityClient = v_durabilityClientNew(spliced,
                                                          durablePolicies,
                                                          partitionRequest,
                                                          partitionDataGlobal,
                                                          partitionDataPrivate);
        if (!spliced->durabilityClient) {
            result = V_RESULT_INTERNAL_ERROR;
        }
    }

    return result;
}

v_durabilityClient
v_splicedDurabilityClientGet(
    v_spliced spliced)
{
    v_durabilityClient dc = NULL;
    assert(C_TYPECHECK(spliced,v_spliced));
    if (spliced->durabilityClient) {
        dc = c_keep(spliced->durabilityClient);
    }
    return dc;
}

void
v_splicedDurabilityClientMain(v_public p, void *arg)
{
    v_spliced spliced = v_spliced(p);
    OS_UNUSED_ARG(arg);
    assert(spliced->durabilityClient);
    v_durabilityClientMain(spliced->durabilityClient);
}

void
v_splicedDurabilityClientTerminate(v_spliced spliced)
{
    assert(spliced->durabilityClient);
    v_durabilityClientTerminate(spliced->durabilityClient);
}

v_leaseManager
v_splicedGetHeartbeatManager(
    v_spliced spliced,
    c_bool create)
{
    v_leaseManager lm;

    assert(C_TYPECHECK(spliced,v_spliced));

    lm = c_keep(spliced->hbManager);
    if (!lm && create) {
        spliced->hbManager = v_leaseManagerNew(v_objectKernel(spliced));
        lm = c_keep(spliced->hbManager);
    }

    return lm;
}

c_bool
v_splicedStartHeartbeat(
    v_spliced spliced,
    os_duration period,
    os_duration renewal)
{
    v_kernel kernel;
    c_bool started;
    v_result result;
    v_leaseManager lm;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    lm = spliced->hbManager;
    if (!lm)
    {
       lm = kernel->livelinessLM;
    }

    spliced->hb.period = v_durationFromOsDuration(period);
    /* The check for heartbeats should be performed at least once every
     * MIN(RemoteSpliced_0_ExpiryTime, ..., RemoteSpliced_n_ExpiryTime). Neither
     * period nor renewal are correct here. Because renewal is shorter, chances
     * that it satisfies the above are greater, but still not correct. See also
     * dds2898 */
    spliced->hbCheck = v_leaseMonotonicNew(kernel, renewal);
    if(spliced->hbCheck)
    {
        result = v_leaseManagerRegister(
            lm,
            spliced->hbCheck,
            V_LEASEACTION_HEARTBEAT_CHECK,
            v_public(spliced),
            TRUE /* repeat lease if expired */);
        if(result != V_RESULT_OK)
        {
            c_free(spliced->hbCheck);
            spliced->hbCheck = NULL;
            OS_REPORT(OS_ERROR, "v_spliced", V_RESULT_INTERNAL_ERROR,
                "A fatal error was detected when trying to register the spliced liveliness hbCheck lease "
                "to the liveliness lease manager of the kernel. The result code was %d.", result);
        }
    }

    spliced->hbUpdate = v_leaseMonotonicNew(kernel, renewal);
    if(spliced->hbUpdate)
    {
        result = v_leaseManagerRegister(
            lm,
            spliced->hbUpdate,
            V_LEASEACTION_HEARTBEAT_SEND,
            v_public(spliced),
            TRUE /* repeat lease if expired */);
        if(result != V_RESULT_OK)
        {
            c_free(spliced->hbUpdate);
            spliced->hbUpdate = NULL;
            OS_REPORT(OS_ERROR, "v_spliced", V_RESULT_INTERNAL_ERROR,
                "A fatal error was detected when trying to register the spliced liveliness hbUpdate lease "
                "to the liveliness lease manager of the kernel. The result code was %d.", result);
        }
    }
    if ((spliced->hbCheck != NULL) && (spliced->hbUpdate != NULL)) {
        started = TRUE;
    } else {
        started = FALSE;
    }

    return started;
}

c_bool
v_splicedStopHeartbeat(
    v_spliced spliced)
{
    v_kernel kernel;
    v_leaseManager lm;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    lm = spliced->hbManager;
    if (!lm)
    {
       lm = kernel->livelinessLM;
    }

    v_leaseManagerDeregister(lm, spliced->hbCheck);
    c_free(spliced->hbCheck);
    spliced->hbCheck = NULL;
    v_leaseManagerDeregister(lm, spliced->hbUpdate);
    c_free(spliced->hbUpdate);
    spliced->hbUpdate = NULL;

    return TRUE;
}


static void
disposeAllDataCandMCommand(
    v_spliced spliced,
    v_controlAndMonitoringCommand *command,
    v_gid writerGid,
    os_timeW timestamp)
{
   v_kernel kernel;
   struct v_commandDisposeAllData *disposeCmd;

   assert(spliced != NULL);
   assert(C_TYPECHECK(spliced,v_spliced));

   disposeCmd = &command->u._u.dispose_all_data_info;

   kernel = v_objectKernel(spliced);

   if (kernel->GID.systemId != writerGid.systemId)
   {
       if (v_kernelDisposeAllData(kernel, disposeCmd->partitionExpr, disposeCmd->topicExpr, timestamp) != V_RESULT_OK)
       {
           OS_REPORT(OS_WARNING, "spliced", V_RESULT_INTERNAL_ERROR,
                   "Dispose All Data failed due to internal error.");
       }
   }
}

typedef void (*cAndMCommandFn_t)(v_spliced spliced,
                                 v_controlAndMonitoringCommand *command,
                                 v_gid writerGid,
                                 os_timeW timestamp);

/* This array is indexed by values of the
   enum v_controlAndMonitoringCommandKind */
static cAndMCommandFn_t cAndMCommandFns[] =
{
   disposeAllDataCandMCommand
};

static void dispatchCandMCommand(v_spliced spliced,
                                 v_dataReaderSample s,
                                 os_timeW timestamp)
{
   v_message msg;
   v_gid writerGid;
   v_controlAndMonitoringCommand *command;

   assert(spliced != NULL);
   assert(C_TYPECHECK(spliced,v_spliced));
   assert(s != NULL);
   assert(C_TYPECHECK(s,v_dataReaderSample));

   msg = v_dataReaderSampleMessage(s);
   writerGid = msg->writerGID;
   command = (v_controlAndMonitoringCommand *) (msg + 1);
   if ( (unsigned) command->u._d < sizeof(cAndMCommandFns)/sizeof(cAndMCommandFn_t) )
   {
      cAndMCommandFns[command->u._d]( spliced, command, writerGid, timestamp );
   }
   else
   {
      OS_REPORT(OS_WARNING, "spliced", V_RESULT_ILL_PARAM,
                "unknown Control and Monitoring Command received.");
   }
}

static void v_splicedTakeCandMCommand(v_spliced spliced)
{
    v_dataReaderSample s;
    v_dataReader reader;
    c_iter samples;
    os_timeW timestamp;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    samples = NULL;
    if ( (reader = spliced->readers[V_C_AND_M_COMMAND_ID]) != NULL )
    {
        (void)v_dataReaderTake(reader, V_MASK_ANY, readerTakeAction, &samples, OS_DURATION_ZERO);
        while ( (s = v_dataReaderSample(c_iterTakeFirst(samples))) != NULL )
        {
           timestamp = v_dataReaderSampleMessage(s)->writeTime;
           dispatchCandMCommand(spliced, s, timestamp);
           c_free(s);
        }
        c_iterFree(samples);
    }
}

struct lookupPublicationArg {
    v_gid wgid;
    struct v_publicationInfo *info;
};

static c_bool
lookupPublication(
    c_object o,
    c_voidp arg)
{
    v_message msg = v_message(o);
    struct lookupPublicationArg *a = (struct lookupPublicationArg *)arg;
    struct v_publicationInfo *info;

    info = v_builtinPublicationInfoData(msg);
    if (v_gidEqual(info->key, a->wgid)) {
        a->info = info;
        return FALSE;
    }
    return TRUE;
}

v_result
v_splicedLookupPublicationInfo(
    v_spliced _this,
    v_gid wgid,
    publicationInfoAction action,
    void *arg)
{
    v_result result = V_RESULT_UNDEFINED;
    c_bool found = FALSE;
    struct lookupPublicationArg lpa;

    lpa.wgid = wgid;
    lpa.info = NULL;
    c_mutexLock(&_this->builtinDataMutex);
    found = !c_walk(_this->builtinData[V_PUBLICATIONINFO_ID], lookupPublication, &lpa);
    if (found && action) {
        result = action(lpa.info, arg);
    }
    c_mutexUnlock(&_this->builtinDataMutex);
    return result;
}

#if 1 /* Following code is deprecated and must be removed as soon as group transactions no longer depends on this*/

struct countMatchesArg {
    v_spliced spliced;
    struct v_publicationInfo  *pInfo;
    c_ulong count;
};

static c_bool
countMatches(
    v_reader o,
    c_voidp arg)
{
    struct countMatchesArg *a = (struct countMatchesArg *)arg;
    v_subscriptionInfoTemplate msg;
    conformState state;

    /* Following is a bit too expensive.
     * It would be better not to create a msg and to cache the resulting lookup
     * in the v_transactionPublisher for future lookups.
     */
    msg = (v_subscriptionInfoTemplate)
                v_builtinCreateSubscriptionInfo(v_objectKernel(o)->builtin, o);
    if (msg) {
        if (strcmp(msg->userData.topic_name, a->pInfo->topic_name) == 0) {
            state = determineState(a->pInfo, &msg->userData);
            if (state == CONFORM_STATE_OK) {
                a->count++;
            }
        }
        c_free(msg);
    }
    return TRUE;
}

c_bool
v_splicedPublicationMatchCount(
    v_spliced _this,
    v_object scope,  /* matching scope: v_kernel or v_subscriber */
    v_gid wgid,
    c_ulong *count)
{
    c_bool found = FALSE;
    struct lookupPublicationArg arg;
    struct countMatchesArg cma;

    arg.wgid = wgid;
    arg.info = NULL;
    c_mutexLock(&_this->builtinDataMutex);
    found = !c_walk(_this->builtinData[V_PUBLICATIONINFO_ID], lookupPublication, &arg);
    c_mutexUnlock(&_this->builtinDataMutex);
    if (found) {
        /* Now check if the found publication matches any of the readers in the matching scope. */
        if (v_objectKind(scope) == K_SUBSCRIBER) {
            cma.spliced = _this;
            cma.pInfo = arg.info;
            cma.count = 0;
            /* Only required for subscribers, kernel scope always matches. */
            v_subscriberWalkReaders(v_subscriber(scope), countMatches, &cma);
            *count = cma.count;
        } else {
            /* The scope is the kernel which implies the durability service.
             * Completeness at the durability service depends on namespace interest but
             * this dependency is not yet implemented.
             * For now the assumption is made that interest always exists which implies
             * that the match count is 1 (fictive durability reader).
             */
            assert(v_objectKind(scope) == K_KERNEL);
            *count = 1;
        }
    }
    return found;
}
#endif

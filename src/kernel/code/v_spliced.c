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

#include "os.h"
#include "os_report.h"
#include "os_stdlib.h"
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
#include "v_subscriber.h"
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
#include "v_state.h"
#include "v_time.h"
#include "v__group.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_statistics.h"
#include "v__waitset.h"
#include "v__messageQos.h"
#include "v__deliveryService.h"
#include "v_writerQos.h"

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
    c_time *cleanTime,
    c_bool isValid,
    c_bool isLocal);

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

/* The read action also reads the invalid samples, unlike the readerTakeAction
 */
static v_actionResult
readerReadAction(
    c_object s,
    c_voidp arg)
{
    v_actionResult result = 0;
    c_iter *samples = (c_iter *)arg;

    if (s != NULL) {
        /* use append, since the oldest message is given first to this copy routine! */
        *samples = c_iterAppend(*samples, c_keep(s));
    } /* else last sample is read */

    v_actionResultSet(result, V_PROCEED);
    return result;
}

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
    c_long i;

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
    c_free(q);

    return requestedMessages;
}
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
    getMatchingWriterArg a = (getMatchingWriterArg)arg;
    struct v_publicationInfo *info = v_builtinPublicationInfoData(a->kernel->builtin,o);

    if (o != NULL) {
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

static v_topicQos
createTopicQos(
    v_kernel kernel,
    struct v_topicInfo *info)
{
    v_topicQos qos;

    qos = v_topicQosNew(kernel, NULL);
    qos->durability         = info->durability;
    qos->durabilityService  = info->durabilityService;
    qos->deadline           = info->deadline;
    qos->latency            = info->latency_budget;
    qos->liveliness         = info->liveliness;
    qos->reliability        = info->reliability;
    qos->transport          = info->transport_priority;
    qos->lifespan           = info->lifespan;
    qos->orderby            = info->destination_order;
    qos->history            = info->history;
    qos->resource           = info->resource_limits;
    qos->ownership          = info->ownership;

    return qos; /* transfer refcount to caller */
}

static c_bool
defineTopic(
    v_kernel kernel,
    struct v_topicInfo *info)
{
    v_topicQos qos;
    v_topic newTopic;
    sd_serializer serializer;
    sd_serializedData meta_data;
    c_type topicType = NULL;
    c_char *msg;
    c_char *loc;
    c_bool result = FALSE;

    assert(C_TYPECHECK(kernel,v_kernel));

    serializer = sd_serializerXMLTypeinfoNew(c_getBase(c_object(kernel)),
                     FALSE /* do not escape " characters */);
    if (serializer != NULL) {
        meta_data = sd_serializerFromString(serializer, info->meta_data);
        if (meta_data != NULL) {
            topicType = c_type(sd_serializerDeserializeValidated(serializer,
                                                                 meta_data));
            if (topicType == NULL) {
                if (sd_serializerLastValidationResult(serializer) == SD_VAL_ERROR) {
                    msg = sd_serializerLastValidationMessage(serializer);
                    loc = sd_serializerLastValidationLocation(serializer);
                    if (loc == NULL) {
                        OS_REPORT_1(OS_ERROR, "v_spliced", 0,
                                    "Deserialization remote topic failed: "
                                    "%s at <unknown>", msg);
                    } else {
                        OS_REPORT_2(OS_ERROR, "v_spliced", 0,
                                    "Deserialization remote topic failed: "
                                    "%s at %s", msg, loc);
                    }
		} else {
                    OS_REPORT(OS_ERROR, "v_spliced", 0,
                              "Deserialization of remote topic failed: "
                              "no addition info available");
                }
            }
            sd_serializedDataFree(meta_data);
        } else {
            OS_REPORT(OS_ERROR, "v_spliced", 0,
                      "Failed to create serializedData object");
        }
        sd_serializerFree(serializer);
    } else {
        OS_REPORT(OS_ERROR, "v_spliced", 0,
                  "Failed to create serializerXMLMetadata");
    }

    if (topicType != NULL) {
        qos = createTopicQos(kernel, info);
        newTopic = v__topicNew(kernel, info->name, info->type_name, info->key_list, qos, FALSE);
        if (newTopic != NULL) {
            result = TRUE;
        }
        v_topicQosFree(qos);
        c_free(newTopic);
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
    if (topic->crcOfName == info->key.systemId) {
        if (topic->crcOfTypeName != info->key.localId) {
            /* inconsistent, so no further checking needed .... */
            consistent = FALSE;
            type_name = c_metaScopedName(c_metaObject(v_topicDataType(topic)));
            OS_REPORT_5(OS_INFO, "v_spliced", 0,
                        "Inconsistent topic %s on types: %s(%d), "
                        "but %s(%d) defined",
                        info->name,
                        info->type_name, info->key.localId,
                        type_name, topic->crcOfTypeName);
            os_free(type_name);
        } else {
            /* check topicQos */
            qos1 = createTopicQos(kernel, info);
            qos2 = v_topicGetQos(topic);
            if (qos1 && qos2) {
                c_bool valid;
                consistent = TRUE;
                valid = v_durabilityPolicyEqual(qos1->durability, qos2->durability);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Durability' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_durabilityServicePolicyEqual(qos1->durabilityService, qos2->durabilityService);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'DurabilityService' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_deadlinePolicyEqual(qos1->deadline, qos2->deadline);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Deadline' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_latencyPolicyEqual(qos1->latency, qos2->latency);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Latency' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_livelinessPolicyEqual(qos1->liveliness, qos2->liveliness);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Liveliness' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_reliabilityPolicyEqual(qos1->reliability, qos2->reliability);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Reliability' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_orderbyPolicyEqual(qos1->orderby, qos2->orderby);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'OrderBy' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_historyPolicyEqual(qos1->history, qos2->history);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'History' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_resourcePolicyEqual(qos1->resource, qos2->resource);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Resource' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_transportPolicyEqual(qos1->transport, qos2->transport);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Transport' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_lifespanPolicyEqual(qos1->lifespan, qos2->lifespan);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                                "Detected Unmatching QoS Policy: 'Lifespan' for Topic <%s>.",
                                info->name);
                    consistent = FALSE;
                }
                valid = v_ownershipPolicyEqual(qos1->ownership, qos2->ownership);
                if (!valid) {
                    OS_REPORT_1(OS_WARNING, "v_spliced", 0,
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
        OS_REPORT_4(OS_INFO, "v_spliced", 0,
                    "Inconsistent topic on name: local %s(%d) remote %s(%d)",
                    v_topicName(topic), topic->crcOfName,
                    info->name, info->key.systemId);
    }
    if (consistent == FALSE) {
        v_observerLock(v_observer(topic));
        v_topicNotifyInconsistentTopic(topic);
        v_observerUnlock(v_observer(topic));
    }
    return consistent;
}

struct checkHeartbeatArg {
    v_spliced  spliced;
    c_iter     missed; /* list of instance handles of heartbeats that
                        * are missed */
    c_time     ct;     /* current time */
    v_duration nextPeriod;
};

static c_bool
checkHeartbeat(
    v_dataReaderSample sample,
    c_voidp argument)
{
    struct checkHeartbeatArg *arg = (struct checkHeartbeatArg *)argument;
    c_time diff;
    struct v_heartbeatInfo *hb;
    v_message msg;
    v_kernel kernel;
    c_time cleanTime;

    if (sample != NULL) {
        kernel = v_objectKernel(arg->spliced);
        msg = v_dataReaderSampleMessage(sample);
        hb = v_builtinHeartbeatInfoData(kernel->builtin,msg);

        /* We will never receive our own heartbeat due to the view filter. So no
         *  checking needed.
         */
        assert(v_gidEqual(hb->id, v_publicGid(v_public(arg->spliced))) == FALSE);
        diff = c_timeSub(arg->ct, v_dataReaderSample(sample)->insertTime);
        if (c_timeCompare(diff, hb->period) == C_GT) {
            /* ct - inserttime > hb->period <--------------
             * We missed a heartbeat, so add it to the list;
             * DO NOT USE the local current time to unregister all instances
             * from the remote node.
             * But estimate the time from the other node, by using the
             * production time of the heartbeat added with the heartbeat period.
             * This is needed since the time does not need to be aligned!
             */
            cleanTime = c_timeAdd(msg->writeTime, hb->period);
            /* store cleantime in period field so gc can determine timestamp of unregister and/or dispose message. */
            hb->period = cleanTime;
            arg->missed = c_iterInsert(arg->missed, c_keep(sample));
        }
        cleanTime = c_timeAdd(v_dataReaderSample(sample)->insertTime, hb->period);
        if (c_timeCompare(cleanTime, arg->nextPeriod) == C_LT) {
            arg->nextPeriod = cleanTime;
        }

    } /* else this was last sample! */

    return TRUE; /* all heartbeats must be checked */
}

static c_bool
dataReaderEntryAction (
    v_entry _this,
    c_voidp arg)
{
    v_publicationInfoTemplate msg = (v_publicationInfoTemplate)arg;
    c_bool result = TRUE;

    if (v_objectKind(_this) == K_DATAREADERENTRY)
    {
        v_dataReaderEntryAbortTransaction(v_dataReaderEntry(_this),
                                          msg->userData.key);
    }
    return result;
}

static void
notifyCoherentReaders (
    v_kernel _this,
    v_dataReaderSample rSample)
{
    v_group group;
    v_topic topic;
    c_long nrOfPartitions, i;
    c_iter groupList;
    c_value params[1];
    v_publicationInfoTemplate rInfo;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_kernel));

    if (_this == NULL) {
        OS_REPORT(OS_WARNING,
                  "v_spliced::notifyCoherentReaders", 0,
                  "Received illegal '_this' reference to kernel.");
        return;
    }
    if (rSample == NULL) {
        OS_REPORT(OS_WARNING,
                  "v_spliced::notifyCoherentReaders", 0,
                  "Received illegal sample: <NULL>.");
        return;
    }
    rInfo = (v_publicationInfoTemplate)v_dataReaderSampleMessage(rSample);
    if (v_messageStateTest(rInfo,L_DISPOSED)) {
        if (rInfo->userData.presentation.coherent_access) {
            /* At this point the message received anounces the
             * disapearance of a coherent DataWriter.
             * For each coherent DataWriter that disapeares we need to look
             * for messages originating from this DataWriter that are still
             * being part of an open tranactions.
             * If a message is found the whole transaction must be discard.
             */
            /* lookup all DataReader-Topic related groups. */
            topic = v_lookupTopic(_this,rInfo->userData.topic_name);
            params[0] = c_objectValue(topic);
            groupList = v_groupSetSelect(_this->groupSet, "topic = %0", params);

            /* filter out DataReader Partition incompliant groups. */
            nrOfPartitions = c_arraySize(rInfo->userData.partition.name);
            group = c_iterTakeFirst(groupList);
            while (group) {
                for (i=0; i<nrOfPartitions; i++) {
                    c_string name = v_entityName(group->partition);
                    if (strcmp(name,rInfo->userData.partition.name[i]) == 0) {
                        v_groupWalkEntries(group,dataReaderEntryAction,rInfo);
                        i = nrOfPartitions; /* exit for loop */
                    }
                }
                c_free(group);
                group = c_iterTakeFirst(groupList);
            }
            c_iterFree(groupList);
            c_free(topic);
        }
    }
}

static c_bool
matchPartition(
    c_array name1,
    c_array name2)
{
    c_long i, j;
    c_value val;
    c_value str1, str2;

    val.kind = V_BOOLEAN;
    val.is.Boolean = FALSE;

    for (i = 0; val.is.Boolean == FALSE && i < c_arraySize(name1); i++) {
        str1 = c_stringValue(name1[i]);
        for (j = 0; val.is.Boolean == FALSE && j < c_arraySize(name2); j++) {
            str2 = c_stringValue(name2[j]);
            val = c_valueStringMatch(str1, str2);
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
        (void)v_dataReaderTake(spliced->readers[V_SUBSCRIPTIONINFO_ID],
                               takeOne,
                               &rSample);
    }
    /* If a subscriptionInfo message exists then process it.
     */
    if (rSample != NULL) {
        result = 1;

        c_mutexLock(&spliced->builtinDataMutex);

        notifyCoherentReaders(kernel, rSample);
        msg = v_dataReaderSampleMessage(rSample);
        rInfo = v_builtinSubscriptionInfoData(kernel->builtin,msg);

        if (v_stateTest(v_readerSample(rSample)->sampleState, L_DISPOSED)) {
            /* The subscription is disposed so the following
             * code will remove and free the registration.
             */
            oldMsg = c_remove(spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                              msg, NULL, NULL);

            if (oldMsg == NULL) {
                /* The instance is not yet know no need to
                 * take action.
                 */
            } else {
                /* Notify the nodal delivery service about any synchronous
                 * DataReader topology changes.
                 */
                 oldInfo = v_builtinSubscriptionInfoData(kernel->builtin, oldMsg);
                 if (oldInfo->reliability.synchronous) {
                     v_deliveryServiceUnregister(kernel->deliveryService, oldMsg);
                 }

                 /* Notify matching writers that a matching subscription disappeared */
                 offeredMessages = lookupMatchingWritersByTopic(spliced, oldInfo);
                 offeredMsg = c_iterTakeFirst(offeredMessages);
                 while (offeredMsg != NULL) {
                     oInfo = v_builtinPublicationInfoData(kernel->builtin,offeredMsg);

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

                 c_free(oldMsg);
            }
        } else {
            /* Notify the nodal delivery service about any synchronous
             * DataReader topology changes.
             */
            if (rInfo->reliability.synchronous) {
                v_deliveryServiceRegister(kernel->deliveryService,msg);
            }
            oldMsg = c_replace(spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                               msg, NULL, NULL);

            if (oldMsg == NULL) {
                /* A new registration.
                 */
                oldInfo = NULL;
            } else if (oldMsg->sequenceNumber >= msg->sequenceNumber) {
                /* Sample already received, ignore duplicate.
                 */
                (void)c_replace(spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                                oldMsg, NULL, NULL);
                cont = FALSE;
            } else {
                /* An update of a registration.
                 */
                oldInfo = v_builtinSubscriptionInfoData(kernel->builtin,oldMsg);
            }

            if (cont == TRUE) {
                r = v_dataReader(v_gidClaim(rInfo->key, kernel));
                /* Check if there are matching DataWriters i.e. with
                 * compatible qos policies.
                 */

                /* Read all publicationInfo, with the same topic name, this way
                 * all remote publications are also taken into account.
                 */
                offeredMessages = lookupMatchingWritersByTopic(spliced, rInfo);
                offeredMsg = c_iterTakeFirst(offeredMessages);
                while (offeredMsg != NULL) {
                    oInfo = v_builtinPublicationInfoData(kernel->builtin,offeredMsg);

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
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, FALSE);
                        }
                    } else if ((oldState == CONFORM_STATE_OK) &&
                               (newState != CONFORM_STATE_OK)) {
                        w = v_writer(v_gidClaim(oInfo->key, kernel));
                        if (newState == CONFORM_STATE_MATCH_NOT_COMPATIBLE) {
                            notifyIncompatibleQos(oInfo, rInfo, w, r);
                        }
                        if (w != NULL) {
                            /* notify the writer that a new subscription no longer matched */
                            v_writerNotifyPublicationMatched(w, rInfo->key, TRUE);
                            v_gidRelease(oInfo->key, kernel);
                        }
                        if (r != NULL) {
                            /* notify the new reader that an existing publication no longer matches */
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, TRUE);
                        }
                    } else if (newState == CONFORM_STATE_MATCH_NOT_COMPATIBLE) {
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
            /*
             */
            if (oldMsg != msg) {
                c_free(oldMsg);
            }
        }

        c_free(v_dataReaderSampleInstance(rSample));
        c_free(rSample); /* msg is freed here */
        rSample = NULL;

        c_mutexUnlock(&spliced->builtinDataMutex);
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
    (void)v_dataReaderTake(spliced->readers[V_PUBLICATIONINFO_ID],
                           takeOne, &oSample);

    /* For every publication check if the qos of a matching reader is
     * compatible.
     */
    if (oSample != NULL) {
        result = 1;

        c_mutexLock(&spliced->builtinDataMutex);

        /* Read all subscriptionInfo, with the same topic name, this way
           all remote subscriptions are also taken into account.
         */
        msg = v_dataReaderSampleMessage(oSample);
        oInfo = v_builtinPublicationInfoData(kernel->builtin,msg);

        if (v_dataReaderSampleInstanceStateTest(oSample, L_DISPOSED)) {
            c_bool validSample = v_dataReaderSampleStateTest(oSample, L_VALIDDATA);
            oldMsg = c_remove(spliced->builtinData[V_PUBLICATIONINFO_ID],
                              msg, NULL, NULL);
            /* If there is no predecessor but the sample is valid, then interpret the sample itself.
             * (This is for example the case when the builtin topics have been disabled, in which
             * case only the dispose message will be transmitted as a VOLATILE sample.)
             */
            if (oldMsg == NULL &&
                validSample &&
                !kernel->builtin->kernelQos->builtin.enabled) {
                oldMsg = msg;
            }

            if (oldMsg == NULL) {
                /* There is no old message and the dispose in received as an invalid-sample. So we cannot
                 * select matching readers because the partition-name in the publication-info isn't present (non-key field).
                 */
                doCleanupPublication(spliced, oInfo, &msg->writeTime, validSample, FALSE);
            } else {
                oldInfo = v_builtinPublicationInfoData(kernel->builtin,oldMsg);

                /* the resulting readers are not matched on partition so the readers that do not belong
                 * to the required partition need to be filtered out later on.
                 * This is because of the partition field being an internal sequence so no expression
                 * can be done on it*/
                requestedMessages = lookupMatchingReadersByTopic(spliced, oldInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL) {
                    rInfo = v_builtinSubscriptionInfoData(kernel->builtin,reqMsg);

                    oldState = determineState(oldInfo, rInfo);
                    r = v_dataReader(v_gidClaim(rInfo->key, kernel));

                    if (oldState == CONFORM_STATE_OK) {
                        if (r != NULL) {
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, TRUE);
                        }
                    }
                    oldLivState = CONFORM_STATE_TO_LIVELINESS_STATE(oldState, oInfo->alive);
                    newLivState = V_STATUSLIVELINESS_DELETED;

                    if (r != NULL) {
                        v_dataReaderNotifyLivelinessChanged(r, oInfo->key,
                        oldLivState, newLivState,
                        msg);
                        v_gidRelease(rInfo->key, kernel);
                    }

                    c_free(reqMsg);
                    reqMsg = c_iterTakeFirst(requestedMessages);
                }
                c_iterFree(requestedMessages);

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
                doCleanupPublication(spliced, oldInfo, &msg->writeTime, TRUE, isLocal);
            }
        } else {
            oldMsg = c_replace(spliced->builtinData[V_PUBLICATIONINFO_ID],
                               msg, NULL, NULL);
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
                (void) c_replace(spliced->builtinData[V_PUBLICATIONINFO_ID],
                                 oldMsg, NULL, NULL);
                cont = FALSE;
            } else {
                /* An update of a registration.
                 */
                oldInfo = v_builtinPublicationInfoData(kernel->builtin,oldMsg);
            }

            if (cont == TRUE) {
                assert(v_dataReaderSampleStateTest(oSample, L_VALIDDATA));
                w = v_writer(v_gidClaim(oInfo->key, kernel));

                requestedMessages = lookupMatchingReadersByTopic(spliced, oInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL) {
                    rInfo = v_builtinSubscriptionInfoData(kernel->builtin,reqMsg);

                    oldState = determineState(oldInfo, rInfo);
                    newState = determineState(oInfo, rInfo);

                    r = v_dataReader(v_gidClaim(rInfo->key, kernel));
                    if ((oldState != CONFORM_STATE_OK) &&
                        (newState == CONFORM_STATE_OK)) {
                        if (r != NULL) {
                            /* Notify the reader that a new publication matched */
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, FALSE);
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
                        if (newState == CONFORM_STATE_MATCH_NOT_COMPATIBLE) {
                            notifyIncompatibleQos(oInfo, rInfo, w, r);
                        }
                        if (r != NULL) {
                            /* Notify the reader that a publication no longer matches */
                            v_dataReaderNotifySubscriptionMatched(r, oInfo->key, TRUE);
                        }
                        if (w != NULL) {
                            /* Notify the new writer that an existing subscription matches */
                            v_writerNotifyPublicationMatched(w, rInfo->key, TRUE);
                        }
                    } else if (newState == CONFORM_STATE_MATCH_NOT_COMPATIBLE) {
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
                    reqMsg = c_iterTakeFirst(requestedMessages);
                }
                c_iterFree(requestedMessages);
                if (w != NULL) {
                    v_gidRelease(oInfo->key, kernel);
                }
            }
            if (oldMsg != msg) {
                c_free(oldMsg);
            }
        }

        c_free(v_dataReaderSampleInstance(oSample));
        c_free(oSample);

        c_mutexUnlock(&spliced->builtinDataMutex);
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
    proceed = v_dataReaderTake(spliced->readers[V_TOPICINFO_ID],
                               readerTakeAction, &samples);
    if (proceed) {
    	proceed = FALSE;

        c_mutexLock(&spliced->builtinDataMutex);

        kernel = v_objectKernel(spliced);
        s = v_dataReaderSample(c_iterTakeFirst(samples));
        while (s != NULL) {
            proceed = TRUE;
            m = v_dataReaderSampleMessage(s);
            info = v_builtinTopicInfoData(kernel->builtin,m);

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
                defineTopic(v_objectKernel(spliced), info);
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

struct matchHeartbeatHelper {
        c_ulong systemId; /* systemId from key of participantInfo sample */
        v_builtin builtin;
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
        hbInfo = v_builtinHeartbeatInfoData(helper->builtin, v_dataReaderSampleMessage(o));

        if (helper->systemId != v_gidSystemId(hbInfo->id)) {
            v_actionResultSet(result, V_PROCEED); /* Proceed when no match is found */
        } /* else a match is found, so do not proceed */
        /* Ensure this routine does not influence sample state at all */
        v_actionResultSet(result, V_SKIP);
    }
    return result;
}

int
v_splicedProcessParticipantInfo(
    v_spliced spliced)
{
    c_iter samples;
    v_dataReaderSample sample;
    v_kernel kernel;
    v_message msg, hbMsg;
    v_group group;
    struct v_participantInfo *pInfo;
    struct v_heartbeatInfo *hbInfo;
    c_bool notFound;
    v_resendScope resendScope = V_RESEND_NONE;
    v_writeResult wres;
    v_writer hbWriter;
    struct matchHeartbeatHelper helper;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    if (spliced->readers[V_PARTICIPANTINFO_ID] != NULL) {
        samples = NULL;
        kernel = v_objectKernel(spliced);
        helper.builtin = kernel->builtin;
        hbWriter = v_builtinWriterLookup(kernel->builtin, V_HEARTBEATINFO_ID);
        assert(hbWriter);

        v_dataReaderTake(spliced->readers[V_PARTICIPANTINFO_ID],
                         readerTakeAction, &samples);

        sample = v_dataReaderSample(c_iterTakeFirst(samples));
        while (sample != NULL) {
            /* Only process this instance if it isn't seen before */
            if (v_dataReaderSampleStateTest(sample, L_NEW)) {
                msg = v_dataReaderSampleMessage(sample);
                pInfo = v_builtinParticipantInfoData(kernel->builtin, msg);
                notFound = TRUE;
                helper.systemId = v_gidSystemId(pInfo->key);
                /* Lookup heartbeat with matching systemId */
                assert(spliced->readers[V_HEARTBEATINFO_ID]);
                notFound = v_dataReaderRead(spliced->readers[V_HEARTBEATINFO_ID], matchHeartbeatAction, &(helper));

                if (notFound) {
                    /* If no matching heartbeat found, create it and publish locally */
                    group = v_groupSetGet(kernel->groupSet, V_BUILTIN_PARTITION, V_HEARTBEATINFO_NAME);
                    assert(group);
                    hbMsg = v_topicMessageNew(v_builtinTopicLookup(kernel->builtin, V_HEARTBEATINFO_ID));
                    if (hbMsg) {
                        /* Create a 'fake' heartbeat message */
                        v_nodeState(hbMsg) = L_WRITE;
                        hbMsg->allocTime = v_timeGet();
                        hbMsg->qos = c_keep(hbWriter->relQos);
                        hbMsg->sequenceNumber = 0;
                        hbMsg->transactionId = 0;
                        hbMsg->writeTime = hbMsg->allocTime;
                        hbMsg->writerGID = v_publicGid(v_public(hbWriter));
                        hbMsg->writerInstanceGID = v_publicGid(NULL);

                        hbInfo = v_builtinHeartbeatInfoData(kernel->builtin, hbMsg);
                        hbInfo->period = spliced->hb.period;
                        hbInfo->id.systemId = v_gidSystemId(pInfo->key); /* Fake sender */
                        hbInfo->id.localId = v_gidLocalId(spliced->hb.id); /* Builtin-writer localId's are identical on all nodes */
                        hbInfo->id.serial = v_gidLifecycleId(spliced->hb.id);

                        wres = v_groupWrite(group, hbMsg, NULL, V_NETWORKID_ANY, &resendScope);
                        assert(wres == V_WRITE_SUCCESS);

                        c_free(hbMsg);

                        hbMsg = v_topicMessageNew(v_builtinTopicLookup(kernel->builtin, V_HEARTBEATINFO_ID));
                        if (hbMsg) {
                            v_nodeState(hbMsg) = L_UNREGISTER;
                            hbMsg->allocTime = v_timeGet();
                            hbMsg->qos = c_keep(hbWriter->relQos);
                            hbMsg->sequenceNumber = 1;
                            hbMsg->transactionId = 0;
                            hbMsg->writeTime = hbMsg->allocTime;
                            hbMsg->writerGID = v_publicGid(v_public(hbWriter));
                            hbMsg->writerInstanceGID = v_publicGid(NULL);

                            hbInfo = v_builtinHeartbeatInfoData(kernel->builtin, hbMsg);
                            hbInfo->id.systemId = v_gidSystemId(pInfo->key); /* Fake sender */
                            hbInfo->id.localId = v_gidLocalId(spliced->hb.id); /* Builtin-writer localId's are identical on all nodes */
                            hbInfo->id.serial = v_gidLifecycleId(spliced->hb.id);

                            resendScope = V_RESEND_NONE;

                            wres = v_groupWrite(group, hbMsg, NULL, V_NETWORKID_ANY, &resendScope);

                            assert(wres == V_WRITE_SUCCESS);
                            c_free(hbMsg);
                        } else {
                            OS_REPORT(OS_ERROR, "v_splicedProcessParticipantInfo", 0,
                                "Failed to allocate heartbeat unregister message");
                        }
                    } else {
                        OS_REPORT(OS_ERROR, "v_splicedProcessParticipantInfo", 0,
                            "Failed to allocate heartbeat message");
                    }
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
    v_statusAction action,
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
            oInfo = v_builtinPublicationInfoData(kernel->builtin, msg);
            v_gidClaim(oInfo->key, kernel);

            if (oInfo != NULL) {
                /* get topic-matching subscriptions */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingReadersByTopic(spliced, oInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL && result == V_RESULT_OK) {
                    rInfo = v_builtinSubscriptionInfoData(kernel->builtin, reqMsg);
                    r = v_dataReader(v_gidClaim(rInfo->key, kernel));
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
    v_statusAction action,
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
            oInfo = v_builtinPublicationInfoData(kernel->builtin, msg);
            v_gidClaim(oInfo->key, kernel);

            if (oInfo != NULL) {
                /* get topic-matching subscriptions */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingReadersByTopic(spliced, oInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL) {
                    rInfo = v_builtinSubscriptionInfoData(kernel->builtin, reqMsg);
                    r = v_dataReader(v_gidClaim(rInfo->key, kernel));

                    if (readerWriterMatch(rInfo, r, oInfo, w) == TRUE) {
                        if (checkOfferedRequested(oInfo, rInfo, compatible) == TRUE) {
                            /*if(v_gidCompare(rInfo->key, subscription) == C_EQ)*/
                            if (rInfo->key.systemId == subscription.systemId
                                    && rInfo->key.localId == subscription.localId) {
                                action(rInfo, arg);
                                result = V_RESULT_OK;
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
    v_statusAction action,
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
        msg = v_builtinCreateSubscriptionInfo(kernel->builtin, r);
        if (msg != NULL) {
            rInfo = v_builtinSubscriptionInfoData(kernel->builtin, msg);
            v_gidClaim(rInfo->key, kernel);

            if (rInfo != NULL) {
                /* get topic-matching publications */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingWritersByTopic(spliced, rInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL && result == V_RESULT_OK) {
                    oInfo = v_builtinPublicationInfoData(kernel->builtin, reqMsg);
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
    v_statusAction action,
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
        msg = v_builtinCreateSubscriptionInfo(kernel->builtin, r);
        if (msg != NULL) {
            rInfo = v_builtinSubscriptionInfoData(kernel->builtin, msg);
            v_gidClaim(rInfo->key, kernel);

            if (rInfo != NULL) {
                /* get topic-matching publications */
                c_mutexLock(&spliced->builtinDataMutex);

                requestedMessages = lookupMatchingWritersByTopic(spliced, rInfo);
                reqMsg = c_iterTakeFirst(requestedMessages);
                while (reqMsg != NULL) {
                    oInfo = v_builtinPublicationInfoData(kernel->builtin, reqMsg);
                    w = v_writer(v_gidClaim(oInfo->key, kernel));

                    if (readerWriterMatch(rInfo, r, oInfo, w) == TRUE) {
                        if (checkOfferedRequested(oInfo, rInfo, compatible) == TRUE) {
                            /*if(v_gidCompare(oInfo->key, publication) == C_EQ)*/
                            if (oInfo->key.systemId == publication.systemId
                                    && oInfo->key.localId == publication.localId) {
                                action(oInfo, arg);
                                result = V_RESULT_OK;
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
    c_long nrOfKeys,totalSize;
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
    sQos->presentation.access_scope = V_PRESENTATION_TOPIC;
    c_free(sQos->partition);
    sQos->partition = c_stringNew(c_getBase(c_object(kernel)),
                                  V_BUILTIN_PARTITION);
    sQos->entityFactory.autoenable_created_entities = TRUE;
    spliced->builtinSubscriber = v_subscriberNew(v_participant(spliced),
                                           "Builtin subscriber", sQos, TRUE);
    v_subscriberQosFree(sQos);

    rQos = v_readerQosNew(kernel, NULL);
    rQos->durability.kind = V_DURABILITY_TRANSIENT;
    rQos->reliability.kind = V_RELIABILITY_RELIABLE;
    rQos->history.kind = V_HISTORY_KEEPLAST;
    rQos->history.depth = 1;

    _INIT_BUILTIN_DATA_(V_TOPICINFO_ID, V_TOPICINFO_NAME);

    if (kernel->qos->builtin.enabled) {
        _INIT_BUILTIN_DATA_(V_SUBSCRIPTIONINFO_ID, V_SUBSCRIPTIONINFO_NAME);
        _INIT_BUILTIN_DATA_(V_PUBLICATIONINFO_ID, V_PUBLICATIONINFO_NAME);
    } else {
        rQos->durability.kind = V_DURABILITY_VOLATILE;
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
    topic = v_builtinTopicLookup(kernel->builtin, V_HEARTBEATINFO_ID);
    str = messageKeyExpr(topic);
    spliced->missedHB = c_tableNew(v_topicMessageType(topic), str);
    os_free(str);

    /* Setup ParticipantInfo reader */
    expr = q_parse("select * from " V_PARTICIPANTINFO_NAME " where _key.systemId != %0;");
    spliced->readers[V_PARTICIPANTINFO_ID] =
                v_dataReaderNew(spliced->builtinSubscriber,
                                V_PARTICIPANTINFO_NAME "Reader",
                                expr, params,
                                rQos, TRUE);
    v_readerQosFree(rQos);
    q_dispose(expr);

    /* Setup CandM Command reader */
    expr = q_parse("select * from " V_C_AND_M_COMMAND_NAME);
    rQos = v_readerQosNew(kernel, NULL);
    rQos->history.kind = V_HISTORY_KEEPALL;
    rQos->history.depth = V_LENGTH_UNLIMITED;
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
    v_kernel kernel)
{
    v_spliced spliced;

    assert(kernel != NULL);
    assert(C_TYPECHECK(kernel,v_kernel));

    spliced = v_spliced(v_objectNew(kernel, K_SPLICED));
    v_splicedInit(spliced);
    v_addParticipant(kernel, v_participant(spliced));

    return spliced;
}

void
v_splicedInit(
    v_spliced spliced)
{
    v_kernel kernel;
    v_participantQos q;
    char* hostName;
    c_type type;
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
        q->userData.size = strlen(hostName);
        type = c_octet_t(c_getBase(kernel));
        q->userData.value = c_arrayNew(type, q->userData.size);
        memcpy(q->userData.value, hostName, q->userData.size);
        c_free(type);
    } else {
        q->userData.size = 0;
    }
    os_free(hostName);
#undef MAX_HOST_NAME_LENGTH

    v_serviceInit(v_service(spliced), kernel->serviceManager,
                  V_SPLICED_NAME, NULL, q, NULL);
    c_free(q);
    /* replace my leaseManager (created in v_serviceInit())
     * with the kernel leaseManager */
    c_free(v_participant(spliced)->leaseManager);
    v_participant(spliced)->leaseManager = c_keep(kernel->livelinessLM);

    c_mutexInit(&spliced->mtx, SHARED_MUTEX);
    c_mutexInit(&spliced->cAndMCommandMutex, SHARED_MUTEX);
    c_mutexInit(&spliced->builtinDataMutex, SHARED_MUTEX);

    spliced->readers[V_PARTICIPANTINFO_ID] = NULL;
    spliced->readers[V_TOPICINFO_ID] = NULL;
    spliced->readers[V_PUBLICATIONINFO_ID] = NULL;
    spliced->readers[V_SUBSCRIPTIONINFO_ID] = NULL;
    spliced->readers[V_C_AND_M_COMMAND_ID] = NULL;

    spliced->builtinData[V_PARTICIPANTINFO_ID] = NULL;
    spliced->builtinData[V_TOPICINFO_ID] = NULL;
    spliced->builtinData[V_PUBLICATIONINFO_ID] = NULL;
    spliced->builtinData[V_SUBSCRIPTIONINFO_ID] = NULL;
    spliced->builtinData[V_C_AND_M_COMMAND_ID] = NULL;

    spliced->hbManager = NULL;
    spliced->ws = NULL;
    spliced->quit = FALSE;
    spliced->cAndMCommandWaitSet = v_waitsetNew(kernel->builtin->participant);
    spliced->cAndMCommandDispatcherQuit = FALSE;

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
    msg = v_topicMessageNew(v_builtinTopicLookup(kernel->builtin,
                                                 V_HEARTBEATINFO_ID));
    if (msg) {
        hb = v_builtinHeartbeatInfoData(kernel->builtin,msg);
        *hb = spliced->hb;
        v_writerWrite(v_builtinWriterLookup(kernel->builtin,
                      V_HEARTBEATINFO_ID),
                      msg, v_timeGet(),NULL);
        c_free(msg);
    }
}

static v_actionResult
alwaysProceed (
    c_object sample, c_voidp arg)
{
    v_actionResult result = 0;
    v_actionResultSet(result, V_PROCEED);
    OS_UNUSED_ARG(sample);
    OS_UNUSED_ARG(arg);
    return result;
}

void
v_splicedCheckHeartbeats(
    v_spliced spliced)
{
    struct checkHeartbeatArg arg;
    v_dataReaderSample s;
    c_iter samples;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    /* Read the heartbeats and perform the following check:
     * 'current time' - 'reception time' <= 'heartbeat period'
     * If this condition is true, the remote splicedaemon is
     * still alive, else clean-up in this kernel.
     * While reading the heartbeats, also determine the
     * smallest heartbeat period.
     */
    arg.spliced = spliced;
    arg.missed = NULL;
    arg.ct = v_timeGet();

    if (spliced->hbUpdate) {
        arg.nextPeriod = spliced->hbUpdate->duration;
    } else {
        arg.nextPeriod = spliced->hb.period;
    }
    samples = NULL;
    if (spliced->readers[V_HEARTBEATINFO_ID] != NULL) {
        v_dataReaderRead(spliced->readers[V_HEARTBEATINFO_ID],
                         readerReadAction, &samples);
        s = v_dataReaderSample(c_iterTakeFirst(samples));
        while (s != NULL) {
            /* If heartbeat is disposed, it is added to the missed set and when it is alive
             * checkHeartbeat is called.
             * Due to local simulation of remote heartbeats, an unregister may occur directly after
             * a write potentially leading to an invalid sample that is not_alive_no_writers.
             * In that case no action is needed */
            if (v_dataReaderSampleStateTest(s, L_DISPOSED)) {
                arg.missed = c_iterInsert(arg.missed, c_keep(s));
            } else if (v_dataReaderSampleStateTest(s, L_VALIDDATA)) {
                checkHeartbeat(s, &arg);
            }
            c_free(s);
            s = v_dataReaderSample(c_iterTakeFirst(samples));
        }
        c_iterFree(samples);
        /* The missed argument contains samples of heartbeats that were missed.
         * All those samples must be taken from the reader to cleanup resources.
         * Although the sender does not dispose the instance, the instance is
         * freed as soon its state becomes NOT_ALIVE_NO_WRITERS, which is
         * already done since the splicedaemon has removed the writer
         * information of the missed heartbeat.
         */
        c_mutexLock(&spliced->mtx);
        s = v_dataReaderSample(c_iterTakeFirst(arg.missed));
        while (s != NULL) {
            v_dataReaderTakeInstance(spliced->readers[V_HEARTBEATINFO_ID],
                            v_dataReaderInstance(v_readerSample(s)->instance),
                            alwaysProceed, NULL);
            if(v_dataReaderSampleStateTest(s, L_VALIDDATA))
            {
                c_insert(spliced->missedHB, v_dataReaderSampleMessage(s));
            }
            c_free(s);
            s = v_dataReaderSample(c_iterTakeFirst(arg.missed));
        }
        c_mutexUnlock(&spliced->mtx);
        c_iterFree(arg.missed);
    }
    v_leaseRenew(spliced->hbCheck, &(arg.nextPeriod));
}

/**************************************************************
 * Public functions
 **************************************************************/
#define DELAY_SEC  (0)
#define DELAY_NSEC (20000000) /* 20ms */
void
v_splicedKernelManager(
    v_spliced spliced)
{
    int dataProcessed;
    os_time delay = {DELAY_SEC, DELAY_NSEC};
    c_bool builtinEnabled;

    builtinEnabled = v_objectKernel(spliced)->qos->builtin.enabled;

    while (!spliced->quit) {
        dataProcessed = v_splicedProcessTopicInfo(spliced);
        if (dataProcessed != 0) { /* sleep and process again */
            os_nanoSleep(delay);
        }
        dataProcessed += v_splicedProcessPublicationInfo(spliced);
        if (builtinEnabled && (dataProcessed == 0)) {
            dataProcessed = v_splicedProcessSubscriptionInfo(spliced);
            dataProcessed += v_splicedProcessParticipantInfo(spliced);
        }
        if (dataProcessed == 0) {
            v_waitsetWait(spliced->ws, NULL, NULL);
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
   v_result res ;
   c_bool boolRes;
   v_dataReader reader;

   c_mutexLock(&spliced->cAndMCommandMutex);
   reader = spliced->readers[V_C_AND_M_COMMAND_ID];
   boolRes = v_waitsetAttach( spliced->cAndMCommandWaitSet,
                              (v_observable)reader,
                              NULL);

   /* The V_EVENT_TRIGGER interest is used stop the C&M command dispatcher thread. */
   v_observerSetEventMask( (v_observer)spliced->cAndMCommandWaitSet,
                            V_EVENT_DATA_AVAILABLE | V_EVENT_TRIGGER);
   assert( boolRes );

   while (!spliced->cAndMCommandDispatcherQuit)
   {
      c_mutexUnlock(&spliced->cAndMCommandMutex);
      res = v_waitsetWait( spliced->cAndMCommandWaitSet,
                           dispatchCandMCommandAction,
                           spliced );
      assert( res == V_RESULT_OK || res == V_RESULT_DETACHING);
      c_mutexLock(&spliced->cAndMCommandMutex);
   }
   c_mutexUnlock(&spliced->cAndMCommandMutex);

   boolRes = v_waitsetDetach (spliced->cAndMCommandWaitSet,
                              v_observable(reader));
   /* FIXME Do we need this? Fails on exit if I assert( boolRes ); */
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
    term.source = v_publicHandle(v_public(spliced));
    term.userData = NULL;
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
    c_time *cleanTime,
    c_bool isValid,
    c_bool isLocal)
{
    v_kernel kernel;
    int i, len;
    v_group group;
    c_iter groups;

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
        for (i = 0; i < len; i++)
        {
            groups = v_groupSetLookup(kernel->groupSet, oInfo->partition.name[i], oInfo->topic_name);
            while ((group = c_iterTakeFirst(groups)) != NULL) {
                v_groupDisconnectWriter(group, oInfo, *cleanTime, isLocal);
                c_free(group);
            }
        }
    } else {
        /* Publication-info is from invalid sample so only key-fields may be interpreted.
         * Select all groups to check if any instances of the writer must be unregistered
         */
        groups = v_groupSetSelectAll(kernel->groupSet);
        while ((group = c_iterTakeFirst(groups)) != NULL) {
            v_groupDisconnectWriter(group, oInfo, *cleanTime, isLocal);
            c_free(group);
        }
    }
}

#define GC_DELAY_SEC  (0)
#define GC_DELAY_NSEC (20000000) /* 20ms */

void
v_splicedGarbageCollector(
v_spliced spliced)
{
    c_iter groups;
    v_message missedHBMsg;
    struct v_heartbeatInfo *missedHB; /* list of missed heartbeats */
    v_kernel kernel;
    c_long length;
    v_group g;
    os_time delay = {GC_DELAY_SEC, GC_DELAY_NSEC};
    c_bool pubComplete;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);
    /* wait until kernelmanager has initialized */
    while (!spliced->missedHB)
    {
        os_nanoSleep(delay);
    }
    /*Continue for as long as the spliced doesn't need to terminate*/
    while (!spliced->quit)
    {
        /*Check if a heartbeat has been missed*/
        c_mutexLock(&spliced->mtx);
        missedHBMsg = c_take(spliced->missedHB);
        c_mutexUnlock(&spliced->mtx);


        /* If a heartbeat has been missed, walk over all groups and clean up
         * data from writers on the removed node.
         */
        if (missedHBMsg)
        {
            missedHB = v_builtinHeartbeatInfoData(kernel->builtin, missedHBMsg);

            /* Wait for alignment of DCPSPublication topic */
            pubComplete = v_readerWaitForHistoricalData(
                v_reader(spliced->readers[V_PUBLICATIONINFO_ID]), C_TIME_ZERO);

            OS_REPORT_2(OS_WARNING, "v_spliced", 0,
                "Missed heartbeat for node - heartbeat id : %u. (DCPSPublication state %d)",
                v_gidSystemId(missedHB->id), pubComplete);

            if (kernel->builtin->kernelQos->builtin.enabled && pubComplete)
            {
                /* In case the builtin topics are enabled, we only need to remove the
                 * DCPSPublication samples that originate from the disconnected node.
                 * The v_splicedProcessPublicationInfo() function will then take it
                 * from there.
                 */
                g = v_groupSetGet(kernel->groupSet, V_BUILTIN_PARTITION, V_PUBLICATIONINFO_NAME);
                /* A group does not necessarily have to exist locally! */
                if (g != NULL)
                {
                    /* Disconnect all the node's registrations from this group. Note
                     * that the heartbeat's period is an effectively cleanup time.
                     */
                    v_groupDisconnectNode(g, missedHB);
                    os_nanoSleep(delay);
                    c_free(g);
                    g = NULL;
                }
            }
            else
            {
                /* In case the builtin topics are disabled, we need to walk over all
                 * available groups and remove everything that originates from the
                 * disconnected node. This is also the case if DCPSPublication
                 * is not complete.
                 */
                groups = v_groupSetSelectAll(kernel->groupSet);
                g = v_group(c_iterTakeFirst(groups));
                while ((g != NULL) && (!spliced->quit))
                {
                    v_groupDisconnectNode(g, missedHB);
                    os_nanoSleep(delay);
                    c_free(g);
                    g = v_group(c_iterTakeFirst(groups));
                }
            }
            c_free(missedHBMsg);

        }
        else
        {
            /*If no heartbeat is missed, do some garbage collection.*/
            groups = v_groupSetSelectAll(kernel->groupSet);
            g = v_group(c_iterTakeFirst(groups));
            while ((g != NULL) && (!spliced->quit))
            {
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
                if(length != 0)
                {
                    g = NULL;
                }
                else
                {
                    os_nanoSleep(delay);
                    g = v_group(c_iterTakeFirst(groups));
                }
            }
            /* Make sure each group is freed if the while loops above are
             * interrupted.
             */
            if(g != NULL)
            {
                c_free(g);
            }
            g = v_group(c_iterTakeFirst(groups));

            while(g != NULL)
            {
                c_free(g);
                g = v_group(c_iterTakeFirst(groups));
            }
            c_iterFree(groups);
        }
    }
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
    v_duration period,
    v_duration renewal,
    c_long priority)
{
    v_kernel kernel;
    c_bool started;
    v_writer w;
    v_writerQos wqos;
    v_result result;
    v_leaseManager lm;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    w = v_builtinWriterLookup(kernel->builtin, V_HEARTBEATINFO_ID);
    if (w && (w->qos->transport.value != priority))
    {
        wqos = v_writerQosNew(kernel, w->qos);
        if (wqos)
        {
            wqos->transport.value = priority;
            v_writerSetQos(w, wqos);
            v_writerQosFree(wqos);
        }
    }

    lm = spliced->hbManager;
    if (!lm)
    {
       lm = kernel->livelinessLM;
    }

    spliced->hb.period = period;
    /* The check for heartbeats should be performed at least once every
     * MIN(RemoteSpliced_0_ExpiryTime, ..., RemoteSpliced_n_ExpiryTime). Neither
     * period nor renewal are correct here. Because renewal is shorter, chances
     * that it satisfies the above are greater, but still not correct. See also
     * dds2898 */
    spliced->hbCheck = v_leaseNew(kernel, renewal);
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
            OS_REPORT_1(OS_ERROR, "v_spliced", 0,
                "A fatal error was detected when trying to register the spliced liveliness hbCheck lease "
                "to the liveliness lease manager of the kernel. The result code was %d.", result);
        }
    }

    spliced->hbUpdate = v_leaseNew(kernel, renewal);
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
            OS_REPORT_1(OS_ERROR, "v_spliced", 0,
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
    c_time timestamp)
{
   v_kernel kernel;
   c_iter list;
   v_group group;
   v_topic topic;
   v_writeResult res;
   struct v_commandDisposeAllData *disposeCmd;

   assert(spliced != NULL);
   assert(C_TYPECHECK(spliced,v_spliced));

   disposeCmd = &command->u._u.dispose_all_data_info;

   kernel = v_objectKernel(spliced);
   list = v_groupSetLookup(kernel->groupSet,
                            disposeCmd->partitionExpr,
                            disposeCmd->topicExpr);
   if (list)
   {
       group = c_iterTakeFirst(list);
       if ( group != NULL )
       {
          while (group)
          {
             res = v_groupDisposeAll( group, timestamp, 0 );
             if ( res != V_WRITE_SUCCESS )
             {
                OS_REPORT(OS_WARNING, "spliced", 0,
                          "Dispose All Data failed due to internal error.");
             }

             topic = v_groupTopic( group );

             v_observerLock( v_observer(topic) );
             v_topicNotifyAllDataDisposed( topic );
             v_observerUnlock( v_observer(topic) );

             c_free(group);
             group = c_iterTakeFirst(list);
          }
       }
       else
       {
          /* Group does not exist yet, store the timestamp etc for when the group is
             created */
          v_pendingDisposeElement element = NULL;
          c_base base = c_getBase(c_object(spliced));
          int found = 0;
          c_long i;

          c_mutexLock(&kernel->pendingDisposeListMutex);
          for(i=0; (i<c_listCount(kernel->pendingDisposeList)); i++)
          {
             element =
             (v_pendingDisposeElement)c_readAt(kernel->pendingDisposeList, i);
             if ( !strcmp( element->disposeCmd.topicExpr, disposeCmd->topicExpr)
                  && !strcmp( element->disposeCmd.partitionExpr,
                              disposeCmd->partitionExpr ))
             {
                found = 1;
                if ( c_timeCompare( element->disposeTimestamp, timestamp ) == C_LT )
                {
                   /* Already an older existing record for this partition
                      and topic combination - update timestamp */
                   element->disposeTimestamp = timestamp;
                }
                break;
             }
          }
          if ( !found )
          {
             v_pendingDisposeElement new;

             new = c_new( v_kernelType(kernel, K_PENDINGDISPOSEELEMENT ) );
             if (new) {
                 new->disposeCmd.topicExpr = c_stringNew(base, disposeCmd->topicExpr);
                 new->disposeCmd.partitionExpr = c_stringNew(base,
                                                             disposeCmd->partitionExpr);
                 new->disposeTimestamp = timestamp;

                 c_append( kernel->pendingDisposeList, new );
             } else {
                 OS_REPORT(OS_ERROR,
                           "v_spliced::disposeAllDataCandMCommand", 0,
                           "Failed to allocated v_pendingDisposeElement object.");
                 assert(FALSE);
             }
          }
          c_mutexUnlock(&kernel->pendingDisposeListMutex);
       }
   }
   c_iterFree(list);
}

typedef void (*cAndMCommandFn_t)(v_spliced spliced,
                                 v_controlAndMonitoringCommand *command,
                                 c_time timestamp);

/* This array is indexed by values of the
   enum v_controlAndMonitoringCommandKind */
static cAndMCommandFn_t cAndMCommandFns[] =
{
   disposeAllDataCandMCommand
};

static void dispatchCandMCommand(v_spliced spliced,
                                 v_dataReaderSample s,
                                 c_time timestamp)
{
   v_kernel kernel;
   v_message msg;
   v_controlAndMonitoringCommand *command;

   assert(spliced != NULL);
   assert(C_TYPECHECK(spliced,v_spliced));
   assert(s != NULL);
   assert(C_TYPECHECK(s,v_dataReaderSample));

   kernel = v_objectKernel(spliced);
   msg = v_dataReaderSampleMessage(s);
   command = v_builtinControlAndMonitoringCommandData(kernel->builtin, msg);
   if ( command->u._d < sizeof(cAndMCommandFns)/sizeof(cAndMCommandFn_t) )
   {
      cAndMCommandFns[command->u._d]( spliced, command, timestamp );
   }
   else
   {
      OS_REPORT(OS_WARNING, "spliced", 0,
                "unknown Control and Monitoring Command received.");
   }
}

static void v_splicedTakeCandMCommand(v_spliced spliced)
{
    v_dataReaderSample s;
    v_dataReader reader;
    c_iter samples;
    c_time timestamp;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    samples = NULL;
    if ( (reader = spliced->readers[V_C_AND_M_COMMAND_ID]) != NULL )
    {
        v_dataReaderTake(reader, readerTakeAction, &samples);
        while ( (s = v_dataReaderSample(c_iterTakeFirst(samples))) != NULL )
        {
           timestamp = v_dataReaderSample(s)->insertTime;
           dispatchCandMCommand(spliced, s, timestamp);
           c_free(s);
        }
        c_iterFree(samples);
    }
}

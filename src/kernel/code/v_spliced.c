/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech
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

#define HB_VIEW_NAME   "heartbeat view"
#define HB_VIEW_EXPR   "select * from " V_HEARTBEATINFO_NAME " where id.systemId != %0;"
#define HB_READER_NAME V_HEARTBEATINFO_NAME "Reader"

#define HB_DEFAULT_SEC  (20)
#define HB_DEFAULT_NSEC (0)
#define HB_DEFAULT_RENEWAL_SEC  (1)
#define HB_DEFAULT_RENEWAL_NSEC (0)

static void
doCleanupPublication(
    v_spliced spliced,
    struct v_publicationInfo *oInfo,
    c_time *cleanTime);

/**************************************************************
 * Private functions
 **************************************************************/
static c_bool
readerAction(
    v_readerSample s,
    c_voidp arg)
{
    c_iter *samples = (c_iter *)arg;

    if ((s != NULL) && v_stateTest(s->sampleState, L_VALIDDATA)) {
/* use append, since the oldest message is given first to this copy routine! */
        *samples = c_iterAppend(*samples, c_keep(s));
    } /* else last sample is read */

    return TRUE;
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

static c_bool
takeOne(
    v_readerSample s,
    c_voidp arg)
{
    v_readerSample *sample = (v_readerSample *)arg;
    c_bool result;

    if (s != NULL) {
        result = TRUE;
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
            result = FALSE;
        }
    } else { /* last sample */
        result = FALSE;
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

    if (v_objectKind(o) == K_DOMAIN) {
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
    v_kernel kernel,
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
        newTopic = v__topicNew(kernel, info->name,
                              info->type_name, info->key_list, qos, FALSE);
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

/* returns 0, when no DCPSSubscriptionInfo sample was available.
 * Otherwise 1 is returned */
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
    v_policyId compatible[V_POLICY_ID_COUNT];
    v_policyId id;
    enum v_statusLiveliness newLivState;
    enum v_statusLiveliness oldLivState;
    v_state state;

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

        msg = v_dataReaderSampleMessage(rSample);
        rInfo = v_builtinSubscriptionInfoData(kernel->builtin,msg);
        state = v_readerSample(rSample)->sampleState;

        if (v_stateTest(v_readerSample(rSample)->sampleState, L_DISPOSED)) {
            /* The subscription is disposed so the following
             * code will remove and free the registration.
             */
            oldMsg = c_remove(spliced->builtinData[V_SUBSCRIPTIONINFO_ID],
                              msg, NULL, NULL);
            if (oldMsg) {
               /* Notify the nodal delivery service about any synchronous
                * DataReader topology changes.
                */
                struct v_subscriptionInfo *rInfo;
                rInfo = v_builtinSubscriptionInfoData(kernel->builtin,oldMsg);
                if (rInfo->reliability.synchronous) {
                    v_deliveryServiceUnregister(kernel->deliveryService,oldMsg);
                }
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

            if (oldMsg != NULL) {
                /* An update of a registration. */
                oldInfo = v_builtinSubscriptionInfoData(kernel->builtin,oldMsg);
            } else {
                /* A new registration. */
                oldInfo = NULL;
            }

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
                w = v_writer(v_gidClaim(oInfo->key, kernel));
                if (readerWriterMatch(kernel, rInfo, r, oInfo, w) == TRUE) {
                    if (checkOfferedRequested(oInfo, rInfo, compatible) == FALSE) {
                        for (id = 0; id < V_POLICY_ID_COUNT; id++) {
                            if (compatible[id] == FALSE) {
                                if (w != NULL) {
                                    v_writerNotifyIncompatibleQos(w, id);
                                }
                                if (r != NULL) {
                                    v_dataReaderNotifyIncompatibleQos(r, id, oInfo->key);
                                }
                            }
                        }
                        newLivState = V_STATUSLIVELINESS_NOTALIVE;
                    } else {
                        if (oInfo->alive == TRUE) {
                            newLivState = V_STATUSLIVELINESS_ALIVE;
                        } else {
                            newLivState = V_STATUSLIVELINESS_NOTALIVE;
                        }
                    }
                } else {
                    newLivState = V_STATUSLIVELINESS_DELETED;
                }
                if (r != NULL) {
                   /* Determine old liveliness state.
                    * Pretend r == NULL, so match is entirely determined by
                    * the old reader partition information!
                    * We need to determine the previous state!
                    */
                    if (oldInfo != NULL) {
                        if (readerWriterMatch(kernel, oldInfo, NULL, oInfo, w)) {
                            if (checkOfferedRequested(oInfo, oldInfo, compatible) == TRUE) {
                                oldLivState = (oInfo->alive?V_STATUSLIVELINESS_ALIVE:V_STATUSLIVELINESS_NOTALIVE);
                            } else {
                                oldLivState = V_STATUSLIVELINESS_NOTALIVE;
                            }
                        } else {
                            oldLivState = V_STATUSLIVELINESS_DELETED;
                        }
                    } else {
                        oldLivState = V_STATUSLIVELINESS_UNKNOWN;
                    }
                    v_dataReaderNotifyLivelinessChanged(r, oInfo->key,
                                                        oldLivState,
                                                        newLivState,
                                                        offeredMsg);
                }
                if (w != NULL) {
                    v_gidRelease(oInfo->key, kernel);
                }

                c_free(offeredMsg);
                offeredMsg = c_iterTakeFirst(offeredMessages);
            }
            c_iterFree(offeredMessages);
            /*
             */
            if (oldMsg != msg) {
                c_free(oldMsg);
            }
            if (r != NULL) {
                v_gidRelease(rInfo->key, kernel);
            }
        }
        c_free(v_dataReaderSampleInstance(rSample));
        c_free(rSample); /* msg is freed here */
        rSample = NULL;
    }

    return result;
}

/* returns 0, when no DCPSPublicationInfo sample was available.
 * Otherwise 1 is returned */
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
    struct v_publicationInfo *oldInfo;
    v_kernel kernel;
    v_writer w;
    v_dataReader r;
    v_policyId compatible[V_POLICY_ID_COUNT];
    v_policyId id;
    enum v_statusLiveliness newLivState;
    enum v_statusLiveliness oldLivState;
    c_time curTime;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    curTime = v_timeGet();
    kernel = v_objectKernel(spliced);

    oSample = NULL;
    (void)v_dataReaderTake(spliced->readers[V_PUBLICATIONINFO_ID],
                           takeOne, &oSample);

    /* For every publication check if the qos of a matching reader is
     * compatible.
     */
    if (oSample != NULL) {
        result = 1;

        /* Read all subscriptionInfo, with the same topic name, this way
           all remote subscriptions are also taken into account.
         */
        msg = v_dataReaderSampleMessage(oSample);
        oInfo = v_builtinPublicationInfoData(kernel->builtin,msg);
        w = v_writer(v_gidClaim(oInfo->key, kernel));

        if (v_dataReaderSampleInstanceStateTest(oSample, L_DISPOSED)) {
            oldMsg = c_remove(spliced->builtinData[V_PUBLICATIONINFO_ID],
                              msg, NULL, NULL);
        } else {
            oldMsg = c_replace(spliced->builtinData[V_PUBLICATIONINFO_ID],
                               msg, NULL, NULL);
            /* Do not free oldMsg yet!
             * We need it to determine the previous liveliness state
             * of the writer.
             */
        }
        if (oldMsg != NULL) {
            oldInfo = v_builtinPublicationInfoData(kernel->builtin,oldMsg);
        } else {
            oldInfo = NULL;
        }

        if (v_dataReaderSampleStateTest(oSample, L_VALIDDATA)) {
            requestedMessages = lookupMatchingReadersByTopic(spliced, oInfo);
        } else {
            if (oldInfo != NULL) {
                requestedMessages = lookupMatchingReadersByTopic(spliced, oldInfo);
            } else {
                requestedMessages = NULL;
            }
        }
        reqMsg = c_iterTakeFirst(requestedMessages);
        while (reqMsg != NULL) {
            rInfo = v_builtinSubscriptionInfoData(kernel->builtin,reqMsg);
            r = v_dataReader(v_gidClaim(rInfo->key, kernel));

            if ((!v_dataReaderSampleInstanceStateTest(oSample, L_DISPOSED)) &&
                (readerWriterMatch(kernel, rInfo, r, oInfo, w) == TRUE)) {
                if (checkOfferedRequested(oInfo, rInfo, compatible) == FALSE) {
                    for (id = 0; id < V_POLICY_ID_COUNT; id++) {
                        if (compatible[id] == FALSE) {
                            if (w != NULL) {
                                v_writerNotifyIncompatibleQos(w, id);
                            }
                            if (r != NULL) {
                                v_dataReaderNotifyIncompatibleQos(r, id,
                                                                  oInfo->key);
                            }
                        }
                    }
                    newLivState = V_STATUSLIVELINESS_NOTALIVE;
                } else {
                    if (oInfo->alive == TRUE) {
                        newLivState = V_STATUSLIVELINESS_ALIVE;
                    } else {
                        newLivState = V_STATUSLIVELINESS_NOTALIVE;
                    }
                }
            } else {
                newLivState = V_STATUSLIVELINESS_DELETED;
            }

            if (r != NULL) {
                /* Determine old liveliness state.
                 * Pretend w == NULL, so match is entirely determined by
                 * the old writer partition information!
                 * We need to determine the previous state!
                 */
                if (oldInfo != NULL) {
                    if (readerWriterMatch(kernel, rInfo, r, oldInfo, NULL)) {
                        if (checkOfferedRequested(oldInfo, rInfo, compatible) == TRUE) {
                            oldLivState = (oldInfo->alive?V_STATUSLIVELINESS_ALIVE:V_STATUSLIVELINESS_NOTALIVE);
                        } else {
                            oldLivState = V_STATUSLIVELINESS_NOTALIVE;
                        }
                    } else {
                        oldLivState = V_STATUSLIVELINESS_DELETED;
                    }
                } else {
                    oldLivState = V_STATUSLIVELINESS_UNKNOWN;
                }
                v_dataReaderNotifyLivelinessChanged(r, oInfo->key,
                                                    oldLivState, newLivState,
                                                    msg);
                v_gidRelease(rInfo->key, kernel);
            }

            c_free(reqMsg);
            reqMsg = c_iterTakeFirst(requestedMessages);
        }
        c_iterFree(requestedMessages);

        if ((v_stateTest(v_readerSample(oSample)->sampleState, L_DISPOSED)) &&
            (oInfo->key.systemId != kernel->GID.systemId)) {
            /* If the message is disposed, the writer is no longer alive,
             * so cleanup all resources taken by this writer. But only if this
             * writer is a remote writer!
             * DO NOT USE local time, but use the production time of the
             * builtin topic! This is needed, since the time might not be
             * aligned properly!
             */
            doCleanupPublication(spliced, oInfo, &msg->writeTime);
        }


        if (oldMsg != msg) {
            c_free(oldMsg);
        }
        if (w != NULL) {
            v_gidRelease(oInfo->key, kernel);
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
    proceed = v_dataReaderTake(spliced->readers[V_TOPICINFO_ID],
                               readerAction, &samples);
    if (proceed) {
    	proceed = FALSE;
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
    }
    assert(c_iterLength(samples) == 0);
    c_iterFree(samples);

    if (proceed) {
        return 1;
    } else {
        return 0;
    }
}

/* We are not interested in participant information at the moment, so
   just take the data away to prevent the reader cache from growing
   indefinately.
*/
/* returns 0, when no DCPSParticipantInfo sample was available. Otherwise 1 is returned */
int
v_splicedProcessParticipantInfo(
    v_spliced spliced)
{
    c_iter samples;
    v_dataReaderSample s;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    if (spliced->readers[V_PARTICIPANTINFO_ID] != NULL) {
        samples = NULL;
        v_dataReaderTake(spliced->readers[V_PARTICIPANTINFO_ID],
                         readerAction, &samples);
        s = v_dataReaderSample(c_iterTakeFirst(samples));
        while (s != NULL) {
            c_free(s);
            s = v_dataReaderSample(c_iterTakeFirst(samples));
        }
        c_iterFree(samples);
    }

    return 0; /* is just a garbage collector, so no worries */
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
            strcat(keyExpr,"userData.");
            strcat(keyExpr,fieldName);
            os_free(fieldName);
            fieldName = c_iterTakeFirst(keyNames);
            if (fieldName) {
                strcat(keyExpr,",");
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

    if (kernel->qos->builtin.enabled) {



        spliced->builtinData[V_PARTICIPANTINFO_ID] = NULL;


        _INIT_BUILTIN_DATA_(V_SUBSCRIPTIONINFO_ID, V_SUBSCRIPTIONINFO_NAME);
        _INIT_BUILTIN_DATA_(V_PUBLICATIONINFO_ID, V_PUBLICATIONINFO_NAME);
    }

    _INIT_BUILTIN_DATA_(V_TOPICINFO_ID, V_TOPICINFO_NAME);

    v_readerQosFree(rQos);

    kernel->deliveryService = v_deliveryServiceNew(
                                      spliced->builtinSubscriber,
                                      "deliveryService");

    spliced->readers[V_DELIVERYINFO_ID] = c_keep(kernel->deliveryService);

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

    expr = q_parse(HB_VIEW_EXPR);
    params[0] = c_ulongValue(v_gidSystemId(spliced->hb.id));

    spliced->readers[V_HEARTBEATINFO_ID] =
                v_dataReaderNew(spliced->builtinSubscriber,
                                HB_READER_NAME,
                                expr, params,
                                NULL, TRUE);
    q_dispose(expr);
    topic = v_builtinTopicLookup(kernel->builtin, V_HEARTBEATINFO_ID);
    str = messageKeyExpr(topic);
    spliced->missedHB = c_tableNew(v_topicMessageType(topic), str);
    os_free(str);
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

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);
    q = v_participantQosNew(kernel, NULL);
    v_serviceInit(v_service(spliced), kernel->serviceManager,
                  V_SPLICED_NAME, NULL, q, NULL);
    c_free(q);
    /* replace my leaseManager (created in v_serviceInit())
     * with the kernel leaseManager */
    c_free(v_participant(spliced)->leaseManager);
    v_participant(spliced)->leaseManager = c_keep(kernel->livelinessLM);

    c_mutexInit(&spliced->mtx, SHARED_MUTEX);

    spliced->readers[V_PARTICIPANTINFO_ID] = NULL;
    spliced->readers[V_TOPICINFO_ID] = NULL;
    spliced->readers[V_PUBLICATIONINFO_ID] = NULL;
    spliced->readers[V_SUBSCRIPTIONINFO_ID] = NULL;

    spliced->builtinData[V_PARTICIPANTINFO_ID] = NULL;
    spliced->builtinData[V_TOPICINFO_ID] = NULL;
    spliced->builtinData[V_PUBLICATIONINFO_ID] = NULL;
    spliced->builtinData[V_SUBSCRIPTIONINFO_ID] = NULL;

    spliced->ws = NULL;
    spliced->quit = FALSE;
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
}

void
v_splicedDeinit(
    v_spliced spliced)
{
    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    /* prevent that the kernel lease manager is destroyed */
    c_free(v_participant(spliced)->leaseManager);
    v_participant(spliced)->leaseManager = NULL;

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
        v_writerWrite(v_builtinWriterLookup(kernel->builtin,V_HEARTBEATINFO_ID),
                      msg, v_timeGet(),NULL);
        c_free(msg);
    }
}

static c_bool alwaysTrue (v_readerSample sample, c_voidp arg) { return TRUE; }

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
                         readerAction, &samples);
        s = v_dataReaderSample(c_iterTakeFirst(samples));
        while (s != NULL) {
            checkHeartbeat(s, &arg);
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
                            alwaysTrue, NULL);

            c_insert(spliced->missedHB, v_dataReaderSampleMessage(s));
            c_free(s);
            s = v_dataReaderSample(c_iterTakeFirst(arg.missed));
        }
        c_mutexUnlock(&spliced->mtx);
        c_iterFree(arg.missed);
    }
    v_leaseRenew(spliced->hbCheck, arg.nextPeriod);
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
        if (builtinEnabled && (dataProcessed == 0)) {
            dataProcessed = v_splicedProcessSubscriptionInfo(spliced);
            dataProcessed += v_splicedProcessParticipantInfo(spliced);
            dataProcessed += v_splicedProcessPublicationInfo(spliced);
        }
        if (dataProcessed == 0) {
            v_waitsetWait(spliced->ws, NULL, NULL);
        } else { /* sleep and process again */
            os_nanoSleep(delay);
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
cleanup(
   v_message regMsg,
   v_group g,
   c_time *writeTime)
{
    v_message msg;

    if (v_messageQos_isAutoDispose(regMsg->qos)) {
        msg = v_topicMessageNew(g->topic);
        if (msg) {
            v_topicMessageCopyKeyValues(g->topic, msg, regMsg);
            v_nodeState(msg) = L_DISPOSED;
            msg->qos = c_keep(regMsg->qos); /* since messageQos does not contain refs */
            msg->writerGID = regMsg->writerGID; /* pretend this message comes from the original writer! */
            msg->writeTime = *writeTime;
            v_groupWrite(g, msg, NULL, V_NETWORKID_ANY);
            c_free(msg);
        }
    }
    msg = v_topicMessageNew(g->topic);
    if (msg) {
        v_topicMessageCopyKeyValues(g->topic, msg, regMsg);
        v_nodeState(msg) = L_UNREGISTER;
        msg->qos = c_keep(regMsg->qos); /* since messageQos does not contain refs */
        msg->writerGID = regMsg->writerGID; /* pretend this message comes from the original writer! */
        msg->writeTime = *writeTime;
        v_groupWrite(g, msg, NULL, V_NETWORKID_ANY);
        c_free(msg);
    }
}

static void
doCleanupPublication(
    v_spliced spliced,
    struct v_publicationInfo *oInfo,
    c_time *cleanTime)
{
    v_kernel kernel;
    int i, len;
    v_group group;
    c_iter messages;
    v_message regMsg;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));
    assert(oInfo != NULL);

    kernel = v_objectKernel(spliced);
    /* now for every partition in the partition info find the group and notify
     * the group that all instances of the writer must be unregistered
     */
    len = c_arraySize(oInfo->partition.name);
    for (i = 0; i < len; i++) {
        group = v_groupSetGet(kernel->groupSet,
            oInfo->partition.name[i],
            oInfo->topic_name);
        /* A group does not necessarily have to exist locally! */
        if (group != NULL) {
            messages = v_groupGetRegisterMessagesOfWriter(group, oInfo->key);
            regMsg = v_message(c_iterTakeFirst(messages));
            while (regMsg != NULL) {
                cleanup(regMsg, group, cleanTime);
                regMsg = v_message(c_iterTakeFirst(messages));
            }
            c_iterFree(messages);
            c_free(group);
        }
    }
}

#define GC_DELAY_SEC  (0)
#define GC_DELAY_NSEC (20000000) /* 20ms */
static void
cleanupWriters(
    c_iter deadWriters,
    v_group g,
    c_time *writeTime)
{
    v_message regMsg;

    regMsg = v_message(c_iterTakeFirst(deadWriters));
    while (regMsg != NULL) {
        cleanup(regMsg, g, writeTime);
        c_free(regMsg);
        regMsg = v_message(c_iterTakeFirst(deadWriters));
    }
    c_iterFree(deadWriters);
}

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
    c_iter deadWriters; /* list of register messages of died writers */

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);
    /* wait until kernelmanager has initialized */
    while (!spliced->missedHB) {
        os_nanoSleep(delay);
    }
    /*Continue for as long as the spliced doesn't need to terminate*/
    while (!spliced->quit) {
        /*Check if a heartbeat has been missed*/
        c_mutexLock(&spliced->mtx);
        missedHBMsg = c_take(spliced->missedHB);
        c_mutexUnlock(&spliced->mtx);

        groups = v_groupSetSelectAll(kernel->groupSet);
        g = v_group(c_iterTakeFirst(groups));

        /* If a heartbeat has been missed, walk over all groups and clean up
         * data from writers on the removed node.
         */
        if (missedHBMsg) {
            missedHB = v_builtinHeartbeatInfoData(kernel->builtin,missedHBMsg);
            OS_REPORT_1(OS_WARNING, "v_spliced", 0,
                "Missed heartbeat for node %d", v_gidSystemId(missedHB->id));

            while ((g != NULL) && (!spliced->quit)) {
                os_nanoSleep(delay);
                deadWriters = v_groupGetRegisterMessages(g, v_gidSystemId(missedHB->id));
                cleanupWriters(deadWriters, g, &missedHB->period /* effectively cleanup time */);
                v_groupUpdatePurgeList(g);
                c_free(g);
                g = v_group(c_iterTakeFirst(groups));
            }
            c_free(missedHBMsg);
        } else {
            /*If no heartbeat is missed, do some garbage collection.*/
            while ((g != NULL) && (!spliced->quit)) {
                v_groupUpdatePurgeList(g);
                c_free(g);

                /* Missed heartbeat handling takes precedence over the updating
                 * of purgeLists. If a heartbeat has been missed, stop
                 * updating purgeLists.
                 */
                c_mutexLock(&spliced->mtx);
                length = c_setCount(spliced->missedHB);
                c_mutexUnlock(&spliced->mtx);

                /* A heartbeat has been missed, stop updating purgeLists now
                 * but don't forget to free the iter of groups.
                 */
                if(length != 0){
                    g = NULL;
                } else {
                    os_nanoSleep(delay);
                    g = v_group(c_iterTakeFirst(groups));
                }
            }
        }
        /* Make sure each group is freed if the while loops above are
         * interrupted.
         */
        if(g != NULL){
            c_free(g);
        }
        g = v_group(c_iterTakeFirst(groups));

        while(g != NULL){
            c_free(g);
            g = v_group(c_iterTakeFirst(groups));
        }
        c_iterFree(groups);
    }
}

c_bool
v_splicedStartHeartbeat(
    v_spliced spliced,
    v_duration period,
    v_duration renewal)
{
    v_kernel kernel;
    c_bool started;

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);
    spliced->hb.period = period;
    spliced->hbCheck = v_leaseManagerRegister(kernel->livelinessLM,
                            v_public(spliced), renewal,
                            V_LEASEACTION_HEARTBEAT_CHECK,
                            V_LEASE_REPEAT_INFINITE);
    spliced->hbUpdate = v_leaseManagerRegister(kernel->livelinessLM,
                            v_public(spliced), renewal,
                            V_LEASEACTION_HEARTBEAT_SEND,
                            V_LEASE_REPEAT_INFINITE);
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

    assert(spliced != NULL);
    assert(C_TYPECHECK(spliced,v_spliced));

    kernel = v_objectKernel(spliced);

    v_leaseManagerDeregister(kernel->livelinessLM, spliced->hbCheck);
    c_free(spliced->hbCheck);
    spliced->hbCheck = NULL;
    v_leaseManagerDeregister(kernel->livelinessLM, spliced->hbUpdate);
    c_free(spliced->hbUpdate);
    spliced->hbUpdate = NULL;

    return TRUE;
}

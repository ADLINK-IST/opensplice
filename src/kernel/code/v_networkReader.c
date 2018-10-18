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

/* Interface */
#include "v__networkReader.h"

/* Implementation */
#include "vortex_os.h"
#include "os_report.h"
#include "os_atomics.h"
#include "c_base.h"
#include "kernelModuleI.h"
#include "v__networkQueue.h"
#include "v_networkReaderEntry.h"
#include "v_networkReaderStatistics.h"
#include "v_networkChannelStatistics.h"
#include "v_networkQueueStatistics.h"
#include "v_entity.h"    /* for v_entity() */
#include "v_observer.h"
#include "v_group.h"     /* for v_group()  */
#include "v__reader.h"    /* for v_reader() */
#include "v_readerQos.h" /* for v_readerQosNew() */
#include "v_entry.h"     /* for v_entry()  */
#include "v_subscriber.h"
#include "v_groupSet.h"
#include "v_topic.h"
#include "v_messageQos.h"
#include "v_statistics.h"
#include "v__statCat.h"
#include "v__kernel.h"
#include "v_networking.h"

#define ORDER_INVERSION_BUGFIX 1

#define MIN_SLEEPTIME {0, 0}

#define SLEEP_UNTIL(until) \
    v_networkQueueSleepUntil(until)
#define CONDWAIT_UNTIL(cvRef, mutexRef, until) \
    v_networkQueueCondWaitUntil(cvRef, mutexRef, until)

#define NW_MAX_QUEUE_CACHE_PRIO (100)

#define NW_MAX_NOF_QUEUES (42)

v_networkReader
v_networkReaderNew(
    v_subscriber subscriber,
    const c_char *name,
    v_readerQos qos,
    c_bool ignoreReliabilityQoS)
{
    /* Note: currently, no qos-es are supported. Everything is redirected
     *       to the defaultQueue
     */

    v_kernel kernel;
    v_networkReader reader = NULL;
    v_result r;
    v_readerQos q;
    c_type queueType;
    c_long i;

    assert(C_TYPECHECK(subscriber,v_subscriber));

    /* Creation */
    kernel = v_objectKernel(subscriber);
    if (v_readerQosCheck(qos) == V_RESULT_OK) {
        q = v_readerQosNew(kernel,qos);

        if (q != NULL) {
            reader = v_networkReader(v_objectNew(kernel,K_NETWORKREADER));
            reader->statistics = v_networkReaderStatisticsNew(kernel);

            /* Initialization of parent */
            v_readerInit(v_reader(reader), name, subscriber, q);
            (void)v_entityEnable(v_entity(reader));

            c_free(q); /* ref now in v_reader(queue)->qos */

            /* This function only ever called once per network instance so no
             * need to store queueType as static variable.  Look up as needed (once)
             */
            queueType = c_resolve(c_getBase(subscriber),"kernelModuleI::v_networkQueue");
            /* Initialization of self */
            reader->queues = NULL;
            reader->queues = c_arrayNew_s(queueType, NW_MAX_NOF_QUEUES);
            if (!reader->queues) {
                OS_REPORT(OS_ERROR, "v_networkReaderNew", V_RESULT_OUT_OF_MEMORY,
                    "Creation of NetworkReader <%s> failed: cannot create queues",
                    name);
                goto err_alloc_queues;
            }
            reader->nofQueues = 0;
            reader->defaultQueue = NULL;
            reader->remoteActivity = FALSE;
            reader->ignoreReliabilityQoS = ignoreReliabilityQoS;
            reader->queueCache = c_arrayNew_s(queueType, 2*NW_MAX_QUEUE_CACHE_PRIO);
            if (!reader->queueCache) {
                OS_REPORT(OS_ERROR, "v_networkReaderNew", V_RESULT_OUT_OF_MEMORY,
                     "Creation of NetworkReader <%s> failed: cannot create queue cache",
                     name);
                goto err_alloc_cache;
            }
            for( i= 0; i < 2*NW_MAX_QUEUE_CACHE_PRIO; i++) {
                reader->queueCache[i] = NULL;
            }
            c_free(queueType);
            /* Add to subscriber */
            r = v_subscriberAddReader(subscriber,v_reader(reader));
            if (r != V_RESULT_OK) {
                OS_REPORT(OS_ERROR, "v_networkReaderNew", r,
                          "Creation of NetworkReader <%s> failed: cannot add reader to subscriber",
                          name);
                goto err_add_reader;
            }
        } else {
            OS_REPORT(OS_ERROR, "v_networkReaderNew", V_RESULT_OUT_OF_MEMORY,
                "Creation of NetworkReader <%s> failed: cannot create reader QoS",
                name);
            reader = NULL;
        }
    }

    return reader;

err_add_reader:
err_alloc_cache:
err_alloc_queues:
    c_free(queueType);
    c_free(reader);
    return NULL;
}


void
v_networkReaderDeinit(
    v_networkReader reader)
{
    assert(reader != NULL);
    assert(C_TYPECHECK(reader,v_networkReader));
    v_readerDeinit(v_reader(reader));
}

c_ulong
v_networkReaderCreateQueue(
    v_networkReader reader,
    c_ulong queueSize,
    c_ulong priority,
    c_bool reliable,
    c_bool P2P,
    os_duration resolution,
    c_bool useAsDefault,
    const c_char *name)
{
    c_ulong result = 0;
    v_networkQueue queue;
    v_networkReaderStatistics s;
    v_networkQueueStatistics nqs;
    v_networkingStatistics nws;
    v_networkChannelStatistics ncs;
    v_kernel kernel;
    v_networking n;
    v_participant p;
    kernel = v_objectKernel(reader);



    if (reader->nofQueues < NW_MAX_NOF_QUEUES) {
        p = v_participant(v_subscriber(v_reader(reader)->subscriber)->participant);

        if ((v_objectKind(p) == K_NETWORKING) && (v_isEnabledStatistics(kernel, V_STATCAT_NETWORKING))) {
            nqs = v_networkQueueStatisticsNew(kernel,name);
            ncs = v_networkChannelStatisticsNew(kernel,name);
        } else {
            nqs = NULL;
            ncs = NULL;
        }

        queue = v_networkQueueNew(c_getBase((c_object)reader),
                queueSize, priority, reliable, P2P, resolution, nqs);


        if (queue != NULL) {
            reader->queues[reader->nofQueues] = queue;
            reader->nofQueues++;
            result = reader->nofQueues;
            /* insert statistics */
            if (nqs != NULL) {
                s = reader->statistics;
                if (s != NULL) {
                    s->queues[s->queuesCount]=nqs;
                    s->queuesCount++;
                }
            }

            if ((useAsDefault) || (reader->defaultQueue == NULL)){
                c_free(reader->defaultQueue);
                reader->defaultQueue = c_keep(queue);
            }

            if (ncs != NULL) {
                /* add channel to the networking statistics also */
                n = v_networking(p);
                nws = n->statistics;
                nws->channels[nws->channelsCount]=ncs;
                nws->channelsCount++;
            }
        }

    } else {
        OS_REPORT(OS_ERROR, "v_networkReaderCreateQueue", V_RESULT_PRECONDITION_NOT_MET,
            "Maximum number of network queues (%d) exceeded, "
            "new queue not created",
            NW_MAX_NOF_QUEUES);
    }

    return result;
}

static v_networkQueue
v_networkReaderSelectBestQueueByReliability(
    v_networkReader reader,
    v_messageQos qos,
    c_bool requiresP2P,
    const char *partitionName,
    const char *topicName)
{
    unsigned int n;
    v_networkQueue currentQueue;
    v_networkQueue bestQueue = NULL;
    v_networkQueue possiblyBestQueue = NULL;
    v_networkQueue bestAlternativeQueue = NULL;
    c_bool reliabilityMatches = FALSE;
    c_bool P2PMatches = FALSE;
    c_bool reliable;
    c_ulong prio,queuePrio;

    assert(reader != NULL);
    assert(C_TYPECHECK(reader, v_networkReader));
    assert(qos != NULL);
    assert(partitionName != NULL);
    assert(topicName != NULL);

    /* Transform kernel prio to networking prio */
    if (v_messageQos_getTransportPriority(qos) >= 0) {
        prio = (c_ulong)v_messageQos_getTransportPriority(qos);
    } else {
        prio = 0;
    }
    reliable = v_messageQos_isReliable(qos);
    /* First select the best queue */
    if (prio < NW_MAX_QUEUE_CACHE_PRIO) {
        if (reliable) {
            bestQueue = reader->queueCache[prio+NW_MAX_QUEUE_CACHE_PRIO];
        } else {
            bestQueue = reader->queueCache[prio];
        }
    }
    if (!bestQueue) {
        for (n=0; (n<reader->nofQueues) && (bestQueue == NULL); n++) {
            currentQueue = reader->queues[n];
            /* Check on reliability */
            if (reliable) {
                reliabilityMatches = v_networkQueueReliable(currentQueue);
            } else {
                reliabilityMatches = !v_networkQueueReliable(currentQueue);
            }
            P2PMatches = (requiresP2P == v_networkQueueP2P(currentQueue));
            if (reliabilityMatches && P2PMatches) {
                queuePrio = v_networkQueuePriority(currentQueue);
                if (prio == queuePrio) {
                    /* An exact match! Stop here */
                    bestQueue = currentQueue;
                } else {
                    if (prio < queuePrio) {
                        /* This queue might be the best fit, it offers higher prio
                         * than requested
                         */
                        if (possiblyBestQueue != NULL) {
                            if (queuePrio < possiblyBestQueue->priority) {
                                possiblyBestQueue = currentQueue;
                            }
                        } else {
                            possiblyBestQueue = currentQueue;
                        }
                    }
                    if (possiblyBestQueue == NULL) {
                        /* No queue fits until now, but this queue
                         * might be the best alternative if no queue
                         * offers the requested prio at all
                         */
                        if (bestAlternativeQueue != NULL) {
                            if (queuePrio > bestAlternativeQueue->priority) {
                                bestAlternativeQueue  = currentQueue;
                            }
                        } else {
                            bestAlternativeQueue = currentQueue;
                        }
                    }
                }
            }
        }
        if (bestQueue == NULL) {
            bestQueue = possiblyBestQueue;
        }
        if (bestQueue == NULL) {
            bestQueue = bestAlternativeQueue;
        }
        if (bestQueue == NULL) {
            OS_REPORT(OS_WARNING, "v_networkReaderSelectBestQueue", V_RESULT_OK,
                "Unable to select best fitting queue for partition \"%s\", "
                "topic \"%s\". Switching to default", partitionName, topicName);
            bestQueue = reader->defaultQueue;
        }
        if (prio < NW_MAX_QUEUE_CACHE_PRIO) {
            /* Store found bestQueue in the cache, while maintaining
             * correct reference counts on the Queues
             */
            if (reliable) {
                c_free(reader->queueCache[prio+NW_MAX_QUEUE_CACHE_PRIO]);
                reader->queueCache[prio+NW_MAX_QUEUE_CACHE_PRIO] = c_keep(bestQueue);
            } else {
                c_free(reader->queueCache[prio]);
                reader->queueCache[prio] = c_keep(bestQueue);
            }
        }
    }

    return bestQueue;
}

static v_networkQueue
v_networkReaderSelectBestQueueIgnoreReliability(
    v_networkReader reader,
    v_messageQos qos,
    c_bool requiresP2P,
    const char *partitionName,
    const char *topicName)
{
    unsigned int n;
    v_networkQueue currentQueue;
    v_networkQueue bestQueue = NULL;
    v_networkQueue possiblyBestQueue = NULL;
    v_networkQueue bestAlternativeQueue = NULL;
    c_bool P2PMatches = FALSE;
    c_ulong prio,queuePrio;

    assert(reader != NULL);
    assert(C_TYPECHECK(reader, v_networkReader));
    assert(qos != NULL);
    assert(partitionName != NULL);
    assert(topicName != NULL);

    /* Transform kernel prio to networking prio */
    if (v_messageQos_getTransportPriority(qos) >= 0) {
        prio = (c_ulong)v_messageQos_getTransportPriority(qos);
    } else {
        prio = 0;
    }

    /* First select the best queue */
    for (n=0; (n<reader->nofQueues) && (bestQueue == NULL); ++n) {
        currentQueue = reader->queues[n];
        P2PMatches = (requiresP2P == v_networkQueueP2P(currentQueue));
        if (P2PMatches) {
            queuePrio = v_networkQueuePriority(currentQueue);
            if (prio == queuePrio) {
                /* An exact match! Stop here */
                bestQueue = currentQueue;
            } else {
                if (prio < queuePrio) {
                    /* This queue might be the best fit, it offers higher prio
                     * than requested */
                    if (possiblyBestQueue != NULL) {
                        if (queuePrio < possiblyBestQueue->priority) {
                            possiblyBestQueue = currentQueue;
                        }
                    } else {
                        possiblyBestQueue = currentQueue;
                    }
                }
                if (possiblyBestQueue == NULL) {
                    /* No queue fits until now, but this queue
                     * might be the best alternative if no queue
                     * offers the requested prio at all
                     */
                    if (bestAlternativeQueue != NULL) {
                        if (queuePrio > bestAlternativeQueue->priority) {
                            bestAlternativeQueue  = currentQueue;
                        }
                    } else {
                        bestAlternativeQueue = currentQueue;
                    }
                }
            }
        }
    }
    if (bestQueue == NULL) {
        bestQueue = possiblyBestQueue;
    }
    if (bestQueue == NULL) {
        bestQueue = bestAlternativeQueue;
    }
    if (bestQueue == NULL) {
        OS_REPORT(OS_WARNING, "v_networkReaderSelectBestQueue", V_RESULT_OK,
            "Unable to select best fitting queue for partition \"%s\", "
            "topic \"%s\". Switching to default", partitionName, topicName);
        bestQueue = reader->defaultQueue;
    }

    return bestQueue;
}

static v_networkQueue
v_networkReaderSelectBestQueue(
    v_networkReader reader,
    v_messageQos qos,
    c_bool requiresP2P,
    const char *partitionName,
    const char *topicName)
{
  if (reader->ignoreReliabilityQoS) {
    /* this mode is required for DDSi, where a singlee channel is
     * serving best-effort and reliable messages
     */
    return v_networkReaderSelectBestQueueIgnoreReliability
      (reader,
       qos,
       requiresP2P,
       partitionName,
       topicName);
  }
  else {
    /* Note: else-branch is the fast path */
    /* this mode is required for legacy OSPL protocol, change order
     * when DDSi becomes the default protocol
     */
    return v_networkReaderSelectBestQueueByReliability
      (reader,
       qos,
       requiresP2P,
       partitionName,
       topicName);
  }
}

c_bool
v_networkReaderWrite(
    v_networkReader reader,
    v_message message,
    v_networkReaderEntry entry,
    c_ulong sequenceNumber,
    v_gid sender,
    c_bool sendTo, /* for p2p writing */
    v_gid receiver)
{
    c_bool result;
    v_networkQueue bestQueue;

    assert(reader != NULL);
    assert(C_TYPECHECK(reader, v_networkReader));

    /* First select the best queue */
    if (reader->remoteActivity && !(pa_ld32(&v_objectKernel(reader)->isolate) & V_ISOLATE_MUTE)) {
        if (message != NULL) {
            bestQueue = v_networkReaderSelectBestQueue(
                            reader,
                            message->qos,
                            sendTo,
                            v_partitionName(v_group(entry->group)->partition),
                            v_topicName(v_groupTopic(entry->group)));
        } else {
            bestQueue = reader->defaultQueue;
        }
        result = v_networkQueueWrite(bestQueue, message, entry,
                                     sequenceNumber, sender, sendTo, receiver);
    } else {
        result = TRUE;
    }

    return result;
}


v_networkReaderWaitResult
v_networkReaderWait(
    v_networkReader reader,
    c_ulong queueId,
    v_networkQueue *queue)
{
    v_networkReaderWaitResult result = V_WAITRESULT_NONE;
    assert(reader != NULL);
    assert(C_TYPECHECK(reader, v_networkReader));
    assert(queueId <= reader->nofQueues);
    assert(queue != NULL);

    *queue = NULL;
    if (queueId > 0) {
        result = v_networkQueueWait(reader->queues[queueId-1]);
        if (result & V_WAITRESULT_MSGWAITING) {
            *queue = reader->queues[queueId-1];
        }
    } else {
        result = V_WAITRESULT_FAIL;
    }

    return result;
}


v_networkReaderWaitResult
v_networkReaderWaitDelayed(
    v_networkReader reader,
    c_ulong queueId,
    v_networkQueue *queue)
{
    os_duration sleep;

    /* Simply sleeping here for resolution time, is not correct.
     * We should wakeup on, or just after the next wakeuptime.
     */
    sleep = os_timeEDiff(v_networkQueue(reader->queues[queueId-1])->nextWakeup, os_timeEGet());
    v__kernelProtectWaitEnter(NULL, NULL);
    ospl_os_sleep(sleep);
    v__kernelProtectWaitExit();

    return V_WAITRESULT_TIMEOUT | v_networkReaderWait(reader, queueId, queue);
}


void
v_networkReaderTrigger(
    v_networkReader reader,
    c_ulong queueId)
{
    /* Wait for data on default queue, ignore qos for now */
    assert(reader != NULL);
    assert(C_TYPECHECK(reader, v_networkReader));
    assert(queueId <= reader->nofQueues);

    /* First select the best queue */
    if (queueId > 0) {
        v_networkQueueTrigger(reader->queues[queueId-1]);
    }
}


void
v_networkReaderFree(
    v_networkReader reader)
{
    c_bool sendTo, more;
    v_message message;
    c_ulong sequenceNumber, priority;
    v_gid sender, receiver;
    os_timeE sendBefore;
    v_networkReaderEntry entry;
    c_ulong i;

    c_keep(reader);

    /* call inherited free,
     * which will remove the entry from the associated groups
     */
    v_readerFree(v_reader(reader));

    /* remove the messages still pressent in the network queues */
    for (i = 0; i < reader->nofQueues; i++) {
        while (v_networkQueueTakeFirst (
                    v_networkQueue(reader->queues[i]), &message, &entry, &sequenceNumber,
                    &sender, &sendTo, &receiver, &sendBefore, &priority, &more)) {
            c_free(message);
            c_free(entry);
        }
    }

    c_free(reader);
}


/* ------------------------ Subscription/Unsubscription --------------------- */

struct v_findEntryArg {
    v_group group;
    v_networkReaderEntry found;
};

static c_bool
v_networkReaderFindEntry(
    c_object o,
    void * walkArg)
{
    struct v_findEntryArg *findEntryArg;
    v_entry entry;
    c_bool result = TRUE;

    entry = v_entry(o);
    assert(entry);
    assert(walkArg);

    if (v_objectKind(entry) == K_NETWORKREADERENTRY) {
        findEntryArg = (struct v_findEntryArg *)walkArg;
        if (v_networkReaderEntry(entry)->group == findEntryArg->group) {
            result = FALSE;
            findEntryArg->found = v_networkReaderEntry(c_keep(entry));
        }
    }

    return result;
}

v_networkReaderEntry
v_networkReaderLookupEntry(
    v_networkReader reader,
    v_group group)
{
    struct v_findEntryArg findEntryArg;

    assert(v_networkReader(reader) == reader);
    assert(v_group(group) == group);

    findEntryArg.group = c_keep(group);
    findEntryArg.found = NULL;

    v_readerWalkEntries(v_reader(reader), (c_action)v_networkReaderFindEntry, &findEntryArg);

    c_free(group);

    /* Note that the keep has been done in the walk (if any entry was found) */
    return findEntryArg.found;
}

void
v_networkReaderRemoteActivityDetected(
    v_networkReader reader)
{
    assert(reader != NULL);

    reader->remoteActivity = TRUE;
}

void v_networkReaderRemoteActivityLost(
    v_networkReader reader)
{
    assert(reader != NULL);

    reader->remoteActivity = FALSE;
}

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
#include "v__group.h"
#include "v__kernel.h"
#include "v_policy.h"
#include "v__entry.h"
#include "v_partition.h"
#include "v__topic.h"
#include "v_message.h"
#include "v_messageQos.h"
#include "v__entity.h"
#include "v_proxy.h"
#include "v__observable.h"
#include "v_writer.h"
#include "v_reader.h"
#include "v_public.h"
#include "v_instance.h"
#include "v__observer.h"
#include "v__dataReaderInstance.h"
#include "v__dataReaderEntry.h"
#include "v_groupCache.h"
#include "v__groupInstance.h"
#include "v__groupStream.h"
#include "v__leaseManager.h"
#include "v__lifespanAdmin.h"
#include "v__transaction.h"
#include "v__transactionGroup.h"
#include "v_networkReaderEntry.h"
#include "v__builtin.h"
#include "v_kernel.h"
#include "c_time.h"

#include "os_heap.h"
#include "os_report.h"
#include "vortex_os.h"

#define TIME_CONVERTION_REQUIRED

C_CLASS(messageActionArg);
C_STRUCT(messageActionArg) {
    v_writeResult result;
    v_message message;
    c_bool (*condition)(c_voidp instance, c_voidp arg);
    c_voidp arg;      /* optional 2nd parameter for (*condition) */
};

static v_writeResult
groupWrite (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    c_bool stream,
    v_entry entry,
    v_resendScope *resendScope
);

static v_result
groupGetHistoricalData(
    v_group g,
    v_entry e,
    c_bool openTransactions);

v_groupSample
v_groupSampleNew (
    v_group group,
    v_message msg)
{
    v_groupSample sample;

    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));
    assert(C_TIME_GET_KIND(msg->writeTime) == C_TIME_REALTIME);

    sample = v_groupSample(c_new(group->sampleType));
    if (sample) {
        v_groupSampleSetMessage(sample,msg);
        sample->older = NULL;
        sample->newer = NULL;
        sample->transaction = NULL;

        /* Messages that originate from a dispose_all_data event have
         * no inline-qos, but represent just a dispose event. Since
         * events cannot expire (only data can), we set the expiry time
         * to infinite in that case.
         */
        if (!msg->qos || v_messageQos_isInfiniteLifespan(msg->qos)) {
            v_lifespanSample(sample)->expiryTime = OS_TIMEE_INFINITE;
        } else {
            os_duration lifespan;

            lifespan = v_messageQos_getLifespanPeriod(msg->qos);
            v_lifespanSample(sample)->expiryTime = os_timeEAdd(msg->allocTime, lifespan);
        }
    }
    return sample;
}

static c_type
createGroupSampleType(
    v_topic topic)
{
    v_kernel kernel;
    c_type sampleType, baseType, foundType;
    c_metaObject o;
    c_base base;
    c_char *name;
    os_size_t length;
    int sres;
    (void)sres;

    kernel = v_objectKernel(topic);
    base = c_getBase(kernel);
    baseType = c_resolve(base,"kernelModuleI::v_groupSample");
    assert(baseType != NULL);

    sampleType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(sampleType)->extends = c_keep(c_class(baseType));
    o = c_metaDeclare(c_metaObject(sampleType),"message",M_ATTRIBUTE);
    c_property(o)->type = c_keep(v_topicMessageType(topic));
    c_free(o);
    c_metaObject(sampleType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(sampleType));

#define SAMPLE_NAME   "v_groupSample<>"
#define SAMPLE_FORMAT "v_groupSample<%s>"
    /* Create a name and bind type to name */
    /* The sizeof contains \0 */
    length = sizeof(SAMPLE_NAME) + strlen(v_topicName(topic));
    name = os_malloc(length);
    sres = snprintf(name,length,SAMPLE_FORMAT,v_topicName(topic));
    assert(sres >= 0 && (os_size_t) sres == (length-1));
    OS_UNUSED_ARG(sres);
#undef SAMPLE_NAME
#undef SAMPLE_FORMAT

    foundType = c_type(c_metaBind(c_metaObject(base),
                                  name,
                                  c_metaObject(sampleType)));
    os_free(name);
    c_free(sampleType);

    return foundType;
}

static c_type
createGroupInstanceType(
    v_topic topic,
    c_type sampleType)
{
    v_kernel kernel;
    c_type instanceType, tmpType, baseType, foundType;
    c_metaObject o;
    c_base base;
    c_char *name;
    os_size_t length;
    int sres;
    (void)sres;

    kernel = v_objectKernel(topic);
    base = c_getBase(kernel);
    baseType = c_resolve(base,"kernelModuleI::v_groupInstance");
    assert(baseType != NULL);

    /* Due to the fact that our database will normalize attributes in classes,
     * it is not possible to define v_groupSample<type> attribute and key-type
     * attribute in one class. Sometimes the key-type is a reference and
     * sometimes it is not, leading to different ordering in memory of the
     * two attributes some situations. As the v_groupInstanceTemplate is
     * used to get a hold of the typed v_groupSample, it needs to be before the
     * key in memory in all cases. The algorithm below introduces an
     * extra anonymous class in between v_groupInstance and the typed
     * v_groupInstance to ensure memory layout is always the same for any
     * typed v_groupInstance
     */
    /* Declare new class that extends from v_groupInstance. */
    tmpType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(tmpType)->extends = c_class(baseType);

    /* Declare typed v_groupSample attribute in the anonymous class. */
    o = c_metaDeclare(c_metaObject(tmpType),"newest",M_ATTRIBUTE);
    c_property(o)->type = c_keep(sampleType);
    c_free(o);

    /* Finalize the anonymous class. */
    c_metaObject(tmpType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(tmpType));

    /* Declare the typed v_groupInstance and make it extend the anonymous
     * class with the typed v_groupSample.
     */
    instanceType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(instanceType)->extends = c_class(tmpType); /* transfer refCount */

    /* If the topic has a key, add a key attribute to the typed
     * v_groupInstance.
     */
    foundType = v_topicKeyType(topic);

    if ( foundType != NULL) {
        o = c_metaDeclare(c_metaObject(instanceType),"key",M_ATTRIBUTE);
        c_property(o)->type = foundType; /* transfer refcount */
        c_free(o);
    }
    /* Finalize typed v_groupInstance. */
    c_metaObject(instanceType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(instanceType));

#define INSTANCE_NAME "v_groupInstance<v_groupSample<>>"
#define INSTANCE_FORMAT "v_groupInstance<v_groupSample<%s>>"
    /* Create a name and bind type to name */
    /* Ths sizeof contains \0 */
    length = sizeof(INSTANCE_NAME) + strlen(v_topicName(topic));
    name = os_malloc(length);
    sres = snprintf(name,length,INSTANCE_FORMAT,v_topicName(topic));
    assert(sres >= 0 && (os_size_t) sres == (length-1));
    OS_UNUSED_ARG(sres);
#undef INSTANCE_NAME
#undef INSTANCE_FORMAT

    foundType = c_type(c_metaBind(c_metaObject(base),
                                  name,
                                  c_metaObject(instanceType)));
    os_free(name);
    c_free(instanceType);

    return foundType;
}

struct targetUnknownArg {
    v_instance instance;
    v_groupCacheItem cacheItem;
};

static c_bool
targetUnknown(
    v_cacheNode node,
    c_voidp arg)
{
    v_cacheItem item = v_cacheItem(node);
    struct targetUnknownArg *a = (struct targetUnknownArg *)arg;

    if (item->instance == a->instance) {
        a->cacheItem = v_groupCacheItem(item);
    }
    return (a->cacheItem == NULL); /* continue until found */
}

struct doRegisterArg {
    v_groupEntry proxy;
    v_groupInstance instance;
};

static c_bool
doRegister (
    v_registration r,
    c_voidp arg)
{
    v_groupCacheItem item;
    struct targetUnknownArg unknownArg;
    v_message message;
    v_instance instance = NULL;
    struct doRegisterArg *a = (struct doRegisterArg *)arg;

    assert(C_TYPECHECK(r,v_registration));

    message = v_groupInstanceCreateMessage(a->instance);
    if (message)
    {
        message->writerGID = r->writerGID;
        message->qos = c_keep(r->qos);
        message->writeTime = r->writeTime;
        message->sequenceNumber = r->sequenceNumber;
        v_stateSet(v_nodeState(message), L_REGISTER);
        if (v_stateTest(r->state, L_IMPLICIT) == TRUE) {
            v_stateSet(v_nodeState(message), L_IMPLICIT);
        }
        v_entryWrite(a->proxy->entry, message, V_NETWORKID_LOCAL, FALSE, &instance, V_CONTEXT_GROUPWRITE);
        if (instance != NULL) {
            unknownArg.instance = instance;
            unknownArg.cacheItem = NULL;
            (void)v_groupCacheWalk(a->instance->targetCache, targetUnknown, &unknownArg);
            if (unknownArg.cacheItem == NULL) {
                /* Instance is not yet cached.
                 * So insert the instance in the cache.
                 */
                item = v_groupCacheItemNew(a->instance, instance);
                if (item) {
                    v_groupCacheInsert(a->proxy->connectionCache, item);
                    v_groupCacheInsert(a->instance->targetCache, item);
                    c_free(item);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_group::doRegister",V_RESULT_INTERNAL_ERROR,
                              "Failed to register instance");
                }
            } else {
                /* Instance is already cached.
                 * So increase the registration count.
                 */
                unknownArg.cacheItem->registrationCount++;
                assert(unknownArg.cacheItem->registrationCount <= v_dataReaderInstance(instance)->liveliness);
            }
        }
        c_free(instance);
        c_free(message);
    }
    else
    {
        OS_REPORT(OS_CRITICAL,
                  "v_group",V_RESULT_INTERNAL_ERROR,
                  "v_group::doRegister(r=0x%"PA_PRIxADDR", arg=0x%"PA_PRIxADDR")\n"
                  "        Failed to allocate a register message.", (os_address)r, (os_address)arg);
    }
    return TRUE;
}

v_message
v_groupCreateInvalidMessage(
    v_kernel kernel,
    v_gid writerGID,
    c_array writerQos,
    os_timeW timestamp)
{
    /* Lookup the meta-data for the v_message type in the kernel. */
    c_type msgType = v_kernelType(kernel, K_MESSAGE);
    v_message msg = c_new(msgType);

    if (msg)
    {
        /* Set correct Qos, timestamp and WriterGID. */
        msg->qos = c_keep(writerQos);
        msg->writerGID = writerGID;
        msg->writeTime = timestamp;
        msg->allocTime = os_timeEGet();
        v_gidSetNil(msg->writerInstanceGID);
        msg->sequenceNumber = 0;
        msg->transactionId = 0;
    }
    else
    {
        OS_REPORT(OS_CRITICAL,
                  "v_group", V_RESULT_INTERNAL_ERROR,
                  "v_group::CreateInvalidMessage(kernel=0x%"PA_PRIxADDR", v_gid={%d, %d, %d}, writerQos=0x%"PA_PRIxADDR", timestamp={%"PA_PRItime"})\n"
                  "        Operation failed to allocate new v_message: result = NULL.",
                  (os_address)kernel, writerGID.systemId, writerGID.localId, writerGID.serial,
                  (os_address)writerQos, OS_TIMEW_PRINT(timestamp));
        assert(FALSE);
    }

    return msg;
}

static c_bool
registerInstance (
    c_object o,
    c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(o);
    v_groupEntry proxy = (v_groupEntry)arg;
    struct doRegisterArg regArg;

    assert(instance != NULL);
    assert(proxy != NULL);
    assert((v_objectKind(proxy->entry) != K_NETWORKREADERENTRY) ||
           (v_reader(v_entry(proxy->entry)->reader)->qos->userKey.v.enable));

    /* Now register all writer instances by forwarding the writers
     * register messages. Then forward all transient data */
    regArg.proxy = proxy;
    regArg.instance = instance;
    return v_groupInstanceWalkRegistrations(instance, doRegister, &regArg);
}

static c_bool
doUnregister (
    v_cacheNode node,
    c_voidp arg)
{
    v_cacheItem item;
    v_groupInstance groupInst;
    v_registration registration;
    os_timeW *timestamp = (os_timeW *)arg;

    item = v_cacheItem(node);

    assert(v_groupCacheItem(item)->registrationCount <=
           v_dataReaderInstance(item->instance)->liveliness);

    /* Walk over all registrations, and unregister each of them. */
    groupInst = v_groupCacheItem(item)->groupInstance;
    registration = groupInst->registrations;
    while (registration)
    {
        v_dataReaderInstanceUnregister(v_dataReaderInstance(item->instance),
                                       registration,
                                       *timestamp);
        registration = registration->next;
    }
    return TRUE;
}


static c_bool
instanceFree (
    c_object o,
    c_voidp arg)
{
    OS_UNUSED_ARG(arg);
    v_groupInstanceFree(v_groupInstance(o));
    return TRUE;
}

static v_groupAction
v_groupActionNew(
    v_groupActionKind kind,
    os_timeE timestamp,
    v_message message,
    v_group group)
{
    v_groupAction action;
    v_kernel kernel;

    assert(group != NULL);
    assert(C_TYPECHECK(group, v_group));

    action = NULL;
    kernel = v_objectKernel(group);
    action = v_groupAction(v_objectNew(kernel,K_GROUPACTION));

    action->kind        = kind;
    action->actionTime  = timestamp;
    action->group       = c_keep(group);

    if(message){
        action->message = c_keep(message);
    } else {
        action->message = NULL;
    }
    return action;
}

struct streamArgs {
    v_groupAction action;
    v_writeResult result;
};

static c_bool
writeToStream(
    v_groupStream stream,
    c_voidp args)
{
    v_writeResult result;
    struct streamArgs* sa;

    assert(stream != NULL);
    assert(C_TYPECHECK(stream, v_groupStream));

    sa = (struct streamArgs*)args;
    result = v_groupStreamWrite(stream, sa->action);

    if(sa->result == V_WRITE_SUCCESS){
        sa->result = result;
    }
    return TRUE;
}


/**
 * This function forwards a message to all available streams.
 * In case the message originates from a call to dispose_all_data, it has no keys
 * so in that case we extract its identity from the groupInstance which should have
 * been passed by the caller in those circumstances.
 * It is allowed not to pass the groupInstance (e.g. leave it NULL), but in that case
 * the message should contain at least the value of its keys.
 */
v_writeResult
forwardMessageToStreams(
    v_group group,
    v_groupInstance instance,
    v_message message,
    os_timeE t,
    v_groupActionKind actionKind)
{
    struct streamArgs args;

    assert(group != NULL);
    assert(C_TYPECHECK(group, v_group));

    args.result = V_WRITE_SUCCESS;
    if (c_count(group->streams) > 0) {
        if (instance && c_getType(message) == v_kernelType(v_objectKernel(instance), K_MESSAGE)) {
            /* Temporarily transform the untyped message without keys into a full-blown typed
             * message with keys, so that the message will describe its own identity.
             */
            v_message typedMessage = v_groupInstanceCreateTypedInvalidMessage(instance, message);
            args.action = v_groupActionNew(actionKind, t, typedMessage, group);
            c_free(typedMessage);
        } else {
            assert(c_getType(message) != v_kernelType(v_objectKernel(group), K_MESSAGE));
            args.action = v_groupActionNew(actionKind, t, message, group);
        }
        c_setWalk(group->streams, (c_action)writeToStream, &args);
        c_free(args.action);
    }
    return args.result;
}

struct lifespanExpiry {
    os_timeE t;
    v_group group;
};

static c_bool
onSampleExpired(
    v_lifespanSample sample,
    c_voidp arg)
{
    struct lifespanExpiry* le = (struct lifespanExpiry*)arg;
    v_groupSample groupSample = v_groupSample(sample);
    v_groupInstance instance = v_groupInstance(groupSample->instance);

    forwardMessageToStreams(le->group, instance, v_groupSampleMessage(sample), le->t, V_GROUP_ACTION_LIFESPAN_EXPIRE);
    v_groupInstanceRemove(groupSample);

    return TRUE;
}

#define v_groupPurgeItem(_this) (C_CAST(_this,v_groupPurgeItem))

void
_dispose_purgeList_insert(
    v_groupInstance instance,
    os_timeE insertTime)
{
    v_groupPurgeItem purgeItem;
    v_group group;

    assert(instance);
    assert(C_TYPECHECK(instance,v_groupInstance));

    group = v_group(instance->group);
    assert(group);

    v_groupInstanceDisconnect(instance);
    v_groupInstanceSetEpoch(instance, insertTime);

    purgeItem = c_new(v_kernelType(v_objectKernel(group),
                      K_GROUPPURGEITEM));
    if (purgeItem) {
        purgeItem->instance = c_keep(instance);
        purgeItem->insertionTime = insertTime;

        purgeItem->next = NULL;
        if (group->disposedInstancesLast) {
            v_groupPurgeItem(group->disposedInstancesLast)->next = purgeItem;
        } else {
            assert(group->disposedInstances == NULL);
            group->disposedInstances = purgeItem;
        }
        group->disposedInstancesLast = purgeItem;
    } else {
        OS_REPORT(OS_FATAL,
                  "v_group::_dispose_purgeList_insert",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate purgeItem");
        assert(FALSE);
    }
}

static v_groupInstance
_dispose_purgeList_take(
    v_group group,
    os_timeE timestamp)
{
    v_groupPurgeItem purgeItem;
    os_compare equality;
    v_groupInstance instance = NULL;

    if (group) {
        purgeItem = group->disposedInstances;
        while (purgeItem) {
            /* This implementation assumes that the list is timely ordered.
             * The assumption is that the time when a purgeItem is inserted
             * by the operation _dispose_purgeList_insert is reflected by
             * the value of purgeItem->insertionTime.
             */
            equality = os_timeECompare(purgeItem->insertionTime, timestamp);
            if (equality == OS_LESS) {
                group->disposedInstances = purgeItem->next;
                purgeItem->next = NULL;
                if (group->disposedInstances == NULL) {
                    assert(group->disposedInstancesLast == purgeItem);
                    group->disposedInstancesLast = NULL;
                }
                instance = purgeItem->instance; /* Transfer ref when returning instance. */
                /* The existence of an instance in the dispose purge list is
                 * only valid if the purgeItem insertion time equals the instance
                 * epoch time. If these timestamps are not equal then the instance
                 * has been updated and should no longer be in this purge list.
                 * for performance the instance is not removed from the list but
                 * instead needs to be ignored and afterall removed at this point.
                 */
                equality = os_timeECompare(purgeItem->insertionTime, instance->epoch);
                if (equality == OS_EQUAL) {
                    assert(v_groupInstanceStateTest(instance, L_DISPOSED));
                    v_groupInstancePurge(instance);
                    purgeItem->instance = NULL; /* Return instance, so prevent purgeItem from releasing it. */
                    c_free(purgeItem);
                    purgeItem = NULL;
                } else {
                    c_free(purgeItem);
                    purgeItem = group->disposedInstances;
                    instance = NULL;
                }
            } else {
                instance = NULL;
                purgeItem = NULL;
            }
        }
    } else {
        assert(0);
        instance = NULL;
    }
    return instance;
}

void
_empty_purgeList_insert(
    v_groupInstance instance,
    os_timeE insertTime)
{
    v_groupPurgeItem purgeItem;
    v_group group;

    assert(instance);
    assert(C_TYPECHECK(instance,v_groupInstance));
    assert(v_groupInstanceStateTest(instance,L_EMPTY));

    group = v_group(instance->group);
    assert(group);

    v_groupInstanceDisconnect(instance);
    v_groupInstanceSetEpoch(instance, insertTime);

    purgeItem = c_new(v_kernelType(v_objectKernel(group),
                      K_GROUPPURGEITEM));
    if (purgeItem) {
        purgeItem->instance = c_keep(instance);
        purgeItem->insertionTime = insertTime;
        purgeItem->next = NULL;
        if (group->purgeListEmptyLast) {
            v_groupPurgeItem(group->purgeListEmptyLast)->next = purgeItem;
            group->purgeListEmptyLast = purgeItem;
        } else {
            assert(group->purgeListEmpty == NULL);
            group->purgeListEmpty = purgeItem;
            group->purgeListEmptyLast = purgeItem;
        }
    } else {
        OS_REPORT(OS_FATAL,
                  "v_group::_empty_purgeList_insert",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate purgeItem");
        assert(FALSE);
    }
}

static v_groupInstance
_empty_purge_take(
    v_group group,
    os_timeE purgeBefore)
{
    v_groupPurgeItem purgeItem;
    v_groupInstance removed;
    v_groupInstance instance = NULL;
    os_compare equality;

    if (group) {
        purgeItem = group->purgeListEmpty;
        while (purgeItem) {
            /* This implementation assumes that the list is timely ordered.
             * The assumption is that the time when a purgeItem is inserted
             * by the operation _empty_purgeList_insert is reflected by
             * the value of purgeItem->insertionTime.
             */
            equality = os_timeECompare(purgeItem->insertionTime, purgeBefore);
            if (equality == OS_LESS) {
                group->purgeListEmpty = purgeItem->next;
                purgeItem->next = NULL;
                if (group->purgeListEmptyLast == purgeItem) {
                    assert(group->purgeListEmpty == NULL);
                    group->purgeListEmptyLast = NULL;
                }
                instance = purgeItem->instance; /* Transfer ref when returning instance. */
                /* The existance of an instance in the empty-nowriter purge list is
                 * only valid if the purgeItem insertion time equals the instance
                 * epoch time. If these timestamps are not equal then the instance
                 * has been updated and should no longer be in this purge list.
                 * for performance the instance is not removed from the list but
                 * instead needs to be ignored and afterall removed at this point.
                 */
                equality = os_timeECompare(purgeItem->insertionTime, instance->epoch);
                if (equality == OS_EQUAL) {
                    removed = c_remove(group->instances,instance,NULL,NULL);
                    /* It is allowed that the instance is already taken and
                     * no longer exists in the group->instances set.
                     */
                    if (removed) {
                        v_groupInstanceFree(removed);
                    }
                    purgeItem->instance = NULL; /* Return instance, so prevent purgeItem from releasing it. */
                    c_free(purgeItem);
                    purgeItem = NULL;
                } else {
                    c_free(purgeItem); /* Instance not returned, so purgeItem may recursively release it. */
                    purgeItem = group->purgeListEmpty;
                    instance = NULL;
                }
            } else {
                instance = NULL;
                purgeItem = NULL;
            }
        }
    } else {
        assert(0);
        instance = NULL;
    }
    return instance;
}


static void
updatePurgeList(
    v_group group,
    os_timeE now /* elapsed-clock */)
{
    const os_timeE currentTime = os_timeEGet();
    os_timeE purgeBefore; /* Data with insertionTime < purgeBefore must be purged */
    os_duration delay;
    v_groupInstance instance;
    struct lifespanExpiry exp;
    v_message message;
    v_kernel kernel;

    kernel = v_objectKernel(group);

    /* Purge all instances that are expired. */
    /* Precondition is that the instances in the purge list are not alive
     * and empty */
    exp.t     = currentTime;
    exp.group = group;

    v_lifespanAdminTakeExpired(group->lifespanAdmin, currentTime, onSampleExpired, &exp);

    if (group->purgeListEmpty) {
        delay = kernel->retentionPeriod;
        purgeBefore = os_timeESub(now, delay);
        /* purgeBefore can be invalid when retentionPeriod is large and
         * current elapsed time is small (node was just started). */
        if (!OS_TIMEE_ISINVALID(purgeBefore)) {
            while ((instance = _empty_purge_take(group, purgeBefore)) != NULL) {
                v_groupInstanceFree(instance);
            }
        }
    }

    if (group->disposedInstances) {
        delay = v_topicQosRef(group->topic)->durabilityService.v.service_cleanup_delay;
        purgeBefore = os_timeESub(now, delay);
        /* purgeBefore can be invalid when service_cleanup_delay is large and
         * current elapsed time is small (node was just started). */
        if (!OS_TIMEE_ISINVALID(purgeBefore)) {
            while ((instance = _dispose_purgeList_take(group, purgeBefore)) != NULL) {
                if (v_stateTest(instance->state, L_EMPTY | L_NOWRITERS)) {
                    message = v_groupInstanceCreateMessage(instance);
                    message->writeTime = os_timeWGet();
                    forwardMessageToStreams(group,
                                            NULL,
                                            message,
                                            purgeBefore,
                                            V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE);
                    c_free(message);
                    _empty_purgeList_insert(instance, now);

                }
                v_groupInstanceFree(instance);
            }
        }
    }

    if (group->transactionAdmin) {
        v_transactionGroupAdmin tgroupadm;
        v_transactionAdminPurgeHistory(group->transactionAdmin);

        tgroupadm = v_transactionGetGroupAdmin(group->transactionAdmin);
        if (tgroupadm) {
            v_transactionGroupAdminPurgeHistory(tgroupadm);
        }
    }
}
#undef MAX_PURGECOUNT

#define v_groupEntryEmpty(set) (set.firstEntry == NULL)

static void
v_groupEntrySetInit (
    struct v_groupEntrySet *set)
{
    set->lastSequenceNumber = 0;
    set->firstEntry = NULL;
}

static v_groupEntry
v_groupEntrySetAdd (
    struct v_groupEntrySet *set,
    v_entry e)
{
    c_type type;
    v_groupEntry proxy;

    type = c_resolve(c_getBase(e),"kernelModuleI::v_groupEntry");
    proxy = c_new(type);
    c_free(type);
    if (proxy) {
        proxy->connectionCache = v_groupCacheNew(v_objectKernel(e),V_CACHE_CONNECTION);
        if (proxy->connectionCache) {
            proxy->entry = c_keep(e);
            proxy->sequenceNumber = set->lastSequenceNumber;
            proxy->next = set->firstEntry;
            set->firstEntry = proxy; /* Transfer refCount */
            set->lastSequenceNumber++;
        } else {
            c_free(proxy);
            proxy = NULL;
            OS_REPORT(OS_ERROR,
                      "v_groupEntrySetAdd",V_RESULT_INTERNAL_ERROR,
                      "Failed to allocate reader instance cache.");
        }
    } else {
        OS_REPORT(OS_FATAL,
                  "v_groupEntrySetAdd",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate reader proxy.");
        assert(FALSE);
    }
    return c_keep(proxy);
}

static v_groupEntry
v_groupEntrySetRemove (
    struct v_groupEntrySet *set,
    v_entry e)
{
    v_groupEntry *proxy, foundProxy;

    foundProxy = NULL;
    proxy = &set->firstEntry;
    while (((*proxy) != NULL) && ((*proxy)->entry != e)) {
        proxy = &(*proxy)->next;
    }
    if ((*proxy) != NULL) {
        foundProxy = *proxy;
        *proxy = (*proxy)->next;
        foundProxy->next = NULL;
    }
    return foundProxy;
}

static void
v_groupEntryFree(
    v_groupEntry entry)
{
    if (entry != NULL) {
        v_groupCacheDeinit(entry->connectionCache);
        c_free(entry);
    }
}

#define v_groupEntrySetEmpty(s) ((s)->firstEntry == NULL)

static c_bool
entrySetDisposeAction(v_groupEntry entry, c_voidp arg)
{
   v_dataReaderEntry reader;
   messageActionArg a = (messageActionArg) arg;

   reader = v_dataReaderEntry(entry->entry);
   a->result = v_dataReaderEntryDisposeAll(reader, a->message, a->condition, a->arg);
   return( a->result == V_WRITE_SUCCESS );
}

static c_bool
entrySetUnmarkReadersInstanceStatesAction(v_groupEntry entry, c_voidp arg)
{
    v_dataReaderEntry reader;
    c_ulong flags = *((c_ulong *)arg);

    reader = v_dataReaderEntry(entry->entry);
    v_dataReaderEntryUnmarkInstanceStates (reader, flags);
    return TRUE;
}

c_bool
v_groupEntrySetWalk(
    struct v_groupEntrySet *s,
    v_groupEntrySetWalkAction action,
    c_voidp arg)
{
    v_groupEntry proxy;
    c_bool proceed = TRUE;

    proxy = s->firstEntry;
    while ((proceed) && (proxy != NULL)) {
        proceed = action(proxy,arg);
        proxy = proxy->next;
    }
    return proceed;
}

/* Reset the specified flags for the instanceStates of the
 * specified DataReader entries.
 */
static void
v_groupEntrySetUnmarkReaderInstanceStates( struct v_groupEntrySet *set, c_ulong flags )
{
   (void)v_groupEntrySetWalk( set, entrySetUnmarkReadersInstanceStatesAction, &flags );
}

static c_bool
v_groupEntrySetDisposeAll( struct v_groupEntrySet *set, c_voidp arg )
{
   return( v_groupEntrySetWalk( set, entrySetDisposeAction, arg ) );
}

static c_char *
createInstanceKeyExpr (
    v_topic topic)
{
    c_char fieldName[16];
    c_char *keyExpr;
    c_size i,nrOfKeys,totalSize;
    c_array keyList;

    assert(C_TYPECHECK(topic,v_topic));
    keyList = c_keep(v_topicMessageKeyList(topic));
    nrOfKeys = c_arraySize(keyList);
    if (nrOfKeys>0) {
        totalSize = nrOfKeys * strlen("key.field0,");
        if (nrOfKeys > 9) {
            totalSize += (nrOfKeys-9);
            if (nrOfKeys > 99) {
                totalSize += (nrOfKeys-99);
            }
        }
        keyExpr = (char *)os_malloc(totalSize);
        keyExpr[0] = 0;
        for (i=0;i<nrOfKeys;i++) {
            os_sprintf(fieldName,"key.field%d",i);
            os_strcat(keyExpr,fieldName);
            if (i<(nrOfKeys-1)) { os_strcat(keyExpr,","); }
        }
    } else {
        keyExpr = NULL;
    }
    return keyExpr;
}

static void
v_groupInit(
    v_group group,
    v_partition partition,
    v_topic topic,
    c_long id)
{
    char *groupName;
    os_size_t groupNameLen;
    v_kernel kernel;
    v_topicQos qos;
    c_bool infWait;
    c_char *keyExpr;
    c_base base;
    c_type type;
    v_partitionPolicyI partqos;

    kernel = v_objectKernel(group);
    base  = c_getBase(kernel);

    /* First initialize baseclass */
    v_observableInit(v_observable(group));

    /* Then initialize class itself */
    groupNameLen = sizeof(V_GROUP_NAME_TEMPLATE) - sizeof("%s%s") +
                   1 /* \0 */ + strlen(v_partitionName(partition)) +
                   strlen(v_topicName(topic));
    groupName = os_malloc(groupNameLen);
    snprintf(groupName, groupNameLen, V_GROUP_NAME_TEMPLATE,
             v_partitionName(partition), v_topicName(topic));
    group->name = c_stringNew(base, groupName);
    os_free(groupName);

    group->lastDisposeAllTime = OS_TIMEW_ZERO;
    group->partition = c_keep(partition);
    group->topic = c_keep(topic);
    group->sequenceNumber = id;
    group->routingEnabled = FALSE;

    group->lifespanAdmin = v_lifespanAdminNew(kernel);

    group->purgeListEmpty = NULL;
    group->purgeListEmptyLast = NULL;

    v_groupEntrySetInit(&group->topicEntrySet);
    v_groupEntrySetInit(&group->networkEntrySet);
    v_groupEntrySetInit(&group->routedEntrySet);
    v_groupEntrySetInit(&group->variantEntrySet);

    type  = c_resolve(base,"kernelModuleI::v_groupStream");
    group->streams = c_setNew(type);

    type  = c_resolve(base, "c_string");
    assert(type);
    group->attachedServices      = c_setNew(type);
    group->notInterestedServices = c_setNew(type);

    qos = v_topicQosRef(topic);
    infWait = OS_DURATION_ISINFINITE(qos->reliability.v.max_blocking_time);

    group->sampleType = createGroupSampleType(topic);
    group->instanceType = createGroupInstanceType(topic, group->sampleType);

    keyExpr = createInstanceKeyExpr(topic);
    group->instances = c_tableNew(group->instanceType,keyExpr);
    os_free(keyExpr);

    group->resourceSampleCount = 0;
    group->infWait = infWait;
    group->disposedInstances = NULL;
    group->disposedInstancesLast = NULL;
    if(qos->durability.v.kind == V_DURABILITY_VOLATILE) {
        group->complete = TRUE;     /* no alignment necessary.*/
    } else {
        group->complete = FALSE;    /* until aligned */
    }
    c_mutexInit(c_getBase(group), &group->mutex);

    group->cachedInstance = NULL;
    group->cachedRegMsg = NULL;
    /* ES, dds1576: For each new group, determine the access mode for the
     * partition involved in this group.
     */
    partqos.v = v_partitionName (partition);
    group->partitionAccessMode = v_kernelPartitionAccessMode(kernel, partqos);

    type  = c_resolve(base,"kernelModuleI::v_groupwriterAdministration");
    group->writerAdministration = c_tableNew(type, "gid.systemId,gid.localId,gid.serial");
    /* The transactionAdmin is created lazy when required. */
    group->transactionAdmin = NULL;
    group->onRequest = FALSE;
    group->pristine = TRUE; /* becomes false as soon a writer connects. */
    c_free(type);
}

v_group
v_groupNew(
    v_partition partition,
    v_topic topic,
    c_long id)
{
    v_kernel kernel;
    v_group group;

    assert(partition != NULL);
    assert(C_TYPECHECK(partition,v_partition));
    assert(topic != NULL);
    assert(C_TYPECHECK(topic,v_topic));

    kernel = v_objectKernel(topic);
    group = v_group(v_objectNew(kernel,K_GROUP));
    v_groupInit(group, partition, topic, id);

    return group;
}

void
v_groupFree(
    v_group group)
{
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    v_observerFree(v_observer(group));
}

void
v_groupDeinit(
    v_group group)
{
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    c_tableWalk(group->instances,instanceFree,NULL);
}

struct findServiceHelper {
    const c_char* search;
    c_bool found;
    c_string serviceName;
};

static c_bool
findService(
    c_object name,
    c_voidp args)
{
    struct findServiceHelper* helper;

    helper = (struct findServiceHelper*)args;

    if(os_strcasecmp((c_string)name, helper->search) == 0){
        helper->found = TRUE;
        helper->serviceName = (c_string)name;
    }
    return !(helper->found);
}

/* remove the service from either the attachedServices or notInterestedServices list */
void
v_groupRemoveAwareness (
    v_group _this,
    const c_char* serviceName)
{
    struct findServiceHelper helper;
    c_object remove;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_group));
    assert(serviceName);

    if(_this && serviceName){
        helper.found  = FALSE;
        helper.search = serviceName;
        helper.serviceName = NULL;

        c_mutexLock(&_this->mutex);
        (void)c_walk(_this->attachedServices, findService, &helper);

        if(helper.found){
            remove = c_remove(_this->attachedServices,helper.serviceName,NULL,NULL);
            assert(remove == helper.serviceName);
            c_free(remove);
            helper.serviceName = NULL;
        } else {
            (void)c_walk(_this->notInterestedServices, findService, &helper);

            if(helper.found){
                remove = c_remove(_this->notInterestedServices,helper.serviceName,NULL,NULL);
                assert(remove == helper.serviceName);
                c_free(remove);
                helper.serviceName = NULL;
            }
        }
        c_mutexUnlock(&_this->mutex);
    }
}

void
v_groupNotifyAwareness (
    v_group _this,
    const c_char* serviceName,
    c_bool interested)
{
    c_string name, found;
    c_base base;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_group));
    assert(serviceName);

    if(_this && serviceName){
        base = c_getBase(_this);
        name = c_stringNew(base, serviceName);

        c_mutexLock(&_this->mutex);
        if(interested){
            found = ospl_c_insert(_this->attachedServices, name);
        } else {
            found = ospl_c_insert(_this->notInterestedServices, name);
        }
        c_mutexUnlock(&_this->mutex);

        if(found){
            c_free(name);
        }
    }
    return;
}

v_groupAttachState
v_groupServiceGetAttachState (
    v_group _this,
    const c_char* serviceName)
{
    v_groupAttachState state;
    struct findServiceHelper helper;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_group));
    assert(serviceName);

    if(_this && serviceName){
        helper.found  = FALSE;
        helper.search = serviceName;
        helper.serviceName = NULL;

        c_mutexLock(&_this->mutex);
        (void)c_walk(_this->attachedServices, findService, &helper);

        if(helper.found){
            state = V_GROUP_ATTACH_STATE_ATTACHED;
        } else {
            (void)c_walk(_this->notInterestedServices, findService, &helper);

            if(helper.found){
                state = V_GROUP_ATTACH_STATE_NO_INTEREST;
            } else {
                state = V_GROUP_ATTACH_STATE_UNKNOWN;
            }
        }
        c_mutexUnlock(&_this->mutex);
    } else {
        state = V_GROUP_ATTACH_STATE_UNKNOWN;
    }
    return state;
}

void
v_groupAddEntry(
    v_group g,
    v_entry e)
{
    v_groupEntry proxy;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entry));

    c_mutexLock(&g->mutex);

    updatePurgeList(g, os_timeEGet());
    /* doubly linked connection */
    if (v_entryAddGroup(e, g))
    {
       if (v_objectKind(e) == K_NETWORKREADERENTRY) {
          c_free(v_groupEntrySetAdd(&g->networkEntrySet,e));
          if(v_networkReaderEntryIsRouting(v_networkReaderEntry(e))){
              c_free(v_groupEntrySetAdd(&g->routedEntrySet,e));
          }
       } else if (v_reader(v_entry(e)->reader)->qos->userKey.v.enable) {
           proxy = v_groupEntrySetAdd(&g->variantEntrySet,e);
           if (proxy) {
               c_free(proxy);
               if (v_reader(v_entry(e)->reader)->qos->durability.v.kind != V_DURABILITY_VOLATILE) {
                   (void)groupGetHistoricalData(g,e,TRUE);
               }
           }
       } else {
           proxy = v_groupEntrySetAdd(&g->topicEntrySet,e);
           if (proxy) {
               c_tableWalk(g->instances,registerInstance,proxy);
               c_free(proxy);
               if (v_reader(v_entry(e)->reader)->qos->durability.v.kind != V_DURABILITY_VOLATILE) {
                   (void)groupGetHistoricalData(g,e,TRUE);
               }
           } else {
               OS_REPORT(OS_ERROR,
                         "v_groupAddEntry",V_RESULT_INTERNAL_ERROR,
                         " Failed to register instance in topicEntrySet");
           }
       }
    }
    c_mutexUnlock(&g->mutex);
}

void
v_groupRemoveEntry(
    v_group g,
    v_entry e)
{
    v_groupEntry proxy;
    os_timeW timestamp;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entry));

    c_mutexLock(&g->mutex);

    if (v_objectKind(e) == K_NETWORKREADERENTRY) {
        proxy = v_groupEntrySetRemove(&g->networkEntrySet,e);
        if(v_networkReaderEntryIsRouting(v_networkReaderEntry(e))){
            v_groupEntry routedProxy;
            routedProxy = v_groupEntrySetRemove(&g->routedEntrySet,e);
            v_groupEntryFree(routedProxy);
        }
    } else if (v_reader(v_entry(e)->reader)->qos->userKey.v.enable) {
        proxy = v_groupEntrySetRemove(&g->variantEntrySet,e);
    } else {
        proxy = v_groupEntrySetRemove(&g->topicEntrySet,e);
        if ((proxy != NULL) && !(v_observerEventMask(v_entry(e)->reader) & V_EVENT_PREPARE_DELETE)) {
            timestamp = os_timeWGet();
            (void)v_groupCacheWalk(proxy->connectionCache, doUnregister, &timestamp);
        }
    }
    v_groupEntryFree(proxy);
    v_entryRemoveGroup(e, g);
    c_mutexUnlock(&g->mutex);
}

v_entry
v_groupLookupEntry(
    v_group g,
    v_reader r)
{
    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(r != NULL);
    assert(C_TYPECHECK(r,v_reader));

    OS_UNUSED_ARG(g);
    OS_UNUSED_ARG(r);

    assert(FALSE); /* To be implemented. */
    return NULL;
}

c_bool
v_groupAddStream(
    v_group group,
    v_groupStream stream)
{
    v_groupStream found;
    c_bool result;

    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));
    assert(stream != NULL);
    assert(C_TYPECHECK(stream,v_groupStream));

    result = FALSE;

    c_mutexLock(&group->mutex);
    found = v_groupStream(c_setInsert(group->streams,stream));

    if (found == stream) {
        result = TRUE;
    }
    c_mutexUnlock(&group->mutex);

    return result;
}

c_bool
v_groupRemoveStream(
    v_group group,
    v_groupStream stream)
{
    c_bool result;
    v_groupStream found;

    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));
    assert(stream != NULL);
    assert(C_TYPECHECK(stream,v_groupStream));

    result = FALSE;
    c_mutexLock(&group->mutex);
    found = v_groupStream(c_remove(group->streams,stream, NULL, NULL));

    if (found == stream) {
        result = TRUE;
    }
    c_mutexUnlock(&group->mutex);

    return result;
}

c_bool
v_groupSetRoutingEnabled(
     v_group group,
     c_bool routingEnabled)
{
    c_bool result;

    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    c_mutexLock(&group->mutex);
    result = group->routingEnabled;
    group->routingEnabled = routingEnabled;
    c_mutexUnlock(&group->mutex);

    return result;
}

C_STRUCT(v_groupFlushArg) {
    c_voidp arg;
    v_group group;
    v_groupFlushCallback action;
    v_entry entry;
    v_groupInstance grInst;
    v_writeResult writeResult;
};
C_CLASS(v_groupFlushArg);

static c_bool
doFlush(
    v_groupSample sample,
    c_voidp arg)
{
    v_groupFlushArg groupFlushArg;
    v_entry         entry;
    v_message       message;
    c_bool          propagateTheMessage;

    assert(C_TYPECHECK(sample,v_groupSample));
    assert(sample != NULL);
    assert(arg != NULL);

    groupFlushArg = (v_groupFlushArg)arg;
    entry = groupFlushArg->entry;
    message = v_groupSampleMessage(sample);
    assert(message);

    assert(v_messageStateTest(message,L_WRITE) ||
           v_messageStateTest(message,L_DISPOSED));

    /* perform action and/or process the message */
    if (groupFlushArg->action == NULL) {
        propagateTheMessage = TRUE;
    } else {
        propagateTheMessage = groupFlushArg->action(message,
                v_groupInstance(sample->instance), V_GROUP_FLUSH_MESSAGE,
                groupFlushArg->arg);
    }
    if (propagateTheMessage) {
        assert(entry != NULL);
        /* write the sample to other node(s) */
        v_entryWrite(entry, message, V_NETWORKID_LOCAL, FALSE, NULL, V_CONTEXT_GETHISTORY);
    }
    return TRUE;
}

static c_bool
doUnregisterFlush(
    v_registration unregister,
    c_voidp arg)
{
    v_groupFlushArg groupFlushArg;
    v_entry         entry;
    v_groupInstance grInst;
    v_message       message;
    c_bool          propagateTheMessage;

    assert(C_TYPECHECK(unregister,v_registration));

    groupFlushArg = (v_groupFlushArg)arg;

    assert(unregister != NULL);
    assert(groupFlushArg != NULL);

    entry = groupFlushArg->entry; /* Note: entry may legally be NULL. */

    grInst = groupFlushArg->grInst;
    assert(grInst);


    /* perform action and/or process the message */
    if (groupFlushArg->action == NULL)
    {
        propagateTheMessage = TRUE;
    }
    else
    {
        propagateTheMessage = groupFlushArg->action(unregister, grInst,
                V_GROUP_FLUSH_UNREGISTRATION, groupFlushArg->arg);
    }
    if (entry && propagateTheMessage)
    {
        message = v_groupInstanceCreateMessage(grInst);

        if (message)
        {
            message->writerGID = unregister->writerGID;
            message->qos = c_keep(unregister->qos);
            message->writeTime = unregister->writeTime;
            message->sequenceNumber = unregister->sequenceNumber;
            if (v_stateTest(unregister->state, L_IMPLICIT) == TRUE) {
                v_stateSet(v_nodeState(message), L_IMPLICIT);
            }

            /* Set the nodeState of the message to UNREGISTER. */
            v_stateSet(v_nodeState(message), L_UNREGISTER);
            /* write the sample to other node(s) */
            v_entryWrite(entry, message, V_NETWORKID_LOCAL, FALSE, NULL, V_CONTEXT_GETHISTORY);
            c_free(message);
        }
        else
        {
            OS_REPORT(OS_ERROR,
                      "v_group", V_RESULT_INTERNAL_ERROR,
                      "v_group::doUnregisterFlush(unregister=0x%"PA_PRIxADDR", arg=0x%"PA_PRIxADDR")\n"
                      "        Failed to allocate an unregister message.",
                      (os_address)unregister, (os_address)arg);
        }
    }
    return TRUE;
}

static c_bool
findWriter(
    v_groupSample o,
    c_voidp arg)
{
    v_gid *gidArg = arg;
    v_message message = v_groupSampleMessage(o);

    if(v_gidCompare(message->writerGID, *gidArg) != C_EQ) {
        /* If the gid does not match the writer we're looking for, keep searching. */
        return TRUE;
    }

    /* Writer is found, stop walk. */
    return FALSE;
}

static c_bool
v_groupInstanceWriterSampleNotExists(
    v_groupInstance grInst,
    v_gid writerGid)
{
    c_bool notExist = FALSE;
    v_group group;

    group = v_group(grInst->group);

    notExist = v_groupInstanceWalkSamples(grInst, findWriter, &writerGid);
    if (group->transactionAdmin) {
        if (notExist) {
            notExist = v_transactionAdminNoMessageFromWriterExist(group->transactionAdmin, grInst, writerGid);
        }
        if (notExist) {
            v_transactionGroupAdmin tgroupadm = v_transactionGetGroupAdmin(group->transactionAdmin);
            if (tgroupadm) {
                notExist = v_transactionGroupAdminNoMessageFromWriterExist(tgroupadm, grInst, writerGid);
            }
        }
    }

    return notExist;
}

static void
flushInstanceRegistrations(
    v_groupInstance grInst,
    v_groupFlushArg arg)
{
    v_registration  registration;
    v_entry entry;

    entry = arg->entry;
    registration = grInst->registrations;

    while(registration) {
        /* Check if writerGID occurs in one of the samples */
        if (v_groupInstanceWriterSampleNotExists(grInst, registration->writerGID)) {
            c_bool propagateTheMessage = FALSE;

            if (arg->action == NULL) {
                propagateTheMessage = TRUE;
            } else {
                propagateTheMessage = arg->action(registration,
                        grInst, V_GROUP_FLUSH_REGISTRATION,
                        arg->arg);
            }
            if(entry && propagateTheMessage) {
                v_message message = v_groupInstanceCreateMessage(grInst);

                if (message)
                {
                    message->writerGID = registration->writerGID;
                    message->qos = c_keep(registration->qos);
                    message->writeTime = registration->writeTime;
                    message->sequenceNumber = registration->sequenceNumber;

                    /* Set the nodeState of the message to REGISTER. */
                    v_stateSet(v_nodeState(message), L_REGISTER);
                    if (v_stateTest(registration->state, L_IMPLICIT) == TRUE) {
                        v_stateSet(v_nodeState(message), L_IMPLICIT);
                    }
                    /* write the sample to other node(s) */
                    v_entryWrite(entry, message, V_NETWORKID_LOCAL, FALSE, NULL, V_CONTEXT_GETHISTORY);
                    c_free(message);
                }
                else
                {
                    OS_REPORT(OS_ERROR,
                              "v_group",V_RESULT_INTERNAL_ERROR,
                              "flushInstanceRegistrations(registration=0x%"PA_PRIxADDR", arg=0x%"PA_PRIxADDR")\n"
                              "        Failed to allocate an unregister message.",
                              (os_address)registration, (os_address)arg);
                }
            }
        }
        registration = registration->next;
    }
}

static c_bool
flushInstance (
    c_object o,
    c_voidp arg)
{
    c_bool result;
    v_groupInstance grInst;
    v_groupFlushArg groupFlushArg;

    grInst = v_groupInstance(o);
    groupFlushArg = (v_groupFlushArg)arg;

    /* For each registration that doesn't have a sample, insert an explicit registration
     * message. When the group contains unregister messages for instances that are still
     * alive, the explicit registrations will make sure that when interpreting the result of
     * a flush the final registration-count will be correct. */
    flushInstanceRegistrations(grInst, groupFlushArg);

    result = v_groupInstanceWalkSamples(grInst,doFlush,arg);
    if(grInst->oldest) {
        if(result == TRUE){
            groupFlushArg->grInst = grInst;
            v_groupInstanceWalkUnregisterMessages(v_groupInstance(o),
                                                  doUnregisterFlush, arg);
        }
    }else {
        /* TODO: This will effectively stop forwarding of unregister-only messages when there is no data in a group. This is a situation that should not occur!  At the moment it still does
         * however, for reasons unknown. This fix (if grInst->oldest) is harmless and is a quick fix for customers. When the real cause is found, this comment should be replaced with
         * an assert. */
    }
    return result;
}

static void
flushElement(
    v_instance instance,
    v_message message,
    c_voidp arg)
{
    v_groupFlushArg groupFlushArg = (v_groupFlushArg)arg;

    groupFlushArg->action(message,
                          v_groupInstance(instance),
                          V_GROUP_FLUSH_TRANSACTION,
                          groupFlushArg->arg);
}

static c_bool
flushTransaction (
    c_object o,
    c_voidp arg)
{
    v_transaction transaction = v_transaction(o);

    v_transactionWalk(transaction, flushElement, arg);

    return TRUE;
}

void
v_groupFlushAction(
    v_group  _this,
    v_groupFlushCallback action,
    c_voidp  arg)
{
    C_STRUCT(v_groupFlushArg) groupFlushArg;

    assert(_this);
    assert(action);
    assert(arg);
    assert(C_TYPECHECK(_this,v_group));

    groupFlushArg.arg = arg;
    groupFlushArg.action = action;
    groupFlushArg.group  = NULL;
    groupFlushArg.entry = NULL;
    groupFlushArg.grInst = NULL;
    groupFlushArg.writeResult = V_WRITE_UNDEFINED;

    c_mutexLock(&_this->mutex);
    (void)c_tableWalk(_this->instances, flushInstance, &groupFlushArg);
    if (_this->transactionAdmin) {
        v_transactionGroupAdmin tgroupadm;
        v_transactionAdminWalkTransactions(_this->transactionAdmin, flushTransaction, &groupFlushArg);

        tgroupadm = v_transactionGetGroupAdmin(_this->transactionAdmin);
        if (tgroupadm) {
            v_transactionGroupAdminWalkTransactions(tgroupadm, _this, flushTransaction, &groupFlushArg);
        }
    }

    c_mutexUnlock(&_this->mutex);
}

C_STRUCT(v_entryRegisterArg) {
    v_message message;
    v_groupInstance instance;
    v_writeResult writeResult;
};

C_CLASS(v_entryRegisterArg);

C_STRUCT(v_instanceWriteArg) {
    v_message message;
    v_writeResult writeResult;
    c_iter deadCacheItems;
    c_bool resend; /* indicates if called from resend */
};

C_CLASS(v_instanceWriteArg);

C_STRUCT(v_entryWriteArg) {
    v_message message;
    v_networkId networkId;
    c_bool groupRoutingEnabled;
    v_writeResult writeResult;
    v_entry entry;
};

C_CLASS(v_entryWriteArg);

C_STRUCT(v_nwEntryWriteArg) {
    v_message message;
    v_networkId networkId;
    c_bool groupRoutingEnabled;
    v_writeResult writeResult;
    v_entry entry;
};

C_CLASS(v_nwEntryWriteArg);

C_STRUCT(v_writerAdmin) {
    c_ulong missedMessages;
    v_message message;
};

C_CLASS(v_writerAdmin);

static c_bool
handleSampleLost(
    v_groupEntry proxy,
    c_voidp arg)
{
    v_writerAdmin wrAdmin = (v_writerAdmin)arg;
    v_reader reader = v_entryReader(proxy->entry);

    /* Filter-out all QoS-incompatible messages. */
    if (v_messageQos_isReaderCompatible(wrAdmin->message->qos,reader)) {
        v_dataReaderNotifySampleLost(v_dataReader(reader),wrAdmin->missedMessages);
    }
    return TRUE;
}

static c_bool
entryRegister(
    v_groupEntry proxy,
    c_voidp arg)
{
    v_writeResult result;
    v_entryRegisterArg writeArg = (v_entryRegisterArg)arg;
    v_instance instance;
    v_groupCacheItem item;
    struct targetUnknownArg unknownArg;

    instance = NULL;

    result = v_dataReaderEntryWrite(v_dataReaderEntry(proxy->entry),
                                    writeArg->message,
                                    &instance,
                                    V_CONTEXT_GROUPWRITE);

    if (result != V_WRITE_SUCCESS) {
        writeArg->writeResult = result;
    } else {
        /* Write is successfull, however instance may not point to the
           reader instance, due to the following reasons:
           - message qos was incompatible with reader qos
           - the reader has a filter which rejects the message.
        */
        if (instance != NULL) {
            /* first check whether instance is already cached */
            unknownArg.instance = instance;
            unknownArg.cacheItem = NULL;
            (void)v_groupCacheWalk(writeArg->instance->targetCache, targetUnknown, &unknownArg);
            if (unknownArg.cacheItem == NULL) {
                item = v_groupCacheItemNew(writeArg->instance,instance);
                if (item) {
                    v_groupCacheInsert(proxy->connectionCache,item);
                    v_groupCacheInsert(writeArg->instance->targetCache,item);
                    c_free(item);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_group::entryRegister",V_RESULT_INTERNAL_ERROR,
                              "Failed to register instance");
                }
            } else {
                unknownArg.cacheItem->registrationCount++;
                assert(unknownArg.cacheItem->registrationCount <=
                       v_dataReaderInstance(instance)->liveliness);
            }
        }
    }
    if( instance != NULL) {
        c_free(instance);
    }
    return TRUE;
}

static c_bool
nwEntryWrite(
    v_groupEntry proxy,
    c_voidp arg)
{
    v_nwEntryWriteArg writeArg = (v_nwEntryWriteArg)arg;

    if ((writeArg->entry) && (writeArg->entry != proxy->entry)) {
        /* The message was not addressed to this network.
         */
    } else {
        v_writeResult result;

        result = v_networkReaderEntryWrite(
                     v_networkReaderEntry(proxy->entry),
                     writeArg->message,
                     writeArg->networkId,
                     writeArg->groupRoutingEnabled);

        /* This method is used in a walk method and visits all network
         * interfaces. The callee expects the writeResult to indecate if
         * a reject has occured so the result will only be set if a message
         * is rejected.
         */
        if (result == V_WRITE_REJECTED) {
            writeArg->writeResult = result;
        } else if (result != V_WRITE_SUCCESS) {
            OS_REPORT(OS_CRITICAL,
                        "v_writerInstance::nwEntryWrite",result,
                        "Internal error (%d) occured",
                        result);
        }
    }
    return TRUE;
}

static c_bool
entryWrite(
    v_groupEntry proxy,
    c_voidp arg)
{
    v_writeResult result;
    v_entryWriteArg writeArg = (v_entryWriteArg)arg;
    v_instance instance = NULL;

    if ((writeArg->entry) && (writeArg->entry != proxy->entry)) {
        /* The message was not addressed to this entry.
         */
        result = V_WRITE_SUCCESS;
    } else {
        result = v_entryWrite(proxy->entry,
                              writeArg->message,
                              writeArg->networkId,
                              writeArg->groupRoutingEnabled,
                              &instance,
                              V_CONTEXT_GROUPWRITE);
    }

    /* This method is used in a walk method and visits all network
     * interfaces. The callee expects the writeResult to indicate if
     * a reject has occurred so the result will only be set if a message
     * is rejected.
     */
    if (result == V_WRITE_REJECTED) {
        writeArg->writeResult = result;
    } else if (result != V_WRITE_SUCCESS) {
        OS_REPORT(OS_CRITICAL,
                    "v_group::entryWrite",result,
                    "Internal error (%d) occured",
                    result);
    }
    c_free(instance);
    return TRUE;
}


static c_bool
variantEntryResend(
    v_groupEntry proxy,
    c_voidp arg)
{
    c_bool success;

    /* Variant entries cannot reject when they have no resource limits. To
     * prevent samples that are rejected by another entry to arrive multiple
     * times (because there is no administration about pending resends when
     * there is no instance cache), don't resend samples in this case.
     */
    if(!v_resourcePolicyIIsUnlimited(v_reader(proxy->entry->reader)->qos->resource)){
        success = entryWrite(proxy, arg);
    } else {
        success = TRUE;
    }
    return success;
}

static c_bool
instanceWrite(
    v_cacheNode node,
    c_voidp arg)
{
    v_groupCacheItem item;
    v_writeResult result = V_WRITE_SUCCESS;
    v_dataReaderInstance instance;
    v_instanceWriteArg writeArg = (v_instanceWriteArg)arg;
    c_bool compatible;

    item = v_groupCacheItem(node);
    instance = v_dataReaderInstance(v_cacheItem(item)->instance);

    if (instance == NULL) {
        item->registrationCount = 0;
    }

    if (item->registrationCount != 0) {
        if (v_objectKind(v_cacheItem(item)->instance) == K_DATAREADERINSTANCE) {
            assert(item->registrationCount <= instance->liveliness);

            compatible = v_messageQos_isReaderCompatible(
                                writeArg->message->qos,
                                v_reader(v_instance(instance)->entity));

            result = v_dataReaderInstanceWrite(instance, writeArg->message);
            /* It is possible at this point that the instance has been freed by a
             * reader take. So, we shouldn't access it anymore. */

            if (result == V_WRITE_SUCCESS) {
                if (v_messageStateTest(writeArg->message,L_UNREGISTER)) {
                    if (compatible) {
                        item->registrationCount--;
                    }
                }
            } else {
                writeArg->writeResult = result;
                /* only increase pendingResends,
                 * when not called from instanceResend.
                 */
                if (!writeArg->resend) {
                    item->pendingResends++;
                }
            }
        } else {
            writeArg->writeResult = V_WRITE_PRE_NOT_MET;
        }
        /* Once registrationCount becomes 0, instance may not be read anymore. */
        assert(item->registrationCount == 0 || item->registrationCount <= instance->liveliness);
    }
    if (item->registrationCount == 0) {
        writeArg->deadCacheItems = c_iterInsert(writeArg->deadCacheItems, item);
    }

    return TRUE;
}

static c_bool
groupReadyToAcceptSample (
                          v_group group
                          )
{
    v_kernel kernel = v_objectKernel(group);

    /* If the kernel's configuration has at least one network service, and
     * they are not all yet connected to the group (i.e. because a service
     * has not finished its initialization), then the group must not accept
     * the sample.  If it did accept, that sample would never be received
     * by v_networkQueue and therefore would never be sent remotely.  This
     * would have the effect of remote nodes not getting some early written
     * samples (see scarab 2907 for discussion)
     * If the group rejects the sample, the ResendManager will be responsible
     * for ensuring the subsequent sending of the sample when the network
     * queues become active
     */

    /* Note this is a window where builtin topics are written before the kernel
     * has even parsed the XML.  In this case it may be possible that these
     * could be missed by the slow starting services.  However this is not an
     * issue since these can be aligned later by the durability service.
     */

    /* Implementation decision: it was considered to add something like an
     * "allServicesConnected" status flag to the group.  However in line with
     * the possible future requirement to support the dynamic starting, stopping
     * and restarting of internal services, as well as a possible altering of
     * the configuration at run time, it was felt that a check that the number
     * of currently attached services matched the number expected was more
     * future proof.  In this case the v_kernelNetworkCount function could be
     * updated to support the idea of a nodal configuration repository.
     */
    if (v_kernelNetworkCount (kernel) > (c_count(group->attachedServices) + c_count(group->notInterestedServices))) {
        return FALSE;
    }

    return TRUE;
}

static v_writeResult
forwardMessageToNetwork (
    v_group group,
    v_message message,
    v_networkId writingNetworkId,
    v_entry entry)
{
    C_STRUCT(v_nwEntryWriteArg) writeArg;
    writeArg.writeResult = V_WRITE_SUCCESS;

    /* Register messages never ever go out, and neither do V_NETWORKID_ANY messages */
    if (!v_messageStateTest(message,L_REGISTER) && writingNetworkId != V_NETWORKID_ANY) {

        writeArg.message = message;
        writeArg.networkId = writingNetworkId;
        writeArg.entry = entry;
        writeArg.groupRoutingEnabled = group->routingEnabled;

        if (writingNetworkId == V_NETWORKID_LOCAL) {
            /* Locally produced data must go out over the network, so check
             * whether the group is connected to all expected networking services.
             * If this is not the case, then reject the sample so it will not be
             * missed by late joining services.
             */
            if (groupReadyToAcceptSample(group)) {
                (void)v_groupEntrySetWalk(&group->networkEntrySet, nwEntryWrite, &writeArg);
            } else {
                writeArg.writeResult = V_WRITE_REJECTED;
            }
        } else if (group->routingEnabled) {
            /* Remotely produced data for a group that has routing enabled also
             * goes to all network services, which then do some additional filtering
             * on network id. We can't reject data coming from elsewhere, but it is
             * clear whether it is best to write it to whichever service is already
             * connected or to just drop it. For consistency, we drop it.
             */
            if (groupReadyToAcceptSample(group)) {
                (void)v_groupEntrySetWalk(&group->networkEntrySet, nwEntryWrite, &writeArg);
            }
        } else {
            /* Only those services that *always* want data from other network services
             * get it if the group doesn't have routingEnabled set. As an optimisation
             * that set of routing services is available as the "routedEntrySet". */
            if (groupReadyToAcceptSample(group)) {
                (void)v_groupEntrySetWalk(&group->routedEntrySet, nwEntryWrite, &writeArg);
            }
        }
    }
    return writeArg.writeResult;
}

static v_writeResult
forwardRegisterMessage (
    v_groupInstance instance,
    v_message message)
{
    C_STRUCT(v_entryRegisterArg) registerArg;
    v_group group;

    assert(C_TYPECHECK(instance,v_groupInstance));

    /* A register message indicated that a new instance is created
     * by a DataWriter.
     * Register messages are only used local node. This means that
     * it will only be sent to local DataReaders for optimization.
     * I.e. the DataReader will create an instance and return the
     * instance handle to build the local instance pipeline.
     */
    if (v_messageStateTest(message,L_REGISTER)) {

        group = v_groupInstanceOwner(instance);

        registerArg.message     = message;
        registerArg.writeResult = V_WRITE_SUCCESS;
        registerArg.instance    = instance;


        /* A connection update occured, so all readers must be addressed
         * to guarantee cache consistency
         */
        (void)v_groupEntrySetWalk(&group->topicEntrySet, entryRegister, &registerArg);
    } else {
        registerArg.writeResult = V_WRITE_PRE_NOT_MET;
    }
    return registerArg.writeResult;
}

v_message
v_groupCreateUntypedInvalidMessage(
    v_kernel kernel,
    v_message typedMsg)
{
    v_message untypedMsg;
    c_type msgType;

    /* Create a message for the invalid sample to carry. */
    msgType = v_kernelType(kernel, K_MESSAGE);
    untypedMsg = c_new(msgType);
    /* Set correct attributes. */
    v_node(untypedMsg)->nodeState = v_node(typedMsg)->nodeState;
    untypedMsg->writerGID = typedMsg->writerGID;
    untypedMsg->writeTime = typedMsg->writeTime;
    untypedMsg->allocTime = typedMsg->allocTime;
    untypedMsg->writerInstanceGID = typedMsg->writerInstanceGID;
    untypedMsg->qos = c_keep(typedMsg->qos);
    untypedMsg->sequenceNumber = typedMsg->sequenceNumber;
    untypedMsg->transactionId = typedMsg->transactionId;
    return untypedMsg;
}

/* forwardMessage now has a resendScope inout parameter so that it can be populated
 * with the rejection reason, in the event that the message is rejected.  This
 * is because forwardMessage is responsible for the initial actions of writing
 * the topic and variant information (which will become V_RESEND_TOPIC and
 * V_RESEND_VARIANT if rejected).
 */
static v_writeResult
forwardMessage (
    v_groupInstance instance,
    v_message message,
    v_networkId writingNetworkId,
    v_entry entry,
    c_bool bypassCache,
    v_resendScope *resendScope)
{
    C_STRUCT(v_entryWriteArg) writeArg;
    C_STRUCT(v_instanceWriteArg) instanceArg;
    v_group group;
    v_groupCacheItem item;
    c_bool doFree;
    v_kernel kernel;

    assert(C_TYPECHECK(instance,v_groupInstance));

    writeArg.writeResult = V_WRITE_SUCCESS;

    /* A register message indicated that a new instance is created
     * by a DataWriter.
     * Register messages are only used local node. This means that
     * it will only be sent to local DataReaders for optimization.
     * I.e. the DataReader will create an instance and return the
     * instance handle to build the local instance pipeline.
     */
    if (v_messageStateTest(message,L_REGISTER)) {
        forwardRegisterMessage(instance, message);
    } else {
        kernel = v_objectKernel(instance);
        group = v_groupInstanceOwner(instance);

        writeArg.networkId = writingNetworkId;
        writeArg.groupRoutingEnabled = group->routingEnabled;
        writeArg.entry = entry;

        /* If the sample has no valid content, replace the typed sample
         * with an untyped sample to save storage space, but only
         * if the provided message isn't an untyped sample already.
         */
        if ((!v_messageStateTest(message,L_WRITE)) &&
                (c_getType(message) != v_kernelType(kernel, K_MESSAGE)))
        {
            writeArg.message = v_groupCreateUntypedInvalidMessage(kernel, message);
            if (!writeArg.message) {
                writeArg.writeResult = V_WRITE_OUT_OF_RESOURCES;
            } else {
                instanceArg.message = writeArg.message;
                doFree = TRUE;
            }
        } else {
            writeArg.message = message;
            instanceArg.message = message;
            doFree = FALSE;
        }

        if (writeArg.writeResult == V_WRITE_SUCCESS) {
            if(!entry && !bypassCache){
                instanceArg.writeResult    = V_WRITE_SUCCESS;
                instanceArg.deadCacheItems = NULL;
                instanceArg.resend         = FALSE;

                /* No connection updates, so forward the messages via the cached
                   instances.
                 */
                (void)v_groupCacheWalk(instance->targetCache, instanceWrite, &instanceArg);
                if (instanceArg.deadCacheItems) {
                    item = v_groupCacheItem(c_iterTakeFirst(instanceArg.deadCacheItems));
                    while (item != NULL) {
                        v_groupCacheItemRemove(item, V_CACHE_ANY);
                        item = v_groupCacheItem(c_iterTakeFirst(instanceArg.deadCacheItems));
                    }
                    c_iterFree(instanceArg.deadCacheItems);
                }
                writeArg.writeResult = instanceArg.writeResult;
            } else {
                /*special case to handle writing to one specific entry*/
                (void)v_groupEntrySetWalk(&group->topicEntrySet, entryWrite, &writeArg);
            }

            if (writeArg.writeResult == V_WRITE_REJECTED) {
                /* if message rejected so far it needs to be resent with V_RESEND_TOPIC scope */
                *resendScope |= V_RESEND_TOPIC;
            }

            if (v_messageStateTest(message,L_WRITE)) {
                (void)v_groupEntrySetWalk(&group->variantEntrySet, entryWrite, &writeArg);
                if (writeArg.writeResult == V_WRITE_REJECTED) {
                    /* if message rejected in this block it needs to be resent with V_RESEND_VARIANT scope */
                    *resendScope |= V_RESEND_VARIANT;
                }
            }
            /* If an untyped message was allocated, then release it since it will
             * no longer be needed beyond this point. (All readers have kept the
             * message when required.)
             */
            if (doFree)
            {
                c_free(writeArg.message);
            }
        }
    }
    return writeArg.writeResult;
}

/*
c_long
v_groupSampleCount (
    v_group group)
{
    c_long count;
    assert(C_TYPECHECK(group,v_group));
    c_mutexLock(&group->mutex);
    count = v_sampleListLength(&group->sampleList);
    c_mutexUnlock(&group->mutex);
    return count;
}
*/


static c_bool
markGroupInstance(c_object o, c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(o);
    c_ulong *flags = (c_ulong *)arg;

    v_stateSet(instance->state, *flags);
    return TRUE;
}

static c_bool
unmarkGroupInstance(c_object o, c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(o);
    c_ulong *flags = (c_ulong *)arg;

    v_stateClear(instance->state, *flags);
    return TRUE;
}

/* Set the specified flags for all DataReader instance states associated
 * with the specified group.
 */
void
v_groupMarkGroupInstanceStates (
    v_group group,
    c_ulong flags)
{
    v_topicQos qos;

    assert(C_TYPECHECK(group,v_group));

    if (flags != 0) {
        c_mutexLock(&group->mutex);

        qos = v_topicQosRef(group->topic);
        if (qos->durability.v.kind != V_DURABILITY_VOLATILE) {
            (void)c_tableWalk(group->instances, markGroupInstance, &flags);
        }

        c_mutexUnlock(&group->mutex);
    }
}

/* Set the specified flags for all DataReader instance states associated
 * with the specified group.
 */
void
v_groupUnmarkGroupInstanceStates (
    v_group group,
    c_ulong flags)
{
    v_topicQos qos;

    assert(C_TYPECHECK(group,v_group));

    if (flags != 0) {
        c_mutexLock(&group->mutex);

        qos = v_topicQosRef(group->topic);
        if (qos->durability.v.kind != V_DURABILITY_VOLATILE) {
            (void)c_tableWalk(group->instances, unmarkGroupInstance, &flags);
        }

        c_mutexUnlock(&group->mutex);
    }
}

/* Reset the specified flags for all DataReader instance states associated
 * with the specified group.
 */
void
v_groupUnmarkReaderInstanceStates (
    v_group group,
    c_ulong flags)
{
    assert(C_TYPECHECK(group,v_group));

    if (flags != 0) {
        c_mutexLock(&group->mutex);

        v_groupEntrySetUnmarkReaderInstanceStates( &group->topicEntrySet, flags );

        c_mutexUnlock(&group->mutex);
    }
}

static c_bool
disposeGroupInstance (
    c_object o,
    c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(o);
    messageActionArg a = (messageActionArg) arg;

    if ((a->condition == NULL) || ((a->condition)(instance, a->arg))) {
        a->result = v_groupInstanceInsert(instance, a->message, FALSE, NULL, TRUE);
        /* Make sure that a dispose that is processed correctly without error is treated as success. */
        if (a->result == V_WRITE_DUPLICATE || a->result == V_WRITE_SUCCESS_NOT_STORED) {
            a->result = V_WRITE_SUCCESS;
        }
    }
    return TRUE;
}

static v_writeResult
disposeMarkedInstance(
    v_groupInstance instance,
    v_message disposeMsg)
{
    v_writeResult result = V_WRITE_SUCCESS;

    v_group group = v_group(instance->group);
    v_resendScope resendScope = V_RESEND_NONE;

    v_message registerMessage = v_groupInstanceCreateMessage(instance);
    if (registerMessage != NULL){
        v_nodeState(registerMessage) = L_REGISTER;
        registerMessage->writerGID = disposeMsg->writerGID;
        registerMessage->writeTime = disposeMsg->writeTime;
        registerMessage->qos = c_keep(disposeMsg->qos);
        result = groupWrite(group, registerMessage, NULL, V_NETWORKID_ANY, TRUE, NULL, &resendScope);
        c_free(registerMessage);
        if (result == V_WRITE_REGISTERED || result == V_WRITE_SUCCESS) {
            result = groupWrite(group, disposeMsg, &instance, V_NETWORKID_ANY, TRUE, NULL, &resendScope);
            if (result != V_WRITE_SUCCESS) {
                OS_REPORT(OS_ERROR, "v_group::disposeMarkedInstances", result, "During CATCHUP failed to dispose instance that no longer exists on master.");
            }
        } else {
            OS_REPORT(OS_ERROR, "v_group::disposeMarkedInstances", result, "During CATCHUP failed to re-register instance that no longer exists on master.");
        }
    } else {
        result = V_WRITE_OUT_OF_RESOURCES;
        OS_REPORT(OS_ERROR, "v_group::disposeMarkedInstances", result, "Failed to allocate message.");
    }
    return result;
}

v_writeResult
v_groupSweepMarkedInstances(
        v_group group,
        os_timeW timestamp)
{
    v_kernel kernel;
    v_gid nullGID;
    v_message disposeMsg;
    c_iter instances;
    v_groupInstance grInst;
    v_writeResult result = V_WRITE_SUCCESS;

    c_mutexLock(&group->mutex);
    kernel = v_objectKernel(group);
    v_gidSetNil(nullGID);

    disposeMsg = v_groupCreateInvalidMessage(kernel, nullGID, NULL, timestamp);
    if (disposeMsg)
    {
        /* Set the nodeState of the message to DISPOSED. UNREGISTER and L_IMPLICIT. */
        v_stateSet(v_nodeState(disposeMsg), L_DISPOSED | L_UNREGISTER | L_IMPLICIT);
        instances = ospl_c_select(group->instances, -1);
        while ((grInst = v_groupInstance(c_iterTakeFirst(instances))) != NULL && result == V_WRITE_SUCCESS) {
            if (v_stateTest(grInst->state, L_MARK)) {
                result = disposeMarkedInstance(grInst, disposeMsg);
            }
            c_free(grInst);
        }
        c_iterFree(instances);
        c_free(disposeMsg);
    } else {
        result = V_WRITE_OUT_OF_RESOURCES;
        OS_REPORT(OS_ERROR, "v_group::v_groupSweepMarkedInstances", result, "Failed to allocate message.");
    }
    c_mutexUnlock(&group->mutex);
    return result;
}


static v_writeResult
v_groupDisposeAllMatchingInstancesLocked (
    v_group group,
    os_timeW timestamp,
    c_ulong flags,
    c_bool (*condition)(c_object instance, c_voidp arg),
    c_voidp arg)
{
    C_STRUCT(messageActionArg) msgActionArg;
    v_kernel kernel;
    v_gid nullGID;
    v_topicQos qos;

    assert(C_TYPECHECK(group,v_group));

    kernel = v_objectKernel(group);
    qos = v_topicQosRef(group->topic);
    v_gidSetNil(nullGID);

    msgActionArg.condition = condition;
    msgActionArg.arg = arg;

    msgActionArg.message = v_groupCreateInvalidMessage(kernel, nullGID, NULL, timestamp);
    if (msgActionArg.message)
    {
        /* Set the nodeState of the message to DISPOSED, additionally, set all bits in the flags. */
        v_stateSet(v_nodeState(msgActionArg.message), L_DISPOSED | flags);
        msgActionArg.result = V_WRITE_SUCCESS;
        if (qos->durability.v.kind != V_DURABILITY_VOLATILE) {
            (void)c_tableWalk(group->instances, disposeGroupInstance, &msgActionArg);
        }
        if ( msgActionArg.result == V_WRITE_SUCCESS )
        {
            v_groupEntrySetDisposeAll( &group->topicEntrySet, &msgActionArg );
        }
        if ( msgActionArg.result == V_WRITE_SUCCESS )
        {
            v_groupEntrySetDisposeAll( &group->variantEntrySet, &msgActionArg );
        }
        c_free(msgActionArg.message);
    } else {
        msgActionArg.result = V_WRITE_OUT_OF_RESOURCES;
        OS_REPORT(OS_ERROR, "v_group::v_groupDisposeAll", msgActionArg.result, "Failed to allocate message.");
    }
    return msgActionArg.result;
}


v_writeResult
v_groupDisposeAllMatchingInstances (
    v_group group,
    os_timeW timestamp,
    c_ulong flags,
    c_bool (*condition)(c_object instance, c_voidp arg),
    c_voidp arg)
{
    v_writeResult res;
    c_mutexLock(&group->mutex);
    res = v_groupDisposeAllMatchingInstancesLocked(group, timestamp, flags, condition, arg);
    c_mutexUnlock(&group->mutex);
    return res;
}

v_writeResult
v_groupDisposeAll (
    v_group group,
    os_timeW timestamp,
    c_ulong flags)
{
    v_writeResult res;
    c_mutexLock(&group->mutex);
    res = v_groupDisposeAllMatchingInstancesLocked(group, timestamp, flags, NULL, NULL);
    group->lastDisposeAllTime = timestamp;
    c_mutexUnlock(&group->mutex);
    return res;
}


void
v_groupCheckForSampleLost(
    v_group group,
    v_message msg)
{
    v_groupwriterAdministration tmp, admin;
    C_STRUCT(v_groupwriterAdministration) templ;
    C_STRUCT(v_writerAdmin) status;
    c_ulong diff;
    c_type type;

    templ.gid = msg->writerGID;

    admin = v_groupwriterAdministration(c_find(group->writerAdministration, &templ));
    if (admin) {
        diff = msg->sequenceNumber - admin->seqNumber;
        if (diff > 1) {
            diff--; /* remove current sample from diff to match missed number of samples*/

            status.missedMessages = diff;
            status.message = msg;

            /* We missed messages notify the reader to update the lostsamples statistic and give sample lost notification.
             * Please note that handleSampleLost() always returns TRUE, and thus v_groupEntrySetWalk() will also always
             * return TRUE. Therefore we consciously ignore its return type by casting it to void.
             */
            (void) v_groupEntrySetWalk(&group->topicEntrySet,
                                       handleSampleLost,
                                       &status);
            (void) v_groupEntrySetWalk(&group->variantEntrySet,
                                       handleSampleLost,
                                       &status);
        }

        if (diff >= 1) {
            admin->seqNumber = msg->sequenceNumber;
        }
    } else {
        /* Add new writer admin */
        type = c_subType(group->writerAdministration);
        admin = c_new(type);
        c_free(type);
        if (admin) {
            admin->gid = msg->writerGID;
            admin->seqNumber = msg->sequenceNumber;
            tmp = ospl_c_insert(group->writerAdministration, admin);
            assert(tmp == admin);
            (void) tmp;
        } else {
            OS_REPORT(OS_ERROR,
              "v_groupCheckForSampleLost",V_RESULT_INTERNAL_ERROR,
              "Failed to allocate v_groupwriterAdministration object.");
            assert(FALSE);
        }
    }
    c_free(admin);
}

os_ssize_t
v_resendScopeImage (
    os_char *str,
    os_size_t len,
    v_resendScope scope)
{
    os_ssize_t tot = 0;

    assert (str != NULL);
    assert (!(scope & ~V_RESEND_ALL));

    if (scope == V_RESEND_NONE) {
        tot = snprintf (str, len, "V_RESEND_NONE");
    } else {
        os_char *flags[] = { NULL, NULL, NULL, NULL };
        os_int idx, max = 0;
        os_size_t pos = 0;
        os_ssize_t cnt = 0;

        if (scope & V_RESEND_TOPIC) {
            flags[max++] = "V_RESEND_TOPIC";
        }
        if (scope & V_RESEND_VARIANT) {
            flags[max++] = "V_RESEND_VARIANT";
        }
        if (scope & V_RESEND_REMOTE) {
            flags[max++] = "V_RESEND_REMOTE";
        }
        if (scope & V_RESEND_DURABLE) {
            flags[max++] = "V_RESEND_DURABLE";
        }

        for (idx = 0; idx < max && tot >= 0; idx++) {
            if (idx == 0) {
                cnt = snprintf (str + pos, len - pos, "%s", flags[idx]);
            } else {
                cnt = snprintf (str + pos, len - pos, " | %s", flags[idx]);
            }

            if (cnt >= 0) {
                if (cnt > (os_ssize_t)(len - pos)) {
                    pos = len;
                } else {
                    pos += (os_size_t)cnt;
                }
                tot += cnt;
            } else {
                tot = -1;
            }
        }
    }

    return tot;
}

static v_groupInstance
lookupInstanceByMessage(
    v_group group,
    v_message msg)
{
    v_groupInstance instance;
    c_value _keyValues[32];
    c_value *keyValues = _keyValues;
    c_array messageKeyList;
    c_ulong i, nrOfKeys;

    messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
    nrOfKeys = c_arraySize(messageKeyList);
    if (nrOfKeys > 32) {
        keyValues = os_malloc(sizeof(c_value) * nrOfKeys);
    }
    for (i=0;i<nrOfKeys;i++) {
        keyValues[i] = c_fieldValue(messageKeyList[i], msg);
    }
    instance = c_tableFind(group->instances, &keyValues[0]);
    /* Key values have to be freed again */
    for (i=0;i<nrOfKeys;i++) {
        c_valueFreeRef(keyValues[i]);
    }
    if (nrOfKeys > 32) {
        os_free(keyValues);
    }
    return instance;
}

/* As part of scarab#2907, the new inout v_resendScope parameter to groupWrite
 * is used to return the scope of the resending that is required in the event
 * that the message is rejected by the group.
 *
 * This is because there are 4 different resend scopes that may be required,
 * so it is unnecessary and inefficient to attempt to resend to those areas
 * that were already successful within groupWrite.  As an example, if the
 * write to the network readers fails, resendScope is set to V_RESEND_REMOTE,
 * but the rest of the operation will continue and attempt to do the other
 * Aspects of the groupWrite.  The resend manager will then use the resendScope
 * value to minimize the resends.
 */
static v_writeResult
groupWrite (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    c_bool stream,
    v_entry entry,
    v_resendScope *resendScope)
{
    v_groupInstance instance, found;
    v_writeResult result;
    v_topicQos qos;
    c_bool rejected = FALSE;
    v_message regMsg, disposeMsg = NULL;
    v_resendScope disposeScope = V_RESEND_NONE;
    c_bool bypassCache = 0;
    const os_timeE now = msg->allocTime;

    assert(C_TYPECHECK(group,v_group));
    assert(C_TYPECHECK(msg,v_message));
    assert(!v_messageStateTest(msg, L_ENDOFTRANSACTION));

    /* cleanup resources,
     * do not delay this call until the data is inserted,
     * it will have undesired side-effects on the just inserted data.
     */
    updatePurgeList(group, now);

    qos = v_topicQosRef(group->topic);

    if ((instancePtr == NULL) || (*instancePtr == NULL)) {
        instance = lookupInstanceByMessage(group, msg);
        if (instance == NULL) {
            if(v_messageStateTest(msg, L_UNREGISTER)){
                /* The instance doesn't exist, meaning no writer has it currently
                 * registered. An implicit register doesn't make sense for an
                 * unregister either. */
                return V_WRITE_SUCCESS;
            }
            instance = v_groupInstanceNew(group, msg);
            found = c_tableInsert(group->instances, instance);
            assert(found == instance);
            OS_UNUSED_ARG(found);
        }

        result = v_groupInstanceRegister(instance, msg);
        if (result == V_WRITE_REGISTERED){
            /* This writer is new for this instance. */
            if (!v_messageStateTest(msg, L_REGISTER)) {
                /* This message is not a register message.
                 * Therefore create an implicit register message and forward it to all peers.
                 */
                regMsg = v_topicMessageNew(group->topic);
                if (regMsg) {
                    memcpy(regMsg, msg, C_SIZEOF(v_message));
                    v_topicMessageCopyKeyValues(group->topic, regMsg, msg);
                    regMsg->qos = c_keep(msg->qos);
                    v_nodeState(regMsg) = (v_nodeState(msg) & L_TRANSACTION) | L_REGISTER;

                    if(stream == TRUE) {
                        forwardMessageToStreams(group, NULL, regMsg, now, V_GROUP_ACTION_REGISTER);
                    }
                    result = forwardRegisterMessage(instance, regMsg);
                    c_free(regMsg);
                } else {
                    OS_REPORT(OS_CRITICAL, OS_FUNCTION, V_RESULT_OUT_OF_MEMORY,
                              "Failed to create a register message for topic %s", v_topicName(group->topic));
                    assert(FALSE);
                    return V_RESULT_OUT_OF_MEMORY;
                }
            }
        } else if (result == V_WRITE_UNREGISTERED) {
            bypassCache = 1;
            result = V_WRITE_SUCCESS;
        }
        if ((instancePtr != NULL) && (*instancePtr == NULL)) {
            *instancePtr = c_keep(instance);
        }
        v_groupInstanceFree(instance);
        /* When the message is delivered out-of-order and belongs to an instance that
         * should have been covered by a newer dispose_all_data event that has already
         * been processed, then make sure the a corresponding dispose message is inserted
         * behind it in the readers. This dispose message is by definition an invalid
         * sample, and so does not count towards the resource limits. It should therefore
         * not be rejected by reason of resource limit exhaustion. If it is rejected
         * because of a still missing connection, then the previous sample is rejected
         * as well. So during its re-transmission this snippet of code will be touched
         * again, thereby re-inserting the dispose message as well.
         */
        if ( qos->orderby.v.kind == V_ORDERBY_SOURCETIME &&
             os_timeWCompare( msg->writeTime, group->lastDisposeAllTime ) == OS_LESS )
        {
            v_gid nullGID;

            v_gidSetNil(nullGID);
            disposeMsg = v_groupCreateInvalidMessage(v_objectKernel(group), nullGID, NULL, group->lastDisposeAllTime);
            if (disposeMsg) {
                v_stateSet(v_nodeState(disposeMsg), L_DISPOSED);
            } else {
                /* Error message already generated by v_groupCreateInvalidMessage. */
                return V_WRITE_OUT_OF_RESOURCES;
            }
        }
    } else {
        instance = *instancePtr;
        if (v_groupInstanceOwner(instance) != group) {
            assert(FALSE);
            return V_WRITE_PRE_NOT_MET;
        }
        if (!v_stateTestOr(v_nodeState(msg), L_WRITE | L_DISPOSED | L_UNREGISTER | L_TRANSACTION)) {
            return V_WRITE_SUCCESS;
        }
        result = V_WRITE_SUCCESS;
    }
    (void) result;

    V_MESSAGE_STAMP(msg,groupLookupTime);

    /* At this point the group instance is either created, resolved
     * or verified if it was provided by the callee.
     */

    assert(instance != NULL);

    /* For Unregister messages first unregister the instance to check
     * if it was registered, if not then no need to continue. */
    if (v_messageStateTest(msg,L_UNREGISTER)) {
        assert(!v_messageStateTest(msg,L_REGISTER));
        /* When entry is set the msg is not meant for the instance, when
         * msg is part of an transaction don't insert it just yet. */
        if ((!entry) &&
            (!v_message_isTransaction(msg))) {
            result = v_groupInstanceUnregister(instance,msg, NULL);
            if (result != V_WRITE_UNREGISTERED) {
                /* The message has no effect on the instance state.
                 * Because the writer was never seen before.
                 * Therefore readers will not be effected.
                 * So abort this method.
                 */
                return result;
            }
        }
    }

    /* Now forward the message to all networks. */
    result = forwardMessageToNetwork(group, msg, writingNetworkId, entry);
    if (result == V_WRITE_REJECTED) {
        rejected = TRUE;
        *resendScope |= V_RESEND_REMOTE;
    }

    /* Now forward the message to all subscribers.
     * The resendScope is passed into this function so it can be set for the
     * particular scope that is required.  This is because forwardMessage does
     * the work for sending of both V_RESEND_VARIANT and V_RESEND_TOPIC
     * functionality (which may be required independently).
     */
    result = forwardMessage(instance,msg,writingNetworkId, entry, bypassCache, resendScope);
    if (result == V_WRITE_REJECTED) {
        rejected = TRUE;
        /* if forwardMessage rejects, it will have already set resendScope itself */
    }
    if (result == V_WRITE_SUCCESS && disposeMsg != NULL) {
        result = forwardMessage(instance, disposeMsg, V_NETWORKID_LOCAL, NULL, 0, &disposeScope);
    }

    /* In case the message contains non volatile data then check the
     * durability resource limits and write the data into the Transient
     * (and persistent) store.
     * Be aware that a dispose message written by dispose_all_data has
     * no inline qos, but should still be stored in the group if the topic
     * qos is non-volatile.
     */
    if (v_message_isTransaction(msg) ||
        (!entry && (qos->durability.v.kind != V_DURABILITY_VOLATILE) &&
                   (!msg->qos || v_messageQos_durabilityKind(msg->qos) != V_DURABILITY_VOLATILE))) {
        result = v_groupInstanceInsert(instance, msg, FALSE, NULL, stream);
        if (result == V_WRITE_REJECTED) {
            rejected = TRUE;
            *resendScope |= V_RESEND_DURABLE;
        }
        if (result == V_WRITE_SUCCESS && disposeMsg != NULL)
        {
            /* When the message is delivered out-of-order and belongs to an instance that
             * should have been covered by a newer dispose_all_data event that has already
             * been processed, then make sure the a corresponding dispose message is inserted
             * behind it in the group and its streams.
             * This dispose message is by definition an invalid sample, and so does not count
             * towards the resource limits. It should therefore not be rejected by reason of
             * resource limit exhaustion. If it is rejected because of a still missing connection,
             * then the previous sample is rejected as well. So during its re-transmission this
             * snippet of code will be touched again, thereby re-inserting the dispose message
             * as well.
             */
            result = v_groupInstanceInsert(instance, disposeMsg, FALSE, NULL, TRUE);
            /* Make sure that a dispose that is processed correctly without error is treated as success. */
        }
        if (result == V_WRITE_DUPLICATE || result == V_WRITE_SUCCESS_NOT_STORED) {
            result = V_WRITE_SUCCESS;
        }
    } else {
        /* The message is VOLATILE. Only if the message is an UNREGISTER message
         * we might need to add the instance to one of the 2 purgelists. As the
         * message is volatile we can ignore the WRITE and DISPOSE messages.
         */
        if ((v_messageStateTest(msg,L_UNREGISTER) &&
             v_groupInstanceStateTest(instance,L_NOWRITERS)))
        {
            if ((qos->durability.v.kind != V_DURABILITY_VOLATILE) &&
                v_groupInstanceStateTest(instance,L_DISPOSED))
            {
                _dispose_purgeList_insert(instance, now);
            } else {
                if (v_groupInstanceStateTest(instance,L_EMPTY)) {
                    _empty_purgeList_insert(instance, now);
                }
            }
        }
    }
    if (rejected) {
        result = V_WRITE_REJECTED;
    }

    c_free(disposeMsg);

    return result;
}

static v_writeResult
groupWriteEOT (
    v_group group,
    v_message msg,
    v_networkId writingNetworkId,
    v_resendScope *resendScope)
{
    v_writeResult result = V_WRITE_SUCCESS;
    v_resendScope inputScope = *resendScope;
    c_bool complete = FALSE;
    v_topicQos qos;
    v_transactionAdmin transactionAdmin;

    /* cleanup resources,
     * do not delay this call until the data is inserted,
     * it will have undesired side-effects on the just inserted data.
     */
    updatePurgeList(group, msg->allocTime);

    /* Now forward the message to all networks. */
    if (inputScope == V_RESEND_NONE || (inputScope & V_RESEND_REMOTE)) {
        result = forwardMessageToNetwork(group, msg, writingNetworkId, NULL);
        if (result == V_WRITE_REJECTED) {
            *resendScope |= V_RESEND_REMOTE;
        }
    }

    /* Now forward the message to all subscribers and to the groups.
     * EOT's will never be rejected by readers or groups since they
     * represent an event, not a new sample. That means that it will
     * only need to be sent to readers and groups initially (during
     * the V_RESEND_NONE scope). There is no possible reason to retransmit
     * the EOT for readers and groups, like there is for the network.
     */
    if (inputScope == V_RESEND_NONE) {
        qos = v_topicQosRef(group->topic);
        if (v_message_isTransaction(msg) ||
            ((qos->durability.v.kind != V_DURABILITY_VOLATILE) &&
             ((!msg->qos || v_messageQos_durabilityKind(msg->qos) != V_DURABILITY_VOLATILE)))) {
            transactionAdmin = v__groupGetTransactionAdmin(group);
            if (transactionAdmin) {
                /* Now update the transaction administration in the group itself. */
                (void)v_transactionAdminInsertMessage(transactionAdmin, msg, NULL, FALSE, &complete);
                if (complete) {
                    if (!v_kernelGroupTransactionLockAccess(v_objectKernel(group))) {
                        v_kernelGroupTransactionFlush(v_objectKernel(group), transactionAdmin);
                        v_kernelGroupTransactionUnlockAccess(v_objectKernel(group));
                    }
                }
                c_free(transactionAdmin);
            }
        }
    }

    return result;
}

v_writeResult
v_groupWrite (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    v_resendScope *resendScope)
{
    v_writeResult result;

    c_mutexLock(&group->mutex);

    V_MESSAGE_STAMP(msg,groupInsertTime);
    if (v_messageStateTest(msg, L_ENDOFTRANSACTION)) {
        result = groupWriteEOT(group, msg, writingNetworkId, resendScope);
    } else {
        result = groupWrite(group, msg, instancePtr, writingNetworkId, TRUE, NULL, resendScope);
    }
    c_mutexUnlock(&group->mutex);

    return result;
}

struct WriteEOTArg {
    v_message msg;
    v_writeResult result;
};

static c_bool
dataReaderEntryWriteEOT(v_groupEntry groupEntry, c_voidp arg)
{
    struct WriteEOTArg *a = (struct WriteEOTArg *)arg;

    a->result = v_dataReaderEntryWriteEOT(v_dataReaderEntry(groupEntry->entry), a->msg);
    return (a->result == V_WRITE_SUCCESS);
}

void
v__groupDataReaderEntriesWriteEOTNoLock(
    v_group _this,
    v_message msg)
{
    struct WriteEOTArg arg;

#ifndef NDEBUG
    assert(os_threadIdToInteger(_this->mutex.owner) == os_threadIdToInteger(os_threadIdSelf()));
#endif
    arg.msg = msg;
    arg.result = V_WRITE_SUCCESS;
    (void)v_groupEntrySetWalk(&_this->topicEntrySet, dataReaderEntryWriteEOT, &arg);
    assert(arg.result == V_WRITE_SUCCESS);

    (void)v_groupEntrySetWalk(&_this->variantEntrySet, dataReaderEntryWriteEOT, &arg);
    assert(arg.result == V_WRITE_SUCCESS);
}

void
v_groupFlushTransactionNoLock(
    v_instance instance,
    v_message message,
    c_voidp arg)
{
    struct v_groupFlushTransactionArg *a = (struct v_groupFlushTransactionArg *)arg;
    v_topicQos qos;

    assert(message != NULL);

    if (v_messageStateTest(message, L_ENDOFTRANSACTION)) {
        /* Create a v_groupAction to notify durability that the
         * transaction has completed. */
        forwardMessageToStreams(a->group, NULL, message, message->allocTime, V_GROUP_ACTION_TRANSACTION_COMPLETE);
    } else {
        assert(instance);

        qos = v_topicQosRef(a->group->topic);
        if (v_messageStateTest(message, L_UNREGISTER)) {
            v_writeResult result;
            result = v_groupInstanceUnregister(v_groupInstance(instance), message, a->txn);
            if (result == V_WRITE_UNREGISTERED) {
                (void)v_groupInstanceInsert(v_groupInstance(instance), message, TRUE, a->txn, TRUE);
            }
        } else if (qos->durability.v.kind == V_DURABILITY_VOLATILE) {
            /* This is as far as a volatile sample goes */
            if (v_messageStateTest(message, L_WRITE)) {
                /* resources are only claimed for messages having set the L_WRITE flag */
                v_groupInstanceReleaseResource(v_groupInstance(instance));
            }
        } else {
            /* Push the message from the transaction into its corresponding instance.
             * Since the sample already consumed resource limits in the transactional
             * administration, no additional resource claim is needed.
             */
            (void)v_groupInstanceInsert(v_groupInstance(instance), message, TRUE, a->txn, TRUE);
        }
    }
}

v_writeResult
v_groupWriteNoStream (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId)
{
    v_writeResult result;
    v_resendScope resendScope = V_RESEND_NONE; /* resendScope not yet used here beyond this function */

    c_mutexLock(&group->mutex);
    result = groupWrite(group, msg, instancePtr, writingNetworkId, FALSE, NULL, &resendScope);
    c_mutexUnlock(&group->mutex);

    return result;
}

v_writeResult
v_groupWriteNoStreamWithEntry (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    v_entry entry)
{
    v_writeResult result;
    v_resendScope resendScope = V_RESEND_NONE; /* resendScope not yet used here beyond this function */

    c_mutexLock(&group->mutex);
    if (v_messageStateTest(msg, L_ENDOFTRANSACTION)) {
        result = groupWriteEOT(group, msg, writingNetworkId, &resendScope);
    } else {
        result = groupWrite(group, msg, instancePtr, writingNetworkId, FALSE, entry, &resendScope);
    }
    c_mutexUnlock(&group->mutex);

    return result;
}

v_writeResult
v_groupWriteCheckSampleLost(
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    v_resendScope *resendScope)
{
    v_writeResult result;

    c_mutexLock(&group->mutex);

    v_groupCheckForSampleLost(group, msg);

    V_MESSAGE_STAMP(msg, groupInsertTime);

    /* If the message constitutes and End Of Transaction (EOT) marker, then send
     * it to its own handling function.
     */
    if (c_baseMakeMemReservation(c_getBase(group), C_MM_RESERVATION_HIGH)) {
        if (v_messageStateTest(msg, L_ENDOFTRANSACTION)) {
            result = groupWriteEOT(group, msg, writingNetworkId, resendScope);
        } else {
            result = groupWrite(group, msg, instancePtr, writingNetworkId, TRUE, NULL, resendScope);
        }
        c_baseReleaseMemReservation(c_getBase(group), C_MM_RESERVATION_HIGH);
    } else {
        result = V_WRITE_OUT_OF_RESOURCES;
    }

    c_mutexUnlock(&group->mutex);

    return result;
}


static c_bool
purgeInstanceTimedAction(
    c_object o,
    c_voidp arg)
{
    os_timeE *t;
    v_groupInstance instance;

    t = (os_timeE *)arg;
    instance = v_groupInstance(o);
    v_groupInstancePurgeTimed(instance, *t);

    return TRUE;
}

static c_bool
isBuiltinGroup(
    v_group group)
{
    c_bool result = FALSE;
    c_char *name;

    if(strcmp(V_BUILTIN_PARTITION, v_partitionName(group->partition)) == 0){
        name = v_topicName(group->topic);
        if(strcmp(V_PARTICIPANTINFO_NAME, name) == 0) {
            result = TRUE;
        } else if(strcmp(V_TOPICINFO_NAME, name) == 0) {
            result = TRUE;
        } else if(strcmp(V_PUBLICATIONINFO_NAME, name) == 0) {
            result = TRUE;
        } else if(strcmp(V_SUBSCRIPTIONINFO_NAME, name) == 0) {
            result = TRUE;
        } else if(strcmp(V_HEARTBEATINFO_NAME, name) == 0) {
            result = TRUE;
        }
    }
    return result;
}

v_writeResult
v_groupDeleteHistoricalData(
    v_group group,
    os_timeE t)
{
    /* Do not delete historical data in builtin groups*/
    if(isBuiltinGroup(group) == FALSE){
        c_mutexLock(&group->mutex);
        c_tableWalk(group->instances, purgeInstanceTimedAction, &t);
        forwardMessageToStreams(group, NULL, NULL, t, V_GROUP_ACTION_DELETE_DATA);
        c_mutexUnlock(&group->mutex);
    }

    return V_WRITE_SUCCESS;
}

static c_bool
instanceResend(
    v_cacheNode node,
    c_voidp arg)
{
    v_groupCacheItem item;
    c_bool result = TRUE;
    v_instanceWriteArg writeArg = (v_instanceWriteArg)arg;

    item = v_groupCacheItem(node);

    result = TRUE;
    /* Only resend when for this pipeline resends are pending */
    if (item->pendingResends > 0) {
        result = instanceWrite(node, arg);
        if ((writeArg->writeResult != V_WRITE_SUCCESS) &&
            (writeArg->writeResult != V_WRITE_REJECTED)) {
                OS_REPORT(OS_ERROR,
                            "v_writerInstance::instanceResend",writeArg->writeResult,
                            "Internal error (%d) occured",
                            writeArg->writeResult);
        } else if (writeArg->writeResult == V_WRITE_SUCCESS){
            item->pendingResends--;
        }
    }

    return result;
}

/* For v_groupResend, the resendScope parameter will be populated with only
 * the scope of resend that is required for this message.  This was determined
 * within the groupWrite operation.  It is inefficient to attempt to resend
 * to the scopes that were satisified initially by groupWrite.
 */
v_writeResult
v_groupResend (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_resendScope *resendScope,
    v_networkId writingNetworkId)
{
    v_groupInstance instance;
    v_groupCacheItem item;
    v_writeResult result;

    assert(C_TYPECHECK(group,v_group));
    assert(C_TYPECHECK(msg,v_message));
    assert(instancePtr != NULL);
    assert(*instancePtr != NULL);

    result = V_WRITE_SUCCESS;

    c_mutexLock(&group->mutex);
    updatePurgeList(group, os_timeEGet());

    if ((instancePtr == NULL) || (*instancePtr == NULL)) {
        c_mutexUnlock(&group->mutex);
        return V_WRITE_ERROR;
    } else {
        instance = *instancePtr;
    }

    /* If the message is Non-Volatile and the resend scope includes
     * Durable storage then insert message into the group instance.
     */
    if (*resendScope & V_RESEND_DURABLE) {
        if ((v_messageQos_durabilityKind(msg->qos) != V_DURABILITY_VOLATILE) ||
            (v_message_isTransaction(msg))) {
            result = v_groupInstanceInsert(instance, msg, FALSE, NULL, TRUE);
            if (result == V_WRITE_SUCCESS || result == V_WRITE_DUPLICATE) {
                *resendScope = (v_resendScope) (*resendScope & ~V_RESEND_DURABLE);
            }
        } else {
            *resendScope = (v_resendScope) (*resendScope & ~V_RESEND_DURABLE);
        }
    }

    /* If the the resend scope includes remote interest
     * then write the message to the network interfaces.
     */
    if (*resendScope & V_RESEND_REMOTE)
    {
        /* The message must be rejected until the group is
         * connected to all expected networking services.
         */
        if (!groupReadyToAcceptSample(group)) {
            result = V_WRITE_REJECTED;
        } else {
            C_STRUCT(v_nwEntryWriteArg) arg;

            arg.message = msg;
            arg.networkId = writingNetworkId;
            arg.writeResult = V_WRITE_SUCCESS;
            arg.groupRoutingEnabled = group->routingEnabled;
            arg.entry = NULL;

            v_groupEntrySetWalk(&group->networkEntrySet, nwEntryWrite, &arg);

            if (arg.writeResult == V_WRITE_SUCCESS) {
                *resendScope = (v_resendScope) (*resendScope & ~V_RESEND_REMOTE);
            } else if (result != V_WRITE_REJECTED) {
                result = arg.writeResult;
            }
        }
    }

    /* If the the resend scope includes user defined key interest
     * then write the message to the DataReader in the variant entry set.
     */
    /* Only forward the message when the WRITE bit is set!
     * So only write and writedispose messages are forwarded to readers
     * with own storage spectrum.
     */
    if (v_messageStateTest(msg,L_WRITE) &&
        (*resendScope & V_RESEND_VARIANT))
    {
        C_STRUCT(v_entryWriteArg) arg;

        arg.message     = msg;
        arg.networkId   = writingNetworkId;
        arg.writeResult = V_WRITE_SUCCESS;
        arg.entry       = NULL;
        arg.groupRoutingEnabled = group->routingEnabled;

        v_groupEntrySetWalk(&group->variantEntrySet,
                            variantEntryResend,
                            &arg);

        if (arg.writeResult == V_WRITE_SUCCESS) {
            *resendScope = (v_resendScope) (*resendScope & ~V_RESEND_VARIANT);
        } else if (result != V_WRITE_REJECTED) {
            result = arg.writeResult;
        }
    }

    if (*resendScope & V_RESEND_TOPIC)
    {
        C_STRUCT(v_instanceWriteArg) arg;

        arg.message        = msg;
        arg.writeResult    = V_WRITE_SUCCESS;
        arg.deadCacheItems = NULL;
        arg.resend         = TRUE; /* indicates if called from resend */

        (void)v_groupCacheWalk(instance->targetCache, instanceResend, &arg);
        if (arg.writeResult == V_WRITE_SUCCESS) {
            *resendScope = (v_resendScope) (*resendScope & ~V_RESEND_TOPIC);
        } else if (result != V_WRITE_REJECTED) {
            result = arg.writeResult;
        }

        item = v_groupCacheItem(c_iterTakeFirst(arg.deadCacheItems));
        while (item != NULL) {
            v_groupCacheItemRemove(item, V_CACHE_ANY);
            item = v_groupCacheItem(c_iterTakeFirst(arg.deadCacheItems));
        }
        c_iterFree(arg.deadCacheItems);
    }

    c_mutexUnlock(&group->mutex);

    return result;
}

c_bool
v_groupNwAttachedGet(
    v_group g)
{
    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));

    return (g->networkEntrySet.firstEntry != NULL);
}

c_bool
v_groupCompleteGet(
    v_group g)
{
    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));

    return g->complete;
}

void
v_groupCompleteSet(
    v_group g,
    c_bool  complete)
{
    c_iter entrySet = NULL;
    v_groupEntry proxy;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));

    c_mutexLock(&g->mutex);
    g->complete = complete;
    proxy = g->topicEntrySet.firstEntry;
    while (proxy) {
        entrySet = c_iterAppend(entrySet, proxy);
        proxy = proxy->next;
    }
    proxy = g->variantEntrySet.firstEntry;
    while (proxy) {
        entrySet = c_iterAppend(entrySet, proxy);
        proxy = proxy->next;
    }
    c_mutexUnlock(&g->mutex);
    while ((proxy = (v_groupEntry) c_iterTakeFirst(entrySet))) {
        v_entryNotifyGroupStateChange(proxy->entry, g);
    }
    c_iterFree(entrySet);
}

struct walkEntryActionArg {
    v_groupEntryAction action;
    c_voidp arg;
};

static c_bool
walkEntryAction(
    v_groupEntry proxy,
    c_voidp arg)
{
    struct walkEntryActionArg *proxyAction = (struct walkEntryActionArg *)arg;

    return proxyAction->action(proxy->entry,proxyAction->arg);
}

c_bool
v_groupWalkEntries(
    v_group g,
    v_groupEntryAction action,
    c_voidp arg)
{
    c_bool result;
    struct walkEntryActionArg proxyAction;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(action != NULL);

    proxyAction.action = action;
    proxyAction.arg = arg;
    c_mutexLock(&g->mutex);
    result = v_groupEntrySetWalk(&g->networkEntrySet,
                                 walkEntryAction,&proxyAction);
    if (result == TRUE) {
        result = v_groupEntrySetWalk(&g->variantEntrySet,
                                     walkEntryAction,&proxyAction);
    }
    if (result == TRUE) {
        result = v_groupEntrySetWalk(&g->topicEntrySet,
                                     walkEntryAction,&proxyAction);
    }
    c_mutexUnlock(&g->mutex);

    return result;
}

/*
 * Provide a target walk function and corresponding arg for walking
 * through all groupInstances until a registration has been found that
 * matches the writerGID.
 */
struct InstanceWalkArg
{
    v_gid writerGID;
    c_iter instanceList;
    c_iter registrationList;
    v_matchIdentityAction predicate;
};

static c_bool
groupCollectMatchingRegistrations(
    c_object obj,
    c_voidp arg)
{
    struct InstanceWalkArg *iwArg = (struct InstanceWalkArg *)arg;
    v_groupInstance instance = v_groupInstance(obj);
    v_registration registration = instance->registrations;
    while(registration != NULL) {
        if (iwArg->predicate(registration->writerGID, iwArg->writerGID) == C_EQ) {
            c_iterAppend(iwArg->instanceList, c_keep(instance));
            c_iterAppend(iwArg->registrationList, c_keep(registration));
            registration = NULL;
        } else {
            registration = registration->next;
        }
    }
    return TRUE;
}

void
removeWriterAdmin(
    v_group _this,
    v_gid writerGID)
{
    C_STRUCT(v_groupwriterAdministration) templ;
    v_groupwriterAdministration found;

    templ.gid = writerGID;

    found = c_remove(_this->writerAdministration, &templ, NULL, NULL);
    if (found) {
        c_free(found);
    }
}

/*
 * This function unregisters all instances that relate through the provided
 * predicate to the GID supplied in the template.
 * The argument 'isImplicit' indicates whether the request to unregister was
 * implicit (i.e., on conto of the splice daemon) or an explicit unregister.
 * Setting 'isImplicit' to TRUE will cause the unregister message (and possibly
 * the dispose message generated by autodispose) to contain the flag L_IMPLICIT.
 */
static void
v_groupUnregisterByGidTemplate(
    v_group _this,
    v_gid tmplGid,
    v_matchIdentityAction predicate,
    os_timeW timestamp,
    c_bool isImplicit)
{
    v_registration registration;
    struct InstanceWalkArg arg;
    v_groupInstance grInst;

    /* Lock group. */
    c_mutexLock(&_this->mutex);

    /* Remove the writer admin for sample lost count */
    removeWriterAdmin(_this, tmplGid);

    /* Update the purgeList to make room for what is to come. */
    updatePurgeList(_this, os_timeEGet());

    /* Retrieve the group-registration for this Writer. (Be aware that this
     * increases its refCount!!) Currently the registration is not a singleton,
     * so just obtain the first registration you encounter that matches the
     * provided GID template using the provided predicate. In the future
     * it will be a singleton, which makes life a lot more convenient when
     * comparing it to the contents of the registration-list of each individual
     * groupInstance. (You can then do a pointer comparison instead of a
     * comparison based on GID.)
     */
    arg.writerGID = tmplGid;
    arg.instanceList = c_iterNew(NULL);
    arg.registrationList = c_iterNew(NULL);
    arg.predicate = predicate;
    c_tableWalk(_this->instances, groupCollectMatchingRegistrations, &arg);
    /* Unlock the group again to be able to forward the messages into the pipeline. */
    c_mutexUnlock(&_this->mutex);

    registration = (v_registration)c_iterTakeFirst(arg.registrationList);
    grInst = v_groupInstance(c_iterTakeFirst(arg.instanceList));
    while (registration)
    {
        v_groupInstancecleanup(grInst, registration, timestamp, isImplicit);
        /* Decrease refCount of the registration and instance again. */
        c_free(registration);
        v_groupInstanceFree(grInst);
        registration = (v_registration)c_iterTakeFirst(arg.registrationList);
        grInst = v_groupInstance(c_iterTakeFirst(arg.instanceList));
    }
    c_iterFree(arg.instanceList);
    c_iterFree(arg.registrationList);
}

/**
 * Unregister all instances in both the group and its readers that were
 * previously registered by the specified writer.
 */
void
v_groupDisconnectWriter(
    v_group _this,
    struct v_publicationInfo *oInfo,
    os_timeW timestamp,
    c_bool isLocal,
    c_bool isImplicit)
{
    if (!isLocal) {
        /* unregister the group because the writer has disconnect */
        v_groupUnregisterByGidTemplate(_this, oInfo->key, v_gidCompare, timestamp, isImplicit);
    }
}

/* This function provides a predicate for finding GIDs that originate on the
 * same node.  */
static c_equality
v_systemIdCompare(
    v_gid id1,
    v_gid id2)
{
    if (id1.systemId > id2.systemId) {
        return C_GT;
    }
    if (id1.systemId < id2.systemId) {
        return C_LT;
    }
    return C_EQ;
}

/**
 * Unregister all instances in both the group and its readers that were
 * previously registered by writers on the specified node. It does this by
 * first locating the registration for the builtin writer of the node that
 * got disconnected for this group, and then using the default mechanism
 * for finding and unregistering all its builtin topics.
 * Such unregistrations are considered to be implicit because no
 * unregistration is actually received; the splice daemon only deduced
 * that the writer is not alive anymore and generates an unregistration
 * (and possibly a disconnect).
 */
void
v_groupDisconnectNode(
    v_group _this,
    c_ulong systemId,
    os_timeW cleanTime)
{
    v_gid tmp;
    v_gidSetNil (tmp);
    tmp.systemId = systemId;
    v_groupUnregisterByGidTemplate(_this, tmp, v_systemIdCompare, cleanTime, TRUE);
}

struct streamHelper{
    v_group group;
    v_groupStream stream;
};

static c_bool
streamHistoricalSample(
    v_groupSample sample,
    c_voidp arg)
{
    struct streamHelper* h;
    v_message msg;
    v_groupAction action;

    h = (struct streamHelper*)arg;
    msg = v_groupSampleTemplate(sample)->message;

    if(v_messageStateTest(msg, L_WRITE)){
        action = v_groupActionNew(
                V_GROUP_ACTION_WRITE, msg->allocTime, msg, h->group);
    } else if(v_messageStateTest(msg, L_DISPOSED)){
        action = v_groupActionNew(
                V_GROUP_ACTION_DISPOSE, msg->allocTime, msg, h->group);
    } else if(v_messageStateTest(msg, L_REGISTER)){
        action = v_groupActionNew(
                V_GROUP_ACTION_REGISTER, msg->allocTime, msg, h->group);
    } else if(v_messageStateTest(msg, L_UNREGISTER)){
        action = v_groupActionNew(
                V_GROUP_ACTION_UNREGISTER, msg->allocTime, msg, h->group);
    } else {
        action = NULL;
    }

    if(action){
        v_groupStreamWrite(h->stream, action);
        c_free(action);
    }
    return TRUE;
}

static c_bool
streamHistoricalData(
    c_object o,
    c_voidp arg)
{
    return v_groupInstanceWalkSamples(v_groupInstance(o),
                                      streamHistoricalSample,
                                      arg);
}

void
v_groupStreamHistoricalData(
    v_group g,
    v_groupStream stream)
{
    struct streamHelper h;

    assert(C_TYPECHECK(g,v_group));
    assert(C_TYPECHECK(stream,v_groupStream));

    c_mutexLock(&g->mutex);
    updatePurgeList(g, os_timeEGet());
    h.group = g;
    h.stream = stream;
    c_tableWalk(g->instances, streamHistoricalData, &h);
    c_mutexUnlock(&g->mutex);
}

struct lookupReaderIntanceArg {
    v_entry trgtEntry;
    v_groupInstance prevGroupInst;
    v_dataReaderInstance readerInst;
    v_result result;
} ;

static c_bool lookupReaderInstance(
    v_cacheNode node,
    c_voidp arg)
{
    v_cacheItem item = v_cacheItem(node);
    struct lookupReaderIntanceArg *lriArg = (struct lookupReaderIntanceArg *)arg;
    v_dataReaderInstance readerInstance = v_dataReaderInstance(item->instance);
    v_index index = v_index(readerInstance->index);
    c_bool keepLooking = TRUE;

    if (index->reader == lriArg->trgtEntry->reader) {
        lriArg->readerInst = v_dataReaderInstance(c_keep(item->instance));
        keepLooking = FALSE;
    }
    return keepLooking;
}

static c_bool
writeHistoricalSample(
    v_groupSample sample,
    c_voidp arg)
{
    c_bool result = TRUE;
    c_base base = c_getBase(sample);
    v_message msg;
    v_groupInstance gi;
    v_writeResult writeResult;
    struct lookupReaderIntanceArg *lriArg = (struct lookupReaderIntanceArg *)arg;

    msg = v_groupSampleTemplate(sample)->message;
    gi = v_groupInstance(sample->instance);

    if ((!v_stateTest(v_nodeState(msg),L_REGISTER)) &&
        (!v_stateTest(v_nodeState(msg),L_UNREGISTER))) {
        v_instance *readerInst = (v_instance *) &lriArg->readerInst;

        /* For transient data, the pipeline should already have been set-up by the preceding
         * register messages. That means that there is a big chance the matching
         * v_dataReaderInstance can already be located in the targetCache of the groupInstance,
         * so it would save a lookup-by-key in the database if we try to establish the identity
         * through the pipeline first.
         */
        if (lriArg->prevGroupInst != gi) {
            lriArg->readerInst = NULL; /* Reset previous readerInstance first. */
            (void)v_groupCacheWalk(gi->targetCache, lookupReaderInstance, lriArg);
        }

        /* Keep track of the current groupInstance/readerInstance pair. */
        lriArg->prevGroupInst = gi;

        /* If the matching v_dataReaderInstance has not yet been located and the message is
         * a mini-message that does not contain the value of its keys, then we need to replace
         * the mini-message with a full-blown message including the keys, so that the entry
         * may do a lookup by key. The instance located this way will remain available
         * for delivery of subsequent messages belonging to the same instance.
         */
        if (c_baseMakeMemReservation(base, C_MM_RESERVATION_ZERO)) {
            if (*readerInst == NULL && c_getType(msg) == v_kernelType(v_objectKernel(gi), K_MESSAGE)) {
                v_message typedMessage = v_groupInstanceCreateTypedInvalidMessage(gi, msg);
                writeResult = v_entryWrite(lriArg->trgtEntry, typedMessage, V_NETWORKID_LOCAL, FALSE, readerInst, V_CONTEXT_GETHISTORY);
                c_free(typedMessage);
            } else {
                writeResult = v_entryWrite(lriArg->trgtEntry, msg, V_NETWORKID_LOCAL, FALSE, readerInst, V_CONTEXT_GETHISTORY);
            }
            c_baseReleaseMemReservation(base, C_MM_RESERVATION_ZERO);

            if (writeResult != V_WRITE_SUCCESS) {
                 OS_REPORT(OS_CRITICAL,
                           "v_group::writeHistoricalSample",result,
                           "writeHistoricalSample(0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR") failed with result %d.",
                           (os_address)sample, (os_address)arg, result);
            }
        } else {
            result = FALSE;
            lriArg->result = V_RESULT_OUT_OF_MEMORY;
            OS_REPORT(OS_CRITICAL,
                      "v_group::writeHistoricalSample",result,
                      "writeHistoricalSample(0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR") failed: out of memory.",
                      (os_address)sample, (os_address)arg);
        }
    }

    return result;
}

struct getHistoricalDataArg {
    v_entry entry;
    c_bool includeEOT;
    v_result result;
};


static c_bool
writeHistoricalData(
    c_object o,
    c_voidp arg)
{
    c_bool result;
    struct getHistoricalDataArg *histArg = arg;
    struct lookupReaderIntanceArg lriArg;
    lriArg.trgtEntry = v_entry(histArg->entry);
    lriArg.readerInst = NULL;
    lriArg.prevGroupInst = NULL;
    lriArg.result = V_RESULT_OK;

    result = v_groupInstanceWalkSamples(v_groupInstance(o), writeHistoricalSample, &lriArg);
    c_free(lriArg.readerInst); /* Ownership was transfered during v_entryWrite. */

    histArg->result = lriArg.result;

    assert((histArg->result == V_RESULT_OK) || (histArg->result == V_RESULT_OUT_OF_MEMORY));
    return result;
}


struct writeTransactionArg {
    v_dataReaderEntry entry;
    v_groupInstance prevGroupInst;
    v_dataReaderInstance readerInst;
    c_bool includeEOT;
};

struct writeHistoricalTransactionArg {
    v_dataReaderEntry entry;
    c_bool includeEOT;
};

static void
writeTransaction(
    v_instance instance,
    v_message message,
    c_voidp arg)
{
    struct writeTransactionArg *a = (struct writeTransactionArg *)arg;
    v_groupInstance gi = v_groupInstance(instance);
    v_instance *readerInst = (v_instance *) &a->readerInst;
    c_base base = c_getBase(instance);
    v_writeResult writeResult;

    if (message && v_messageStateTest(message, L_ENDOFTRANSACTION)) {
        if (a->includeEOT) {
            v_dataReaderEntryWriteEOT(a->entry, message);
         }
    } else if (instance) {
        if (a->prevGroupInst != gi) {
            struct lookupReaderIntanceArg lriArg = {0};
            lriArg.trgtEntry = v_entry(a->entry);
            (void)v_groupCacheWalk(gi->targetCache, lookupReaderInstance, &lriArg);
            a->readerInst = lriArg.readerInst;
        }

        /* Keep track of the current groupInstance/readerInstance pair. */
        a->prevGroupInst = gi;

        if (c_baseMakeMemReservation(base, C_MM_RESERVATION_ZERO)) {
            writeResult = v_entryWrite(v_entry(a->entry), message, V_NETWORKID_LOCAL, FALSE, readerInst, V_CONTEXT_GROUPWRITE);
            c_baseReleaseMemReservation(base, C_MM_RESERVATION_ZERO);

            if (writeResult != V_WRITE_SUCCESS) {
                 OS_REPORT(OS_CRITICAL,
                           "v_group::writeHistoricalSample",0,
                           "writeHistoricalSample(0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR") failed with result %d.",
                           (os_address)message, (os_address)arg, writeResult);
            }
        } else {
            OS_REPORT(OS_CRITICAL,
                      "v_group::writeHistoricalSample",0,
                      "writeHistoricalSample(0x%"PA_PRIxADDR", 0x%"PA_PRIxADDR") failed: out of memory.",
                      (os_address)message, (os_address)arg);
        }
    }
}

static c_bool
writeHistoricalTransaction(
    c_object o,
    c_voidp arg)
{
    v_transaction transaction = v_transaction(o);
    struct getHistoricalDataArg *histArgs = arg;
    struct writeTransactionArg writeArg;

    writeArg.entry = v_dataReaderEntry(histArgs->entry);
    writeArg.prevGroupInst = NULL;
    writeArg.readerInst = NULL;
    writeArg.includeEOT = histArgs->includeEOT;

    v_transactionWalk(transaction, writeTransaction, &writeArg);

    return TRUE;
}


static v_result
groupGetHistoricalData(
    v_group g,
    v_entry e,
    c_bool openTransactions)
{
    c_bool success;
    struct getHistoricalDataArg histArgs;

    OS_UNUSED_ARG(success);

    histArgs.entry = e;
    histArgs.includeEOT = FALSE;
    histArgs.result = V_RESULT_OK;

    success = c_tableWalk(g->instances, writeHistoricalData, &histArgs);
    assert((success && (histArgs.result == V_RESULT_OK)) ||
           (!success && (histArgs.result != V_RESULT_OK)));
    if (openTransactions && g->transactionAdmin) {
        v_transactionGroupAdmin tgroupadm;
        v_transactionAdminWalkTransactions(g->transactionAdmin, writeHistoricalTransaction, &histArgs);

        tgroupadm = v_transactionGetGroupAdmin(g->transactionAdmin);
        if (tgroupadm) {
            histArgs.includeEOT = TRUE;
            v_transactionGroupAdminWalkTransactions(tgroupadm, g, writeHistoricalTransaction, &histArgs);
        }
    }
    return histArgs.result;
}

v_result
v_groupGetHistoricalData(
    v_group g,
    v_entry e,
    c_bool openTransactions)
{
    v_result result = V_RESULT_OK;
    v_topicQos qos;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entry));

    c_mutexLock(&g->mutex);
    qos = v_topicQosRef(g->topic);
    if (qos->durability.v.kind != V_DURABILITY_VOLATILE) {
        updatePurgeList(g, os_timeEGet());
        result = groupGetHistoricalData(g, e, openTransactions);
    }
    c_mutexUnlock(&g->mutex);

    return result;
}

static q_expr
resolveField(
    v_group _this,
    const c_char *name)
{
    c_field field;
    c_array path;
    c_ulong i, length;
    q_list list;
    c_string str;
    c_type instanceType;
    c_char *fieldName;
    q_expr expr;

    instanceType = c_subType(_this->instances);
    field = c_fieldNew(instanceType,name);

    if (!field) {
        fieldName = os_alloca(strlen(name) + strlen("newest.message.userData.") + 1);
        os_sprintf(fieldName,"newest.%s",name);
        field = c_fieldNew(instanceType,fieldName);

        if (!field) {
            os_sprintf(fieldName,"newest.message.%s",name);
            field = c_fieldNew(instanceType,fieldName);
            if (!field) {
                os_sprintf(fieldName,"newest.message.userData.%s",name);
                field = c_fieldNew(instanceType,fieldName);
            }
        }
        os_freea(fieldName);
    }
    c_free(instanceType);
    if(field){
        path = c_fieldPath(field);
        length = c_arraySize(path);
        list = NULL;
        i = length;
        while (i-- > 0) {
            str = c_metaName(path[i]);
            list = q_insert(list,q_newId(str));
            c_free(str);
        }
        c_free(field);
        expr = q_newFnc(Q_EXPR_PROPERTY,list);
    } else {
        expr = NULL;
    }
    return expr;
}

static c_bool
resolveFields (
    v_group _this,
    q_expr e)
{
    /* search fields in result, data or info type. */

    q_expr p;
    c_long i;
    c_char *name;

    switch(q_getKind(e)) {
    case T_FNC:
        switch(q_getTag(e)) {
        case Q_EXPR_PROPERTY:
            name = q_propertyName(e);
            p = resolveField(_this,name);
            if (p == NULL) {
                OS_REPORT(OS_ERROR,
                            "v_groupGetHistoricalDataWithCondition failed",V_RESULT_ILL_PARAM,
                            "Parsing query expression failed: field '%s' "\
                            "is undefined for group '%s'",
                            name, _this->name);
                os_free(name);
                return FALSE;
            }
            os_free(name);
            q_swapExpr(e,p);
            q_dispose(p);
        break;
        default: /* process sub-expression */
            i=0;
            while ((p = q_getPar(e,i)) != NULL) {
                if (!resolveFields(_this,p)) {
                    return FALSE;
                }
                i++;
            }
        }
    break;
    case T_ID:
        name = q_getId(e);
        p = resolveField(_this,name);
        if (p == NULL) {
            OS_REPORT(OS_ERROR,
                        "v_groupGetHistoricalDataWithCondition failed",V_RESULT_ILL_PARAM,
                        "Parsing query expression failed: field '%s' "\
                        "is undefined for group '%s'",
                        name, _this->name);
            return FALSE;
        } else {
            q_swapExpr(e,p);
            q_dispose(p);
        }
    break;
    default:
    break;
    }
    return TRUE;
}

static c_iter
deOr(
    q_expr e,
    c_iter list)
{
    c_iter results;

    if (q_getTag(e) == Q_EXPR_OR) {
        results = deOr(q_takePar(e,0),deOr(q_takePar(e,0),list));
        q_dispose(e);
    } else {
        results = c_iterInsert(list,e);
    }
    return results;
}

struct historicalCondition {
    c_array instanceQ;
    c_array sampleQ;
    v_historicalDataRequest request;
    c_action action;
    c_voidp actionArgs;
    c_long insertedInstances;
    c_long insertedSamples;
    c_bool handleUnregistrations;
    v_result result;
};


#define PREFIX "newest.message"
static c_array
createKeyList(c_type instanceType, c_array keyList){
    c_ulong size, i;
    c_array newKeyList = NULL;

    assert(instanceType);

    if(keyList){
        size = c_arraySize(keyList);

        newKeyList = c_arrayNew(c_field_t(c_getBase(instanceType)), size);

        if(newKeyList){
            for(i = 0; i<size; i++){
                c_field f = c_fieldNew(instanceType, PREFIX);
                assert(f);
                if(f){
                    newKeyList[i] = c_fieldConcat(f, keyList[i]);
                    c_free(f);
                } else {
                    OS_REPORT(OS_ERROR,
                                "createKeyList", V_RESULT_INTERNAL_ERROR,
                                "Could not create c_field");
                }
            }
        } else {
            OS_REPORT(OS_ERROR,
                        "createKeyList", V_RESULT_INTERNAL_ERROR,
                        "Could not create array");
        }
    }
    return newKeyList;
}
#undef PREFIX



static c_bool
calculateCondition(
    v_group g,
    struct historicalCondition *condition)
{
    c_bool result = TRUE;
    q_expr predicate, e, subExpr, keyExpr, progExpr;
    c_ulong i,len;
    c_iter list;
    c_type type;
    c_array keyList;
    c_table instanceSet;
    c_value *params = NULL;
    c_ulong nparams;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));

    if(condition->request->filter){
        predicate = q_parse(condition->request->filter);

        if(predicate){
            e = q_takePar(predicate,0);

            if (!resolveFields(g,e)) {
                q_dispose(e);
                result = FALSE;
            } else {
                /* Normalize the query to the disjunctive form. */
                q_disjunctify(e);
                e = q_removeNots(e);

                list = deOr(e,NULL);
                len = c_iterLength(list);

                type = c_resolve(c_getBase(c_object(v_objectKernel(g))),"c_query");

                condition->instanceQ = c_arrayNew(type,len);
                condition->sampleQ   = c_arrayNew(type,len);
                c_free(type);

                if(condition->request->filterParams){
                    nparams = c_arraySize(condition->request->filterParams);
                    params = (c_value *) os_malloc(nparams * sizeof(struct c_value));

                    if ( params != NULL ) {
                        for ( i = 0; i < nparams; i++ ) {
                            params[i] = c_stringValue(condition->request->filterParams[i]);
                        }
                    } else {
                        result = FALSE;
                    }
                }
                instanceSet = g->instances;

                keyList = createKeyList(g->instanceType, v_topicMessageKeyList(g->topic));

                for (i=0;i<len && result;i++) {
                    subExpr = c_iterTakeFirst(list);
                    assert(subExpr != NULL);
                    keyExpr = q_takeKey(&subExpr, keyList);

                    if (keyExpr != NULL) {
                        progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
                        condition->instanceQ[i] = c_queryNew(instanceSet,
                                                  progExpr,
                                                  params);
                        q_dispose(progExpr);

                        if (condition->instanceQ[i] == NULL) {
                            OS_REPORT(OS_ERROR,
                                      "calculateCondition failed",
                                      V_RESULT_ILL_PARAM, "error in query expression");
                            result = FALSE;
                        }
                    } else {
                        condition->instanceQ[i] = NULL;
                    }
                    if (subExpr != NULL) {
                        progExpr   = F1(Q_EXPR_PROGRAM,subExpr);
                        condition->sampleQ[i] = c_queryNew(instanceSet,
                                                progExpr,
                                                params);
                        q_dispose(progExpr);

                        if (condition->sampleQ[i] == NULL) {
                            OS_REPORT(OS_ERROR,
                                      "calculateCondition failed",V_RESULT_ILL_PARAM,
                                      "error in query expression");
                            result = FALSE;
                        }
                    } else {
                        condition->sampleQ[i] = NULL;
                    }
                }
                c_iterFree(list);
                c_free(keyList);
            }
            q_dispose(predicate);
        } else {
            result = FALSE;
        }
    } else {
        /*No instance/sample query*/
        result = TRUE;
    }
    if(!result){
        if(condition->instanceQ){
            c_free(condition->instanceQ);
            condition->instanceQ = NULL;
        }
        if(condition->sampleQ){
            c_free(condition->sampleQ);
            condition->sampleQ = NULL;
        }
    }
    return result;
}

static c_bool
checkResourceLimits(
    struct historicalCondition* condition,
    c_long samplesBefore)
{
    c_bool proceed = TRUE;

    if (condition->request != NULL) {
        v_resourcePolicyI *resourceLimits = &condition->request->resourceLimits;
        if((resourceLimits->v.max_samples != -1) &&
           (resourceLimits->v.max_samples <= condition->insertedSamples))
        {
            proceed = FALSE;
        } else if((resourceLimits->v.max_samples_per_instance != -1) &&
                  (resourceLimits->v.max_samples_per_instance <= condition->insertedInstances - samplesBefore))
        {
             proceed = FALSE;
        } else if((resourceLimits->v.max_instances != -1) &&
                  (resourceLimits->v.max_instances <= condition->insertedInstances))
        {
            proceed = FALSE;
        }
    }
    return proceed;
}

static c_bool
checkConditionPeriod(
    v_groupSample sample,
    struct historicalCondition* condition)
{
    c_bool pass = TRUE;
    if (condition->request != NULL) {
        os_timeW writeTime = v_groupSampleTemplate(sample)->message->writeTime;
        if (!OS_TIMEW_ISINVALID(condition->request->minSourceTimestamp)) {
            if (os_timeWCompare(condition->request->minSourceTimestamp, writeTime) == OS_MORE) {
                /*produced before minimum*/
                pass = FALSE;
            }
        }
        if (pass && !OS_TIMEW_ISINVALID(condition->request->maxSourceTimestamp)) {
            if(os_timeWCompare(condition->request->maxSourceTimestamp, writeTime) == OS_LESS) {
                /*produced after maximum*/
                pass = FALSE;
            }
        }
    }
    return pass;
}

static c_bool
checkTransientLocal(
    v_groupSample sample)
{
    v_message vmessage;
    v_groupInstance instance;
    v_registration reg;
    c_bool pass = TRUE;

    vmessage = v_groupSampleTemplate(sample)->message;
    if ( (v_messageQos_durabilityKind(vmessage->qos) == V_DURABILITY_TRANSIENT_LOCAL) ) {
        /* check if the sample is registered; if so, the writer is alive */
        instance = v_groupInstance(sample->instance);
        reg = instance->registrations;
        pass = FALSE;
        while ( (reg != NULL) && ! pass ) {
            if ( v_gidCompare(reg->writerGID,vmessage->writerInstanceGID) == C_EQ) {
                pass = TRUE;
            } else {
                reg = reg->next;
            }
        }
    }
    return pass;
}

static c_bool
walkMatchingSamples(
    c_object obj,
    c_voidp args)
{
    struct historicalCondition* condition;
    c_ulong len, i;
    c_long samplesBefore;
    v_groupSample firstSample, sample;
    c_bool pass, proceed;
    v_groupInstance instance;
    v_groupFlushArg groupFlushArg;
    c_iter matchingSamples = NULL;

    instance = (v_groupInstance)obj;
    condition = (struct historicalCondition*)args;
    samplesBefore = condition->insertedSamples;

    proceed = TRUE;

    if (! v_stateTest(instance->state, L_EMPTY)) {
        /* only walk in case the instance queue is not empty */
        if (condition->instanceQ && condition->sampleQ) {
            len = c_arraySize(condition->instanceQ);
            for (i=0; (i<len) && proceed;i++) {
                if (condition->instanceQ[i]) {
                    pass = c_queryEval(condition->instanceQ[i],instance);
                } else {
                    pass = TRUE;
                }
                if (pass) { /* instance matches query*/
                    /* Since history is 'replayed' here, the oldest sample should be
                     * processed first. We keep a reference to the first sample and
                     * set the current sample to the tail of the instance (oldest). */
                    firstSample = v_groupInstanceHead(instance);
                    sample = v_groupInstanceTail(instance);
                    while ((sample != NULL) && proceed) {
                        if (len == 1 || !c_iterContains(matchingSamples, sample)) {
                            /* prevent aligning local-transient data without alive writers */
                            pass = checkTransientLocal(sample);
                            if (pass) {
                                /* prevent aligning data outside request period */
                                pass = checkConditionPeriod(sample, condition);
                            }
                            if (pass) {
                                /* prevent aligning data not fulfilling query */
                                if (sample != firstSample) {
                                    v_groupInstanceSetHeadNoRefCount(instance,sample);
                                }
                                if ((condition->sampleQ[i])) {
                                    pass = c_queryEval(condition->sampleQ[i], instance);
                                } else {
                                    pass = TRUE;
                                }
                                if (sample != firstSample) {
                                    v_groupInstanceSetHeadNoRefCount(instance,firstSample);
                                }
                            }
                            if (pass) {
                                if (len > 1) {
                                    matchingSamples = c_iterInsert(matchingSamples, sample);
                                }
                                condition->insertedSamples++;
                                condition->action(sample, condition->actionArgs);
                                proceed = checkResourceLimits(condition, samplesBefore);
                            }
                        }
                        sample = sample->newer;
                    }
                }
            }
            c_iterFree(matchingSamples);
        } else {
            sample = v_groupInstanceTail(instance);
            while ((sample != NULL) && proceed) {
               /* prevent aligning local-transient data without alive writers */
                pass = checkTransientLocal(sample);
                if (pass) {
                    /* prevent aligning data outside request period */
                    pass = checkConditionPeriod(sample, condition);
                }
                if (pass) {
                    condition->insertedSamples++;
                    condition->action(sample, condition->actionArgs);
                    proceed = checkResourceLimits(condition, samplesBefore);
                }
                sample = sample->newer;
            }
        }

        if (condition->insertedSamples > samplesBefore) {
            /*In case the instance matches the condition, also check whether
             * unregister messages need to be forwarded
             */
            if (condition->handleUnregistrations) {
                groupFlushArg = (v_groupFlushArg)condition->actionArgs;
                groupFlushArg->grInst = instance;
                v_groupInstanceWalkUnregisterMessages(
                        instance, doUnregisterFlush, groupFlushArg);
            }
            /* Increase instance count and check max instance resource limits. */
            condition->insertedInstances++;
            proceed = checkResourceLimits(condition, samplesBefore);
        }
    }
    return proceed;
}

v_result
v_groupGetHistoricalDataWithCondition(
    v_group g,
    v_entry entry,
    v_historicalDataRequest request)
{
    c_bool success;
    v_result result;
    struct historicalCondition condition;
    struct lookupReaderIntanceArg actionArgs;

    OS_UNUSED_ARG(success);

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(entry != NULL);
    assert(C_TYPECHECK(entry,v_entry));
    assert(C_TYPECHECK(request,v_historicalDataRequest));

    actionArgs.trgtEntry = entry;
    actionArgs.readerInst = NULL;
    actionArgs.prevGroupInst = NULL;
    actionArgs.result = V_RESULT_OK;

    condition.instanceQ             = NULL;
    condition.sampleQ               = NULL;
    condition.request               = request;
    condition.actionArgs            = &actionArgs;
    condition.action                = (c_action)writeHistoricalSample;
    condition.insertedInstances     = 0;
    condition.insertedSamples       = 0;
    condition.handleUnregistrations = FALSE;

    if(calculateCondition(g, &condition)){
        /*Get all matching data and send it to the reader*/
        success = c_walk(g->instances, walkMatchingSamples, &condition);
        result = actionArgs.result;
        c_free(condition.instanceQ);
        c_free(condition.sampleQ);
    } else {
        result = V_RESULT_ILL_PARAM;
    }

    return result;
}

void
v_groupFlushActionWithCondition(
    v_group  g,
    v_historicalDataRequest request,
    v_groupFlushCallback action,
    c_voidp  arg)
{
    c_bool result;
    struct historicalCondition condition;
    C_STRUCT(v_groupFlushArg) groupFlushArg;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(C_TYPECHECK(request,v_historicalDataRequest));

    result = TRUE;

    groupFlushArg.arg    = arg;
    groupFlushArg.group  = g;
    groupFlushArg.action = action;
    groupFlushArg.entry = NULL;
    groupFlushArg.grInst = NULL;

    condition.instanceQ             = NULL;
    condition.sampleQ               = NULL;
    condition.request               = request;
    condition.actionArgs            = &groupFlushArg;
    condition.action                = (c_action)doFlush;
    condition.insertedInstances     = 0;
    condition.insertedSamples       = 0;
    condition.handleUnregistrations = TRUE;

    if (request != NULL) {
        result = calculateCondition(g, &condition);
    }
    if(result){
        (void) c_walk(g->instances, walkMatchingSamples, &condition);
    }
    c_free(condition.instanceQ);
    c_free(condition.sampleQ);
}

void
v_groupUpdatePurgeList(
    v_group group)
{
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    if(group){
        c_mutexLock(&group->mutex);
        updatePurgeList(group, os_timeEGet());
        c_mutexUnlock(&group->mutex);
    }
}

v_groupInstance
v_groupLookupInstance(
    v_group group,
    c_value keyValue[])
{
    v_groupInstance result;

    if(group && keyValue){
        c_mutexLock(&group->mutex);
        result = c_tableFind(group->instances, &keyValue[0]);
        c_mutexUnlock(&group->mutex);
    } else {
        result = NULL;
    }
    return result;
}

v_groupInstance
v_groupLookupInstanceAndRegistration(
    v_group group,
    c_value keyValue[],
    v_gid gidTemplate,
    v_matchIdentityAction predicate,
    v_registration *registration)
{
    v_groupInstance result;

    if(group && keyValue){
        c_mutexLock(&group->mutex);
        result = c_tableFind(group->instances, &keyValue[0]);
        if (result) {
            if (registration) {
                *registration = v_groupInstanceGetRegistration(result, gidTemplate, predicate);
            }
        }
        c_mutexUnlock(&group->mutex);
    } else {
        result = NULL;
    }
    return result;
}

void
v_groupNotifyGroupCoherentPublication(
    v_group _this,
    v_message msg)
{
    struct v_publicationInfo *info;
    c_bool dispose;
    c_bool isImplicit;

    c_mutexLock(&_this->mutex);
    if (_this->transactionAdmin) {
        info = v_builtinPublicationInfoData(msg);
        dispose = v_messageStateTest(msg, L_DISPOSED);
        isImplicit = v_messageStateTest(msg, L_IMPLICIT);
        (void)v_transactionAdminNotifyPublication(_this->transactionAdmin, info->key, dispose, info, isImplicit);
    }
    c_mutexUnlock(&_this->mutex);
}

os_boolean
v_groupIsDurable(
    v_group _this)
{
    return (v_topicQosRef(v_groupTopic(_this))->durability.v.kind != V_DURABILITY_VOLATILE);
}

/**
 * \brief              This operation gets the transaction admin from the group
 *                     and lazy creates it if it does not exist yet.
 *
 * \pre              : The group is locked
 *
 * \param _this      : The v_group this operation operates on
 *
 * \return           : A kept reference to the v_transactionAdmin
 */
v_transactionAdmin
v__groupGetTransactionAdmin(
    v_group _this)
{
    if (_this->transactionAdmin == NULL) {
        _this->transactionAdmin = v_transactionAdminNew(v_object(_this),
                                  v_kernelTransactionGroupAdmin(v_objectKernel(_this)),
                                  _this->topic);
    }
    return c_keep(_this->transactionAdmin);
}

void
v_groupSetOnRequest(
    v_group _this,
    c_bool value)
{
    c_mutexLock(&_this->mutex);
    _this->onRequest = value;
    c_mutexUnlock(&_this->mutex);
}

c_bool
v_groupIsOnRequest(
    v_group _this)
{
    return _this->onRequest;
}

void
v_groupNotifyWriter(
    v_group _this,
    v_writer w)
{
    v_kernel kernel;
    C_STRUCT(v_event) event;

    c_mutexLock(&_this->mutex);
    if (_this->pristine) {
        kernel = v_objectKernel(_this);

        /* Connect writer event
         *
         * This event is introduced for durability to be able to act on a writer connecting to a group, which
         * signals that that group is being written to. For durability, this is a trigger to mark the
         * corresponding namespace with an infinite quality, which causes potential delayed alignment actions
         * of late-joining persistent sources to be discarded.
         */
        event.kind = V_EVENT_CONNECT_WRITER;
        event.source = v_observable(w);
        event.data = _this;
        v_observableNotify(v_observable(kernel),&event);
        _this->pristine = FALSE;
    }
    c_mutexUnlock(&_this->mutex);
}


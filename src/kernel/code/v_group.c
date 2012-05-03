/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include "v__group.h"
#include "v_kernel.h"
#include "v_policy.h"
#include "v__entry.h"
#include "v_partition.h"
#include "v__topic.h"
#include "v_message.h"
#include "v__messageQos.h"
#include "v__entity.h"
#include "v_proxy.h"
#include "v_entity.h"
#include "v_writer.h"
#include "v_reader.h"
#include "v_public.h"
#include "v_instance.h"
#include "v__dataReaderInstance.h"
#include "v_groupCache.h"
#include "v_groupInstance.h"
#include "v__groupStream.h"
#include "v__leaseManager.h"
#include "v__groupEntry.h"
#include "v__lifespanAdmin.h"
#include "v_networkReaderEntry.h"
#include "v_time.h"
#include "v__builtin.h"
#include "c_time.h"
#include "c_extent.h"

#include "os_heap.h"
#include "os_report.h"
#include "os.h"

#define V_GROUP_NAME_TEMPLATE "Group<%s,%s>"

v_groupSample
v_groupSampleNew (
    v_group group,
    v_message msg)
{
    v_groupSample sample;

    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    sample = v_groupSample(c_extentCreate(group->sampleExtent));
    if (sample) {
        v_groupSampleSetMessage(sample,msg);
        sample->older = NULL;
        sample->newer = NULL;


        if (v_messageQos_isInfiniteLifespan(msg->qos)) {
            v_lifespanSample(sample)->expiryTime = C_TIME_INFINITE;
        } else {
            c_time lifespan;

            lifespan = v_messageQos_getLifespanPeriod(msg->qos);
#ifdef _NAT_
            v_lifespanSample(sample)->expiryTime = c_timeAdd(v_timeGet(),
                                                             lifespan);
#else
            v_lifespanSample(sample)->expiryTime = c_timeAdd(msg->allocTime,
                                                             lifespan);
#endif
        }
        if (v_messageStateTest(msg,L_WRITE)) {
            group->count++;
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
    c_long length,sres;

    kernel = v_objectKernel(topic);
    base = c_getBase(kernel);
    baseType = c_resolve(base,"kernelModule::v_groupSampleTemplate");
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
    assert(sres == (length-1));
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
    v_topic topic)
{
    v_kernel kernel;
    c_type instanceType, baseType, foundType;
    c_metaObject o;
    c_base base;
    c_char *name;
    c_long length,sres;

    kernel = v_objectKernel(topic);
    base = c_getBase(kernel);
    baseType = c_resolve(base,"kernelModule::v_groupInstanceTemplate");
    assert(baseType != NULL);

    instanceType = c_type(c_metaDefine(c_metaObject(base),M_CLASS));
    c_class(instanceType)->extends = c_class(baseType); /* transfer refCount */
    foundType = v_topicKeyType(topic);
    if ( foundType != NULL) {
        o = c_metaDeclare(c_metaObject(instanceType),"key",M_ATTRIBUTE);
        c_property(o)->type = foundType; /* transfer refcount */
        c_free(o);
    }
    c_metaObject(instanceType)->definedIn = c_keep(base);
    c_metaFinalize(c_metaObject(instanceType));

#define INSTANCE_NAME "v_groupInstance<v_groupSample<>>"
#define INSTANCE_FORMAT "v_groupInstance<v_groupSample<%s>>"
    /* Create a name and bind type to name */
    /* Ths sizeof contains \0 */
    length = sizeof(INSTANCE_NAME) + strlen(v_topicName(topic));
    name = os_malloc(length);
    sres = snprintf(name,length,INSTANCE_FORMAT,v_topicName(topic));
    assert(sres == (length-1));
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
        v_stateSet(v_nodeState(message), L_REGISTER);
        v_entryWrite(a->proxy->entry, message, V_NETWORKID_LOCAL, &instance);
        if (instance != NULL) {
            unknownArg.instance = instance;
            unknownArg.cacheItem = NULL;
            v_groupCacheWalk(a->instance->targetCache,
                                   targetUnknown, &unknownArg);
            if (unknownArg.cacheItem == NULL) {
                /* Instance is not yet cached.
                 * So insert the instance in the cache.
                 */
                item = v_groupCacheItemNew(a->instance, instance);
                if (item) {
                    v_groupCacheInsert(a->proxy->connectionCache, item);
                    v_groupCacheInsert(a->instance->targetCache, item);
                    v_dataReaderInstanceRegisterSource(instance, item);
                    c_free(item);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_group::doRegister",0,
                              "Failed to register instance");
                }
            } else {
                /* Instance is already cached.
                 * So increase the registration count.
                 */
                unknownArg.cacheItem->registrationCount++;
                assert(unknownArg.cacheItem->registrationCount <=
                       v_dataReaderInstance(instance)->liveliness);
            }
        }
        c_free(instance);
        c_free(message);
    }
    else
    {
        OS_REPORT_2(OS_ERROR,
                  "v_group",0,
                  "v_group::doRegister(r=0x%x, arg=0x%x)\n"
                  "        Failed to allocate a register message.", r, arg);
    }
    return TRUE;
}

v_message
v_groupCreateInvalidMessage(
    v_kernel kernel,
    v_gid writerGID,
    c_array writerQos,
    c_time timestamp)
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
        v_gidSetNil(msg->writerInstanceGID);
        msg->sequenceNumber = 0;
        msg->transactionId = 0;
    }
    else
    {
        OS_REPORT_7(OS_ERROR,
                  "v_group", 0,
                  "v_group::CreateInvalidMessage(kernel=0x%x, v_gid={%d, %d, %d}, writerQos=0x%x, timestamp={%d, %d})\n"
                  "        Operation failed to allocate new v_message: result = NULL.",
                  kernel, writerGID.systemId, writerGID.localId, writerGID.serial,
                  writerQos, timestamp.seconds, timestamp.nanoseconds);
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
           (v_reader(v_entry(proxy->entry)->reader)->qos->userKey.enable));

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
    c_time *timestamp = (c_time *)arg;

    item = v_cacheItem(node);

    assert(v_groupCacheItem(item)->registrationCount <=
           v_dataReaderInstance(item->instance)->liveliness);

    /* Walk over all registrations, and unregister each of them. */
    groupInst = v_groupCacheItem(item)->groupInstance;
    registration = groupInst->registrations;
    while (registration)
    {
        v_instanceUnregister(item->instance, registration, *timestamp);
        registration = registration->next;
    }
    return TRUE;
}

static c_bool
instanceFree (
    c_object o,
    c_voidp arg)
{
    v_groupInstanceFree(v_groupInstance(o));
    return TRUE;
}

static v_groupAction
v_groupActionNew(
    v_groupActionKind kind,
    c_time timestamp,
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


static v_writeResult
forwardMessageToStreams(
    v_group group,
    v_message message,
    c_time t,
    v_groupActionKind actionKind)
{
    struct streamArgs args;

    assert(group != NULL);
    assert(C_TYPECHECK(group, v_group));

    args.result = V_WRITE_SUCCESS;
    if (c_count(group->streams) > 0) {
        args.action = v_groupActionNew(actionKind, t, message, group);
        c_setWalk(group->streams, (c_action)writeToStream, &args);
        c_free(args.action);
    }
    return args.result;
}

struct lifespanExpiry {
    c_time t;
    v_group group;
};

static c_bool
onSampleExpired(
    v_lifespanSample sample,
    c_voidp arg)
{
    struct lifespanExpiry* le;

    le = (struct lifespanExpiry*)arg;

    forwardMessageToStreams(le->group, v_groupSampleMessage(sample),
                            le->t, V_GROUP_ACTION_LIFESPAN_EXPIRE);
    v_groupInstanceRemove(v_groupSample(sample));

    return TRUE;
}

#define v_groupPurgeItem(_this) (C_CAST(_this,v_groupPurgeItem))

static void
_dispose_purgeList_insert(
    v_group group,
    v_groupInstance instance,
    c_time insertTime)
{
    v_groupPurgeItem purgeItem;

    if (group && instance) {
        assert(C_TYPECHECK(group,v_group));
        assert(C_TYPECHECK(instance,v_groupInstance));

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
            OS_REPORT(OS_ERROR,
                      "v_group::_dispose_purgeList_insert",0,
                      "Failed to allocate purgeItem");
            assert(FALSE);
        }
    }
}

static v_groupInstance
_dispose_purgeList_take(
    v_group group,
    c_time timestamp)
{
    v_groupPurgeItem purgeItem;
    c_equality equality;
    v_groupInstance instance;

    if (group) {
        purgeItem = group->disposedInstances;
        if (purgeItem) {
            /* This implementation assumes that the list is timely ordered.
             * The assumption is that the time when a purgeItem is inserted
             * by the operation _dispose_purgeList_insert is reflected by
             * the value of purgeItem->insertionTime.
             */
            equality = v_timeCompare(purgeItem->insertionTime,timestamp);
            if (equality == C_LT) {
                group->disposedInstances = purgeItem->next;
                purgeItem->next = NULL;
                if (group->disposedInstances == NULL) {
                    assert(group->disposedInstancesLast == purgeItem);
                    group->disposedInstancesLast = NULL;
                }
                instance = purgeItem->instance;
                purgeItem->instance = NULL;
                /* The existance of an instance in the dispose purge list is
                 * only valid if the purgeItem insertion time equals the instance
                 * epoch time. If these timestamps are not equal then the instance
                 * has been updated and should no longer be in this purge list.
                 * for performance the instance is not removed from the list but
                 * instead needs to be ignored and afterall removed at this point.
                 */
                equality = v_timeCompare(purgeItem->insertionTime,
                                         instance->epoch);
                if (equality == C_EQ) {
                    assert(v_groupInstanceStateTest(instance, L_DISPOSED));
                    group->count -= v_groupInstanceMessageCount(instance);
                    v_groupInstancePurge(instance);
                    group->count += v_groupInstanceMessageCount(instance);
                }
                c_free(purgeItem);
            } else {
                instance = NULL;
            }
        } else {
            instance = NULL;
        }
    } else {
        assert(0);
        instance = NULL;
    }
    return instance;
}

static void
_empty_purgeList_insert(
    v_group group,
    v_groupInstance instance,
    c_time insertTime)
{
    v_groupPurgeItem purgeItem;

    if (group && instance) {
        assert(C_TYPECHECK(group,v_group));
        assert(C_TYPECHECK(instance,v_groupInstance));

        assert(v_groupInstanceStateTest(instance,L_EMPTY));

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
            OS_REPORT(OS_ERROR,
                      "v_group::_empty_purgeList_insert",0,
                      "Failed to allocate purgeItem");
            assert(FALSE);
        }
    }
}

static v_groupInstance
_empty_purge_take(
    v_group group,
    c_time timestamp)
{
    v_groupPurgeItem purgeItem;
    v_groupInstance removed;
    v_groupInstance instance;
    c_equality equality;

    if (group) {
        purgeItem = group->purgeListEmpty;
        if (purgeItem) {
            /* This implementation assumes that the list is timely ordered.
             * The assumption is that the time when a purgeItem is inserted
             * by the operation _empty_purgeList_insert is reflected by
             * the value of purgeItem->insertionTime.
             */
            equality = v_timeCompare(purgeItem->insertionTime,timestamp);
            if (equality == C_LT) {
                group->purgeListEmpty = purgeItem->next;
                purgeItem->next = NULL;
                if (group->purgeListEmptyLast == purgeItem) {
                    assert(group->purgeListEmpty == NULL);
                    group->purgeListEmptyLast = NULL;
                }
                instance = purgeItem->instance;
                purgeItem->instance = NULL;
                /* The existance of an instance in the empty-nowriter purge list is
                 * only valid if the purgeItem insertion time equals the instance
                 * epoch time. If these timestamps are not equal then the instance
                 * has been updated and should no longer be in this purge list.
                 * for performance the instance is not removed from the list but
                 * instead needs to be ignored and afterall removed at this point.
                 */
                equality = v_timeCompare(purgeItem->insertionTime,
                                         instance->epoch);
                if (equality == C_EQ) {
                    removed = c_remove(group->instances,instance,NULL,NULL);
                    assert(removed != NULL);
                    v_groupInstanceFree(removed);
                }
                c_free(purgeItem);
            } else {
                instance = NULL;
            }
        } else {
            instance = NULL;
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
    c_time now)
{
    c_time delay = {5,0};
    c_time timestamp;
    v_groupInstance instance;
    struct lifespanExpiry exp;
    v_message message;

    /* Purge all instances that are expired. */
    /* Precondition is that the instances in the purge list are not alive
     * and empty */
    exp.t     = now;
    exp.group = group;

    v_lifespanAdminTakeExpired(group->lifespanAdmin, onSampleExpired, &exp);

    if (group->purgeListEmpty) {
        timestamp = c_timeSub(now, delay);
        instance = _empty_purge_take(group, timestamp);
        while (instance != NULL) {
            v_groupInstanceFree(instance);
            instance = _empty_purge_take(group, timestamp);

        }
    }

    if (group->disposedInstances) {
        delay = v_topicQosRef(group->topic)->durabilityService.service_cleanup_delay;
        timestamp = c_timeSub(now, delay);
        instance = _dispose_purgeList_take(group, timestamp);
        while (instance != NULL) {
            if (v_stateTest(instance->state, L_EMPTY | L_NOWRITERS)) {
                message = v_groupInstanceCreateMessage(instance);
                forwardMessageToStreams(group,
                                        message,
                                        now,
                                        V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE);
                c_free(message);
                _empty_purgeList_insert(group, instance, now);

            }
            v_groupInstanceFree(instance);
            instance = _dispose_purgeList_take(group, timestamp);
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

    type = c_resolve(c_getBase(e),"kernelModule::v_groupEntry");
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
                      "v_groupEntrySetAdd",0,
                      "Failed to allocate reader instance cache.");
        }
    } else {
        OS_REPORT(OS_ERROR,
                  "v_groupEntrySetAdd",0,
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

C_CLASS(disposeAllArg);
C_STRUCT(disposeAllArg) {
    v_writeResult result;
    v_message disposeMsg;
};

typedef c_bool (*v_groupEntrySetWalkAction)(v_groupEntry entry, c_voidp arg);

static c_bool
entrySetDisposeAction(v_groupEntry entry, c_voidp arg)
{
   v_dataReaderEntry reader;
   disposeAllArg a = (disposeAllArg) arg;

   reader = v_dataReaderEntry(entry->entry);
   a->result = v_dataReaderEntryDisposeAll( reader, a->disposeMsg );
   return( a->result == V_WRITE_SUCCESS );
}

static c_bool
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

static c_bool
v_groupEntrySetDisposeAll( struct v_groupEntrySet *set, c_voidp arg )
{
   return( v_groupEntrySetWalk( set, entrySetDisposeAction, arg ) );
}

static v_groupEntry
v_groupEntrySetLookupEntry(
    struct v_groupEntrySet *s,
    v_entry entry)
{
    v_groupEntry result;
    v_groupEntry proxy;

    result = NULL;
    proxy = s->firstEntry;
    while ((proxy != NULL) && (result == NULL)) {
        if (proxy->entry == entry) {
            result = proxy;
        } else {
            proxy = proxy->next;
        }
    }
    return proxy;
}

static c_char *
createInstanceKeyExpr (
    v_topic topic)
{
    c_char fieldName[16];
    c_char *keyExpr;
    c_long i,nrOfKeys,totalSize;
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

static c_time
v_groupGetPendingLastDisposeTime( v_kernel kernel,
                                  v_partition partition,
                                  v_topic topic )
{
   int i;
   v_pendingDisposeElement element;

   c_time result = C_TIME_MIN_INFINITE;

   c_mutexLock(&kernel->pendingDisposeListMutex);
   for(i=0; i<c_listCount(kernel->pendingDisposeList); i++)
   {
      element =
         (v_pendingDisposeElement)c_readAt(kernel->pendingDisposeList, i);
      if ( !strcmp( element->disposeCmd.topicExpr,
                    v_topicName(topic) )
           && !strcmp( element->disposeCmd.partitionExpr,
                       "*" ) )
      {
         result = element->disposeTimestamp;
         c_free( element->disposeCmd.topicExpr );
         c_free( element->disposeCmd.partitionExpr );
         c_removeAt( kernel->pendingDisposeList, i );
         break;
      }
   }
   c_mutexUnlock(&kernel->pendingDisposeListMutex);
   return( result );
}

static void
v_groupInit(
    v_group group,
    v_partition partition,
    v_topic topic,
    c_long id)
{
    c_type instanceType, sampleType;
    char *groupName;
    int groupNameLen;
    v_kernel kernel;
    v_topicQos qos;
    c_bool infWait;
    c_char *keyExpr;
    c_base base;
    c_type type;

    /* First initialize baseclass */
    groupNameLen = sizeof(V_GROUP_NAME_TEMPLATE) - sizeof("%s%s") +
                   1 /* \0 */ + strlen(v_partitionName(partition)) +
                   strlen(v_topicName(topic));
    groupName = os_malloc(groupNameLen);
    snprintf(groupName, groupNameLen, V_GROUP_NAME_TEMPLATE,
             v_partitionName(partition), v_topicName(topic));
    v_entityInit(v_entity(group), groupName, NULL, TRUE);
    os_free(groupName);

    /* Then initialize class itself */
    kernel = v_objectKernel(group);
    group->creationTime = v_timeGet();
    group->partition = c_keep(partition);
    group->topic = c_keep(topic);
    group->sequenceNumber = id;

    group->lifespanAdmin = v_lifespanAdminNew(kernel);

    group->purgeListEmpty = NULL;
    group->purgeListEmptyLast = NULL;

    v_groupEntrySetInit(&group->topicEntrySet);
    v_groupEntrySetInit(&group->networkEntrySet);
    v_groupEntrySetInit(&group->variantEntrySet);

    base  = c_getBase(kernel);
    type  = c_resolve(base,"kernelModule::v_groupStream");
    group->streams = c_setNew(type);

    type  = c_resolve(base, "c_string");
    assert(type);
    group->attachedServices      = c_setNew(type);
    group->notInterestedServices = c_setNew(type);

    /* TODO: Is this still necessary? And what is a pending lastDispose anyway??
//    group->lastDisposeAll = v_groupGetPendingLastDisposeTime(kernel,
//                                                             partition,
//                                                             topic);
*/
    qos = v_topicQosRef(topic);
    if (qos->durabilityService.history_kind == V_HISTORY_KEEPLAST) {
        if (qos->history.depth < 0) {
            group->depth = 0x7fffffff; /* MAX_INT */
        } else {
            group->depth = qos->durabilityService.history_depth;
        }
    } else {
        if (qos->durabilityService.max_samples_per_instance < 0) {
            group->depth = 0x7fffffff; /* MAX_INT */
        } else {
            group->depth = qos->durabilityService.max_samples_per_instance;
        }
    }
    infWait = (c_timeCompare(qos->reliability.max_blocking_time,C_TIME_INFINITE) == C_EQ);

    instanceType = createGroupInstanceType(topic);
    keyExpr = createInstanceKeyExpr(topic);
    group->instances = c_tableNew(instanceType,keyExpr);
    os_free(keyExpr);

    sampleType = createGroupSampleType(topic);

#define _COUNT_ (32)
    group->instanceExtent = c_extentNew(instanceType,_COUNT_);
    group->sampleExtent = c_extentNew(sampleType,_COUNT_*4);

    group->count = 0;
    group->infWait = infWait;
    c_free(instanceType);
    c_free(sampleType);

    group->disposedInstances = NULL;
    group->disposedInstancesLast = NULL;

    if(qos->durability.kind == V_DURABILITY_VOLATILE) {
        group->complete = TRUE;     /* no alignment necessary.*/
    } else {
        group->complete = FALSE;    /* until aligned */
    }
    c_mutexInit(&group->mutex,SHARED_MUTEX);
    c_condInit(&group->cv,&group->mutex,SHARED_COND);

    group->cachedInstance = NULL;
    group->cachedRegMsg = NULL;
    /* ES, dds1576: For each new group, determine the access mode for the
     * partition involved in this group.
     */
    group->partitionAccessMode = v_kernelPartitionAccessMode(kernel, v_partitionName(partition));
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

    v_entityFree(v_entity(group));
}

void
v_groupDeinit(
    v_group group)
{
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    c_free(group->streams);
    c_tableWalk(group->instances,instanceFree,NULL);
    c_condDestroy(&group->cv);
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
            found = c_insert(_this->attachedServices, name);
        } else {
            found = c_insert(_this->notInterestedServices, name);
        }
        c_mutexUnlock(&_this->mutex);

        if(found){
            c_free(name);
        }
    }
    return;
}

struct findServiceHelper {
    const c_char* search;
    c_bool found;
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
    }
    return !(helper->found);
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

        c_mutexLock(&_this->mutex);
        c_walk(_this->attachedServices, findService, &helper);

        if(helper.found){
            state = V_GROUP_ATTACH_STATE_ATTACHED;
        } else {
            c_walk(_this->notInterestedServices, findService, &helper);

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

    updatePurgeList(g, v_timeGet());
    /* doubly linked connection */
    if (v_entryAddGroup(e, g))
    {
       if (v_objectKind(e) == K_NETWORKREADERENTRY) {
          c_free(v_groupEntrySetAdd(&g->networkEntrySet,e));
       } else if (v_reader(v_entry(e)->reader)->qos->userKey.enable) {
          c_free(v_groupEntrySetAdd(&g->variantEntrySet,e));
       } else {
          proxy = v_groupEntrySetAdd(&g->topicEntrySet,e);
          if (proxy) {
             c_tableWalk(g->instances,registerInstance,proxy);
             c_free(proxy);
          } else {
             OS_REPORT(OS_ERROR,
                       "v_groupAddEntry",0,
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
    c_time timestamp;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entry));

    c_mutexLock(&g->mutex);

    if (v_objectKind(e) == K_NETWORKREADERENTRY) {
        proxy = v_groupEntrySetRemove(&g->networkEntrySet,e);
    } else if (v_reader(v_entry(e)->reader)->qos->userKey.enable) {
        proxy = v_groupEntrySetRemove(&g->variantEntrySet,e);
    } else {
        proxy = v_groupEntrySetRemove(&g->topicEntrySet,e);
        if (proxy != NULL) {
            timestamp = v_timeGet();
            v_groupCacheWalk(proxy->connectionCache, doUnregister, &timestamp);
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

C_STRUCT(v_groupFlushArg) {
    c_voidp         arg;
    v_group         group;
    c_action        action;
    v_entry         entry;
    v_groupInstance grInst;
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

    groupFlushArg = (v_groupFlushArg)arg;

    assert(sample != NULL);
    assert(groupFlushArg != NULL);

    entry = groupFlushArg->entry;

    assert(entry != NULL);

    message = v_groupSampleMessage(sample);
    assert(message);

    assert(v_messageStateTest(message,L_WRITE) ||
           v_messageStateTest(message,L_DISPOSED));
    /* perform action and/or process the message */
    if (groupFlushArg->action == NULL) {
        propagateTheMessage = TRUE;
    } else {
        propagateTheMessage = groupFlushArg->action(message, groupFlushArg->arg);
    }
    if (propagateTheMessage) {
        /* write the sample to other node(s) */
        v_entryWrite(entry, message, V_NETWORKID_LOCAL,NULL);
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

    message = v_groupInstanceCreateMessage(grInst);
    if (message)
    {
        message->writerGID = unregister->writerGID;
        message->qos = c_keep(unregister->qos);
        message->writeTime = unregister->writeTime;

        /* Set the nodeState of the message to UNREGISTER. */
        v_stateSet(v_nodeState(message), L_UNREGISTER);

        /* perform action and/or process the message */
        if (groupFlushArg->action == NULL)
        {
            propagateTheMessage = TRUE;
        }
        else
        {
            propagateTheMessage = groupFlushArg->action(message, groupFlushArg->arg);
        }
        if (entry && propagateTheMessage)
        {
            /* write the sample to other node(s) */
            v_entryWrite(entry, message, V_NETWORKID_LOCAL,NULL);
        }
        c_free(message);
    }
    else
    {
        OS_REPORT_2(OS_ERROR,
                  "v_group",0,
                  "v_group::doUnregisterFlush(unregister=0x%x, arg=0x%x)\n"
                  "        Failed to allocate an unregister message.",
                  unregister, arg);
    }
    return TRUE;
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

    result = v_groupInstanceWalkSamples(grInst,doFlush,arg);
    if(result == TRUE){
        groupFlushArg->grInst = grInst;
        v_groupInstanceWalkUnregisterMessages(v_groupInstance(o),
                                              doUnregisterFlush, arg);
    }
    return result;
}

static c_bool
flushAction(
    v_groupEntry proxy,
    c_voidp  arg)
{
    v_groupFlushArg groupFlushArg;

    assert(C_TYPECHECK(proxy,v_groupEntry));
    assert(arg != NULL);

    groupFlushArg = (v_groupFlushArg)arg;

    assert(C_TYPECHECK(groupFlushArg->group,v_group));

    if ((v_objectKind(proxy->entry) == K_NETWORKREADERENTRY) ||
        (v_reader(v_entry(proxy->entry)->reader)->qos->userKey.enable)) {
        groupFlushArg->entry = proxy->entry;
        c_tableWalk(groupFlushArg->group->instances,flushInstance,groupFlushArg);
    }
    return TRUE;
}

void
v_groupFlush(
    v_group g)
{
    C_STRUCT(v_groupFlushArg) groupFlushArg;

    assert(g);
    assert(C_TYPECHECK(g,v_group));
    groupFlushArg.arg = NULL;
    groupFlushArg.group = g;
    groupFlushArg.action = NULL;
    groupFlushArg.grInst = NULL;
    c_mutexLock(&g->mutex);
    v_groupEntrySetWalk(&g->networkEntrySet,flushAction,&groupFlushArg);
    c_mutexUnlock(&g->mutex);
}

void
v_groupFlushAction(
    v_group  g,
    c_action action,
    c_voidp  arg)
{
    C_STRUCT(v_groupFlushArg) groupFlushArg;

    assert(g);
    assert(action);
    assert(arg);
    assert(C_TYPECHECK(g,v_group));

    c_mutexLock(&g->mutex);
    updatePurgeList(g, v_timeGet());
    groupFlushArg.arg = arg;
    groupFlushArg.group = g;
    groupFlushArg.action = action;
    groupFlushArg.grInst = NULL;
    v_groupEntrySetWalk(&g->networkEntrySet,flushAction,&groupFlushArg);
    c_mutexUnlock(&g->mutex);
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
    v_writeResult writeResult;
    v_entry entry;
};

C_CLASS(v_entryWriteArg);

C_STRUCT(v_nwEntryWriteArg) {
    v_message message;
    v_networkId networkId;
    v_writeResult writeResult;
    v_entry entry;
};

C_CLASS(v_nwEntryWriteArg);

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
                                    &instance);

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
            v_groupCacheWalk(writeArg->instance->targetCache,
                             targetUnknown,
                             &unknownArg);
            if (unknownArg.cacheItem == NULL) {
                item = v_groupCacheItemNew(writeArg->instance,instance);
                if (item) {
                    v_groupCacheInsert(proxy->connectionCache,item);
                    v_groupCacheInsert(writeArg->instance->targetCache,item);
                    v_dataReaderInstanceRegisterSource(instance,item);
                    c_free(item);
                } else {
                    OS_REPORT(OS_ERROR,
                              "v_group::entryRegister",0,
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
                     writeArg->networkId);

        /* This method is used in a walk method and visits all network
         * interfaces. The callee expects the writeResult to indecate if
         * a reject has occured so the result will only be set if a message
         * is rejected.
         */
        if (result == V_WRITE_REJECTED) {
            writeArg->writeResult = result;
        } else if (result != V_WRITE_SUCCESS) {
            OS_REPORT_1(OS_ERROR,
                        "v_writerInstance::nwEntryWrite",0,
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
                              &instance);
    }

    /* This method is used in a walk method and visits all network
     * interfaces. The callee expects the writeResult to indecate if
     * a reject has occured so the result will only be set if a message
     * is rejected.
     */
    if (result == V_WRITE_REJECTED) {
        writeArg->writeResult = result;
    } else if (result != V_WRITE_SUCCESS) {
        OS_REPORT_1(OS_ERROR,
                    "v_group::entryWrite",0,
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
	if(!v_resourcePolicyIsUnlimited(v_reader(proxy->entry->reader)->qos->resource)){
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

    item = v_groupCacheItem(node);
    instance = v_dataReaderInstance(v_cacheItem(item)->instance);

    if (instance == NULL) {
        item->registrationCount = 0;
    }

    if (item->registrationCount != 0) {
        if (v_objectKind(v_cacheItem(item)->instance) == K_DATAREADERINSTANCE) {
            assert(item->registrationCount <= instance->liveliness);
            result = v_dataReaderInstanceWrite(instance, writeArg->message);
            if (result == V_WRITE_SUCCESS) {
                if (v_messageStateTest(writeArg->message,L_UNREGISTER)) {
                    item->registrationCount--;
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
        assert(item->registrationCount <= instance->liveliness);
    }
    if (item->registrationCount == 0) {
        writeArg->deadCacheItems = c_iterInsert(writeArg->deadCacheItems, item);
    }

    return TRUE;
}

static v_writeResult
forwardMessageToNetwork (
    v_groupInstance instance,
    v_message message,
    v_networkId writingNetworkId,
    v_entry entry)
{
    C_STRUCT(v_nwEntryWriteArg) writeArg;
    v_group group;

    assert(C_TYPECHECK(instance,v_groupInstance));

    writeArg.writeResult = V_WRITE_SUCCESS;

    if (!v_messageStateTest(message,L_REGISTER)) {
        if (writingNetworkId == V_NETWORKID_LOCAL) {
            writeArg.message     = message;
            writeArg.networkId   = writingNetworkId;
            writeArg.entry       = entry;

            group = v_groupInstanceOwner(instance);

            v_groupEntrySetWalk(&group->networkEntrySet,
                                nwEntryWrite,
                                &writeArg);
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
        v_groupEntrySetWalk(&group->topicEntrySet,
                            entryRegister,
                            &registerArg);
    }
    return registerArg.writeResult;
}

static v_message
CreateUntypedInvalidMessage(
    v_kernel kernel,
    v_message typedMsg)
{
    v_message untypedMsg;
    c_type msgType;

    /* Create a message for the invalid sample to carry. */
    msgType = v_kernelType(kernel, K_MESSAGE);
    untypedMsg = c_new(msgType);
    if (untypedMsg)
    {
        /* Set correct attributes. */
        v_node(untypedMsg)->nodeState = v_node(typedMsg)->nodeState;
        untypedMsg->writerGID = typedMsg->writerGID;
        untypedMsg->writeTime = typedMsg->writeTime;
        untypedMsg->writerInstanceGID = typedMsg->writerInstanceGID;
        untypedMsg->qos = c_keep(typedMsg->qos);
        untypedMsg->sequenceNumber = typedMsg->sequenceNumber;
        untypedMsg->transactionId = typedMsg->transactionId;
    }
    else
    {
        OS_REPORT_1(OS_ERROR,
                  "v_dataReaderInstance", 0,
                  "CreateUntypedInvalidMessage(typedMsg=0x%x)\n"
                  "        Operation failed to allocate new v_message: result = NULL.",
                  untypedMsg);
        assert(FALSE);
    }

    return untypedMsg;
}

static v_writeResult
forwardMessage (
    v_groupInstance instance,
    v_message message,
    v_networkId writingNetworkId,
    v_entry entry)
{
    C_STRUCT(v_entryWriteArg) writeArg;
    C_STRUCT(v_instanceWriteArg) instanceArg;
    v_group group;
    v_groupCacheItem item;
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

        writeArg.networkId = writingNetworkId;
        writeArg.entry     = entry;

        /* If the sample has no valid content, replace the typed sample
         * with an untyped sample to save storage space.
         */
        if (!v_messageStateTest(message,L_WRITE))
        {
            kernel = v_objectKernel(instance);
            writeArg.message = CreateUntypedInvalidMessage(kernel, message);
        }
        else
        {
            writeArg.message = message;
        }

        group = v_groupInstanceOwner(instance);

        if(!entry){
            instanceArg.message        = message;
            instanceArg.writeResult    = V_WRITE_SUCCESS;
            instanceArg.deadCacheItems = NULL;
            instanceArg.resend         = FALSE;

            /* No connection updates, so forward the messages via the cached
               instances.
             */
            v_groupCacheWalk(instance->targetCache,
                               instanceWrite,
                               &instanceArg);

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
            v_groupEntrySetWalk(&group->topicEntrySet,
                                        entryWrite,
                                        &writeArg);
        }
        if (v_messageStateTest(message,L_WRITE)) {
            v_groupEntrySetWalk(&group->variantEntrySet,
                                entryWrite,
                                &writeArg);
        }

        /* If an untyped message was being used, then release it since it will
         * no longer be needed beyond this point. (All readers have kept the
         * message when required.)
         */
        if (!v_messageStateTest(message,L_WRITE))
        {
            c_free(writeArg.message);
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

static v_writeResult
groupWrite (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId,
    c_bool stream,
    v_entry entry)
{
    v_groupInstance instance, found;
    v_writeResult result;
    v_topicQos qos;
    v_duration delay;
    c_time now;
    c_bool rejected = FALSE;
    c_bool full = FALSE;
    v_groupActionKind actionKind;
    v_message regMsg;
    c_value keyValues[32];
    c_array messageKeyList;
    c_long i, nrOfKeys;

    assert(C_TYPECHECK(group,v_group));
    assert(C_TYPECHECK(msg,v_message));

#ifdef _NAT_
    now = v_timeGet();
#else
    now = msg->allocTime;
#endif

    /* cleanup resources,
     * do not delay this call until the data is inserted,
     * it will have undesired side-effects on the just inserted data.
     */
    updatePurgeList(group, now);

    qos = v_topicQosRef(group->topic);

    if ((instancePtr == NULL) || (*instancePtr == NULL)) {
        messageKeyList = v_topicMessageKeyList(v_groupTopic(group));
        nrOfKeys = c_arraySize(messageKeyList);
        if (nrOfKeys > 32) {
            OS_REPORT_1(OS_ERROR,
                        "v_group::groupWrite",0,
                        "too many keys %d exceeds limit of 32",
                        nrOfKeys);
            return V_WRITE_PRE_NOT_MET;
        }
        for (i=0;i<nrOfKeys;i++) {
            keyValues[i] = c_fieldValue(messageKeyList[i], msg);
        }
        instance = c_tableFind(group->instances, &keyValues[0]);
        if (instance == NULL) {
            instance = v_groupInstanceNew(group, msg);
            found = c_tableInsert(group->instances, instance);
            assert(found == instance);
        }
        /* Key values have to be freed again */
        for (i=0;i<nrOfKeys;i++) {
            c_valueFreeRef(keyValues[i]);
        }
        regMsg = NULL;
        result = v_groupInstanceRegister(instance, msg, &regMsg);
        if (result == V_WRITE_REGISTERED){
            /* This flag will notify subscribers that
             * a new writer is detected. */
            if (!v_messageStateTest(msg, L_REGISTER)) {
                assert(regMsg != NULL);
                if(stream == TRUE) {
                    forwardMessageToStreams(group, regMsg, now,
                                            V_GROUP_ACTION_REGISTER);
                }
                /* Current writer writes this instance for the first time.
                 * So it is an implicit registration, which is realized by an
                 * explicit register message.
                 * So this register message must be forwarded, so readers can
                 * update liveliness counts.
                 */
                result = forwardRegisterMessage(instance, regMsg);
            }
            c_free(regMsg);
            if (!v_stateTest(instance->state, L_DISPOSED)) {
                /* The instance was maybe disposed and now is not (anymore).
                 * So to be sure that it will not be disposed reset the epoch.
                 * Note that the v_groupInstanceRegister should perform this.
                 */
                instance->epoch = C_TIME_ZERO;
            }
        }
        if ((instancePtr != NULL) && (*instancePtr == NULL)) {
            *instancePtr = c_keep(instance);
        }
        v_groupInstanceFree(instance);
    } else {
        instance = *instancePtr;
        if (v_groupInstanceOwner(instance) != group) {
            assert(FALSE);
            return V_WRITE_PRE_NOT_MET;
        }
        if (!(v_nodeState(msg) & (L_WRITE|L_DISPOSED|L_UNREGISTER|L_TRANSACTION))) {
            return V_WRITE_SUCCESS;
        }
        result = V_WRITE_SUCCESS;
    }

    V_MESSAGE_STAMP(msg,groupLookupTime);

    /* At this point the group instance is either created, resolved
     * or verified if it was provided by the callee.
     */

    assert(instance != NULL);

    if (v_messageStateTest(msg,L_UNREGISTER)) {
        assert(!v_messageStateTest(msg,L_REGISTER));
        result = v_groupInstanceUnregister(instance,msg);
        if (result != V_WRITE_UNREGISTERED) {
            /* The message has no effect on the instance state.
             * Because the writer was never seen before.
             * Therefore readers will not be effected.
             * So abort this method.
             */
            return result;
        }
    }

    /* Now forward the message to all networks. */
    result = forwardMessageToNetwork(instance,msg,writingNetworkId, entry);
    if (result == V_WRITE_REJECTED) {
        rejected = TRUE;
    }
    /* Now forward the message to all subscribers. */
    result = forwardMessage(instance,msg,writingNetworkId, entry);
    if (result == V_WRITE_REJECTED) {
        rejected = TRUE;
    }

    /* In case the message contains non volatile data then check the
     * durability resource limits and write the data into the Transient
     * (and persistent) store.
     */
    if ((v_messageQos_durabilityKind(msg->qos) != V_DURABILITY_VOLATILE) && (!entry)) {
        if (v_messageStateTest(msg,L_WRITE)) {
            actionKind = V_GROUP_ACTION_WRITE;
            if ((qos->durabilityService.max_samples != V_LENGTH_UNLIMITED) &&
                (group->count >= qos->durabilityService.max_samples)) {
                full = (group->count >= qos->durabilityService.max_samples);
            }
            if ((qos->durabilityService.history_kind == V_HISTORY_KEEPALL) &&
                (v_groupInstanceMessageCount(instance) >= group->depth) &&
                (!full)) {
                full = (v_groupInstanceMessageCount(instance) >= group->depth);
            }
        } else if (v_messageStateTest(msg,L_DISPOSED)) {
            actionKind = V_GROUP_ACTION_DISPOSE;
        } else if (v_messageStateTest(msg,L_UNREGISTER)) {
            actionKind = V_GROUP_ACTION_UNREGISTER;
        } else if (v_messageStateTest(msg,L_REGISTER)) {
            actionKind = V_GROUP_ACTION_REGISTER;
        } else {
            actionKind = V_GROUP_ACTION_WRITE;
        }
        if (!full) {
            if(!v_messageStateTest(msg, L_TRANSACTION)){
                result = v_groupInstanceInsert(instance,msg);
            }

            if (result != V_WRITE_SUCCESS_NOT_STORED)
            {
            	/* if the instance state is NOWRITERS and DISPOSED then and only
				 * then add the instance to the purge admin.
				 */
                if (v_groupInstanceStateTest(instance,L_DISPOSED) &&
                    v_groupInstanceStateTest(instance,L_NOWRITERS)) {
                    delay = qos->durabilityService.service_cleanup_delay;
                    if (v_groupInstanceStateTest(instance,L_EMPTY)) {
                        if (v_timeIsZero(delay)) {
                            actionKind = V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE;
                        } else {
                            actionKind = V_GROUP_ACTION_UNREGISTER;
                        }
                    } else {
                        if (v_timeIsZero(delay)) {
                            actionKind = V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE;
                            v_groupInstancePurge(instance);
                        } else {
                            actionKind = V_GROUP_ACTION_UNREGISTER;
                            if (!v_timeIsInfinite(delay)) {
                                _dispose_purgeList_insert(group, instance, now);
                            }
                        }
                    }
                    if (v_timeIsZero(delay)) {
                        assert(v_groupInstanceStateTest(instance,L_EMPTY));
                        _empty_purgeList_insert(group, instance, now);
                    }
				}
            	/* Only forward to stream when sample is inserted in groupInstance and
            	 * when streaming is required */
				if(stream == TRUE) {
					forwardMessageToStreams(group, msg, now, actionKind);
				}
            }else if (result == V_WRITE_SUCCESS_NOT_STORED) {
            	/* No need to distinct between insertion states from here */
				result = V_WRITE_SUCCESS;
			}
        } else {
            rejected = TRUE;
        }
        assert(instance != NULL);
    } else {
        /* The message is VOLATILE. Only if the message is an UNREGISTER message
         * we might need to add the instance to one of the 2 purgelists. As the
         * message is volatile we can ignore the WRITE and DISPOSE messages.
         */
        if ((v_messageStateTest(msg,L_UNREGISTER) &&
             v_groupInstanceStateTest(instance,L_NOWRITERS)))
        {
            if ((qos->durability.kind != V_DURABILITY_VOLATILE) &&
                v_groupInstanceStateTest(instance,L_DISPOSED))
            {
                _dispose_purgeList_insert(group, instance, now);
            } else {
                _empty_purgeList_insert(group, instance, now);
            }
        }
    }
    if (rejected) {
        result = V_WRITE_REJECTED;
    }
    return result;
}

v_writeResult
v_groupWrite (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId)
{
    v_writeResult result;

    c_mutexLock(&group->mutex);

    V_MESSAGE_STAMP(msg,groupInsertTime);
    result = groupWrite(group, msg, instancePtr, writingNetworkId, TRUE, NULL);
    c_mutexUnlock(&group->mutex);

    return result;
}

v_writeResult
v_groupWriteNoStream (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId)
{
    v_writeResult result;

    c_mutexLock(&group->mutex);
    result = groupWrite(group, msg, instancePtr, writingNetworkId, FALSE, NULL);
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

    c_mutexLock(&group->mutex);
    result = groupWrite(group, msg, instancePtr, writingNetworkId, FALSE, entry);
    c_mutexUnlock(&group->mutex);

    return result;
}


static c_bool
purgeInstanceTimedAction(
    c_object o,
    c_voidp arg)
{
    c_time *t;
    v_groupInstance instance;

    t = (c_time*)arg;
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
    c_time t)
{
    /* Do not delete historical data in builtin groups*/
    if(isBuiltinGroup(group) == FALSE){
        c_mutexLock(&group->mutex);
        c_tableWalk(group->instances, purgeInstanceTimedAction, &t);
        forwardMessageToStreams(group, NULL, t, V_GROUP_ACTION_DELETE_DATA);
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
                OS_REPORT_1(OS_ERROR,
                            "v_writerInstance::instanceResend",0,
                            "Internal error (%d) occured",
                            writeArg->writeResult);
        } else if (writeArg->writeResult == V_WRITE_SUCCESS){
            item->pendingResends--;
        }
    }
    return result;
}

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
    v_topicQos qos;
    v_writeResult result;

    assert(C_TYPECHECK(group,v_group));
    assert(C_TYPECHECK(msg,v_message));
    assert(instancePtr != NULL);
    assert(*instancePtr != NULL);
    assert((!v_messageStateTest(msg, L_REGISTER)) ||
             v_messageStateTest(msg, L_IMPLICIT));


    result = V_WRITE_SUCCESS;

    c_mutexLock(&group->mutex);
    updatePurgeList(group, v_timeGet());

    qos = v_topicQosRef(group->topic);

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
        if (v_messageQos_durabilityKind(msg->qos) == V_DURABILITY_VOLATILE) {
            *resendScope &= ~V_RESEND_DURABLE;
        } else {
            if ((qos->durabilityService.max_samples != V_LENGTH_UNLIMITED) &&
                (group->count >= qos->durabilityService.max_samples))
            {
                result = V_WRITE_REJECTED;
            } else if ((qos->durabilityService.history_kind == V_HISTORY_KEEPALL) &&
                       (v_groupInstanceMessageCount(instance) >= group->depth))
            {
                result = V_WRITE_REJECTED;
            } else {
                result = v_groupInstanceInsert(instance,msg);
                if (result == V_WRITE_SUCCESS) {
                    *resendScope &= ~V_RESEND_DURABLE;
                }
            }
        }
    }

    /* If the the resend scope includes remote interest
     * then write the message to the network interfaces.
     */
    if (*resendScope & V_RESEND_REMOTE)
    {
        C_STRUCT(v_nwEntryWriteArg) arg;

        arg.message     = msg;
        arg.networkId   = writingNetworkId;
        arg.writeResult = V_WRITE_SUCCESS;
        arg.entry       = NULL;

        v_groupEntrySetWalk(&group->networkEntrySet, nwEntryWrite, &arg);

        if (arg.writeResult == V_WRITE_SUCCESS) {
            *resendScope&= ~V_RESEND_REMOTE;
        } else if (result != V_WRITE_REJECTED) {
            result = arg.writeResult;
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

        v_groupEntrySetWalk(&group->variantEntrySet,
                            variantEntryResend,
                            &arg);

        if (arg.writeResult == V_WRITE_SUCCESS) {
            *resendScope &= ~V_RESEND_VARIANT;
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

    	v_groupCacheWalk(instance->targetCache,
                           instanceResend,
                           &arg);
        if (arg.writeResult == V_WRITE_SUCCESS) {
            *resendScope &= ~V_RESEND_TOPIC;
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

c_time
v_groupCreationTimeGet(
    v_group g)
{
    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));

    return g->creationTime;
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
    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));

    c_mutexLock(&g->mutex);
    g->complete = complete;
    c_condBroadcast(&g->cv);
    c_mutexUnlock(&g->mutex);
}

c_bool
v_groupWaitForComplete(
    v_group g,
    c_time  waitTime)
{
    c_bool status = TRUE;

    c_mutexLock(&g->mutex);
    if (g->complete == FALSE) {
        if (c_timeCompare(waitTime, C_TIME_INFINITE) != C_EQ) {
            if (c_condTimedWait(&g->cv, &g->mutex, waitTime) != SYNC_RESULT_SUCCESS) {
                status = FALSE;
            }
        } else {
            if (c_condWait(&g->cv, &g->mutex) != SYNC_RESULT_SUCCESS) {
                status = FALSE;
            }
        }
    }
    c_mutexUnlock(&g->mutex);
    return status;
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
    v_registration registration;
    v_matchIdentityAction predicate;
};

static c_bool
groupWalkInstancesUntilRegistrationFound(
    c_object obj,
    c_voidp arg)
{
    v_groupInstance grInst = v_groupInstance(obj);
    struct InstanceWalkArg *iwArg = (struct InstanceWalkArg *)arg;

    iwArg->registration = v_groupInstanceGetRegistration(grInst,
            iwArg->writerGID, iwArg->predicate);
    return (iwArg->registration == NULL);
}

/*
 * This function returns the matching registration for the specified writerGID.
 * Currently each groupInstance has its own registration instance, but in the
 * future registrations of the same writer might well become a singleton.
 * This function can then be optimized to look up the singleton registration
 * from some fort of map. For now, we just walk through all groupInstances
 * until we find one that has a registration for the specified writerGID. Then
 * we stop the walk and return that registration.
 */
v_registration
v_groupGetRegistration(
    v_group _this,
    v_gid writerGid,
    v_matchIdentityAction predicate)
{
    struct InstanceWalkArg arg;

    arg.writerGID = writerGid;
    arg.registration = NULL;
    arg.predicate = predicate;
    c_tableWalk(_this->instances, groupWalkInstancesUntilRegistrationFound, &arg);

    return arg.registration;
}

/*
 * Provide a target walk function and corresponding arg for disconnecting
 * a writer for all appropriate groupEntries.
 */
struct EntryWalkArg
{
    v_registration registration;
    v_message unregisterMsg;
    c_time timestamp;
};

static c_bool
disconnectWriterFromGroupEntry(v_groupEntry grEntry, c_voidp arg)
{
    struct EntryWalkArg *ewArg = (struct EntryWalkArg *)arg;

    return v_groupEntryApplyUnregisterMessage(grEntry, ewArg->unregisterMsg, ewArg->registration);
}

/*
 * Provide a target walk function for removing the registration from all groupInstances.
 */
static c_bool
removeRegistrationFromGroupInstance(
    c_object obj,
    c_voidp arg)
{
    v_groupInstance grInst = v_groupInstance(obj);
    struct EntryWalkArg *ewArg = (struct EntryWalkArg *)arg;

    v_groupInstanceRemoveRegistration(grInst, ewArg->registration, ewArg->timestamp);
    return TRUE;
}

/*
 * TODO: The code snippet below can be removed when v_groupUnregisterByGidTemplate
 * starts using the part in the #if 0 clause instead of in its #else clause.
 */
#if 1
struct FindMatchingGroupInstancesArg
{
    v_registration registration;
    c_iter instances;
};

static c_bool
findMatchingGroupInstances(
    c_object obj,
    c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(obj);
    struct FindMatchingGroupInstancesArg *fmiArg = (struct FindMatchingGroupInstancesArg *) arg;
    v_registration registration = fmiArg->registration;

    if (v_groupInstanceHasRegistration(instance, registration))
    {
        fmiArg->instances = c_iterInsert(fmiArg->instances, c_keep(instance));
    }

    return TRUE;
}
#endif

/*
 * This function unregisters all instances that relate through the provided
 * predicate to the GID supplied in the template.
 */
static void
v_groupUnregisterByGidTemplate(
    v_group _this,
    v_gid tmplGid,
    v_matchIdentityAction predicate,
    c_time timestamp)
{
    v_registration registration;
/*  v_kernel kernel;*/
/*  v_message unregMsg;*/

    /* Lock group. */
    c_mutexLock(&_this->mutex);

    /* Update the purgeList to make room for what is to come. */
    updatePurgeList(_this, v_timeGet());

    /* Retrieve the group-registration for this Writer. (Be aware that this
     * increases its refCount!!) Currently the registration is not a singleton,
     * so just obtain the first registration you encounter that matches the
     * provided GID template using the provided predicate. In the future
     * it will be a singleton, which makes life a lot more convenient when
     * comparing it to the contents of the registration-list of each individual
     * groupInstance. (You can then do a pointer comparison instead of a
     * comparison based on GID.)
     */
    registration = v_groupGetRegistration(_this, tmplGid, predicate);
    if (registration)
    {
/*
 * TODO: The code snippet in the #if clause below is meant to replace the code
 * snippet in #else clause, but the group and streams are not yet covered by
 * this more efficient cleanup. Therefore we still use the 'reliable old way'
 * of cleaning up by sending DISPOSE/UNREGISTER messages into the pipeline.
 */
#if 0
        struct EntryWalkArg arg;

        /* Create an unregister message for this particular registration. */
        kernel = v_objectKernel(_this);
        unregMsg = v_groupCreateInvalidMessage(kernel,
                registration->writerGID, registration->qos, timestamp);
        if (unregMsg)
        {
            /* Set the UNREGISTER flag for the message. */
            v_stateSet(v_nodeState(unregMsg), L_UNREGISTER);

            /* When the writer has autodispose enabled, set the DISPOSE flag as well. */
            if (v_messageQos_isAutoDispose(registration->qos))
            {
                /* Set the nodeState of the message to UNREGISTER. */
                v_stateSet(v_nodeState(unregMsg), L_DISPOSED);
            }

            /* Walk over all topicEntries and disconnect the writer from them. */
            arg.registration = registration;
            arg.unregisterMsg = unregMsg;
            arg.timestamp = timestamp;
            v_groupEntrySetWalk(&_this->topicEntrySet, disconnectWriterFromGroupEntry, &arg);

            /* Now move the registration for each groupInstance from the registrations
             * list to the unregistrations list. */
            c_tableWalk(_this->instances, removeRegistrationFromGroupInstance, &arg);
            c_free(unregMsg);
        }
        /* Decrease refCount of the registration again. */
        c_free(registration);
    }
    /* Unlock the group again. */
    c_mutexUnlock(&_this->mutex);
#else
        struct FindMatchingGroupInstancesArg arg;
        v_groupInstance grInst;

        arg.registration = registration;
        arg.instances = c_iterNew(NULL);
        c_tableWalk(_this->instances, findMatchingGroupInstances, &arg);

        /* Unlock the group again to be able to forward the messages into the pipeline. */
        c_mutexUnlock(&_this->mutex);

        /* Now walk over all affected instances and send cleanup messages into the pipeline. */
        grInst = v_groupInstance(c_iterTakeFirst(arg.instances));
        while (grInst != NULL)
        {
            v_groupInstancecleanup(grInst, registration, timestamp);
            c_free(grInst);
            grInst = v_groupInstance(c_iterTakeFirst(arg.instances));
        }
        /* Decrease refCount of the registration again and free the iterator. */
        c_free(registration);
        c_iterFree(arg.instances);
    }
    else
    {
        /* Unlock the group again. */
        c_mutexUnlock(&_this->mutex);
    }
#endif
}

/**
 * Unregister all instances in both the group and its readers that were
 * previously registered by the specified writer.
 */
void
v_groupDisconnectWriter(
    v_group _this,
    struct v_publicationInfo *oInfo,
    c_time timestamp)
{
    v_groupUnregisterByGidTemplate(_this, oInfo->key, v_gidCompare, timestamp);
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
 */
void
v_groupDisconnectNode(
    v_group _this,
    struct v_heartbeatInfo *missedHB)
{
    v_groupUnregisterByGidTemplate(_this, missedHB->id,
            v_systemIdCompare, missedHB->period);
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
                V_GROUP_ACTION_WRITE, msg->writeTime, msg, h->group);
    } else if(v_messageStateTest(msg, L_DISPOSED)){
        action = v_groupActionNew(
                V_GROUP_ACTION_DISPOSE, msg->writeTime, msg, h->group);
    } else if(v_messageStateTest(msg, L_REGISTER)){
        action = v_groupActionNew(
                V_GROUP_ACTION_REGISTER, msg->writeTime, msg, h->group);
    } else if(v_messageStateTest(msg, L_UNREGISTER)){
        action = v_groupActionNew(
                V_GROUP_ACTION_UNREGISTER, msg->writeTime, msg, h->group);
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
    updatePurgeList(g, v_timeGet());
    h.group = g;
    h.stream = stream;
    c_tableWalk(g->instances, streamHistoricalData, &h);
    c_mutexUnlock(&g->mutex);
}

static c_bool
writeHistoricalSample(
    v_groupSample sample,
    c_voidp arg)
{
    v_entry e;
    v_message msg;

    e = v_entry(arg);
    msg = v_groupSampleTemplate(sample)->message;

    if ((!v_stateTest(v_nodeState(msg),L_REGISTER)) &&
        (!v_stateTest(v_nodeState(msg),L_UNREGISTER))) {
        v_entryWrite(e, msg, V_NETWORKID_LOCAL, NULL);
    }

    return TRUE;
}

static c_bool
writeHistoricalData(
    c_object o,
    c_voidp arg)
{
    return v_groupInstanceWalkSamples(v_groupInstance(o),
                                      writeHistoricalSample,
                                      arg);
}

void
v_groupGetHistoricalData(
    v_group g,
    v_entry e)
{
    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(e != NULL);
    assert(C_TYPECHECK(e,v_entry));

    c_mutexLock(&g->mutex);
    updatePurgeList(g, v_timeGet());
    c_tableWalk(g->instances, writeHistoricalData, e);
    c_mutexUnlock(&g->mutex);
}

static q_expr
resolveField(
    v_group _this,
    const c_char *name)
{
    c_field field;
    c_array path;
    c_long i, length;
    q_list list;
    c_string str;
    c_type instanceType;
    c_char *fieldName;
    q_expr expr;

    instanceType = c_subType(_this->instances);
    field = c_fieldNew(instanceType,name);

    if (!field) {
        fieldName = os_alloca(strlen(name) + strlen("newest.message.userData.."));
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
        for (i=(length-1);i>=0;i--) {
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
                OS_REPORT_1(OS_ERROR,
                            "v_groupGetHistoricalDataWithCondition failed",0,
                            "field %s undefined",name);
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
            OS_REPORT_1(OS_ERROR,
                        "v_groupGetHistoricalDataWithCondition failed",0,
                        "field %s undefined",name);
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
};

static c_bool
calculateCondition(
    v_group g,
    struct historicalCondition *condition)
{
    c_bool result;
    q_expr predicate, e, subExpr, keyExpr, progExpr;
    c_long i,len;
    c_iter list;
    c_type type;
    c_array keyList;
    c_table instanceSet;
    c_value *params = NULL;
    c_long nparams;

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
                result      = TRUE;

                for (i=0;i<len && result;i++) {
                    subExpr = c_iterTakeFirst(list);
                    assert(subExpr != NULL);
                    keyList = c_tableKeyList(g->instances); /* Is this correct??*/
                    keyExpr = q_takeKey(&subExpr, keyList);
                    c_free(keyList);

                    if (keyExpr != NULL) {
                        progExpr = F1(Q_EXPR_PROGRAM,keyExpr);
                        condition->instanceQ[i] = c_queryNew(instanceSet,
                                                  progExpr,
                                                  params);
                        q_dispose(progExpr);

                        if (condition->instanceQ[i] == NULL) {
                            OS_REPORT(OS_ERROR,
                                      "calculateCondition failed",
                                      0, "error in expression");
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
                                      "calculateCondition failed",0,
                                      "error in expression");
                            result = FALSE;
                        }
                    } else {
                        condition->sampleQ[i] = NULL;
                    }
                }
                c_iterFree(list);
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
handleMatchingSample(
    v_groupSample sample,
    struct historicalCondition* condition)
{
    c_bool pass;

    if(c_timeCompare(condition->request->minSourceTimestamp,
                v_groupSampleTemplate(sample)->message->writeTime) == C_GT)
    {
        /*produced before minimum*/
        pass = FALSE;
    } else if(c_timeCompare(condition->request->maxSourceTimestamp,
                 v_groupSampleTemplate(sample)->message->writeTime) == C_LT)
    {
        /*produced after maximum*/
        pass = FALSE;
    } else {
        condition->action(sample, condition->actionArgs);
        pass = TRUE;
    }
    return pass;
}

static c_bool
checkResourceLimits(
    struct v_resourcePolicy *resourceLimits,
    c_long instances,
    c_long samples,
    c_long samplesPerInstance)
{
    c_bool proceed;

    if((resourceLimits->max_samples != -1) &&
       (resourceLimits->max_samples <= samples))
    {
        proceed = FALSE;
    } else if((resourceLimits->max_samples_per_instance != -1) &&
              (resourceLimits->max_samples_per_instance <= samplesPerInstance))
    {
         proceed = FALSE;
    } else if((resourceLimits->max_instances != -1) &&
              (resourceLimits->max_instances <= instances))
    {
        proceed = FALSE;
    }  else {
        proceed = TRUE;
    }
    return proceed;
}

static c_bool
walkMatchingSamples(
    c_object obj,
    c_voidp args)
{
    struct historicalCondition* condition;
    c_long len, i, samplesBefore;
    v_groupSample firstSample, sample;
    c_bool pass, instancePass, sampleMatch, proceed;
    v_groupInstance instance;
    v_groupFlushArg groupFlushArg;

    instance = (v_groupInstance)obj;
    condition = (struct historicalCondition*)args;
    samplesBefore = condition->insertedSamples;

    proceed = TRUE;

    if(condition->instanceQ && condition->sampleQ){
        len = c_arraySize(condition->instanceQ);
        instancePass = FALSE;

        for (i=0; (i<len) && (!instancePass) && proceed;i++) {
            if (condition->instanceQ[i]) {
                instancePass = c_queryEval(condition->instanceQ[i],instance);
            } else {
                instancePass = TRUE;
            }

            if(instancePass){ /* instance matches query*/
                /* Since history is 'replayed' here, the oldest sample should be
                 * processed first. We keep a reference to the first sample and
                 * set the current sample to the tail of the instance (oldest). */
                firstSample = v_groupInstanceHead(instance);
                sample = v_groupInstanceTail(instance);

                while ((sample != NULL) && proceed) {
                    if (sample != firstSample) {
                        v_groupInstanceSetHeadNoRefCount(instance,sample);
                    }
                    if((condition->sampleQ[i])){
                        pass = c_queryEval(condition->sampleQ[i], instance);
                    } else {
                        pass = TRUE;
                    }

                    if (sample != firstSample) {
                        v_groupInstanceSetHeadNoRefCount(instance,firstSample);
                    }
                    if(pass){
                        sampleMatch = handleMatchingSample(sample, condition);

                        if(sampleMatch){
                            condition->insertedSamples++;
                            proceed = checkResourceLimits(
                                    &condition->request->resourceLimits,
                                    condition->insertedInstances,
                                    condition->insertedSamples,
                                    condition->insertedSamples - samplesBefore);
                        }
                    }
                    sample = sample->newer;
                }
            }
        }
    } else {
        sample = v_groupInstanceTail(instance);

        while ((sample != NULL) && proceed) {
            sampleMatch = handleMatchingSample(sample, condition);

            if(sampleMatch){
                condition->insertedSamples++;

                proceed = checkResourceLimits(
                                    &condition->request->resourceLimits,
                                    condition->insertedInstances,
                                    condition->insertedSamples,
                                    condition->insertedSamples - samplesBefore);
            }
            sample = sample->newer;
        }
    }

    if(condition->insertedSamples > samplesBefore){
        condition->insertedInstances++;

        /*In case the instance matches the condition, also check whether
         * unregister messages need to be forwarded
         */
        if(condition->handleUnregistrations){
            groupFlushArg = (v_groupFlushArg)condition->actionArgs;
            groupFlushArg->grInst = instance;
            v_groupInstanceWalkUnregisterMessages(
                    instance, doUnregisterFlush, groupFlushArg);
        }

        proceed = checkResourceLimits(
                    &condition->request->resourceLimits,
                    condition->insertedInstances,
                    condition->insertedSamples,
                    condition->insertedSamples - samplesBefore);
    }
    return proceed;
}

c_bool
v_groupGetHistoricalDataWithCondition(
    v_group g,
    v_entry entry,
    v_historicalDataRequest request)
{
    c_bool result;
    struct historicalCondition condition;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(entry != NULL);
    assert(C_TYPECHECK(entry,v_entry));
    assert(C_TYPECHECK(request,v_historicalDataRequest));

    condition.instanceQ             = NULL;
    condition.sampleQ               = NULL;
    condition.request               = request;
    condition.actionArgs            = entry;
    condition.action                = (c_action)writeHistoricalSample;
    condition.insertedInstances     = 0;
    condition.insertedSamples       = 0;
    condition.handleUnregistrations = FALSE;

    result = calculateCondition(g, &condition);

    if(result){
        /*Get all matching data and send it to the reader*/
        result = c_walk(g->instances, walkMatchingSamples, &condition);
    }
    return result;

}

void
v_groupFlushActionWithCondition(
    v_group  g,
    v_historicalDataRequest request,
    c_action action,
    c_voidp  arg)
{
    c_bool result;
    struct historicalCondition condition;
    C_STRUCT(v_groupFlushArg) groupFlushArg;

    assert(g != NULL);
    assert(C_TYPECHECK(g,v_group));
    assert(C_TYPECHECK(request,v_historicalDataRequest));

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

    result = calculateCondition(g, &condition);

    if(result){
        result = c_walk(g->instances, walkMatchingSamples, &condition);
    }
    return;
}

void
v_groupUpdatePurgeList(
    v_group group)
{
    assert(group != NULL);
    assert(C_TYPECHECK(group,v_group));

    if(group){
        c_mutexLock(&group->mutex);
        updatePurgeList(group, v_timeGet());
        c_mutexUnlock(&group->mutex);
    }
}

static c_bool
disposeAll (
    c_object o,
    c_voidp arg)
{
    v_groupInstance instance = v_groupInstance(o);
    disposeAllArg a = (disposeAllArg) arg;

    a->result = v_groupInstanceDispose(instance,a->disposeMsg->writeTime);

    return TRUE;
}


v_writeResult
v_groupDisposeAll (
    v_group group,
    c_time timestamp)
{
    C_STRUCT(disposeAllArg) disposeArg;
    v_kernel kernel;
    v_message disposeMsg;
    v_gid nullGID;

    assert(C_TYPECHECK(group,v_group));

    c_mutexLock(&group->mutex);

    kernel = v_objectKernel(group);
    v_gidSetNil(nullGID);
    disposeMsg = v_groupCreateInvalidMessage(kernel, nullGID, NULL, timestamp);
    if (disposeMsg)
    {
        /* Set the nodeState of the message to DISPOSED. */
        v_stateSet(v_nodeState(disposeMsg), L_DISPOSED);

        disposeArg.result = V_WRITE_SUCCESS;
        disposeArg.disposeMsg = disposeMsg;

        c_tableWalk(group->instances, disposeAll, &disposeArg);

        if ( disposeArg.result == V_WRITE_SUCCESS )
        {
            v_groupEntrySetDisposeAll( &group->topicEntrySet, &disposeArg );
        }
        if ( disposeArg.result == V_WRITE_SUCCESS )
        {
            v_groupEntrySetDisposeAll( &group->variantEntrySet, &disposeArg );
        }

        forwardMessageToStreams(group, NULL, timestamp, V_GROUP_ACTION_DISPOSE_ALL);
        c_free(disposeMsg);
    }

    c_mutexUnlock(&group->mutex);

    return disposeArg.result;
}


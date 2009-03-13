#include "v__group.h"
#include "v_kernel.h"
#include "v__entry.h"
#include "v_domain.h"
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
#include "v__lifespanAdmin.h"
#include "v_networkReaderEntry.h"
#include "v_time.h"
#include "v_builtin.h"
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
    c_keep(foundType);

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
    c_class(instanceType)->extends = c_keep(c_class(baseType));
    if (v_topicKeyType(topic) != NULL) {
        o = c_metaDeclare(c_metaObject(instanceType),"key",M_ATTRIBUTE);
        c_property(o)->type = c_keep(v_topicKeyType(topic));
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

struct instanceUnknownArg {
    v_instance instance;
    v_groupCacheItem cacheItem;
};

static c_bool
instanceUnknown(
    v_cacheNode node,
    c_voidp arg)
{
    v_groupCacheItem item = v_groupCacheItem(node);
    struct instanceUnknownArg *a = (struct instanceUnknownArg *)arg;

    if (item->instance == a->instance) {
        a->cacheItem = item;
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
    v_instance instance;
    struct doRegisterArg *a = (struct doRegisterArg *)arg;
    v_groupCacheItem item;
    struct instanceUnknownArg unknownArg;

    assert(C_TYPECHECK(r,v_registration));

    instance = NULL;
    v_entryWrite(a->proxy->entry,r->message,V_NETWORKID_LOCAL,&instance);
    if (instance != NULL) {
        unknownArg.instance = instance;
        unknownArg.cacheItem = NULL;
        v_groupCacheWalk(a->instance->readerInstanceCache,
                         instanceUnknown, &unknownArg);
        if (unknownArg.cacheItem == NULL) {
            item = v_groupCacheItemNew(a->instance,instance);
            v_groupCacheInsert(a->proxy->readerInstanceCache,item);
            v_groupCacheInsert(a->instance->readerInstanceCache,item);
            c_free(item);
        } else {
            unknownArg.cacheItem->registrationCount++;
        }
    }
    c_free(instance);
    return TRUE;
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
    return v_groupInstanceWalkRegistrations(instance,doRegister,&regArg);
}

static c_bool
doUnregister (
    v_cacheNode node,
    c_voidp arg)
{
    v_groupCacheItem item;

    item = v_groupCacheItem(node);

    v_instanceUnregister(item->instance,item->registrationCount);
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

    args.action = v_groupActionNew(actionKind, t, message, group);
    args.result = V_WRITE_SUCCESS;
    c_setWalk(group->streams, (c_action)writeToStream, &args);
    c_free(args.action);

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

static void
updatePurgeList(
    v_group group,
    c_time now)
{
    c_time delay = {5,0};
    c_time timestamp;
    c_list purgeList;
    v_groupPurgeItem purgeItem, found;
    v_groupInstance instance;
    v_groupInstance removed;
    struct lifespanExpiry exp;
    c_long purgeCount;
    v_message message;

    /* Purge all instances that are expired. */
    /* Precondition is that the instances in the purge list are not alive
     * and empty */

    exp.t     = now;
    exp.group = group;

    v_lifespanAdminTakeExpired(group->lifespanAdmin, onSampleExpired, &exp);

    purgeItem = NULL;

    purgeList = group->purgeListEmpty;
    if (c_listCount(purgeList) > 0) {
        timestamp = c_timeSub(now, delay);
        purgeItem = c_removeAt(purgeList, 0);
        purgeCount = 0;
        while ((purgeItem != NULL) ){ 
            instance = purgeItem->instance;
            if (v_timeCompare(purgeItem->insertionTime,timestamp) == C_LT) {
                if (v_timeCompare(purgeItem->insertionTime,
                                  instance->epoch) == C_EQ) {
                    assert(v_groupInstanceStateTest(instance, L_NOWRITERS | L_EMPTY));
                    removed = c_remove(group->instances,instance,NULL,NULL);
                    assert(removed != NULL);
                    v_groupInstanceFree(instance);
                }
                c_free(purgeItem);
                purgeItem = c_removeAt(purgeList, 0);
            } else {
               /* the taken instance was not old enough yet and is
                * therefore re-inserted.
                */
               found = c_listInsert(purgeList, purgeItem);
               assert(found == purgeItem);
               c_free(purgeItem);
               purgeItem = NULL;
            }
            purgeCount++;
        }
    }

    purgeList = group->disposedInstances;
    delay = v_topicQosRef(group->topic)->durabilityService.service_cleanup_delay;
    if (c_listCount(purgeList) > 0) {
        timestamp = c_timeSub(now, delay);
        purgeItem = c_removeAt(purgeList, 0);
        purgeCount = 0;
        while ((purgeItem != NULL) ) {
            instance = purgeItem->instance;

            if (v_timeCompare(purgeItem->insertionTime,timestamp) == C_LT) {
                if (v_timeCompare(purgeItem->insertionTime, instance->epoch) == C_EQ) {
                    assert(v_groupInstanceStateTest(instance, L_DISPOSED));
                    group->count -= v_groupInstanceMessageCount(instance);
                    v_groupInstancePurge(instance);
                    group->count += v_groupInstanceMessageCount(instance);
                    if (v_stateTest(instance->state, L_EMPTY | L_NOWRITERS)) {
                        /* put in purgelist */
                        purgeItem->insertionTime = now;
                        v_groupInstanceSetEpoch(instance,
                                                purgeItem->insertionTime);
                        message = v_groupInstanceCreateMessage(instance);
                        forwardMessageToStreams(group,
                                                message,
                                                now,
                                                V_GROUP_ACTION_CLEANUP_DELAY_EXPIRE);
                        c_free(message);
                        c_append(group->purgeListEmpty, purgeItem);
                    }
                }
                c_free(purgeItem);
                purgeItem = c_removeAt(purgeList, 0);
            } else {
               /* the taken instance was not old enough yet and
                * is therefore re-inserted. */
               found = c_listInsert(purgeList, purgeItem);
               assert(found == purgeItem);
               c_free(purgeItem);
               purgeItem = NULL;
            }
            purgeCount++;
        }
    }
}
#undef MAX_PURGECOUNT

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
    proxy->entry = c_keep(e);
    proxy->sequenceNumber = set->lastSequenceNumber;
    proxy->next = set->firstEntry;
    proxy->readerInstanceCache = v_groupCacheNew(v_objectKernel(e),V_CACHE_OWNER);
    set->firstEntry = proxy;
    set->lastSequenceNumber++;

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
        v_groupCacheDeinit(entry->readerInstanceCache);
        c_free(entry);
    }
}

typedef c_bool (*v_groupEntrySetWalkAction)(v_groupEntry entry, c_voidp arg);

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
            sprintf(fieldName,"key.field%d",i);
            strcat(keyExpr,fieldName);
            if (i<(nrOfKeys-1)) { strcat(keyExpr,","); }
        }
    } else {
        keyExpr = NULL;
    }
    return keyExpr;
}

static void
v_groupInit(
    v_group group,
    v_domain partition,
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
    group->purgeListEmpty = c_listNew(v_kernelType(kernel, K_GROUPPURGEITEM));
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

    group->disposedInstances = c_listNew(v_kernelType(kernel, K_GROUPPURGEITEM));

    if(qos->durability.kind == V_DURABILITY_VOLATILE) {
        group->complete = TRUE;     /* no alignment necessary.*/
    } else {
        group->complete = FALSE;    /* until aligned */
    }
    c_mutexInit(&group->mutex,SHARED_MUTEX);
    c_condInit(&group->cv,&group->mutex,SHARED_COND);

    group->cachedInstance = NULL;
    group->cachedRegMsg = NULL;
}

v_group
v_groupNew(
    v_domain partition,
    v_topic topic,
    c_long id)
{
    v_kernel kernel;
    v_group group;

    assert(partition != NULL);
    assert(C_TYPECHECK(partition,v_domain));
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
    v_entryAddGroup(e, g);
    if (v_objectKind(e) == K_NETWORKREADERENTRY) {
        c_free(v_groupEntrySetAdd(&g->networkEntrySet,e));
    } else if (v_reader(v_entry(e)->reader)->qos->userKey.enable) {
        c_free(v_groupEntrySetAdd(&g->variantEntrySet,e));
    } else {
        proxy = v_groupEntrySetAdd(&g->topicEntrySet,e);
        c_tableWalk(g->instances,registerInstance,proxy);
        c_free(proxy);
    }
    c_mutexUnlock(&g->mutex);
}

void
v_groupRemoveEntry(
    v_group g,
    v_entry e)
{
    v_groupEntry proxy;
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
            v_groupCacheWalk(proxy->readerInstanceCache, doUnregister, NULL);
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
    c_voidp        arg;
    v_group        group;
    c_action       action;
    v_entry        entry;
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
    v_message       message;
    c_bool          propagateTheMessage;

    assert(C_TYPECHECK(unregister,v_registration));

    groupFlushArg = (v_groupFlushArg)arg;

    assert(unregister != NULL);
    assert(groupFlushArg != NULL);

    entry = groupFlushArg->entry;

    assert(entry != NULL);

    message = unregister->message;
    assert(message);

    assert(v_messageStateTest(message,L_UNREGISTER));
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
flushInstance (
    c_object o,
    c_voidp arg)
{
    c_bool result;

    result = v_groupInstanceWalkSamples(v_groupInstance(o),doFlush,arg);

    if(result == TRUE){
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
    v_groupEntrySetWalk(&g->networkEntrySet,flushAction,&groupFlushArg);
    c_mutexUnlock(&g->mutex);
}


C_STRUCT(v_entryWriteArg) {
    v_message message;
    v_groupInstance instance;
    v_networkId writingNetworkId;
    v_writeResult writeResult;
    c_iter deadCacheItems;
    v_entry entry;
};

C_CLASS(v_entryWriteArg);

static c_bool
entryRegister(
    v_groupEntry proxy,
    c_voidp arg)
{
    v_writeResult result;
    v_entryWriteArg writeArg = (v_entryWriteArg)arg;
    v_instance instance;
    v_groupCacheItem item;
    struct instanceUnknownArg unknownArg;

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
            v_groupCacheWalk(writeArg->instance->readerInstanceCache,
                             instanceUnknown,
                             &unknownArg);
            if (unknownArg.cacheItem == NULL) {
                item = v_groupCacheItemNew(writeArg->instance,instance);
                v_groupCacheInsert(proxy->readerInstanceCache,item);
                v_groupCacheInsert(writeArg->instance->readerInstanceCache,item);
                c_free(item);
            } else {
                unknownArg.cacheItem->registrationCount++;
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
    v_writeResult result;
    v_entryWriteArg writeArg = (v_entryWriteArg)arg;

    if(writeArg->entry){
        if(writeArg->entry == proxy->entry){
            result = v_networkReaderEntryWrite(v_networkReaderEntry(proxy->entry),
                                               writeArg->message,
                                               writeArg->writingNetworkId);
        } else {
            result = V_WRITE_SUCCESS;
        }
    } else {
        result = v_networkReaderEntryWrite(v_networkReaderEntry(proxy->entry),
                                           writeArg->message,
                                           writeArg->writingNetworkId);
    }
    if (result == V_WRITE_REJECTED) {
        writeArg->writeResult = result;
    } else if (result != V_WRITE_SUCCESS) {
        OS_REPORT_1(OS_ERROR,
                    "v_writerInstance::nwEntryWrite",0,
                    "Internal error (%d) occured",
                    result);
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

    if(writeArg->entry){
        if(writeArg->entry == proxy->entry){
            result = v_entryWrite(proxy->entry,
                                  writeArg->message,
                                  writeArg->writingNetworkId,
                                  &instance);
        } else {
            result = V_WRITE_SUCCESS;
        }
    } else {
        result = v_entryWrite(proxy->entry,
                              writeArg->message,
                              writeArg->writingNetworkId,
                              &instance);
    }
    if (result == V_WRITE_REJECTED) {
        writeArg->writeResult = result;
    } else if (result != V_WRITE_SUCCESS) {
        OS_REPORT_1(OS_ERROR,
                    "v_writerInstance::entryWrite",0,
                    "Internal error (%d) occured",
                    result);
    }
    c_free(instance);
    return TRUE;
}

static c_bool
instanceWrite(
    v_cacheNode node,
    c_voidp arg)
{
    v_groupCacheItem item;
    v_writeResult result = V_WRITE_SUCCESS;
    v_entryWriteArg writeArg = (v_entryWriteArg)arg;

    item = v_groupCacheItem(node);

    if (item->registrationCount == 0) {
        writeArg->deadCacheItems = c_iterInsert(writeArg->deadCacheItems, item);
    } else {
        if (v_objectKind(item->instance) == K_DATAREADERINSTANCE) {
            result = v_dataReaderInstanceWrite(v_dataReaderInstance(item->instance),
                                               writeArg->message);
        } else {
            result = V_WRITE_PRE_NOT_MET;
        }
        if (result == V_WRITE_SUCCESS) {
            if (v_messageStateTest(writeArg->message,L_UNREGISTER)) {
                item->registrationCount--;
                if (item->registrationCount == 0) {
                    writeArg->deadCacheItems =
                            c_iterInsert(writeArg->deadCacheItems, item);
                }
            }
        } else {
            writeArg->writeResult = result;
        }
    }
    return TRUE;
}

static v_writeResult
forwardMessage (
    v_groupInstance instance,
    v_message message,
    v_networkId writingNetworkId,
    v_entry entry)
{
    C_STRUCT(v_entryWriteArg) writeArg;
    v_group group;
    v_groupCacheItem item;

    assert(C_TYPECHECK(instance,v_groupInstance));

    writeArg.message          = message;
    writeArg.writingNetworkId = writingNetworkId;
    writeArg.writeResult      = V_WRITE_SUCCESS;
    writeArg.instance         = instance;
    writeArg.deadCacheItems   = NULL;
    writeArg.entry            = entry;

    group = v_groupInstanceOwner(instance);

    if (v_messageStateTest(message,L_REGISTER)) {
        /* A connection update occured, so all readers must be addressed
           to guarantee cache consistency
         */
        v_groupEntrySetWalk(&group->topicEntrySet,
                            entryRegister,
                            &writeArg);
    } else {
        if (writingNetworkId == V_NETWORKID_LOCAL) {
            v_groupEntrySetWalk(&group->networkEntrySet,
                                nwEntryWrite,
                                &writeArg);
        }
        if(!entry){
            /* No connection updates, so forward the messages via the cached
               instances.
             */
            v_groupCacheWalk(instance->readerInstanceCache,
                             instanceWrite,
                             &writeArg);
        } else {
            /*special case to handle writing to one specific entry*/
            v_groupEntrySetWalk(&group->topicEntrySet,
                                        entryWrite,
                                        &writeArg);
        }
    }
    if (v_messageStateTest(message,L_WRITE)) {
        v_groupEntrySetWalk(&group->variantEntrySet,
                            entryWrite,
                            &writeArg);
    }
    if (writeArg.deadCacheItems) {
        item = v_groupCacheItem(c_iterTakeFirst(writeArg.deadCacheItems));
        while (item != NULL) {
            v_groupCacheItemRemove(item, V_CACHE_ANY);
            item = v_groupCacheItem(c_iterTakeFirst(writeArg.deadCacheItems));
        }
        c_iterFree(writeArg.deadCacheItems);
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
    v_groupPurgeItem purgeItem;
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
        for (i=0;i<nrOfKeys;i++) {
            keyValues[i] = c_fieldValue(messageKeyList[i],msg);
        }
        instance = c_tableFind(group->instances, &keyValues[0]);
        if (instance == NULL) {
            instance = v_groupInstanceNew(group,msg);
            found = c_tableInsert(group->instances,instance);
            assert(found == instance);
        } else {
            c_keep(instance);
        }
        /* Key values have to be freed again */
        for (i=0;i<nrOfKeys;i++) {
            c_valueFreeRef(keyValues[i]);
        }

        regMsg = NULL;
        result = v_groupInstanceRegister(instance,msg, &regMsg);
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
                 * So it is an implicit registration, which is realised by an
                 * explicit register message.
                 * So this register message must be forwarded, so readers can
                 * update liveliness counts.
                 */
                result = forwardMessage(instance, regMsg, writingNetworkId, entry);
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
        if (!(v_nodeState(msg) & (L_WRITE|L_DISPOSED|L_UNREGISTER))) {
            return V_WRITE_SUCCESS;
        }
        result = V_WRITE_SUCCESS;
    }

    V_MESSAGE_STAMP(msg,groupLookupTime);

    /* At this point the group instance is either created, resolved
     * or verified if it was provided by the callee.
     * And in case of a new Instance the register flag of the message
     * is set.
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
        actionKind = V_GROUP_ACTION_WRITE;
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
        }
        if (!full) {
            result = v_groupInstanceInsert(instance,msg);
            /* if the instance state is NOWRITERS and DISPOSED then and only
             * then add the instance to the purge admin.
             */
            if (v_groupInstanceStateTest(instance,(L_NOWRITERS | L_DISPOSED))) {
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
                            purgeItem = c_new(v_kernelType(v_objectKernel(group),
                                              K_GROUPPURGEITEM));
                            purgeItem->instance = c_keep(instance);
                            purgeItem->insertionTime = now;
                            v_groupInstanceSetEpoch(instance,
                                                    purgeItem->insertionTime);
                            v_groupInstanceDisconnect(instance);
                            c_append(group->disposedInstances, purgeItem);
                            c_free(purgeItem);
                        }
                    }
                }
            }
            if(stream == TRUE) {
                forwardMessageToStreams(group, msg, now, actionKind);
            }
        } else {
            rejected = TRUE;
        }
        assert(instance != NULL);
    } else {
        if ((v_messageQos_durabilityKind(msg->qos) == V_DURABILITY_VOLATILE) &&
            (v_messageStateTest(msg,L_UNREGISTER) &&
             v_groupInstanceStateTest(instance,L_NOWRITERS))) {
            purgeItem = c_new(v_kernelType(v_objectKernel(group),
                              K_GROUPPURGEITEM));
            purgeItem->instance = c_keep(instance);
            purgeItem->insertionTime = now;
            v_groupInstanceSetEpoch(instance,
                                    purgeItem->insertionTime);
            v_groupInstanceDisconnect(instance);
            c_append(group->purgeListEmpty, purgeItem);
            c_free(purgeItem);
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
    v_writeResult result;
    v_entryWriteArg writeArg = (v_entryWriteArg)arg;

    item = v_groupCacheItem(node);

    result = v_instanceWrite(item->instance,writeArg->message);
    if (result == V_WRITE_REJECTED) {
        writeArg->writeResult = result;
    }

    if (item->registrationCount == 0) {
        writeArg->deadCacheItems = c_iterInsert(writeArg->deadCacheItems, item);
    }
    return TRUE;
}

v_writeResult
v_groupResend (
    v_group group,
    v_message msg,
    v_groupInstance *instancePtr,
    v_networkId writingNetworkId)
{
    C_STRUCT(v_entryWriteArg) writeArg;
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


    result = V_WRITE_UNDEFINED;

    c_mutexLock(&group->mutex);
    updatePurgeList(group, v_timeGet());

    qos = v_topicQosRef(group->topic);

    if ((instancePtr == NULL) || (*instancePtr == NULL)) {
        c_mutexUnlock(&group->mutex);
        return V_WRITE_ERROR;
    } else {
        instance = *instancePtr;
    }

    if (v_messageQos_durabilityKind(msg->qos) != V_DURABILITY_VOLATILE) {
        if ((qos->durabilityService.max_samples != V_LENGTH_UNLIMITED) &&
            (group->count >= qos->durabilityService.max_samples)) {
             c_mutexUnlock(&group->mutex);
             return V_WRITE_REJECTED;
        }
        if ((qos->durabilityService.history_kind == V_HISTORY_KEEPALL) &&
            (v_groupInstanceMessageCount(instance) >= group->depth)) {
             c_mutexUnlock(&group->mutex);
             return V_WRITE_REJECTED;
        }
        result = v_groupInstanceInsert(instance,msg);
        if (result != V_WRITE_SUCCESS) {
            c_mutexUnlock(&group->mutex);
            return result;
        }
    }

    writeArg.message = msg;
    writeArg.writingNetworkId = writingNetworkId;
    writeArg.writeResult = V_WRITE_SUCCESS;
    writeArg.instance = instance;
    writeArg.deadCacheItems = NULL;
    writeArg.entry = NULL;

    v_groupEntrySetWalk(&group->networkEntrySet,
                        nwEntryWrite,
                        &writeArg);

    /* Only forward the message when the WRITE bit is set!
     * So only write and writedispose messages are forwarded to readers
     * with own storage spectrum.
     */
    if (v_messageStateTest(msg,L_WRITE)) {
        v_groupEntrySetWalk(&group->variantEntrySet,entryWrite,&writeArg);
    }
    v_groupCacheWalk(instance->readerInstanceCache,
                     instanceResend,
                     &writeArg);
    item = v_groupCacheItem(c_iterTakeFirst(writeArg.deadCacheItems));
    while (item != NULL) {
        v_groupCacheItemRemove(item, V_CACHE_ANY);
        item = v_groupCacheItem(c_iterTakeFirst(writeArg.deadCacheItems));
    }
    c_iterFree(writeArg.deadCacheItems);

    c_mutexUnlock(&group->mutex);
    return writeArg.writeResult;
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

c_iter
v_groupGetRegisterMessages(
    v_group g,
    c_ulong systemId)
{
    c_iter instances;
    v_groupInstance i;
    c_iter messages;

    messages = NULL;
    /* For every registration of the identified writer, unregister the instance */
    c_mutexLock(&g->mutex);
    /*    c_tableWalk(g->instances, cleanupInstancesOfWriter, (c_voidp)&arg);*/
    /* never perform a walk! Since unregistering might remove an instance from
       the g->instances table. */
    instances = c_select(g->instances, 0);
    i = c_iterTakeFirst(instances);
    while (i != NULL) {
        v_groupInstanceGetRegisterMessages(i, systemId, &messages);
        c_free(i);
        i = c_iterTakeFirst(instances);
    }
    c_iterFree(instances);
    c_mutexUnlock(&g->mutex);

    return messages;
}

c_iter
v_groupGetRegisterMessagesOfWriter(
    v_group g,
    v_gid writerGid)
{
    c_iter instances;
    v_groupInstance i;
    c_iter messages;
    v_message regMsg;

    messages = NULL;
    /* For every registration of the identified writer, unregister the instance */
    c_mutexLock(&g->mutex);
    /*    c_tableWalk(g->instances, cleanupInstancesOfWriter, (c_voidp)&arg);*/
    /* never perform a walk! Since unregistering might remove an instance from
       the g->instances table. */
    instances = c_select(g->instances, 0);
    i = c_iterTakeFirst(instances);
    while (i != NULL) {
        regMsg = v_groupInstanceGetRegisterMessageOfWriter(i, writerGid);
        messages = c_iterInsert(messages, regMsg);
        c_free(i);
        i = c_iterTakeFirst(instances);
    }
    c_iterFree(instances);
    c_mutexUnlock(&g->mutex);

    return messages;
}

struct writeHistoricalDataHelper {
    v_group group;
    v_entry entry;
    c_bool found;
    c_bool subsKeysEnabled;
};

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
        sprintf(fieldName,"newest.%s",name);
        field = c_fieldNew(instanceType,fieldName);
        if (!field) {
            sprintf(fieldName,"newest.message.%s",name);
            field = c_fieldNew(instanceType,fieldName);
            if (!field) {
                sprintf(fieldName,"newest.message.userData.%s",name);
                field = c_fieldNew(instanceType,fieldName);
            }
        }
        os_freea(fieldName);
    }
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
                firstSample = v_groupInstanceHead(instance);
                sample = firstSample;

                while ((sample != NULL) && proceed) {
                    if (sample != firstSample) {
                        v_groupInstanceSetHead(instance,sample);
                    }
                    if((condition->sampleQ[i])){
                        pass = c_queryEval(condition->sampleQ[i], instance);
                    } else {
                        pass = TRUE;
                    }

                    if (sample != firstSample) {
                        v_groupInstanceSetHead(instance,firstSample);
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
                    sample = sample->older;
                }
            }
        }
    } else {
        sample = v_groupInstanceHead(instance);

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
            sample = sample->older;
        }
    }

    if(condition->insertedSamples > samplesBefore){
        condition->insertedInstances++;

        /*In case the instance matches the condition, also check whether
         * unregister messages need to be forwarded
         */
        if(condition->handleUnregistrations){
            v_groupInstanceWalkUnregisterMessages(
                    instance, doUnregisterFlush, condition->actionArgs);
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

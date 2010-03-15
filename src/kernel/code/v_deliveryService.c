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
#include "v_deliveryServiceEntry.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_partition.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_readerQos.h"

#include "v__deliveryService.h"
#include "v__deliveryGuard.h"
#include "v__subscriber.h"
#include "v__writer.h"
#include "v__reader.h"
#include "v__builtin.h"

#include "os_report.h"

static void
deliveryServiceSubscribe(
    void *o,
    void *arg)
{
    v_partition p = v_partition(o);
    v_deliveryServiceEntry e = v_deliveryServiceEntry(arg);
    v_kernel kernel;
    v_group g;

    assert(C_TYPECHECK(e,v_deliveryServiceEntry));
    assert(C_TYPECHECK(p,v_partition));

    kernel = v_objectKernel(e);
    g = v_groupSetCreate(kernel->groupSet,p,e->topic);

    if(v_groupPartitionAccessMode(g) == V_ACCESS_MODE_READ_WRITE ||
       v_groupPartitionAccessMode(g) == V_ACCESS_MODE_READ)
    {
        v_groupAddEntry(g,v_entry(e));
    }
    c_free(g);
}

static c_bool
subscribe(
    c_object entry,
    c_voidp partition)
{
    v_partition p = v_partition(partition);
    v_deliveryServiceEntry e = v_deliveryServiceEntry(entry);

    deliveryServiceSubscribe(p, e);

    return TRUE;
}

static void
deliveryServiceUnSubscribe(
    void *o,
    void *arg)
{
    v_partition p = v_partition(o);
    v_deliveryServiceEntry e = v_deliveryServiceEntry(arg);
    v_kernel kernel;
    v_group g;
    c_value params[2];
    c_iter list;

    assert(C_TYPECHECK(e,v_deliveryServiceEntry));
    assert(C_TYPECHECK(p,v_partition));

    params[0] = c_objectValue(p);
    params[1] = c_objectValue(e->topic);
    kernel = v_objectKernel(e);
    list = v_groupSetSelect(kernel->groupSet,
                            "partition = %0 and topic = %1",
                            params);
    while ((g = c_iterTakeFirst(list)) != NULL) {
        v_groupRemoveEntry(g,v_entry(e));
        c_free(g);
    }
    c_iterFree(list);
}

static c_bool
unsubscribe(
    c_object entry,
    c_voidp partition)
{
    v_partition p = v_partition(partition);
    v_deliveryServiceEntry e = v_deliveryServiceEntry(entry);

    deliveryServiceUnSubscribe(p, e);

    return TRUE;
}

v_deliveryService
v_deliveryServiceNew (
    v_subscriber subscriber,
    const c_char *name)
{
    v_kernel kernel;
    v_deliveryService _this;
    v_readerQos q;
    v_topic topic;
    c_base base;
    c_type type;

    assert(C_TYPECHECK(subscriber,v_subscriber));
    base = c_getBase(subscriber);
    kernel = v_objectKernel(subscriber);
    topic = v_lookupTopic (kernel, V_DELIVERYINFO_NAME);
    /* ES, dds1576: Before creating the ackreader we have to verify that read
     * access to the topic is allowed. We can accomplish this by checking the
     * access mode of the topic.
     */
    if(!topic)
    {
        OS_REPORT(OS_ERROR, "v_deliveryServiceNew",0,
                  "DeliveryService not created: "
                  "Could not locate topic with name DCPS_Delivery.");
        return NULL;
    }
    if(v_topicAccessMode(topic) == V_ACCESS_MODE_READ ||
       v_topicAccessMode(topic) == V_ACCESS_MODE_READ_WRITE)
    {
        q = v_readerQosNew(kernel,NULL);
        if (q == NULL) {
            OS_REPORT(OS_ERROR, "v_deliveryServiceNew", 0,
                      "DeliveryService not created: inconsistent qos");
            return NULL;
        }
        _this = v_deliveryService(v_objectNew(kernel,K_DELIVERYSERVICE));

        type = c_resolve(base, "kernelModule::v_deliveryGuard");
        _this->guards = c_tableNew(type, "writerGID.localId");
        c_free(type);

        type = c_resolve(base, "kernelModule::v_subscriptionInfoTemplate");
        _this->subscriptions = c_tableNew(type, "userData.key.systemId,userData.key.localId");
        c_free(type);
        v_readerInit(v_reader(_this),name, subscriber,q, NULL, TRUE);

        c_free(q);

        v_deliveryServiceEnable(_this);
    } else
    {
        OS_REPORT_1(OS_ERROR, "v_deliveryServiceNew", 0,
                    "Creation of DeliveryService <%s> failed. Topic DCPS_Delivery."
                    "does not have read access rights.", name);
        _this = NULL;
    }
    return _this;
}

v_result
v_deliveryServiceEnable(
    v_deliveryService _this)
{
#if 0
    v_kernel kernel;
    v_message builtinMsg;
#endif
    v_subscriber subscriber;
    v_result result;

    if (_this) {
        result = V_RESULT_OK;
        subscriber = v_subscriber(v_reader(_this)->subscriber);

        v_subscriberAddReader(subscriber,v_reader(_this));

#if 0
        kernel = v_objectKernel(_this);
        builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin, _this);
        v_writeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
        c_free(builtinMsg);
#endif
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

void
v_deliveryServiceFree (
    v_deliveryService _this)
{
#if 0
    v_message builtinMsg;
    v_message unregisterMsg;
#endif

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_deliveryService));

    v_readerFree(v_reader(_this));

#if 0
    /* First create message, only at the end dispose. Applications expect
     * the disposed sample to be the last!
     */
    kernel = v_objectKernel(_this);
    builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin,_this);
    unregisterMsg = v_builtinCreateSubscriptionInfo(kernel->builtin,_this);

    v_writeDisposeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
    v_unregisterBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, unregisterMsg);
    c_free(builtinMsg);
    c_free(unregisterMsg);
#endif
}

void
v_deliveryServiceDeinit (
    v_deliveryService _this)
{
    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_deliveryService));
    v_readerDeinit(v_reader(_this));
}

c_bool
v_deliveryServiceSubscribe(
    v_deliveryService _this,
    v_partition d)
{
    assert(C_TYPECHECK(_this,v_deliveryService));

    v_readerWalkEntries(v_reader(_this), subscribe, d);

    return TRUE;
}

c_bool
v_deliveryServiceUnSubscribe(
    v_deliveryService _this,
    v_partition d)
{
    assert(C_TYPECHECK(_this,v_deliveryService));

    return v_readerWalkEntries(v_reader(_this), unsubscribe, d);
}

C_STRUCT(MatchingGuardsArg) {
    v_topic topic;
    v_writer writer;
    v_deliveryGuard guard;
    c_iter groupList;
    v_gid readerGID;
    v_kernel kernel;
    c_bool alive;
};

static c_bool
updatePublication (
    v_group g,
    c_voidp arg)
{
    c_bool result = TRUE;
    C_STRUCT(MatchingGuardsArg) *a = (C_STRUCT(MatchingGuardsArg) *)arg;
    v_deliveryPublisher publication, found;
    c_type type;

    if (g) {
synch_printf("v_deliveryService::updatePublication: Entry\n");
        if (c_iterContains(a->groupList,g)) {
            type = c_subType(a->guard->publications);
            publication = c_new(type);
            publication->systemId = a->readerGID.systemId;
            publication->count = 1;
            found = c_find(a->guard->publications, publication);
            if (a->alive) {
                if (found) {
                    found->count++;
                } else {
synch_printf("v_deliveryService::updatePublication: insert new publication\n");
                    found = c_insert(a->guard->publications, publication);
                }
            } else if (found) {
                found->count--;
                if (found->count == 0) {
synch_printf("v_deliveryService::updatePublication: remove publication\n");
                    found = c_remove(a->guard->publications, publication, NULL, NULL);
                    v_deliveryGuardIgnore(a->guard,publication->systemId);
//                    assert(c_refCount(found) == 1);
                    c_free(found);
                }
            }
            c_free(publication);
            c_free(type);
                
            result = FALSE; /* stop search. */
        }
synch_printf("v_deliveryService::updatePublication: Exit\n");
    }
    return result;
}

static c_bool
updateMatchingGuards(
    c_object o,
    c_voidp arg)
{
    C_STRUCT(MatchingGuardsArg) *a = (C_STRUCT(MatchingGuardsArg) *)arg;
    c_bool result = TRUE;

synch_printf("v_deliveryService::updateMatchingGuards: Entry\n");
    /* select writers having equal topic and compatible qos policy.
     */
    a->guard = v_deliveryGuard(o);
    a->writer = v_writer(v_gidClaim(a->guard->writerGID, a->kernel));
    if (a->writer) {
        if (a->writer->topic == a->topic) {
synch_printf("v_deliveryService::updateMatchingGuards: Found matching synchronous DataWriter\n");
            v_writerGroupWalk(a->writer,updatePublication,arg);
        }
        v_gidRelease(a->guard->writerGID, a->kernel);
    } else {
    }
synch_printf("v_deliveryService::updateMatchingGuards: Exit\n");
    return result;
}

void
v_deliveryServiceRegister(
    v_deliveryService _this,
    v_message msg)
{
    C_STRUCT(MatchingGuardsArg) arg;
    v_message found;
    v_group group;
    v_topic topic;
    v_kernel kernel;
    c_long nrOfPartitions, i;
    c_iter groupList;
    c_value params[1];
    v_subscriptionInfoTemplate rInfo;

    found = c_replace(_this->subscriptions,msg,NULL,NULL);
    if (found) {
        /* reevaluate relations. */
synch_printf("v_deliveryServiceRegister: update synchronous DataReader\n");
        /* TBD */
    } else {
synch_printf("v_deliveryServiceRegister: discovered synchronous DataReader\n");
        rInfo = (v_subscriptionInfoTemplate)msg;
        /* At this point the message received anounces the
         * discovery of synchronous DataReader.
         * For each new discovery we need to look for matching
         * DataWriters and for each found DataWriter we will
         * add the DataReaders node id to the associated
         * c_deliveryGuard admin.
         */
        arg.kernel = v_objectKernel(_this);
        /* lookup topic */
        topic = v_lookupTopic(arg.kernel,rInfo->userData.topic_name);
        /* lookup all topic related groups. */
        params[0] = c_objectValue(topic);
        groupList = v_groupSetSelect(arg.kernel->groupSet, "topic = %0", params);
        /* filter out partition compliant groups. */
        nrOfPartitions = c_arraySize(rInfo->userData.partition.name);
        arg.groupList = NULL;
        group = c_iterTakeFirst(groupList);
        while (group) {
            for (i=0; i<nrOfPartitions; i++) {
                c_string name = v_entityName(group->partition);
                if (strcmp(name,rInfo->userData.partition.name[i]) == 0) {
                    arg.groupList = c_iterInsert(arg.groupList,group);
                    i = nrOfPartitions; /* exit for loop */
                }
            }
            group = c_iterTakeFirst(groupList);
        }
        c_iterFree(groupList);

        /* At this point the Topic and all Groups addressed by
         * the deiscovered DataReader are collected and we can now
         * update the systemId list of all matching DataWriters.
         */
        if (arg.groupList) {
            arg.readerGID = rInfo->userData.key;
            arg.topic = topic;
            arg.writer = NULL;
            arg.alive = TRUE;
            c_walk(_this->guards,updateMatchingGuards,&arg);
            group = c_iterTakeFirst(arg.groupList);
            while (group) {
                c_free(group);
                group = c_iterTakeFirst(arg.groupList);
            }
        }

        c_free(topic);
    }
}

void
v_deliveryServiceUnregister(
    v_deliveryService _this,
    v_message msg)
{
    C_STRUCT(MatchingGuardsArg) arg;
    v_message found;
    v_group group;
    v_topic topic;
    v_kernel kernel;
    c_long nrOfPartitions, i;
    c_iter groupList;
    c_value params[1];
    v_subscriptionInfoTemplate rInfo;

synch_printf("v_deliveryServiceUnregister: \n");
    found = c_remove(_this->subscriptions,msg,NULL,NULL);
    if (found) {
        rInfo = (v_subscriptionInfoTemplate)found;
        /* At this point the message received anounces the
         * deletion of a synchronous DataReader.
         * For each deletion we need to look for matching
         * DataWriters and for each found DataWriter we will
         * remove the DataReaders node id from the associated
         * c_deliveryGuard admin.
         */
        arg.kernel = v_objectKernel(_this);
        /* lookup topic */
        topic = v_lookupTopic(arg.kernel,rInfo->userData.topic_name);
        /* lookup all topic related groups. */
        params[0] = c_objectValue(topic);
        groupList = v_groupSetSelect(arg.kernel->groupSet, "topic = %0", params);
        /* filter out partition compliant groups. */
        nrOfPartitions = c_arraySize(rInfo->userData.partition.name);
        arg.groupList = NULL;
        group = c_iterTakeFirst(groupList);
        while (group) {
            for (i=0; i<nrOfPartitions; i++) {
                c_string name = v_entityName(group->partition);
                if (strcmp(name,rInfo->userData.partition.name[i]) == 0) {
                    arg.groupList = c_iterInsert(arg.groupList,group);
                    i = nrOfPartitions; /* exit for loop */
                }
            }
            group = c_iterTakeFirst(groupList);
        }
        c_iterFree(groupList);

        /* At this point the Topic and all Groups addressed by
         * the deleted DataReader are collected and we can now
         * update the systemId list of all matching DataWriters.
         */
        if (arg.groupList) {
            arg.readerGID = rInfo->userData.key;
            arg.topic = topic;
            arg.writer = NULL;
            arg.alive = FALSE;
            c_walk(_this->guards,updateMatchingGuards,&arg);
            group = c_iterTakeFirst(arg.groupList);
            while (group) {
                c_free(group);
                group = c_iterTakeFirst(arg.groupList);
            }
        }
        c_free(found);
    }
}

v_writeResult
v_deliveryServiceWrite(
    v_deliveryService _this,
    v_deliveryInfoTemplate msg)
{
    C_STRUCT(v_deliveryGuard) template;
    v_deliveryGuard found;
    v_writeResult result;

    template.writerGID = msg->userData.writerGID;

synch_printf("v_deliveryServiceWrite: received delivery acknowledgement for writer %d\n",template.writerGID.systemId);
    found = v_deliveryGuard(c_find(_this->guards,&template));
    if (found) {
synch_printf("v_deliveryServiceWrite: found Writer\n");
        /* The writer addressed by this delivery message exists
         * on this node so pass the delivery information to the
         * associated writer delivery admin.
         */
        v_deliveryGuardNotify(found,msg);
        c_free(found);
    }
    result = V_WRITE_SUCCESS;
    return result;
}


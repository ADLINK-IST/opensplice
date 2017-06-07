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
#include "v_deliveryServiceEntry.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_partition.h"
#include "v_entity.h"
#include "v_public.h"
#include "v_readerQos.h"
#include "v_observer.h"

#include "v__deliveryService.h"
#include "v__deliveryGuard.h"
#include "v__subscriber.h"
#include "v__writer.h"
#include "v__reader.h"
#include "v__builtin.h"
#include "v__partition.h"

#include "os_report.h"
#include "os_abstract.h"

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
    v_entry entry, found;

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
        q = v_readerQosNew(kernel, NULL);
        if (q == NULL) {
            OS_REPORT(OS_ERROR, "v_deliveryServiceNew", V_RESULT_INTERNAL_ERROR,
                      "Creation of DeliveryService <%s> failed: Cannot create reader QoS.",
                      name);
            return NULL;
        }
        _this = v_deliveryService(v_objectNew(kernel,K_DELIVERYSERVICE));

        type = c_resolve(base, "kernelModuleI::v_deliveryGuard");
        _this->guards = c_tableNew(type, "writerGID.localId");
        c_free(type);

        type = c_resolve(base, "kernelModuleI::v_subscriptionInfoTemplate");
        _this->subscriptions = c_tableNew(type, "userData.key.systemId,userData.key.localId");
        c_free(type);

        c_mutexInit(c_getBase(_this), &_this->mutex);

        q->userKey.v.enable = TRUE;
        q->userKey.v.expression = NULL;
        v_readerInit(v_reader(_this), name, subscriber, q, TRUE);
        v_entityEnable(v_entity(_this));

        c_free(q);

        entry = v_entry(v_deliveryServiceEntryNew(_this,topic));
        found = v_readerAddEntry(v_reader(_this),v_entry(entry));
        c_free(entry);
        c_free(found);

        v_deliveryServiceEnable(_this);
    } else
    {
        OS_REPORT(OS_ERROR, "v_deliveryServiceNew", V_RESULT_ILL_PARAM,
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
    v_kernel kernel;
    v_message builtinMsg, builtinCMMsg;
    v_subscriber subscriber;
    v_result result;

    if (_this) {
        result = V_RESULT_OK;
        subscriber = v_subscriber(v_reader(_this)->subscriber);

        result = v_subscriberAddReader(subscriber,v_reader(_this));
        if (result == V_RESULT_OK) {
            kernel = v_objectKernel(_this);
            builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin, v_reader(_this));
            builtinCMMsg = v_builtinCreateCMDataReaderInfo(kernel->builtin, v_reader(_this));
            v_writeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
            v_writeBuiltinTopic(kernel, V_CMDATAREADERINFO_ID, builtinCMMsg);
            c_free(builtinMsg);
            c_free(builtinCMMsg);
        }
    } else {
        result = V_RESULT_ILL_PARAM;
    }
    return result;
}

void
v_deliveryServiceFree (
    v_deliveryService _this)
{
    v_message builtinMsg, builtinCMMsg;
    v_message unregisterMsg, unregisterCMMsg;
    v_kernel kernel;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_deliveryService));

    /* First create message, only at the end dispose. Applications expect
     * the disposed sample to be the last!
     */
    kernel = v_objectKernel(_this);
    builtinMsg = v_builtinCreateSubscriptionInfo(kernel->builtin,v_reader(_this));
    builtinCMMsg = v_builtinCreateCMDataReaderInfo(kernel->builtin,v_reader(_this));
    unregisterMsg = v_builtinCreateSubscriptionInfo(kernel->builtin,v_reader(_this));
    unregisterCMMsg = v_builtinCreateCMDataReaderInfo(kernel->builtin,v_reader(_this));

    v_readerFree(v_reader(_this));

    v_writeDisposeBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, builtinMsg);
    v_writeDisposeBuiltinTopic(kernel, V_CMDATAREADERINFO_ID, builtinCMMsg);
    v_unregisterBuiltinTopic(kernel, V_SUBSCRIPTIONINFO_ID, unregisterMsg);
    v_unregisterBuiltinTopic(kernel, V_CMDATAREADERINFO_ID, unregisterCMMsg);
    c_free(builtinMsg);
    c_free(builtinCMMsg);
    c_free(unregisterMsg);
    c_free(unregisterCMMsg);
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
    v_deliveryPublisher publication;
};

static c_bool
updatePublication (
    v_group g,
    c_voidp arg)
{
    C_STRUCT(MatchingGuardsArg) *a = (C_STRUCT(MatchingGuardsArg) *)arg;

    if ((g) && (c_iterContains(a->groupList,g)))
    {
        a->publication->count++;
    }
    return TRUE;
}

static c_bool
updateMatchingGuards(
    c_object o,
    c_voidp arg)
{
    C_STRUCT(MatchingGuardsArg) *a = (C_STRUCT(MatchingGuardsArg) *)arg;
    c_bool result = TRUE;

    /* select writers having equal topic and compatible qos policy.
     */
    a->guard = v_deliveryGuard(o);
    a->writer = v_writer(v_gidClaim(a->guard->writerGID, a->kernel));
    if (a->writer) {
        if (a->writer->topic == a->topic) {
            /* if this writer has groups matching the reader groups
             * then update the guards publication list.
             */
            c_type type;
            v_deliveryPublisher publication, found;
            v_observerLock(v_observer(a->writer));
            c_mutexLock(&a->guard->mutex);

            type = c_subType(a->guard->publications);
            publication = c_new(type);
            c_free(type);
            if (publication) {
                publication->readerGID = a->readerGID;
                publication->count = 0;
                if (a->alive) {
                    found = c_replace(a->guard->publications, publication, NULL, NULL);
                    c_free(found);
                    a->publication = publication;
                    v_writerGroupWalkUnlocked(a->writer, updatePublication, arg);
                } else {
                    found = c_find(a->guard->publications, publication);
                    c_free(publication);
                    publication = found;
                    if (publication) {
                        publication->count--;
                    }
                }

            } else {
                OS_REPORT(OS_ERROR,
                          "v_deliveryService::updateMatchingGuards",V_RESULT_INTERNAL_ERROR,
                          "Failed to allocate v_deliveryPublisher object.");
                assert(FALSE);
            }
            if ((publication) && (publication->count == 0)) {
                /* The DataReader is no longer connected so there is no need
                 * to wait for acknoledgements from this DataReader.
                 * Remove DataReader from internal admin and inform active
                 * waitlists to ignore the DataReader.
                 */
                found = c_remove(a->guard->publications, publication,NULL,NULL);
                v_deliveryGuardIgnore(a->guard, publication->readerGID);
                assert(found == publication);
                c_free(found);
            }

            c_mutexUnlock(&a->guard->mutex);
            v_observerUnlock(v_observer(a->writer));
        }
        v_gidRelease(a->guard->writerGID, a->kernel);
    }
    return result;
}

/*
 * This method updates the v_deliveryGuard->publications list with all synchronous
 * writers the v_deliveryService is aware of, preconditions for to be added to the list are:
 *  a) guard->writer->topic matches the sInfo->topic
 *  b) guard->writer->partitions match the sInfo->partitions
 *
 * parameters:
 *  o - a v_subscriptionInfoTemplate instance from the v_deliveryService->subscriptions list
 *  arg - the v_deliveryGuard who's publications list is to be updated
 *
 */
static c_bool
getMatchingPublications(
    c_object o,
    c_voidp arg)
{
    v_subscriptionInfoTemplate sInfo;
    v_deliveryPublisher p, found;
    C_STRUCT(v_deliveryPublisher) template;
    c_type type;
    c_ulong nrOfPartitions, i;
    v_deliveryGuard guard;
    v_writer writer;
    v_writerGroup wGroup;
    struct v_writerGroupSet groupSet;
    c_bool result = TRUE;

    sInfo = (v_subscriptionInfoTemplate)o;
    guard = (v_deliveryGuard)arg;

    assert(guard);
    assert(C_TYPECHECK(guard,v_deliveryGuard));

    writer = v_writer(v_gidClaim(guard->writerGID, v_objectKernel(guard->owner)));

    if (writer){
        groupSet = writer->groupSet;

        wGroup = groupSet.firstGroup;
        while(wGroup){
            nrOfPartitions = c_arraySize(sInfo->userData.partition.name);
            if(strcmp(v_topicName(wGroup->group->topic), sInfo->userData.topic_name) == 0) {
                for(i=0; i<nrOfPartitions; i++){
                    if(v_partitionStringMatchesExpression(v_entity(wGroup->group->partition)->name, sInfo->userData.partition.name[i])){

                        c_mutexLock(&guard->mutex);

                        template.readerGID = sInfo->userData.key;
                        found = v_deliveryPublisher(c_find(guard->publications, &template));
                        if (found) {
                            found->count++;
                        } else {
                            type = c_subType(guard->publications);
                            p = c_new(type);
                            c_free(type);
                            if (p) {
                                p->count = 1;
                                p->readerGID = sInfo->userData.key;
                                (void) ospl_c_insert(guard->publications, p);
                            } else {
                                OS_REPORT(OS_CRITICAL,
                                          "getMatchingPublications",V_RESULT_INTERNAL_ERROR,
                                          "Failed to allocate v_deliveryPublisher object.");
                                assert(FALSE);
                                result = FALSE;
                            }
                        }

                        c_mutexUnlock(&guard->mutex);
                    }
                }
            }
            wGroup = wGroup->next;
        }
        v_gidRelease(guard->writerGID, v_objectKernel(guard->owner));
    }
    else
    {
        OS_REPORT(OS_CRITICAL,
                  "getMatchingPublications",V_RESULT_INTERNAL_ERROR,
                  "Could not claim writer; writer = NULL.");
        assert(writer);
        result = FALSE;

    }

    return result;
}


static c_bool
collectGuardsAction(
    c_object o,
    c_voidp arg)
{
    /* select writers having equal topic and compatible qos policy.
     */
    c_iterAppend((c_iter)arg, c_keep(o));

    return TRUE;
}

static c_bool
doGuardsAction(
    v_deliveryService _this,
    c_action action,
    c_voidp actionArg)
{
    c_object guard;
    c_iter guards = c_iterNew(NULL);

    c_mutexLock(&(_this->mutex));
    (void)c_walk(_this->guards, collectGuardsAction, guards);
    c_mutexUnlock(&(_this->mutex));

    guard = c_iterTakeFirst(guards);

    while(guard){
        action(guard, actionArg);
        c_free(guard);
        guard = c_iterTakeFirst(guards);
    }
    c_iterFree(guards);

    return TRUE;
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
    c_ulong nrOfPartitions, i;
    c_iter groupList;
    c_value params[1];
    v_subscriptionInfoTemplate rInfo;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_deliveryService));

    if (msg == NULL) {
        OS_REPORT(OS_WARNING,
                  "v_deliveryServiceRegister", V_RESULT_ILL_PARAM,
                  "Received illegal message: <NULL>.");
        return;
    }

    c_mutexLock(&_this->mutex);
    found = c_replace(_this->subscriptions, msg, NULL, NULL);
    c_mutexUnlock(&(_this->mutex));
    c_free(found);

    rInfo = (v_subscriptionInfoTemplate)msg;
    /* At this point the message received anounces the
     * discovery or update of a synchronous DataReader.
     * For each discovery or update we need to look for matching
     * DataWriters and for each found DataWriter the publication
     * list (the Guards list of connected DataReaders) will be updated.
     */
    arg.kernel = v_objectKernel(_this);
    /* All matching DataWriters are found by:
     * first:  find all local groups for the DataReaders Topic.
     * second: filterout all groups that are not within the DataReaders
     *         partition scope.
     * third:  visit all guards an select those that are associated to a
     *         DataWriter which writes to at least one of the remaining
     *         groups.
     */

    /* lookup all DataReader-Topic related groups. */
    topic = v_lookupTopic(arg.kernel,rInfo->userData.topic_name);
    params[0] = c_objectValue(topic);
    groupList = v_groupSetSelect(arg.kernel->groupSet, "topic = %0", params);
    /* filter out DataReader Partition incompliant groups. */
    nrOfPartitions = c_arraySize(rInfo->userData.partition.name);
    arg.groupList = NULL;
    group = c_iterTakeFirst(groupList);
    while (group) {
        for (i=0; i<nrOfPartitions; i++) {
            c_string name = v_entityName(group->partition);
            if (v_partitionStringMatchesExpression(name,rInfo->userData.partition.name[i])) {
                arg.groupList = c_iterInsert(arg.groupList,group);
                i = nrOfPartitions; /* exit for loop */
            }
        }
        group = c_iterTakeFirst(groupList);
    }
    c_iterFree(groupList);

    /* At this point all Groups addressed by the discovered DataReader
     * are collected and we can now update the readerID list of all
     * matching DataWriters.
     */
    if (arg.groupList) {
        arg.readerGID = rInfo->userData.key;
        arg.topic = topic;
        arg.writer = NULL;
        arg.alive = TRUE;

        /* Need to ensure not to update the matching guards while
         * having the delivery service locked as the path
         * to delete a writer locks the writer and then the delivery service
         * If we would update the matching guards while having the
         * delivery service locked, we would take these locks in opposite
         * order, which would lead to deadlocks. Therefore the
         * approach here is to collect the guards first while having
         * the lock, followed by updating the collected guards without
         * having that lock.
         */
        doGuardsAction(_this, updateMatchingGuards, &arg);

        group = c_iterTakeFirst(arg.groupList);
        while (group) {
            c_free(group);
            group = c_iterTakeFirst(arg.groupList);
        }
        c_iterFree(arg.groupList);
    }

    c_free(topic);
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
    c_ulong nrOfPartitions, i;
    c_iter groupList;
    c_value params[1];
    v_subscriptionInfoTemplate rInfo;

    assert(_this != NULL);
    assert(C_TYPECHECK(_this,v_deliveryService));

    if (msg == NULL) {
        OS_REPORT(OS_WARNING,
                  "v_deliveryServiceUnregister", V_RESULT_ILL_PARAM,
                  "Received illegal message: <NULL>.");
        return;
    }

    c_mutexLock(&_this->mutex);
    found = c_remove(_this->subscriptions, msg, NULL, NULL);
    c_mutexUnlock(&(_this->mutex));

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
                if (v_partitionStringMatchesExpression(name,rInfo->userData.partition.name[i])) {
                    arg.groupList = c_iterInsert(arg.groupList,c_keep(group));
                    i = nrOfPartitions; /* exit for loop */
                }
            }
            c_free(group);
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

            /* Need to ensure not to update the matching guards while
             * having the delivery service locked as the path
             * to delete a writer locks the writer and then the delivery service
             * If we would update the matching guards while having the
             * delivery service locked, we would take these locks in opposite
             * order, which would lead to deadlocks. Therefore the
             * approach here is to collect the guards first while having
             * the lock, followed by updating the collected guards without
             * having that lock.
             */
            doGuardsAction(_this, updateMatchingGuards, &arg);

            group = c_iterTakeFirst(arg.groupList);
            while (group) {
                c_free(group);
                group = c_iterTakeFirst(arg.groupList);
            }
            c_iterFree(arg.groupList);
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
    v_deliveryGuard guard;
    v_result result;

    template.writerGID = msg->userData.writerGID;
    guard = v_deliveryServiceLookupGuard(_this, &template);
    if (guard) {
        /* The writer addressed by this delivery message exists
         * on this node so pass the delivery information to the
         * associated writer delivery admin.
         */
        result = v_deliveryGuardNotify(guard, msg);
        if (result != V_RESULT_OK) {
            OS_REPORT(OS_ERROR,
                "v_deliveryServiceWrite",result,
                "Failed to notify delivery guards");
        }
        c_free(guard);
    }
    return V_WRITE_SUCCESS;
}

v_writeResult
v_deliveryServiceAckMessage (
    v_deliveryService _this,
    v_message message,
    v_gid gid)
{
    struct v_deliveryInfo *info;
    v_message msg;
    v_topic topic;
    v_kernel kernel;

    if ((_this != NULL) && (v_stateTest(v_nodeState(message),L_SYNCHRONOUS))) {
        kernel = v_objectKernel(_this);
        topic = v_builtinTopicLookup(kernel->builtin, V_DELIVERYINFO_ID);
        if (topic) {
            msg = v_topicMessageNew(topic);
            if (msg != NULL) {
                info = (struct v_deliveryInfo *) (msg + 1);
                info->writerGID = message->writerGID;
                info->readerGID = gid;
                info->sequenceNumber = message->sequenceNumber;
                v_writeBuiltinTopic(kernel,V_DELIVERYINFO_ID,msg);
                c_free(msg);
            } else {
                OS_REPORT(OS_WARNING,
                          "v_deliveryServiceAckMessage", V_RESULT_INTERNAL_ERROR,
                          "Failed to produce built-in delivery message");
            }
        } else {
            /* Valid use-case (topic == NULL) */
        }
    }
    return V_WRITE_SUCCESS;
}

v_result
v_deliveryServiceRemoveGuard(
    v_deliveryService _this,
    v_deliveryGuard guard)
{
    v_result result = V_RESULT_UNDEFINED;
    v_deliveryGuard found;

    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryService));
    assert(guard);
    assert(C_TYPECHECK(guard,v_deliveryGuard));

    c_mutexLock(&(_this->mutex));
    found = c_remove(_this->guards,guard,NULL,NULL);
    if (found != guard) {
        /* This should not happen! */
        result = V_RESULT_INTERNAL_ERROR;
        OS_REPORT(OS_ERROR,
                  "v_deliveryGuardFree",result,
                  "Detected inconsistent guard-list in delivery service; found = 0x%"PA_PRIxADDR", guard = 0x%"PA_PRIxADDR".",
                  (os_address)found, (os_address)guard);
    } else {
        result = V_RESULT_OK;
    }
    c_mutexUnlock(&(_this->mutex));
    return result;
}

v_result
v_deliveryServiceAddGuard(
    v_deliveryService _this,
    v_deliveryGuard guard)
{
    v_deliveryGuard found;

    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryService));
    assert(guard);
    assert(C_TYPECHECK(guard,v_deliveryGuard));

    c_mutexLock(&_this->mutex);

    found = c_tableInsert(_this->guards, guard);
    assert(found == guard);
    (void)found;

    /* Now update publication list from delivery service subscriptions. */
    c_walk(_this->subscriptions, getMatchingPublications, guard);

    c_mutexUnlock(&(_this->mutex));

    return V_RESULT_OK;
}

v_deliveryGuard
v_deliveryServiceLookupGuard(
    v_deliveryService _this,
    v_deliveryGuard template)
{
    v_deliveryGuard found;

    assert(_this);
    assert(C_TYPECHECK(_this,v_deliveryService));

    c_mutexLock(&_this->mutex);
    found = c_find(_this->guards, template);
    c_mutexUnlock(&(_this->mutex));

    return found;
}

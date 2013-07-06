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
#include "d__groupCreationQueue.h"
#include "d_groupCreationQueue.h"
#include "d_lock.h"
#include "d_group.h"
#include "d_admin.h"
#include "d_configuration.h"
#include "d_misc.h"
#include "u_group.h"
#include "u_entity.h"
#include "v_builtin.h"
#include "c_iterator.h"
#include "os_heap.h"
#include "os_time.h"
#include "os_thread.h"

static void
d_waitForCompletenessDCPSTopic(
    d_groupCreationQueue queue)
{
    d_group dcpstopicGroup;
    d_completeness completeness;
    d_admin admin;
    os_time sleepTime;
    d_durability durability;

    sleepTime.tv_sec  = 1;
    sleepTime.tv_nsec = 0;
    admin             = queue->admin;
    durability        = d_adminGetDurability(admin);

    d_printTimedEvent(durability,
                            D_LEVEL_FINE,
                            D_THREAD_GROUP_CREATION,
                            "Waiting for group '%s.%s' to be created.\n",
                            V_BUILTIN_PARTITION, V_TOPICINFO_NAME);

    do{
        dcpstopicGroup    = d_adminGetLocalGroup(admin,
                                    V_BUILTIN_PARTITION, V_TOPICINFO_NAME,
                                    D_DURABILITY_TRANSIENT);
        if((dcpstopicGroup == NULL) && (queue->terminate == FALSE)){
            os_nanoSleep(sleepTime);
        }
    } while((dcpstopicGroup == NULL) && (queue->terminate == FALSE));

    if(queue->terminate == FALSE){
        d_printTimedEvent(durability,
                                D_LEVEL_FINE,
                                D_THREAD_GROUP_CREATION,
                                "Group '%s.%s' is available. Waiting for completeness...\n",
                                V_BUILTIN_PARTITION, V_TOPICINFO_NAME);
    }

    do{
        if(dcpstopicGroup){
            completeness = d_groupGetCompleteness(dcpstopicGroup);

            if((completeness != D_GROUP_COMPLETE) && (queue->terminate == FALSE)){
                os_nanoSleep(sleepTime);
            }
        } else {
            completeness = D_GROUP_KNOWLEDGE_UNDEFINED;
        }
    } while((completeness != D_GROUP_COMPLETE) && (queue->terminate == FALSE));

    if(queue->terminate == FALSE){
        d_printTimedEvent(durability,
                                D_LEVEL_FINE,
                                D_THREAD_GROUP_CREATION,
                                "Group '%s.%s' is complete now.\n",
                                V_BUILTIN_PARTITION, V_TOPICINFO_NAME);
    } else {
        d_printTimedEvent(durability,
                                D_LEVEL_FINE,
                                D_THREAD_GROUP_CREATION,
                                "Not waiting for group '%s.%s', because termination is in progress.\n",
                                V_BUILTIN_PARTITION, V_TOPICINFO_NAME);
    }
    return;
}

static void*
d_groupCreationQueueRun(
    void* userData)
{
    d_groupCreationQueue queue;
    c_iter groupsToCreate, reinsert;
    d_group group, localGroup;
    d_durability durability;
    d_durabilityKind kind;
    u_participant uservice;
    c_char *partition, *topic;
    v_duration duration;
    u_group ugroup;
    c_long creationCountVolatile, creationCountTransient, creationCountPersistent;
    c_ulong maxBurst;
    os_time sleepTime, maxBurstSleepTime;
    c_bool update;

    sleepTime.tv_sec          = 1;
    sleepTime.tv_nsec         = 0;
    duration.seconds          = 0;
    duration.nanoseconds      = 5000000; /*5ms*/
    creationCountVolatile     = 0;
    creationCountTransient    = 0;
    creationCountPersistent   = 0;
    maxBurstSleepTime.tv_sec  = 0;
    maxBurstSleepTime.tv_nsec = 10000000; /*10ms*/

    queue                = d_groupCreationQueue(userData);
    groupsToCreate       = c_iterNew(NULL);
    reinsert             = c_iterNew(NULL);
    durability           = d_adminGetDurability(queue->admin);
    uservice             = u_participant(d_durabilityGetService(durability));

    d_waitForCompletenessDCPSTopic(queue);

    while(queue->terminate == FALSE) {
        d_lockLock(d_lock(queue));

        queue->groupsToCreateVolatile -= creationCountVolatile;
        assert(queue->groupsToCreateVolatile >= 0);
        queue->groupsToCreateTransient -= creationCountTransient;
        assert(queue->groupsToCreateTransient >= 0);
        queue->groupsToCreatePersistent -= creationCountPersistent;
        assert(queue->groupsToCreatePersistent >= 0);

        group = d_group(c_iterTakeFirst(queue->groups));

        while(group){
            groupsToCreate = c_iterInsert(groupsToCreate, group);
            group = d_group(c_iterTakeFirst(queue->groups));
        }
        assert((queue->groupsToCreateVolatile +
                queue->groupsToCreateTransient +
                queue->groupsToCreatePersistent) ==
               c_iterLength(groupsToCreate));

        durability = d_adminGetDurability(queue->admin);
        d_durabilityUpdateStatistics(durability, d_statisticsUpdateGroupsToCreate, queue);

        d_lockUnlock(d_lock(queue));

        creationCountVolatile   = 0;
        creationCountTransient  = 0;
        creationCountPersistent = 0;
        maxBurst                = 10;

        group = c_iterTakeFirst(groupsToCreate);

        while(group && (queue->terminate == FALSE)){
            partition  = d_groupGetPartition(group);
            topic      = d_groupGetTopic(group);
            kind       = d_groupGetKind(group);
            localGroup = d_adminGetLocalGroup(queue->admin, partition, topic, kind);
            update     = FALSE;

            if(localGroup) {
                d_printTimedEvent(durability,
                            D_LEVEL_FINE,
                            D_THREAD_GROUP_CREATION,
                            "Remote group %s.%s has already been created locally.\n",
                            partition, topic);
                update = TRUE;
            } else {
                ugroup = u_groupNew(uservice, partition, topic, duration);

                if(ugroup){
                    d_printTimedEvent(durability,
                                    D_LEVEL_FINE,
                                    D_THREAD_GROUP_CREATION,
                                    "Remote group %s.%s created locally.\n",
                                    partition, topic);
                    update = TRUE;
                    u_entityFree(u_entity(ugroup));
                    maxBurst--;

                    if(maxBurst == 0){
                        os_nanoSleep(maxBurstSleepTime);
                        maxBurst = 10;
                    }
                } else {
                    maxBurst++;
                    d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_GROUP_CREATION,
                                "Remote group %s.%s could not be created locally.\n",
                                partition, topic);

                    if(d_durabilityGetState(durability) == D_STATE_COMPLETE){
                        d_printTimedEvent(durability, D_LEVEL_FINE,
                                D_THREAD_GROUP_CREATION,
                                "I am complete so it will not be available anymore.\n");
                        update = TRUE;
                    } else if(d_adminGetFellowCount(queue->admin) == 0){
                        d_printTimedEvent(durability, D_LEVEL_WARNING,
                                D_THREAD_GROUP_CREATION,
                                "No fellows available to provide me with group information. " \
                                "Ignoring group.\n",
                                partition, topic);
                        update = TRUE;
                    } else {
                        reinsert = c_iterInsert(reinsert, group);
                    }
                }
            }
            if(update){
                switch(d_groupGetKind(group)){
                case D_DURABILITY_VOLATILE:
                    creationCountVolatile++;
                    break;
                case D_DURABILITY_TRANSIENT:
                case D_DURABILITY_TRANSIENT_LOCAL:
                    creationCountTransient++;
                    break;
                case D_DURABILITY_PERSISTENT:
                    creationCountPersistent++;
                    break;
                default:
                    assert(FALSE);
                    break;
                }
                d_groupFree(group);
            }
            os_free(partition);
            os_free(topic);
            group = c_iterTakeFirst(groupsToCreate);
        }
        group = d_group(c_iterTakeFirst(reinsert));

        while(group){
            groupsToCreate = c_iterInsert(groupsToCreate, group);
            group = d_group(c_iterTakeFirst(reinsert));
        }
        if(queue->terminate == FALSE){
            os_nanoSleep(sleepTime);
        }
    }

    group = d_group(c_iterTakeFirst(groupsToCreate));

    while(group) {
        d_groupFree(group);
        group = d_group(c_iterTakeFirst(groupsToCreate));
    }
    c_iterFree(groupsToCreate);
    c_iterFree(reinsert);

    d_lockLock(d_lock(queue));
    group = d_group(c_iterTakeFirst(queue->groups));

    while(group) {
        d_groupFree(group);
        group = d_group(c_iterTakeFirst(queue->groups));
    }
    d_lockUnlock(d_lock(queue));

    return NULL;
}

static c_equality
compareGroups(
    d_group group,
    d_group compareGroup)
{
    c_equality result;
    c_char *partition, *partition2, *topic, *topic2;
    c_long comp;

    assert(group);
    assert(compareGroup);

    if(group == compareGroup){
        result = C_EQ;
    } else {
        partition = d_groupGetPartition(group);
        partition2 = d_groupGetPartition(compareGroup);
        comp = strcmp(partition, partition2);

        if(comp == 0){
            topic = d_groupGetTopic(group);
            topic2 = d_groupGetTopic(compareGroup);
            comp = strcmp(topic, topic2);

            if(comp == 0){
                result = C_EQ;
            }  else if(comp > 0){
                result = C_GT;
            } else {
                result = C_LT;
            }
            os_free(topic);
            os_free(topic2);
        } else if(comp > 0){
            result = C_GT;
        } else {
            result = C_LT;
        }
        os_free(partition);
        os_free(partition2);
    }
    return result;
}

d_groupCreationQueue
d_groupCreationQueueNew(
    d_admin admin)
{
    d_groupCreationQueue queue;
    os_result osr;
    os_threadAttr attr;

    assert(admin);

    queue = d_groupCreationQueue(os_malloc(C_SIZEOF(d_groupCreationQueue)));

    if(queue) {
        d_lockInit(d_lock(queue), D_GROUP_CREATION_QUEUE, d_groupCreationQueueDeinit);

        if(queue) {
            queue->groups                   = c_iterNew(NULL);
            queue->terminate                = FALSE;
            queue->admin                    = admin;
            queue->groupsToCreateVolatile   = 0;
            queue->groupsToCreateTransient  = 0;
            queue->groupsToCreatePersistent = 0;

            osr                             = os_threadAttrInit(&attr);

            if(osr == os_resultSuccess) {
                osr = os_threadCreate(&queue->actionThread, "groupCreationThread",
                                      &attr, (void*(*)(void*))d_groupCreationQueueRun,
                                      (void*)queue);

                if(osr != os_resultSuccess) {
                    d_groupCreationQueueFree(queue);
                    queue = NULL;
                }
            } else {
                d_groupCreationQueueFree(queue);
                queue = NULL;
            }
        }
    }
    return queue;
}

void
d_groupCreationQueueDeinit(
    d_object object)
{
    d_groupCreationQueue queue;

    assert(d_objectIsValid(object, D_GROUP_CREATION_QUEUE) == TRUE);

    if(object) {
        queue = d_groupCreationQueue(object);

        if(os_threadIdToInteger(queue->actionThread)) {
            queue->terminate = TRUE;
            os_threadWaitExit(queue->actionThread, NULL);
        }
        if(queue->groups){
            c_iterFree(queue->groups);
        }
    }
}

void
d_groupCreationQueueFree(
    d_groupCreationQueue queue)
{
    assert(d_objectIsValid(d_object(queue), D_GROUP_CREATION_QUEUE) == TRUE);

    if(queue) {
        d_lockFree(d_lock(queue), D_GROUP_CREATION_QUEUE);
    }
    return;
}

c_bool
d_groupCreationQueueAdd(
    d_groupCreationQueue queue,
    d_group group)
{
    c_bool result;
    d_group found;
    d_durability durability;
    assert(d_objectIsValid(d_object(queue), D_GROUP_CREATION_QUEUE) == TRUE);

    result = FALSE;

    if(queue) {
        d_lockLock(d_lock(queue));

        found = c_iterResolve(queue->groups, (c_iterResolveCompare)compareGroups, group);

        if(found == NULL){
            queue->groups = c_iterInsert(queue->groups, group);

            switch(d_groupGetKind(group)){
            case D_DURABILITY_VOLATILE:
                queue->groupsToCreateVolatile++;
                break;
            case D_DURABILITY_TRANSIENT:
            case D_DURABILITY_TRANSIENT_LOCAL:
                queue->groupsToCreateTransient++;
                break;
            case D_DURABILITY_PERSISTENT:
                queue->groupsToCreatePersistent++;
                break;
            default:
                assert(FALSE);
                break;
            }
            durability = d_adminGetDurability(queue->admin);
            d_durabilityUpdateStatistics(durability, d_statisticsUpdateGroupsToCreate, queue);
            result = TRUE;
        }
        d_lockUnlock(d_lock(queue));

    }
    return result;
}

c_bool
d_groupCreationQueueIsEmpty(
    d_groupCreationQueue queue)
{
    c_bool result;

    assert(d_objectIsValid(d_object(queue), D_GROUP_CREATION_QUEUE) == TRUE);

    d_lockLock(d_lock(queue));

    if((queue->groupsToCreateVolatile == 0) &&
       (queue->groupsToCreateTransient == 0) &&
       (queue->groupsToCreatePersistent == 0)){
        result = TRUE;
    } else {
        result = FALSE;
    }
    d_lockUnlock(d_lock(queue));

    return result;
}

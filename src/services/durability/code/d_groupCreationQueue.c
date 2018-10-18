/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "d__groupCreationQueue.h"
#include "d__lock.h"
#include "d__admin.h"
#include "d__configuration.h"
#include "d__misc.h"
#include "d__conflictResolver.h"
#include "d__group.h"
#include "d__thread.h"
#include "d__group.h"
#include "d__durability.h"
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
    d_thread self = d_threadLookupSelf ();
    d_group dcpstopicGroup;
    d_completeness completeness;
    d_admin admin;
    os_duration sleepTime = OS_DURATION_INIT(1, 0);
    d_durability durability;

    admin             = queue->admin;
    durability        = d_adminGetDurability(admin);

    d_printTimedEvent(durability, D_LEVEL_FINEST,
         "Waiting for group '%s.%s' to be created.\n",V_BUILTIN_PARTITION, V_TOPICINFO_NAME);

    do {
        dcpstopicGroup    = d_adminGetLocalGroup(admin,
                                    V_BUILTIN_PARTITION, V_TOPICINFO_NAME,
                                    D_DURABILITY_TRANSIENT);
        if((dcpstopicGroup == NULL) && (queue->terminate == FALSE)){
            d_sleep(self, sleepTime);
        }
    } while((dcpstopicGroup == NULL) && (queue->terminate == FALSE));

    if(queue->terminate == FALSE){
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Group '%s.%s' is available. Waiting for completeness...\n", V_BUILTIN_PARTITION, V_TOPICINFO_NAME);
    }

    do{
        if(dcpstopicGroup){
            completeness = d_groupGetCompleteness(dcpstopicGroup);

            if((completeness != D_GROUP_COMPLETE) && (queue->terminate == FALSE)){
                d_sleep(self, sleepTime);
            }
        } else {
            completeness = D_GROUP_KNOWLEDGE_UNDEFINED;
        }
    } while((completeness != D_GROUP_COMPLETE) && (queue->terminate == FALSE));

    if (queue->terminate == FALSE) {
        d_printTimedEvent(durability, D_LEVEL_FINER,
            "Group '%s.%s' is complete now.\n", V_BUILTIN_PARTITION, V_TOPICINFO_NAME);
    } else {
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Not waiting for group '%s.%s', because termination is in progress.\n",
            V_BUILTIN_PARTITION, V_TOPICINFO_NAME);
    }
    return;
}

static void*
d_groupCreationQueueRun(
    void* userData)
{
    d_thread self = d_threadLookupSelf ();
    d_groupCreationQueue queue;
    c_iter groupsToCreate, reinsert;
    d_group group, localGroup;
    d_durability durability;
    d_durabilityKind kind;
    u_participant uservice;
    c_char *partition, *topic;
    u_group ugroup;
    c_ulong creationCountVolatile, creationCountTransient, creationCountPersistent;
    c_ulong maxBurst;
    os_duration sleepTime = OS_DURATION_INIT(1,0);
    os_duration maxBurstSleepTime = OS_DURATION_INIT(0, 10000000);  /* 10 ms */
    c_bool update;
    c_ulong nrOfPendingConflicts;

    creationCountVolatile     = 0;
    creationCountTransient    = 0;
    creationCountPersistent   = 0;

    queue                = d_groupCreationQueue(userData);
    groupsToCreate       = c_iterNew(NULL);
    reinsert             = c_iterNew(NULL);
    durability           = d_adminGetDurability(queue->admin);
    uservice             = u_participant(d_durabilityGetService(durability));

    d_waitForCompletenessDCPSTopic(queue);

    while(queue->terminate == FALSE) {
        d_lockLock(d_lock(queue));

        assert(queue->groupsToCreateVolatile >= creationCountVolatile);
        queue->groupsToCreateVolatile -= creationCountVolatile;
        assert(queue->groupsToCreateTransient >= creationCountTransient);
        queue->groupsToCreateTransient -= creationCountTransient;
        assert(queue->groupsToCreatePersistent >= creationCountPersistent);
        queue->groupsToCreatePersistent -= creationCountPersistent;

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
                d_printTimedEvent(durability, D_LEVEL_FINEST,
                            "Remote group %s.%s has already been created locally.\n",
                            partition, topic);
                update = TRUE;
            } else {
                ugroup = u_groupNew(uservice, partition, topic, 0);

                if(ugroup){
                    d_printTimedEvent(durability, D_LEVEL_FINER,
                                    "Remote group %s.%s created locally.\n",
                                    partition, topic);
                    update = TRUE;
                    /* FIXME: enable again after fix is in place
                    u_objectFree(u_object(ugroup)); */
                    maxBurst--;

                    if(maxBurst == 0){
                        d_sleep(self, maxBurstSleepTime);
                        maxBurst = 10;
                    }
                } else {
                    maxBurst++;
                    d_printTimedEvent(durability, D_LEVEL_FINEST,
                                "Remote group %s.%s could not be created locally.\n",
                                partition, topic);
                    if(d_durabilityGetState(durability) == D_STATE_COMPLETE){
                        /* SUB-OPTIMAL:
                         * If group creation failed then retry if there
                         * still exist pending conflicts. When there are
                         * are no conflicts then retry 300 times (=5 min.).
                         * If there are no conflicts and 300 times.
                         * When still not created, then give up.
                         *
                         * This solution is sub-optimal because there is
                         * still the (theoretical?) risk that the topic
                         * definition is not discovered after 300 retries.
                         * A better solution would be to DISCOVER the
                         * topic definition by waiting for the DCPSTopic
                         * of fellows that have its definition. */
                        os_mutexLock(&queue->admin->conflictQueueMutex);
                        nrOfPendingConflicts = c_iterLength(queue->admin->conflictQueue);
                        os_mutexUnlock(&queue->admin->conflictQueueMutex);
                        if (nrOfPendingConflicts > 0) {
                            d_printTimedEvent(durability, D_LEVEL_FINEST,
                                    "There are still pending conflicts. Retry to create group %s.%s when no pending conflicts exist.\n",
                                    partition, topic);
                            group->creationRetryCount = 0;
                            reinsert = c_iterInsert(reinsert, group);
                        } else if (group->creationRetryCount < 299) {
                            if (group->creationRetryCount % 10 == 0) {
                                d_printTimedEvent(durability, D_LEVEL_FINEST,
                                        "No pending conflicts, but the type definition for group %s.%s may not have been discovered yet, retry (%u).\n",
                                        partition, topic, group->creationRetryCount);
                            }
                            group->creationRetryCount++;
                            reinsert = c_iterInsert(reinsert, group);
                        } else {
                            d_printTimedEvent(durability, D_LEVEL_WARNING,
                                    "No pending conflicts, but I failed to create local group %s.%s after %u retries, I give up.\n",
                                    partition, topic, group->creationRetryCount);
                            update = TRUE;
                        }
                    } else if(d_adminGetFellowCount(queue->admin) == 0){
                        d_printTimedEvent(durability, D_LEVEL_WARNING,
                                "No fellows available to provide me with group information. " \
                                "Ignoring group %s.%s.\n",
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
            d_sleep(self, sleepTime);
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

    assert(d_adminIsValid(admin));

    /* Allocate groupCreationQueue object */
    queue = d_groupCreationQueue(os_malloc(C_SIZEOF(d_groupCreationQueue)));
    if (queue) {
        /* Call super-init */
        d_lockInit(d_lock(queue), D_GROUP_CREATION_QUEUE,
                   (d_objectDeinitFunc)d_groupCreationQueueDeinit);
        /* Initialize groupCreationQueue */
        if (queue) {
            queue->groups                   = c_iterNew(NULL);
            queue->terminate                = FALSE;
            queue->admin                    = admin;
            queue->groupsToCreateVolatile   = 0;
            queue->groupsToCreateTransient  = 0;
            queue->groupsToCreatePersistent = 0;

            os_threadAttrInit(&attr);
            osr = d_threadCreate(&queue->actionThread, "groupCreationThread",
                                 &attr, (void*(*)(void*))d_groupCreationQueueRun,
                                 (void*)queue);
            if (osr != os_resultSuccess) {
                d_groupCreationQueueFree(queue);
                queue = NULL;
            }
        }
    }
    return queue;
}


void
d_groupCreationQueueDeinit(
    d_groupCreationQueue queue)
{
    assert(d_groupCreationQueueIsValid(queue));

    if (os_threadIdToInteger(queue->actionThread)) {
        queue->terminate = TRUE;
        d_threadWaitExit(queue->actionThread, NULL);
    }
    if (queue->groups){
        c_iterFree(queue->groups);
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(queue));
}


void
d_groupCreationQueueFree(
    d_groupCreationQueue queue)
{
    assert(d_groupCreationQueueIsValid(queue));

    d_objectFree(d_object(queue));
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

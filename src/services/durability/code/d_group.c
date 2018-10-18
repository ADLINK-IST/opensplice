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

#include "d__group.h"
#include "d__misc.h"
#include "d__durability.h"
#include "d__configuration.h"
#include "d__conflictResolver.h"
#include "d__durabilityStateRequest.h"
#include "d__durabilityStateRequestListener.h"
#include "d__subscriber.h"
#include "d__eventListener.h"
#include "vortex_os.h"
#include "d__thread.h"

d_group
d_groupNew(
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind,
    d_completeness completeness,
    d_quality quality)
{
    d_group group = NULL;

    if (topic && partition) {
        /* Allocate group object */
        group = os_malloc(sizeof *group);
        /* Call super-init */
        d_objectInit(d_object(group), D_GROUP, (d_objectDeinitFunc)d_groupDeinit);
        /* Initialize group */
        group->topic               = os_strdup(topic);
        group->partition           = os_strdup(partition);
        group->kind                = kind;
        group->completeness        = completeness;
        group->quality             = quality;
        group->vgroup              = NULL;
        group->storeCount          = 0;
        group->private             = FALSE;
        group->creationRetryCount  = 0;    /* Number of retries to create the group */
        group->storeMessagesLoaded = FALSE;
    }
    return group;
}

void
d_groupSetKernelGroup(
    d_group group,
    v_group vgroup)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
    assert(vgroup);

    if(group){
        group->vgroup = c_keep(vgroup);
    }
}

void
d_groupSetKernelGroupCompleteness(
    _In_ d_group group,
    _In_ c_bool complete)
{
    v_alignState c = complete ? V_ALIGNSTATE_COMPLETE : V_ALIGNSTATE_INCOMPLETE;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
    assert(group->vgroup);

    if(group->completeness != D_GROUP_UNKNOWN) {
        if(group->vgroup){
            v_groupCompleteSet(group->vgroup, c);
        }
    }
}

void
d_groupSetKernelGroupNoInterest(
    _In_ d_group group)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
    assert(d_groupIsPrivate(group) || (d_groupGetCompleteness(group) == D_GROUP_UNKNOWN));
    assert(group->vgroup);

    if(group->vgroup){
        v_groupCompleteSet(group->vgroup, V_ALIGNSTATE_NO_INTEREST);
    }
}

v_group
d_groupGetKernelGroup(
    d_group group)
{
    v_group result = NULL;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        result = c_keep(group->vgroup);
    }
    return result;
}

void
d_groupDeinit(
    d_group group)
{

    assert(d_groupIsValid(group));

    os_free(group->topic);
    os_free(group->partition);

    if (group->vgroup) {
        /* TODO: can't just c_free something from user-space. OSPL-9419 */
        c_free(group->vgroup);
    }
    /* Call super-deinit */
    d_objectDeinit(d_object(group));
}


void
d_groupFree(
    d_group group)
{
    assert(d_groupIsValid(group));

    d_objectFree(d_object(group));
}


/**
 * \brief Return the completeness of a group
 *
 * The completeness of a group is primarily determined by the completeness
 * of the associated vgroup. This will always give the correct group
 * completeness, even if durability is not responsible for alignment of the
 * group. For example, if durability does not align the builtin topics
 * because DDSI is responsible for discovery, then the completeness of
 * the group will remain D_GROUP_KNOWLEDGE_UNDEFINED although the vgroup
 * can be complete. If a historicalDataRequest for a builtin topic is received
 * then the advertised completeness of the group should be D_GROUP_COMPLETE
 * and not D_GROUP_KNOWLEDGE_UNDEFINED.
 */
d_completeness
d_groupGetCompleteness(
    d_group group)
{
    d_completeness result;

    if (group) {
        result = group->completeness;
    } else {
        result = D_GROUP_KNOWLEDGE_UNDEFINED;
    }
    return result;
}


void
d_groupUpdate(
    d_group group,
    d_completeness completeness,
    d_quality quality,
    d_admin admin)
{
    c_bool changed;

    assert(d_groupIsValid(group));
    assert(d_adminIsValid(admin));

    /* TEMPORARY SOLUTION:
     * Prevent that a D_COMPLETE state becomes 'not D_COMPLETE'.
     * This is a temporary solution for a bigger problem that
     * causes old information to be send later than new information.
     * OSPL-5057 is there to fix this.
     */
    if (!((group->completeness == D_GROUP_COMPLETE) && (completeness != D_GROUP_COMPLETE))) {
        changed = (group->completeness != completeness);
        group->completeness = completeness;
        group->quality = quality;
        /* Publish the completeness state if it has changed */
        if (changed) {
            d_groupPublishStateUpdate(group, admin);
        }
    }
}

void
d_groupSetComplete(
    d_group group,
    d_admin admin)
{
    d_conflict conflict;
    d_durability durability;

    assert(d_groupIsValid(group));
    assert(d_adminIsValid(admin));

    group->quality = D_QUALITY_INFINITE;
    if(group->completeness != D_GROUP_COMPLETE) {

        durability = d_adminGetDurability(admin);
        conflict = d_conflictNew(D_CONFLICT_LOCAL_GROUP_COMPLETE, NULL, NULL, NULL);
        d_conflictSetId(conflict, durability);
        d_printTimedEvent(durability, D_LEVEL_FINEST, "Local group complete conflict created for group '%s.%s', conflict %d created\n",
                group->partition, group->topic, d_conflictGetId(conflict));
        d_conflictResolverAddConflict(durability->admin->conflictResolver, conflict);

        group->completeness = D_GROUP_COMPLETE;
        d_adminReportGroup(admin, group);
    }

    /* Publish the completeness state */
    d_groupPublishStateUpdate(group, admin);
}

void
d_groupSetNoInterest(
    d_group group,
    d_admin admin)
{
    assert(d_groupIsValid(group));
    assert(d_adminIsValid(admin));

    group->quality = OS_TIMEW_INIT(0,0);
    group->completeness = D_GROUP_UNKNOWN;

    /* Publish the completeness state */
    d_groupPublishStateUpdate(group, admin);
}

void
d_groupSetUnaligned(
    d_group group,
    d_admin admin)
{
    assert(d_groupIsValid(group));
    assert(d_adminIsValid(admin));

    group->quality = D_QUALITY_ZERO;
    group->completeness = D_GROUP_UNKNOWN;
    if (group->vgroup) {
        v_groupCompleteSet(group->vgroup, V_ALIGNSTATE_NO_INTEREST);
    }
    /* Publish the completeness state */
    d_groupPublishStateUpdate(group, admin);
}

void d_groupSetIncomplete(
    d_group group,
    d_admin admin)
{
    assert(d_groupIsValid(group));
    assert(d_adminIsValid(admin));

    group->quality = D_QUALITY_ZERO;
    group->completeness = D_GROUP_INCOMPLETE;

    /* Publish the completeness state */
    d_groupPublishStateUpdate(group, admin);
}

int
d_groupCompare(
    d_group group1,
    d_group group2)
{
    int result;

    result = 0;

    if (group1 != group2) {
        result = strcmp(group1->topic, group2->topic);

        if (result == 0) {
            result = strcmp(group1->partition, group2->partition);
        }
    }
    return result;
}

d_partition
d_groupGetPartition(
    d_group group)
{
    d_partition p;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
    p = NULL;

    if ((group) && (group->partition != NULL)) {
        p = os_strdup(group->partition);
    }
    return p;
}

d_topic
d_groupGetTopic(
    d_group group)
{
    d_topic t;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
    t = NULL;

    if ((group) && (group->topic != NULL)) {
        t = os_strdup(group->topic);
    }
    return t;
}

d_durabilityKind
d_groupGetKind(
    d_group group)
{
    d_durabilityKind k;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        k = group->kind;
    } else {
        k = D_DURABILITY_VOLATILE;
    }
    return k;
}

d_quality
d_groupGetQuality(
    d_group group)
{
    assert(d_groupIsValid(group));

    return group->quality;
}

c_bool
d_groupIsBuiltinGroup(
    d_group group)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);
    if(group){
        return d_isBuiltinGroup (group->partition, group->topic);
    } else {
        return FALSE;
    }
}

void
d_groupSetPrivate(
    d_group group,
    c_bool isPrivate)
{
    assert(d_groupIsValid(group));

    group->private = isPrivate;
    return;
}

c_bool
d_groupIsPrivate(
    d_group group)
{
    assert(d_groupIsValid(group));

    return group->private;
}

void
d_groupSetStoreMessagesLoaded(
    d_group group,
    c_bool isLoaded)
{
    assert(d_groupIsValid(group));

    group->storeMessagesLoaded = isLoaded;
    return;
}

c_bool
d_groupIsStoreMessagesLoaded(
    d_group group)
{
    assert(d_groupIsValid(group));

    return group->storeMessagesLoaded;
}


/**
 * \brief Publish a durabilityState update for the group, but only if client-durability
 *        is enabled.
 *
 * The state is published immediately.
 */
void
d_groupPublishStateUpdate (
    d_group group,
    d_admin admin)
{
    d_durability durability;
    d_configuration config;
    d_subscriber subscriber;
    d_durabilityStateRequestListener dsrlistener = NULL;
    d_historicalDataRequestListener hdrlistener = NULL;
    struct _DDS_DurabilityVersion_t version;
    struct _DDS_RequestId_t requestId;
    d_topic topic;
    d_partition partition;
    c_iter partitions;
    c_iter serverIds;
    c_time timeout;
    c_iter extensions;
    d_durabilityStateRequest request = NULL;

    assert(d_groupIsValid(group));
    assert(d_adminIsValid(admin));

    durability = d_adminGetDurability(admin);
    config = d_durabilityGetConfiguration(durability);
    if (config->clientDurabilityEnabled) {
        /* Trigger the publication of a durabilityState message to everybody.
         * This is done by creating a (fake) durabilityStateRequest and
         * adding this request to the durabilityStateRequestListener.
         */
        version = d_durabilityGetMyVersion(durability);     /* Pretend that the request came from a server with the same version as me */
        requestId = d_durabilityGetRequestId(durability);   /* Get a new request Id */
        if ((topic = d_groupGetTopic(group)) == NULL) {             /* uses os_strdup, must be freed later */
            goto err_allocTopic;
        }
        if ((partition = d_groupGetPartition(group)) == NULL) {     /* uses os_strdup. The partition is passed to d_durabilityStateRequestNew and freed when d_durabilityStateRequestFree() is called */
            goto err_allocPartition;
        }
        if ((partitions = c_iterNew(partition)) == NULL) {          /* Set the partition */
            goto err_allocPartitions;
        }
        if ((serverIds = c_iterNew(NULL)) == NULL) {                /* Broadcast to everybody */
            goto err_allocServerIds;
        }
        timeout = C_TIME_ZERO;                                      /* Send the message without delay */
        extensions = c_iterNew(NULL);                               /* No extensions */
        if (extensions == NULL) {
            goto err_allocExtensions;
        }
        request = d_durabilityStateRequestNew(
                                    admin,
                                    version,
                                    requestId,
                                    topic,
                                    partitions,
                                    serverIds,
                                    timeout,
                                    extensions);
        if (request == NULL) {
            goto err_allocRequest;
        }
        subscriber = d_adminGetSubscriber(admin);
        assert(d_subscriberIsValid(subscriber));
        dsrlistener = subscriber->durabilityStateRequestListener;
        if (dsrlistener != NULL) {
            d_printTimedEvent(durability, D_LEVEL_FINER,
                 "Update for group '%s.%s' detected (completeness: %d), adding self-inflicted durabilityStateRequest (%lld,%lld:%lu) to queue\n",
                 partition,
                 topic,
                 group->completeness,
                 request->requestId.clientId.prefix,
                 request->requestId.clientId.suffix,
                 request->requestId.requestId);
            /* Add the request to the listener */
            d_durabilityStateRequestListenerAddRequest(dsrlistener, request);
        }
        hdrlistener = subscriber->historicalDataRequestListener;
        if (hdrlistener != NULL) {
            /* Notify listeners of group state update event */
            if (group->completeness == D_GROUP_COMPLETE) {
                d_adminNotifyListeners(admin, D_GROUP_LOCAL_COMPLETE, NULL, NULL, group, hdrlistener);
            }
        }
        /* Free the stuff */
        /* No need to free partition, because this is freed when request is freed  */
        os_free(topic);
        c_iterFree(partitions);
        c_iterFree(serverIds);
        c_iterFree(extensions);
    }
    return;

err_allocRequest:
    c_iterFree(extensions);
err_allocExtensions:
    c_iterFree(serverIds);
err_allocServerIds:
    c_iterFree(partitions);
err_allocPartitions:
    os_free(partition);
err_allocPartition:
    os_free(topic);
err_allocTopic:
    return;
}


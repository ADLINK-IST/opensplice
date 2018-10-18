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

#include "vortex_os.h"
#include "d__groupsRequestListener.h"
#include "d__readerListener.h"
#include "d__listener.h"
#include "d__configuration.h"
#include "d__admin.h"
#include "d__misc.h"
#include "d__table.h"
#include "d__publisher.h"
#include "d__group.h"
#include "d__durability.h"
#include "d__thread.h"
#include "d_networkAddress.h"
#include "d_message.h"
#include "d_newGroup.h"
#include "v_time.h"
#include "os_heap.h"

/**
 * Macro that checks the d_groupsRequestListener validity.
 * Because d_groupsRequestListener is a concrete class typechecking is required.
 */
#define d_groupsRequestListenerIsValid(_this)   \
        d_listenerIsValidKind(d_listener(_this), D_GROUPS_REQ_LISTENER)

/**
 * \brief The d_groupsRequestListener cast macro.
 *
 * This macro casts an object to a d_groupsRequestListener object.
 */
#define d_groupsRequestListener(_this) ((d_groupsRequestListener)(_this))

C_STRUCT(d_groupsRequestListener){
    C_EXTENDS(d_readerListener);
};

struct collectGroupsHelper {
    d_table groups;
    d_admin admin;
    d_configuration config;
    d_networkAddress addressee;
    d_partition partition;
    d_topic topic;
};


/**
 * \brief Collect group messages for all non-private local groups
 *        that match the partition/topic specified in args.
 */
static c_bool
collectLocalGroups(
    d_group group,
    c_voidp args)
{
    struct collectGroupsHelper *helper;
    c_char *partition, *topic;
    d_durabilityKind kind;
    d_completeness completeness;
    d_quality quality;
    c_bool proceed;

    d_newGroup newGroup;

    helper = (struct collectGroupsHelper *)args;

    partition    = d_groupGetPartition(group);
    topic        = d_groupGetTopic(group);
    kind         = d_groupGetKind(group);
    completeness = d_groupGetCompleteness(group);
    quality      = d_groupGetQuality(group);

    /* Only send groups that fulfill the partition and/or topic that
     * are requested. Besides that, local groups must also NOT be sent.
     */
    proceed = TRUE;
    if (proceed) {
        if (d_groupIsPrivate(group)) {
            proceed = FALSE; /* Don't send private groups */
        }
    }
    if (helper->partition) {
        if(strcmp(helper->partition, partition) != 0){
            proceed = FALSE; /* Don't send groups with unmatching partition */
        }
    }
    if (proceed) {
        if (helper->topic) {
            if (strcmp(helper->topic, topic) != 0) {
                proceed = FALSE; /* Don't send groups with unmatching topic */
            }
        }
    }
    if (proceed) {
        /* Also send the groups where I am not the aligner for since the master
         * might not know this group and the data for this group will not be aligned
         * when not sending the group.
         */
        newGroup = d_newGroupNew(helper->admin, partition, topic, kind,
                                 completeness, quality);
        d_messageSetAddressee(d_message(newGroup), helper->addressee);
        d_tableInsert(helper->groups, newGroup);
    }
    os_free(partition);
    os_free(topic);

    return TRUE;
}

struct sendGroupsHelper{
    d_publisher publisher;
    d_networkAddress addressee;
    d_durability durability;
    c_ulong groupCount;
    d_thread self;
};


/**
 * \brief Send the group to another durability service
 *
 * The group is send in a newGroup message. Each newGroup message contains
 * the number of groups that the recipient can expect.
 */
static c_bool
sendLocalGroups(
    d_newGroup group,
    c_voidp args)
{
    struct sendGroupsHelper* helper;

    helper = (struct sendGroupsHelper*)args;
    d_threadAwake(helper->self);
    d_newGroupSetAlignerCount(group, helper->groupCount);
    if (group->partition && group->topic) {
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
            "-Group %s.%s with completeness=%d and quality=%d.%d\n",
            group->partition, group->topic, group->completeness,
            group->quality.seconds, group->quality.nanoseconds);
    }
    d_publisherNewGroupWrite(helper->publisher, group, helper->addressee);

    return TRUE;
}

static void
d_groupsRequestListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    struct collectGroupsHelper helper;
    struct sendGroupsHelper sendHelper;
    d_newGroup group;
    d_groupsRequest request;
    d_thread self = d_threadLookupSelf();

    assert(d_groupsRequestListenerIsValid(listener));

    request = (d_groupsRequest)message;

    helper.admin     = d_listenerGetAdmin(listener);
    durability       = d_adminGetDurability(helper.admin);
    helper.partition = request->partition;
    helper.topic     = request->topic;
    helper.groups    = d_tableNew(d_newGroupCompare, d_newGroupFree);
    helper.config    = d_durabilityGetConfiguration(durability);
    helper.addressee = d_networkAddressNew(message->senderAddress.systemId,
                                           message->senderAddress.localId,
                                           message->senderAddress.lifecycleId);

    d_printTimedEvent(durability, D_LEVEL_FINE,
        "Received groupsRequest from fellow %u; sending all groups\n",
        message->senderAddress.systemId);

    d_adminGroupWalk(helper.admin, collectLocalGroups, &helper);

    sendHelper.groupCount = d_tableSize(helper.groups);

    d_printTimedEvent(durability, D_LEVEL_FINE,
        "Sending %u groups to fellow %u.\n", sendHelper.groupCount, message->senderAddress.systemId);
    sendHelper.publisher  = d_adminGetPublisher(helper.admin);
    sendHelper.addressee  = helper.addressee;
    sendHelper.durability = durability;
    sendHelper.self = self;

    d_tableWalk(helper.groups, sendLocalGroups, &sendHelper);

    if(sendHelper.groupCount == 0){ /*let the sender know I received the request.*/
        group = d_newGroupNew(helper.admin, NULL, NULL, D_DURABILITY_TRANSIENT,
                                D_GROUP_COMPLETE, os_timeWGet());
        d_newGroupSetAlignerCount(group, 0);
        d_publisherNewGroupWrite(sendHelper.publisher, group, sendHelper.addressee);
        d_newGroupFree(group);
    }

    d_networkAddressFree(helper.addressee);
    d_tableFree(helper.groups);

    d_printTimedEvent(durability, D_LEVEL_FINEST, "All local groups sent to fellow\n");
    return;
}

static void
d_groupsRequestListenerDeinit(
    d_groupsRequestListener listener)
{
    assert(d_groupsRequestListenerIsValid(listener));

    /* Stop the groupsRequestListener before cleaning up. */
    d_groupsRequestListenerStop(listener);
    /* Nothing to deallocate, all super-deinit */
    d_readerListenerDeinit(d_readerListener(listener));
}

static void
d_groupsRequestListenerInit(
    d_groupsRequestListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    /* Do not assert the listener because the initialization
     * of the listener has not yet completed
     */

    assert(d_subscriberIsValid(subscriber));

    /* Call super-init */
    os_threadAttrInit(&attr);
    d_readerListenerInit(   d_readerListener(listener),
                            D_GROUPS_REQ_LISTENER,
                            d_groupsRequestListenerAction,
                            subscriber,
                            D_GROUPS_REQ_TOPIC_NAME,
                            D_GROUPS_REQ_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            attr,
                            (d_objectDeinitFunc)d_groupsRequestListenerDeinit);
}

d_groupsRequestListener
d_groupsRequestListenerNew(
    d_subscriber subscriber)
{
    d_groupsRequestListener listener;

    assert(d_subscriberIsValid(subscriber));

    /* Allocate groupsRequestListener object */
    listener = d_groupsRequestListener(os_malloc(C_SIZEOF(d_groupsRequestListener)));
    if (listener) {
        /* Initialize the groupsRequestListener */
        d_groupsRequestListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_groupsRequestListenerFree(
    d_groupsRequestListener listener)
{
    assert(d_groupsRequestListenerIsValid(listener));

    d_objectFree(d_object(listener));
}

c_bool
d_groupsRequestListenerStart(
    d_groupsRequestListener listener)
{
    return d_readerListenerStart(d_readerListener(listener));
}

c_bool
d_groupsRequestListenerStop(
    d_groupsRequestListener listener)
{
    return d_readerListenerStop(d_readerListener(listener));
}


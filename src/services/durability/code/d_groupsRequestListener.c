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

#include "os.h"
#include "d__groupsRequestListener.h"
#include "d_groupsRequestListener.h"
#include "d_readerListener.h"
#include "d__readerListener.h"
#include "d_listener.h"
#include "d_networkAddress.h"
#include "d_group.h"
#include "d_configuration.h"
#include "d_admin.h"
#include "d_misc.h"
#include "d_message.h"
#include "d_newGroup.h"
#include "d_table.h"
#include "d_publisher.h"
#include "v_time.h"
#include "os_heap.h"

struct collectGroupsHelper {
    d_table groups;
    d_admin admin;
    d_configuration config;
    d_networkAddress addressee;
    d_partition partition;
    d_topic topic;
};

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

    if(proceed){
        if(d_groupIsPrivate(group)){
            proceed = FALSE; /*Don't send private groups*/
        }
    }

    if(helper->partition){
        if(strcmp(helper->partition, partition) != 0){
            proceed = FALSE;
        }
    }

    if(proceed){
        if(helper->topic){
            if(strcmp(helper->topic, topic) != 0){
                proceed = FALSE;
            }
        }
    }

    if(proceed){
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
};

static c_bool
sendLocalGroups(
    d_newGroup group,
    c_voidp args)
{
    struct sendGroupsHelper* helper;

    helper = (struct sendGroupsHelper*)args;

    d_newGroupSetAlignerCount(group, helper->groupCount);

    if(group->partition && group->topic){
        d_printTimedEvent(helper->durability, D_LEVEL_FINEST,
                    D_THREAD_GROUPS_REQUEST_LISTENER,
                    "-Group %s.%s with completeness=%d and quality=%d.%d\n",
                    group->partition, group->topic, group->completeness,
                    group->quality.seconds, group->quality.nanoseconds);
    }


    d_publisherNewGroupWrite(helper->publisher, group, helper->addressee);

    return TRUE;
}

d_groupsRequestListener
d_groupsRequestListenerNew(
    d_subscriber subscriber)
{
    d_groupsRequestListener listener;

    listener = NULL;

    if(subscriber){
        listener = d_groupsRequestListener(os_malloc(C_SIZEOF(d_groupsRequestListener)));
        d_listener(listener)->kind = D_GROUPS_REQ_LISTENER;
        d_groupsRequestListenerInit(listener, subscriber);
    }
    return listener;
}

void
d_groupsRequestListenerInit(
    d_groupsRequestListener listener,
    d_subscriber subscriber)
{
    os_threadAttr attr;

    os_threadAttrInit(&attr);
    d_readerListenerInit(   d_readerListener(listener),
                            d_groupsRequestListenerAction, subscriber,
                            D_GROUPS_REQ_TOPIC_NAME, D_GROUPS_REQ_TOP_NAME,
                            V_RELIABILITY_RELIABLE,
                            V_HISTORY_KEEPALL,
                            V_LENGTH_UNLIMITED,
                            attr,
                            d_groupsRequestListenerDeinit);

}

void
d_groupsRequestListenerFree(
    d_groupsRequestListener listener)
{
    assert(d_listenerIsValid(d_listener(listener), D_GROUPS_REQ_LISTENER));

    if(listener){
        d_readerListenerFree(d_readerListener(listener));
    }
}

void
d_groupsRequestListenerDeinit(
    d_object object)
{
    OS_UNUSED_ARG(object);
    assert(d_listenerIsValid(d_listener(object), D_GROUPS_REQ_LISTENER));

    return;

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

void
d_groupsRequestListenerAction(
    d_listener listener,
    d_message message)
{
    d_durability durability;
    struct collectGroupsHelper helper;
    struct sendGroupsHelper sendHelper;
    d_newGroup group;
    d_groupsRequest request;

    assert(d_listenerIsValid(d_listener(listener), D_GROUPS_REQ_LISTENER));

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
                D_THREAD_GROUPS_REQUEST_LISTENER,
                "Received groupsRequest from fellow %u; sending all groups\n",
                message->senderAddress.systemId);

    d_adminGroupWalk(helper.admin, collectLocalGroups, &helper);

    sendHelper.groupCount = d_tableSize(helper.groups);

    d_printTimedEvent(durability, D_LEVEL_FINE,
                D_THREAD_GROUPS_REQUEST_LISTENER,
                "Sending %u groups to fellow %u.\n",
                sendHelper.groupCount,
                message->senderAddress.systemId);
    sendHelper.publisher  = d_adminGetPublisher(helper.admin);
    sendHelper.addressee  = helper.addressee;
    sendHelper.durability = durability;

    d_tableWalk(helper.groups, sendLocalGroups, &sendHelper);

    if(sendHelper.groupCount == 0){ /*let the sender know I received the request.*/
        group = d_newGroupNew(helper.admin, NULL, NULL, D_DURABILITY_TRANSIENT,
                                D_GROUP_COMPLETE, v_timeGet());
        d_newGroupSetAlignerCount(group, 0);
        d_publisherNewGroupWrite(sendHelper.publisher, group, sendHelper.addressee);
        d_newGroupFree(group);
    }

    d_networkAddressFree(helper.addressee);
    d_tableFree(helper.groups);

    d_printTimedEvent(durability, D_LEVEL_FINE,
        D_THREAD_GROUPS_REQUEST_LISTENER, "All local groups sent to fellow\n");
    return;
}

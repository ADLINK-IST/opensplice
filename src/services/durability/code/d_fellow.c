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

#include "d__fellow.h"
#include "d__listener.h"
#include "d__admin.h"
#include "d__misc.h"
#include "d__table.h"
#include "d__group.h"
#include "d__nameSpace.h"
#include "d__configuration.h"
#include "d__publisher.h"
#include "d__durability.h"
#include "d__mergeState.h"
#include "d__thread.h"
#include "d__durability.h"
#include "d__capability.h"
#include "d__thread.h"
#include "d_message.h"
#include "d_networkAddress.h"
#include "d_nameSpacesRequest.h"
#include "os_heap.h"
#include "os_stdlib.h"
#include "os_time.h"
#include "os_report.h"

struct findNameSpace{
    d_nameSpace template;
    d_nameSpace nameSpace;
};

struct alignerHelper{
    const c_char* partition;
    const c_char* topic;
    d_durabilityKind kind;
    c_bool aligner;
    c_ulong masterPriority;
};

struct alignerNSHelper{
    d_nameSpace nameSpace;
    c_bool aligner;
};

char* d_fellowState_text[] = {
        "D_STATE_INIT",
        "D_STATE_DISCOVER_FELLOWS_GROUPS",
        "D_STATE_DISCOVER_LOCAL_GROUPS",
        "D_STATE_DISCOVER_PERSISTENT_SOURCE",
        "D_STATE_INJECT_PERSISTENT",
        "D_STATE_FETCH_INITIAL",
        "D_STATE_COMPLETE",
        "D_STATE_FETCH",
        "D_STATE_ALIGN",
        "D_STATE_FETCH_ALIGN",
        "D_STATE_TERMINATING",
        "D_STATE_TERMINATED"
};

d_fellow
d_fellowNew(
    d_networkAddress address,
    d_serviceState state,
    c_bool isConfirmed)
{
    d_fellow fellow;

    /* Allocate fellow object */
    fellow = d_fellow(os_malloc(C_SIZEOF(d_fellow)));
    if (fellow) {
        /* Call super-init */
        d_lockInit(d_lock(fellow), D_FELLOW,
                   (d_objectDeinitFunc)d_fellowDeinit);
        /* Initialize the fellow */
        fellow->address = d_networkAddressNew(address->systemId,
                                              address->localId,
                                              address->lifecycleId);
        fellow->communicationState           = D_COMMUNICATION_STATE_UNKNOWN;
        fellow->state                        = state;
        fellow->lastStatusReport             = OS_TIMEM_ZERO; /* last status report */
        fellow->lastSeqNum                   = 0ULL; /* unsigned long long */
        fellow->groups                       = NULL; /*Lazy initialization*/
        fellow->nameSpaces                   = NULL; /*Lazy initialization*/
        fellow->requestCount                 = 0;
        fellow->expectedGroupCount           = -1;
        fellow->expectedNameSpaces           = 1; /*At least one namespace is expected*/
        fellow->groupsRequested              = FALSE;
        fellow->role                         = NULL;
        fellow->isConfirmed                  = isConfirmed;
        fellow->hasConfirmed                 = isConfirmed;  /* Initially, when a confirmed fellow is
                                                              * created it is confirmed for the first time. */
        fellow->recentlyTerminated           = FALSE;
        fellow->removalTime                  = OS_TIMEM_ZERO;
        fellow->heartbeatDisposeCount        = -1;
        fellow->capabilitySupport            = TRUE; /* Initially assume that all fellows support capabilities.
                                                      * This is set to FALSE when a message is received from
                                                      * this fellow with the most-significant bit of the
                                                      * nanoseconds in the production timestamp field set to 0
                                                      */
        fellow->readers                      = 0; /* No readers detected yet for this fellow */
        fellow->requiredReaders              = D_BASIC_DURABILITY_READER_FLAGS | D_CAPABILITY_READER_FLAG;
        fellow->responsive                   = FALSE; /* Initially no nameSpaceRequest has been sent to this fellow */
        fellow->recently_joined              = TRUE;  /* Initially the fellow did not recently join; it is set when the fellow is added to the admin */
        fellow->capabilities.groupHash       = TRUE;
        fellow->capabilities.EOTSupport      = TRUE;
        fellow->capabilities.masterSelection = D_USE_MASTER_PRIORITY_ALGORITHM;  /* Use priority-based masterSelection algorithm */
        fellow->capabilities.incarnation     = 0; /* Initial incarnation */
        fellow->has_requested_namespaces     = FALSE; /* Initially the fellow has not yet requested namespaces from me */
        fellow->incarnation                  = 0; /* The incarnation that I advertise to my fellow */
     }
    return fellow;
}

/**
 * \brief Set the property fellow->groupsRequested to TRUE.
 *
 * This property indicates that groups are requested from the fellow.
 *
 * @return TRUE if groups->requested changed from FALSE to TRUE,
 *         FALSE otherwise
 */
c_bool
d_fellowSetGroupsRequested(
    d_fellow fellow)
{
    c_bool result;

    assert(d_fellowIsValid(fellow));

    d_lockLock(d_lock(fellow));
    if (fellow->groupsRequested) {
        result = FALSE;
    } else {
        fellow->groupsRequested = TRUE;
        result = TRUE;
    }
    d_lockUnlock(d_lock(fellow));
    return result;
}

c_bool
d_fellowGetGroupsRequested(
    d_fellow fellow)
{
    c_bool result;

    assert(d_fellowIsValid(fellow));

    d_lockLock(d_lock(fellow));
    result = fellow->groupsRequested;
    d_lockUnlock(d_lock(fellow));
    return result;
}

d_networkAddress
d_fellowGetAddress(
    d_fellow fellow)
{
    d_networkAddress address = NULL;
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if (fellow) {
        address = d_networkAddressNew(fellow->address->systemId,
                                      fellow->address->localId,
                                      fellow->address->lifecycleId);
    }
    return address;
}

void
d_fellowSetAddress(
    d_fellow fellow,
    d_networkAddress address)
{
    assert(d_fellowIsValid(fellow));

    d_lockLock(d_lock(fellow));
    fellow->address->systemId    = address->systemId;
    fellow->address->localId     = address->localId;
    fellow->address->lifecycleId = address->lifecycleId;
    d_lockUnlock(d_lock(fellow));
    return;
}

void
d_fellowDeinit(
    d_fellow fellow)
{
    assert(d_fellowIsValid(fellow));

    if (fellow->groups) {
        d_tableFree(fellow->groups);
        fellow->groups = NULL;
    }
    if (fellow->address) {
        d_networkAddressFree(fellow->address);
        fellow->address = NULL;
    }
    if (fellow->nameSpaces) {
        d_tableFree(fellow->nameSpaces);
        fellow->nameSpaces = NULL;
    }
    if (fellow->role) {
        os_free (fellow->role);
        fellow->role = NULL;
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(fellow));

}

void
d_fellowFree(
    d_fellow fellow)
{
    assert(d_fellowIsValid(fellow));

    d_objectFree(d_object(fellow));
}

d_serviceState
d_fellowGetState(
    d_fellow fellow)
{
    d_serviceState state = D_STATE_INIT;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
       state = fellow->state;
    }
    return state;
}

void
d_fellowUpdateStatus(
    d_fellow fellow,
    d_serviceState state,
    os_uint32 seqnum)
{
    d_durability durability = d_threadsDurability();
    os_timeM timestamp = os_timeMGet();

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
       d_lockLock(d_lock(fellow));
       /* Check if seqnum do not go backward (modulo wrapping).
        * If no sequence number support is available then the sequence number will
        * always be 0 (which is never handed out when sequence numbers are supported).
        * To keep updating in this case >= iso >
        */
      if (D_SEQNUM_ISVALID(seqnum) && ((seqnum >= fellow->lastSeqNum) || ((os_uint32)(fellow->lastSeqNum + seqnum) < fellow->lastSeqNum))) {
           /* Update the last state of the fellow.
            * Only print a log line if the state changed.
            */
           if (fellow->state != state) {
                d_printTimedEvent (durability, D_LEVEL_FINEST,
                       "Updating state of fellow '%d' to '%s' (lastSeqNum=%"PA_PRIu32", seqnum=%"PA_PRIu32", lastStatusReport=%"PA_PRItime", timestamp=%"PA_PRItime").\n",
                       fellow->address->systemId, d_fellowStateText(state), fellow->lastSeqNum, seqnum,
                       OS_TIMEM_PRINT(fellow->lastStatusReport),
                       OS_TIMEM_PRINT(timestamp));
           }
           fellow->lastStatusReport = timestamp;
           fellow->lastSeqNum = seqnum;
           fellow->state = state;
       }
       d_lockUnlock(d_lock(fellow));
    }
}

c_bool
d_fellowAddGroup(
    d_fellow fellow,
    d_group group)
{
    c_bool result;
    d_group duplicate;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    result = FALSE;

    if(group && fellow){
        d_lockLock(d_lock(fellow));

        if(!fellow->groups){
            fellow->groups = d_tableNew(d_groupCompare, d_groupFree);
        }
        duplicate = d_tableInsert(fellow->groups, group);
        d_lockUnlock(d_lock(fellow));

        if(!duplicate){
            result = TRUE;
        }
    }
    return result;
}

void
d_fellowSetExpectedGroupCount(
    d_fellow fellow,
    c_long count)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        fellow->expectedGroupCount = count;
    }
}

c_long
d_fellowGetExpectedGroupCount(
    d_fellow fellow)
{
    c_long count;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    count = -1;

    if(fellow){
        d_lockLock(d_lock(fellow));
        count = fellow->expectedGroupCount;
        d_lockUnlock(d_lock(fellow));
    }
    return count;
}

c_ulong
d_fellowGetGroupCount(
    d_fellow fellow)
{
    c_ulong count;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if (fellow) {
        d_lockLock(d_lock(fellow));
        if (fellow->groups) {
            count = d_tableSize(fellow->groups);
        } else {
            count = 0;
        }
        d_lockUnlock(d_lock(fellow));
    } else {
        count = 0;
    }
    return count;
}

d_group
d_fellowRemoveGroup(
    d_fellow fellow,
    d_group group)
{
    d_group result = NULL;
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group && fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->groups){
            result = d_tableRemove(fellow->groups, group);
        }
        d_lockUnlock(d_lock(fellow));
    }
    return result;
}

d_group
d_fellowGetGroup(
    d_fellow fellow,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    d_group dummy;
    d_group found = NULL;

    assert(d_fellowIsValid(fellow));

    dummy = d_groupNew(partition, topic, kind, D_GROUP_KNOWLEDGE_UNDEFINED, D_QUALITY_ZERO);
    d_lockLock(d_lock(fellow));
    if (fellow->groups) {
        found = d_tableFind(fellow->groups, dummy);
        if(found){
            d_objectKeep(d_object(found));
        }
    }
    d_lockUnlock(d_lock(fellow));
    d_groupFree(dummy);
    return found;

}

c_bool
d_fellowHasGroup(
    d_fellow fellow,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    c_bool hasGroup = FALSE;
    d_group found;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        found = d_fellowGetGroup(fellow, partition, topic, kind);

        if(found){
            hasGroup = TRUE;
            d_groupFree(found);
        }
    }
    return hasGroup;
}


c_bool
d_fellowGroupWalk(
    d_fellow fellow,
    c_bool ( * action ) (d_group group, c_voidp userData),
    c_voidp userData)
{
    c_bool result;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    result = FALSE;

    if(fellow){
        result = TRUE;
        d_lockLock(d_lock(fellow));

        if(fellow->groups){
            d_tableWalk(fellow->groups, action, userData);
        }
        d_lockUnlock(d_lock(fellow));
    }
    return result;
}

int
d_fellowCompare(
    d_fellow fellow1,
    d_fellow fellow2)
{
    int result = 0;

    if (fellow1 != fellow2) {
        result = d_networkAddressCompare(fellow1->address, fellow2->address);
    }
    return result;
}

os_timeM
d_fellowGetLastStatusReport(
    d_fellow fellow)
{
    os_timeM s = OS_TIMEM_INIT(0,0);

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow) {
       s = fellow->lastStatusReport;
    }
    return s;
}

d_communicationState
d_fellowGetCommunicationState(
    d_fellow fellow)
{
    d_communicationState state = D_COMMUNICATION_STATE_UNKNOWN;
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow) {
        state = fellow->communicationState;
    }
    return state;
}

void
d_fellowSetCommunicationState(
    d_fellow fellow,
    d_communicationState state)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    d_lockLock(d_lock(fellow));
    fellow->communicationState = state;
    d_lockUnlock(d_lock(fellow));
}

c_bool
d_fellowAddNameSpace(
    d_fellow fellow,
    d_nameSpace nameSpace)
{
    c_bool result;
    d_nameSpace duplicate;
    d_networkAddress master, myAddr;
    c_bool masterConfirmed;
    d_mergeState mergeState;
    d_durability durability;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    assert(d_objectIsValid(d_object(nameSpace), D_NAMESPACE) == TRUE);

    result = FALSE;
    durability = d_threadsDurability();
    myAddr = d_adminGetMyAddress(durability->admin);
    if (fellow && nameSpace) {
        d_lockLock(d_lock(fellow));
        master = d_nameSpaceGetMaster(nameSpace);
        if (!fellow->nameSpaces) {
            fellow->nameSpaces = d_tableNew(d_nameSpaceNameCompare, d_nameSpaceFree);
        }
        duplicate = d_tableInsert(fellow->nameSpaces, nameSpace);
        if (duplicate) {
            /* There was a duplicate.
             * Update the master fellow of the namespace.
             */
            d_nameSpaceSetMaster(duplicate, master);
            /* update the confirmed property of the fellow nameSpace */
            masterConfirmed = d_nameSpaceIsMasterConfirmed(nameSpace);
            d_nameSpaceSetMasterConfirmed(duplicate, masterConfirmed);
            /* update the merge state of the fellow namespace in its native role */
            mergeState = d_nameSpaceGetMergeState(nameSpace, NULL);
            d_nameSpaceSetMergeState(duplicate, mergeState);
            d_mergeStateFree(mergeState);
            nameSpace = duplicate;
        }
        d_networkAddressFree(master);
        d_lockUnlock(d_lock(fellow));

        if (!duplicate) {
            /* A duplicate was found, return FALSE */
            result = TRUE;
        }
    }
    d_networkAddressFree(myAddr);
    return result;
}

static c_bool
determineNameSpace(
    d_nameSpace nameSpace,
    c_voidp args)
{
    struct findNameSpace* fn;
    c_bool proceed;

    fn = (struct findNameSpace*)args;
    proceed = TRUE;

    if(d_nameSpaceNameCompare(nameSpace, fn->template) == 0){
        proceed = FALSE;
        fn->nameSpace = nameSpace;
    }
    return proceed;
}

d_nameSpace
d_fellowGetNameSpace(
    d_fellow fellow,
    d_nameSpace template)
{
    struct findNameSpace fn;

    fn.template  = template;
    fn.nameSpace = NULL;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            d_tableWalk(fellow->nameSpaces, determineNameSpace, &fn);
        }
        d_lockUnlock(d_lock(fellow));
    }
    return fn.nameSpace;
}

static c_bool
clearMaster(
    d_nameSpace nameSpace,
    c_voidp args)
{
    d_networkAddress addr;

    if(nameSpace){
        addr = d_nameSpaceGetMaster(nameSpace);

        if(d_networkAddressEquals(addr, d_networkAddress(args))){
            d_networkAddressFree(addr);
            addr = d_networkAddressUnaddressed();
            d_nameSpaceSetMaster(nameSpace, addr);
        }
        d_networkAddressFree(addr);
    }
    return TRUE;
}

void
d_fellowClearMaster(
    d_fellow fellow,
    d_networkAddress master)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            d_tableWalk(fellow->nameSpaces, clearMaster, master);
        }
        d_lockUnlock(d_lock(fellow));
    }
    return;
}

c_ulong
d_fellowNameSpaceCount(
    d_fellow fellow)
{
    c_ulong count;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            count = d_tableSize(fellow->nameSpaces);
        } else {
            count = 0;
        }
        d_lockUnlock(d_lock(fellow));
    } else {
        count = 0;
    }
    return count;
}

c_bool
d_fellowAreNameSpacesComplete(
    d_fellow fellow)
{
    c_bool result;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            if(fellow->expectedNameSpaces == d_tableSize(fellow->nameSpaces)){
                result = TRUE;
            } else {
                result = FALSE;
            }
        } else {
            if(fellow->expectedNameSpaces == 0){
                result = TRUE;
            } else {
                result = FALSE;
            }
        }
        d_lockUnlock(d_lock(fellow));
    } else {
        result = FALSE;
    }
    return result;
}

c_ulong
d_fellowGetExpectedNameSpaces(
    d_fellow fellow)
{
    c_ulong count = 0;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));
        count = fellow->expectedNameSpaces;
        d_lockUnlock(d_lock(fellow));
    }
    return count;
}

void
d_fellowSetExpectedNameSpaces(
    d_fellow fellow,
    c_ulong expected)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));
        fellow->expectedNameSpaces = expected;
        d_lockUnlock(d_lock(fellow));
    }
    return;
}

c_bool
d_fellowNameSpaceWalk(
    d_fellow fellow,
    c_bool ( * action ) (d_nameSpace nameSpace, c_voidp userData),
    c_voidp userData)
{
    c_bool result;
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    result = TRUE;

    if(fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            result = d_tableWalk(fellow->nameSpaces, action, userData);
        }
        d_lockUnlock(d_lock(fellow));
    }
    return result;
}

void
d_fellowRequestAdd(
    d_fellow fellow)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));
        fellow->requestCount++;
        assert(fellow->requestCount >= 1);
        d_lockUnlock(d_lock(fellow));
    }
    return;
}

void
d_fellowRequestRemove(
    d_fellow fellow)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));
        assert(fellow->requestCount >= 1);
        fellow->requestCount--;
        d_lockUnlock(d_lock(fellow));
    }
    return;
}

c_ulong
d_fellowRequestCountGet(
    d_fellow fellow)
{
    c_ulong count = 0;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));
        count = fellow->requestCount;
        d_lockUnlock(d_lock(fellow));
    }
    return count;
}

void
d_fellowRequestCountSet(
    d_fellow fellow,
    c_ulong count)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));
        fellow->requestCount = count;
        d_lockUnlock(d_lock(fellow));
    }
    return;
}

c_bool
d_fellowIsCompleteForGroup(
    d_fellow fellow,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    c_bool result = FALSE;
    d_group group;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        group = d_fellowGetGroup(fellow, partition, topic, kind);

        if(group){
            if(d_groupGetCompleteness(group) == D_GROUP_COMPLETE){
                result = TRUE;
            }
            d_groupFree(group);
        }
    }
    return result;
}



static c_bool
isAligner(
    d_nameSpace nameSpace,
    c_voidp args)
{
    struct alignerHelper* helper;

    helper = (struct alignerHelper*)args;

    helper->aligner = d_adminInNameSpace(
                                            nameSpace,
                                            (d_partition)(helper->partition),
                                            (d_topic)(helper->topic),
                                            TRUE);
    helper->masterPriority = d_nameSpaceGetMasterPriority(nameSpace);
    return (!helper->aligner);
}

d_fellowAlignStatus
d_fellowIsAlignerForGroup(
    d_fellow fellow,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind,
    c_ulong *masterPriority)
{
    struct alignerHelper helper;
    d_fellowAlignStatus status;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    *masterPriority = D_MINIMUM_MASTER_PRIORITY;
    if(fellow){
        helper.partition = partition;
        helper.topic     = topic;
        helper.kind      = kind;
        helper.aligner   = FALSE;
        helper.masterPriority = D_MINIMUM_MASTER_PRIORITY;

        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            d_tableWalk(fellow->nameSpaces, isAligner, &helper);
            /* Make sure to return the masterPriority of the fellow namespace
             * It is used in findAligner to determine whether the fellow
             * is a candidate aligner */
            *masterPriority = helper.masterPriority;
            if(helper.aligner == FALSE){
                status = D_ALIGN_FALSE;
            } else {
                status = D_ALIGN_TRUE;
            }
        } else {
            status = D_ALIGN_FALSE;
        }
        d_lockUnlock(d_lock(fellow));
    } else {
        status = D_ALIGN_FALSE;
    }
    return status;
}

static c_bool
inNameSpace(
    d_nameSpace nameSpace,
    c_voidp args)
{
    struct alignerHelper* helper;

    helper = (struct alignerHelper*)args;

    helper->aligner = d_adminInNameSpace(
                                            nameSpace,
                                            (d_partition)(helper->partition),
                                            (d_topic)(helper->topic),
                                            FALSE);

    return (!helper->aligner);
}

c_bool
d_fellowIsGroupInNameSpaces(
    d_fellow fellow,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    struct alignerHelper helper;
    c_bool result;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        helper.partition = partition;
        helper.topic     = topic;
        helper.kind      = kind;
        helper.aligner   = FALSE;

        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            d_tableWalk(fellow->nameSpaces, inNameSpace, &helper);
        }
        result = helper.aligner;

        d_lockUnlock(d_lock(fellow));
    } else {
        result = FALSE;
    }
    return result;
}


static c_bool
isNSAligner(
    d_nameSpace nameSpace,
    c_voidp args)
{
    struct alignerNSHelper* helper;
    c_bool match;

    helper = (struct alignerNSHelper*)args;
    match = (d_nameSpaceCompatibilityCompare(nameSpace, helper->nameSpace) == 0);

    if(match == TRUE){
        helper->aligner = d_nameSpaceIsAligner(nameSpace);
    }
    return (!helper->aligner);
}

d_fellowAlignStatus
d_fellowIsAlignerForNameSpace(
    d_fellow fellow,
    d_nameSpace nameSpace)
{
    struct alignerNSHelper helper;
    d_fellowAlignStatus status;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        helper.nameSpace = nameSpace;
        helper.aligner   = FALSE;

        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            d_tableWalk(fellow->nameSpaces, isNSAligner, &helper);

            if(helper.aligner == FALSE){
                status = D_ALIGN_FALSE;
            } else {
                status = D_ALIGN_TRUE;
            }
        } else {
            status = D_ALIGN_FALSE;
        }
        d_lockUnlock(d_lock(fellow));
    } else {
        status = D_ALIGN_FALSE;
    }
    return status;
}

void
d_fellowSetRole (
    d_fellow fellow,
    d_name role)
{
    if (fellow) {
        assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
        d_lockLock(d_lock(fellow));

        if (!fellow->role) {
            fellow->role = os_strdup(role);
        }

        d_lockUnlock(d_lock(fellow));
    }
}

d_name
d_fellowGetRole (
    d_fellow fellow)
{
    d_name result;

    result = NULL;

    if (fellow) {
        assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
        d_lockLock(d_lock(fellow));

        result = fellow->role;

        d_lockUnlock(d_lock(fellow));
    }

    return result;
}


void
d_fellowAddReader(
    d_fellow fellow,
    c_ulong reader)
{
    assert(d_fellowIsValid(fellow));

    /* If a new reader is discovered then add the reader flag
     * to the fellow and check if the fellow has become responsive.
     *
     * There is no need to lock the fellow when modifying the
     * reader flags because only a single thread calls this
     * function.
     */
    if ((fellow->readers & reader) == 0) {
        d_durability durability = d_threadsDurability();
        d_networkAddress fellowAddr;

        fellowAddr = d_fellowGetAddress(fellow);
        fellow->readers |= reader;
        d_printTimedEvent(durability, D_LEVEL_FINEST,
               "Durability reader %04lx of fellow %u discovered (readers: %04lx, required readers: %04lx).\n",
               reader, fellowAddr->systemId, fellow->readers, fellow->requiredReaders);
        /* Check whether or not to send capabilities to the fellow for the first time */
        d_fellowCheckSendCapabilities(fellow, TRUE);
        /* Check initial responsiveness */
        d_fellowCheckInitialResponsiveness(fellow);

        d_networkAddressFree(fellowAddr);
    }
}


void
d_fellowRemoveReader(
    d_fellow fellow,
    c_ulong reader)
{
    d_fellow fellow2;
    c_bool oldResponsive, newResponsive;

    assert(d_fellowIsValid(fellow));

    /* A previously discovered reader was lost for the fellow.
     * Reset the reader flag for this fellow and recheck if
     * responsiveness is not lost.
     *
     * There is no need to lock the fellow when modifying the
     * reader flags because only a single thread calls this
     * function.
     */
    if ((fellow->readers & reader) == reader) {
        d_durability durability = d_threadsDurability();
        d_configuration config = d_durabilityGetConfiguration(durability);
        d_networkAddress fellowAddr;

        fellowAddr = d_fellowGetAddress(fellow);
        oldResponsive = d_fellowIsResponsive(fellow, config->waitForRemoteReaders);
        fellow->readers &= (~reader);
        d_printTimedEvent(durability, D_LEVEL_FINEST,
               "Durability reader %lx of fellow %u lost (readers: %lx, requiredReaders: %lx).\n",
               reader, fellowAddr->systemId, fellow->readers, fellow->requiredReaders);
        newResponsive = d_fellowIsResponsive(fellow, config->waitForRemoteReaders);
        if ((oldResponsive) && !newResponsive) {
            d_admin admin;

            admin = durability->admin;
            /* The fellow has just become non-responsive */
            d_printTimedEvent(durability, D_LEVEL_FINER,
                "Fellow %u has become non-responsive, removing the fellow\n",
                fellowAddr->systemId);
            fellow2 = d_adminRemoveFellow(admin, fellow, FALSE);
            if (fellow2) {
                d_fellowFree(fellow2);
            }
        }
        d_networkAddressFree(fellowAddr);
    }
}

/**
 * \brief Return the 'confirmed' status of the fellow.
 *
 * @return TRUE if the fellow is confirmed, FALSE otherwise.
 */
c_bool
d_fellowIsConfirmed(
    d_fellow fellow)
{
    c_bool result;

    assert(d_fellowIsValid(fellow));

    d_lockLock(d_lock(fellow));
    result = fellow->isConfirmed;
    d_lockUnlock(d_lock(fellow));
    return result;
}

/**
 * \brief Set the 'confirmed' property of the fellow.
 *
 * If the fellow moves from 'unconfirmed' to 'confirmed' then
 * the fellow->hasConfirmed property is set to TRUE.
 */
void
d_fellowSetConfirmed(
    d_fellow fellow,
    c_bool confirmed)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW));

    d_lockLock(d_lock(fellow));
    if ((!fellow->isConfirmed) && (confirmed == TRUE)) {
        /* The fellow moves from unconfirmed to confirmed,
         * so set fellow->hasConfirmed.
         */
        fellow->hasConfirmed = TRUE;
    }
    fellow->isConfirmed = confirmed;
    d_lockUnlock(d_lock(fellow));
}


/**
 * \brief Returns whether the fellow has recently become confirmed.
 *
 * When the fellow has recently become confirmed TRUE will be returned and
 * its hasConfirmed property is reset to FALSE.
 *
 * @return TRUE if the fellow has recently become confirmed, FALSE otherwise
 */
c_bool
d_fellowHasConfirmed(
    d_fellow fellow)
{
    c_bool result = FALSE;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW));

    d_lockLock(d_lock(fellow));
    if (fellow->hasConfirmed) {
        /* Reset hasConfirmed to prevent that the same fellow
         * is considered a recently confirmed fellow again.
         */
        result = TRUE;
        fellow->hasConfirmed = FALSE;
    }
    d_lockUnlock(d_lock(fellow));
    return result;
}


/**
 * \brief Check whether a message that the fellow has recently terminated is
 *        printed or not.
 *
 * When the message has not been printed FALSE is returned and
 * fellow->recentlyTerminated is set to TRUE to prevent that
 * the same message is printed the next time.
 *
 * @return FALSE, if the has not been printed, TRUE otherwise.
 */
c_bool
d_fellowHasRecentlyTerminated(
    d_fellow fellow)
{
    c_bool result = TRUE;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW));

    d_lockLock(d_lock(fellow));
    if (!fellow->recentlyTerminated) {
        /* Prevent printing of messages the next time. */
        fellow->recentlyTerminated = TRUE;
        result = FALSE;
    }
    d_lockUnlock(d_lock(fellow));
    return result;
}


/**
 * \brief Verify if all required readers of this fellow have been discovered.
 *
 * @return FALSE, if the fellow must wait for remote readers but not all remote readers have been discovered yet,
 *         TRUE, otherwise
 */
c_bool
d_fellowHasAllRequiredReaders(
    d_fellow fellow,
    c_bool waitForRemoteReaders)
{
    c_bool result = TRUE;

    assert(d_fellowIsValid(fellow));

    d_lockLock(d_lock(fellow));
    if (waitForRemoteReaders) {
        /* Check if all required readers are discovered. */
        result = (((fellow->readers) & (fellow->requiredReaders)) == (fellow->requiredReaders));
    }
    d_lockUnlock(d_lock(fellow));
    return result;
}

/**
 * \brief Check if all readers in the mask have been discovered for the fellow
 *
 * If waitForRemoteReaders is FALSE then there is no need to discover readers,
 * so TRUE is returned.
 */
c_bool
d_fellowHasDiscoveredReaders(
    d_fellow fellow,
    c_ulong mask,
    c_bool waitForRemoteReaders)
{
    c_bool result = TRUE;

    assert(d_fellowIsValid(fellow));

    if (waitForRemoteReaders) {
        /* Check if the discovered readers of the fellow
         * match the ones specified in the mask.
         */
        result = (((fellow->readers) & (mask)) == (mask));
    }
    return result;

}

c_long
d_fellowSetLastDisposeCount(
    d_fellow fellow,
    c_long disposeCount)
{
    c_long prevCount;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW));

    d_lockLock(d_lock(fellow));
    prevCount = fellow->heartbeatDisposeCount;
    fellow->heartbeatDisposeCount = disposeCount;
    d_lockUnlock(d_lock(fellow));

    return prevCount;
}

c_long
d_fellowGetLastDisposeCount(
    d_fellow fellow)
{
    c_long prevCount;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW));

    d_lockLock(d_lock(fellow));
    prevCount = fellow->heartbeatDisposeCount;
    d_lockUnlock(d_lock(fellow));

    return prevCount;
}


/**
 * \brief Set capability support for the fellow to the specified value.
 */
void
d_fellowSetCapabilitySupport(
    d_fellow fellow,
    c_bool capabilitySupport)
{
    d_durability durability = d_threadsDurability();

    assert(d_fellowIsValid(fellow));

    d_lockLock(d_lock(fellow));
    if (!capabilitySupport) {
        /* The fellow has no support for capabilities.
         * This means we do NOT have to wait for the
         * remote d_capability reader to check if all
         * readers have become available.
         * Update the fellow if not done so already
         */
        if (fellow->capabilitySupport) {
            durability = d_threadsDurability();

            fellow->capabilitySupport = FALSE;
            /* Reset the flag for the capability interface in the mask */
            fellow->requiredReaders = fellow->requiredReaders & (~D_CAPABILITY_READER_FLAG);

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                 "Fellow %u has NO capability support, required readers becomes %04lx\n",
                 fellow->address->systemId, fellow->requiredReaders);

        }
    } else {
        /* The fellow has support for capabilities.
         * This means we must wait for the d_capability
         * reader to be present.
         * Update the fellow if not done so already
         */
        if (!fellow->capabilitySupport) {

            fellow->capabilitySupport = TRUE;
            /* Set the flag for the capability interface in the mask */
            fellow->requiredReaders = fellow->requiredReaders | (D_CAPABILITY_READER_FLAG);

            d_printTimedEvent(durability, D_LEVEL_FINEST,
                "Fellow %u has capability support, required readers becomes %04lx\n",
                fellow->address->systemId, fellow->requiredReaders);

        }
    }
    d_lockUnlock(d_lock(fellow));
}


/**
 * \brief Set the capabilities of the fellow
 */
void
d_fellowSetCapability(
    d_fellow fellow,
    d_capability capability)
{
    c_ulong i, length;
    struct d_nameValue_s *cap;
    c_bool has_y2038_capability = FALSE;
    d_durability durability = d_threadsDurability();
    c_bool check_for_conflicts = FALSE;
    c_bool incarnation_present = FALSE;
    c_ulong incarnation = 0;
    c_bool progress;
    const c_ulong thres = 1u << (8* sizeof(c_ulong)-1);

    assert(d_fellowIsValid(fellow));

    if (capability) {
        d_conflictMonitor conflictMonitor = durability->admin->conflictMonitor;

        d_lockLock(d_lock(fellow));

        /* Store the capabilities received */
        length = c_sequenceSize(capability->capabilities);

        for (i = 0; i < length; i++) {
            cap = &((struct d_nameValue_s *)capability->capabilities)[i];

            /* Group hash capability */
            if (strcmp(cap->name, D_CAPABILITY_GROUP_HASH) == 0) {
                fellow->capabilities.groupHash = (((c_octet *)cap->value)[0] == 0) ? FALSE : TRUE;
            /* EOT support capability */
            } else if (strcmp(cap->name, D_CAPABILITY_EOT_SUPPORT) == 0) {
                fellow->capabilities.EOTSupport = (((c_octet *)cap->value)[0] == 0) ? FALSE : TRUE;
            } else if (strcmp(cap->name, D_CAPABILITY_MASTER_SELECTION) == 0) {
                c_ulong version;
                memcpy (&version, cap->value, sizeof (c_ulong));
                fellow->capabilities.masterSelection = d_swap4uFromBE(version);
            } else if (strcmp(cap->name, D_CAPABILITY_INCARNATION) == 0) {
                c_ulong inc;
                memcpy(&inc, cap->value, sizeof(c_ulong));
                incarnation = d_swap4uFromBE(inc);
                incarnation_present = TRUE;
            } else if (strcmp(cap->name, D_CAPABILITY_Y2038READY) == 0) {
                /* Y2038Ready support
                 * Receiving this capability means that the fellow is capable to receive extended (64-bit) times.
                 * It does not mean that the fellow actually sends extended timestamp,
                 * that depends on the actual value
                 */
                fellow->capabilities.Y2038Ready = (((c_octet *)cap->value)[0] == 0) ? FALSE : TRUE;
                has_y2038_capability = TRUE;
            }
            /* Any other capability that is send to me I cannot understand, I simply ignore it. */
        } /* for */

        /* If a fellow does not specify an incarnation, then the best guess
         * is to take the new incarnation as the increment of the incarnation
         */
        if (!incarnation_present) {
            incarnation = fellow->capabilities.incarnation+1;
        }

        /* Now incarnation contains the new incarnation, even in case the received
         * capabilities did not contain an incarnation at all
         */

        /* Incarnations are assumed to be monotonically increasing (modulo wrapping) */
        progress = ((incarnation > fellow->capabilities.incarnation) && (incarnation - fellow->capabilities.incarnation < thres)) ||
                   ((incarnation < fellow->capabilities.incarnation) && (incarnation > thres - fellow->capabilities.incarnation));


        /* Detect asymmetric disconnect, ignore the first time the fellow was detected
         * Account for possible wrapping, see https://en.wikipedia.org/wiki/Serial_number_arithmetic
         */
        if ( (fellow->capabilities.incarnation != 0) && progress) {
            /* A new incarnation is received, meaning the fellow has disconnected from
             * me and reconnected to me without me noticing it. This is an asymmetrical
             * disconnect
             */

            d_printTimedEvent(durability, D_LEVEL_FINE,
                 "Fellow %u was asymmetrically disconnected from me, I may have published data that was missed by the fellow.\n",
                  fellow->address->systemId);

            check_for_conflicts = TRUE;

            /* Drop all knowledge of the fellow's namespaces to not interact with it until
             * after it has responded to our namespace request
             */
            if (fellow->nameSpaces) {
                d_tableFree(fellow->nameSpaces);
                fellow->nameSpaces = NULL; /* New initializes it lazily, too */
            }
        }

        /* To prevent stuttering we only update the incarnation state in case progress was made */
        if (progress) {
            fellow->capabilities.incarnation = incarnation;
        }

        d_lockUnlock(d_lock(fellow));

        /* I received a new capability from the fellow
         * so I may have to send my capability back
         */
        if (progress) {
            d_fellowCheckSendCapabilities(fellow, FALSE);
        }

        /* If this fellow does NOT advertise the D_CAPABILITY_Y2038READY
         * capability, then this fellow must be a legacy fellow that is
         * not capable to understand time stamps beyond 2038. In that case
         * we may not advertise 64-bit times. If we do, then we have an
         * invalid system configuration, but continue anyway.
         */
        if ((!has_y2038_capability) && (durability->configuration->capabilityY2038Ready)) {
            /* I advertise timestamps beyond 2038. In
             * combination with legacy fellows this is
             * considered an invalid configuration
             */
            OS_REPORT(OS_FATAL, D_CONTEXT, 0,
                    "I advertise timestamps that go beyond 2038, but federation with "\
                    "systemid %"PA_PRIu32" can only understand legacy timestamps.\n" \
                    "This is considered an invalid configuration, see the user manual for information",
                    fellow->address->systemId);
        }
        if (check_for_conflicts) {
            /* Schedule a conflict to resolve the asymmetric disconnect */
            d_conflictMonitorCheckFellowDisconnected(conflictMonitor, fellow);
            d_fellowSendNSRequest(fellow);
        }
    }
    return;
}


/**
 * \brief Check if a fellow supports capabilities
 *
 * @return TRUE if the fellow's capabilitySupport attribute is set to TRUE and the fellow is confirmed
 *         FALSE, otherwise
 *
 * Note that unconfirmed fellows are never assumed to have capabilitySupport.
 */
c_bool
d_fellowHasCapabilitySupport(
    d_fellow fellow)
{
    assert(d_fellowIsValid(fellow));

    return fellow->capabilitySupport;
}


/**
 * \brief Returns TRUE if all expected readers of the fellow have been discovered,
 *              and (fellow support capabilities => I must have received its capabilities)
 *
 * If there is no need to wait for remote readers then always TRUE is returned
 */
c_bool
d_fellowIsResponsive(
    d_fellow fellow,
    c_bool waitForRemoteReaders)
{
    d_durability durability = d_threadsDurability();
    d_configuration config = d_durabilityGetConfiguration(durability);
    c_bool result = TRUE;

    assert(d_fellowIsValid(fellow));

    /* All remote readers must have been discovered when ddsi is used */
    if (!d_fellowHasDiscoveredReaders(fellow, fellow->requiredReaders, waitForRemoteReaders)) {
        result = FALSE;

    /* When capabilities are supported the fellow's capabilities must have been received */
    } else if (config->capabilitySupport) {
        /* I have capability support
         * If the fellow has capability support then check if the capabilities are received
         */
        if (d_fellowHasCapabilitySupport(fellow)) {
            /* The fellow must have send me his capabilities at least once */
            result = (fellow->capabilities.incarnation > 0);
        }
    }
    return result;
}


/**
 * \brief Return if the fellow supports the capability to calculate group hashes
 */
c_bool
d_fellowHasCapabilityGroupHash (
    d_fellow fellow)
{
    assert(d_fellowIsValid(fellow));

    if (fellow->capabilitySupport) {
        return fellow->capabilities.groupHash;
    } else {
        return FALSE;
    }
}


/**
 * \brief Return if the fellow supports the capability to support alignment of EOTs
 */
c_bool
d_fellowHasCapabilityEOTSupport (
    d_fellow fellow)
{
    assert(d_fellowIsValid(fellow));

    if (fellow->capabilitySupport) {
        return fellow->capabilities.EOTSupport;
    } else {
        return FALSE;
    }
}


/**
 * \brief Return the fellow its masterSelectionVersion
 */
c_ulong
d_fellowGetCapabilityMasterSelection (
    d_fellow fellow)
{
    assert(d_fellowIsValid(fellow));

    if (fellow->capabilitySupport) {
        return fellow->capabilities.masterSelection;
    } else {
        return 0;
    }
}


/**
 * \brief Return true if the fellow can accept extended timestamps
 */
c_bool
d_fellowHasCapabilityY2038Ready (
    d_fellow fellow)
{
    assert(d_fellowIsValid(fellow));

    if (fellow->capabilitySupport) {
        return fellow->capabilities.Y2038Ready;
    } else {
        return FALSE;
    }
}


/**
 * \brief Request nameSpaces from fellow
 */
void
d_fellowSendNSRequest(
    d_fellow fellow)
{
    d_durability durability;
    d_admin admin;
    d_publisher publisher;
    d_nameSpacesRequest request;

    assert(d_fellowIsValid(fellow));

    durability = d_threadsDurability();
    admin = durability->admin;
    publisher = d_adminGetPublisher(admin);

    d_printTimedEvent(durability, D_LEVEL_FINE,
        "Requesting nameSpaces from fellow %u.\n",
        fellow->address->systemId);
    request = d_nameSpacesRequestNew(admin);
    d_messageSetAddressee(d_message(request), fellow->address);
    d_publisherNameSpacesRequestWrite(publisher, request, fellow->address, d_durabilityGetState(durability));
    d_nameSpacesRequestFree(request);
}


/* Send my capabilities
 *
 * The initial parameter indicates whether the function is called to send out the
 * initial capability, or whether a capability has to be sent out because an
 * asymmetrical disconnect was detected.
 */
void
d_fellowCheckSendCapabilities(
    d_fellow fellow, c_bool initial)
{
    d_durability durability;
    d_admin admin;
    d_publisher publisher;
    d_capability capability;
    d_configuration config;

    assert(d_fellowIsValid(fellow));

    durability = d_threadsDurability();
    admin = durability->admin;
    publisher = d_adminGetPublisher(admin);
    config = d_durabilityGetConfiguration(durability);

    /* Ensure that capabilities are sent once to the fellow
     * Capabilities may only be sent when the fellow is past its initializing state
     */
    d_lockLock(d_lock(fellow));

    /* skip if request is initial, but an incarnation has already been sent to the fellow */
    if (initial && (fellow->incarnation != 0)) {
        goto skip;
    }

    /* skip if I do not support capabilities */
    if (!config->capabilitySupport) {
        goto skip;
    }

    /* skip if fellow does not support capabilities */
    if (!fellow->capabilitySupport) {
        goto skip;
    }

    /* skip if the fellow's capability reader has not yet been discovered in case ddsi is being used*/
    if ((config->waitForRemoteReaders) &&
        ((fellow->readers & D_CAPABILITY_READER_FLAG) != D_CAPABILITY_READER_FLAG)) {
        goto skip;
    }

    /* skip if fellow is still initializing */
    if ((d_fellowGetState(fellow) == D_STATE_INIT) || d_fellowGetState(fellow) >= D_STATE_TERMINATING) {
        goto skip;
    }

    /* skip if I am still initializing */
    if ((d_durabilityGetState(durability) == D_STATE_INIT) || d_fellowGetState(fellow) >= D_STATE_TERMINATING) {
        goto skip;
    }

    d_printTimedEvent(durability, D_LEVEL_FINE, "Sending my capabilities to fellow %u.\n",
        fellow->address->systemId);

    /* Set the incarnation that I must sent to the fellow
     * Determine a new value when no incarnation is set before
     */
    if (fellow->incarnation == 0) {
        fellow->incarnation = d_durabilityGetNewIncarnation(durability);
    }

    /* Send the capability */
    capability = d_capabilityNew(admin, fellow->incarnation);
    d_messageSetAddressee(d_message(capability), fellow->address);
    d_publisherCapabilityWrite(publisher, capability, fellow->address);
    d_capabilityFree(capability);

    d_lockUnlock(d_lock(fellow));
    return;

skip:
    d_lockUnlock(d_lock(fellow));
    return;
}

os_uint32
d_fellowGetLastSeqNum(
    d_fellow fellow)
{
    os_uint32 lastSeqNum;

    assert(d_fellowIsValid(fellow));

    lastSeqNum = fellow->lastSeqNum;
    return lastSeqNum;
}

c_bool
d_fellowHasRecentlyJoined(
    d_fellow fellow)
{
    c_bool result;

    assert(d_fellowIsValid(fellow));

    d_lockLock(d_lock(fellow));
    result = fellow->recently_joined;
    d_lockUnlock(d_lock(fellow));
    return result;
}

void
d_fellowCheckInitialResponsiveness(
    d_fellow fellow)
{
    d_durability durability = d_threadsDurability();
    d_configuration config = d_durabilityGetConfiguration(durability);

    d_lockLock(d_lock(fellow));
    if (d_fellowIsResponsive(fellow, config->waitForRemoteReaders) && (!fellow->responsive)) {
        /* The fellow has become responsive for the first time, so I have discovered
         * its readers and capabilities. We can safely sent a namespacesRequest to
         * the fellow now to start the durability protocol
         */
        fellow->responsive = TRUE;
        d_printTimedEvent(durability, D_LEVEL_FINEST,
            "Fellow %u became responsive, I am going to send a request for namespaces to this fellow\n",
            fellow->address->systemId);
        d_fellowSendNSRequest(fellow);
        /* If the fellow already sent a request for namespaces, then send the namespaces now */
        if (fellow->has_requested_namespaces) {
            /* Create a request to send namesSpaces to the fellow */
            d_admin admin = durability->admin;
            d_publisher publisher = d_adminGetPublisher(admin);
            d_networkAddress myAddr = d_adminGetMyAddress(admin);
            d_nameSpacesRequest request;

            d_printTimedEvent(durability, D_LEVEL_FINE,
                "Going to answer cached namespace request to fellow %u.\n", fellow->address->systemId);
            request = d_nameSpacesRequestNew(admin);
            d_messageSetAddressee(d_message(request), myAddr);               /* Send the nameSpacesRequest to myself. */
            d_messageSetSenderAddress(d_message(request), fellow->address);  /* Pretend that the request to send the nameSpaces came from (0,0,0). */
            d_publisherNameSpacesRequestWrite(publisher, request, fellow->address, d_fellowGetState(fellow));
            d_nameSpacesRequestFree(request);
            d_networkAddressFree(myAddr);
        }
    }
    d_lockUnlock(d_lock(fellow));

}

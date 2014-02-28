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

#include "d__fellow.h"
#include "d_fellow.h"
#include "d_table.h"
#include "d_group.h"
#include "d_nameSpace.h"
#include "d_configuration.h"
#include "d_admin.h"
#include "d_networkAddress.h"
#include "v_time.h"
#include "os_heap.h"
#include "os_stdlib.h"

struct findNameSpace{
    d_nameSpace template;
    d_nameSpace nameSpace;
};

struct alignerHelper{
    const c_char* partition;
    const c_char* topic;
    d_durabilityKind kind;
    c_bool aligner;
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
        "D_STATE_TERMINATED"};

d_fellow
d_fellowNew(
    d_networkAddress address,
    d_serviceState state)
{
    d_fellow fellow;

    fellow = d_fellow(os_malloc(C_SIZEOF(d_fellow)));
    d_lockInit(d_lock(fellow), D_FELLOW, d_fellowDeinit);

    if(fellow){
        fellow->address = d_networkAddressNew(address->systemId,
                                              address->localId,
                                              address->lifecycleId);

        fellow->communicationState  = D_COMMUNICATION_STATE_UNKNOWN;
        fellow->state               = state;
        fellow->lastStatusReport    = C_TIME_INFINITE;
        fellow->groups              = NULL; /*Lazy initialization*/
        fellow->nameSpaces          = NULL; /*Lazy initialization*/
        fellow->requestCount        = 0;
        fellow->expectedGroupCount  = -1;
        fellow->expectedNameSpaces  = 1; /*At lease one namespace is expected*/
        fellow->groupsRequested     = FALSE;
        fellow->role                = NULL;
    }
    return fellow;
}

/** This function sets groupsRequested to TRUE if it's not TRUE already and
 *  returns whether the boolean has been changed by this call.
 *
 * @return TRUE if groupsRequested was set to TRUE and FALSE if it was set already.
 */
c_bool
d_fellowSetGroupsRequested(
    d_fellow fellow)
{
    c_bool result;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->groupsRequested){
            result = FALSE;
        } else {
            fellow->groupsRequested = TRUE;
            result = TRUE;
        }
        d_lockUnlock(d_lock(fellow));
    } else {
        result = FALSE;
    }
    return result;
}

c_bool
d_fellowGetGroupsRequested(
    d_fellow fellow)
{
    c_bool result;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockLock(d_lock(fellow));
        result = fellow->groupsRequested;
        d_lockUnlock(d_lock(fellow));
    } else {
        result = FALSE;
    }
    return result;
}

d_networkAddress
d_fellowGetAddress(
    d_fellow fellow)
{
    d_networkAddress address = NULL;
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
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
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        fellow->address->systemId    = address->systemId;
        fellow->address->localId     = address->localId;
        fellow->address->lifecycleId = address->lifecycleId;
    }
    return;
}

void
d_fellowDeinit(
    d_object object)
{
    d_fellow fellow;

    assert(d_objectIsValid(object, D_FELLOW) == TRUE);

    if(object){
        fellow = d_fellow(object);

        if(fellow->groups){
            d_tableFree(fellow->groups);
            fellow->groups = NULL;
        }
        if(fellow->address){
            d_networkAddressFree(fellow->address);
            fellow->address = NULL;
        }
        if(fellow->nameSpaces){
            d_tableFree(fellow->nameSpaces);
            fellow->nameSpaces = NULL;
        }
        if (fellow->role) {
            os_free (fellow->role);
            fellow->role = NULL;
        }
    }
}

void
d_fellowFree(
    d_fellow fellow)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        d_lockFree(d_lock(fellow), D_FELLOW);
    }
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
    d_timestamp timestamp)
{
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
       d_lockLock(d_lock(fellow));
       fellow->lastStatusReport.seconds = timestamp.seconds;
       fellow->lastStatusReport.nanoseconds = timestamp.nanoseconds;
       fellow->state = state;
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
    c_ulong count)
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

    if(fellow){
        d_lockLock(d_lock(fellow));

        if(fellow->groups){
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
    d_group found;
    d_quality quality;
    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    found = NULL;

    if(fellow){
        quality.seconds = 0;
        quality.nanoseconds = 0;
        dummy = d_groupNew(partition, topic, kind, D_GROUP_KNOWLEDGE_UNDEFINED, quality);
        d_lockLock(d_lock(fellow));

        if(fellow->groups){
            found = d_tableFind(fellow->groups, dummy);

            if(found){
                d_objectKeep(d_object(found));
            }
        }
        d_lockUnlock(d_lock(fellow));
        d_groupFree(dummy);
    }
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
    int result;

    result = 0;

    if (fellow1 != fellow2) {
        if(fellow1->address->systemId < fellow2->address->systemId){
            result = -1;
        } else if(fellow1->address->systemId > fellow2->address->systemId){
            result = 1;
        } else {
            if(fellow1->address->localId < fellow2->address->localId){
                result = -1;
            }  else if(fellow1->address->localId > fellow2->address->localId){
                result = 1;
            } else {
                if(fellow1->address->lifecycleId < fellow2->address->lifecycleId){
                    result = -1;
                } else if(fellow1->address->lifecycleId > fellow2->address->lifecycleId){
                    result = 1;
                } else {
                    result = 0;
                }
            }
        }
    }
    return result;
}

d_timestamp
d_fellowGetLastStatusReport(
    d_fellow fellow)
{
    d_timestamp s;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow) {
       s.seconds = fellow->lastStatusReport.seconds;
       s.nanoseconds = fellow->lastStatusReport.nanoseconds;
    }else {
        s.seconds = 0;
        s.nanoseconds = 0;
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

    if(fellow) {
        fellow->communicationState = state;
    }
}

c_bool
d_fellowAddNameSpace(
    d_fellow fellow,
    d_nameSpace nameSpace)
{
    c_bool result;
    d_nameSpace duplicate;
    d_networkAddress master;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);
    assert(d_objectIsValid(d_object(nameSpace), D_NAMESPACE) == TRUE);

    result = FALSE;

    if(fellow && nameSpace){
        d_lockLock(d_lock(fellow));

        if(!fellow->nameSpaces){
            fellow->nameSpaces = d_tableNew(d_nameSpaceNameCompare, d_nameSpaceFree);
        }
        duplicate = d_tableInsert(fellow->nameSpaces, nameSpace);

        if(duplicate){
            master = d_nameSpaceGetMaster(nameSpace);
            d_nameSpaceSetMaster(duplicate, master);
            d_networkAddressFree(master);
        }
        d_lockUnlock(d_lock(fellow));

        if(!duplicate){
            result = TRUE;
        }
    }
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

    return (!helper->aligner);
}

d_fellowAlignStatus
d_fellowIsAlignerForGroup(
    d_fellow fellow,
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind)
{
    struct alignerHelper helper;
    d_fellowAlignStatus status;

    assert(d_objectIsValid(d_object(fellow), D_FELLOW) == TRUE);

    if(fellow){
        helper.partition = partition;
        helper.topic     = topic;
        helper.kind      = kind;
        helper.aligner   = FALSE;

        d_lockLock(d_lock(fellow));

        if(fellow->nameSpaces){
            d_tableWalk(fellow->nameSpaces, isAligner, &helper);

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
    match = d_nameSpaceCompatibilityCompare(nameSpace, helper->nameSpace);


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

        fellow->role = os_strdup(role);

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



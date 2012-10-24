/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#include <string.h>

#include "d_misc.h"
#include "d_table.h"
#include "d_admin.h"
#include "d__nameSpace.h"
#include "d_nameSpace.h"
#include "d_policy.h"
#include "d__policy.h"
#include "d_nameSpaces.h"
#include "d_networkAddress.h"
#include "d_configuration.h"
#include "d__mergeState.h"
#include "os_report.h"
#include "os_stdlib.h"

/**************************************************************
 * Private functions
 **************************************************************/

static int
elementCompare(
    c_voidp  object1,
    c_voidp  object2 )
{
    d_element element1, element2;
    int       result;

    result = 0;
    element1 = d_element(object1);
    element2 = d_element(object2);
    if (element1 != element2) {
        result = strncmp(element1->topic, element2->topic, element2->strlenTopic);
        if (result == 0) {
            result = strncmp(element1->partition, element2->partition, element2->strlenPartition);
        }
    }
    return result;
}

/**************************************************************/

static void
elementFree(
    d_element element )
{
    d_free(element->name);
    d_free(element->partition);
    d_free(element->topic);
    d_free(element);
}

/**************************************************************/

static d_element
elementNew(
    const char * name,
    const char * partition,
    const char * topic )
{
    d_element element;

    element = (d_element)d_malloc((os_uint32)C_SIZEOF(d_element), "element");
    if (element) {
        /* QAC EXPECT 1253; */
        element->strlenName      = strlen(name) + 1;
        /* QAC EXPECT 1253; */
        element->strlenPartition = strlen(partition) + 1;
        /* QAC EXPECT 1253; */
        element->strlenTopic     = strlen(topic) + 1;
        element->name      = (char *)d_malloc(element->strlenName, "name");
        element->partition = (char *)d_malloc(element->strlenPartition, "partition");
        element->topic     = (char *)d_malloc(element->strlenTopic, "topic");
        if ((element->name == NULL) || (element->partition == NULL) || (element->topic == NULL)) {
            elementFree(element);
            element = NULL;
        } else {
            /* QAC EXPECT 5007; use of strcpy */
            os_strcpy(element->name, name);
            /* QAC EXPECT 5007; use of strcpy */
            os_strcpy(element->partition, partition);
            /* QAC EXPECT 5007; use of strcpy */
            os_strcpy(element->topic, topic);
        }
    }
    return element;
}

/**************************************************************/

/* PRE: partition and topic are correct values */
static void
d_nameSpaceAddPartitionTopic(
    d_nameSpace  nameSpace,
    const char * name,
    const char * partition,
    const char * topic )
{
    d_element element;
    d_element found;

    element = elementNew(name, partition, topic);
    if (element) {
        found = d_tableInsert(nameSpace->elements, element);
        if (found) {
            elementFree(element); /* this was not unique */
        }
    }
}

/**************************************************************/



/**************************************************************/

static c_bool
nameSpaceIsIn(
    c_object object,
    c_voidp  actionArg )
{
    c_bool            continueTheSearch;
    d_element         element;
    d_nameSpaceSearch nameSpaceSearch;

    continueTheSearch = TRUE;
    nameSpaceSearch = (d_nameSpaceSearch)actionArg;
    element = d_element(object);

    if (d_patternMatch(nameSpaceSearch->partition, element->partition) &&
        d_patternMatch(nameSpaceSearch->topic, element->topic)){
        nameSpaceSearch->match = TRUE;
        continueTheSearch = FALSE;
    }
    return continueTheSearch;
}

/**************************************************************/

static c_bool
isANameSpace(
    d_nameSpace nameSpace )
{
    return d_objectIsValid(d_object(nameSpace), D_NAMESPACE);
}

/**************************************************************
 * Public functions
 **************************************************************/

int
d_nameSpaceCompare(
    d_nameSpace ns1,
    d_nameSpace ns2)
{
    int r;
    c_char* partitions1;
    c_char* partitions2;
    d_policy p1;
    d_policy p2;



    if((!ns1) && (!ns2)){
        r = 0;
    } else if(!ns1){
        r = -1;
    } else if(!ns2){
        r = 1;
    }else
    {
        p1 = ns1->policy;
        p2 = ns2->policy;

        if (p1->aligner && !(p2->aligner)){
            r = 1;
        }else if (!(p1->aligner) && p2->aligner){
            r = -1;
        }else if(p1->alignmentKind != p2->alignmentKind){
            if(((c_ulong)p1->alignmentKind) > ((c_ulong)p2->alignmentKind)){
                r = 1;
            } else {
                r = -1;
            }
        } else if(p1->durabilityKind != p2->durabilityKind) {
            if(((c_ulong)p1->durabilityKind) > ((c_ulong)p2->durabilityKind)){
                r = 1;
            } else {
                r = -1;
            }
        } else if((!ns1->elements) && (!ns2->elements)){
            r = 0;
        } else if(!ns1->elements){
            r = -1;
        } else if(!ns2->elements){
            r = 1;
        } else {
            partitions1 = d_nameSpaceGetPartitions(ns1);
            partitions2 = d_nameSpaceGetPartitions(ns2);
            r = strcmp(partitions1, partitions2);
            os_free(partitions1);
            os_free(partitions2);
        }
    }

    return r;
}

int
d_nameSpaceCompatibilityCompare(
    d_nameSpace ns1,
    d_nameSpace ns2)
{
    int r;
    c_char* partitions1;
    c_char* partitions2;

    if((!ns1) && (!ns2)){
        r = 0;
    } else if(!ns1){
        r = -1;
    } else if(!ns2){
        r = 1;
    }
    else if((!ns1->elements) && (!ns2->elements)){
        r = 0;
    } else if(!ns1->elements){
        r = -1;
    } else if(!ns2->elements){
        r = 1;
    } else {
        partitions1 = d_nameSpaceGetPartitions(ns1);
        partitions2 = d_nameSpaceGetPartitions(ns2);
        r = strcmp(partitions1, partitions2);
        os_free(partitions1);
        os_free(partitions2);
    }
    return r;
}

struct d_nsCompare {
    d_nameSpace ns1;
    d_nameSpace ns2;
    d_nsEquality equality;
};

char *
d_nameSpaceGetName(
    d_nameSpace  nameSpace )
{
    char * name;

    name = NULL;
    if (isANameSpace(nameSpace)) {
        name = nameSpace->name;
    }
    return name;
}

d_policy
d_nameSpaceGetPolicy(
    d_nameSpace  nameSpace )
{
    d_policy policy;

    policy = NULL;

    if (isANameSpace(nameSpace)) {
        policy = nameSpace->policy;
    }
    return policy;
}

c_bool
d_nameSpaceIsAligner(
    d_nameSpace nameSpace)
{
    c_bool result = FALSE;

    if (isANameSpace(nameSpace)) {
        result = d_policyGetAligner(nameSpace->policy);
    }
    return result;
}

d_alignmentKind
d_nameSpaceGetAlignmentKind(
    d_nameSpace nameSpace)
{
    d_alignmentKind kind = D_ALIGNEE_INITIAL;

    if (isANameSpace(nameSpace)) {
        kind = d_policyGetAlignmentKind (nameSpace->policy);
    }
    return kind;
}


c_bool
d_nameSpaceIsAlignmentNotInitial(
    d_nameSpace nameSpace)
{
    c_bool notInitial;
    d_alignmentKind alignmentKind;

    alignmentKind = d_policyGetAlignmentKind (nameSpace->policy);

    if (isANameSpace(nameSpace)) {
        switch(alignmentKind){
        case D_ALIGNEE_INITIAL:
            notInitial = FALSE;
            break;
        case D_ALIGNEE_LAZY:
        /*fallthrough intentional*/
        case D_ALIGNEE_ON_REQUEST:
            notInitial = TRUE;
            break;
        default:
            OS_REPORT_1(OS_ERROR, D_CONTEXT, 0,
                        "Unknown alignment kind '%d' found",
                        alignmentKind);
            notInitial = FALSE;
            assert(FALSE);
            break;
        }
    } else {
        notInitial = FALSE;
    }
    return notInitial;
}

d_durabilityKind
d_nameSpaceGetDurabilityKind(
    d_nameSpace nameSpace)
{
    d_durabilityKind kind = D_DURABILITY_ALL;

    if (isANameSpace(nameSpace)) {
        kind = d_policyGetDurabilityKind (nameSpace->policy);
    }
    return kind;
}

/**************************************************************/

/* EXPECTING:
    - (partitionTopic = <partitionName>.<topicName>) AND (topicGiven == NULL)
    OR
    - (partitionTopic = <partitionName>)             AND (topicGiven = <topicName>)

    both <partitionName> and <topicName> may contain wildcards ('?' or'*')
*/
void
d_nameSpaceAddElement(
    d_nameSpace  nameSpace,
    const char * name,
    const char * partitionTopic,
    const char * topicGiven )
{
    char *  partition;
    char *  topic;
    os_uint32 strlenPartitionTopic;
    os_uint32 strlenTopicGiven;

    /* QAC EXPECT 1253; */
    strlenPartitionTopic = strlen(partitionTopic) + 1;
    if (isANameSpace(nameSpace) &&
        /* QAC EXPECT 1253; */
        (strlenPartitionTopic < D_MAX_STRLEN_NAMESPACE)) {
        if (topicGiven) {
            /* QAC EXPECT 1253; */
            strlenTopicGiven = strlen(topicGiven) + 1;
            /* QAC EXPECT 1253; */
            if (strlenTopicGiven < D_MAX_STRLEN_NAMESPACE) {
                d_nameSpaceAddPartitionTopic(nameSpace, name, partitionTopic, topicGiven);
            }
        } else {
            partition = (char *)d_malloc(strlenPartitionTopic, "Partition");
            if (partition) {
               os_strncpy(partition, partitionTopic, strlenPartitionTopic);
                topic = partition;

                /* QAC EXPECT 2106,3123; */
                while ((*topic != '.') && (*topic != 0)) {
                    /* QAC EXPECT 0489; */
                    topic++;
                }
                /* QAC EXPECT 2106,3123; */
                if (*topic == '.') {
                    *topic = 0;
                    /* QAC EXPECT 0489; */
                    topic++;
                    /* QAC EXPECT 2106; */
                    if (*topic != 0) {
                        d_nameSpaceAddPartitionTopic(nameSpace, name, partition, topic);
                    }
                }
                d_free(partition);
            }
        }
    }
}

/**************************************************************/

c_bool
d_nameSpaceIsEmpty(
    d_nameSpace nameSpace )
{
    c_bool empty = TRUE;

    if (isANameSpace(nameSpace)) {
        if(d_tableSize(nameSpace->elements)) {
            empty = FALSE;
        }
    }
    return empty;
}

/**************************************************************/

c_bool
d_nameSpaceIsIn(
    d_nameSpace nameSpace,
    d_partition partition,
    d_topic     topic )
{
    c_bool                      match;
    C_STRUCT(d_nameSpaceSearch) nameSpaceSearch;

    match = FALSE;
    if (isANameSpace(nameSpace)) {
        nameSpaceSearch.match = FALSE;
        nameSpaceSearch.partition = partition;
        nameSpaceSearch.topic = topic;
        d_tableWalk(nameSpace->elements, nameSpaceIsIn, &nameSpaceSearch);
        match = nameSpaceSearch.match;
    }
    return match;
}

/**************************************************************/
typedef struct policyWalkData
{
    const char* nameSpace;
    d_policy policy_out;
}policyWalkData;

void
policyWalk (
    void* o,
    c_iterActionArg userData)
{
    d_policy policy = d_policy(o);
    policyWalkData* walkData = (policyWalkData*)userData;

    if (!(walkData->policy_out)) {
        if (d_patternMatch((c_string)walkData->nameSpace, (c_string)policy->nameSpace)) {
            walkData->policy_out = policy;
        }
    }
}

d_policy
d_nameSpaceFindPolicy (
    d_configuration config,
    const char* nameSpace)
{
    policyWalkData walkData;

    walkData.nameSpace = nameSpace;
    walkData.policy_out = NULL;

    c_iterWalk (config->policies, policyWalk, &walkData);

    return walkData.policy_out;
}

/**************************************************************/

void
d_nameSpaceDeinit(
    d_object object)
{
    d_nameSpace nameSpace;

    nameSpace = d_nameSpace(object);

    if (isANameSpace(nameSpace)) {
        d_networkAddressFree(nameSpace->master);
        d_tableFree(nameSpace->elements); /* frees all */
        nameSpace->elements = NULL;
        d_tableFree(nameSpace->mergedRoleStates);
        nameSpace->mergedRoleStates = NULL;
        d_mergeStateFree (nameSpace->mergeState);
        d_free(nameSpace->name);

        d_policyFree(nameSpace->policy);
    }
}

void
d_nameSpaceFree(
    d_nameSpace nameSpace )
{
    if (isANameSpace(nameSpace)) {
        d_lockFree(d_lock(nameSpace), D_NAMESPACE);
    }
}

/**************************************************************/

/* Failure will result in the service quitting */
d_nameSpace
d_nameSpaceNew(
    d_configuration config,
    const char * name)
{
    d_nameSpace     nameSpace;
    d_policy        policy;

    nameSpace = NULL;
    assert (name);

    policy = d_nameSpaceFindPolicy (config, name);
    if (policy)
    {
        d_objectKeep (d_object(policy)); /* Explicit keep of policy */

        /* Create namespace object */
        nameSpace = d_nameSpace(d_malloc((os_uint32)C_SIZEOF(d_nameSpace), "NameSpace"));

        if(nameSpace != NULL){
            d_lockInit(d_lock(nameSpace), D_NAMESPACE, d_nameSpaceDeinit);
            /* QAC EXPECT 3892; */

            nameSpace->name = os_strdup(name);
            nameSpace->policy               = policy;
            nameSpace->elements             = d_tableNew(elementCompare, elementFree);
            nameSpace->quality.seconds      = 0;
            nameSpace->quality.nanoseconds  = 0;
            nameSpace->master               = d_networkAddressUnaddressed();
            nameSpace->masterState			= D_STATE_COMPLETE;
            nameSpace->masterConfirmed		= FALSE;
            nameSpace->mergeState           = d_mergeStateNew(config->role, 0);
            nameSpace->mergedRoleStates     = d_tableNew(d_mergeStateCompare, d_mergeStateFree);
         }
    }

    return nameSpace;
}

/* Failure will result in the service quitting */
/* Create namespace with private policy */
d_nameSpace
d_nameSpaceNew_w_policy(
    d_configuration config,
    const char * name,
    c_bool aligner,
    d_alignmentKind alignmentKind,
    d_durabilityKind durabilityKind)
{
    d_nameSpace    nameSpace;

    nameSpace = d_nameSpace(d_malloc((os_uint32)C_SIZEOF(d_nameSpace), "NameSpace"));

    if(nameSpace != NULL){
        d_lockInit(d_lock(nameSpace), D_NAMESPACE, d_nameSpaceDeinit);
        /* QAC EXPECT 3892; */

        if(name != NULL){
            nameSpace->name = os_strdup(name);
        } else {
            nameSpace->name = os_strdup("NoName");
        }
        nameSpace->policy               = d_policyNew (name, aligner, alignmentKind, durabilityKind);
        nameSpace->elements             = d_tableNew(elementCompare, elementFree);
        nameSpace->quality.seconds      = 0;
        nameSpace->quality.nanoseconds  = 0;
        nameSpace->master               = d_networkAddressUnaddressed();
        nameSpace->masterState          = D_STATE_COMPLETE;
        nameSpace->masterConfirmed      = FALSE;
        nameSpace->mergeState           = d_mergeStateNew(config->role, 0);
        nameSpace->mergedRoleStates     = d_tableNew(d_mergeStateCompare, d_mergeStateFree);
    }

    return nameSpace;
}

static c_bool
insertMergeState(
    d_mergeState state,
    d_table table)
{
    d_tableInsert(table, d_mergeStateNew(state->role, state->value));
    return TRUE;
}

d_nameSpace
d_nameSpaceCopy(
    d_nameSpace ns)
{
    d_nameSpace nameSpace;

    nameSpace = d_nameSpace(d_malloc((os_uint32)C_SIZEOF(d_nameSpace), "NameSpace"));

    if(nameSpace != NULL){
        d_lockInit(d_lock(nameSpace), D_NAMESPACE, d_nameSpaceDeinit);
        /* QAC EXPECT 3892; */

        d_objectKeep (d_object(ns->policy));
        d_objectKeep (d_object(ns->elements));

        nameSpace->name                 = os_strdup(ns->name);
        nameSpace->policy               = ns->policy;
        nameSpace->elements             = ns->elements;
        nameSpace->quality.seconds      = ns->quality.seconds;
        nameSpace->quality.nanoseconds  = ns->quality.nanoseconds;
        nameSpace->master               = d_networkAddressNew(ns->master->systemId, ns->master->localId,
                                                              ns->master->lifecycleId);
        nameSpace->masterState          = ns->masterState;
        nameSpace->masterConfirmed      = ns->masterConfirmed;
        nameSpace->mergeState           = d_mergeStateNew(ns->mergeState->role, ns->mergeState->value);
        nameSpace->mergedRoleStates     = d_tableNew(d_mergeStateCompare, d_mergeStateFree);
        d_tableWalk(ns->mergedRoleStates, insertMergeState, nameSpace->mergedRoleStates);
    }

    return nameSpace;
}

void
d_nameSpaceReplaceMergeStates(
    d_nameSpace ns1,
    d_nameSpace ns2)
{
    d_tableFree (ns1->mergedRoleStates);

    ns1->mergedRoleStates = d_tableNew(d_mergeStateCompare, d_mergeStateFree);
    d_tableWalk(ns2->mergedRoleStates, insertMergeState, ns1->mergedRoleStates);
}

d_nameSpace
d_nameSpaceFromNameSpaces(
    d_configuration config,
    d_nameSpaces ns)
{
    d_nameSpace nameSpace = NULL;
    c_char *partitions, *temp;
    d_quality quality;
    c_bool aligner;
    d_alignmentKind alignmentKind;
    d_durabilityKind durabilityKind;
    os_uint32 i;

    if(ns){
        nameSpace = d_nameSpace(d_malloc((os_uint32)C_SIZEOF(d_nameSpace), "NameSpace"));
        if(nameSpace != NULL){
            d_lockInit(d_lock(nameSpace), D_NAMESPACE, d_nameSpaceDeinit);

            aligner = d_nameSpacesIsAligner(ns);
            alignmentKind = d_nameSpacesGetAlignmentKind(ns);
            durabilityKind = d_nameSpacesGetDurabilityKind(ns);

            if (d_nameSpacesGetName(ns)) {
                nameSpace->name            = os_strdup(d_nameSpacesGetName(ns));
            }else {
                nameSpace->name            = NULL;
            }
            nameSpace->policy              = d_policyNew (
                                                NULL,
                                                aligner,
                                                alignmentKind,
                                                durabilityKind);
            quality                        = d_nameSpacesGetInitialQuality(ns);
            nameSpace->quality.seconds     = quality.seconds;
            nameSpace->quality.nanoseconds = quality.nanoseconds;
            nameSpace->master              = d_networkAddressNew(
                                                    ns->master.systemId,
                                                    ns->master.localId,
                                                    ns->master.lifecycleId);
            nameSpace->masterState		   = D_STATE_COMPLETE;
            nameSpace->elements            = d_tableNew(elementCompare, elementFree);

            partitions = d_nameSpacesGetPartitions(ns);
            temp = strtok(partitions, ",");

            while(temp){
                d_nameSpaceAddElement(nameSpace, "element", temp, "*");
                temp = strtok(NULL, ",");
            }
            os_free(partitions);

            nameSpace->masterConfirmed = ns->masterConfirmed;

            /* Add merge state for own role */
            nameSpace->mergeState = d_mergeStateNew (ns->state.role, 0);

            /* Add merge states for other roles */
            nameSpace->mergedRoleStates = d_tableNew(d_mergeStateCompare, d_mergeStateFree);
            d_nameSpaceSetMergeState(nameSpace, &(ns->state));
            for(i=0; i<ns->mergedStatesCount; i++){
                d_nameSpaceSetMergeState(nameSpace, &((d_mergeState)(ns->mergedStates))[i]);
            }
        }
    }
    return nameSpace;
}

void
d_nameSpaceSetMasterState(
		d_nameSpace nameSpace,
		d_serviceState serviceState)
{
	if (isANameSpace(nameSpace)) {
		d_lockLock (d_lock(nameSpace));
		nameSpace->masterState = serviceState;
		d_lockUnlock(d_lock(nameSpace));
	}
	return;
}

d_serviceState
d_nameSpaceGetMasterState(
		d_nameSpace nameSpace)
{
	d_serviceState masterState;

	masterState = D_STATE_INIT;

	if (isANameSpace(nameSpace)) {
		d_lockLock (d_lock(nameSpace));
		masterState = nameSpace->masterState;
		d_lockUnlock (d_lock(nameSpace));
	}

	return masterState;
}

void
d_nameSpaceSetMaster(
    d_nameSpace nameSpace,
    d_networkAddress master)
{
    if (isANameSpace(nameSpace)) {
        d_lockLock(d_lock(nameSpace));
        d_networkAddressFree(nameSpace->master);
        nameSpace->master = d_networkAddressNew(master->systemId, master->localId,
                                                        master->lifecycleId);
        d_lockUnlock(d_lock(nameSpace));
    }
    return;
}

d_networkAddress
d_nameSpaceGetMaster(
    d_nameSpace nameSpace)
{
    d_networkAddress addr = NULL;

    if (isANameSpace(nameSpace)) {
        d_lockLock(d_lock(nameSpace));
        addr = d_networkAddressNew( nameSpace->master->systemId,
                                    nameSpace->master->localId,
                                    nameSpace->master->lifecycleId);
        d_lockUnlock(d_lock(nameSpace));
    }
    return addr;
}

void
d_nameSpaceMasterConfirmed(
		d_nameSpace nameSpace)
{
	if (isANameSpace(nameSpace))
	{
		d_lockLock (d_lock(nameSpace));
		nameSpace->masterConfirmed = TRUE;
		d_lockUnlock(d_lock(nameSpace));
	}
}

void
d_nameSpaceMasterPending(
		d_nameSpace nameSpace)
{
	if (isANameSpace(nameSpace))
	{
		d_lockLock (d_lock(nameSpace));
		nameSpace->masterConfirmed = FALSE;
		d_lockUnlock(d_lock(nameSpace));
	}
}

c_bool
d_nameSpaceIsMasterConfirmed(
		d_nameSpace nameSpace)
{
	c_bool masterConfirmed;

	masterConfirmed = FALSE;

	if (isANameSpace (nameSpace))
	{
		masterConfirmed = nameSpace->masterConfirmed;
	}

	return masterConfirmed;
}

c_bool
d_nameSpaceMasterIsMe(
    d_nameSpace nameSpace,
    d_admin admin)
{
    c_bool result;
    d_networkAddress myAddr, master;

    if (isANameSpace(nameSpace)) {
        myAddr = d_adminGetMyAddress(admin);
        master = d_nameSpaceGetMaster(nameSpace);
        result = d_networkAddressEquals(myAddr, master);
        d_networkAddressFree(myAddr);
        d_networkAddressFree(master);
    } else {
        assert(FALSE);
        result = FALSE;
    }
    return result;
}

void
d_nameSpaceElementWalk(
    d_nameSpace nameSpace,
    c_bool ( * action ) (d_element element, c_voidp userData),
    c_voidp args)
{
    if (isANameSpace(nameSpace)) {
        d_tableWalk(nameSpace->elements, action, args);
    }
}

c_char*
d_nameSpaceGetPartitions(
    d_nameSpace nameSpace)
{
    struct d_nameSpaceHelper data;

    data.kind = D_NS_COUNT;
    data.count = 0;
    d_tableWalk(nameSpace->elements, d_nameSpaceGetPartitionsAction, &data);

    if(data.count == 0){
        data.value = os_malloc(1);
        *data.value = '\0';
    } else {
        data.kind = D_NS_COPY;
        data.value = os_malloc(data.count + 1);
        *data.value = '\0';
        d_tableWalk(nameSpace->elements, d_nameSpaceGetPartitionsAction, &data);
    }

    return data.value;
}

c_bool
d_nameSpaceGetPartitionsAction(
    d_element element,
    c_voidp args)
{
    struct d_nameSpaceHelper* data;

    data = (struct d_nameSpaceHelper*)args;

    switch(data->kind){
        case D_NS_COUNT:
            data->count += element->strlenPartition + 1;
            break;
        case D_NS_COPY:
            if(strlen(data->value) == 0){
                os_sprintf(data->value, "%s", element->partition);
            } else {
                os_sprintf(data->value, "%s,%s", data->value, element->partition);
            }
            break;
    }
    return TRUE;
}

void
d_nameSpaceCopyPartitions(
    d_nameSpace to,
    d_nameSpace from)
{
    /* Keep partitionlist */
    d_objectKeep (d_object(from->elements));

    /* Free old table */
    if (to->elements)
    {
        d_tableFree (to->elements);
    }

    /* Copy table to 'to' nameSpace */
    to->elements = from->elements;
}

void
d_nameSpaceSetInitialQuality(
    d_nameSpace nameSpace,
    d_quality quality)
{
    if (isANameSpace(nameSpace)) {
        nameSpace->quality.seconds = quality.seconds;
        nameSpace->quality.nanoseconds = quality.nanoseconds;
    }
}

d_quality
d_nameSpaceGetInitialQuality(
    d_nameSpace nameSpace)
{
    d_quality q;

    q.seconds = 0;
    q.nanoseconds = 0;

    if (isANameSpace(nameSpace)) {
        q.seconds = nameSpace->quality.seconds;
        q.nanoseconds = nameSpace->quality.nanoseconds;
    }
    return q;
}

d_mergePolicy
d_nameSpaceGetMergePolicy(
    d_nameSpace nameSpace,
    d_name role)
{
    return d_policyGetMergePolicy (nameSpace->policy, role);
}

d_name
d_nameSpaceGetRole(
    d_nameSpace nameSpace)
{
    assert (isANameSpace(nameSpace));
    return os_strdup(nameSpace->mergeState->role);
}

d_mergeState
d_nameSpaceGetMergeState(
    d_nameSpace nameSpace,
    d_name role)
{
    d_mergeState dummy;
    d_mergeState state = NULL;

    if (isANameSpace(nameSpace)) {
        if((!role) || (strcmp(role, nameSpace->mergeState->role) == 0)){
            state = nameSpace->mergeState;
        } else {
            dummy = d_mergeStateNew(role, 0);
            state = d_tableFind(nameSpace->mergedRoleStates, dummy);
            d_mergeStateFree(dummy);
        }
        if(state){
            state = d_mergeStateNew(state->role, state->value);
        }
    }
    return state;
}

void
d_nameSpaceSetMergeState(
    d_nameSpace nameSpace,
    d_mergeState state)
{
    d_mergeState toAdd, found;

    if (isANameSpace(nameSpace)) {
        if((!state->role) || (strcmp(state->role, nameSpace->mergeState->role) == 0)){
            nameSpace->mergeState->value = state->value;
        } else {
            toAdd = d_mergeStateNew(state->role, state->value);
            found = d_tableInsert(nameSpace->mergedRoleStates, toAdd);

            if(found){
                d_mergeStateFree(toAdd);
                d_mergeStateSetValue(found, state->value);
            }
        }
    }
    return;
}

void
d_nameSpaceClearMergeState(
    d_nameSpace nameSpace,
    d_name role)
{
    d_mergeState dummy;

    if (isANameSpace(nameSpace)) {
        dummy = d_mergeStateNew (role, 0);
        d_tableRemove (nameSpace->mergedRoleStates, dummy);
        d_mergeStateFree (dummy);
    }
}

struct nsMergeStateDiffHelper
{
    d_table mergedStates;
    c_iter diff;
};

struct nsFindDiffHelper
{
    c_bool found;
    const char* role;
};

void
d_nameSpaceFindDiffWalk (
    c_voidp o,
    c_voidp userData)
{
    struct nsFindDiffHelper* helper;
    d_mergeState state;

    helper = (struct nsFindDiffHelper*)userData;
    state = d_mergeState(o);

    /* If role is found in diff, return true */
    if (!strcmp (state->role, helper->role)) {
        helper->found = TRUE;
    }
}

static c_bool
d_nameSpaceMergeStateDiffWalk (
    c_voidp o,
    c_voidp userData)
{
    d_mergeState state1;
    d_mergeState state2;
    struct nsMergeStateDiffHelper* helper;
    struct nsFindDiffHelper walkData;

    state1 = (d_mergeState)o;
    helper = (struct nsMergeStateDiffHelper*)userData;

    /* Check if role is not already in diff list */
    walkData.role = state1->role;
    walkData.found = FALSE;
    c_iterWalk(helper->diff, d_nameSpaceFindDiffWalk, &walkData);

    /* Look it up, when a conflict is found, at it to diff list */
    if (!walkData.found) {
        state2 = d_tableFind (helper->mergedStates, state1);
        if (state2) {
            if (state1->value != state2->value) {
                c_iterInsert (helper->diff, d_mergeStateNew(state1->role, state1->value));
            }
        }else {
            c_iterInsert (helper->diff, d_mergeStateNew(state1->role, state1->value));
        }
    }
    return TRUE;
}

/* Returns a list of conflicting merge states */
c_iter
d_nameSpaceGetMergedStatesDiff(
    d_nameSpace ns1,
    d_nameSpace ns2)
{
    c_iter result;
    struct nsMergeStateDiffHelper walkData;

    result = c_iterNew(NULL);

    /* 1st walk: check ns 1 */
    walkData.mergedStates = ns2->mergedRoleStates;
    walkData.diff = result;
    d_tableWalk (ns1->mergedRoleStates, d_nameSpaceMergeStateDiffWalk, &walkData);

    /* 2nd walk: check ns 2 */
    walkData.mergedStates = ns1->mergedRoleStates;
    d_tableWalk (ns2->mergedRoleStates, d_nameSpaceMergeStateDiffWalk, &walkData);

    /* If diff list is empty, free iterator */
    if (!c_iterLength(result)) {
        c_iterFree (result);
        result = NULL;
    }

    return result;
}


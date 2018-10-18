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
#include <string.h>

#include "d__misc.h"
#include "d__table.h"
#include "d__admin.h"
#include "d__nameSpace.h"
#include "d__configuration.h"
#include "d__policy.h"
#include "d__element.h"
#include "d__nameSpaces.h"
#include "d_nameSpaces.h"
#include "d_networkAddress.h"
#include "d__mergeState.h"
#include "d__thread.h"
#include "d__durability.h"
#include "os_report.h"
#include "os_stdlib.h"


/**************************************************************
 * Private functions
 **************************************************************/

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

    element = d_elementNew(name, partition, topic);
    if (element) {
        found = d_tableInsert(nameSpace->elements, element);
        if (found) {
            d_elementFree(element); /* this was not unique */
        }
    }
}


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
        r = strcmp(ns1->name, ns2->name);
        if (r == 0)
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
    }

    return r;
}


static int
d_nameSpaceCompareFull(
    d_nameSpace ns1,
    d_nameSpace ns2)
{
    int r = 0;
    d_element wc = d_elementNew("wildcards", "*", "*");
    d_element e1 = d_tableFind(ns1->elements, wc);
    d_element e2 = d_tableFind(ns2->elements, wc);

    d_elementFree(wc);

    if (e1 && e2) {
        r = 0;
    } else if (e1) {
        r = 1;
    } else if (e2) {
        r = -1;
    } else {
        c_ulong l1, l2;

        l1 = d_tableSize(ns1->elements);
        l2 = d_tableSize(ns2->elements);

        if (l1 < l2) {
            r = -1;
        } else if (l1 > l2) {
            r = 1;
        } else {
            d_table t = d_tableNew(d_elementCompare, NULL);
            d_tableIter tableIter;
            d_element e;

            e = d_tableIterFirst(ns1->elements, &tableIter);
            while (e) {
                d_tableInsert(t, e);
                e = d_tableIterNext(&tableIter);
            }

            e = d_tableIterFirst(ns2->elements, &tableIter);
            while (e && !r) {
                if (!d_tableInsert(t, e)) {
                    r = 1;
                }
                e = d_tableIterNext(&tableIter);
            }

            d_tableFree(t);
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
        partitions1 = d_nameSpaceGetPartitionTopics(ns1);
        partitions2 = d_nameSpaceGetPartitionTopics(ns2);
        r = strcmp(partitions1, partitions2);
        os_free(partitions1);
        os_free(partitions2);
        if (r != 0) {
            r = d_nameSpaceCompareFull(ns1, ns2);
        }
    }
    return r;
}

int
d_nameSpaceNameCompare(
    d_nameSpace ns1,
    d_nameSpace ns2)
{
    int r;

    if((!ns1) && (!ns2)){
        r = 0;
    } else if(!ns1){
        r = -1;
    } else if(!ns2){
        r = 1;
    } else if((!ns1->name) && (!ns2->name)){
        r = 0;
    } else if(!ns1->name){
        r = -1;
    } else if(!ns2->name){
        r = 1;
    } else {
        r = strcmp(ns1->name, ns2->name);
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

c_bool
d_nameSpaceGetDelayedAlignment(
    d_nameSpace nameSpace)
{
    c_bool result;

    result = FALSE;

    if (isANameSpace(nameSpace)) {
        if(nameSpace->policy) {
            result = nameSpace->policy->delayedAlignment;
        }
    }

    return result;
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
        /* fallthrough intentional */
        case D_ALIGNEE_ON_REQUEST:
            notInitial = TRUE;
            break;
        default:
            OS_REPORT(OS_ERROR, D_CONTEXT, 0,
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
 *   - (partitionTopic = <partitionName>.<topicName>) AND (topicGiven == NULL)
 *   OR
 *   - (partitionTopic = <partitionName>)             AND (topicGiven = <topicName>)
 *
 * both <partitionName> and <topicName> may contain wildcards ('?' or'*')
 */
int
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
    strlenPartitionTopic = (os_uint32) (strlen(partitionTopic) + 1);
    if (isANameSpace(nameSpace)) {
          /* QAC EXPECT 1253; */
        if (strlenPartitionTopic < D_MAX_STRLEN_NAMESPACE) {
            if (topicGiven) {
                /* QAC EXPECT 1253; */
                strlenTopicGiven = (os_uint32) (strlen(topicGiven) + 1);
                /* QAC EXPECT 1253; */
                if (strlenTopicGiven < D_MAX_STRLEN_NAMESPACE) {
                    d_nameSpaceAddPartitionTopic(nameSpace, name, partitionTopic, topicGiven);
                }
            } else {
                partition = (char *)d_malloc(strlenPartitionTopic, "Partition");
                if (partition) {
                    os_strncpy(partition, partitionTopic, strlenPartitionTopic);
                    /* Make topic point to last character in partition string.
                     * partition points to first character and strlenPartitionTopic
                     * includes '\0', so subtract 2 to point to last character.
                     */
                    topic = partition + (strlenPartitionTopic-2);

                    /* QAC EXPECT 2106,3123; */
                    while ((*topic != '.') && (topic != partition)) {
                        /* QAC EXPECT 0489; */
                        topic--;
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
                        else
                        {
                          d_free(partition);
                          return -1;
                        }
                    } else {
                        /* Though <PartitionTopic> was used in the definition of a namespace, only
                         * a partition is provided.
                         */
                        d_nameSpaceAddPartitionTopic(nameSpace, name, partition, "*");
                    }
                    d_free(partition);
                }
            }
        }
        else
        {
            OS_REPORT(OS_ERROR, D_CONTEXT, 0, "//OpenSplice/DurabilityService/NameSpaces/NameSpace/PartitionTopic length is too long");
            return -1;
        }
    }

    return 0;
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

void
d_nameSpaceDeinit(
    d_nameSpace nameSpace)
{
    assert(d_nameSpaceIsValid(nameSpace));

    if (nameSpace->master) {
        d_networkAddressFree(nameSpace->master);
        nameSpace->master = NULL;
    }
    if (nameSpace->elements) {
        d_tableFree(nameSpace->elements); /* frees all */
        nameSpace->elements = NULL;
    }
    if (nameSpace->mergedRoleStates) {
        d_tableFree(nameSpace->mergedRoleStates);
        nameSpace->mergedRoleStates = NULL;
    }
    if (nameSpace->mergeState) {
        d_mergeStateFree (nameSpace->mergeState);
        nameSpace->mergeState = NULL;
    }
    if (nameSpace->advertisedMergeState) {
        d_mergeStateFree (nameSpace->advertisedMergeState);
        nameSpace->advertisedMergeState = NULL;
    }
    if (nameSpace->name) {
        d_free(nameSpace->name);
        nameSpace->name = NULL;
    }
    if (nameSpace->policy) {
        d_policyFree(nameSpace->policy);
        nameSpace->policy = NULL;
    }
    /* Call super-deinit */
    d_lockDeinit(d_lock(nameSpace));
}

void
d_nameSpaceFree(
    d_nameSpace nameSpace)
{
    /* Note: This function can be called with a NULL-nameSpace */
    if (isANameSpace(nameSpace)) {
        d_objectFree(d_object(nameSpace));
    }
}

/**************************************************************/
d_nameSpace
d_nameSpaceNew(
    const char * name,
    d_policy policy)
{
    d_nameSpace nameSpace = NULL;
    d_durability durability;
    d_configuration config;

    assert(name);

    durability = d_threadsDurability();
    config = d_durabilityGetConfiguration(durability);

    if ((policy == NULL) ||
        (!d_patternMatch(name, d_policyGetNameSpace(policy)))) {
        return NULL;
    }

    assert(d_policyIsValid(policy));

    nameSpace = d_nameSpace(d_malloc(C_SIZEOF(d_nameSpace), "NameSpace"));
    /* Call super-init */
    d_lockInit(d_lock(nameSpace), D_NAMESPACE,
               (d_objectDeinitFunc)d_nameSpaceDeinit);
    /* Initialize namespace */
    nameSpace->name                 = os_strdup(name);
    nameSpace->policy               = d_policy(d_objectKeep(d_object(policy)));
    nameSpace->elements             = d_tableNew(d_elementCompare, d_elementFree);
    nameSpace->quality              = D_QUALITY_ZERO;
    nameSpace->master               = d_networkAddressUnaddressed();
    nameSpace->masterState          = D_STATE_COMPLETE;
    nameSpace->masterConfirmed      = FALSE;
    nameSpace->alignable            = d_policyGetAligner(policy) ? TRUE : FALSE;
    nameSpace->mustAlignBuiltinTopics = config->mustAlignBuiltinTopics;
    nameSpace->mergeState           = d_mergeStateNew(config->role, config->initialMergeStateValue);
    nameSpace->advertisedMergeState = d_mergeStateNew(config->role, config->initialMergeStateValue);
    nameSpace->mergedRoleStates     = d_tableNew(d_mergeStateCompare, d_mergeStateFree);
    nameSpace->compatibility_check_required = TRUE;
    nameSpace->mergeCount           = 0;

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

    /* Allocate nameSpace object */
    nameSpace = d_nameSpace(d_malloc(C_SIZEOF(d_nameSpace), "NameSpace"));
    if (nameSpace != NULL) {
        /* Call super-init */
        d_lockInit(d_lock(nameSpace), D_NAMESPACE,
                   (d_objectDeinitFunc)d_nameSpaceDeinit);
        /* Initialize namespace */
        d_objectKeep (d_object(ns->policy));
        d_objectKeep (d_object(ns->elements));
        nameSpace->name                 = os_strdup(ns->name);
        nameSpace->policy               = ns->policy;
        nameSpace->elements             = ns->elements;
        nameSpace->quality              = ns->quality;
        nameSpace->master               = d_networkAddressNew(ns->master->systemId, ns->master->localId,
                                                              ns->master->lifecycleId);
        nameSpace->masterState          = ns->masterState;
        nameSpace->masterConfirmed      = ns->masterConfirmed;
        nameSpace->alignable            = ns->alignable;
        nameSpace->mergeState           = d_mergeStateNew(ns->mergeState->role, ns->mergeState->value);
        if (ns->advertisedMergeState) {
           nameSpace->advertisedMergeState = d_mergeStateNew(ns->advertisedMergeState->role, ns->advertisedMergeState->value);
        } else {
            nameSpace->advertisedMergeState = NULL;
        }
        nameSpace->mergedRoleStates     = d_tableNew(d_mergeStateCompare, d_mergeStateFree);
        d_tableWalk(ns->mergedRoleStates, insertMergeState, nameSpace->mergedRoleStates);
        nameSpace->compatibility_check_required = ns->compatibility_check_required;
        nameSpace->mustAlignBuiltinTopics = ns->mustAlignBuiltinTopics;
        nameSpace->mergeCount           = ns->mergeCount;
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
    c_char *partitionTopics, *partitionTopic, *partition, *topic, *savePtr;
    c_bool aligner;
    d_alignmentKind alignmentKind;
    d_durabilityKind durabilityKind;
    os_uint32 i;
    int count;
    const struct d_mergeState_s *ms;
    c_ulong masterPriority = D_DEFAULT_MASTER_PRIORITY;

    if(ns){
        /* Allocate nameSpace */
        nameSpace = d_nameSpace(d_malloc(C_SIZEOF(d_nameSpace), "NameSpace"));
        if(nameSpace != NULL){
            /* Call super-init */
            d_lockInit(d_lock(nameSpace), D_NAMESPACE,
                      (d_objectDeinitFunc)d_nameSpaceDeinit);

            aligner = d_nameSpacesIsAligner(ns);
            alignmentKind = d_nameSpacesGetAlignmentKind(ns);
            durabilityKind = d_nameSpacesGetDurabilityKind(ns);

            if (d_nameSpacesGetName(ns)) {
                nameSpace->name            = os_strdup(d_nameSpacesGetName(ns));
            }else {
                nameSpace->name            = NULL; /* LH: This should never happen!!!! */
            }

            /* mergedStates can contain name-value pairs beyond mergedStatesCount
             * these name-value pairs are no mergedStates and thus have to be
             * parsed separately.
             */
            ms = (const struct d_mergeState_s *) ns->mergedStates;
            for (i=ns->mergedStatesCount; i<c_sequenceSize(ns->mergedStates); i++) {
                if (ms[i].role) {
                    if (strcmp(ms[i].role, "masterPriority") == 0) {
                        masterPriority = ms[i].value;
                    }
                }
            }

            nameSpace->policy              = d_policyNew (
                                                NULL,
                                                aligner,
                                                alignmentKind,
                                                FALSE,
                                                durabilityKind,
                                                D_DEFAULT_EQUALITY_CHECK,
                                                masterPriority);
            nameSpace->quality             = d_nameSpacesGetInitialQuality(ns);
            nameSpace->master              = d_networkAddressNew(
                                                    ns->master.systemId,
                                                    ns->master.localId,
                                                    ns->master.lifecycleId);
            nameSpace->masterState         = D_STATE_COMPLETE;
            nameSpace->elements            = d_tableNew(d_elementCompare, d_elementFree);
            nameSpace->mustAlignBuiltinTopics = config->mustAlignBuiltinTopics;
            nameSpace->compatibility_check_required = TRUE;
            nameSpace->mergeCount          = 0;

            partitionTopics = d_nameSpacesGetPartitions(ns);

           /* Legacy versions of the product send a comma-separated list of partitions wheras
            * newer versions of the product send a comma-separated list of partition-topic
            * combinations where the partition and topic are separated by a '.'. In the former
            * case one should interpret the topic expression to be '*'. Additionally, new
            * versions only send the partition-expression in case the topic-expression is
            * '*' for backwards compatibility reasons. Obviously, the topic-expression
            * needs to be interpreted as '*' in that case as well.
            */
            partitionTopic = os_strtok_r(partitionTopics, ",", &savePtr);

            while(partitionTopic){
                /* Allocate worst-case sizes for partition and topic expression. */
                partition = os_malloc(strlen(partitionTopic) + 1);
                topic = os_malloc(strlen(partitionTopic) + 1);

                if (partition && topic) {

                    if(strlen(partitionTopic) == 0){
                        /* This means that the default partition (empty string)
                         * has been configured, without any topic expression
                         * (leads to "*" as topic expression).
                         */
                        os_sprintf(partition, "");
                        os_sprintf(topic, "*");

                    } else if((strlen(partitionTopic) > 0) && (partitionTopic[0] == '.')){
                        /* To ensure matching of the default partition (empty
                         * string) check if expression starts with a dot. If
                         * that is the case, use empty string as partition and
                         * strip the dot of the string and interpret the rest as
                         * the topic expression.
                         */
                        os_sprintf(partition, "");
                        os_sprintf(topic, "%s", &partitionTopic[1]);
                    } else {
                        /* Match partition and topic parts of the expression.*/
                        count = sscanf(partitionTopic, "%[^.].%[^.]", partition, topic);

                        /* The partition-topic expression may not have a topic part and
                         * in that case will not contain a '.' character. The count
                         * will then be 1. Use '*' as topic-expression in that
                         * case.
                         */
                        if(count < 2){
                            os_sprintf(topic, "*");

                            /* In case count is less than 1, no partition could be
                             * found either. Match all partitions in that case by
                             * using '*'.
                             */
                            if(count < 1){
                                os_sprintf(partition, "*");
                            }
                        }
                    }
                    d_nameSpaceAddElement(nameSpace, "element", partition, topic);
                }
                os_free(partition);
                os_free(topic);

                partitionTopic = os_strtok_r(NULL, ",", &savePtr);
            }
            os_free(partitionTopics);

            nameSpace->masterConfirmed = ns->masterConfirmed;

            nameSpace->alignable = aligner;

            /* Add merge state for own role */
            nameSpace->mergeState = d_mergeStateNew (ns->state.role, 0);
            /* Note that it is NOT necessary to maintain a dedicated advertisedMergeState
             * for fellows, because the mergeState in the ns message already contains the
             * advertisedMergeState of a fellow. For now we will set the advertisedMergeState
             * attribute of fellow namespaces explicitly to NULL.
             */
            nameSpace->advertisedMergeState = NULL;

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
        d_lockLock(d_lock(nameSpace));
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
        d_lockLock(d_lock(nameSpace));
        masterState = nameSpace->masterState;
        d_lockUnlock(d_lock(nameSpace));
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
    if (isANameSpace(nameSpace)) {
        d_lockLock (d_lock(nameSpace));
        nameSpace->masterConfirmed = TRUE;
        d_lockUnlock(d_lock(nameSpace));
    }
}

void
d_nameSpaceMasterPending(
    d_nameSpace nameSpace)
{
    if (isANameSpace(nameSpace)) {
        d_lockLock (d_lock(nameSpace));
        nameSpace->masterConfirmed = FALSE;
        d_lockUnlock(d_lock(nameSpace));
    }
}


void
d_nameSpaceSetMasterConfirmed(
    d_nameSpace nameSpace,
    c_bool masterConfirmed)
{
    if (isANameSpace(nameSpace)) {
        d_lockLock (d_lock(nameSpace));
        nameSpace->masterConfirmed = masterConfirmed;
        d_lockUnlock(d_lock(nameSpace));
    }
}


c_bool
d_nameSpaceIsMasterConfirmed(
    d_nameSpace nameSpace)
{
    c_bool masterConfirmed;

    masterConfirmed = FALSE;
    if (isANameSpace (nameSpace)) {
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
        if (master) {
            /* A master has been found. */
            result = d_networkAddressEquals(myAddr, master);
        } else {
            /* No master found for the namespace.
             * This situation can occur when at some point
             * in time a master has left the network, and
             * the only remaining nodes use the property
             * 'aligner="false"' for the namespace.
             */
            result = FALSE;
        }
        d_networkAddressFree(myAddr);
        d_networkAddressFree(master);
    } else {
        assert(FALSE);
        result = FALSE;
    }
    return result;
}

c_bool
d_nameSpaceMasterIsAddress(
    d_nameSpace nameSpace,
    d_networkAddress addr)
{
    c_bool result = FALSE;

    assert(d_nameSpaceIsValid(nameSpace));
    assert(addr);

    d_lockLock(d_lock(nameSpace));
    if ((d_networkAddressEquals(nameSpace->master, addr) == TRUE) &&
        (nameSpace->masterConfirmed == TRUE)) {
        result = TRUE;
    }
    d_lockUnlock(d_lock(nameSpace));

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
    data.ns = nameSpace;
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

/** This function returns a comma-separated list of partition-topic expressions.
  * Partition- and topic-expressions are separated by a dot.
  * Legacy versions of the durability service do not support the topic
  * part of the expression and are expecting a comma-separated list of partition
  * expressions. To allow interoperability between legacy and new versions, the
  * new versions will use a "*" as topic-expression when interpreting partition-
  * expressions and also they will leave out the topic-expression if it is '*'.
  * This allows interoperability between new and legacy when only <Partition>
  * configuration is used in name-space configurations.
  */
c_char*
d_nameSpaceGetPartitionTopics(
    d_nameSpace nameSpace)
{
    struct d_nameSpaceHelper data;

    data.kind = D_NS_COUNT;
    data.count = 0;
    data.ns = nameSpace;
    d_tableWalk(nameSpace->elements, d_nameSpaceGetPartitionTopicsAction, &data);

    if(data.count == 0){
        data.value = os_malloc(1);
        *data.value = '\0';
    } else {
        data.kind = D_NS_COPY;
        data.value = os_malloc(data.count + 1);
        *data.value = '\0';
        d_tableWalk(nameSpace->elements, d_nameSpaceGetPartitionTopicsAction, &data);
    }

    return data.value;
}

struct d_nameSpaceLookupPartitionHelper {
    d_element found;
    d_partition partition;
};

static c_bool
d_nameSpaceComparePartitionAction(
    d_element e,
    void* args)
{
    struct d_nameSpaceLookupPartitionHelper* data;
    data = args;

    if(strcmp(data->partition, e->partition) == 0) {
        data->found = e;
    }
    return data->found == NULL;
}

static d_element
d_nameSpaceLookupPartition(
    d_nameSpace nameSpace,
    d_partition partition)
{
    struct d_nameSpaceLookupPartitionHelper walkData;

    walkData.found = NULL;
    walkData.partition = partition;
    d_tableWalk(nameSpace->elements, d_nameSpaceComparePartitionAction, &walkData);
    return walkData.found;
}


c_bool
d_nameSpaceGetPartitionsAction(
    d_element element,
    c_voidp args)
{
    struct d_nameSpaceHelper* data;

    data = (struct d_nameSpaceHelper*)args;

    /* Because of the recently added functionality to support partition.topic expressions to identify
     * the scope of a namespace, partitions can have more than one occurrence in the d_nameSpace->elements list. To
     * prevent double entries in the partitionstring, lookup the first occurrence of a partition(!) in the elements-list
     * and add it only when it equals the current element.
     */
    if(element == d_nameSpaceLookupPartition(data->ns, element->partition)) {
        switch(data->kind){
            case D_NS_COUNT:
                data->count += element->strlenPartition + 1;
                break;
            case D_NS_COPY:
                if(strlen(data->value)){
                    os_strcat(data->value, ",");
                }
                os_strcat(data->value, element->partition);

                break;
        }
    }
    return TRUE;
}

c_bool
d_nameSpaceGetPartitionTopicsAction(
    d_element element,
    c_voidp args)
{
    struct d_nameSpaceHelper* data;

    data = (struct d_nameSpaceHelper*)args;

    switch(data->kind){
        case D_NS_COUNT:
            if(!element->topic) {
                data->count += element->strlenTopic + element->strlenPartition + 1;
            }else {
                data->count += element->strlenTopic + element->strlenPartition + 1 + 1;
            }
            break;
        case D_NS_COPY:
            if(strlen(data->value)){
                os_strcat(data->value, ",");
            }
            if(!element->topic) {
                os_strcat(data->value, element->partition);
                os_strcat(data->value, ".");
                os_strcat(data->value, "*");
            } else {
                os_strcat(data->value, element->partition);
                os_strcat(data->value, ".");
                os_strcat(data->value, element->topic);
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
        nameSpace->quality = quality;
    }
}

d_quality
d_nameSpaceGetInitialQuality(
    d_nameSpace nameSpace)
{
    d_quality q = D_QUALITY_ZERO;

    if (isANameSpace(nameSpace)) {
        q = nameSpace->quality;
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

/**
 * \brief Get the mergeState of a nameSpace
 *
 * If the role is NULL or equals the role of the native state of the namespace,
 * then the native state is returned.
 * If the role of the state is not NULL and differs from the role of
 * the namespace, then the foreign state is returned.
 */
d_mergeState
d_nameSpaceGetMergeState(
    d_nameSpace nameSpace,
    d_name role)
{
    d_mergeState dummy;
    d_mergeState state = NULL;

    if (isANameSpace(nameSpace)) {
        if ((!role) || (strcmp(role, nameSpace->mergeState->role) == 0)) {
            /* Native merge state */
            state = nameSpace->mergeState;
            /* Do not return your own state if you have cleared the state */
            if (state->value == (c_ulong)-1) {
                state = NULL;
            }
        } else {
            /* Foreign merge state */
            dummy = d_mergeStateNew(role, 0);
            state = d_tableFind(nameSpace->mergedRoleStates, dummy);
            d_mergeStateFree(dummy);
        }
        if (state) {
            state = d_mergeStateNew(state->role, state->value);
        }
    }
    return state;
}


/** \brief Get the advertisedMergeState of a nameSpace */
d_mergeState
d_nameSpaceGetAdvertisedMergeState(
        d_nameSpace nameSpace)
{
    d_mergeState state = NULL;

    if (isANameSpace(nameSpace)) {
        state = nameSpace->advertisedMergeState;
        /* Do not return your own advertised state if you have cleared the state */
        if (state && (state->value == (c_ulong)-1)) {
            state = NULL;
        }
        if (state) {
            /* Precondition: when an advertised mergeState exists, it MUST have a role */
            state = d_mergeStateNew(state->role, state->value);
        }
    }
    return state;
}

/**
 * \brief Set the mergeState of a nameSpace
 *
 * If state is NULL then the merge state value of the native state of
 * the namespace is set to -1 to indicate that no aligner is available.
 *
 * If the role of the state is NULL or equals the role of the namespace,
 * then the native state of the nameSpace is set.
 * If the role of the state is not NULL and differs from the role of
 * the namespace, then the foreign state is set.
 */
void
d_nameSpaceSetMergeState(
    d_nameSpace nameSpace,
    d_mergeState state)
{
    d_mergeState toAdd, found;

    if (isANameSpace(nameSpace)) {
        if (state == NULL) {
            /* A mergeState NULL means that no aligner is present.
             * This is represented by setting the value of the
             * native mergeState to -1. */
            nameSpace->mergeState->value = (c_ulong)-1;
        } else {
            if ((!state->role) || (strcmp(state->role, nameSpace->mergeState->role) == 0)) {
                /* Native state */
                nameSpace->mergeState->value = state->value;
            } else {
                /* Foreign state */
                toAdd = d_mergeStateNew(state->role, state->value);
                found = d_tableInsert(nameSpace->mergedRoleStates, toAdd);
                if (found) {
                    d_mergeStateFree(toAdd);
                    d_mergeStateSetValue(found, state->value);
                }
            }
        }
    }
    return;
}

/**
 * \brief Clear the mergeState of the namespace.
 *
 * If the role is NULL then the namespace for the native state is cleared.
 * If the role differs from the role of the native state, then the
 * foreign state corresponding to the role is removed.
 */
void
d_nameSpaceClearMergeState(
    d_nameSpace nameSpace,
    d_name role)
{
    d_mergeState dummy;

    if (isANameSpace(nameSpace)) {
        /* Use -1 to clear the namespace for your own role */
        if ((!role) || (strcmp(role, nameSpace->mergeState->role) == 0)) {
            /* clear the mergestate for this namespace */
            nameSpace->mergeState->value = (c_ulong)-1;
        } else {
            /* Apparently the namespace has a different role than my own.
             * Remove the namespace from my mergedRoles tables
             */
            dummy = d_mergeStateNew (role, 0);
            d_tableRemove (nameSpace->mergedRoleStates, dummy);
            d_mergeStateFree (dummy);
        }
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
    if (strcmp (state->role, helper->role) == 0) {
        helper->found = TRUE;
    }
}

static c_bool
d_nameSpaceMergeStateDiffWalk(
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

    /* Look it up, when a conflict is found, add it to diff list */
    if (!walkData.found) {
        state2 = d_tableFind (helper->mergedStates, state1);
        if (state2) {
            if (state1->value != state2->value) {
                helper->diff = c_iterInsert(
                    helper->diff, d_mergeStateNew(state1->role, state1->value));
            }
        } else {
            helper->diff = c_iterInsert(
                helper->diff, d_mergeStateNew(state1->role, state1->value));
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
    c_iter result = NULL;
    struct nsMergeStateDiffHelper walkData;

    if ((ns1 == NULL) || (ns2 == NULL)) {
        return NULL;
    }

    result = c_iterNew(NULL);

    /* 1st walk: check ns 1 */
    walkData.mergedStates = ns2->mergedRoleStates;
    walkData.diff = result;
    d_tableWalk (ns1->mergedRoleStates, d_nameSpaceMergeStateDiffWalk, &walkData);

    /* 2nd walk: check ns 2 */
    walkData.mergedStates = ns1->mergedRoleStates;
    d_tableWalk (ns2->mergedRoleStates, d_nameSpaceMergeStateDiffWalk, &walkData);

    /* If diff list is empty, free iterator */
    if (c_iterLength(result) == 0) {
        c_iterFree (result);
        result = NULL;
    }


    return result;
}


c_bool
d_nameSpaceIsAlignable(
    d_nameSpace nameSpace)
{
    assert(d_nameSpaceIsValid(nameSpace));

    return nameSpace->alignable;
}


void
d_nameSpaceSetAlignable(
    d_nameSpace nameSpace,
    c_bool alignable)
{
    assert(d_nameSpaceIsValid(nameSpace));

    nameSpace->alignable = alignable;
}

c_bool
d_nameSpaceGetEqualityCheck(
    d_nameSpace nameSpace)
{
    assert (isANameSpace(nameSpace));
    return d_policyGetEqualityCheck(nameSpace->policy);
}

c_ulong
d_nameSpaceGetMasterPriority(
    d_nameSpace nameSpace)
{
    assert (isANameSpace(nameSpace));
    return d_policyGetMasterPriority(nameSpace->policy);
}

/* Synchronize the value of the advertisedMergeState to that of the mergeState */
void
d_nameSpaceSyncMergeState(
    d_nameSpace nameSpace)
{
    assert(d_nameSpaceIsValid(nameSpace));

    d_lockLock(d_lock(nameSpace));
    nameSpace->advertisedMergeState->value = nameSpace->mergeState->value;
    d_lockUnlock(d_lock(nameSpace));
}

void
d_nameSpaceSetMergeCount(
    d_nameSpace nameSpace,
    c_ulong mergeCount)
{
    assert(d_nameSpaceIsValid(nameSpace));
    nameSpace->mergeCount = mergeCount;
}

c_ulong
d_nameSpaceGetMergeCount(
    d_nameSpace nameSpace)
{
    assert(d_nameSpaceIsValid(nameSpace));
    return nameSpace->mergeCount;
}

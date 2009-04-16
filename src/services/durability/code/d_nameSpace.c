/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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
#include "d_nameSpaces.h"
#include "d_networkAddress.h"
#include "d_configuration.h"
#include "os_report.h"

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
            strcpy(element->name, name);
            /* QAC EXPECT 5007; use of strcpy */
            strcpy(element->partition, partition);
            /* QAC EXPECT 5007; use of strcpy */
            strcpy(element->topic, topic);
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

static c_bool
d_nameSpaceStringMatches(
    c_string str,
    c_string pattern )
{
    c_bool   stop = FALSE;
    c_bool   matches = FALSE;
    c_string strRef = NULL;
    c_string patternRef = NULL;

    /* QAC EXPECT 2106,2100; */
    while ((*str != 0) && (*pattern != 0) && (stop == FALSE)) {
        /* QAC EXPECT 2106,3123; */
        if (*pattern == '*') {
            /* QAC EXPECT 0489; */
            pattern++;
            /* QAC EXPECT 2106; */
            while ((*str != 0) && (*str != *pattern)) {
                /* QAC EXPECT 0489; */
                str++;
            }
            /* QAC EXPECT 2106; */
            if (*str != 0) {
                /* QAC EXPECT 0489; */
                strRef = str+1; /* just behind the matching char */
                patternRef = pattern-1; /* on the '*' */
            }
        /* QAC EXPECT 2106,3123; */
        } else if (*pattern == '?') {
            /* QAC EXPECT 0489; */
            pattern++;
            /* QAC EXPECT 0489; */
            str++;
        /* QAC EXPECT 2004,3401,0489,2106; */
        } else if (*pattern++ != *str++) {
            if (strRef == NULL) {
                matches = FALSE;
                stop = TRUE;
            } else {
                str = strRef;
                pattern = patternRef;
                strRef = NULL;
            }
        }
    }
    /* QAC EXPECT 3892,2106,2100; */
    if ((*str == (char)0) && (stop == FALSE)) {
        /* QAC EXPECT 2106,3123; */
        while (*pattern == '*') {
            /* QAC EXPECT 0489; */
            pattern++;
        }
        /* QAC EXPECT 3892,2106; */
        if (*pattern == (char)0) {
            matches = TRUE;
        } else {
            matches = FALSE;
        }
    } else {
        matches = FALSE;
    }
    return matches;
    /* QAC EXPECT 5101; */
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

    if (d_nameSpaceStringMatches(nameSpaceSearch->partition, element->partition) &&
        d_nameSpaceStringMatches(nameSpaceSearch->topic, element->topic)){
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

    if((!ns1) && (!ns2)){
        r = 0;
    } else if(!ns1){
        r = -1;
    } else if(!ns2){
        r = 1;
    } else if(ns1->alignmentKind != ns2->alignmentKind){
        if(((c_ulong)ns1->alignmentKind) > ((c_ulong)ns2->alignmentKind)){
            r = 1;
        } else {
            r = -1;
        }
    } else if(ns1->durabilityKind != ns2->durabilityKind) {
        if(((c_ulong)ns1->durabilityKind) > ((c_ulong)ns2->durabilityKind)){
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

c_bool
d_nameSpaceIsAligner(
    d_nameSpace nameSpace)
{
    c_bool result = FALSE;

    if (isANameSpace(nameSpace)) {
        if(nameSpace->alignmentKind == D_ALIGNEE_INITIAL_AND_ALIGNER){
            result = TRUE;
        } else {
            result = FALSE;
        }
    }
    return result;
}

d_alignmentKind
d_nameSpaceGetAlignmentKind(
    d_nameSpace nameSpace)
{
    d_alignmentKind kind = D_ALIGNEE_INITIAL_AND_ALIGNER;

    if (isANameSpace(nameSpace)) {
        kind = nameSpace->alignmentKind;
    }
    return kind;
}


c_bool
d_nameSpaceIsAlignmentNotInitial(
    d_nameSpace nameSpace)
{
    c_bool notInitial;

    if (isANameSpace(nameSpace)) {
        switch(nameSpace->alignmentKind){
        case D_ALIGNEE_INITIAL_AND_ALIGNER:
            /*fallthrough intentional*/
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
                        nameSpace->alignmentKind);
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
        kind = nameSpace->durabilityKind;
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
                strncpy(partition, partitionTopic, strlenPartitionTopic);
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
        d_free(nameSpace->name);
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
    const char * name,
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
        nameSpace->alignmentKind        = alignmentKind;
        nameSpace->durabilityKind       = durabilityKind;
        nameSpace->elements             = d_tableNew(elementCompare, elementFree);
        nameSpace->quality.seconds      = 0;
        nameSpace->quality.nanoseconds  = 0;
        nameSpace->master               = d_networkAddressUnaddressed();

    }

    return nameSpace;
}

d_nameSpace
d_nameSpaceFromNameSpaces(
    d_nameSpaces ns)
{
    d_nameSpace nameSpace = NULL;
    c_char *partitions, *temp;
    d_quality quality;

    if(ns){
        nameSpace = d_nameSpace(d_malloc((os_uint32)C_SIZEOF(d_nameSpace), "NameSpace"));

        if(nameSpace != NULL){
            d_lockInit(d_lock(nameSpace), D_NAMESPACE, d_nameSpaceDeinit);

            nameSpace->name                = NULL;
            nameSpace->alignmentKind       = d_nameSpacesGetAlignmentKind(ns);
            nameSpace->durabilityKind      = d_nameSpacesGetDurabilityKind(ns);
            quality                        = d_nameSpacesGetInitialQuality(ns);
            nameSpace->quality.seconds     = quality.seconds;
            nameSpace->quality.nanoseconds = quality.nanoseconds;
            nameSpace->master              = d_networkAddressNew(
                                                    ns->master.systemId,
                                                    ns->master.localId,
                                                    ns->master.lifecycleId);
            nameSpace->elements            = d_tableNew(elementCompare, elementFree);

            partitions = d_nameSpacesGetPartitions(ns);
            temp = strtok(partitions, ",");

            while(temp){
                d_nameSpaceAddElement(nameSpace, "element", temp, "*");
                temp = strtok(NULL, ",");
            }
            os_free(partitions);
        }
    }
    return nameSpace;
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
                sprintf(data->value, "%s", element->partition);
            } else {
                sprintf(data->value, "%s,%s", data->value, element->partition);
            }
            break;
    }
    return TRUE;
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

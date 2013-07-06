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

#include "d__group.h"
#include "d_group.h"
#include "v_builtin.h"
#include "os.h"

d_group
d_groupNew(
    const c_char* partition,
    const c_char* topic,
    d_durabilityKind kind,
    d_completeness completeness,
    d_quality quality)
{
    d_group group;
    group = NULL;

    if(topic && partition){
        group = d_group(os_malloc(C_SIZEOF(d_group)));
        d_objectInit(d_object(group), D_GROUP, d_groupDeinit);
        group->topic = (c_char*)(os_malloc(strlen(topic) + 1));
        group->partition = (c_char*)(os_malloc(strlen(partition) + 1));
        os_strcpy(group->topic, topic);
        os_strcpy(group->partition, partition);
        group->kind                = kind;
        group->completeness        = completeness;
        group->quality.seconds     = quality.seconds;
        group->quality.nanoseconds = quality.nanoseconds;
        group->vgroup              = NULL;
        group->storeCount          = 0;
        group->private             = FALSE;
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
    d_object object)
{
    d_group group;

    assert(d_objectIsValid(object, D_GROUP) == TRUE);

    if(object){
        group = d_group(object);
        os_free(group->topic);
        os_free(group->partition);

        if(group->vgroup){
            c_free(group->vgroup);
        }
    }
}

void
d_groupFree(
    d_group group)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        d_objectFree(d_object(group), D_GROUP);
    }
}

d_completeness
d_groupGetCompleteness(
    d_group group)
{
    d_completeness result;
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
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
    d_quality quality)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        group->completeness        = completeness;
        group->quality.seconds     = quality.seconds;
        group->quality.nanoseconds = quality.nanoseconds;
    }
}

void
d_groupSetComplete(
    d_group group)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        group->quality.seconds = D_MAX_QUALITY;
        group->quality.nanoseconds = 0;
        group->completeness = D_GROUP_COMPLETE;

        if(group->vgroup) {
            v_groupCompleteSet(group->vgroup, TRUE);
        }
    }
}

void
d_groupSetUnaligned(
    d_group group)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        group->quality.seconds = 0;
        group->quality.nanoseconds = 0;
        group->completeness = D_GROUP_UNKNOWN;
    }
}

void d_groupSetIncomplete(
    d_group group)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        group->quality.seconds = 0;
        group->quality.nanoseconds = 0;
        group->completeness = D_GROUP_INCOMPLETE;

        if(group->vgroup) {
            v_groupCompleteSet(group->vgroup, FALSE);
        }
    }
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

    if(group){
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

    if(group){
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
    d_quality q;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        q.seconds = group->quality.seconds;
        q.nanoseconds = group->quality.nanoseconds;
    }else
    {
        q.seconds = 0;
        q.nanoseconds = 0;
    }
    return q;
}

c_bool
d_groupIsBuiltinGroup(
    d_group group)
{
    c_bool result = FALSE;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        if(strcmp(group->partition, V_BUILTIN_PARTITION) == 0){
            if( (strcmp(group->topic, V_PARTICIPANTINFO_NAME) == 0) ||
                (strcmp(group->topic, V_TOPICINFO_NAME) == 0) ||
                (strcmp(group->topic, V_PUBLICATIONINFO_NAME) == 0) ||
                (strcmp(group->topic, V_SUBSCRIPTIONINFO_NAME) == 0))
            {
                result = TRUE;
            }
        }
    }
    return result;
}

void
d_groupSetPrivate(
    d_group group,
    c_bool isPrivate)
{
    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        group->private = isPrivate;
    }
    return;
}

c_bool
d_groupIsPrivate(
    d_group group)
{
    c_bool result;

    assert(d_objectIsValid(d_object(group), D_GROUP) == TRUE);

    if(group){
        result = group->private;
    } else {
        result = FALSE;
    }
    return result;
}


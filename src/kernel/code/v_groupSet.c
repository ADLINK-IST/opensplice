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
#include "v_groupSet.h"
#include "v__observable.h"
#include "v_entity.h"
#include "v__group.h"
#include "v_event.h"
#include "v_public.h"

#include "c_collection.h"

v_groupSet
v_groupSetNew(
    v_kernel kernel)
{
    v_groupSet groupSet;

    groupSet = v_groupSet(v_objectNew(kernel,K_GROUPSET));
    v_observableInit(v_observable(groupSet),"GroupSet", NULL, TRUE);
    groupSet->sequenceNumber = 0;
    groupSet->groups = c_tableNew(v_kernelType(kernel,K_GROUP),"partition,topic");
    c_lockInit(&groupSet->lock,SHARED_LOCK);

    return groupSet;
}


static c_bool
alwaysFalse(
    c_object found,
    c_object requested,
    c_voidp arg)
{
    v_group *groupFound = (v_group *)arg;

    OS_UNUSED_ARG(requested);
    assert(groupFound != NULL);
    assert(*groupFound == NULL); /* out param */

    *groupFound = c_keep(found);

    return FALSE;
}


v_group
v_groupSetCreate(
    v_groupSet set,
    v_partition partition,
    v_topic topic)
{
    v_group group, found;
    C_STRUCT(v_event) event;
    v_kernel kernel;
    C_STRUCT(v_group) dummyGroup;

    assert(set != NULL);
    assert(partition != NULL);
    assert(topic != NULL);
    assert(C_TYPECHECK(set,v_groupSet));
    assert(C_TYPECHECK(partition,v_partition));
    assert(C_TYPECHECK(topic,v_topic));

    c_lockWrite(&set->lock);
    /* First create a dummy group used for checking existence */
    memset(&dummyGroup, 0, sizeof(dummyGroup));
    dummyGroup.partition = partition;
    dummyGroup.topic = topic;

    /* Note: the following call does not execute the actual remove because
     *       the alwaysFalse function returns FALSE */
    found = NULL;
    /* Note: The alwaysFalse function increases the refCount of
     * found, which is the out-parameter of the tableRemove. */
    c_tableRemove(set->groups, &dummyGroup, alwaysFalse, &found);

    if (!found) {
        group = v_groupNew(partition, topic, set->sequenceNumber);
        found = c_tableInsert(set->groups,group);
        /* Because understanding assertion holds true, we practically
         * transferred the refCount from group to found. Because found
         * is our return parameter, we do not free group here.
         */
        assert(found == group);
        set->sequenceNumber++;
        kernel = v_objectKernel(set);

        c_lockUnlock(&set->lock);

        /* Update the status of the observers */
        /* And trigger the waiting observers */
        event.kind = V_EVENT_NEW_GROUP;
        event.source = v_publicHandle(v_public(kernel));
        event.userData = group;
        v_observableNotify(v_observable(kernel),&event);
    } else {
        c_lockUnlock(&set->lock);
    }

    return found;
}

v_group
v_groupSetRemove(
    v_groupSet set,
    v_group group)
{
    v_group found;

    assert(set != NULL);
    assert(C_TYPECHECK(set,v_groupSet));
    assert(C_TYPECHECK(group,v_group));

    c_lockWrite(&set->lock);
    found = c_tableRemove(set->groups,group,NULL,NULL);
    c_lockUnlock(&set->lock);

    return found;
}

c_iter
v_groupSetSelect(
    v_groupSet set,
    c_char *expression,
    c_value params[])
{
    c_collection q;
    q_expr expr;
    c_iter list;

    assert(set != NULL);
    assert(C_TYPECHECK(set,v_groupSet));

    expr = (q_expr)q_parse(expression);
    if (expr == NULL) {
        assert(expr != NULL);
        return NULL;
    }

    c_lockRead(&set->lock);
    q = c_queryNew(set->groups,expr,params);
    if (q == NULL) {
        list = NULL;
    } else {
        list = ospl_c_select(q,0);
    }
    c_lockUnlock(&set->lock);
    assert(q != NULL);
    c_free(q);
    q_dispose(expr);

    return list;
}

c_bool
v_groupSetWalk (
    v_groupSet set,
    c_action action,
    c_voidp arg)
{
    c_bool result;
    c_lockRead(&set->lock);
    result = c_walk(set->groups,action,arg);
    c_lockUnlock(&set->lock);
    return result;
}

static c_bool
addGroup(
    c_object o,
    c_voidp arg)
{
    v_group g = v_group(o);
    c_iter *list = (c_iter *)arg;

    *list = c_iterInsert(*list, c_keep(g));

    return TRUE;
}

c_iter
v_groupSetSelectAll(
    v_groupSet set)
{
    c_iter result;

    assert(set != NULL);
    assert(C_TYPECHECK(set,v_groupSet));

    result = NULL;
    c_lockRead(&set->lock);
    c_tableWalk(set->groups, addGroup, &result);
    c_lockUnlock(&set->lock);

    return result;
}

c_iter
v_groupSetLookup(
    v_groupSet set,
    const c_char *partitionExpr,
    const c_char *topicExpr)
{
    c_collection q;
    q_expr expr;
    c_iter list;
    c_value params[2];

    assert(set != NULL);
    assert(C_TYPECHECK(set,v_groupSet));

    /* TODO : the creation of this query should be done once for
     *        a groupSet, the induced processing overhead doesn't
     *        meet the requirements for this operation.
     *        At this point only set query parameters should be used.
     */
    expr = (q_expr)q_parse("partition.name like %0 and topic.name like %1");
    if (expr == NULL) {
        assert(expr != NULL);
        return NULL;
    }
    params[0] = c_stringValue((c_string)partitionExpr);
    params[1] = c_stringValue((c_string)topicExpr);
    c_lockRead(&set->lock);
    q = c_queryNew(set->groups, expr, params);
    if (q == NULL) {
        list = NULL;
    } else {
        list = ospl_c_select(q,0);
    }
    c_lockUnlock(&set->lock);
    assert(q != NULL);
    c_free(q);
    q_dispose(expr);

    return list;
}

v_group
v_groupSetGet(
    v_groupSet set,
    const c_char *partitionName,
    const c_char *topicName)
{
    c_iter list;
    v_group g;

    list = v_groupSetLookup(set, partitionName, topicName);
    assert(c_iterLength(list) <= 1);
    g = v_group(c_iterTakeFirst(list));
    c_iterFree(list);

    return g;
}


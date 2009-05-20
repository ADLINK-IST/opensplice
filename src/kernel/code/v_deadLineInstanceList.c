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
#include "v__deadLineInstanceList.h"

#include "v__lease.h"
#include "v__leaseManager.h"
#include "v__status.h"

#include "os.h"
#include "os_report.h"

v_deadLineInstanceList
v_deadLineInstanceListNew(
    c_base base,
    v_leaseManager leaseManager,
    v_duration leaseDuration,
    v_leaseActionId actionId,
    v_public o)
{
    v_deadLineInstanceList list;
    c_type type;

    assert(C_TYPECHECK(leaseManager,v_leaseManager));
    assert(C_TYPECHECK(o,v_public));

    type = c_resolve(base, "kernelModule::v_deadLineInstanceList");
    assert(type);
    list = c_new(type);
    c_free(type);
    v_objectKind(list) = K_DEADLINEINSTANCE;
    v_instanceInit(v_instance(list));
    list->leaseManager = c_keep(leaseManager);
    list->leaseDuration = leaseDuration;
    list->deadlineLease = NULL;
    list->actionObject = o; /* no keep, since actionObject is onwer of v_deadLineInstanceList */
    list->actionId = actionId;

    return list;
}

void
v_deadLineInstanceListFree(
    v_deadLineInstanceList list)
{
    assert(C_TYPECHECK(list,v_deadLineInstanceList));

    v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
    c_free(list->leaseManager);
    list->leaseManager = NULL;
    c_free(list->deadlineLease);
    list->deadlineLease = NULL;
}

void
v_deadLineInstanceListSetDuration(
    v_deadLineInstanceList list,
    v_duration duration)
{
    assert(C_TYPECHECK(list,v_deadLineInstanceList));

    list->leaseDuration = duration;
    if (list->deadlineLease != NULL) {
        if ((c_timeCompare(duration, C_TIME_ZERO) != C_EQ) &&
            (c_timeCompare(duration, C_TIME_INFINITE) != C_EQ)) {
            v_leaseRenew(list->deadlineLease,duration);
        } else {
            v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
            c_free(list->deadlineLease);
            list->deadlineLease = NULL;
        }
    } else {
        if ((v_objectKind(v_instance(list)->prev) != K_DEADLINEINSTANCE) &&  /* not in list */
            (c_timeCompare(duration, C_TIME_ZERO) != C_EQ) &&
            (c_timeCompare(duration, C_TIME_INFINITE) != C_EQ)) { /* new instance */
            list->deadlineLease = v_leaseManagerRegister(list->leaseManager,
                                                         v_public(list->actionObject),
                                                         duration,
                                                         list->actionId,
                                                         V_LEASE_REPEAT_INFINITE);
        }
    }
}

/* put at end of list */
void
v_deadLineInstanceListInsertInstance(
    v_deadLineInstanceList list,
    v_instance instance)
{
    v_instance head = v_instance(list);

    assert(C_TYPECHECK(instance,v_instance));
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    assert(v_instanceAlone(instance));
    assert(c_refCount(list) > 0);
    assert(c_refCount(instance) > 0);

    /* As the instance is put at the end of the list no need to update the
       lease!
     */
    v_instanceUpdate(instance); /* Updates instance checkTime */
    v_instanceAppend(head,instance);
    if (list->deadlineLease == NULL) {
        if ((c_timeCompare(list->leaseDuration, C_TIME_ZERO) != C_EQ) &&
            (c_timeCompare(list->leaseDuration, C_TIME_INFINITE) != C_EQ)) {
            list->deadlineLease = v_leaseManagerRegister(list->leaseManager,
                                                         v_public(list->actionObject),
                                                         list->leaseDuration,
                                                         list->actionId,
                                                         V_LEASE_REPEAT_INFINITE);
        }
    }
}

void
v_deadLineInstanceListRemoveInstance(
    v_deadLineInstanceList list,
    v_instance instance)
{
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    assert((instance->prev != NULL) && (instance->next != NULL));
    assert(c_refCount(list) > 0);
    assert(c_refCount(instance) > 0);

    /* As the instance is removed, there is no need to update the lease.
     * Updating the lease might trigger the leasemanager thread to determine
     * the next expiry time. The next expiry time can also be determined on
     * the next deadline check, which is more efficient as the administration
     * might already have changed many times.
     */
    v_instanceRemove(instance);
    if (v_instanceAlone(v_instance(list))) { /* list has become empty */
        if (list->deadlineLease != NULL) {
            v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
            c_free(list->deadlineLease);
            list->deadlineLease = NULL;
        }
    }
}

void
v_deadLineInstanceListUpdate(
    v_deadLineInstanceList list,
    v_instance instance)
{
    assert(C_TYPECHECK(instance,v_instance));
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    assert(c_refCount(list) > 0);
    assert(c_refCount(instance) > 0);

    /* This will also place the current instance at the end of the list.
       Again no need to update the lease, we can determine the next wake-up
       as soon as the next deadlinecheck is performed.
    */
    if (v_instanceAlone(instance)) {
        v_deadLineInstanceListInsertInstance(list,instance);
    } else {
        v_instanceRemove(instance);
        v_instanceUpdate(instance); /* Updates instance checkTime */
        v_instanceAppend(v_instance(list), instance);
    }
}

c_iter
v_deadLineInstanceListCheckDeadlineMissed(
    v_deadLineInstanceList list,
    v_duration deadlineTime,
    c_time now)
{
    c_time expiryTime;
    v_instance listItem;
    c_iter missed;

    assert(C_TYPECHECK(list,v_deadLineInstanceList));

    missed = NULL;
    if (v_instanceAlone(v_instance(list))) { /* list is empty */
        assert (list->deadlineLease != NULL);
        v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
        c_free(list->deadlineLease);
        list->deadlineLease = NULL;
    } else {
        listItem = v_instance(list)->prev;
        expiryTime = c_timeSub(now, deadlineTime);
        while ((listItem != NULL) &&
               (v_objectKind(listItem) != K_DEADLINEINSTANCE)  &&
               (c_timeCompare(expiryTime, listItem->lastCheckTime) != C_LT)) {
            missed = c_iterInsert(missed, listItem);
            listItem->lastCheckTime = now;
            listItem = listItem->prev;
        }
        /* determine next wake-up time */
        if (v_objectKind(listItem) == K_DEADLINEINSTANCE) {
            /* listItem is the deadline list itself, so if there
             * were instances all instances have been missed. Just
             * set the new check to be in 'deadlineTime'.
             */
            expiryTime = deadlineTime;
        } else {
            /* 
             * The new lease duration can be calculated:
             * lastCheckTime + deadlineTime = next expiry time
             * next expiry time - now = lease duration
             */
            expiryTime = c_timeAdd(listItem->lastCheckTime, deadlineTime);
            expiryTime = c_timeSub(expiryTime, now);
            v_leaseRenew(list->deadlineLease, expiryTime);
        }
    }
    return missed;
}

c_bool
v_deadLineInstanceListEmpty(
    v_deadLineInstanceList list)
{
    if (v_instanceAlone(v_instance(list))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

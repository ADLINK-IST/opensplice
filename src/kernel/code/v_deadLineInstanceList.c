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
    if (list) {
        v_objectKind(list) = K_DEADLINEINSTANCE;
        v_instanceInit(v_instance(list));
        list->leaseManager = c_keep(leaseManager);
        list->leaseDuration = leaseDuration;
        list->deadlineLease = NULL;
        list->actionObject = o; /* no keep, since actionObject is onwer of v_deadLineInstanceList */
        list->actionId = actionId;
    } else {
        OS_REPORT(OS_ERROR,
                  "v_deadLineInstanceListNew",0,
                  "Failed to allocate v_deadLineInstanceList.");
        assert(FALSE);
    }

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
    v_kernel k;
    v_result result;

    assert(C_TYPECHECK(list,v_deadLineInstanceList));

    list->leaseDuration = duration;
    if (list->deadlineLease != NULL) {
        if (c_timeCompare(duration, C_TIME_INFINITE) != C_EQ) {
            v_leaseRenew(list->deadlineLease, &duration);
        } else {
            v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
            c_free(list->deadlineLease);
            list->deadlineLease = NULL;
        }
    } else {
        if ((v_objectKind(v_instance(list)->prev) != K_DEADLINEINSTANCE) &&  /* not in list */
            (c_timeCompare(duration, C_TIME_INFINITE) != C_EQ)) { /* new instance */
            k = v_objectKernel(list->leaseManager);
            list->deadlineLease = v_leaseNew(k, duration);
            if(list->deadlineLease)
            {
                result = v_leaseManagerRegister(
                    list->leaseManager,
                    list->deadlineLease,
                    list->actionId,
                    v_public(list->actionObject),
                    FALSE /* do not repeat lease if expired because of a periodic nature */);
                if(result != V_RESULT_OK)
                {
                    c_free(list->deadlineLease);
                    list->deadlineLease = NULL;
                    OS_REPORT_1(OS_ERROR, "v_deadLineInstanceList", 0,
                        "A fatal error was detected when trying to register the deadline lease."
                        "The result code was %d.", result);
                }
            }
        }
    }
}

/* put at end of list */
void
v_deadLineInstanceListInsertInstance(
    v_deadLineInstanceList list,
    v_instance instance,
    c_time timestamp)
{
    v_instance head = v_instance(list);
    v_kernel k;
    v_result result;

    assert(C_TYPECHECK(instance,v_instance));
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    assert(v_instanceAlone(instance));
    assert(c_refCount(list) > 0);
    assert(c_refCount(instance) > 0);

    /* As the instance is put at the end of the list no need to update the
       lease!
     */
    v_instanceUpdate(instance, timestamp); /* Updates instance checkTime */
    v_instanceAppend(head,instance);
    if (list->deadlineLease == NULL) {
        if (c_timeCompare(list->leaseDuration, C_TIME_INFINITE) != C_EQ) {
            k = v_objectKernel(list->leaseManager);
            list->deadlineLease = v_leaseNew(k, list->leaseDuration);
            if(list->deadlineLease)
            {
                result = v_leaseManagerRegister(
                    list->leaseManager,
                    list->deadlineLease,
                    list->actionId,
                    v_public(list->actionObject),
                    FALSE /* do not repeat lease if expired because of a periodic nature */);
                if(result != V_RESULT_OK)
                {
                    c_free(list->deadlineLease);
                    list->deadlineLease = NULL;
                    OS_REPORT_1(OS_ERROR, "v_deadLineInstanceList", 0,
                        "A fatal error was detected when trying to register the deadline lease."
                        "The result code was %d.", result);
                }
            }
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
    v_instance instance,
    c_time timestamp)
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
        v_deadLineInstanceListInsertInstance(list,instance,timestamp);
    } else {
        v_instanceRemove(instance);
        v_instanceUpdate(instance, timestamp); /* Updates instance checkTime */
        v_instanceAppend(v_instance(list), instance);
    }
}

c_iter
v_deadLineInstanceListCheckDeadlineMissed(
    v_deadLineInstanceList list,
    v_duration deadlineDuration,
    c_time now)
{
    v_instance listItem;
    c_iter missed;
    v_result result;

    assert(C_TYPECHECK(list,v_deadLineInstanceList));

    missed = NULL;
    if (v_instanceAlone(v_instance(list))) { /* list is empty */
        assert (list->deadlineLease != NULL);
        v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
        c_free(list->deadlineLease);
        list->deadlineLease = NULL;
    } else {
        if (list->deadlineLease != NULL) {
            c_time nextExpiration;
            c_time latestExpired = c_timeSub(now, deadlineDuration);
            listItem = v_instance(list)->prev;
            while ((listItem != v_instance(list)) && /* One full cycle */
                   (c_timeCompare(latestExpired, listItem->lastDeadlineResetTime) != C_LT)) {
                missed = c_iterInsert(missed, listItem);
                /* A deadline can be missed for an instance since the last update on
                 * the instance OR since the last notification of a missed deadline
                 * on that instance, i.e., =each deadline period= for each instance
                 * for which data was not received a deadline is missed. */
                listItem->lastDeadlineResetTime = now;
                listItem = listItem->prev;
            }
            /* determine next wake-up time */
            if (listItem != v_instance(list)) {
                /* The new lease duration can be calculated:
                 * lastDeadlineResetTime + deadlineTime = next expiry time
                 * next expiry time - now = lease duration */
                nextExpiration = c_timeAdd(listItem->lastDeadlineResetTime, deadlineDuration);
                nextExpiration = c_timeSub(nextExpiration, now); /* make relative */
            } else {
                /* listItem is the deadline list itself, so if there
                 * were instances all instance-deadlines have been missed. Just
                 * renew the lease with deadlineDuration. NOTE: while the lease is
                 * normally automatically repeated with the current lease-duration,
                 * that duration can be too short (due to condition above), so it
                 * has to be reset explicitly here. */
                nextExpiration = deadlineDuration;
            }

            result = v_leaseManagerRegister(list->leaseManager,list->deadlineLease, list->actionId,v_public(list->actionObject),FALSE);
            if(result != V_RESULT_OK)
            {
                c_free(list->deadlineLease);
                list->deadlineLease = NULL;
                OS_REPORT_1(OS_ERROR, "v_deadLineInstanceList", 0,
                    "A fatal error was detected when trying to register the deadline lease."
                    "The result code was %d.", result);
            } else {
                v_leaseRenew(list->deadlineLease, &nextExpiration);
            }
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

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
#include "v__deadLineInstanceList.h"
#include "v__deadLineInstance.h"

#include "v_instance.h"
#include "v__lease.h"
#include "v__lease.h"
#include "v__leaseManager.h"
#include "v__status.h"

#include "vortex_os.h"
#include "os_report.h"

#define _CHECK_LIST_(list) assert(((list->head != NULL) && (list->tail != NULL)) || \
                                  ((list->head == NULL) && (list->tail == NULL)))

#define _CHECK_HEAD_(list, inst) \
        ((list->head == inst) ? assert(inst->prev == NULL) : assert(inst->prev != NULL))

#define _CHECK_TAIL_(list, inst) \
        ((list->tail == inst) ? assert(inst->next == NULL) : assert(inst->next != NULL))

v_deadLineInstanceList
v_deadLineInstanceListNew(
    c_base base,
    v_leaseManager leaseManager,
    os_duration leaseDuration,
    v_leaseActionId actionId,
    v_public o)
{
    v_deadLineInstanceList list;
    c_type type;

    assert(C_TYPECHECK(leaseManager,v_leaseManager));
    assert(C_TYPECHECK(o,v_public));

    type = c_resolve(base, "kernelModuleI::v_deadLineInstanceList");
    assert(type);
    list = c_new(type);
    c_free(type);
    if (list) {
        v_objectKind(list) = K_DEADLINEINSTANCELIST;
        list->leaseManager = c_keep(leaseManager);
        list->leaseDuration = leaseDuration;
        list->deadlineLease = NULL;
        list->actionObject = o; /* no keep, since actionObject is onwer of v_deadLineInstanceList */
        list->actionId = actionId;
        list->head = NULL;
        list->tail = NULL;
    } else {
        OS_REPORT(OS_FATAL,
                  "v_deadLineInstanceListNew",V_RESULT_INTERNAL_ERROR,
                  "Failed to allocate v_deadLineInstanceList.");
        assert(FALSE);
    }

    _CHECK_LIST_(list);
    return list;
}

void
v_deadLineInstanceListFree(
    v_deadLineInstanceList list)
{
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    _CHECK_LIST_(list);

    /* Stop lease manager activity.
     */
    v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
    c_free(list->leaseManager);
    list->leaseManager = NULL;
    c_free(list->deadlineLease);
    list->deadlineLease = NULL;

    /* Free contained deadLine Instances.
     */
    while (list->head != NULL) {
        assert(list->tail != NULL);
        v_deadLineInstanceListRemoveInstance(list, list->head);
    }
    assert(list->tail == NULL);
    _CHECK_LIST_(list);
}

void
v_deadLineInstanceListSetDuration(
    v_deadLineInstanceList list,
    os_duration duration)
{
    v_kernel k;
    v_result result;

    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    _CHECK_LIST_(list);

    list->leaseDuration = duration;
    if (list->deadlineLease != NULL) {
        if (duration != OS_DURATION_INFINITE) {
            v_leaseRenew(list->deadlineLease, duration);
        } else {
            v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
            c_free(list->deadlineLease);
            list->deadlineLease = NULL;
        }
    } else {
        if ((list->head != NULL) && (duration != OS_DURATION_INFINITE)) {
            k = v_objectKernel(list->leaseManager);
            list->deadlineLease = v_leaseElapsedNew(k, duration);
            if(list->deadlineLease)
            {
                result = v_leaseManagerRegister(
                    list->leaseManager,
                    list->deadlineLease,
                    list->actionId,
                    v_public(list->actionObject),
                    TRUE /* repeat lease if expired */);
                if(result != V_RESULT_OK)
                {
                    c_free(list->deadlineLease);
                    list->deadlineLease = NULL;
                    OS_REPORT(OS_CRITICAL, "v_deadLineInstanceList", result,
                        "A fatal error was detected when trying to register the deadline lease."
                        "The result code was %d.", result);
                }
            }
        }
    }
    _CHECK_LIST_(list);
}

/* put at end of list */
void
v_deadLineInstanceListInsertInstance(
    v_deadLineInstanceList list,
    v_deadLineInstance instance)
{
    v_kernel k;
    v_result result;

    assert(C_TYPECHECK(instance,v_deadLineInstance));
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    assert(v_deadLineInstance_alone(instance));
    assert(c_refCount(list) > 0);
    assert(c_refCount(instance) > 0);
    _CHECK_LIST_(list);
    _CHECK_HEAD_(list, instance);
    _CHECK_TAIL_(list, instance);

    /* As the instance is put at the end of the list no need to update the
       lease!
     */
    if (list->head == NULL) {
        assert(list->tail == NULL);
        list->head = instance;
        list->tail = instance;
        instance->next = NULL;
        instance->prev = NULL;
    } else {
        assert(list->tail != NULL);
        assert(list->tail->next == NULL);
        list->tail->next = instance;
        instance->prev = list->tail;
        list->tail = instance;
        instance->next = NULL;
    }
    if (list->deadlineLease == NULL) {
        k = v_objectKernel(list->leaseManager);
        list->deadlineLease = v_leaseElapsedNew(k, list->leaseDuration);
        if(list->deadlineLease) {
            result = v_leaseManagerRegister(
                list->leaseManager,
                list->deadlineLease,
                list->actionId,
                v_public(list->actionObject),
                TRUE /* repeat lease if expired */);
            if(result != V_RESULT_OK)
            {
                c_free(list->deadlineLease);
                list->deadlineLease = NULL;
                OS_REPORT(OS_CRITICAL, "v_deadLineInstanceList", result,
                    "A fatal error was detected when trying to register the deadline lease."
                    "The result code was %d.", result);
            }
        }
    }
    _CHECK_LIST_(list);
    _CHECK_HEAD_(list, instance);
    _CHECK_TAIL_(list, instance);
}

void
v_deadLineInstanceListRemoveInstance(
    v_deadLineInstanceList list,
    v_deadLineInstance instance)
{
    assert(list != NULL);
    assert(instance != NULL);
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    assert(c_refCount(list) > 0);
    assert(c_refCount(instance) > 0);
    _CHECK_LIST_(list);
    _CHECK_HEAD_(list, instance);
    _CHECK_TAIL_(list, instance);

    /* As the instance is removed, there is no need to update the lease.
     * Updating the lease might trigger the leasemanager thread to determine
     * the next expiry time. The next expiry time can also be determined on
     * the next deadline check, which is more efficient as the administration
     * might already have changed many times.
     */
    /* Update next pointer list.
     */
    if (list->head == instance) {
        assert(instance->prev == NULL);
        list->head = instance->next;
        if (list->head != NULL) {
            list->head->prev = NULL;
        }
    } else {
        assert(instance->prev != NULL);
        v_deadLineInstance(instance->prev)->next = instance->next;
    }

    /* Update prev pointer list.
     */
    if (list->tail == instance) {
        assert(instance->next == NULL);
        list->tail = instance->prev;
        if (list->tail != NULL) {
            list->tail->next = NULL;
        }
    } else {
        assert(instance->next != NULL);
        v_deadLineInstance(instance->next)->prev = instance->prev;
    }

    instance->next = instance;
    instance->prev = instance;

    /* Update Lease manager registration.
     */
    if (list->head == NULL) {
        assert(list->tail == NULL);
        if (list->deadlineLease != NULL) {
            v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
            c_free(list->deadlineLease);
            list->deadlineLease = NULL;
        }
    }
    _CHECK_LIST_(list);
    _CHECK_HEAD_(list, instance);
    _CHECK_TAIL_(list, instance);
}

void
v_deadLineInstanceListUpdate(
    v_deadLineInstanceList list,
    v_deadLineInstance instance,
    os_timeE timestamp)
{
    assert(list != NULL);
    assert(instance != NULL);
    assert(C_TYPECHECK(instance,v_deadLineInstance));
    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    _CHECK_LIST_(list);
    _CHECK_HEAD_(list, instance);
    _CHECK_TAIL_(list, instance);
    assert(c_refCount(list) > 0);
    assert(c_refCount(instance) > 0);

    /* This will also place the current instance at the end of the list.
       Again no need to update the lease, we can determine the next wake-up
       as soon as the next deadlinecheck is performed.
    */
    if (list->leaseDuration != OS_DURATION_INFINITE) {
        instance->lastDeadlineResetTime = timestamp;
        if (v_deadLineInstance_alone(instance)) {
            v_deadLineInstanceListInsertInstance(list, instance);
        } else {
            if (instance != list->tail) {
                /* Remove from next pointer list.
                 */
                if (list->head == instance) {
                    assert(instance->prev == NULL);
                    list->head = instance->next;
                    if (list->head != NULL) {
                        list->head->prev = NULL;
                    }
                } else {
                    assert(instance->prev != NULL);
                    v_deadLineInstance(instance->prev)->next = instance->next;
                }
                /* Remove from prev pointer list.
                 */
                assert(instance->next != NULL);
                v_deadLineInstance(instance->next)->prev = instance->prev;

                /* Reset deadline list timestamp and reinsert instance at the tail of the list.
                 */
                assert(list->tail != NULL);
                assert(list->tail->next == NULL);
                list->tail->next = instance;
                instance->prev = list->tail;
                list->tail = instance;
                instance->next = NULL;
            }
        }
    }
    _CHECK_LIST_(list);
    _CHECK_HEAD_(list, instance);
    _CHECK_TAIL_(list, instance);
}

c_iter
v_deadLineInstanceListCheckDeadlineMissed(
    v_deadLineInstanceList list,
    os_duration deadlineDuration,
    os_timeE now)
{
    v_deadLineInstance lastItem;
    v_deadLineInstance listItem;
    c_iter missed;

    assert(C_TYPECHECK(list,v_deadLineInstanceList));
    _CHECK_LIST_(list);

    missed = NULL;
    if (list->head == NULL) { /* list is empty */
        /*
         * expected was that the deadlineLease should be != NULL
         * However when an entity is being deleted this lease will be deleted but the
         * leaseManager of the participant may already have received a deadline
         * missed notification and will call this operation and when the deletion has not
         * finished it will be possible to claim the entity.
         *
         * It is however not a problem to continue so only de-register it if
         * it is not NULL.
         */
        if(list->deadlineLease != NULL){
            v_leaseManagerDeregister(list->leaseManager, list->deadlineLease);
            c_free(list->deadlineLease);
            list->deadlineLease = NULL;
        }
    } else {
        os_duration nextExpiration;
        os_timeE latestExpired = os_timeESub(now, deadlineDuration);
        lastItem = list->tail;
        listItem = list->head;
        while (os_timeECompare(latestExpired, listItem->lastDeadlineResetTime) != OS_LESS)
        {
            missed = c_iterAppend(missed, listItem);
            /* A deadline can be missed for an instance since the last update on
             * the instance OR since the last notification of a missed deadline
             * on that instance, i.e., =each deadline period= for each instance
             * for which data was not received a deadline is missed. */
            listItem->lastDeadlineResetTime = now;

            if (listItem->next) {
                list->head = listItem->next;
                list->head->prev = NULL;
                listItem->next = NULL;
                list->tail->next = listItem;
                listItem->prev = list->tail;
                list->tail = listItem;
            }
            if (listItem == lastItem) {
                /* One full cycle completed: quit! */
                break;
            }
            listItem = list->head;
        }
        /* determine next wake-up time */
        if (listItem != NULL) {
            /* The new lease duration can be calculated:
             * lastDeadlineResetTime + deadlineTime = next expiry time
             * next expiry time - now = lease duration */
            latestExpired = os_timeEAdd(listItem->lastDeadlineResetTime, deadlineDuration);
            nextExpiration = os_timeEDiff(latestExpired, now); /* make relative */
        } else {
            /* listItem is the deadline list itself, so if there
             * were instances all instance-deadlines have been missed. Just
             * renew the lease with deadlineDuration. NOTE: while the lease is
             * normally automatically repeated with the current lease-duration,
             * that duration can be too short (due to condition above), so it
             * has to be reset explicitly here. */
            nextExpiration = deadlineDuration;
        }
        v_leaseRenew(list->deadlineLease, nextExpiration);
    }
    _CHECK_LIST_(list);
    return missed;
}

c_bool
v_deadLineInstanceListEmpty(
    v_deadLineInstanceList list)
{
    _CHECK_LIST_(list);
    return (list->head == NULL);
}

void
v_deadLineInstanceInit(
    v_deadLineInstance _this,
    v_entity entity)
{
    assert(C_TYPECHECK(_this, v_deadLineInstance));

    /* Public part is initialised at reader or writer */

    v_instanceInit(v_instance(_this), entity);

    /* Elements that are not in a list will refer to itself. */
    _this->next = _this;
    _this->prev = _this;
    _this->lastDeadlineResetTime = OS_TIMEE_ZERO;
}

void
v_deadLineInstanceDeinit(
    v_deadLineInstance _this)
{
    assert(C_TYPECHECK(_this, v_deadLineInstance));

    /* Can only deinit an instance if it is not part of a list. */
    assert(_this->next == _this);
    assert(_this->prev == _this);

    _this->lastDeadlineResetTime = OS_TIMEE_ZERO;
    v_instanceDeinit(v_instance(_this));
}


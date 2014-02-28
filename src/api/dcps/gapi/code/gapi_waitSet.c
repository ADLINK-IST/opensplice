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
#include "gapi_waitSet.h"
#include "gapi_waitSetDomainEntry.h"
#include "gapi_condition.h"
#include "gapi_status.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_domainParticipant.h"
#include "gapi_fooDataReader.h"
#include "gapi_entity.h"

#include "u_waitset.h"
#include "v_event.h"
#include "v_handle.h"
#include "v_entity.h"
#include "v_query.h"
#include "os_heap.h"
#include "os_report.h"

#define _ConditionEntry(o) \
        ((_ConditionEntry)(o))

#define getDomainCount(waitset) \
        c_iterLength(((_WaitSet)waitset)->domains)

C_CLASS(_ConditionEntry);
C_STRUCT(_ConditionEntry) {
    _ConditionEntry     next;
    _Condition          condition;
    gapi_boolean        active;
    _WaitSetDomainEntry domain;
};

static gapi_returnCode_t
getConditionDomainId(
    _Condition condition,
    gapi_domainName_t *id);

static gapi_equality
domainCompare(
    _Object domain_a,
    _Object domain_b)
{
    if (domain_a == domain_b) {
        /* for pointers to the same string */
        /* mainly here to prevent NULL domain_ID's to crash the strcmp()  */
        return GAPI_EQ;
    }
    if (strcmp((char*)domain_a,(char*)domain_b) == 0) {
        return GAPI_EQ;
    }
    return GAPI_NE;
}

gapi_boolean
_WaitSetFree(
    void *_waitset)
{
    _ConditionEntry c_entry;
    _WaitSetDomainEntry wsde;
    _WaitSet waitset = (_WaitSet)_waitset;

    if (waitset->busy) {
        os_condBroadcast(&waitset->cv);
        _ObjectRelease((_Object)_waitset);
        _ObjectClaimNotBusy((_Object)_waitset);
    }
    assert(waitset->busy == FALSE);

    while (waitset->conditions != NULL) {
        c_entry = waitset->conditions;
        waitset->conditions = c_entry->next;
        _EntityClaim(c_entry->condition);
        if (c_entry->condition != NULL) {
            if (c_entry->domain) {
                /* only if attached to a domain */
                _WaitSetDomainEntryDetachCondition(c_entry->domain,
                                                   c_entry->condition);
            } else {
                _ConditionRemoveWaitset(c_entry->condition,
                                        _EntityHandle(waitset),
                                        NULL);
            }
            _EntityRelease(c_entry->condition);
        }
        os_free(c_entry);
    }
    waitset->length = 0;

    /* now iterate though all WaitSetDomainEnties, and remove them */
    if (waitset->domains) {
        wsde = _WaitSetDomainEntry(c_iterTakeFirst(waitset->domains));
        while (wsde != NULL) {
            _WaitSetDomainEntryDelete(wsde);
            wsde = _WaitSetDomainEntry(c_iterTakeFirst(waitset->domains));
        }
        c_iterFree(waitset->domains);
        waitset->domains = NULL;
    }
    os_condDestroy(&waitset->cv);
    os_mutexDestroy(&waitset->mutex);
    return TRUE;
}

_WaitSet
_WaitSetNew(void)
{
    _WaitSet newWaitSet;
    os_result osResult;

    newWaitSet = _WaitSetAlloc();

    if (newWaitSet) {
        newWaitSet->busy       = FALSE;
        newWaitSet->multidomain= TRUE;
        newWaitSet->conditions = NULL;
        newWaitSet->length     = 0;
        newWaitSet->domains    = NULL;
    }

    if (newWaitSet) {
        os_mutexAttr osMutexAttr;
        osResult = os_mutexAttrInit(&osMutexAttr);
        if (osResult == os_resultSuccess) {
            osMutexAttr.scopeAttr = OS_SCOPE_PRIVATE;
            osResult = os_mutexInit(&newWaitSet->mutex, &osMutexAttr);
            if (osResult != os_resultSuccess) {
                gapi_free(newWaitSet);
                newWaitSet = NULL;
            }
        } else {
            gapi_free(newWaitSet);
            newWaitSet = NULL;
        }
    }

    if (newWaitSet) {
        os_condAttr osCondAttr;
        osResult = os_condAttrInit(&osCondAttr);
        if (osResult == os_resultSuccess) {
            osCondAttr.scopeAttr = OS_SCOPE_PRIVATE;
            osResult = os_condInit(&newWaitSet->cv, &newWaitSet->mutex, &osCondAttr);
            if (osResult != os_resultSuccess) {
                gapi_free(newWaitSet);
                newWaitSet = NULL;
            }
        } else {
            gapi_free(newWaitSet);
            newWaitSet = NULL;
        }
    }
    /* Add the new waitset to the DomainParticipantFactory, so we can cleanup
     * when the process terminates
     */
    _DomainParticipantFactoryRegister((_Object)newWaitSet);

    return newWaitSet;
}

static void
notifyDomain (
    c_object o,
    c_voidp arg)
{
    _WaitSetDomainEntry entry = (_WaitSetDomainEntry)o;

    u_waitsetNotify(entry->uWaitset, arg);
}

void
_WaitSetNotify(
    _WaitSet _this,
    _Condition cond)
{
    if (_this->busy) {
        if (_this->multidomain) {
            os_condSignal(&_this->cv);
        } else {
            c_iterWalk(_this->domains, notifyDomain, (c_voidp)cond);
        }
    }
}

gapi_waitSet
gapi_waitSet__alloc(void)
{
    return (gapi_waitSet)_EntityRelease(_WaitSetNew());
}

struct TestAndListArg {
    gapi_conditionSeq *conditions;
    _WaitSet waitset;
};


static void
TestAndList(
    v_waitsetEvent _this,
    c_voidp arg)
{
    struct TestAndListArg *a = (struct TestAndListArg *)arg;
    v_handleResult r;
    v_entity e;
    u_entity ue;
    _Object go;
    _Condition cd;
    _ConditionEntry entry;
    GetTriggerValue getTriggerValue;

    if (_this) {
        /* The source field of the event normally contains a handle to the
         * Condition that generated the event. Since the GuardCondition is
         * not implemented on kernel level, an event can not have a handle to it
         * so it uses the handle to the WaitSet instead. That is how we can
         * distinguish between events coming from GuardConditions and from
         * other Conditions.
         */
        r = v_handleClaim(_this->source,(v_object*)&e);
        if (r == V_HANDLE_OK) {
            ue = v_entityGetUserData(e);
            if (ue) {
                go = u_entityGetUserData(ue);
                /* If the event source is our WaitSet, then the event originates
                 * from a GuardCondition. Otherwise it originates from another
                 * type of Condition, either a ReadCondition or a StatusCondition.
                 */
                if (go == (_Object) a->waitset) {
                    /*
                     * _GuardConditionGetTriggerValue(cd) =>
                     *                  cd->triggerValue;
                     *
                     * In case the event originates from a GuardCondition, the
                     * creator of the event pushed a pointer to the corresponding
                     * _GuardCondition object in the gapi in the userData of the
                     * event.
                     */
                    cd = (_Condition)_this->userData;

                    /* Now we need to check whether this GuardCondition is still
                     * attached to the WaitSet. The following lookup of the condition
                     * is a poor mans patch for that purpose. Actually it should
                     * not happen that an event exists while the originating
                     * condition is already detached.
                     * Expected behavior is that the detach of a condition waits
                     * until all events originating from that condition are processed.
                     * In the current implementation this is not possible because guard
                     * conditions only exists in the gapi.
                     * A detach of a guard condition will therefore not invoke the
                     * kernel and therefore will not be aware of any pending events
                     * as these are passed via the kernel.
                     */
                    entry = a->waitset->conditions;
                    while ((entry != NULL) && (entry->condition != cd)) {
                        entry = entry->next;
                    }
                    if (entry) {
                        getTriggerValue = cd->getTriggerValue;

                        if (_ObjectIsValid(_Object(cd))) {
                            if (getTriggerValue(cd)) {
                                /* gapi_waitSet_wait always initializes the conditions
                                 * sequence with enough buffer space to store the worst-
                                 * case number of elements, i.e. the case where all
                                 * conditions attached to the waitset have triggered.
                                 */
                                if (a->conditions->_length < a->conditions->_maximum) {
                                    a->conditions->_buffer[a->conditions->_length++] = _EntityHandle(cd);
                                }
                            }
                        }
                    }
                } else {
                    /* _ReadConditionGetTriggerValue(cd) =>
                     *                  u_queryTriggerTest(cd->uQuery);
                     * _StatusConditionGetTriggerValue(cd) =>
                     *                  gapi_entity_get_status_changes(entity) &
                     *                  statuscondition->enabledStatusMask;
                     */
                    /* Should'nt the following operations make sure that the objects
                     * are locked?
                     */

                    if (u_entityKind(ue) == U_QUERY) {
                        cd = (_Condition) go;
                        if (_ObjectIsValid(_Object(cd))) {
                            if (v_queryTest(v_query(e), gapi_matchesReaderMask, &((_ReadCondition)cd)->readerMask)) {
                                if (a->conditions->_length < a->conditions->_maximum) {
                                    a->conditions->_buffer[a->conditions->_length++] = _EntityHandle(cd);
                                }
                            }
                        }
                    } else {
                        _Entity entity;
                        gapi_statusMask currentStatus;

                        entity = _Entity(gapi_objectPeekUnchecked((gapi_object)go));
                        cd = _Condition(_EntityStatusCondition(entity));

                        currentStatus = _StatusGetMaskStatus(cd->entity->status,v_statusGetMask(e->status));

                        if (currentStatus & ((_StatusCondition)cd)->enabledStatusMask) {
                            if (a->conditions->_length < a->conditions->_maximum) {
                                a->conditions->_buffer[a->conditions->_length++] = _EntityHandle(cd);
                            }
                        }
                    }
                }
            }
            v_handleRelease(_this->source);
        }
    }
}


gapi_returnCode_t
gapi_waitSet_wait(
    gapi_waitSet _this,
    gapi_conditionSeq *conditions,
    const gapi_duration_t *timeout)
{
    _WaitSet          waitset;
    gapi_returnCode_t result      = GAPI_RETCODE_OK;
    _ConditionEntry   entry;
    os_time           osTime;
    os_time           osTimeMax;
    os_time           osTimeWait;
    gapi_boolean      infinite    = FALSE;
    gapi_boolean      ready       = FALSE;
    os_result         osResult    = os_resultSuccess;
    u_result          uResult     = U_RESULT_OK;
    _WaitSetDomainEntry wsentry;
    c_time waitTime;
    struct TestAndListArg args;

    waitset = gapi_waitSetClaim(_this, &result);

    if (waitset == NULL) {
        return result;
    }
    _ObjectSetBusy((_Object)waitset);

    if (waitset->busy) {
        _EntityRelease(waitset);
        return GAPI_RETCODE_PRECONDITION_NOT_MET;
    }

    waitset->busy = TRUE;

    /* First evaluate all condition.
     * Return the list of 'TRUE' conditions if at least one of the
     * conditions is TRUE.
     * If none of the conditions is TRUE then wait for a trigger and
     * when triggered test again.
     */
    conditions->_length = 0;

    args.conditions = conditions;
    args.waitset = waitset;

    /* If timeout is not infinite, calculate the absolute time
     * for the timeout.
     */
    if(!((timeout->sec == GAPI_DURATION_INFINITE_SEC) &&
        (timeout->nanosec == GAPI_DURATION_INFINITE_NSEC)))
    {
        os_time osTimeOut;

        osTimeMax = os_timeGet();
        osTimeOut.tv_sec  = (os_timeSec)timeout->sec;
        osTimeOut.tv_nsec = (os_int32)timeout->nanosec;

        osTimeMax = os_timeAdd(osTimeMax, osTimeOut);
    }


    do {
        /* Make sure the 'conditions' sequence has enough buffer space to store
         * the worst-case number of elements, i.e. the case where all conditions
         * attached to the waitset have triggered.
         */
        gapi_sequence_replacebuf(conditions,
                             (_bufferAllocatorType)gapi_conditionSeq_allocbuf,
                             waitset->length);

        entry = waitset->conditions;
        while (entry != NULL) {
            _EntityClaim(entry->condition);
            if (entry->condition->getTriggerValue(entry->condition)) {
                conditions->_buffer[conditions->_length++] = _EntityHandle(entry->condition);
                ready = TRUE;
            }
            _EntityRelease(entry->condition);
            entry = entry->next;
        }

        if ((timeout->sec    == GAPI_DURATION_ZERO_SEC) &&
            (timeout->nanosec == GAPI_DURATION_ZERO_NSEC)) {
            ready = TRUE;
            if (conditions->_length > 0) {
                result = GAPI_RETCODE_OK;
            } else {
                result = GAPI_RETCODE_TIMEOUT;
            }
        }

        if (!ready) {
            /* None of the conditions is TRUE so calculate the wait time and
             * wait for the next event or timeout.
             */
            if ((timeout->sec     == GAPI_DURATION_INFINITE_SEC) &&
                (timeout->nanosec == GAPI_DURATION_INFINITE_NSEC)) {
                infinite = TRUE;
            }

            if (waitset->multidomain) {
                /* Enter the multi-domain wait. */
                if (infinite) {
                    /* Wait for an event to occur. */
                    osResult = _ObjectWait((_Object)waitset, &waitset->cv);
                } else {
                    osTime = os_timeGet();
                    if (os_timeCompare(osTime, osTimeMax) == OS_LESS) {
                        osTimeWait = os_timeSub(osTimeMax, osTime);
                        /* Wait until an event or a timeout occurs. */
                        osResult = _ObjectTimedWait((_Object)waitset,
                                                    &waitset->cv,
                                                    &osTimeWait);
                    } else {
                        /* Time to wait has already passed. */
                        osResult = os_resultTimeout;
                    }
                }

                /* Determine the result. */
                if (_ObjectIsValid((_Object)waitset)) {
                    switch (osResult) {
                    case os_resultSuccess:
                        /* multi-domain threads cannot handle event data yet!
                         * therefore reevaluate all conditions here until
                         * the problem is fixed.
                         */
                        gapi_sequence_replacebuf(conditions,
                                                 (_bufferAllocatorType)gapi_conditionSeq_allocbuf,
                                                 waitset->length);
                        entry = waitset->conditions;
                        while (entry != NULL) {
                            _EntityClaim(entry->condition);
                            if (entry->condition->getTriggerValue(entry->condition)) {
                                conditions->_buffer[conditions->_length++] =
                                _EntityHandle(entry->condition);
                                ready = TRUE;
                            }
                            _EntityRelease(entry->condition);
                            entry = entry->next;
                        }
                    break;
                    case os_resultTimeout:
                        result = GAPI_RETCODE_TIMEOUT;
                        ready = TRUE;
                    break;
                    default:
                        assert(FALSE);
                        result = GAPI_RETCODE_ERROR;
                        ready = TRUE;
                    }
                } else {
                    result = GAPI_RETCODE_ALREADY_DELETED;
                    ready = TRUE;
                }
            } else {
                /* Enter the single domain wait. */
                wsentry = _WaitSetDomainEntry(c_iterObject(waitset->domains,0));
                if (wsentry) {
                    _EntityRelease(waitset);
                    wsentry->busy = TRUE;

                    if (infinite) {
                        /* Wait for an event to occur. */
                        uResult = u_waitsetWaitAction(wsentry->uWaitset,
                                                      TestAndList,
                                                      &args);
                    } else {
                        osTime = os_timeGet();
                        if (os_timeCompare(osTime, osTimeMax) == OS_LESS) {
                            osTimeWait = os_timeSub(osTimeMax, osTime);
                            waitTime.seconds = osTimeWait.tv_sec;
                            waitTime.nanoseconds = osTimeWait.tv_nsec;
                            /* Wait until an event or a timeout occurs. */
                            uResult = u_waitsetTimedWaitAction(wsentry->uWaitset,
                                                               TestAndList,
                                                               &args,
                                                               waitTime);
                        } else {
                            /* Time to wait has already passed. */
                            uResult = U_RESULT_TIMEOUT;
                        }
                    }
                    wsentry->busy = FALSE;
                    _EntityClaim(waitset);

                    /* Determine the result. */
                    if (_ObjectIsValid((_Object)waitset)) {
                        switch (uResult) {
                        case U_RESULT_OK:
                            if (conditions->_length > 0) {
                                result = GAPI_RETCODE_OK;
                                ready = TRUE;
                            }
                        break;
                        case U_RESULT_TIMEOUT:
                            result = GAPI_RETCODE_TIMEOUT;
                            ready = TRUE;
                        break;
                        case U_RESULT_ILL_PARAM:
                        case U_RESULT_DETACHING:
                        case U_RESULT_ALREADY_DELETED:
                            result = GAPI_RETCODE_ALREADY_DELETED;
                            ready = TRUE;
                        break;
                        case U_RESULT_INTERNAL_ERROR:
                            result = GAPI_RETCODE_ERROR;
                            ready = TRUE;
                        break;
                        default:
                            assert(FALSE);
                            result = GAPI_RETCODE_ERROR;
                            ready = TRUE;
                        }
                    } else {
                        result = GAPI_RETCODE_ALREADY_DELETED;
                        ready = TRUE;
                    }

                } else {
                    assert(waitset->multidomain == TRUE);
                }
            }
        }
    } while (!ready);

    waitset->busy = FALSE;
    _EntityRelease(waitset);
    gapi_objectClearBusy(_this);

    return result;
}


static void
set_multi_node (
    c_object o,
    c_voidp arg)
{
    _WaitSetDomainEntry entry = (_WaitSetDomainEntry)o;

    _WaitSetDomainEntryMultiMode(entry, *(c_bool *)arg);
}

static void
_WaitSet_set_multi_mode(
    _WaitSet _this)
{
    c_bool mode;

    /* Only in case of one domain the waitset can operate in
     * the single domain mode. If a waitset does not contain
     * domain related conditions (i.e. only zero or more guard
     * conditions) then it will operate in the multi mode.
     * In this case there is not a single domain and the waitset
     * is still in the single domain mode so it will be set to
     * the multi domain mode including all waitset entries.
     */
    mode = (getDomainCount(_this) != 1); /* multi mode if (count != 1). */

    if (_this->multidomain != mode) {
        c_iterWalk(_this->domains, set_multi_node, (c_voidp)&mode);
        /* First notify according to the current mode before modifying the mode
         * otherwise triggers will not arrive!
         */
        _WaitSetNotify(_this, NULL);
        _this->multidomain = mode;
    }
}

static c_equality
compareDomainId (
    c_object o,
    c_voidp arg)
{
    _WaitSetDomainEntry entry = (_WaitSetDomainEntry)o;
    gapi_domainName_t *id = (gapi_domainName_t *)arg;
    c_equality result;

    if (entry->domainId == *id) {
        result = C_EQ;
    } else if (*id == NULL) {
        result = C_NE;
    } else if (strcmp(entry->domainId, *id) == 0) {
        result = C_EQ;
    } else {
        result = C_NE;
    }
    return result;
}

gapi_returnCode_t
gapi_waitSet_attach_condition(
    gapi_waitSet _this,
    const gapi_condition cond)
{
    gapi_domainName_t DomainId;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _WaitSetDomainEntry wsdEntry = NULL;
    _WaitSet waitset;
    _Condition condition;
    _ConditionEntry entry;

    waitset = gapi_waitSetClaim(_this, &result);
    if (waitset != NULL) {
        condition = _ConditionFromHandle(cond);
        if (condition != NULL) {
            entry = waitset->conditions;
            while ((entry != NULL) && (entry->condition != condition)) {
                entry = entry->next;
            }
            if (entry == NULL) {
                /* The given condition is not yet attached.
                 */
                if (_ConditionKind(condition) == OBJECT_KIND_GUARDCONDITION) {
                    /* The condition is a guard condition and is not part of
                     * a domain. Therefore the waitset can be attached directly
                     * to the guard condition.
                     * The guard condition can immediatly trigger the waitset
                     * upon notification.
                     */
                    result = _ConditionAddWaitset(condition,
                                                  _EntityHandle(waitset),
                                                  NULL);
                } else {
                    result = getConditionDomainId(condition, &DomainId);
                    if (result == GAPI_RETCODE_OK) {
                        wsdEntry = c_iterResolve(waitset->domains, compareDomainId, &DomainId);
                        if (wsdEntry == NULL) {
                            /* Create a new WaitSetDomainEntry */
                            wsdEntry = _WaitSetDomainEntryNew(waitset, DomainId);
                            if (wsdEntry) {
                                waitset->domains = c_iterInsert(waitset->domains, wsdEntry);
                            } else {
                                result = GAPI_RETCODE_OUT_OF_RESOURCES;
                                OS_REPORT_2(OS_WARNING,
                                            "gapi_waitSet_attach_condition", 0,
                                            "Attach Condition 0x%x to Waitset 0x%x skipped, "
                                            "Out of resources, failed to allocate internal WaitSet entry.",
                                            cond, _this);
                            }
                        }

                        /* Now there is a domain-entry for this condition,
                         * lets add it.
                         */
                        if (result == GAPI_RETCODE_OK) {
                            _EntityClaim(condition);
                            result = _WaitSetDomainEntryAttachCondition(wsdEntry,
                                                                        condition);
                            _EntityRelease(condition);
                        }
                        _WaitSet_set_multi_mode(waitset);
                    } else {
                        OS_REPORT_2(OS_WARNING, "gapi_waitSet_attach_condition", 0,
                                    "Attach Condition 0x%x to Waitset 0x%x skipped, "
                                    "Could not get the Domain Id from the Condition.",
                                    cond, _this);
                    }
                }

                if (result == GAPI_RETCODE_OK) {
                    entry = os_malloc(C_SIZEOF(_ConditionEntry));
                    if (entry) {
                        entry->active = FALSE;
                        entry->condition = condition;
                        entry->domain = wsdEntry; /* NULL for guard-conditions */
                        entry->next = waitset->conditions;
                        waitset->conditions = entry;
                        waitset->length++;
                    } else {
                        OS_REPORT_2(OS_WARNING,
                                    "gapi_waitSet_attach_condition", 0,
                                    "Attach Condition 0x%x to Waitset 0x%x skipped, "
                                    "Out of resources, failed to allocate internal WaitSet entry.",
                                    cond, _this);
                        result = GAPI_RETCODE_OUT_OF_RESOURCES;
                    }
                }

                if (result == GAPI_RETCODE_OK) {
                    if (waitset->busy) {
                        _EntityClaim(condition);
                        if (condition->getTriggerValue(condition)) {
                            _WaitSetNotify(waitset, condition);
                        }
                        _EntityRelease(condition);
                    }
                }
            } else {
                OS_REPORT_2(OS_INFO, "gapi_waitSet_attach_condition", 0,
                            "Attach Condition 0x%x to Waitset 0x%x skipped, "
                            "The Condition was already attached.",
                            cond, _this);
            }
        } else {
            /* The given condition is already attached to the waitset.
             * return BAD PARAMETER according to the DDS specification.
             */
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }
    _EntityRelease(waitset);

    return result;
}

gapi_returnCode_t
gapi_waitSet_detach_condition(
    gapi_waitSet _this,
    const gapi_condition cond)
{
    _WaitSet waitset;
    _Condition condition;
    _ConditionEntry entry, prev;
    _WaitSetDomainEntry wsdEntry = NULL;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_domainName_t DomainId;

    waitset = gapi_waitSetClaim(_this, &result);
    if (waitset != NULL) {
        condition = _ConditionFromHandle(cond);
        if (condition != NULL) {
            prev = NULL;
            entry = waitset->conditions;
            while ((entry != NULL) && (entry->condition != condition)) {
                prev = entry;
                entry = entry->next;
            }
            if (entry != NULL) {
                /* The condition is found in the waitset admin and can now
                 * be removed from the waitset.
                 */
                if (prev == NULL) {
                    waitset->conditions = entry->next;
                } else {
                    prev->next = entry->next;
                }
                entry->next = NULL;
                waitset->length--;

                /* detach condition from domain-entry.
                 * ( not for guardconditions)
                 */
                _EntityClaim(condition);
                if (entry->domain) {
                    result =_WaitSetDomainEntryDetachCondition(entry->domain,
                                                               condition);
                    if (_WaitSetDomainEntryConditionCount(entry->domain) == 0) {
                        result = getConditionDomainId(condition, &DomainId);
                        if (result == GAPI_RETCODE_OK) {
                            wsdEntry = c_iterResolve(waitset->domains, compareDomainId, &DomainId);
                            if (wsdEntry != NULL) {
                                c_iterTake(waitset->domains, wsdEntry);
                                _WaitSetDomainEntryDelete(wsdEntry);
                            }
                        }
                    }
                } else {
                    result = _ConditionRemoveWaitset(condition,
                                                     _EntityHandle(waitset),
                                                     NULL);
                }
                _WaitSet_set_multi_mode(waitset);
                _EntityRelease(condition);
                os_free(entry);
            } else {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            }
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }
    }
    _EntityRelease(waitset);

    return result;
}

gapi_returnCode_t
gapi_waitSet_get_conditions(
    gapi_waitSet _this,
    gapi_conditionSeq *attached_conditions)
{
    gapi_returnCode_t result;
    _WaitSet waitset;
    _ConditionEntry entry;
    int i;

    waitset = gapi_waitSetClaim(_this, &result);

    if (waitset == NULL) {
        return result;
    }

    if (attached_conditions == NULL) {
        _EntityRelease(waitset);
        return GAPI_RETCODE_BAD_PARAMETER;
    }

    gapi_sequence_replacebuf(attached_conditions,
                             (_bufferAllocatorType)gapi_conditionSeq_allocbuf,
                             waitset->length);

    i = 0;
    entry = waitset->conditions;
    while (entry != NULL) {
        attached_conditions->_buffer[i++] = _EntityHandle(entry->condition);
        entry = entry->next;
    }
    attached_conditions->_length = i;

    assert(i == waitset->length);

    _EntityRelease(waitset);

    return GAPI_RETCODE_OK;
}

static gapi_returnCode_t
getConditionDomainId(
    _Condition condition,
    gapi_domainName_t *id)
{
    _DomainParticipant participant;
    _Entity entity = _ConditionEntity(condition);
    gapi_returnCode_t result;

    if ( _ObjectGetKind(_Object(entity)) == OBJECT_KIND_DOMAINPARTICIPANT ) {
        participant = _DomainParticipant(entity);
    } else if (_ObjectGetKind(_Object(entity)) == OBJECT_KIND_UNDEFINED) {
        participant = NULL;
    } else {
        participant = _EntityParticipant(_Entity(entity));
    }
    if (participant) {
        *id = _DomainParticipantGetDomainId(participant);
        result = GAPI_RETCODE_OK;
    } else {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    }
    return result;
}


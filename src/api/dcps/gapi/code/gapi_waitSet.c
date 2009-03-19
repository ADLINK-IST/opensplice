#include "gapi_waitSet.h"
#include "gapi_waitSetDomainEntry.h"
#include "gapi_condition.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainEntity.h"
#include "gapi_map.h"

#include "u_waitset.h"
#include "v_event.h"
#include "v_handle.h"
#include "v_entity.h"
#include "os_heap.h"

#define _ConditionEntry(o) \
        ((_ConditionEntry)(o))

#define getDomainCount(waitset) \
        gapi_mapLength(((_WaitSet)waitset)->domains)

C_CLASS(_ConditionEntry);
C_STRUCT(_ConditionEntry) {
    _ConditionEntry     next;
    _Condition          condition;
    gapi_boolean        active;
    _WaitSetDomainEntry domain;
};

static gapi_domainId_t
getConditionDomainId(
    _Condition condition);

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

void
_WaitSetFree(
    void *_waitset)
{
    _ConditionEntry c_entry;
    _WaitSetDomainEntry wsde;
    gapi_mapIter iter;
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
        iter = gapi_mapFirst(waitset->domains);
        wsde = _WaitSetDomainEntry(gapi_mapIterObject(iter));
        while (wsde != NULL) {
            gapi_mapIterRemove(iter);
            if (wsde != NULL) {
                _WaitSetDomainEntryDelete(wsde);
            }
            wsde = _WaitSetDomainEntry(gapi_mapIterObject (iter));
        }
        gapi_mapIterFree(iter);

        gapi_mapFree(waitset->domains);
    }
    os_condDestroy(&waitset->cv);
    os_mutexDestroy(&waitset->mutex);
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
        newWaitSet->domains    = gapi_mapNew(domainCompare, FALSE, FALSE);
        if (!newWaitSet->domains) {
            gapi_free(newWaitSet);
            newWaitSet = NULL;
        }
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

void
_WaitSetNotify(
    _WaitSet _this,
    _Condition cond)
{
    gapi_mapIter iter;
    _WaitSetDomainEntry entry;

    if (_this->busy) {
        if (_this->multidomain) {
            os_condSignal(&_this->cv);
        } else {
            iter = gapi_mapFirst(_this->domains);
            entry = (_WaitSetDomainEntry)gapi_mapIterObject(iter);
            if (entry) {
                u_waitsetNotify(entry->uWaitset, (c_voidp)cond);
            }
            gapi_mapIterFree(iter);
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
    _Condition cd;

    if (_this) {
        if (v_eventTest(_this->kind,V_EVENT_TRIGGER) && (_this->userData)) {
            /*
             * _GuardConditionGetTriggerValue(cd) =>
             *                  cd->triggerValue;
             */
            cd = (_Condition)_this->userData;
            if (cd->getTriggerValue(cd)) {
                if (a->conditions->_maximum == a->conditions->_length) {
                    gapi_sequence_replacebuf(a->conditions,
                                             (_bufferAllocatorType)gapi_conditionSeq_allocbuf,
                                             a->waitset->length);
                }
                a->conditions->_buffer[a->conditions->_length++] = _EntityHandle(cd);
            }
        } else {
            r = v_handleClaim(_this->source,(v_object*)&e);
            if (r == V_HANDLE_OK) {
                ue = v_entityGetUserData(e);
                if (ue) {
                    cd = u_entityGetUserData(ue);
                    if (cd) {
                        /* _ReadConditionGetTriggerValue(cd) =>
                         *                  u_queryTriggerTest(cd->uQuery);
                         * _StatusConditionGetTriggerValue(cd) =>
                         *                  gapi_entity_get_status_changes(entity) &
                         *                  statuscondition->enabledStatusMask;
                         */
                        if (cd->getTriggerValue(cd)) {
                            if (a->conditions->_maximum == a->conditions->_length) {
                                gapi_sequence_replacebuf(a->conditions,
                                                         (_bufferAllocatorType)gapi_conditionSeq_allocbuf,
                                                         a->waitset->length);
                            }
                            a->conditions->_buffer[a->conditions->_length++] = _EntityHandle(cd);
                        }
                    }
                }
            }
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
    gapi_mapIter iter;
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

    do {
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
        if ((timeout->sec     == GAPI_DURATION_ZERO_SEC) &&
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
            } else {
                os_time osTimeOut;
                osTimeOut.tv_sec  = (os_timeSec)timeout->sec;
                osTimeOut.tv_nsec = (os_int32)timeout->nanosec;
                osTimeMax = os_timeAdd (os_timeGet(), osTimeOut);
            }

            if (waitset->multidomain) {
                /* Enter the multi domain wait. */
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
                        /* multi domain threads cannot handle event data yet!
                         * therefore reevaluate all conditions here until
                         * the problem is fixed.
                         */
                    break;
                    case os_resultTimeout:
                        result = GAPI_RETCODE_TIMEOUT;
                        ready = TRUE;
                    break;
                    default:
                        assert(FALSE);
                        ready = TRUE;
                    }
                } else {
                    result = GAPI_RETCODE_ALREADY_DELETED;
                    ready = TRUE;
                }
            } else {
                /* Enter the single domain wait. */
                iter = gapi_mapFirst(waitset->domains);
                wsentry = (_WaitSetDomainEntry)gapi_mapIterObject(iter);
                gapi_mapIterFree (iter);

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
                        case U_RESULT_DETACHING:
                            result = GAPI_RETCODE_ALREADY_DELETED;
                            ready = TRUE;
                        break;
                        default:
                            assert(FALSE);
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
_WaitSet_set_multi_mode(
    _WaitSet _this)
{
    gapi_mapIter iter;
    _WaitSetDomainEntry entry;
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
        iter = gapi_mapFirst(_this->domains);
        entry = (_WaitSetDomainEntry)gapi_mapIterObject(iter);
        while (entry) {
            _WaitSetDomainEntryMultiMode(entry, mode);
            gapi_mapIterNext(iter);
            entry = (_WaitSetDomainEntry)gapi_mapIterObject(iter);
        }
        gapi_mapIterFree(iter);
        _WaitSetNotify(_this, NULL);
        _this->multidomain = mode;
    }
}

gapi_returnCode_t
gapi_waitSet_attach_condition(
    gapi_waitSet _this,
    const gapi_condition cond)
{
    gapi_mapIter    iter;
    gapi_domainId_t DomainId;
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
                     * a domain. Therefore the waitset can be attache directly
                     * to the guard condition.
                     * The guard condition can immediatly trigger the waitset
                     * upon notification.
                     */
                    result = _ConditionAddWaitset(condition,
                                                  _EntityHandle(waitset),
                                                  NULL);
                } else {
                    DomainId = getConditionDomainId(condition);
                    iter = gapi_mapFind(waitset->domains, (gapi_object)DomainId);
                    wsdEntry = (_WaitSetDomainEntry)gapi_mapIterObject(iter);
                    if (wsdEntry == NULL) {
                        /* Create a new WaitSetDomainEntry */
                        wsdEntry = _WaitSetDomainEntryNew(waitset, DomainId);
                        if (wsdEntry) {
                            result = gapi_mapAdd(waitset->domains,
                                                 (gapi_object)DomainId,
                                                 (gapi_object)wsdEntry);
                        } else {
                            result = GAPI_RETCODE_OUT_OF_RESOURCES;
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
                    gapi_mapIterFree(iter);

                    _WaitSet_set_multi_mode(waitset);
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
    gapi_mapIter iter;

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
                        iter = gapi_mapFind(waitset->domains,
                                            (gapi_object)getConditionDomainId(condition));

                        wsdEntry = (_WaitSetDomainEntry)gapi_mapIterObject(iter);
                        (void)gapi_mapIterRemove(iter);
                        gapi_mapIterFree(iter);

                        assert(wsdEntry);
                        _WaitSetDomainEntryDelete(wsdEntry);
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

static gapi_domainId_t
getConditionDomainId(
    _Condition condition)
{
    _DomainParticipant participant;
    _Entity entity = _ConditionEntity(condition);

    if ( _ObjectGetKind(_Object(entity)) == OBJECT_KIND_DOMAINPARTICIPANT ) {
        participant = _DomainParticipant(entity);
    } else {
        participant = _DomainEntityParticipant(_DomainEntity(entity));
    }
    return _DomainParticipantGetDomainId(participant);
}


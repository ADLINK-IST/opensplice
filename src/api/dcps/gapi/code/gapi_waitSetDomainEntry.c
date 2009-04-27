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
#include "gapi_waitSetDomainEntry.h"
#include "gapi_waitSet.h"
#include "gapi_condition.h"
#include "gapi_domainParticipant.h"
#include "gapi_domainEntity.h"
#include "gapi_map.h"

#include "u_waitset.h"
#include "os_heap.h"
#include "os_report.h"

static void *
_WaitSetDomainEntryRun(
    _WaitSetDomainEntry _this) 
{
    c_iter list = NULL;
    os_time delay;
    
    delay.tv_sec = 0;
    delay.tv_nsec = 10000000; /* 10 millisec */

    while (_this->busy) {
        /* do nothing,
         * just wait until the active wait has finished before starting.
         */
        os_nanoSleep(delay);
    }
    while (_this->running) {
        u_waitsetWait(_this->uWaitset, &list);
        if (list) {
            if (c_iterLength(list) > 0) {
                _WaitSetNotify(_this->waitset, NULL);
            }
            c_iterFree(list);
            list = NULL;
        }

    }
    return NULL;
}

_WaitSetDomainEntry
_WaitSetDomainEntryNew(
    _WaitSet waitset,
    gapi_domainId_t domain_id) 
{
    _WaitSetDomainEntry _this = NULL;
    gapi_domainParticipant dp = NULL;

    _this = os_malloc (C_SIZEOF(_WaitSetDomainEntry));
    if (_this != NULL) {
        _this->waitset = waitset;
        _this->busy = FALSE;
        _this->multimode = FALSE;
        _this->running = FALSE;
        _this->condition_count = 0;
        dp = gapi_domainParticipantFactory_lookup_participant(
                     gapi_domainParticipantFactory_get_instance(),
                     domain_id);
        assert(dp);
        _this->uParticipant =
            u_participant(_EntityUEntity(gapi_objectPeek(dp, OBJECT_KIND_DOMAINPARTICIPANT)));

        _this->uWaitset = u_waitsetNew(_this->uParticipant);
        if (_this->uWaitset == NULL) {
            os_free(_this);
            _this = NULL;
        }
    }
    return _this;

}
void
_WaitSetDomainEntryDelete(
    _WaitSetDomainEntry _this) 
{
    u_result uresult;
    os_time delay;
    
    delay.tv_sec = 0;
    delay.tv_nsec = 10000000; /* 10 millisec */
    if (_this->uWaitset) {
        if (_this->running) {
            _this->running = FALSE;
            uresult = u_waitsetNotify(_this->uWaitset, NULL);
            assert (uresult == U_RESULT_OK);
            os_threadWaitExit(_this->thread, NULL);
            _this->thread = 0;
        } else {
            uresult = u_waitsetNotify(_this->uWaitset, NULL);
            assert(uresult == U_RESULT_OK);
            while (_this->busy) {
                /* do nothing,
                 * just wait until the blocked thread leaves the
                 * u_waitset_wait.
                 */
                os_nanoSleep(delay);
            }
        }
        u_waitsetFree(_this->uWaitset);
        _this->uWaitset = NULL;
    }
    
    os_free(_this);
}

gapi_returnCode_t
_WaitSetDomainEntryMultiMode(
    _WaitSetDomainEntry _this,
    c_bool multimode) 
{
    gapi_returnCode_t      result = GAPI_RETCODE_OK;
    os_result              osResult;
    os_threadAttr          osThreadAttr;

    if (multimode != _this->multimode) {

        if (multimode) {
            if (_this->busy) {
                u_waitsetNotify(_this->uWaitset, NULL);
            }
             _this->running = TRUE;
    
            osResult = os_threadAttrInit(&osThreadAttr);
            if (osResult == os_resultSuccess) {
                osResult = os_threadCreate(
                                &_this->thread,
                                "WaitSetDomainThread",
                                &osThreadAttr,
                                (void *(*)(void *))_WaitSetDomainEntryRun,
                                (void *)_this);
            }
            if (osResult == os_resultSuccess) {
                _this->multimode = TRUE;
            } else {
                _this->running = FALSE;
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            _this->running = FALSE;
            u_waitsetNotify(_this->uWaitset, NULL);
            os_threadWaitExit(_this->thread, NULL);
            _this->thread = (os_threadId)0;
        }
    }
    return result;
}

gapi_returnCode_t
_WaitSetDomainEntryAttachCondition(
    _WaitSetDomainEntry _this,
    _Condition condition) 
{
    gapi_returnCode_t result;
    
    result =  _ConditionAddWaitset(condition,
                                   _EntityHandle(_this->waitset),
                                   _this->uWaitset);

    if (result == GAPI_RETCODE_OK) {
        _this->condition_count++;
    }

    return result;
    
}

gapi_returnCode_t
_WaitSetDomainEntryDetachCondition(
    _WaitSetDomainEntry _this,
    _Condition condition) 
{
    gapi_returnCode_t result;
    
    result =  _ConditionRemoveWaitset(condition,
                                      _EntityHandle(_this->waitset),
                                      _this->uWaitset);
    
    if (result == GAPI_RETCODE_OK) {
        _this->condition_count--;
    }

    return result;
    
}


/* returns either os_resultFail or os_resultSuccess */
os_result
_WaitSetDomainEntryWait(
    _WaitSetDomainEntry _this) 
{
    c_iter    list = NULL;
    os_result result = os_resultFail;

    _this->busy = TRUE;
    u_waitsetWait(_this->uWaitset,&list);
    if (list) {
        if (c_iterLength(list) > 0) {
            result = os_resultSuccess;
        }
        c_iterFree(list);
        list = NULL;
    }
    _this->busy = FALSE;
    return result;
}

/* returns either os_resultTimeout or os_resultSuccess */
os_result
_WaitSetDomainEntryTimedWait(
    _WaitSetDomainEntry _this,
    const os_time t) 
{
    c_iter     list   = NULL;
    os_result  result = os_resultTimeout;
    c_time     ctime;
    
    ctime.seconds     = t.tv_sec;
    ctime.nanoseconds = t.tv_nsec;
    
    _this->busy = TRUE;
    u_waitsetTimedWait(_this->uWaitset,ctime,&list);
    if (list) {
        if (c_iterLength(list) > 0) {
            result = os_resultSuccess;
        }
        c_iterFree(list);
        list = NULL;
    }
    _this->busy = FALSE;
    return result;
}

gapi_long
_WaitSetDomainEntryConditionCount(
    _WaitSetDomainEntry _this)
{
    return _this->condition_count;
}


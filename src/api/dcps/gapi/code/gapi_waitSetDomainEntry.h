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
#ifndef GAPI_WAITSETDOMAINENTRY_H
#define GAPI_WAITSETDOMAINENTRY_H

#include "gapi_common.h"
#include "gapi_condition.h"

#define _WaitSetDomainEntry(o) ((_WaitSetDomainEntry)(o))

C_STRUCT(_WaitSetDomainEntry) {
    gapi_domainName_t domainId;
    gapi_boolean  busy;
    gapi_boolean  running;
    gapi_boolean  multimode;
    gapi_long     condition_count;
    _WaitSet      waitset;
    u_waitset     uWaitset;
    u_participant uParticipant;
    os_threadId   thread;
};

_WaitSetDomainEntry
_WaitSetDomainEntryNew(
    _WaitSet waitset,
    gapi_domainName_t domain_id);

void
_WaitSetDomainEntryDelete(
    _WaitSetDomainEntry _this);

gapi_returnCode_t
_WaitSetDomainEntryMultiMode(
    _WaitSetDomainEntry _this,
    c_bool multimode);

gapi_returnCode_t
_WaitSetDomainEntryAttachCondition(
    _WaitSetDomainEntry _this,
    _Condition condition);

gapi_returnCode_t
_WaitSetDomainEntryDetachCondition(
    _WaitSetDomainEntry _this,
    _Condition condition);


os_result
_WaitSetDomainEntryWait(
    _WaitSetDomainEntry _this);

os_result
_WaitSetDomainEntryTimedWait(
    _WaitSetDomainEntry _this,
    const os_time t);

gapi_long
_WaitSetDomainEntryConditionCount(
    _WaitSetDomainEntry _this);

#endif

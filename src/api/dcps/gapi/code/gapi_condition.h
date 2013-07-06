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
#ifndef GAPI_CONDITION_H
#define GAPI_CONDITION_H

#include "gapi_common.h"
#include "gapi_object.h"
#include "gapi_waitSet.h"
#include "gapi_expression.h"

#include "u_user.h"
#include "os_mutex.h"

#define _Condition(o) ((_Condition)(o))
#define _GuardCondition(o) ((_GuardCondition)(o))
#define _StatusCondition(o) ((_StatusCondition)(o))
#define _ReadCondition(o) ((_ReadCondition)(o))
#define _QueryCondition(o) ((_QueryCondition)(o))

#define U_QUERY_GET(t) u_query(_Condition(t)->uEntity)

#define gapi_conditionClaim(h,r) \
        (_Condition(gapi_objectClaim(h,OBJECT_KIND_CONDITION,r)))

#define gapi_conditionClaimNB(h,r) \
        (_Condition(gapi_objectClaimNB(h,OBJECT_KIND_CONDITION,r)))

#define _ConditionFromHandle(h) \
        (_Condition(gapi_objectPeek(h,OBJECT_KIND_CONDITION)))

#define gapi_quardConditionClaim(h,r) \
        (_GuardCondition(gapi_objectClaim(h,OBJECT_KIND_GUARDCONDITION,r)))

#define gapi_quardConditionClaimNB(h,r) \
        (_GuardCondition(gapi_objectClaimNB(h,OBJECT_KIND_GUARDCONDITION,r)))

#define gapi_statusConditionClaim(h,r) \
        (_StatusCondition(gapi_objectClaim(h,OBJECT_KIND_STATUSCONDITION,r)))

#define gapi_statusConditionClaimNB(h,r) \
        (_StatusCondition(gapi_objectClaimNB(h,OBJECT_KIND_STATUSCONDITION,r)))

#define gapi_readConditionClaim(h,r) \
        (_ReadCondition(gapi_objectClaim(h,OBJECT_KIND_READCONDITION,r)))

#define gapi_readConditionClaimNB(h,r) \
        (_ReadCondition(gapi_objectClaimNB(h,OBJECT_KIND_READCONDITION,r)))

#define _ReadConditionFromHandle(h) \
        (_ReadCondition(gapi_objectPeek(h,OBJECT_KIND_READCONDITION)))

#define gapi_queryConditionClaim(h,r) \
        (_QueryCondition(gapi_objectClaim(h,OBJECT_KIND_QUERYCONDITION,r)))

#define gapi_queryConditionClaimNB(h,r) \
        (_QueryCondition(gapi_objectClaimNB(h,OBJECT_KIND_QUERYCONDITION,r)))


#define _GuardConditionAlloc() \
        (_GuardCondition(_ObjectAlloc(OBJECT_KIND_GUARDCONDITION, \
                                      C_SIZEOF(_GuardCondition), \
                                      _GuardConditionFree)))

#define _StatusConditionAlloc() \
        (_StatusCondition(_ObjectAlloc(OBJECT_KIND_STATUSCONDITION, \
                                       C_SIZEOF(_StatusCondition), \
                                       NULL)))

#define _ReadConditionAlloc() \
        (_ReadCondition(_ObjectAlloc(OBJECT_KIND_READCONDITION, \
                                     C_SIZEOF(_ReadCondition), \
                                     NULL)))

#define _QueryConditionAlloc() \
        (_QueryCondition(_ObjectAlloc(OBJECT_KIND_QUERYCONDITION, \
                                      C_SIZEOF(_QueryCondition), \
                                      NULL)))

#define _ConditionKind(_this) \
        _ObjectGetKind(_Object(_this))

typedef gapi_boolean (*GetTriggerValue)(_Condition condition);

C_STRUCT(_Condition) {
    C_EXTENDS(_Object);
    _Entity entity;
    u_entity uEntity;
    c_iter waitsets;
    GetTriggerValue getTriggerValue;
};

C_STRUCT(_StatusCondition) {
    C_EXTENDS(_Condition);
    gapi_statusMask enabledStatusMask;
};

C_STRUCT(_ReadCondition) {
    C_EXTENDS(_Condition);
    gapi_readerMask readerMask;
    u_query         uQuery;
    _DataReader     dataReader;
    _DataReaderView dataReaderView;
};

C_STRUCT(_QueryCondition) {
    C_EXTENDS(_ReadCondition);
    gapi_char *query_expression;
    gapi_stringSeq *query_parameters;
    gapi_expression expression;
};

C_STRUCT(_GuardCondition) {
    C_EXTENDS(_Condition);
    gapi_boolean triggerValue;
};

void
_ConditionInit (
    _Condition condition,
    _Entity entity,
    GetTriggerValue getTriggerValue);

void
_ConditionDispose (
    _Condition condition);

_Entity
_ConditionEntity (
    _Condition condition);

u_entity
_ConditionUentity (
    _Condition condition);

gapi_returnCode_t
_ConditionAddWaitset (
    _Condition   condition,
    gapi_waitSet waitset,
    u_waitset    uWaitset);

gapi_returnCode_t
_ConditionRemoveWaitset (
    _Condition   condition,
    gapi_waitSet waitset,
    u_waitset    uWaitset);

_GuardCondition
_GuardConditionNew (
    void);

void
_ReadConditionDispose (
    _ReadCondition readCondition);

_ReadCondition
_ReadConditionNew (
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    _DataReader datareader,
    _DataReaderView datareaderview);

gapi_returnCode_t
_ReadConditionFree (
    _ReadCondition readcondition);

gapi_boolean
_ReadConditionPrepareDelete (
    _ReadCondition readCondition);

_QueryCondition
_QueryConditionNew (
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    const gapi_char *query_expression,
    const gapi_stringSeq *query_parameters,
    _DataReader datareader,
    _DataReaderView datareaderview);

gapi_returnCode_t
_QueryConditionFree (
    _QueryCondition querycondition);

gapi_boolean
_QueryConditionPrepareDelete (
    _QueryCondition queryCondition);

_StatusCondition
_StatusConditionNew (
    _Entity  entity,
    u_entity userEntity);

gapi_returnCode_t
_StatusConditionFree (
    _StatusCondition statuscondition);

void
_ConditionFree(
    _Condition _this);

#endif

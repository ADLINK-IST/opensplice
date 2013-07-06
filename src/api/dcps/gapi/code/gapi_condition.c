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
#include "u_participant.h"
#include "u_reader.h"
#include "u_dataView.h"
#include "u_waitset.h"

#include "gapi_condition.h"
#include "gapi_fooDataReader.h"
#include "gapi_dataReaderView.h"
#include "gapi_subscriber.h"
#include "gapi_structured.h"

#include "os_heap.h"

#define GAPI_STATUS_KIND_FULL 0x1fffU
#define GAPI_STATUS_KIND_NULL 0x0U

static gapi_boolean
_GuardConditionFree(
    void * _guardcondition);

void
_ConditionInit(
    _Condition      _this,
    _Entity         entity,
    GetTriggerValue getTriggerValue)
{
    _this->entity = entity;
    _this->waitsets = NULL;
    _this->uEntity = NULL;
    _this->getTriggerValue = getTriggerValue;
}

void
_ConditionDispose(
    _Condition _this)
{
    gapi_waitSet waitset;

    waitset = c_iterTakeFirst(_this->waitsets);
    while (waitset) {
        gapi_condition handle = _EntityRelease(_this);
        gapi_waitSet_detach_condition(waitset, handle);
        _this = gapi_conditionClaim(handle, NULL);
        waitset = c_iterTakeFirst(_this->waitsets);
    }
    c_iterFree(_this->waitsets);
    if (_ObjectGetKind(_Object(_this)) != OBJECT_KIND_GUARDCONDITION) {
        _EntityDelete(_this);
    }
}

void
_ConditionFree(
    _Condition _this)
{
    switch (_ObjectGetKind(_Object(_this))) {
    case OBJECT_KIND_QUERYCONDITION:
        _QueryConditionFree((_QueryCondition)_this);
    break;
    case OBJECT_KIND_READCONDITION:
        _ReadConditionFree((_ReadCondition)_this);
    break;
    case OBJECT_KIND_STATUSCONDITION:
        _StatusConditionFree((_StatusCondition)_this);
    break;
    case OBJECT_KIND_GUARDCONDITION:
        _GuardConditionFree(_this);
    break;
    default:
        assert(0);
    break;
    }
}

_Entity
_ConditionEntity(
    _Condition _this)
{
    return _this->entity;
}

u_entity
_ConditionUentity(
    _Condition _this)
{
    return _this->uEntity;
}

gapi_returnCode_t
_ConditionAddWaitset(
    _Condition   _this,
    gapi_waitSet waitset,
    u_waitset    uWaitset)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;

    _this->waitsets = c_iterInsert(_this->waitsets,waitset);
    if ((uWaitset != NULL) && (_this->uEntity != NULL)) {
        uResult = u_waitsetAttach(uWaitset,
                                  _this->uEntity,
                                  (c_voidp)_this);
        if (uResult != U_RESULT_OK) {
            result = GAPI_RETCODE_ERROR;
        }
    }
    return result;
}

gapi_returnCode_t
_ConditionRemoveWaitset(
    _Condition   _this,
    gapi_waitSet waitset,
    u_waitset    uWaitset)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;

    c_iterTake(_this->waitsets, waitset);
    if ((_this->uEntity != NULL) && (uWaitset != NULL)) {
        uResult = u_waitsetDetach(uWaitset, _this->uEntity);
        if (uResult != U_RESULT_OK) {
            result = GAPI_RETCODE_ERROR;
        }
    }
    return result;
}

/*
 *     boolean
 *     get_trigger_value();
 */
gapi_boolean
gapi_condition_get_trigger_value(
    gapi_condition _this)
{
    _Condition condition;
    gapi_boolean result = FALSE;

    condition = gapi_conditionClaim(_this, NULL);

    if (condition != NULL) {
        result = condition->getTriggerValue(condition);
    } else {
        result = FALSE;
    }
    _EntityRelease(condition);

    return result;
}

static gapi_boolean
_GuardConditionGetTriggerValue(
    _Condition _this)
{
    _GuardCondition guardcondition = _GuardCondition(_this);

    return guardcondition->triggerValue;
}

/*
 * deallocator, called by gapi_free
 */
static gapi_boolean
_GuardConditionFree(
    void *_guardcondition)
{
    _GuardCondition guardcondition = (_GuardCondition) _guardcondition;

    _ConditionDispose(_Condition(guardcondition));
    return TRUE;
}

gapi_guardCondition
gapi_guardCondition__alloc(void)
{
    _GuardCondition newGuardCondition;

    newGuardCondition = _GuardConditionAlloc();

    if (newGuardCondition != NULL) {
        _ConditionInit(_Condition(newGuardCondition), NULL,
                       _GuardConditionGetTriggerValue);
        newGuardCondition->triggerValue = FALSE;
    }

    return (gapi_guardCondition)_EntityRelease(newGuardCondition);
}


/*     void
 *     set_trigger_value(
 *         in boolean value);
 *
 */

gapi_returnCode_t
gapi_guardCondition_set_trigger_value(
    gapi_guardCondition _this,
    const gapi_boolean value)
{
    _GuardCondition guardcondition = (_GuardCondition)_this;
    _WaitSet waitset;
    gapi_waitSet wsh;
    gapi_returnCode_t result;
    c_iter wsHandles;

    wsHandles = NULL;
    guardcondition = gapi_quardConditionClaim(_this, NULL);

    if (guardcondition != NULL) {
        if (value) {
            guardcondition->triggerValue = TRUE;
            wsHandles = c_iterCopy(_Condition(guardcondition)->waitsets);
            _EntityRelease(guardcondition);
            wsh = (gapi_waitSet)c_iterTakeFirst(wsHandles);
            while (wsh != NULL) {
                waitset = gapi_waitSetClaim(wsh, &result);
                if (waitset) {
                    _WaitSetNotify(waitset, (_Condition)guardcondition);
                    _EntityRelease(waitset);
                }
                wsh = (gapi_waitSet)c_iterTakeFirst(wsHandles);
            }
            c_iterFree(wsHandles);
        } else {
            guardcondition->triggerValue = FALSE;
            _EntityRelease(guardcondition);
        }
    }
    return GAPI_RETCODE_OK;
}

static gapi_boolean
_ReadConditionGetTriggerValue(
    _Condition condition)
{
    _ReadCondition readcondition = _ReadCondition(condition);

    return u_queryTest(readcondition->uQuery,
                       gapi_matchesReaderMask,
                       &readcondition->readerMask);
}

void
_ReadConditionDispose(
    _ReadCondition readcondition)
{
    u_query query;

    query = readcondition->uQuery;

    _Condition(readcondition)->uEntity = u_entity(0);

    _ConditionDispose(_Condition(readcondition));

    u_queryFree(query);
}

_ReadCondition
_ReadConditionNew(
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    _DataReader datareader,
    _DataReaderView datareaderview)
{
    _ReadCondition _this;
    u_reader      uReader;
    u_dataView    uDataView;
    q_expr        predicate;


    /* The datareader and the datareaderview (should) share
     * a baseclass and therefore it is not necessary to have
     * both a datareader and a datareaderview as properties.
     * At run-time the right owner can be determined
     */
    _this = _ReadConditionAlloc();
    if ( _this != NULL ) {
        if (datareaderview != NULL) {
          _ConditionInit(_Condition(_this),
                         _Entity(datareaderview),
                         _ReadConditionGetTriggerValue);
        } else {
          _ConditionInit(_Condition(_this),
                         _Entity(datareader),
                         _ReadConditionGetTriggerValue);
        }
        _this->dataReader = datareader;
        _this->dataReaderView = datareaderview;
        _this->readerMask.sampleStateMask = sample_states;
        _this->readerMask.viewStateMask = view_states;
        _this->readerMask.instanceStateMask = instance_states;

        predicate = q_parse("1=1");
        if (datareaderview) {
            uDataView = u_dataView(_DataReaderViewUreaderView(datareaderview));
            if ( predicate != NULL ) {
                _this->uQuery = u_queryNew(u_reader(uDataView), NULL, predicate, NULL);
            }
        } else {
            uReader = u_reader(_DataReaderUreader(datareader));
            if ( predicate != NULL ) {
                _this->uQuery = u_queryNew(uReader, NULL, predicate, NULL);
            }
        }
        q_dispose(predicate);
        if (_this->uQuery != NULL) {
            u_entitySetUserData(u_entity(_this->uQuery),_this);
            _Condition(_this)->uEntity = u_entity(_this->uQuery);
        } else {
            _ConditionDispose(_Condition(_this));
            _this = NULL;
        }
    }

    return _this;
}

gapi_returnCode_t
_ReadConditionFree(
    _ReadCondition readcondition)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    if (_ObjectGetKind(_Object(readcondition)) == OBJECT_KIND_QUERYCONDITION ) {
        _QueryConditionFree((_QueryCondition)readcondition);
    } else {
        _ReadConditionDispose(readcondition);
    }

    return result;
}

gapi_boolean
_ReadConditionPrepareDelete(
    _ReadCondition readcondition)
{
    return TRUE;
}


/*     SampleStateMask
 *     get_sample_state_mask();
 */
gapi_sampleStateMask
gapi_readCondition_get_sample_state_mask(
    gapi_readCondition _this)
{
    _ReadCondition readcondition;
    gapi_sampleStateMask sample_state = 0U;

    readcondition = gapi_readConditionClaim(_this, NULL);
    if (readcondition != NULL) {
        sample_state = readcondition->readerMask.sampleStateMask;
        if (sample_state == 0U) {
            sample_state = GAPI_ANY_SAMPLE_STATE;
        }
    }
    _EntityRelease(readcondition);

    return sample_state;
}

/*     ViewStateMask
 *     get_view_state_mask();
 */
gapi_viewStateMask
gapi_readCondition_get_view_state_mask(
    gapi_readCondition _this)
{
    _ReadCondition readcondition;
    gapi_viewStateMask view_state = 0U;

    readcondition = gapi_readConditionClaim(_this, NULL);
    if (readcondition != NULL) {
        view_state = readcondition->readerMask.viewStateMask;
        if (view_state == 0U) {
            view_state = GAPI_ANY_VIEW_STATE;
        }
    }
    _EntityRelease(readcondition);

    return view_state;
}

/*     InstanceStateMask
 *     get_instance_state_mask();
 */
gapi_instanceStateMask
gapi_readCondition_get_instance_state_mask(
    gapi_readCondition _this)
{
    _ReadCondition readcondition;
    gapi_instanceStateMask instance_state = 0U;

    readcondition = gapi_readConditionClaim(_this, NULL);
    if (readcondition != NULL) {
        instance_state = readcondition->readerMask.instanceStateMask;
        if (instance_state == 0U) {
            instance_state = GAPI_ANY_INSTANCE_STATE;
        }
    }
    _EntityRelease(readcondition);

    return instance_state;
}

/*     DataReader
 *     get_datareader();
 */
gapi_dataReader
gapi_readCondition_get_datareader(
    gapi_readCondition _this)
{
    _ReadCondition readcondition;
    _DataReader dataReader = (_DataReader)0;

    readcondition = gapi_readConditionClaim(_this, NULL);
    if (readcondition != NULL) {
        dataReader = readcondition->dataReader;
    }
    _EntityRelease(readcondition);

    return (gapi_dataReader)_EntityHandle(dataReader);
}

/*     DataReaderView
 *     get_datareaderview();
 */
gapi_dataReader
gapi_readCondition_get_datareaderview(
    gapi_readCondition _this)
{
    _ReadCondition readcondition;
    _DataReaderView dataReaderView = (_DataReaderView)0;

    readcondition = gapi_readConditionClaim(_this, NULL);
    if (readcondition != NULL) {
        dataReaderView = readcondition->dataReaderView;
    }
    _EntityRelease(readcondition);

    return (gapi_dataReaderView)_EntityHandle(dataReaderView);
}

_QueryCondition
_QueryConditionNew(
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    const gapi_char *query_expression,
    const gapi_stringSeq *query_parameters,
    _DataReader datareader,
    _DataReaderView datareaderview)
{
    _QueryCondition _this;
    u_reader      uReader;
    u_dataView    uDataView;


    /* Allocate QueryCondition */
    _this = _QueryConditionAlloc();
    if (_this != NULL) {
        /* Initialise QueryCondition */
        if (datareaderview != NULL) {
          _ConditionInit(_Condition(_this),
                         _Entity(datareaderview),
                         _ReadConditionGetTriggerValue);
        } else {
          _ConditionInit(_Condition(_this),
                         _Entity(datareader),
                         _ReadConditionGetTriggerValue);
        }
        _this->_parent.dataReader = datareader;
        _this->_parent.dataReaderView = datareaderview;
        _this->_parent.readerMask.sampleStateMask = sample_states;
        _this->_parent.readerMask.viewStateMask = view_states;
        _this->_parent.readerMask.instanceStateMask = instance_states;
        _this->query_expression = gapi_string_dup(query_expression);
        _this->query_parameters = gapi_stringSeq_dup(query_parameters);

        uReader = u_reader(_DataReaderUreader(datareader));

        _this->expression =
            gapi_createQueryExpression(u_entity(uReader),
                                       _this->query_expression);
        if (_this->expression) {
            if (datareaderview) {
                uDataView = u_dataView(_DataReaderViewUreaderView(datareaderview));
                _this->_parent.uQuery = gapi_expressionCreateQuery(
                                                _this->expression,
                                                 u_reader(uDataView),
                                                 NULL,
                                                 _this->query_parameters);
            } else {
                uReader = u_reader(_DataReaderUreader(datareader));
                _this->_parent.uQuery = gapi_expressionCreateQuery(
                                                _this->expression,
                                                 uReader,
                                                 NULL,
                                                 _this->query_parameters);
            }


            if (_this->_parent.uQuery) {
                /* Success: fill UserData and mark as valid */
                u_entitySetUserData(u_entity(_this->_parent.uQuery),_this);
                _Condition(_this)->uEntity = u_entity(_this->_parent.uQuery);
            } else {
                gapi_free(_this->query_expression);
                gapi_free(_this->query_parameters);
                gapi_expressionFree(_this->expression);
                _ConditionDispose(_Condition(_this));
                _this = NULL;
            }
        } else {
            gapi_free(_this->query_expression);
            gapi_free(_this->query_parameters);
            _ConditionDispose(_Condition(_this));
            _this = NULL;
        }
    }

    return _this;
}

gapi_returnCode_t
_QueryConditionFree(
    _QueryCondition querycondition)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    gapi_free(querycondition->query_expression);
    gapi_free(querycondition->query_parameters);

    if (querycondition->expression) {
        gapi_expressionFree(querycondition->expression);
    }

    _ReadConditionDispose(_ReadCondition(querycondition));

    return result;
}

gapi_boolean
_QueryConditionPrepareDelete(
    _QueryCondition querycondition)
{
    return TRUE;
}

/*     string
 *     get_query_expression();
 */
gapi_string
gapi_queryCondition_get_query_expression(
    gapi_queryCondition _this)
{
    _QueryCondition querycondition;
    gapi_string expression = NULL;

    querycondition = gapi_queryConditionClaim(_this, NULL);
    if (querycondition != NULL) {
        expression = gapi_string_dup(querycondition->query_expression);
    }
    _EntityRelease(querycondition);

    return expression;
}

/*     ReturnCode_t
 *     get_query_parameters(inout StringSeq query_parameters);
 */
gapi_returnCode_t
gapi_queryCondition_get_query_parameters(
    gapi_queryCondition _this,
    gapi_stringSeq * query_parameters)
{
    _QueryCondition  querycondition;
    gapi_returnCode_t   result;

    querycondition = gapi_queryConditionClaim(_this, &result);
    if (querycondition != NULL) {
        gapi_stringSeqCopyout(querycondition->query_parameters,
                              query_parameters);
        _EntityRelease(querycondition);
    }

    return result;
}

/*     ReturnCode_t
 *     set_query_parameters(
 *         in StringSeq query_parameters);
 */
gapi_returnCode_t
gapi_queryCondition_set_query_parameters(
    gapi_queryCondition _this,
    const gapi_stringSeq *query_parameters)
{
    gapi_returnCode_t   result         = GAPI_RETCODE_OK;
    _QueryCondition     querycondition = (_QueryCondition)_this;

    querycondition = gapi_queryConditionClaim(_this, &result);

    if (querycondition != NULL && gapi_sequence_is_valid(query_parameters)) {
         u_query uQuery = ((_ReadCondition)querycondition)->uQuery;

         result = gapi_expressionSetQueryArgs(querycondition->expression,
                                              uQuery,
                                              query_parameters);

        if (result == GAPI_RETCODE_OK) {
            /* Store new params for later retrieval */
            gapi_free(querycondition->query_parameters);
            querycondition->query_parameters = gapi_stringSeq_dup(query_parameters);
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    _EntityRelease(querycondition);

    return result;
}

static gapi_boolean
_StatusConditionGetTriggerValue(
    _Condition _this)
{
    _StatusCondition statuscondition = _StatusCondition(_this);
    gapi_statusMask currentStatus;
    gapi_boolean result = FALSE;

    /* We are accessing the entity 'status' attribute here, but we do not lock
    * the entity. This is safe as we do have the condition locked at this point.
    * and the entity can not be gone as long as the condition is locked.
    * the 'status' attribute of the entity is just a statusMask and even if
    * it were to already be reset it would be safe to access.
    */
    if(_this->entity)
    {
        currentStatus = _StatusGetCurrentStatus(_this->entity->status) &
                 statuscondition->enabledStatusMask;
    } else
    {
        currentStatus = GAPI_STATUS_KIND_NULL;
    }
    if (currentStatus) {
        result = TRUE;
    }

    return result;
}

_StatusCondition
_StatusConditionNew(
    _Entity entity,
    u_entity userEntity)
{
    _StatusCondition _this;

    _this = _StatusConditionAlloc();
    if (_this != NULL) {
        _ConditionInit(_Condition(_this), entity,
                       _StatusConditionGetTriggerValue);
        _this->enabledStatusMask = GAPI_STATUS_KIND_FULL;
        /* The _Condition has no uEntity counter part,
         * The uEntity is set to the user part of the entity
         * that is the holder of this status condition.
         */
        _Condition(_this)->uEntity = userEntity;
    }

    return _this;
}

gapi_returnCode_t
_StatusConditionFree(
    _StatusCondition statuscondition)
{
    _ConditionDispose(_Condition(statuscondition));

    return GAPI_RETCODE_OK;
}

/*     StatusMask
 *     get_enabled_statuses();
 */
gapi_statusMask
gapi_statusCondition_get_enabled_statuses(
    gapi_statusCondition _this)
{
    _StatusCondition statuscondition;
    gapi_statusMask result = GAPI_STATUS_KIND_NULL;

    statuscondition = gapi_statusConditionClaim(_this, NULL);
    if (statuscondition != NULL) {
        result = statuscondition->enabledStatusMask;
    }
    _EntityRelease(statuscondition);

    return result;
}

/*     ReturnCode_t
 *     set_enabled_statuses(
 *         in StatusMask mask);
 */
gapi_returnCode_t
gapi_statusCondition_set_enabled_statuses(
    gapi_statusCondition _this,
    const gapi_statusMask mask)
{
    _StatusCondition statuscondition;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    statuscondition = gapi_statusConditionClaim(_this, &result);
    if (statuscondition != NULL) {
        statuscondition->enabledStatusMask = mask;
    }
    _EntityRelease(statuscondition);

    return result;
}

/*     Entity
 *     get_entity();
 */
gapi_entity
gapi_statusCondition_get_entity(
    gapi_statusCondition _this)
{
    _StatusCondition statuscondition;
    _Entity entity = NULL;

    statuscondition = gapi_statusConditionClaim(_this, NULL);
    if ( statuscondition != NULL ) {
        entity = _ConditionEntity(_Condition(statuscondition));
        _EntityClaim(entity);
    }
    _EntityRelease(statuscondition);

    return (gapi_entity)_EntityRelease(entity);
}


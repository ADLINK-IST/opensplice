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

#include "gapi.h"
#include "gapi_dataReaderView.h"
#include "gapi_dataReader.h"
#include "gapi_qos.h"
#include "gapi_typeSupport.h"
#include "gapi_subscriber.h"
#include "gapi_topicDescription.h"
#include "gapi_topic.h"
#include "gapi_condition.h"
#include "gapi_domainParticipantFactory.h"
#include "gapi_domainParticipant.h"
#include "gapi_structured.h"
#include "gapi_objManag.h"
#include "gapi_kernel.h"
#include "gapi_error.h"

#include "os_heap.h"
#include "u_user.h"
#include "u_instanceHandle.h"
#include "u_dataView.h"
#include "u_dataViewQos.h"
#include "v_kernel.h"
#include "v_state.h"
#include "kernelModule.h"


static gapi_boolean
copyReaderViewQosIn (
    const gapi_dataReaderViewQos *srcQos,
    v_dataViewQos dstQos)
{
    gapi_boolean copied = TRUE;

    dstQos->userKey.enable = srcQos->view_keys.use_key_list;
    dstQos->userKey.expression = gapi_stringSeq_to_String(&srcQos->view_keys.key_list, ",");
    if ( (srcQos->view_keys.key_list._length > 0UL) && !dstQos->userKey.expression ) {
        assert(FALSE);
        copied = FALSE;
    }

    return copied;
}

static gapi_boolean
copyReaderViewQosOut (
    const v_dataViewQos srcQos,
    gapi_dataReaderViewQos *dstQos)
{
    gapi_boolean copied = TRUE;

    dstQos->view_keys.use_key_list = srcQos->userKey.enable;
    copied = gapi_string_to_StringSeq(srcQos->userKey.expression,
                                      ",",
                                      &dstQos->view_keys.key_list);
    return copied;
}

_DataReaderView
_DataReaderViewNew (
    const gapi_dataReaderViewQos * qos,
    const _DataReader datareader)
{
    _DataReaderView _this;
    v_dataViewQos ViewQos;
    u_dataView uReaderView;
    _TypeSupport typeSupport;

    _this = _DataReaderViewAlloc();

    if ( _this != NULL ) {
        _EntityInit(_Entity(_this),
                          _Entity(datareader));

        typeSupport = _TopicDescriptionGetTypeSupport(datareader->topicDescription);

        assert(typeSupport);
        _this->datareader = datareader;
        ViewQos = u_dataViewQosNew(NULL);
        if ( ViewQos != NULL ) {
            if ( !copyReaderViewQosIn(qos, ViewQos) ) {
                u_dataViewQosFree(ViewQos);
                _EntityDispose(_Entity(_this));
                _this = NULL;
            }
        } else {
            _EntityDispose(_Entity(_this));
            _this = NULL;
        }
    }

    if ( _this != NULL ) {
        uReaderView = u_dataViewNew(u_dataReader(_EntityUEntity (datareader)),
                                    "dataReaderView",
                                    ViewQos);
        if ( uReaderView ) {
            U_DATAREADERVIEW_SET(_this, uReaderView);
        } else {
            _EntityDispose(_Entity(_this));
            _this = NULL;
        }
        u_dataViewQosFree(ViewQos);
    }

    if ( _this != NULL ) {
        _EntityStatus(_this) = _Entity(datareader)->status;
    }

    return _this;

}

gapi_returnCode_t
_DataReaderViewFree (
    _DataReaderView dataReaderView)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_dataView v;
    assert(dataReaderView);

    gapi_loanRegistry_free(dataReaderView->loanRegistry);

    v = U_DATAREADERVIEW_GET(dataReaderView);
    _EntityDispose (_Entity(dataReaderView));
    u_dataViewFree(v);

    return result;
}

gapi_boolean
_DataReaderViewPrepareDelete (
    _DataReaderView dataReaderView,
    gapi_context *context)
{
    gapi_boolean result = TRUE;

    assert(dataReaderView);

    /* Note: one internal query always exists! */
    if ( u_readerQueryCount(U_READER_GET(dataReaderView)) > 1 ) {
        gapi_errorReport(context, GAPI_ERRORCODE_CONTAINS_CONDITIONS);
        result = FALSE;
    }

    if ( !gapi_loanRegistry_is_empty(dataReaderView->loanRegistry) ) {
        gapi_errorReport(context, GAPI_ERRORCODE_CONTAINS_LOANS);
        result = FALSE;
    }
    return result;
}

u_dataView
_DataReaderViewUreaderView (
    _DataReaderView dataReaderView)
{
    return U_DATAREADERVIEW_GET(dataReaderView);
}

_DataReader
_DataReaderViewDataReader (
    _DataReaderView dataReaderView)
{
    _DataReader datareader;

    datareader = dataReaderView->datareader;
    _EntityClaim(datareader);
    return datareader;
}

gapi_dataReaderViewQos *
_DataReaderViewGetQos (
    _DataReaderView dataReaderView,
    gapi_dataReaderViewQos * qos)
{
    v_dataViewQos dataViewQos;
    u_dataView uDataView;

    assert(dataReaderView);

    uDataView = u_dataView(U_DATAREADERVIEW_GET(dataReaderView));

    if ( u_entityQoS(u_entity(uDataView), (v_qos*)&dataViewQos) == U_RESULT_OK ) {
        copyReaderViewQosOut(dataViewQos,  qos);
        u_dataViewQosFree(dataViewQos);
    }

    return qos;
}

/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderViewQos qos);
 *
 * Function will operate independent of the enable flag
 */
gapi_returnCode_t
gapi_dataReaderView_set_qos (
    gapi_dataReaderView _this,
    const gapi_dataReaderViewQos *qos)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    u_result uResult;
    _DataReaderView dataReaderView;
    v_dataViewQos dataReaderViewQos;
    gapi_context context;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_SET_QOS);

    dataReaderView = gapi_dataReaderViewClaim(_this, &result);

    if (dataReaderView != NULL) {
        if ( dataReaderView && qos) {
           result = gapi_dataReaderViewQosIsConsistent(qos, &context);
        } else {
            result = GAPI_RETCODE_BAD_PARAMETER;
        }

        if ( result == GAPI_RETCODE_OK ) {
            gapi_dataReaderViewQos * existing_qos = gapi_dataReaderViewQos__alloc();

            result = gapi_dataReaderViewQosCheckMutability(qos, _DataReaderViewGetQos(dataReaderView, existing_qos), &context);
            gapi_free(existing_qos);
        }

        if ( result == GAPI_RETCODE_OK ) {
            dataReaderViewQos = u_dataViewQosNew(NULL);
            if (dataReaderViewQos) {
                if ( copyReaderViewQosIn(qos, dataReaderViewQos) ) {
                    uResult = u_entitySetQoS(_EntityUEntity(dataReaderView),(v_qos)(dataReaderViewQos) );
                    result = kernelResultToApiResult(uResult);
                } else {
                    result = GAPI_RETCODE_OUT_OF_RESOURCES;
                }
                u_dataViewQosFree(dataReaderViewQos);
            } else {
                result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        }

        _EntityRelease(dataReaderView);
    }

    return result;
}

gapi_statusCondition
gapi_dataReaderView_get_statuscondition(
    gapi_dataReaderView _this)
{
    return GAPI_HANDLE_NIL;
}

gapi_statusMask
gapi_dataReaderView_get_status_changes(
    gapi_dataReaderView _this)
{
    gapi_statusMask result = GAPI_STATUS_KIND_NULL;
    _DataReaderView dataReaderView;
    _DataReader dataReader;

    dataReaderView = gapi_dataReaderViewClaim(_this, NULL);

    if ( dataReaderView != NULL ) {
        dataReader = _DataReaderViewDataReader(dataReaderView);

        if (dataReader != NULL) {
            result = _StatusGetCurrentStatus(_Entity(dataReader)->status);
            _EntityRelease(dataReader);
        }

        _EntityRelease(dataReaderView);
    }
    return result;
}

/*     void
 *     get_qos(
 *         inout DataReaderViewQos qos);
 *
 * Function will operate indepenedent of the enable flag
 */
gapi_returnCode_t
gapi_dataReaderView_get_qos (
    gapi_dataReaderView _this,
    gapi_dataReaderViewQos *qos)
{
    _DataReaderView dataReaderView;
    gapi_returnCode_t result;

    dataReaderView = gapi_dataReaderViewClaim(_this, &result);
    if ( dataReaderView && qos ) {
        _DataReaderViewGetQos(dataReaderView, qos);
    }

    _EntityRelease(dataReaderView);
    return result;
}

/*     DataReader
 *     get_datareader();
 */
gapi_dataReader
gapi_dataReaderView_get_datareader(
    gapi_dataReaderView _this)
{
    _DataReader datareader = NULL;
    _DataReaderView dataReaderView;

    dataReaderView = gapi_dataReaderViewClaim(_this, NULL);

    if ( dataReaderView != NULL ) {
        datareader = _DataReaderViewDataReader(dataReaderView);
    }

    _EntityRelease(dataReaderView);

    return (gapi_dataReader)_EntityRelease(datareader);
}

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
gapi_readCondition
gapi_dataReaderView_create_readcondition (
    gapi_dataReaderView _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReaderView datareaderview;
    _ReadCondition readCondition = NULL;

    datareaderview = gapi_dataReaderViewClaim(_this, NULL);

    if ( datareaderview && _EntityEnabled(datareaderview) &&
         gapi_stateMasksValid(sample_states, view_states, instance_states) ) {

        _DataReader datareader;

        datareader = _DataReaderViewDataReader(datareaderview);
        readCondition = _ReadConditionNew ( sample_states,
                                            view_states,
                                            instance_states,
                                            datareader,
                                            datareaderview);
        _EntityRelease(datareader);
        if ( readCondition != NULL ) {
            gapi_deleteEntityAction deleteAction;
            void *actionArg;

            if ( _ObjectGetDeleteAction(_Object(readCondition),
                                        &deleteAction, &actionArg) ) {
                _ObjectSetDeleteAction(_Object(readCondition),
                                       deleteAction,
                                       actionArg);
            }

            _ENTITY_REGISTER_OBJECT(_Entity(datareaderview),
                                    (_Object)readCondition);
        }
    }

    _EntityRelease(datareaderview);

    return (gapi_readCondition)_EntityRelease(readCondition);
}

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
gapi_queryCondition
gapi_dataReaderView_create_querycondition (
    gapi_dataReaderView _this,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states,
    const gapi_char *query_expression,
    const gapi_stringSeq *query_parameters)
{
    _DataReaderView datareaderview;
    _QueryCondition queryCondition = NULL;

    datareaderview = gapi_dataReaderViewClaim(_this, NULL);

    if ( datareaderview && _EntityEnabled(datareaderview) &&
         query_expression && gapi_sequence_is_valid(query_parameters) &&
         gapi_stateMasksValid(sample_states, view_states, instance_states) ) {

        _DataReader datareader;

        datareader = _DataReaderViewDataReader(datareaderview);
        queryCondition = _QueryConditionNew(sample_states,
                                            view_states,
                                            instance_states,
                                            query_expression,
                                            query_parameters,
                                            datareader,
                                            datareaderview);
        _EntityRelease(datareader);
        if ( queryCondition != NULL ) {
            _ENTITY_REGISTER_OBJECT(_Entity(datareaderview),
                                    (_Object)queryCondition);
        }
    }

    _EntityRelease(datareaderview);

    return (gapi_queryCondition)_EntityRelease(queryCondition);
}


/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
gapi_returnCode_t
gapi_dataReaderView_delete_readcondition (
    gapi_dataReaderView _this,
    const gapi_readCondition a_condition)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReaderView datareaderview;
    _ReadCondition readCondition = NULL;
    c_bool contains;

    if (_this && a_condition) {
        datareaderview = gapi_dataReaderViewClaim(_this, &result);
        if (datareaderview != NULL) {
            readCondition = gapi_readConditionClaim(a_condition, NULL);
            if (readCondition != NULL ) {
                contains = u_readerContainsQuery(U_READER_GET(datareaderview),
                                                 U_QUERY_GET(readCondition));
                if (contains) {
                    _ReadConditionFree(readCondition);
                } else {
                    result = GAPI_RETCODE_PRECONDITION_NOT_MET;
                    _EntityRelease(readCondition);
                }
            } else {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
            _EntityRelease(datareaderview);
        } else {
            result = GAPI_RETCODE_ALREADY_DELETED;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }
    return result;
}


/*     ReturnCode_t
 *     delete_contained_entities();
 */
gapi_returnCode_t
gapi_dataReaderView_delete_contained_entities (
    gapi_dataReaderView _this)
{
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReaderView datareaderview;
    gapi_context context;
    _Condition condition = NULL;
    c_iter entities;
    u_entity e;

    GAPI_CONTEXT_SET(context, _this, GAPI_METHOD_DELETE_CONTAINED_ENTITIES);

    if ( _this != NULL ) {
        datareaderview = gapi_dataReaderViewClaim(_this, &result);
        if ( datareaderview != NULL ) {
            if (!gapi_loanRegistry_is_empty(datareaderview->loanRegistry)) {
                result = GAPI_RETCODE_PRECONDITION_NOT_MET;
            } else {
                entities = u_readerLookupQueries(U_READER_GET(datareaderview));
                e = c_iterTakeFirst(entities);
                while (e) {
                    condition = u_entityGetUserData(e);
                    if (condition) {
                        _ObjectReadClaimNotBusy(_Object(condition));
                        _ConditionFree(condition);
                    } else {
                        assert(condition);
                        result = GAPI_RETCODE_BAD_PARAMETER;
                    }
                    e = c_iterTakeFirst(entities);
                }
                c_iterFree(entities);
            }
            _EntityRelease(datareaderview);
        } else {
            result = GAPI_RETCODE_ALREADY_DELETED;
        }
    } else {
        result = GAPI_RETCODE_BAD_PARAMETER;
    }

    return result;
}


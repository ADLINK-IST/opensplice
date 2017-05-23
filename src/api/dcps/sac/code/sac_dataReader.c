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

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_common.h"
#include "sac_object.h"
#include "sac_objManag.h"
#include "sac_genericCopyIn.h"
#include "sac_genericCopyOut.h"
#include "sac_entity.h"
#include "sac_dataReader.h"
#include "dds_dcps_builtintopics.h"
#include "dds_builtinTopicsSplDcps.h"
#include "sac_domainParticipant.h"
#include "sac_topicDescription.h"
#include "sac_typeSupport.h"
#include "sac_contentFilteredTopic.h"
#include "sac_readCondition.h"
#include "sac_queryCondition.h"
#include "sac_dataReaderView.h"
#include "sac_loanRegistry.h"
#include "sac_readerCommon.h"
#include "sac_subscriber.h"
#include "u_entity.h"
#include "u_reader.h"
#include "u_dataReader.h"
#include "dds_dcps.h"
#include "kernelModule.h"
#include "v_state.h"
#include "v_topic.h"
#include "v_dataReaderInstance.h"
#include "sac_report.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"


#define DDS_DataReaderClaim(_this, reader) \
        DDS_Object_claim(DDS_Object(_this), DDS_DATAREADER, (_Object *)reader)

#define DDS_DataReaderClaimRead(_this, reader) \
        DDS_Object_claim(DDS_Object(_this), DDS_DATAREADER, (_Object *)reader)

#define DDS_DataReaderRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_DataReaderCheck(_this, reader) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_DATAREADER, (_Object *)reader)

#define _DataReader_get_user_entity(_this) \
        u_dataReader(_Entity_get_user_entity(_Entity(_this)))

#define _DataReader_get_uReader(_this) \
        u_reader(_Entity_get_user_entity(_Entity(_this)))


#define DATAREADER_STATUS_MASK (DDS_SAMPLE_REJECTED_STATUS            |\
                                DDS_LIVELINESS_CHANGED_STATUS         |\
                                DDS_REQUESTED_DEADLINE_MISSED_STATUS  |\
                                DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS |\
                                DDS_DATA_AVAILABLE_STATUS             |\
                                DDS_SAMPLE_LOST_STATUS                |\
                                DDS_SUBSCRIPTION_MATCHED_STATUS)

static DDS_ReturnCode_t
_DataReader_delete_contained_entities(
    _DataReader _this)
{
    DDS_ReturnCode_t result, endResult = DDS_RETCODE_OK;
    DDS_ReadCondition rc;
    DDS_QueryCondition qc;
    DDS_DataReaderView view;
    c_ulong i, nrEntities;

    /* Make sure we attempt to delete each entity only once:
     * entities that are not ready to be deleted should be
     * inserted back into the list, but should not be encountered
     * again during this iteration. So iterate by the number
     * of elements instead of by taking until the list is empty.
     */
    nrEntities = c_iterLength(_this->readConditionList);
    for (i = 0; i < nrEntities; i++) {
        rc = DDS_ReadCondition(c_iterTakeFirst(_this->readConditionList));
        result = DDS__free(rc);
        if (result != DDS_RETCODE_OK) {
            c_iterInsert(_this->readConditionList, rc);
            endResult = result;
        }
    }

    nrEntities = c_iterLength(_this->queryConditionList);
    for (i = 0; i < nrEntities; i++) {
        qc = DDS_QueryCondition(c_iterTakeFirst(_this->queryConditionList));
        result = DDS__free(qc);
        if (result != DDS_RETCODE_OK) {
            c_iterInsert(_this->queryConditionList, qc);
            endResult = result;
        }
    }

    nrEntities = c_iterLength(_this->dataReaderViewList);
    for (i = 0; i < nrEntities; i++) {
        view = DDS_DataReaderView(c_iterTakeFirst(_this->dataReaderViewList));
        result = DDS_DataReaderViewFree(view);
        if (result != DDS_RETCODE_OK) {
            c_iterInsert(_this->dataReaderViewList, view);
            endResult = result;
        }
    }
    return endResult;
}

static DDS_ReturnCode_t
_DataReader_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DataReader reader;

    reader = _DataReader(_this);
    if (reader != NULL) {
        if (c_iterLength(reader->readConditionList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DataReader has %d ReadConditions",
                        c_iterLength(reader->readConditionList));
        }
        if (c_iterLength(reader->queryConditionList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DataReader has %d QueryConditions",
                        c_iterLength(reader->queryConditionList));
        }
        if (c_iterLength(reader->dataReaderViewList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DataReader has %d DataReaderViews",
                        c_iterLength(reader->dataReaderViewList));
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReader = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        DDS_Entity_set_listener_interest(DDS_Entity(reader), 0);
        DDS_Entity_disable_callbacks(DDS_Entity(reader));
        DDS_free(reader->defaultDataReaderViewQos);
        DDS_TopicDescription_free(reader->topicDescription);
        c_iterFree(reader->dataReaderViewList);
        c_iterFree(reader->queryConditionList);
        c_iterFree(reader->readConditionList);
        DDS_LoanRegistry_free(reader->loanRegistry);
        cmn_samplesList_free(reader->samplesList);
        _Entity_deinit(_this);
    }
    return result;
}

static DDS_ReturnCode_t
_DataReader_samples_flush_copy(
    _DataReader _this,
    cmn_samplesList samplesList,
    DDS_long length,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq)
{
    struct flushCopyArg arg;
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_long testlength;
    u_entity uEntity = _Entity_get_user_entity(_Entity(_this));

    /* Prepare the buffers. */
    if (length > 0) {
        assert(cmn_samplesList_length(samplesList) <= info_seq->_maximum);
        arg.reader         = _this;
        arg.data_seq       = data_seq;
        arg.info_seq       = info_seq;
        arg.seqIndex       = 0;
        result = DDS_ReturnCode_get(u_readerProtectCopyOutEnter(uEntity));
        if (result == DDS_RETCODE_OK) {
            testlength = (DDS_long) cmn_samplesList_flush(samplesList, DDS_ReaderCommon_samples_flush_copy, &arg);
            if (testlength < 0) {
                result = DDS_RETCODE_ALREADY_DELETED;
            }
            u_readerProtectCopyOutExit(uEntity);
            assert((result != DDS_RETCODE_OK) || (length == testlength));
        }
    } else {
        data_seq->_length = 0;
        info_seq->_length = 0;
        result = DDS_RETCODE_NO_DATA;
    }

    /* Use the reader common code to do the actual copying. */

    return result;
}


DDS_DataReader
DDS_DataReaderNew (
    DDS_Subscriber subscriber,
    const DDS_char *name,
    const DDS_DataReaderQos *qos,
    const DDS_TopicDescription tdesc)
{
    DDS_ReturnCode_t result;
    DDS_TypeSupport typeSupport;
    _DataReader _this = NULL;
    u_subscriber uSubscriber;
    u_dataReader uReader;
    u_readerQos rQos = NULL;
    os_char *expr;
    c_value *params;

    result = DDS_TopicDescription_get_typeSupport(tdesc, &typeSupport);
    if (typeSupport == NULL) {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
        SAC_REPORT(result, "No TypeSupport registered for TopicDescription");
    }
    /*
     * Translate parameters to kernel representation. QoS is guaranteed to be
     * consistent at this point.
     */
    if (result == DDS_RETCODE_OK) {
        rQos = DDS_DataReaderQos_copyIn(qos);
        if (rQos == NULL) {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to copy in qos values");
        }
    }
    if (result == DDS_RETCODE_OK) {
        if (DDS_Object_get_kind(tdesc) == DDS_CONTENTFILTEREDTOPIC) {
            result = DDS_ContentFilteredTopic_get_parameters(tdesc, &params);
        } else {
            params = NULL;
        }
        if (result == DDS_RETCODE_OK) {
            uSubscriber = u_subscriber(_Entity_get_user_entity(subscriber));
            if (uSubscriber != NULL) {
                expr = DDS_TopicDescription_get_expr(tdesc);
                uReader = u_dataReaderNew(uSubscriber, name, expr, params, rQos, FALSE);
                os_free(expr);
                if (uReader!= NULL) {
                    result = DDS_Object_new(DDS_DATAREADER, _DataReader_deinit, (_Object *)&_this);
                    if (result == DDS_RETCODE_OK) {
                        result = DDS_Entity_init(_this, u_entity(uReader));
                        DDS_Object_set_domain_id(_Object(_this), DDS_Object_get_domain_id(subscriber));
                    }
                } else {
                    result = DDS_RETCODE_OUT_OF_RESOURCES;
                }
            } else {
                result = DDS_RETCODE_BAD_PARAMETER;
            }
            os_free(params);
        }
        u_readerQosFree(rQos);
    }
    if (result == DDS_RETCODE_OK) {
        _this->subscriber = subscriber;
        _this->topicDescription = DDS_TopicDescription_keep(tdesc);
        _this->defaultDataReaderViewQos = DDS_DataReaderViewQos__alloc();
        if (_this->defaultDataReaderViewQos != NULL) {
            result = DDS_DataReaderViewQos_init(
                _this->defaultDataReaderViewQos, DDS_DATAREADERVIEW_QOS_DEFAULT);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
        }
        _this->readConditionList = NULL;
        _this->queryConditionList = NULL;
        _this->dataReaderViewList = NULL;
        _this->loanRegistry = DDS_LoanRegistry_new(typeSupport);

        _this->userdataOffset = C_SIZEOF(v_message);
        _this->messageOffset = 0;
        _this->copy_in = DDS_TypeSupportCopyIn (typeSupport);
        _this->copy_out = DDS_TypeSupportCopyOut (typeSupport);
        _this->copy_cache = DDS_TypeSupportCopyCache (typeSupport);
        _this->samplesList = cmn_samplesList_new(FALSE);
    }
    return (DDS_DataReader)_this;
}

DDS_ReturnCode_t
DDS_DataReaderFree (
    DDS_DataReader _this)
{
    DDS_ReturnCode_t result;
    _DataReader reader;

    result = DDS_DataReaderClaim(_this, &reader);
    if (result == DDS_RETCODE_OK) {
        if (!DDS_LoanRegistry_is_empty(reader->loanRegistry)) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DataReader has outstanding loans");
        } else {
            result = _DataReader_delete_contained_entities(reader);
        }
        DDS_DataReaderRelease(_this);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS__free(_this);
    }
    return result;
}

/*     ReadCondition
 *     create_readcondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states);
 */
DDS_ReadCondition
DDS_DataReader_create_readcondition (
    DDS_DataReader _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_ReadCondition rc = NULL;
    _DataReader dr;

    SAC_REPORT_STACK();

    result = DDS_DataReaderClaim(_this, &dr);
    if (result == DDS_RETCODE_OK) {
        rc = DDS_ReadConditionNew(_this, sample_states, view_states, instance_states);
        if (rc != NULL) {
            dr->readConditionList = c_iterInsert(dr->readConditionList, rc);
        }
        (void)DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, rc == NULL);
    return rc;
}

/*     QueryCondition
 *     create_querycondition(
 *         in SampleStateMask sample_states,
 *         in ViewStateMask view_states,
 *         in InstanceStateMask instance_states,
 *         in string query_expression,
 *         in StringSeq query_parameters);
 */
DDS_QueryCondition
DDS_DataReader_create_querycondition (
    DDS_DataReader _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states,
    const DDS_char *query_expression,
    const DDS_StringSeq *query_parameters)
{
    DDS_ReturnCode_t result;
    DDS_QueryCondition qc = NULL;
    _DataReader dr;

    SAC_REPORT_STACK();

    result = DDS_DataReaderClaim(_this, &dr);
    if (result == DDS_RETCODE_OK) {
        qc = DDS_QueryConditionNew(_this, sample_states, view_states, instance_states,
                                   query_expression, query_parameters);
        if (qc != NULL) {
            dr->queryConditionList = c_iterInsert(dr->queryConditionList, qc);
        }
        (void)DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, qc == NULL);
    return qc;
}

/*     ReturnCode_t
 *     delete_readcondition(
 *         in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS_DataReader_delete_readcondition (
    DDS_DataReader _this,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;
    DDS_ReadCondition found;
    _DataReader dr;

    SAC_REPORT_STACK();

    if (a_condition != NULL) {
        result = DDS_DataReaderClaim(_this, &dr);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "ReadCondition = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(dr->readConditionList, a_condition);
        if (found) {
            assert(found == a_condition);
            (void)DDS__free(found);
        } else {
            found = c_iterTake(dr->queryConditionList, a_condition);
            if (found) {
                assert(found == a_condition);
                (void)DDS__free(found);
            } else {
                /* The following call is expensive so only use it in case of exceptions. */
                if (DDS_Object_get_kind(DDS_Object(a_condition)) == DDS_READCONDITION) {
                    result = DDS_RETCODE_PRECONDITION_NOT_MET;
                    SAC_REPORT(result, "ReadCondition does not belong to this DataReader");
                } else {
                    result = DDS_RETCODE_BAD_PARAMETER;
                    SAC_REPORT(result, "ReadCondition parameter 'a_condition' is of type %s",
                                DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(a_condition))));
                }
            }
        }
        DDS_DataReaderRelease(dr);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

struct check_handle_arg {
    DDS_InstanceHandle_t handle;
    DDS_boolean result;
};

static c_bool
view_check_handle(
    void *object,
    struct check_handle_arg *arg)
{
    assert(object);
    assert(arg);

    if (!arg->result) {
        arg->result = DDS_Entity_check_handle(DDS_Entity(object), arg->handle);
    }
    return !arg->result;
}

DDS_boolean
DDS_DataReader_contains_entity (
    DDS_DataReader _this,
    DDS_InstanceHandle_t a_handle)
{
    DDS_ReturnCode_t result;
    _DataReader sub;
    struct check_handle_arg arg = { DDS_HANDLE_NIL, FALSE };

    SAC_REPORT_STACK();

    result = DDS_DataReaderClaimRead(_this, &sub);
    if (result == DDS_RETCODE_OK) {
        arg.handle = a_handle;
        c_iterWalkUntil(sub->dataReaderViewList, (c_iterAction)view_check_handle, &arg);
        result = DDS_DataReaderRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return arg.result;
}

/*     ReturnCode_t
 *     delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_DataReader_delete_contained_entities (
    DDS_DataReader _this)
{
    DDS_ReturnCode_t result;
    _DataReader dr;

    SAC_REPORT_STACK();

    result = DDS_DataReaderClaim(_this, &dr);
    if (result == DDS_RETCODE_OK) {
        result = _DataReader_delete_contained_entities(dr);
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     DataReaderView
 *     create_view (
 *     in DataReaderViewQos * qos);
 */
DDS_DataReaderView
DDS_DataReader_create_view (
    DDS_DataReader _this,
    const DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DataReaderView view = NULL;
    _DataReader dr;

    SAC_REPORT_STACK();

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else if (qos != DDS_DATAREADERVIEW_QOS_DEFAULT) {
        result = DDS_DataReaderViewQos_is_consistent(qos);
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_DataReaderClaim(_this, &dr);
    }
    if (result == DDS_RETCODE_OK) {
        if (_Entity_is_enabled(_Entity(dr))) {
            if (qos == DDS_DATAREADERVIEW_QOS_DEFAULT) {
                qos = dr->defaultDataReaderViewQos;
            }

            if (result == DDS_RETCODE_OK) {
                view = DDS_DataReaderViewNew(_this, "dataReaderView", qos, dr->topicDescription);
                if (view != NULL) {
                    dr->dataReaderViewList = c_iterInsert(dr->dataReaderViewList, view);
                }
            }
        } else {
            SAC_REPORT(result, "DataReader is not enabled");
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return DDS_DataReaderView(view);
}

/*     ReturnCode_t
 *     delete_view(
 *        in DataReaderView a_view);
 */
DDS_ReturnCode_t
DDS_DataReader_delete_view (
    DDS_DataReader _this,
    DDS_DataReaderView a_view)
{
    DDS_ReturnCode_t result;
    DDS_DataReaderView found;
    _DataReader dr;

    SAC_REPORT_STACK();

    if (_this && a_view) {
        result = DDS_DataReaderClaim(_this, &dr);
        if (result == DDS_RETCODE_OK) {
            found = c_iterTake(dr->dataReaderViewList, a_view);
            if (found) {
                assert(found == a_view);
                result = DDS__free(found);
                if (result != DDS_RETCODE_OK) {
                    c_iterInsert(dr->dataReaderViewList, a_view);
                }
            } else {
                /* The following call is expensive so only use it in case of exceptions. */
                if (DDS_Object_get_kind(DDS_Object(a_view)) == DDS_DATAREADERVIEW) {
                    result = DDS_RETCODE_PRECONDITION_NOT_MET;
                    SAC_REPORT(result, "DataReaderView does not belong to this DataReader");
                } else {
                    result = DDS_RETCODE_BAD_PARAMETER;
                    SAC_REPORT(result, "DataReaderView parameter 'a_view' is of type %s",
                                DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(a_view))));
                }
            }
            DDS_DataReaderRelease(dr);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReader = 0x%x, DataReaderView = 0x%x",
                    _this, a_view);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}


/*     ReturnCode_t
 *     set_qos(
 *         in DataReaderQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_set_qos (
    DDS_DataReader _this,
    const DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DataReaderQos readerQos;
    _DataReader r;
    u_readerQos rQos = NULL;
    u_result uResult;
    u_dataReader uReader;

    SAC_REPORT_STACK();

    memset(&readerQos, 0, sizeof(DDS_DataReaderQos));
    (void)DDS_DataReaderQos_init(&readerQos, DDS_DATAREADER_QOS_DEFAULT);

    result = DDS_DataReaderQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataReaderClaim(_this, &r);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_DATAREADER_QOS_DEFAULT ||
            qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS)
        {
            result = DDS_Subscriber_get_default_datareader_qos(
                r->subscriber, &readerQos);
            qos = &readerQos;
        }
        if (qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
            result = DDS_Subscriber_copy_from_topicdescription(
                r->subscriber, &readerQos, r->topicDescription);
            if (result == DDS_RETCODE_OK) {
                result = DDS_DataReaderQos_is_consistent(&readerQos);
            }
        }
        if (result == DDS_RETCODE_OK) {
            rQos = DDS_DataReaderQos_copyIn(qos);
            if (rQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to copy DDS_DataReaderQos");
            }
        }
        if (result == DDS_RETCODE_OK) {
            uReader = _DataReader_get_user_entity(r);
            uResult = u_dataReaderSetQos(uReader, rQos);
            result = DDS_ReturnCode_get(uResult);
            u_readerQosFree(rQos);
        }
        DDS_DataReaderRelease(_this);
    }

    (void)DDS_DataReaderQos_deinit(&readerQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_qos(
 *         inout DataReaderQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_get_qos (
    DDS_DataReader _this,
    DDS_DataReaderQos *qos)
{
    DDS_ReturnCode_t result;
    _DataReader reader;
    u_readerQos uQos;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DataReaderCheck(_this, &reader);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "DataReaderQos = NULL");
        } else if (qos == DDS_DATAREADER_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAREADER_QOS_DEFAULT' is read-only.");
        } else if (qos == DDS_DATAREADER_QOS_USE_TOPIC_QOS) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAREADER_QOS_USE_TOPIC_QOS' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataReaderGetQos(_DataReader_get_user_entity(reader), &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS_DataReaderQos_copyOut(uQos, qos);
            u_readerQosFree(uQos);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_listener(
 *         in DataReaderListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DataReader_set_listener (
    DDS_DataReader _this,
    const struct DDS_DataReaderListener *a_listener,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;
    _DataReader r;

    SAC_REPORT_STACK();

    result = DDS_DataReaderClaim(_this, &r);
    if (result == DDS_RETCODE_OK) {
        if (a_listener != NULL) {
            r->listener = *a_listener;
            result = DDS_Entity_set_listener_interest(DDS_Entity(r), mask);
        } else {
            memset(&r->listener, 0, sizeof(struct DDS_DataReaderListener));
            result = DDS_Entity_set_listener_interest(DDS_Entity(r), mask);
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     DataReaderListener
 *     get_listener();
 */
struct DDS_DataReaderListener
DDS_DataReader_get_listener (
    DDS_DataReader _this)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    struct DDS_DataReaderListener listener;

    SAC_REPORT_STACK();

    result = DDS_DataReaderCheck(_this, &r);
    if (result == DDS_RETCODE_OK) {
        listener = r->listener;
    } else {
        memset(&listener, 0, sizeof(struct DDS_DataReaderListener));
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return listener;
}

/*     ReturnCode_t
 *     set_listener_mask(
 *         in DataReaderListener a_listener,
 *         in StatusKindMask mask);
 */
DDS_ReturnCode_t
DDS_DataReader_set_listener_mask (
    _DataReader _this,
    const DDS_StatusMask mask)
{
    DDS_ReturnCode_t result;

    assert(_this);

    result = DDS_Entity_set_listener_interest(DDS_Entity(_this), mask);

    return result;
}

/*     TopicDescription
 *     get_topicdescription();
 */
DDS_TopicDescription
DDS_DataReader_get_topicdescription (
    DDS_DataReader _this)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    DDS_TopicDescription td = NULL;

    SAC_REPORT_STACK();

    result = DDS_DataReaderCheck(_this, &r);
    if (result == DDS_RETCODE_OK) {
        td = r->topicDescription;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return td;
}

/*     Subscriber
 *     get_subscriber();
 */
DDS_Subscriber
DDS_DataReader_get_subscriber (
    DDS_DataReader _this)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    DDS_Subscriber s = NULL;

    SAC_REPORT_STACK();

    result = DDS_DataReaderCheck(_this, &r);
    if (result == DDS_RETCODE_OK) {
        s = r->subscriber;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return s;
}

static v_result
copy_sample_rejected_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_sampleRejectedInfo *from;
    DDS_SampleRejectedStatus *to;

    from = (struct v_sampleRejectedInfo *)info;
    to = (DDS_SampleRejectedStatus *)arg;

    to->total_count        = from->totalCount;
    to->total_count_change = from->totalChanged;
    switch ( from->lastReason ) {
    case S_NOT_REJECTED:
        to->last_reason = DDS_NOT_REJECTED;
    break;
    case S_REJECTED_BY_INSTANCES_LIMIT:
        to->last_reason = DDS_REJECTED_BY_INSTANCES_LIMIT;
    break;
    case S_REJECTED_BY_SAMPLES_LIMIT:
        to->last_reason = DDS_REJECTED_BY_SAMPLES_LIMIT;
    break;
    case S_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT:
        to->last_reason = DDS_REJECTED_BY_SAMPLES_PER_INSTANCE_LIMIT;
    break;
    }
    to->last_instance_handle = u_instanceHandleFromGID(from->instanceHandle);

    return V_RESULT_OK;

}

/* ReturnCode_t
 *get_sample_rejected_status(
 *inout SampleRejectedStatus status);
 */
DDS_ReturnCode_t
DDS_DataReader_get_sample_rejected_status (
    DDS_DataReader _this,
    DDS_SampleRejectedStatus *status)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            uResult = u_readerGetSampleRejectedStatus(
                          _DataReader_get_uReader(r),
                          TRUE,
                          copy_sample_rejected_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "SampleRejectedStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_liveliness_changed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_livelinessChangedInfo *from;
    DDS_LivelinessChangedStatus *to;

    from = (struct v_livelinessChangedInfo *)info;
    to = (DDS_LivelinessChangedStatus *)arg;

    to->alive_count = from->activeCount;
    to->not_alive_count = from->inactiveCount;
    to->alive_count_change = from->activeChanged;
    to->not_alive_count_change = from->inactiveChanged;

    to->last_publication_handle = u_instanceHandleFromGID(from->instanceHandle);

    return V_RESULT_OK;
}

/* ReturnCode_t
 *get_liveliness_changed_status(
 *inout LivelinessChangedStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_liveliness_changed_status (
    DDS_DataReader _this,
    DDS_LivelinessChangedStatus *status)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            uResult = u_readerGetLivelinessChangedStatus(
                          _DataReader_get_uReader(r),
                          TRUE,
                          copy_liveliness_changed_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "LivelinessChangedStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_deadline_missed_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_deadlineMissedInfo *from;
    DDS_RequestedDeadlineMissedStatus *to;
    v_result result;
    v_object instance;

    result = V_RESULT_INTERNAL_ERROR;
    from = (struct v_deadlineMissedInfo *)info;
    to = (DDS_RequestedDeadlineMissedStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    if (!v_handleIsNil(from->instanceHandle)) {
        if (v_handleClaim(from->instanceHandle, &instance) == V_HANDLE_OK) {
            to->last_instance_handle = u_instanceHandleNew(v_public(instance));
            if (v_handleRelease(from->instanceHandle) == V_HANDLE_OK) {
                result = V_RESULT_OK;
            }
        }
    } else {
        result = V_RESULT_OK;
    }

    return result;
}

/* ReturnCode_t
 *get_requested_deadline_missed_status(
 *inout RequestedDeadlineMissedStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_requested_deadline_missed_status (
    DDS_DataReader _this,
    DDS_RequestedDeadlineMissedStatus *status)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            uResult = u_readerGetDeadlineMissedStatus(
                          _DataReader_get_uReader(r),
                          TRUE,
                          copy_deadline_missed_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "RequestedDeadlineMissedStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_incompatible_qos_status(
    c_voidp info,
    c_voidp arg)
{
    DDS_unsigned_long i, j, len;
    v_result result = V_RESULT_OK;
    struct v_incompatibleQosInfo *from;
    DDS_RequestedIncompatibleQosStatus *to;

    from = (struct v_incompatibleQosInfo *)info;
    to = (DDS_RequestedIncompatibleQosStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    to->last_policy_id = from->lastPolicyId;

    len = 0;
    for (i=0; i<V_POLICY_ID_COUNT; i++) {
        if (from->policyCount[i] > 0) len++;
    }

    if ((to->policies._buffer != NULL) && (len > to->policies._maximum)) {
        DDS_free(to->policies._buffer);
        to->policies._buffer = NULL;
    }
    if ((to->policies._buffer == NULL) && (len > 0)) {
        to->policies._buffer = DDS_QosPolicyCountSeq_allocbuf(len);
        if (to->policies._buffer == NULL) {
            result = V_RESULT_OUT_OF_RESOURCES;
            SAC_REPORT(DDS_RETCODE_OUT_OF_RESOURCES, "No resources to allocate buffer for QosPolicyCountSeq");
        } else {
            to->policies._maximum = len;
        }
    }
    if ( to->policies._buffer ) {
        to->policies._length = len;
        j=0;
        for ( i = 0; i < V_POLICY_ID_COUNT; i++ ) {
            if (from->policyCount[i] > 0) {
                to->policies._buffer[j].policy_id = (DDS_QosPolicyId_t) i;
                to->policies._buffer[j++].count = from->policyCount[i];
            }
        }
    } else {
        to->policies._maximum = 0;
        to->policies._length = 0;
    }
    return result;
}

/* ReturnCode_t
 *get_requested_incompatible_qos_status(
 *inout RequestedIncompatibleQosStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_requested_incompatible_qos_status (
    DDS_DataReader _this,
    DDS_RequestedIncompatibleQosStatus *status)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            uResult = u_readerGetIncompatibleQosStatus(
                          _DataReader_get_uReader(r),
                          TRUE,
                          copy_incompatible_qos_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "RequestedIncompatibleQosStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_sample_lost_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_sampleLostInfo *from;
    DDS_SampleLostStatus *to;

    from = (struct v_sampleLostInfo *)info;
    to = (DDS_SampleLostStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;

    return V_RESULT_OK;
}

/* ReturnCode_t
 *get_sample_lost_status(
 *inout SampleLostStatus status);
*/
DDS_ReturnCode_t
DDS_DataReader_get_sample_lost_status (
    DDS_DataReader _this,
    DDS_SampleLostStatus *status)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            uResult = u_readerGetSampleLostStatus(
                          _DataReader_get_uReader(r),
                          TRUE,
                          copy_sample_lost_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "SampleLostStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

static v_result
copy_subscription_matched_status(
    c_voidp info,
    c_voidp arg)
{
    struct v_topicMatchInfo *from;
    DDS_SubscriptionMatchedStatus *to;

    from = (struct v_topicMatchInfo *)info;
    to = (DDS_SubscriptionMatchedStatus *)arg;

    to->total_count = from->totalCount;
    to->total_count_change = from->totalChanged;
    to->current_count = from->currentCount;
    to->current_count_change = from->currentChanged;
    to->last_publication_handle = u_instanceHandleFromGID(from->instanceHandle);
    return V_RESULT_OK;
}

/* ReturnCode_t
 * get_subscription_matched_status(
 *      inout SubscriptionMatchedStatus status);
 */
DDS_ReturnCode_t
DDS_DataReader_get_subscription_matched_status (
    DDS_DataReader _this,
    DDS_SubscriptionMatchedStatus *status)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (status != NULL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            uResult = u_readerGetSubscriptionMatchStatus(
                          _DataReader_get_uReader(r),
                          TRUE,
                          copy_subscription_matched_status,
                          status);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "SubscriptionMatchedStatus holder = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     wait_for_historical_data(
 *         in Duration_t max_wait);
 */
DDS_ReturnCode_t
DDS_DataReader_wait_for_historical_data (
    DDS_DataReader _this,
    const DDS_Duration_t *max_wait)
{
    DDS_ReturnCode_t result;
    _DataReader dr;
    u_result uResult;
    os_duration timeout;

    SAC_REPORT_STACK();

    result = DDS_DataReaderCheck(_this, &dr);
    if (result == DDS_RETCODE_OK) {
        if (!max_wait) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Duration_t max_wait = NULL");
        } else if (!DDS_Duration_is_valid(max_wait)) {
            result = DDS_RETCODE_BAD_PARAMETER;
        } else if (!_Entity_is_enabled(_Entity(dr))) {
            result = DDS_RETCODE_NOT_ENABLED;
            SAC_REPORT(result, "DataReader is not enabled");
        } else {
            result = DDS_Duration_copyIn(max_wait, &timeout);

            if (result == DDS_RETCODE_OK) {
                uResult = u_dataReaderWaitForHistoricalData(
                              _DataReader_get_user_entity(dr),
                              timeout);
                result = DDS_ReturnCode_get(uResult);
            }
        }
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_wait_for_historical_data_w_condition (
    DDS_DataReader _this,
    const DDS_char *filter_expression,
    const DDS_StringSeq *filter_parameters,
    const DDS_Time_t *min_source_timestamp,
    const DDS_Time_t *max_source_timestamp,
    const DDS_ResourceLimitsQosPolicy *resource_limits,
    const DDS_Duration_t *max_wait)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;
    os_duration maxWait;
    os_timeW  minSourceTimestamp,
              maxSourceTimestamp;
    c_ulong length, i;
    const os_char **params = NULL;
    os_int64 maxSupportedSeconds;

    SAC_REPORT_STACK();

    result = DDS_DataReaderCheck(_this, &r);
    if (result == DDS_RETCODE_OK) {
        if (max_wait == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Duration_t max_wait = NULL");
        } else if (!DDS_Duration_is_valid(max_wait)) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Duration_t max_wait is invalid");
        } else if (filter_parameters && !DDS_StringSeq_is_valid(filter_parameters)) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "StringSeq filter_parameters = NULL");
        } else {
            if (filter_parameters) {
                length = filter_parameters->_length;
                if(length > 0) {
                    params = (const os_char**)(os_malloc(length*sizeof(os_char*)));
                    for(i=0; i<length; i++){
                        params[i] = filter_parameters->_buffer[i];
                    }
                } else {
                    params = NULL;
                }
            } else {
                params = NULL;
                length = 0;
            }
            maxSupportedSeconds = SAC_ENTITY_MAX_SUPPORTED_SECONDS(_this);
            result = DDS_Time_copyIn(min_source_timestamp, &minSourceTimestamp, maxSupportedSeconds);
            if (result != DDS_RETCODE_OK) {
                SAC_REPORT(result, "Time_t min_source_timestamp is invalid or not supported");
            }
            result = DDS_Time_copyIn(max_source_timestamp, &maxSourceTimestamp, maxSupportedSeconds);
            if (result != DDS_RETCODE_OK) {
                SAC_REPORT(result, "Time_t max_source_timestamp is invalid or not supported");
            }
            if (result == DDS_RETCODE_OK) {
                result = DDS_Duration_copyIn(max_wait, &maxWait);
                if (result != DDS_RETCODE_OK) {
                    SAC_PANIC("result is %d", result);
                }

                uResult= u_dataReaderWaitForHistoricalDataWithCondition(
                            _DataReader_get_user_entity(r),
                            (os_char *)filter_expression,
                            params,
                            length,
                            minSourceTimestamp,
                            maxSourceTimestamp,
                            resource_limits->max_samples,
                            resource_limits->max_instances,
                            resource_limits->max_samples_per_instance,
                            maxWait);
                result = DDS_ReturnCode_get(uResult);
            }
            os_free((void *)params);
        }
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_TIMEOUT));
    return result;
}

static v_result
copy_matched_publication(
    u_publicationInfo *info,
    c_voidp arg)
{
    DDS_InstanceHandleSeq *to;
    DDS_InstanceHandle_t *tmp_buffer;

    to = (DDS_InstanceHandleSeq *)arg;

    if (to->_maximum <= to->_length) {
        tmp_buffer = to->_buffer;
        to->_buffer = DDS_InstanceHandleSeq_allocbuf(to->_length + 10);
        to->_maximum = to->_length + 10;
        if (tmp_buffer) {
            memcpy(to->_buffer, tmp_buffer, to->_length * sizeof(*to->_buffer));
            DDS_free(tmp_buffer);
        }
    }

    to->_buffer[to->_length] = u_instanceHandleFromGID(info->key);
    ++to->_length;
    return V_RESULT_OK;
}

/*     ReturnCode_t get_matched_publications(
 *     inout InstanceHandleSeq publication_handles);
 */
DDS_ReturnCode_t
DDS_DataReader_get_matched_publications (
    DDS_DataReader _this,
    DDS_InstanceHandleSeq *publication_handles)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (publication_handles != NULL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            publication_handles->_length = 0;
            uResult = u_readerGetMatchedPublications(
                          _DataReader_get_uReader(r),
                          copy_matched_publication,
                          publication_handles);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "InstanceHandleSeq publication_handles = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/* TODO:
 * Temporary wrapper to fix the missing result value.
 * The preprocessor must be changes to generate copy operation with a result value.
 */
static v_result
___DDS_PublicationBuiltinTopicData__copyOut(
    u_publicationInfo *info,
    void *arg)
{
    __DDS_PublicationBuiltinTopicData__copyOut(info, arg);
    return V_RESULT_OK;
}

/*     ReturnCode_t
 *     get_matched_publication_data(
 *         inout PublicationBuiltinTopicData publication_data,
 *         in InstanceHandle_t publication_handle);
 */
DDS_ReturnCode_t
DDS_DataReader_get_matched_publication_data (
    DDS_DataReader _this,
    DDS_PublicationBuiltinTopicData *publication_data,
    const DDS_InstanceHandle_t publication_handle)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    if (publication_handle != DDS_HANDLE_NIL) {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            uResult = u_readerGetMatchedPublicationData(
                          _DataReader_get_uReader(r),
                          publication_handle,
                          ___DDS_PublicationBuiltinTopicData__copyOut,
                          publication_data);
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DDS_InstanceHandle_t publication_handle = NULL");
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_default_datareaderview_qos(
 *         in DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_set_default_datareaderview_qos (
    DDS_DataReader _this,
    const DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DataReaderViewQos *viewQos = NULL;
    _DataReader r;

    SAC_REPORT_STACK();

    if (qos == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else if (qos != DDS_DATAREADERVIEW_QOS_DEFAULT) {
        result = DDS_DataReaderViewQos_is_consistent(qos);
    }
    if (result == DDS_RETCODE_OK) {
        viewQos = DDS_DataReaderViewQos__alloc();
        if (viewQos != NULL) {
            result = DDS_DataReaderViewQos_init(viewQos, qos);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "Failed to copy DDS_DataReaderViewQos");
        }
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataReaderClaim(_this, &r);
    }
    if (result == DDS_RETCODE_OK) {
        DDS_free(r->defaultDataReaderViewQos);
        r->defaultDataReaderViewQos = viewQos;
        DDS_DataReaderRelease(_this);
    } else {
        DDS_free(viewQos);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     get_default_datareaderview_qos(
 *         inout DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReader_get_default_datareaderview_qos (
    DDS_DataReader _this,
    DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result;
    _DataReader r;

    SAC_REPORT_STACK();

    result = DDS_DataReaderClaimRead(_this, &r);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataReaderViewQos_init(qos, r->defaultDataReaderViewQos);
        DDS_DataReaderRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}


/*
 * Typeless DataReader operations
 *
 */

DDS_ReturnCode_t
DDS_DataReader_read (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;
    u_sampleMask mask;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderClaim(_this, &r);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(r->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataReaderRead(_DataReader_get_user_entity(r),
                                       mask, cmn_reader_action,
                                       r->samplesList, OS_DURATION_ZERO);
            if (uResult == U_RESULT_OK) {
                DDS_long length = (DDS_long) cmn_samplesList_length(r->samplesList);
                assert(length > 0);
                result = DDS_LoanRegistry_register(r->loanRegistry, data_seq, info_seq, length);
                if (result == DDS_RETCODE_OK) {
                    result = _DataReader_samples_flush_copy(r, r->samplesList, length, data_seq, info_seq);
                }
            } else {
                result = DDS_ReturnCode_get(uResult);
                data_seq->_length = 0;
                info_seq->_length = 0;
            }
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_take (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;
    u_sampleMask mask;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderClaim(_this, &r);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(r->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataReaderTake(_DataReader_get_user_entity(r),
                                       mask, cmn_reader_action,
                                       r->samplesList, OS_DURATION_ZERO);
            if (uResult == U_RESULT_OK) {
                DDS_long length = (DDS_long) cmn_samplesList_length(r->samplesList);
                assert(length > 0);
                result = DDS_LoanRegistry_register(r->loanRegistry, data_seq, info_seq, length);
                if (result == DDS_RETCODE_OK) {
                    result = _DataReader_samples_flush_copy(r, r->samplesList, length, data_seq, info_seq);
                }
            } else {
                result = DDS_ReturnCode_get(uResult);
                data_seq->_length = 0;
                info_seq->_length = 0;
            }
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_read_w_condition (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;

    SAC_REPORT_STACK();

    result = DDS_ReadCondition_read(a_condition, _this, data_seq, info_seq, max_samples);
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}


DDS_ReturnCode_t
DDS_DataReader_take_w_condition (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;

    SAC_REPORT_STACK();

    result = DDS_ReadCondition_take(a_condition, _this, data_seq, info_seq, max_samples);
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_read_next_sample (
    DDS_DataReader _this,
    DDS_Sample data,
    DDS_SampleInfo *sample_info)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(data);
    OS_UNUSED_ARG(sample_info);

    return DDS_RETCODE_UNSUPPORTED;
}

DDS_ReturnCode_t
DDS_DataReader_take_next_sample (
    DDS_DataReader _this,
    DDS_Sample data,
    DDS_SampleInfo *sample_info)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(data);
    OS_UNUSED_ARG(sample_info);

    return DDS_RETCODE_UNSUPPORTED;
}

DDS_ReturnCode_t
DDS_DataReader_read_instance_action (
    DDS_DataReader _this,
    const DDS_InstanceHandle_t a_handle,
    v_actionResult (*action)(void *, void *),
    void *arg)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    if (action != NULL) {
        result = DDS_DataReaderClaim(_this, &r);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Internal error, callback operation missing");
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataReaderReadInstance(_DataReader_get_user_entity(r),
                                           a_handle, U_STATE_ANY,
                                           action, arg, OS_DURATION_ZERO);
        result = DDS_ReturnCode_get(uResult);
        DDS_DataReaderRelease(_this);
    }
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_read_instance (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderClaim(_this, &r);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList samplesList = cmn_samplesList_new(FALSE);
            cmn_samplesList_reset(samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataReaderReadInstance(_DataReader_get_user_entity(r),
                                               a_handle, mask,
                                               cmn_reader_action,
                                               samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                DDS_long length = (DDS_long) cmn_samplesList_length(samplesList);
                assert(length > 0);
                result = DDS_LoanRegistry_register(r->loanRegistry, data_seq, info_seq, length);
                if (result == DDS_RETCODE_OK) {
                    result = _DataReader_samples_flush_copy(r, samplesList, length, data_seq, info_seq);
                }
            } else {
                result = DDS_ReturnCode_get(uResult);
                data_seq->_length = 0;
                info_seq->_length = 0;
            }
            cmn_samplesList_free(samplesList);
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return result;
}


DDS_ReturnCode_t
DDS_DataReader_take_instance_action (
    DDS_DataReader _this,
    const DDS_InstanceHandle_t a_handle,
    v_actionResult (*action)(void *, void *),
    void *arg)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    if (action != NULL) {
        result = DDS_DataReaderClaim(_this, &r);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Internal error, callback operation missing");
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataReaderTakeInstance(_DataReader_get_user_entity(r),
                                           a_handle, U_STATE_ANY,
                                           action, arg, OS_DURATION_ZERO);
        result = DDS_ReturnCode_get(uResult);
        DDS_DataReaderRelease(_this);
    }
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_take_instance (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderClaim(_this, &r);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList samplesList = cmn_samplesList_new(FALSE);
            cmn_samplesList_reset(samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataReaderTakeInstance(_DataReader_get_user_entity(r),
                                               a_handle, mask,
                                               cmn_reader_action,
                                               samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                DDS_long length = (DDS_long) cmn_samplesList_length(samplesList);
                assert(length > 0);
                result = DDS_LoanRegistry_register(r->loanRegistry, data_seq, info_seq, length);
                if (result == DDS_RETCODE_OK) {
                    result = _DataReader_samples_flush_copy(r, samplesList, length, data_seq, info_seq);
                }
            } else {
                result = DDS_ReturnCode_get(uResult);
                data_seq->_length = 0;
                info_seq->_length = 0;
            }
            cmn_samplesList_free(samplesList);
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_read_next_instance_internal(
    DDS_DataReader _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult = U_RESULT_OK;


    result = DDS_DataReaderCheck(_this, &r);
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataReaderReadNextInstance(_DataReader_get_user_entity(r),
                                               a_handle, mask,
                                               cmn_reader_nextInstanceAction,
                                               samplesList, OS_DURATION_ZERO);
        if (uResult != U_RESULT_HANDLE_EXPIRED) {
            result = DDS_ReturnCode_get(uResult);
        } else {
            result = DDS_RETCODE_HANDLE_EXPIRED;
        }
    }


    return result;
}

DDS_ReturnCode_t
DDS_DataReader_take_next_instance_internal(
    DDS_DataReader _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult = U_RESULT_OK;

    result = DDS_DataReaderCheck(_this, &r);
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataReaderTakeNextInstance(_DataReader_get_user_entity(r),
                                               a_handle, mask,
                                               cmn_reader_nextInstanceAction,
                                               samplesList, OS_DURATION_ZERO);
        if (uResult != U_RESULT_HANDLE_EXPIRED) {
            result = DDS_ReturnCode_get(uResult);
        } else {
            result = DDS_RETCODE_HANDLE_EXPIRED;
        }
    }

    return result;
}


DDS_ReturnCode_t
DDS_DataReader_read_next_instance(
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderClaim(_this, &r);
#if 0 /* TODO: spec states DDS_RETCODE_ILLEGAL_OPERATION should be returned if
               "the operation is invoked on an inappropriate object." */
        if (result == DDS_RETCODE_BAD_PARAMETER) {
            result = DDS_RETCODE_ILLEGAL_OPERATION;
        }
#endif
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(r->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataReaderReadNextInstance(_DataReader_get_user_entity(r),
                                                   a_handle, mask,
                                                   cmn_reader_nextInstanceAction,
                                                   r->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                DDS_long length = (DDS_long) cmn_samplesList_length(r->samplesList);
                assert(length > 0);
                result = DDS_LoanRegistry_register(r->loanRegistry, data_seq, info_seq, length);
                if (result == DDS_RETCODE_OK) {
                    result = _DataReader_samples_flush_copy(r, r->samplesList, length, data_seq, info_seq);
                }
            } else {
                result = DDS_ReturnCode_get(uResult);
                data_seq->_length = 0;
                info_seq->_length = 0;
            }
        }
        DDS_DataReaderRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);

    return result;
}

DDS_ReturnCode_t
DDS_DataReader_take_next_instance(
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderClaim(_this, &r);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(r->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataReaderTakeNextInstance(_DataReader_get_user_entity(r),
                                                   a_handle, mask,
                                                   cmn_reader_nextInstanceAction,
                                                   r->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                DDS_long length = (DDS_long) cmn_samplesList_length(r->samplesList);
                assert(length > 0);
                result = DDS_LoanRegistry_register(r->loanRegistry, data_seq, info_seq, length);
                if (result == DDS_RETCODE_OK) {
                    result = _DataReader_samples_flush_copy(r, r->samplesList, length, data_seq, info_seq);
                }
            } else {
                result = DDS_ReturnCode_get(uResult);
                data_seq->_length = 0;
                info_seq->_length = 0;
            }
        }
        DDS_DataReaderRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);

    return result;
}

DDS_ReturnCode_t
DDS_DataReader_read_next_instance_w_condition (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition)
{
    int result;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_ReadCondition_read_next_instance(a_condition, _this, a_handle,
                                                  data_seq, info_seq, max_samples);
    if (result == DDS_RETCODE_HANDLE_EXPIRED) {
        result = DDS_RETCODE_BAD_PARAMETER;
        noReport = TRUE;
    }

    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return (DDS_ReturnCode_t)result;
}

DDS_ReturnCode_t
DDS_DataReader_take_next_instance_w_condition (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition)
{
    int result;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_ReadCondition_take_next_instance(a_condition, _this, a_handle,
                                                  data_seq, info_seq, max_samples);

    if (result == DDS_RETCODE_HANDLE_EXPIRED) {
        result = DDS_RETCODE_BAD_PARAMETER;
        noReport = TRUE;
    }

    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return (DDS_ReturnCode_t)result;
}

DDS_ReturnCode_t
DDS_DataReader_return_loan (
    DDS_DataReader _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq)
{
    DDS_ReturnCode_t result;
    _DataReader r;

    SAC_REPORT_STACK();

    result = DDS_DataReaderClaim(_this, &r);
    if (result == DDS_RETCODE_OK) {
        if (!DDS_sequence_is_valid(data_seq)) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Sequence data_seq is invalid");
        } else if (!DDS_sequence_is_valid((_DDS_sequence )info_seq) ) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Sequence info_seq is invalid");
        } else {
            if (data_seq->_release == info_seq->_release) {
                if ((!data_seq->_release) && (data_seq->_buffer != NULL)) {
                    result = DDS_LoanRegistry_deregister(r->loanRegistry, data_seq, info_seq);
                }
            } else {
                result = DDS_RETCODE_PRECONDITION_NOT_MET;
                SAC_REPORT(result, "Info_seq._release (%s) != data_seq._release (%s)",
                            info_seq->_release?"TRUE":"FALSE", data_seq->_release?"TRUE":"FALSE");
            }
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

DDS_ReturnCode_t
DDS_DataReader_get_key_value (
    DDS_DataReader _this,
    DDS_Sample key_holder,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_RETCODE_OK;
    if (key_holder == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Sample key_holder = NULL");
    }
    if (handle == DDS_HANDLE_NIL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "InstanceHandle = DDS_HANDLE_NIL");
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataReaderClaim(_this, &r);
    }
    if (result == DDS_RETCODE_OK) {
        PREPEND_COPYOUTCACHE(r->copy_cache, key_holder, NULL);
        if (key_holder != NULL) {
            uResult = u_dataReaderCopyKeysFromInstanceHandle(
                        _DataReader_get_user_entity(r),
                        (u_instanceHandle)handle,
                        (u_copyOut)r->copy_out,
                        key_holder);
            result = DDS_ReturnCode_get(uResult);
            REMOVE_COPYOUTCACHE(r->copy_cache, key_holder);
        } else {
            result = DDS_RETCODE_OUT_OF_RESOURCES;
            SAC_REPORT(result, "DataReader could not copy out key value");
        }
        /* The OpenSplice user-layer may detect that the instance is deleted
         * In that case according to the spec return PRECONDITION_NOT_MET.
         */
        if (result == DDS_RETCODE_ALREADY_DELETED) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "InstanceHandle was already deleted");
        }
        DDS_DataReaderRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

typedef struct readerCopyInInfo_s {
    _DataReader reader;
    void *data;
} readerCopyInInfo;


static v_copyin_result
_DataReaderCopyIn (
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result = V_COPYIN_RESULT_OK;
    c_base base = c_getBase(c_object(type));
    const readerCopyInInfo *info = (const readerCopyInInfo *) data;

    if (info->reader->copy_cache) {
        C_STRUCT(DDS_srcInfo) dataInfo;

        dataInfo.copyProgram = info->reader->copy_cache;
        dataInfo.src = info->data;

        result = info->reader->copy_in (base, &dataInfo, to);
    } else {
        result = info->reader->copy_in (base, info->data, to);
    }
    return result;
}

DDS_InstanceHandle_t
DDS_DataReader_lookup_instance (
    DDS_DataReader _this,
    DDS_Sample instance_data)
{
    DDS_ReturnCode_t result;
    _DataReader r;
    DDS_InstanceHandle_t handle = DDS_HANDLE_NIL;
    u_result uResult;

    SAC_REPORT_STACK();

    if (instance_data == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Sample instance_data = NULL");
    } else {
        result = DDS_DataReaderCheck(_this, &r);
        if (result == DDS_RETCODE_OK) {
            readerCopyInInfo rData;

            rData.reader = r;
            rData.data = (void *)instance_data;

            uResult = u_dataReaderLookupInstance(
                              _DataReader_get_user_entity(r),
                              &rData,
                              _DataReaderCopyIn,
                              (u_instanceHandle *)&handle);
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return handle;
}


DDS_ReturnCode_t
DDS_DataReader_notify_listener (
    DDS_DataReader _this,
    v_listenerEvent event)
{
    DDS_ReturnCode_t result;
    u_eventMask triggerMask;
    struct DDS_DataReaderListener cb;

    cb = _DataReader(_this)->listener;
    triggerMask = event->kind;
    result = DDS_RETCODE_OK;

    if ((triggerMask & V_EVENT_DATA_AVAILABLE) &&
            (cb.on_data_available != NULL))
    {
        result = DDS_Entity_reset_dataAvailable_status(DDS_Entity(_this));
        if (result == DDS_RETCODE_OK) {
            cb.on_data_available(cb.listener_data, _this);
        }
    }
    if ((triggerMask & V_EVENT_SAMPLE_REJECTED) &&
            (cb.on_sample_rejected != NULL))
    {
        DDS_SampleRejectedStatus status;
        DDS_SampleRejectedStatus_init(&status, &((v_readerStatus)event->eventData)->sampleRejected);
        cb.on_sample_rejected(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_LIVELINESS_CHANGED) &&
            (cb.on_liveliness_changed != NULL))
    {
        DDS_LivelinessChangedStatus status;
        DDS_LivelinessChangedStatus_init(&status, &((v_readerStatus)event->eventData)->livelinessChanged);
        cb.on_liveliness_changed(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_REQUESTED_DEADLINE_MISSED) &&
            (cb.on_requested_deadline_missed != NULL))
    {
        DDS_RequestedDeadlineMissedStatus status;
        DDS_RequestedDeadlineMissedStatus_init(&status, &((v_readerStatus)event->eventData)->deadlineMissed);
        cb.on_requested_deadline_missed(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_REQUESTED_INCOMPATIBLE_QOS) &&
            (cb.on_requested_incompatible_qos != NULL))
    {
        DDS_RequestedIncompatibleQosStatus status;
        DDS_RequestedIncompatibleQosStatus_init(&status, &((v_readerStatus)event->eventData)->incompatibleQos);
        cb.on_requested_incompatible_qos(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_SAMPLE_LOST) &&
            (cb.on_sample_lost != NULL))
    {
        DDS_SampleLostStatus status;
        DDS_SampleLostStatus_init(&status, &((v_readerStatus)event->eventData)->sampleLost);
        cb.on_sample_lost(cb.listener_data, _this, &status);
    }
    if ((triggerMask & V_EVENT_SUBSCRIPTION_MATCHED) &&
            (cb.on_subscription_matched != NULL))
    {
        DDS_SubscriptionMatchedStatus status;
        DDS_SubscriptionMatchedStatus_init(&status, &((v_readerStatus)event->eventData)->subscriptionMatch);
        cb.on_subscription_matched(cb.listener_data, _this, &status);
    }

    return result;
}

/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
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
#include "sac_dataReaderView.h"
#include "sac_condition.h"
#include "sac_readCondition.h"
#include "sac_queryCondition.h"
#include "sac_loanRegistry.h"
#include "sac_readerCommon.h"
#include "sac_typeSupport.h"
#include "sac_topicDescription.h"
#include "u_dataView.h"
#include "u_dataViewQos.h"
#include "v_dataReaderInstance.h"
#include "v_state.h"
#include "v_dataView.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"
#include "sac_report.h"


#define DDS_DataReaderViewClaim(_this, reader) \
        DDS_Object_claim(DDS_Object(_this), DDS_DATAREADERVIEW, (_Object *)reader)

#define DDS_DataReaderViewClaimRead(_this, reader) \
        DDS_Object_claim(DDS_Object(_this), DDS_DATAREADERVIEW, (_Object *)reader)

#define DDS_DataReaderViewRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_DataReaderViewCheck(_this, reader) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_DATAREADERVIEW, (_Object *)reader)

#define _DataReaderView_get_user_entity(_this) \
        u_dataView(_Entity_get_user_entity(_Entity(_DataReaderView(_this))))


static DDS_ReturnCode_t
_DataReaderView_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DataReaderView view;

    view = _DataReaderView(_this);
    if (view != NULL) {
        if (!DDS_LoanRegistry_is_empty(view->loanRegistry)) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DataReaderView has outstanding loans");
        }
        if (c_iterLength(view->queryConditionList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DataReaderView has %d QueryConditions",
                        c_iterLength(view->queryConditionList));
        }
        if (c_iterLength(view->readConditionList) != 0) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "DataReaderView has %d ReadConditions",
                        c_iterLength(view->readConditionList));
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReaderView = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        c_iterFree(view->queryConditionList);
        c_iterFree(view->readConditionList);
        result = DDS_LoanRegistry_free(view->loanRegistry);
        if (result != DDS_RETCODE_OK) {
            SAC_PANIC("Could not free DDS_LoanRegistry");
        }
        cmn_samplesList_free(view->samplesList);
        result = _Entity_deinit(_this);
    }
    return result;
}

static DDS_ReturnCode_t
_DataReaderView_samples_flush_copy(
    _DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq)
{
    struct flushCopyArg arg;
    DDS_ReturnCode_t result;
    DDS_long length, testlength;
    u_entity uEntity = _Entity_get_user_entity(_Entity(_this));

    /* Prepare the buffers. */
    length = (DDS_long) cmn_samplesList_length(_this->samplesList);
    if (length > 0) {
        result = DDS_LoanRegistry_register(_this->loanRegistry, data_seq, info_seq, length);
        assert(cmn_samplesList_length(_this->samplesList) <= info_seq->_maximum);

        /* Use the reader common code to do the actual copying. */
        if (result == DDS_RETCODE_OK) {
            arg.reader         = _DataReader(_this->datareader);
            arg.data_seq       = data_seq;
            arg.info_seq       = info_seq;
            arg.seqIndex       = 0;
            result = DDS_ReturnCode_get(u_readerProtectCopyOutEnter(uEntity));
            if (result == DDS_RETCODE_OK) {
                testlength = cmn_samplesList_flush(_this->samplesList, DDS_ReaderCommon_samples_flush_copy, &arg);
                if (testlength < 0) {
                    result = DDS_RETCODE_ALREADY_DELETED;
                }
                u_readerProtectCopyOutExit(uEntity);
                assert((result != DDS_RETCODE_OK) || (length == testlength));
            }
        }
    } else {
        data_seq->_length = 0;
        info_seq->_length = 0;
        result = DDS_RETCODE_NO_DATA;
    }

    return result;
}

DDS_DataReaderView
DDS_DataReaderViewNew (
    const DDS_DataReader datareader,
    const DDS_char *name,
    const DDS_DataReaderViewQos *qos,
    const DDS_TopicDescription tdesc)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DataReaderView _this = NULL;
    u_dataViewQos uViewQos = NULL;
    u_dataView uReaderView;
    u_dataReader uReader;
    DDS_TypeSupport typeSupport;

    uViewQos = DDS_DataReaderViewQos_copyIn(qos);
    if (uViewQos == NULL) {
        result = DDS_RETCODE_OUT_OF_RESOURCES;
    }

    if (result == DDS_RETCODE_OK) {
        result = DDS_TopicDescription_get_typeSupport(tdesc, &typeSupport);
    }
    if (result == DDS_RETCODE_OK) {
        uReader = u_dataReader(_Entity_get_user_entity(datareader));
        uReaderView = u_dataViewNew(uReader, name, uViewQos);
        result = DDS_Object_new(DDS_DATAREADERVIEW, _DataReaderView_deinit, (_Object *)&_this);
        if (result == DDS_RETCODE_OK) {
            result = DDS_Entity_init(_this, u_entity(uReaderView));
            DDS_Object_set_domain_id(_Object(_this), DDS_Object_get_domain_id(datareader));
        }
        if (result == DDS_RETCODE_OK) {
            _this->datareader = datareader;
            _this->readConditionList = NULL;
            _this->queryConditionList = NULL;
            _this->loanRegistry = DDS_LoanRegistry_new(typeSupport);
            _this->samplesList = cmn_samplesList_new(TRUE);
        }
    }

    if (uViewQos != NULL) {
        u_dataViewQosFree(uViewQos);
    }

    return (DDS_DataReaderView)_this;
}

static DDS_ReturnCode_t
_DataReaderView_delete_contained_entities(
    _DataReaderView _this)
{
    DDS_ReturnCode_t result, endResult = DDS_RETCODE_OK;
    DDS_ReadCondition rc;
    DDS_QueryCondition qc;
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

    return endResult;
}

DDS_ReturnCode_t
DDS_DataReaderViewFree (
    DDS_DataReaderView _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DataReaderView view;

    result = DDS_DataReaderViewClaim(_this, &view);
    if (result == DDS_RETCODE_OK) {
        result = _DataReaderView_delete_contained_entities(view);
        DDS_DataReaderViewRelease(_this);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS__free(_this);
    }
    return result;
}


/* From Entity
 *      get_statuscondition
 */
DDS_StatusCondition
DDS_DataReaderView_get_statuscondition(
    DDS_DataReaderView _this)
{
    OS_UNUSED_ARG(_this);

    return DDS_StatusCondition(DDS_HANDLE_NIL);
}

/* From Entity
 *      DDS_DataReaderView_get_status_changes
 */
DDS_StatusMask
DDS_DataReaderView_get_status_changes(
    DDS_DataReaderView _this)
{
    DDS_ReturnCode_t result;
    DDS_StatusMask mask = 0;
    _DataReaderView view;

    SAC_REPORT_STACK();

    result = DDS_DataReaderViewCheck(_this, &view);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_DataReader_get_status_changes(view->datareader);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return mask;
}

/* ReturnCode_t
 * set_qos(
 *      in DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReaderView_set_qos(
    DDS_DataReaderView _this,
    const DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    DDS_DataReaderViewQos viewQos;
    _DataReaderView view;
    u_dataViewQos uViewQos = NULL;
    u_result uResult;
    u_dataView uView;

    SAC_REPORT_STACK();

    memset(&viewQos, 0, sizeof(DDS_DataReaderViewQos));
    (void)DDS_DataReaderViewQos_init(&viewQos, DDS_DATAREADERVIEW_QOS_DEFAULT);

    result = DDS_DataReaderViewQos_is_consistent(qos);
    if (result == DDS_RETCODE_OK) {
        result = DDS_DataReaderViewClaim(_this, &view);
    }
    if (result == DDS_RETCODE_OK) {
        if (qos == DDS_DATAREADERVIEW_QOS_DEFAULT) {
            result = DDS_DataReader_get_default_datareaderview_qos(
                view->datareader, &viewQos);
            qos = &viewQos;
        }
        if (result == DDS_RETCODE_OK) {
            uViewQos = DDS_DataReaderViewQos_copyIn(qos);
            if (uViewQos == NULL) {
                result = DDS_RETCODE_OUT_OF_RESOURCES;
                SAC_REPORT(result, "Failed to copy DDS_DataReaderViewQos");
            }
        }
        if (result == DDS_RETCODE_OK) {
            uView = _DataReaderView_get_user_entity(view);
            uResult = u_dataViewSetQos(uView, uViewQos);
            result = DDS_ReturnCode_get(uResult);
            u_dataViewQosFree(uViewQos);
        }
        DDS_DataReaderViewRelease(_this);
    }

    (void)DDS_DataReaderViewQos_deinit(&viewQos);

    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/* ReturnCode_t
 * get_qos(
 *      inout DataReaderViewQos qos);
 */
DDS_ReturnCode_t
DDS_DataReaderView_get_qos (
    DDS_DataReaderView _this,
    DDS_DataReaderViewQos *qos)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    u_dataViewQos uQos;
    u_result uResult;

    SAC_REPORT_STACK();

    result = DDS_DataReaderViewCheck(_this, &view);
    if (result == DDS_RETCODE_OK) {
        if (qos == NULL) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "DataReaderViewQos = NULL");
        } else if (qos == DDS_DATAREADERVIEW_QOS_DEFAULT) {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "QoS 'DATAREADERVIEW_QOS_DEFAULT' is read-only.");
        }
    }
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataViewGetQos(_DataReaderView_get_user_entity(view), &uQos);
        if (uResult == U_RESULT_OK) {
            result = DDS_DataReaderViewQos_copyOut(uQos, qos);
            u_dataViewQosFree(uQos);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/* DataReader
 * get_datareader();
 */
DDS_DataReader
DDS_DataReaderView_get_datareader(
    DDS_DataReaderView _this)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    DDS_DataReader reader = NULL;

    SAC_REPORT_STACK();

    result = DDS_DataReaderViewCheck(_this, &view);
    if (result == DDS_RETCODE_OK) {
        reader = view->datareader;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return reader;
}

/* ReadCondition
 * create_readcondition(
 *      in SampleStateMask sample_states,
 *      in ViewStateMask view_states,
 *      in InstanceStateMask instance_states);
 */
DDS_ReadCondition
DDS_DataReaderView_create_readcondition(
    DDS_DataReaderView _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_ReadCondition rc = NULL;
    _DataReaderView view;

    SAC_REPORT_STACK();

    result = DDS_DataReaderViewClaim(_this, &view);
    if (result == DDS_RETCODE_OK) {
        rc = DDS_ReadConditionNew(_this,sample_states, view_states, instance_states);
        if (rc != NULL) {
            view->readConditionList = c_iterInsert(view->readConditionList, rc);
        }
        (void)DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, rc == NULL);
    return rc;
}

/* QueryCondition
 * create_querycondition(
 *      in SampleStateMask sample_states,
 *      in ViewStateMask view_states,
 *      in InstanceStateMask instance_states,
 *      in string query_expression,
 *      in StringSeq query_parameters);
 */
DDS_QueryCondition
DDS_DataReaderView_create_querycondition(
    DDS_DataReaderView _this,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states,
    const DDS_char *query_expression,
    const DDS_StringSeq *query_parameters)
{
    DDS_ReturnCode_t result;
    DDS_QueryCondition qc = NULL;
    _DataReaderView view;

    SAC_REPORT_STACK();

    result = DDS_DataReaderViewClaim(_this, &view);
    if (result == DDS_RETCODE_OK) {
        qc = DDS_QueryConditionNew(_this, sample_states, view_states, instance_states,
                                   query_expression, query_parameters);
        if (qc != NULL) {
            view->queryConditionList = c_iterInsert(view->queryConditionList, qc);
        }
        (void)DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, qc == NULL);
    return qc;
}

/* ReturnCode_t
 * delete_readcondition(
 *      in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS_DataReaderView_delete_readcondition(
    DDS_DataReaderView _this,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;
    DDS_ReadCondition found;
    _DataReaderView view;

    SAC_REPORT_STACK();

    if (a_condition != NULL) {
        result = DDS_DataReaderViewClaim(_this, &view);
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "ReadCondition = NULL");
    }
    if (result == DDS_RETCODE_OK) {
        found = c_iterTake(view->readConditionList, a_condition);
        if (found) {
            assert(found == a_condition);
            (void)DDS__free(found);
        } else {
            found = c_iterTake(view->queryConditionList, a_condition);
            if (found) {
                assert(found == a_condition);
                (void)DDS__free(found);
            } else {
                /* The following call is expensive so only use it in case of exceptions. */
                if (DDS_Object_get_kind(DDS_Object(a_condition)) == DDS_READCONDITION) {
                    result = DDS_RETCODE_PRECONDITION_NOT_MET;
                    SAC_REPORT(result, "ReadCondition does not belong to this DataReaderView");
                } else {
                    result = DDS_RETCODE_BAD_PARAMETER;
                    SAC_REPORT(result, "ReadCondition parameter 'a_condition' is of type %s",
                                DDS_ObjectKind_image(DDS_Object_get_kind(DDS_Object(a_condition))));
                }
            }
        }
        DDS_DataReaderViewRelease(view);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/* ReturnCode_t
 * delete_contained_entities();
 */
DDS_ReturnCode_t
DDS_DataReaderView_delete_contained_entities(
    DDS_DataReaderView _this)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;

    SAC_REPORT_STACK();

    result = DDS_DataReaderViewClaim(_this, &view);
    if (result == DDS_RETCODE_OK) {
        result = _DataReaderView_delete_contained_entities(view);
        DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}


/*
 * Typeless DataReaderView operations.
 *
 */

#define statemasks_unsupported(sample_states, view_states, instance_states) \
        ((DDS_boolean)((sample_states != DDS_ANY_SAMPLE_STATE) || \
                       (view_states != DDS_ANY_VIEW_STATE) || \
                       (instance_states != DDS_ANY_INSTANCE_STATE)))

DDS_ReturnCode_t
DDS_DataReaderView_read (
    DDS_DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    u_result uResult;
    u_sampleMask mask;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderViewClaim(_this, &view);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(view->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataViewRead(_DataReaderView_get_user_entity(view),
                                     mask, cmn_reader_action, view->samplesList, OS_DURATION_ZERO);
            if (uResult == U_RESULT_OK) {
                result = _DataReaderView_samples_flush_copy(view, data_seq, info_seq);
            } else {
                result = DDS_ReturnCode_get(uResult);
            }
        }
        DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderView_take (
    DDS_DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    u_result uResult;
    u_sampleMask mask;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderViewClaim(_this, &view);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(view->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataViewTake(_DataReaderView_get_user_entity(view),
                                     mask, cmn_reader_action,
                                     view->samplesList, OS_DURATION_ZERO);
            if (uResult == U_RESULT_OK) {
                result = _DataReaderView_samples_flush_copy(view, data_seq, info_seq);
            } else {
                result = DDS_ReturnCode_get(uResult);
            }
        }
        DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA));
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderView_read_next_sample (
    DDS_DataReaderView _this,
    DDS_Sample data,
    DDS_SampleInfo *sample_info)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(data);
    OS_UNUSED_ARG(sample_info);

    return DDS_RETCODE_UNSUPPORTED;
}

DDS_ReturnCode_t
DDS_DataReaderView_take_next_sample (
    DDS_DataReaderView _this,
    DDS_Sample data,
    DDS_SampleInfo *sample_info)
{
    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(data);
    OS_UNUSED_ARG(sample_info);

    return DDS_RETCODE_UNSUPPORTED;
}

DDS_ReturnCode_t
DDS_DataReaderView_read_instance (
    DDS_DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderViewClaim(_this, &view);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(view->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataViewReadInstance(_DataReaderView_get_user_entity(view),
                                             a_handle, mask,
                                             cmn_reader_action,
                                             view->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                result = _DataReaderView_samples_flush_copy(view, data_seq, info_seq);
            } else {
                result = DDS_ReturnCode_get(uResult);
            }
        }
        DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderView_take_instance (
    DDS_DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderViewClaim(_this, &view);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(view->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataViewTakeInstance(_DataReaderView_get_user_entity(view),
                                             a_handle, mask,
                                             cmn_reader_action,
                                             view->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                result = _DataReaderView_samples_flush_copy(view, data_seq, info_seq);
            } else {
                result = DDS_ReturnCode_get(uResult);
            }
        }
        DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderView_read_next_instance_internal(
    DDS_DataReaderView _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList)
{
    int result;
    _DataReaderView view;
    u_result uResult = U_RESULT_OK;

    result = DDS_DataReaderViewCheck(_this, &view);
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataViewReadNextInstance(_DataReaderView_get_user_entity(view),
                                             a_handle, mask,
                                             cmn_reader_nextInstanceAction_OSPL3588,
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
DDS_DataReaderView_take_next_instance_internal(
    DDS_DataReaderView _this,
    const DDS_InstanceHandle_t a_handle,
    u_sampleMask mask,
    cmn_samplesList samplesList)
{
    int result;
    _DataReaderView view;
    u_result uResult = U_RESULT_OK;

    result = DDS_DataReaderViewCheck(_this, &view);
    if (result == DDS_RETCODE_OK) {
        uResult = u_dataViewTakeNextInstance(_DataReaderView_get_user_entity(view),
                                             a_handle, mask,
                                             cmn_reader_nextInstanceAction_OSPL3588,
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
DDS_DataReaderView_read_next_instance (
    DDS_DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderViewClaim(_this, &view);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(view->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataViewReadNextInstance(_DataReaderView_get_user_entity(view),
                                                 a_handle, mask,
                                                 cmn_reader_nextInstanceAction_OSPL3588,
                                                 view->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                result = _DataReaderView_samples_flush_copy(view, data_seq, info_seq);
            } else {
                result = DDS_ReturnCode_get(uResult);
            }
        }
        DDS_DataReaderViewRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderView_take_next_instance (
    DDS_DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    u_result uResult;
    u_sampleMask mask;
    DDS_boolean noReport = FALSE;

    SAC_REPORT_STACK();

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result == DDS_RETCODE_OK) {
        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        result = DDS_DataReaderViewClaim(_this, &view);
    } else {
        SAC_REPORT(result, "Invalid mask, sample_states = 0x%x, view_states = 0x%x, instance_states = 0x%x",
                    sample_states, view_states, instance_states);
    }
    if (result == DDS_RETCODE_OK) {
        result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
        if (result == DDS_RETCODE_OK) {
            cmn_samplesList_reset(view->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
            uResult = u_dataViewTakeNextInstance(_DataReaderView_get_user_entity(view),
                                                 a_handle, mask,
                                                 cmn_reader_nextInstanceAction_OSPL3588,
                                                 view->samplesList, OS_DURATION_ZERO);
            noReport = (uResult == U_RESULT_HANDLE_EXPIRED);
            if (uResult == U_RESULT_OK) {
                result = _DataReaderView_samples_flush_copy(view, data_seq, info_seq);
            } else {
                result = DDS_ReturnCode_get(uResult);
            }
        }
        DDS_DataReaderViewRelease(_this);
    }

    SAC_REPORT_FLUSH(_this, (result != DDS_RETCODE_OK) && (result != DDS_RETCODE_NO_DATA) && !noReport);
    return result;
}

DDS_ReturnCode_t
DDS_DataReaderView_return_loan (
    DDS_DataReaderView _this,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq)
{
    DDS_ReturnCode_t result = DDS_RETCODE_OK;
    _DataReaderView view;

    SAC_REPORT_STACK();

    if ( !DDS_sequence_is_valid(data_seq) ||
         !DDS_sequence_is_valid((_DDS_sequence)info_seq) )
    {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        if (data_seq->_release != info_seq->_release) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Data_seq->_release (%s) != info_seq->_release (%s)",
                        data_seq->_release?"TRUE":"FALSE", info_seq->_release?"TRUE":"FALSE");
        }
        if (data_seq->_length != info_seq->_length) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Data_seq->_length (%d) != info_seq->_length (%d)",
                        data_seq->_length, info_seq->_length);
        }
        if (data_seq->_maximum != info_seq->_maximum) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "Data_seq->_maximum (%d) != info_seq->_maximum (%d)",
                        data_seq->_maximum, info_seq->_maximum);
        }

        if (result == DDS_RETCODE_OK) {
            if (!data_seq->_release) {
                result = DDS_DataReaderViewClaim(_this, &view);
                if (result == DDS_RETCODE_OK) {
                    result = DDS_LoanRegistry_deregister(view->loanRegistry, data_seq, info_seq);
                    DDS_DataReaderViewRelease(_this);
                }
            }
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/* ReturnCode_t
 * read_w_condition(
 *      inout DataSeq data_seq,
 *      inout SampleInfoSeq info_seq,
 *      in long max_samples,
 *      in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS_DataReaderView_read_w_condition (
    DDS_DataReaderView _this,
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

/* ReturnCode_t
 * take_w_condition(
 *      inout DataSeq data_seq,
 *      inout SampleInfoSeq info_seq,
 *      in long max_samples,
 *      in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS_DataReaderView_take_w_condition (
    DDS_DataReaderView _this,
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
DDS_DataReaderView_read_next_instance_w_condition (
    DDS_DataReaderView _this,
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
DDS_DataReaderView_take_next_instance_w_condition (
    DDS_DataReaderView _this,
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
DDS_DataReaderView_get_key_value (
    DDS_DataReaderView _this,
    DDS_Sample key_holder,
    const DDS_InstanceHandle_t handle)
{
    DDS_ReturnCode_t result;

    OS_UNUSED_ARG(_this);
    OS_UNUSED_ARG(key_holder);
    OS_UNUSED_ARG(handle);

    SAC_REPORT_STACK();

    result = DDS_RETCODE_UNSUPPORTED;
    SAC_REPORT(result, "Operation is currently unsupported");
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

typedef struct readerViewCopyInInfo_s {
    _DataReader reader;
    void *data;
} readerViewCopyInInfo;

static v_copyin_result
_DataReaderViewCopyIn (
    c_type type,
    const void *data,
    void *to)
{
    v_copyin_result result;
    c_base base = c_getBase(c_object(type));
    const readerViewCopyInInfo *info = (const readerViewCopyInInfo *) data;

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

/* InstanceHandle_t
 * lookup_instance(
 *      in Data instance);
 */
DDS_InstanceHandle_t
DDS_DataReaderView_lookup_instance (
    DDS_DataReaderView _this,
    DDS_Sample instance_data)
{
    DDS_ReturnCode_t result;
    _DataReaderView view;
    DDS_InstanceHandle_t handle = DDS_HANDLE_NIL;
    readerViewCopyInInfo rData;
    u_result uResult;

    SAC_REPORT_STACK();

    if (instance_data == NULL) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Sample instance_data = NULL");
    } else {
        result = DDS_DataReaderViewClaim(_this, &view);
    }
    if (result == DDS_RETCODE_OK) {
        rData.reader = view->datareader;
        rData.data = (void *)instance_data;

        uResult = u_dataViewLookupInstance(
                      _DataReaderView_get_user_entity(view),
                      &rData,
                      _DataReaderViewCopyIn,
                      (u_instanceHandle *)&handle);
        result = DDS_ReturnCode_get(uResult);
        DDS_DataReaderViewRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return handle;
}


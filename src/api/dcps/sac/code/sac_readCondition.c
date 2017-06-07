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
#include "sac_common.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_condition.h"
#include "sac_readerCommon.h"
#include "sac_loanRegistry.h"
#include "sac_dataReader.h"
#include "sac_dataReaderView.h"
#include "u_object.h"
#include "u_reader.h"
#include "u_dataReader.h"
#include "u_dataView.h"
#include "u_query.h"
#include "v_dataView.h"
#include "v_dataReaderInstance.h"
#include "cmn_samplesList.h"
#include "cmn_reader.h"
#include "sac_report.h"

#define DDS_ReadConditionClaim(_this, condition) \
        DDS_Object_claim(DDS_Object(_this), DDS_READCONDITION, (_Object *)condition)

#define DDS_ReadConditionClaimRead(_this, condition) \
        DDS_Object_claim(DDS_Object(_this), DDS_READCONDITION, (_Object *)condition)

#define DDS_ReadConditionRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

#define DDS_ReadConditionCheck(_this, condition) \
        DDS_Object_check_and_assign(DDS_Object(_this), DDS_READCONDITION, (_Object *)condition)


static c_bool
test_sample_states (
    c_object o,
    c_voidp args)
{
    OS_UNUSED_ARG(o);
    OS_UNUSED_ARG(args);

    return TRUE; /* state evaluation is now in the kernel */
}

static DDS_ReturnCode_t
_ReadCondition_samples_flush_copy(
    _ReadCondition _this,
    DDS_Entity     source,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq)
{
    struct flushCopyArg arg;
    DDS_ReturnCode_t result;
    DDS_long length, testlength;
    DDS_LoanRegistry loanRegistry;
    u_entity uEntity = _Entity_get_user_entity(source);

    /* Use the reader common code to do the actual copying. */
    if (_this->sourceKind == U_READER) {
        arg.reader = _DataReader(source);
        loanRegistry = _DataReader(source)->loanRegistry;
    } else {
        arg.reader = _DataReader(_DataReaderView(source)->datareader);
        loanRegistry = _DataReaderView(source)->loanRegistry;
    }

    /* Prepare the buffers. */
    length = (DDS_long) cmn_samplesList_length(_this->samplesList);
    if (length > 0) {
        result = DDS_LoanRegistry_register(loanRegistry, data_seq, info_seq, length);
        assert(cmn_samplesList_length(_this->samplesList) <= info_seq->_maximum);

        if (result == DDS_RETCODE_OK) {
            arg.data_seq = data_seq;
            arg.info_seq = info_seq;
            arg.seqIndex = 0;
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

DDS_boolean
_ReadConditionGetTriggerValue(
    _Condition _this)
{
    DDS_boolean trigger = FALSE;

    if (_ReadCondition(_this)->uQuery != NULL) {
        trigger = u_queryTest(_ReadCondition(_this)->uQuery, test_sample_states, _this);
    }
    return trigger;
}

DDS_ReturnCode_t
_ReadCondition_deinit (
    _Object _this)
{
    DDS_ReturnCode_t result = DDS_RETCODE_BAD_PARAMETER;
    _ReadCondition rc;
    u_result uResult;

    rc = _ReadCondition(_this);
    if (rc != NULL) {
        uResult = u_objectFree_s(u_object(rc->uQuery));
        if (uResult == U_RESULT_OK) {
            rc->uQuery = NULL;
            result = _Condition_deinit(_this);
        } else {
            result = DDS_ReturnCode_get(uResult);
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "ReadCondition = NULL");
    }
    return result;
}

DDS_ReadCondition
DDS_ReadConditionNew (
    DDS_Entity source,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    u_kind kind;
    _ReadCondition _this = NULL;
    u_query uQuery;
    u_reader uReader;
    u_sampleMask mask;

    result = DDS_SAMPLE_MASK_IS_VALID(sample_states, view_states, instance_states);
    if (result != DDS_RETCODE_OK) {
        return NULL;
    }
    if (source != NULL) {
        result = DDS_Entity_get_user_entity(source, DDS_ENTITY, (u_entity *)&uReader);
        if (result != DDS_RETCODE_OK) {
            return NULL;
        }
        if (!u_entityEnabled(u_entity(uReader))) {
            SAC_REPORT(DDS_RETCODE_NOT_ENABLED, "DataReader not enabled");
            return NULL;
        }
        kind = u_objectKind(u_object(uReader));

        assert((kind == U_READER) || (kind == U_DATAVIEW));

        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uQuery = u_queryNew(uReader, NULL, "1=1", NULL, 0, mask);
        if (uQuery != NULL) {
            result = DDS_Object_new(DDS_READCONDITION, _ReadCondition_deinit, (_Object *)&_this);
            if (result == DDS_RETCODE_OK) {
                result = DDS_Condition_init(_this, NULL, _ReadConditionGetTriggerValue);
                DDS_Object_set_domain_id(_Object(_this), DDS_Object_get_domain_id(source));
            }
            if (result == DDS_RETCODE_OK) {
                _this->uQuery = uQuery;
                _this->sourceKind = kind;
                _this->source = source;
                _this->sample_states = sample_states;
                _this->view_states = view_states;
                _this->instance_states = instance_states;
/* TODO: samplesLists are retrieved by unchecked conversion!
 * implement a checked retrieval operation or reconsider the use of the samplesList.
 */
                switch (kind) {
                case U_READER: _this->samplesList = _DataReader(source)->samplesList; break;
                case U_DATAVIEW: _this->samplesList = _DataReaderView(source)->samplesList; break;
                default: assert(0);
                }
            }
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "DataReader is invalid");
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReader = NULL");
    }
    return (DDS_ReadCondition)_this;
}

u_query
DDS_ReadCondition_get_uQuery (
    DDS_ReadCondition _this)
{
    DDS_ReturnCode_t result;
    u_query uQuery = 0U;
    _ReadCondition rc;

    result = DDS_ReadConditionCheck(_this, &rc);
    if (result == DDS_RETCODE_OK) {
        uQuery = rc->uQuery;
    }
    return uQuery;
}

/*     SampleStateMask
 *     get_sample_state_mask();
 */
DDS_SampleStateMask
DDS_ReadCondition_get_sample_state_mask (
    DDS_ReadCondition _this)
{
    DDS_ReturnCode_t result;
    DDS_SampleStateMask mask = 0U;
    _ReadCondition rc;

    SAC_REPORT_STACK();

    result = DDS_ReadConditionCheck(_this, &rc);
    if (result == DDS_RETCODE_OK) {
        mask = rc->sample_states;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return mask;
}

/*     ViewStateMask
 *     get_view_state_mask();
 */
DDS_ViewStateMask
DDS_ReadCondition_get_view_state_mask (
    DDS_ReadCondition _this)
{
    DDS_ReturnCode_t result;
    DDS_ViewStateMask mask = 0U;
    _ReadCondition rc;

    SAC_REPORT_STACK();

    result = DDS_ReadConditionCheck(_this, &rc);
    if (result == DDS_RETCODE_OK) {
        mask = rc->view_states;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return mask;
}

/*     InstanceStateMask
 *     get_instance_state_mask();
 */
DDS_InstanceStateMask
DDS_ReadCondition_get_instance_state_mask (
    DDS_ReadCondition _this)
{
    DDS_ReturnCode_t result;
    DDS_InstanceStateMask mask = 0U;
    _ReadCondition rc;

    SAC_REPORT_STACK();

    result = DDS_ReadConditionCheck(_this, &rc);
    if (result == DDS_RETCODE_OK) {
        mask = rc->instance_states;
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return mask;
}

/*     DataReader
 *     get_datareader();
 */
DDS_DataReader
DDS_ReadCondition_get_datareader (
    DDS_ReadCondition _this)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;
    DDS_DataReader reader;

    SAC_REPORT_STACK();

    reader = NULL;
    result = DDS_ReadConditionCheck(_this, &rc);
    if (result == DDS_RETCODE_OK) {
        if (rc->sourceKind == U_READER) {
            reader = rc->source;
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return reader;
}

/*     DataReaderView
 *     get_datareaderview();
 */
DDS_DataReaderView
DDS_ReadCondition_get_datareaderview (
    DDS_ReadCondition _this)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;
    DDS_DataReaderView view;

    SAC_REPORT_STACK();

    view = NULL;
    result = DDS_ReadConditionCheck(_this, &rc);
    if (result == DDS_RETCODE_OK) {
        if (rc->sourceKind == U_DATAVIEW) {
            view = rc->source;
        }
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return view;
}

DDS_ReturnCode_t
DDS_ReadCondition_check(
    DDS_ReadCondition _this,
    const DDS_DataReader reader)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;

    rc = _ReadCondition(_this);
    if ((rc != NULL) && (reader != NULL)) {
        if (rc->source == reader ) {
            result = DDS_RETCODE_OK;
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "ReadCondition does not belong to DataReader");
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "DataReader = 0x%x, ReadCondition = 0x%x",
                    reader, rc);
    }
    return result;
}

DDS_ReturnCode_t
DDS_ReadCondition_read (
    DDS_ReadCondition _this,
    DDS_Entity source,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;
    u_result uResult;

    result = DDS_ReadConditionClaim(_this, &rc);
    if ( result == DDS_RETCODE_OK ) {
        if (rc->source != source) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "ReadCondition does not belong to DataReader");
            DDS_ReadConditionRelease(_this);
        } else {
            if (_ObjectKind(rc) == DDS_READCONDITION) {
                u_kind kind = rc->sourceKind;
                DDS_SampleStateMask sample_states = rc->sample_states;
                DDS_ViewStateMask view_states = rc->view_states;
                DDS_InstanceStateMask instance_states = rc->instance_states;
                DDS_ReadConditionRelease(_this);
                if (kind == U_READER) {
                    result = DDS_DataReader_read(
                                     source, data_seq, info_seq, max_samples,
                                     sample_states, view_states, instance_states);
                } else {
                    result = DDS_DataReaderView_read(
                                     source, data_seq, info_seq, max_samples,
                                     sample_states, view_states, instance_states);
                }
            } else {
                result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
                if (result == DDS_RETCODE_OK) {
                    cmn_samplesList_reset(rc->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
                    uResult = u_queryRead(rc->uQuery, cmn_reader_action, rc->samplesList, OS_DURATION_ZERO);
                    if (uResult == U_RESULT_OK) {
                        result = _ReadCondition_samples_flush_copy(rc, source, data_seq, info_seq);
                    } else {
                        result = DDS_ReturnCode_get(uResult);
                    }
                }
                DDS_ReadConditionRelease(_this);
            }
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_ReadCondition_take (
    DDS_ReadCondition _this,
    DDS_Entity source,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;
    u_result uResult;

    result = DDS_ReadConditionClaim(_this, &rc);
    if ( result == DDS_RETCODE_OK ) {
        if (rc->source != source) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "ReadCondition does not belong to DataReader");
            DDS_ReadConditionRelease(_this);
        } else {
            if (_ObjectKind(rc) == DDS_READCONDITION) {
                u_kind kind = rc->sourceKind;
                DDS_SampleStateMask sample_states = rc->sample_states;
                DDS_ViewStateMask view_states = rc->view_states;
                DDS_InstanceStateMask instance_states = rc->instance_states;
                DDS_ReadConditionRelease(_this);
                if (kind == U_READER) {
                    result = DDS_DataReader_take(
                                     source, data_seq, info_seq, max_samples,
                                     sample_states, view_states, instance_states);
                } else {
                    result = DDS_DataReaderView_take(
                                     source, data_seq, info_seq, max_samples,
                                     sample_states, view_states, instance_states);
                }
            } else {
                result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
                if (result == DDS_RETCODE_OK) {
                    cmn_samplesList_reset(rc->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
                    uResult = u_queryTake(rc->uQuery, cmn_reader_action, rc->samplesList, OS_DURATION_ZERO);
                    if (uResult == U_RESULT_OK) {
                        result = _ReadCondition_samples_flush_copy(rc, source, data_seq, info_seq);
                    } else {
                        result = DDS_ReturnCode_get(uResult);
                    }
                }
                DDS_ReadConditionRelease(_this);
            }
        }
    }
    return result;
}

DDS_ReturnCode_t
DDS_ReadCondition_read_next_instance (
    DDS_ReadCondition _this,
    DDS_Entity source,
    const DDS_InstanceHandle_t a_handle,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;
    u_result uResult;
    u_sampleMask mask;

    result = DDS_ReadConditionClaim(_this, &rc);
    if ( result == DDS_RETCODE_OK ) {
        if (rc->source != source) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "ReadCondition does not belong to DataReader");
        }
        if ( result == DDS_RETCODE_OK ) {
            result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
            if (result == DDS_RETCODE_OK) {
                cmn_samplesList_reset(rc->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
                if (_ObjectKind(rc) == DDS_READCONDITION) {
                    mask = DDS_SAMPLE_MASK(rc->sample_states, rc->view_states, rc->instance_states);
                    if (rc->sourceKind == U_READER) {
                        result = DDS_DataReader_read_next_instance_internal(source, a_handle, mask, rc->samplesList);
                    } else {
                        result = DDS_DataReaderView_read_next_instance_internal(source, a_handle, mask, rc->samplesList);
                    }
                } else {
                    uResult = u_queryReadNextInstance(rc->uQuery, a_handle, cmn_reader_action, rc->samplesList, OS_DURATION_ZERO);
                    if (uResult != U_RESULT_HANDLE_EXPIRED) {
                        result = DDS_ReturnCode_get(uResult);
                    } else {
                        result = DDS_RETCODE_HANDLE_EXPIRED;
                    }
                }
                if (result == DDS_RETCODE_OK) {
                    result = _ReadCondition_samples_flush_copy(rc, source, data_seq, info_seq);
                }
            }
        }
        DDS_ReadConditionRelease(_this);
    }

    return result;
}


DDS_ReturnCode_t
DDS_ReadCondition_take_next_instance (
    DDS_ReadCondition _this,
    DDS_Entity source,
    const DDS_InstanceHandle_t a_handle,
    _DDS_sequence data_seq,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;
    u_result uResult;
    u_sampleMask mask;

    result = DDS_ReadConditionClaim(_this, &rc);
    if ( result == DDS_RETCODE_OK ) {
        if (rc->source != source) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "ReadCondition does not belong to DataReader");
        }
        if ( result == DDS_RETCODE_OK ) {
            result = DDS_ReaderCommon_check_read_args(data_seq, info_seq, max_samples);
            if (result == DDS_RETCODE_OK) {
                cmn_samplesList_reset(rc->samplesList, REAL_MAX_SAMPLES(max_samples, info_seq));
                if (_ObjectKind(rc) == DDS_READCONDITION) {
                    mask = DDS_SAMPLE_MASK(rc->sample_states, rc->view_states, rc->instance_states);
                    if (rc->sourceKind == U_READER) {
                        result = DDS_DataReader_take_next_instance_internal(source, a_handle, mask, rc->samplesList);
                    } else {
                        result = DDS_DataReaderView_take_next_instance_internal(source, a_handle, mask, rc->samplesList);
                    }
                } else {
                    uResult = u_queryTakeNextInstance(rc->uQuery, a_handle, cmn_reader_action, rc->samplesList, OS_DURATION_ZERO);
                    if (uResult != U_RESULT_HANDLE_EXPIRED) {
                        result = DDS_ReturnCode_get(uResult);
                    } else {
                        result = DDS_RETCODE_HANDLE_EXPIRED;
                    }
                }
                if (result == DDS_RETCODE_OK) {
                    result = _ReadCondition_samples_flush_copy(rc, source, data_seq, info_seq);
                }
            }
        }
        DDS_ReadConditionRelease(_this);
    }

    return result;
}




DDS_ReturnCode_t
DDS_ReadCondition_get_settings(
     DDS_ReadCondition _this,
     DDS_Entity source,
     u_object *obj,
     os_uint32 *mask)
{
    DDS_ReturnCode_t result;
    _ReadCondition rc;

    if (!obj) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "The obj parameter is NULL");
    } else if (!mask) {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "The mask parameter is NULL");
    } else {
        result = DDS_ReadConditionClaim(_this, &rc);
    }

    if ( result == DDS_RETCODE_OK ) {
        if (rc->source != source) {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
            SAC_REPORT(result, "ReadCondition does not belong to DataReader");
        } else {
            if (_ObjectKind(rc) == DDS_READCONDITION) {
                *obj = u_object(_Entity_get_user_entity(_Entity(source)));
            } else {
                *obj = u_object(rc->uQuery);
            }
            *mask = DDS_SAMPLE_MASK(rc->sample_states, rc->view_states, rc->instance_states);
        }
        DDS_ReadConditionRelease(_this);
    }

    return result;
}



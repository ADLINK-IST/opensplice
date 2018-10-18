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
#include "sac_common.h"
#include "sac_object.h"
#include "sac_entity.h"
#include "sac_condition.h"
#include "sac_readCondition.h"
#include "sac_objManag.h"
#include "u_object.h"
#include "u_reader.h"
#include "u_dataView.h"
#include "u_query.h"
#include "sac_report.h"


#define DDS_QueryConditionClaim(_this, reader) \
        DDS_Object_claim(DDS_Object(_this), DDS_QUERYCONDITION, (_Object *)reader)

#define DDS_QueryConditionClaimRead(_this, reader) \
        DDS_Object_claim(DDS_Object(_this), DDS_QUERYCONDITION, (_Object *)reader)

#define DDS_QueryConditionRelease(_this) \
        DDS_Object_release(DDS_Object(_this))

static DDS_ReturnCode_t
_QueryCondition_deinit (
    _Object _this)
{
    _QueryCondition qc = (_QueryCondition)_this;

    if (qc->query_parameters) {
        DDS_free(qc->query_parameters);
        qc->query_parameters = NULL;
    }
    if (qc->query_expression) {
        DDS_free(qc->query_expression);
        qc->query_expression = NULL;
    }

    return _ReadCondition_deinit(_this);
}

DDS_QueryCondition
DDS_QueryConditionNew (
    DDS_Entity source,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states,
    const DDS_char *query_expression,
    const DDS_StringSeq *query_parameters)
{
    DDS_ReturnCode_t result;
    _QueryCondition _this;
    u_query uQuery;
    u_reader uReader;
    u_kind kind;
    DDS_long length;
    const os_char **params;
    u_sampleMask mask;

    _this = NULL;
    if ((source != NULL) && (query_expression != NULL)) {
        if ((query_parameters != NULL) && !(DDS_StringSeq_is_valid(query_parameters)))
        {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Invalid query_parameters");
        } else {
            result = DDS_RETCODE_OK;
        }
    } else {
        result = DDS_RETCODE_BAD_PARAMETER;
        SAC_REPORT(result, "Query source = 0x%x, query_expression = 0x%x",
                    source, query_expression);
    }
    if (result == DDS_RETCODE_OK) {
        if (query_parameters != NULL) {
            length = query_parameters->_length;
            params = (const os_char **)query_parameters->_buffer;
        } else {
            length = 0;
            params = NULL;
        }
        uReader = u_reader(_Entity_get_user_entity(_Entity(source)));
        /* If uReader == NULL, then it's already deleted. */
        if (uReader == NULL) {
            SAC_REPORT(DDS_RETCODE_ALREADY_DELETED, "DataReader already deleted");
            return NULL;
        }
        if (!u_entityEnabled(u_entity(uReader))) {
            SAC_REPORT(DDS_RETCODE_NOT_ENABLED, "DataReader not enabled");
            return NULL;
        }
        kind = u_objectKind(u_object(uReader));

        assert((kind == U_READER) || (kind == U_DATAVIEW));

        mask = DDS_SAMPLE_MASK(sample_states, view_states, instance_states);
        uQuery = u_queryNew(uReader, NULL, query_expression, params, length, mask);
        if (uQuery != NULL) {
            result = DDS_Object_new(DDS_QUERYCONDITION,
                                    _QueryCondition_deinit,
                                    (_Object *)&_this);
            if (result == DDS_RETCODE_OK) {
                result = DDS_Condition_init(_this, NULL,
                                            _ReadConditionGetTriggerValue);
                DDS_Object_set_domain_id(_Object(_this), DDS_Object_get_domain_id(source));

            }
            if (result == DDS_RETCODE_OK) {
/* TODO: following lines should be a performed by a_ReadCondition_init operation.
 */
                _ReadCondition(_this)->uQuery = uQuery;
                _ReadCondition(_this)->source = source;
                _ReadCondition(_this)->sample_states = sample_states;
                _ReadCondition(_this)->view_states = view_states;
                _ReadCondition(_this)->instance_states = instance_states;
                _ReadCondition(_this)->sourceKind = kind;
/* TODO: samplesLists are retrieved by unchecked conversion!
 * implement a checked retrieval operation or reconsider the use of the samplesList.
 */
                switch (kind) {
                case U_READER: _ReadCondition(_this)->samplesList = _DataReader(source)->samplesList; break;
                case U_DATAVIEW: _ReadCondition(_this)->samplesList = _DataReaderView(source)->samplesList; break;
                default: assert(0);
                }
                _this->query_expression = DDS_string_dup(query_expression);
                _this->query_parameters = DDS_StringSeq_dup(query_parameters);
            }
        } else {
            result = DDS_RETCODE_ERROR;
        }
    }
    return (DDS_QueryCondition)_this;
}

/*     string
 *     get_query_expression();
 */
DDS_string
DDS_QueryCondition_get_query_expression (
    DDS_QueryCondition _this)
{
    DDS_ReturnCode_t result;
    DDS_string expr = NULL;
    _QueryCondition qc;

    SAC_REPORT_STACK();

    result = DDS_QueryConditionClaimRead(_this, &qc);
    if (result == DDS_RETCODE_OK) {
        expr = DDS_string_dup(qc->query_expression);
        DDS_QueryConditionRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return expr;
}

/*     ReturnCode_t
 *     get_query_parameters(
 *         inout StringSeq query_arguments);
 */
DDS_ReturnCode_t
DDS_QueryCondition_get_query_parameters (
    DDS_QueryCondition _this,
    DDS_StringSeq *query_parameters)
{
    DDS_ReturnCode_t result;
    _QueryCondition qc;

    SAC_REPORT_STACK();

    result = DDS_QueryConditionClaimRead(_this, &qc);
    if (result == DDS_RETCODE_OK) {
        DDS_sequence_clean((_DDS_sequence) query_parameters);
        result = DDS_StringSeq_init(query_parameters, qc->query_parameters);
        DDS_QueryConditionRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}

/*     ReturnCode_t
 *     set_query_parameters(
 *         in StringSeq query_arguments);
 */
DDS_ReturnCode_t
DDS_QueryCondition_set_query_parameters (
    DDS_QueryCondition _this,
    const DDS_StringSeq *query_parameters)
{
    DDS_ReturnCode_t result;
    _QueryCondition qc;
    DDS_long length;
    u_result uResult;
    const os_char **params;

    SAC_REPORT_STACK();

    result = DDS_QueryConditionClaim(_this, &qc);
    if (result == DDS_RETCODE_OK) {
        if (DDS_StringSeq_is_valid(query_parameters)) {
            if (query_parameters != NULL) {
                length = query_parameters->_length;
                params = (const os_char **)query_parameters->_buffer;
            } else {
                length = 0;
                params = NULL;
            }
            if (result == DDS_RETCODE_OK) {
                uResult = u_querySet(_ReadCondition(qc)->uQuery, params, length);
                result = DDS_ReturnCode_get(uResult);
            }
            if (result == DDS_RETCODE_OK) {
                /* Store new params for later retrieval */
                DDS_free(qc->query_parameters);
                qc->query_parameters = DDS_StringSeq_dup(query_parameters);
            }
        } else {
            result = DDS_RETCODE_BAD_PARAMETER;
            SAC_REPORT(result, "Invalid query parameter string sequence");
        }
        DDS_QueryConditionRelease(_this);
    }
    SAC_REPORT_FLUSH(_this, result != DDS_RETCODE_OK);
    return result;
}


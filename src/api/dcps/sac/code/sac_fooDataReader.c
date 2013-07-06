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

#include "dds_dcps.h"
#include "dds_dcps_private.h"
#include "sac_fooDataReader.h"
#include "sac_common.h"

static DDS_boolean
sequenceIsValid (
    void *_seq)
{
    _Sequence    *seq   = (_Sequence *) _seq;

    if ( seq == NULL ) {
        return FALSE;
    }

    if ( seq->_maximum > 0 && seq->_buffer == NULL ) {
        return FALSE;
    }

    if ( seq->_maximum == 0 && seq->_buffer != NULL ) {
        return FALSE;
    }

    if ( seq->_length > seq->_maximum ) {
        return FALSE;
    }

    return TRUE;
}

static DDS_boolean
checkParameters (
    DDS_long          *maxSamples_inout,
    DDS_sequence      *_dataSeq,
    DDS_SampleInfoSeq *infoSeq,
    DDS_ReturnCode_t  *retval)
{
    _Sequence *dataSeq = (_Sequence*) _dataSeq;
    DDS_long maxSamples;

    assert (maxSamples_inout);

    maxSamples = *maxSamples_inout;

    if ( !sequenceIsValid(dataSeq) ||
         !sequenceIsValid(infoSeq) ||
         (maxSamples < -1) ) {
        *retval = DDS_RETCODE_BAD_PARAMETER;
        return FALSE;
    }

    /* Rule 1 : Both sequences must have equal len,
     * max_len and owns properties.
     */
    if ( (dataSeq->_length  != infoSeq->_length)  ||
         (dataSeq->_maximum != infoSeq->_maximum) ||
         (dataSeq->_release != infoSeq->_release) ) {
        *retval = DDS_RETCODE_PRECONDITION_NOT_MET;
        return FALSE;
    }

    /* Rule 4: When max_len > 0, then own must be true.
     */
    if ( (infoSeq->_maximum > 0) && (!infoSeq->_release) ) {
        *retval = DDS_RETCODE_PRECONDITION_NOT_MET;
        return FALSE;
    }

    /* Rule 5: when max_samples != LENGTH_UNLIMITED,
     * then the following condition needs to be met:
     * maxSamples <= max_len
     */
    if ( (infoSeq->_maximum > 0) &&
         (((DDS_unsigned_long)maxSamples) > infoSeq->_maximum) &&
         (maxSamples != DDS_LENGTH_UNLIMITED) ) {
        *retval = DDS_RETCODE_PRECONDITION_NOT_MET;
        return FALSE;
    }

    /* If length is unlimited, but release is true, maxSamples equals the
     * maximum for the sequence (scdds2032).
     */
    else if ((maxSamples == DDS_LENGTH_UNLIMITED) && infoSeq->_maximum > 0) {
    	maxSamples = infoSeq->_maximum;
    }

    /* In all other cases, the provided sequences are valid.
     */
    *retval = DDS_RETCODE_OK;
    *maxSamples_inout = maxSamples;
    return TRUE;
}

DDS_ReturnCode_t
DDS__FooDataReader_read (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_read (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_take (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_take (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_read_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_read_w_condition (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_take_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_take_w_condition (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_read_next_sample (
    DDS_DataReader _this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info)
{
    return (DDS_ReturnCode_t)
        gapi_fooDataReader_read_next_sample (
            (gapi_dataReader)_this,
            (gapi_foo *)data_values,
            (gapi_sampleInfo *)sample_info
        );
}

DDS_ReturnCode_t
DDS__FooDataReader_take_next_sample (
    DDS_DataReader _this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info)
{
    return (DDS_ReturnCode_t)
        gapi_fooDataReader_take_next_sample (
            (gapi_dataReader)_this,
            (gapi_foo *)data_values,
            (gapi_sampleInfo *)sample_info
        );
}

DDS_ReturnCode_t
DDS__FooDataReader_read_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_read_instance (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_instanceHandle_t)a_handle,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_take_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_take_instance (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_instanceHandle_t)a_handle,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_read_next_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_read_next_instance (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_instanceHandle_t)a_handle,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_take_next_instance (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_take_next_instance (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_instanceHandle_t)a_handle,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_read_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_read_next_instance_w_condition (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_instanceHandle_t)a_handle,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_take_next_instance_w_condition (
    DDS_DataReader _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition)
{
    DDS_ReturnCode_t result;
    DDS_long realMax = max_samples;

    if ( checkParameters(&realMax, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReader_take_next_instance_w_condition (
                (gapi_dataReader)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)realMax,
                (gapi_instanceHandle_t)a_handle,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_return_loan (
    DDS_DataReader _this,
    DDS_sequence _data_seq,
    DDS_SampleInfoSeq *info_seq)
{
    DDS_ReturnCode_t result;
    _Sequence *data_seq = (_Sequence *)_data_seq;

    if (data_seq->_release == info_seq->_release) {
        if (!data_seq->_release) {
            if ( !sequenceIsValid(data_seq) || !sequenceIsValid(info_seq) ) {
                result = DDS_RETCODE_BAD_PARAMETER;
            } else {
                result = (DDS_ReturnCode_t)
                    gapi_fooDataReader_return_loan (
                        (gapi_dataReader)_this,
                        (void *)data_seq->_buffer,
                        (void *)info_seq->_buffer);
                if ( result == DDS_RETCODE_OK ) {
                    DDS__free(data_seq->_buffer);
                    data_seq->_length  = 0;
                    data_seq->_maximum = 0;
                    data_seq->_buffer  = NULL;

                    DDS__free(info_seq->_buffer);
                    info_seq->_length  = 0;
                    info_seq->_maximum = 0;
                    info_seq->_buffer  = NULL;
                }
            }
        } else {
            result = DDS_RETCODE_OK;
        }
    } else {
        result = DDS_RETCODE_PRECONDITION_NOT_MET;
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReader_get_key_value (
    DDS_DataReader _this,
    DDS_sample key_holder,
    const DDS_InstanceHandle_t handle)
{
    return (DDS_ReturnCode_t)
        gapi_fooDataReader_get_key_value (
            (gapi_dataReader)_this,
            (gapi_foo *)key_holder,
            (gapi_instanceHandle_t)handle
        );
}

DDS_InstanceHandle_t
DDS__FooDataReader_lookup_instance (
    DDS_DataReader _this,
    DDS_sample instance_data)
{
    return (DDS_InstanceHandle_t)
        gapi_fooDataReader_lookup_instance (
            (gapi_dataReader)_this,
            (gapi_foo *)instance_data
        );
}

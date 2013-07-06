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
#include "sac_common.h"

static DDS_boolean
sequenceIsValid (
    void *_seq
    )
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
    DDS_long           maxSamples,
    DDS_sequence      *_dataSeq,
    DDS_SampleInfoSeq *infoSeq,
    DDS_ReturnCode_t  *retval
    )
{
    _Sequence *dataSeq = (_Sequence*) _dataSeq;
    
    if ( !sequenceIsValid(dataSeq) || !sequenceIsValid(infoSeq) || (maxSamples < -1) ) {
        *retval = DDS_RETCODE_BAD_PARAMETER;
        return FALSE;
    }
    
    /* Rule 1 : Both sequences must have equal len, max_len and owns properties.*/
    if ( (dataSeq->_length  != infoSeq->_length)  ||
         (dataSeq->_maximum != infoSeq->_maximum) ||
         (dataSeq->_release != infoSeq->_release) ) {
        *retval = DDS_RETCODE_PRECONDITION_NOT_MET;
        return FALSE;
    }
    
    /* Rule 4: When max_len > 0, then own must be true.*/
    if ( (infoSeq->_maximum > 0) && (!infoSeq->_release) ) {
        *retval = DDS_RETCODE_PRECONDITION_NOT_MET;
        return FALSE;
    }

    /* Rule 5: when max_samples != LENGTH_UNLIMITED, then the following condition 
    // needs to be met: maxSamples <= max_len  */
    if ( (infoSeq->_maximum > 0) &&
         (((DDS_unsigned_long)maxSamples) > infoSeq->_maximum) && 
         (maxSamples != DDS_LENGTH_UNLIMITED) ) {
        *retval = DDS_RETCODE_PRECONDITION_NOT_MET;
        return FALSE;
    }
    
    /* In all other cases, the provided sequences are valid.*/
    *retval = DDS_RETCODE_OK;
    return TRUE;
}

DDS_ReturnCode_t
DDS__FooDataReaderView_read (
    DDS_DataReaderView this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_read (
                (gapi_dataReaderView)this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReaderView_take (
    DDS_DataReaderView this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_take (
                (gapi_dataReaderView)this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
                (gapi_sampleStateMask)sample_states,
                (gapi_viewStateMask)view_states,
                (gapi_instanceStateMask)instance_states
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReaderView_read_next_sample (
    DDS_DataReaderView this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info
    )
{
    return (DDS_ReturnCode_t)
        gapi_fooDataReaderView_read_next_sample (
            (gapi_dataReaderView)this,
            (gapi_foo *)data_values,
            (gapi_sampleInfo *)sample_info
        );
}

DDS_ReturnCode_t
DDS__FooDataReaderView_take_next_sample (
    DDS_DataReaderView this,
    DDS_sample data_values,
    DDS_SampleInfo *sample_info
    )
{
    return (DDS_ReturnCode_t)
        gapi_fooDataReaderView_take_next_sample (
            (gapi_dataReaderView)this,
            (gapi_foo *)data_values,
            (gapi_sampleInfo *)sample_info
        );
}

DDS_ReturnCode_t
DDS__FooDataReaderView_read_instance (
    DDS_DataReaderView this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_read_instance (
                (gapi_dataReaderView)this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
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
DDS__FooDataReaderView_take_instance (
    DDS_DataReaderView this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_take_instance (
                (gapi_dataReaderView)this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
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
DDS__FooDataReaderView_read_next_instance (
    DDS_DataReaderView this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_read_next_instance (
                (gapi_dataReaderView)this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
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
DDS__FooDataReaderView_take_next_instance (
    DDS_DataReaderView this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_SampleStateMask sample_states,
    const DDS_ViewStateMask view_states,
    const DDS_InstanceStateMask instance_states
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_take_next_instance (
                (gapi_dataReaderView)this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
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
DDS__FooDataReaderView_return_loan (
    DDS_DataReaderView     this,
    DDS_sequence      _data_seq,
    DDS_SampleInfoSeq *info_seq
    )
{
    DDS_ReturnCode_t result;
    _Sequence *data_seq = (_Sequence *)_data_seq;
    
    if ( !sequenceIsValid(data_seq) || !sequenceIsValid(info_seq) ) {
        result = DDS_RETCODE_BAD_PARAMETER;
    } else {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_return_loan (
                (gapi_dataReaderView)this,
                (void *)data_seq->_buffer,
                (void *)info_seq->_buffer);
    }
    
    if ( result == DDS_RETCODE_OK ) {
            DDS__free(data_seq->_buffer);
            data_seq->_length  = 0;
            data_seq->_maximum = 0;
            data_seq->_buffer  = NULL;
            
            DDS__free(info_seq->_buffer);
            info_seq->_length  = 0;
            info_seq->_maximum = 0;
            info_seq->_buffer  = NULL;
    } else if ( result == DDS_RETCODE_NO_DATA ) {
        if ( data_seq->_release ) {
            result = DDS_RETCODE_OK;
        } else {
            result = DDS_RETCODE_PRECONDITION_NOT_MET;
        }
    }

    return result;
}

/* ReturnCode_t
 * read_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS__FooDataReaderView_read_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_read_w_condition (
                (gapi_dataReaderView)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

/* ReturnCode_t
 * take_w_condition(
 *     inout DataSeq data_values,
 *     inout SampleInfoSeq info_seq,
 *     in long max_samples,
 *     in ReadCondition a_condition);
 */
DDS_ReturnCode_t
DDS__FooDataReaderView_take_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_take_w_condition (
                (gapi_dataReaderView)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReaderView_read_next_instance_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_read_next_instance_w_condition (
                (gapi_dataReaderView)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
                (gapi_instanceHandle_t)a_handle,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}

DDS_ReturnCode_t
DDS__FooDataReaderView_take_next_instance_w_condition (
    DDS_DataReaderView _this,
    DDS_sequence data_values,
    DDS_SampleInfoSeq *info_seq,
    const DDS_long max_samples,
    const DDS_InstanceHandle_t a_handle,
    const DDS_ReadCondition a_condition
    )
{
    DDS_ReturnCode_t result;

    if ( checkParameters(max_samples, data_values, info_seq, &result) ) {
        result = (DDS_ReturnCode_t)
            gapi_fooDataReaderView_take_next_instance_w_condition (
                (gapi_dataReaderView)_this,
                (gapi_fooSeq *)data_values,
                (gapi_sampleInfoSeq *)info_seq,
                (gapi_long)max_samples,
                (gapi_instanceHandle_t)a_handle,
                (gapi_readCondition)a_condition
            );
        CHECK_NO_DATA(result, data_values, info_seq);
    }
    return result;
}


DDS_ReturnCode_t
DDS__FooDataReaderView_get_key_value (
    DDS_DataReaderView _this,
    DDS_sample key_holder,
    const DDS_InstanceHandle_t handle
    )
{
    return (DDS_ReturnCode_t)
        gapi_fooDataReaderView_get_key_value (
            (gapi_dataReaderView)_this,
            (gapi_foo *)key_holder,
            (gapi_instanceHandle_t)handle
        );
}

DDS_InstanceHandle_t
DDS__FooDataReaderView_lookup_instance (
    DDS_DataReaderView _this,
    DDS_sample instance_data
    )
{
    return (DDS_InstanceHandle_t)
        gapi_fooDataReaderView_lookup_instance (
            (gapi_dataReaderView)_this,
            (gapi_foo *)instance_data
        );
}       


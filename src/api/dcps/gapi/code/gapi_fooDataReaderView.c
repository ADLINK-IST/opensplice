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
#include "gapi_fooDataReaderView.h"
#include "gapi_dataReaderView.h"
#include "gapi_dataReader.h"
#include "gapi_fooDataReader.h"
#include "gapi_qos.h"
#include "gapi_kernel.h"
#include "gapi_objManag.h"
#include "gapi_structured.h"
#include "gapi_genericCopyOut.h"
#include "gapi_genericCopyIn.h"

#include "os_heap.h"
#include "u_user.h"
#include "u_handle.h"
#include "u_instanceHandle.h"
#include "u_dataView.h"
#include "v_kernel.h"
#include "v_state.h"
#include "v_public.h"
#include "kernelModule.h"
#include "v_dataReader.h"
#include "v_dataViewSample.h"

#define V_SAMPLESEQ_INCREMENT 128
#define MAX_DATASAMPLESEQ_SIZE_ON_STACK  16

C_STRUCT(_FooDataReaderView) {
    C_EXTENDS(_DataReaderView);
};

typedef struct readerViewActionArg_s {
    _Status             readerStatus;
    gapi_unsigned_long  max;
    _DataReaderView     datareaderview;
    gapi_readerInfo    *readerInfo;
    gapi_readerCopy     readerCopy;
    v_readerSampleSeq  *samples;
    gapi_returnCode_t   result;
} readerViewActionArg;


static gapi_boolean
statemasks_unsupported (
     const gapi_sampleStateMask sample_states,
     const gapi_viewStateMask view_states,
     const gapi_instanceStateMask instance_states)
{

    if ( sample_states != GAPI_ANY_SAMPLE_STATE ) {
        return TRUE;
    }
    if ( view_states != GAPI_ANY_VIEW_STATE ) {
        return TRUE;
    }
    if ( instance_states != GAPI_ANY_INSTANCE_STATE ) {
        return TRUE;
    }
    return FALSE;
}

static gapi_boolean
sampleSeqContains (
    v_readerSampleSeq *samples,
    v_readerSample     sample)
{
    gapi_unsigned_long i;
    gapi_boolean       found = FALSE;

    for ( i = 0; !found && (i < samples->_length); i++ ) {
        if ( sample == samples->_buffer[i] ) {
            found = TRUE;
        }
    }

    return found;
}

static void
copySampleInfoView (
    v_readerSample sample,
    v_message message,
    gapi_sampleInfo *to
    )
{
    v_state              state;
    v_dataReaderSample   master;
    v_dataViewInstance   viewInstance;
    v_dataReaderInstance masterInstance;

    viewInstance = (v_dataViewInstance)sample->instance;

    master = v_dataReaderSample(v_dataViewSampleTemplate(sample)->sample);
    masterInstance = (v_dataReaderInstance)v_readerSampleInstance(master);

    state = v_readerSample(sample)->sampleState;
    if (v_stateTest (state, L_READ)) {
        to->sample_state = GAPI_READ_SAMPLE_STATE;
    } else {
        to->sample_state = GAPI_NOT_READ_SAMPLE_STATE;
    }

    if (v_stateTest (state, L_NEW)) {
        to->view_state = GAPI_NEW_VIEW_STATE;
    } else {
        to->view_state = GAPI_NOT_NEW_VIEW_STATE;
    }

    state = masterInstance->instanceState;
    to->instance_state = GAPI_ALIVE_INSTANCE_STATE;
    if (v_stateTest (state, L_NOWRITERS)) {
        to->instance_state = GAPI_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
    }
    if (v_stateTest (state, L_DISPOSED)) {
        to->instance_state = GAPI_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
    }

    /* Data is always valid for views */
    to->valid_data = TRUE;
    to->source_timestamp.sec        = (gapi_long)(message->writeTime.seconds);
    to->source_timestamp.nanosec    = (gapi_unsigned_long)(message->writeTime.nanoseconds);

    to->disposed_generation_count   = master->disposeCount;
    to->no_writers_generation_count = master->noWritersCount;
    to->sample_rank                 = 0;
    to->generation_rank             = 0;
    to->absolute_generation_rank    = 0;
    to->instance_handle             = u_instanceHandleNew(v_public(viewInstance));

    to->publication_handle          = u_instanceHandleFromGID(master->publicationHandle);

    to->reception_timestamp.sec       = (gapi_long)(master->insertTime.seconds);
    to->reception_timestamp.nanosec   = (gapi_unsigned_long)(master->insertTime.nanoseconds);
}

static void
computeGenerationRanksView (
    v_readerSampleSeq  *samples,
    gapi_dataSampleSeq *dataSamples)
{
    gapi_unsigned_long first = 0;

    while ( first < samples->_length ) {
        gapi_unsigned_long i;
        gapi_unsigned_long next = first + 1;
        gapi_unsigned_long last = first;
        gapi_boolean       found = FALSE;
        v_readerSample     fs;
        c_voidp            instance;
        gapi_viewStateKind vs;

        fs = samples->_buffer[first];
        instance = fs->instance;

        while ( !found && (next < samples->_length) ) {
            v_readerSample s = samples->_buffer[next];
            if ( instance != s->instance ) {
                found = TRUE;
            } else {
                last = next++;
            }
        }

        if ( v_stateTest(fs->sampleState, L_NEW) ) {
            vs = GAPI_NEW_VIEW_STATE;
        } else {
            vs = GAPI_NOT_NEW_VIEW_STATE;
        }

        for ( i = first; i <= last; i++ ) {
            dataSamples->_buffer[i].info.sample_rank = last - i;
            dataSamples->_buffer[i].info.generation_rank = 0;
            dataSamples->_buffer[i].info.absolute_generation_rank = 0;
            /* copy new state of last sample of an instance */
            dataSamples->_buffer[i].info.view_state = vs;
        }

        first = last + 1;
    }
}

static void
determineSampleInfoView (
     readerViewActionArg *info)
{
    gapi_unsigned_long  i;
    v_readerSampleSeq  *samples = info->samples;
    gapi_unsigned_long  length;
    gapi_dataSampleSeq  dataSamples;
    gapi_dataSample     onePlaceBuffer;
    gapi_boolean        onHeap  = FALSE;
    gapi_boolean        onStack = FALSE;

    assert(samples->_length > 0);

    length = samples->_length;

    if ( length == 1 ) {
        dataSamples._buffer  = &onePlaceBuffer;
    } else if ( length < MAX_DATASAMPLESEQ_SIZE_ON_STACK ) {
        dataSamples._buffer = (gapi_dataSample *)os_alloca(length*sizeof(gapi_dataSample));
        if ( dataSamples._buffer ) {
            onStack = TRUE;
        } else {
            dataSamples._buffer = (gapi_dataSample *)os_malloc(length*sizeof(gapi_dataSample));
            onHeap = TRUE;
        }
    } else {
        dataSamples._buffer = (gapi_dataSample *)os_malloc(length*sizeof(gapi_dataSample));
        onHeap = TRUE;
    }

    if ( dataSamples._buffer ) {
        dataSamples._length  = length;
        dataSamples._maximum = length;
        dataSamples._release = FALSE;

        for ( i = 0; i < length; i++ ) {
            v_dataViewSampleTemplate viewSample = v_dataViewSampleTemplate(samples->_buffer[i]);
            v_dataReaderSampleTemplate sample = v_dataReaderSampleTemplate(viewSample->sample);
            v_message message = sample->message;

            dataSamples._buffer[i].data = (void *)C_DISPLACE(message,
                    info->datareaderview->datareader->userdataOffset);
            copySampleInfoView(v_readerSample(viewSample), message, &dataSamples._buffer[i].info);
        }

        computeGenerationRanksView(samples, &dataSamples);
        info->readerCopy(&dataSamples, info->readerInfo);

        if ( onStack ) {
            os_freea(dataSamples._buffer);
        } else {
            if ( onHeap ) {
                os_free(dataSamples._buffer);
            }
        }

    } else {
        info->result = GAPI_RETCODE_OUT_OF_RESOURCES;
    }

}

static v_actionResult
readerActionView (
    c_object o,
    c_voidp copyArg)
{
    readerViewActionArg    *info    = (readerViewActionArg *) copyArg;
    v_readerSampleSeq  *samples = info->samples;
    v_readerSample      sample;
    v_actionResult      result  = V_PROCEED;
    gapi_unsigned_long  i;

    if ( o ) {
        sample = v_readerSample(o);
        if (gapi_matchesReaderMask(o, &info->datareaderview->reader_mask)) {
            if ( !sampleSeqContains(samples, sample) ) {
                i = samples->_length;

                if ( v_readerSampleSeq_setLength(samples, i+1) ) {
                    samples->_buffer[i] = c_keep(sample);
                    c_keep(sample->instance);
                } else {
                    info->result = GAPI_RETCODE_OUT_OF_RESOURCES;
                    v_actionResultClear(result, V_PROCEED);
                }

                if ( samples->_length >= info->max ) {
                    v_actionResultClear(result, V_PROCEED);
                }
            }
        } else {
            v_actionResultSet(result, V_SKIP);
        }
    } else {
        if ( samples->_length > 0 ) {
            determineSampleInfoView(info);
            for ( i = 0UL; i < samples->_length; i++ ) {
                c_free(samples->_buffer[i]->instance);
                c_free(samples->_buffer[i]);
            }
        } else {
            info->readerCopy(NULL, info->readerInfo);
            info->result = GAPI_RETCODE_NO_DATA;
        }
        v_actionResultClear(result, V_PROCEED);
    }

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_read (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReaderView     datareaderview;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    datareaderview = gapi_dataReaderViewClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareaderview == NULL ) {
        return result;
    } else if (!gapi_stateMasksValid(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (statemasks_unsupported(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_UNSUPPORTED;
    } else if (max_samples == 0) {
        result = GAPI_RETCODE_NO_DATA;
    } else {
        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        reader = u_reader(U_DATAREADERVIEW_GET(datareaderview));
        datareaderview->reader_mask.sampleStateMask = sample_states;
        datareaderview->reader_mask.viewStateMask = view_states;
        datareaderview->reader_mask.instanceStateMask = instance_states;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerRead(reader,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    }
    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_take (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReaderView     datareaderview;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);

    if (datareaderview == NULL) {
        return result;
    } else if (!gapi_stateMasksValid(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (statemasks_unsupported(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_UNSUPPORTED;
    } else if (max_samples == 0) {
        result = GAPI_RETCODE_NO_DATA;
    } else {
        reader = u_reader(U_DATAREADERVIEW_GET(datareaderview));
        datareaderview->reader_mask.sampleStateMask = sample_states;
        datareaderview->reader_mask.viewStateMask = view_states;
        datareaderview->reader_mask.instanceStateMask = instance_states;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerTake(reader,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    }

    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_read_next_sample (
    gapi_fooDataReaderView _this,
    gapi_foo *data_values,
    gapi_sampleInfo *sample_info)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_fooDataReaderView_take_next_sample (
    gapi_fooDataReaderView _this,
    gapi_foo *data_values,
    gapi_sampleInfo *sample_info)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_fooDataReaderView_read_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReaderView     datareaderview;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);

    if ( datareaderview == NULL ) {
        return result;
    } else if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if ( statemasks_unsupported(sample_states,view_states,instance_states) ) {
        result = GAPI_RETCODE_UNSUPPORTED;
    } else if ( max_samples == 0 ) {
        result = GAPI_RETCODE_NO_DATA;
    } else if ( a_handle == GAPI_HANDLE_NIL ) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else {
        reader = u_reader(U_DATAREADERVIEW_GET(datareaderview));
        datareaderview->reader_mask.sampleStateMask = sample_states;
        datareaderview->reader_mask.viewStateMask = view_states;
        datareaderview->reader_mask.instanceStateMask = instance_states;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerReadInstance(reader,a_handle,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    }
    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_take_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReaderView     datareaderview;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);

    if (datareaderview == NULL) {
        return result;
    } else if (!gapi_stateMasksValid(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (statemasks_unsupported(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_UNSUPPORTED;
    } else if (max_samples == 0) {
        result = GAPI_RETCODE_NO_DATA;
    } else if (a_handle == GAPI_HANDLE_NIL) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else {
        reader = u_reader(U_DATAREADERVIEW_GET(datareaderview));
        datareaderview->reader_mask.sampleStateMask = sample_states;
        datareaderview->reader_mask.viewStateMask = view_states;
        datareaderview->reader_mask.instanceStateMask = instance_states;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerTakeInstance(reader,a_handle,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    }

    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_read_next_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReaderView     datareaderview;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);

    if (datareaderview == NULL) {
        return result;
    } else if (!gapi_stateMasksValid(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (statemasks_unsupported(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_UNSUPPORTED;
    } else if (max_samples == 0) {
        result = GAPI_RETCODE_NO_DATA;
    } else {
        reader = u_reader(U_DATAREADERVIEW_GET(datareaderview));
        datareaderview->reader_mask.sampleStateMask = sample_states;
        datareaderview->reader_mask.viewStateMask = view_states;
        datareaderview->reader_mask.instanceStateMask = instance_states;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerReadNextInstance(reader,a_handle,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    }

    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_take_next_instance (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReaderView     datareaderview;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);

    if (datareaderview == NULL ) {
        return result;
    } else if (!gapi_stateMasksValid(sample_states, view_states, instance_states)) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (statemasks_unsupported(sample_states,view_states,instance_states)) {
        result = GAPI_RETCODE_UNSUPPORTED;
    } else if (max_samples == 0) {
        result = GAPI_RETCODE_NO_DATA;
    } else {
        reader = u_reader(U_DATAREADERVIEW_GET(datareaderview));
        datareaderview->reader_mask.sampleStateMask = sample_states;
        datareaderview->reader_mask.viewStateMask = view_states;
        datareaderview->reader_mask.instanceStateMask = instance_states;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerTakeNextInstance(reader,a_handle,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    }

    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_return_loan (
    gapi_fooDataReaderView _this,
    void               *data_buffer,
    void               *info_buffer)
{
    _DataReaderView datareaderview;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    datareaderview = gapi_dataReaderViewClaim(_this, &result);

    if (datareaderview == NULL) {
        return result;
    } else if (data_buffer == NULL || info_buffer == NULL) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else {
        result = gapi_loanRegistry_deregister(datareaderview->loanRegistry,
                                              data_buffer,
                                              info_buffer);
    }

    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_read_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition)
{
    _DataReaderView     datareaderview;
    _ReadCondition      readcondition;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    if (datareaderview == NULL) {
        return result;
    } else if (readcondition == NULL) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (readcondition->dataReaderView != datareaderview) {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    } else if ( max_samples != 0) {
        reader = u_reader(readcondition->uQuery);
        datareaderview->reader_mask = readcondition->readerMask;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerRead(reader,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    } else {
        datareaderview->datareader->readerCopy(NULL, &readerInfo);
        result = GAPI_RETCODE_NO_DATA;
    }
    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_take_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition)
{
    _DataReaderView     datareaderview;
    _ReadCondition      readcondition;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    if (datareaderview == NULL) {
        return result;
    } else if (readcondition == NULL) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (readcondition->dataReaderView != datareaderview){
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    } else if ( max_samples != 0) {
        reader = u_reader(readcondition->uQuery);
        datareaderview->reader_mask = readcondition->readerMask;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerTake(reader,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    } else {
        datareaderview->datareader->readerCopy(NULL, &readerInfo);
        result = GAPI_RETCODE_NO_DATA;
    }
    _EntityRelease(datareaderview);

    return result;
}

gapi_returnCode_t
gapi_fooDataReaderView_read_next_instance_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition)
{
    _DataReaderView     datareaderview;
    _ReadCondition      readcondition;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    if (datareaderview == NULL) {
        return result;
    } else if (readcondition == NULL) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (readcondition->dataReaderView != datareaderview) {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    } else if ( max_samples != 0) {
        reader = u_reader(readcondition->uQuery);
        datareaderview->reader_mask = readcondition->readerMask;
        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerReadNextInstance(reader,a_handle,readerActionView,(c_voidp)&arg);
        result = kernelResultToApiResult(uResult);
        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    } else {
        datareaderview->datareader->readerCopy(NULL, &readerInfo);
        result = GAPI_RETCODE_NO_DATA;
    }
    _EntityRelease(datareaderview);

    return result;
}


gapi_returnCode_t
gapi_fooDataReaderView_take_next_instance_w_condition (
    gapi_fooDataReaderView _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition)
{
    _DataReaderView     datareaderview;
    _ReadCondition      readcondition;
    gapi_returnCode_t   result = GAPI_RETCODE_OK;
    gapi_readerInfo     readerInfo;
    u_reader            reader;
    v_readerSampleSeq   samples;
    v_readerSample      buffer[V_DATAREADERSAMPLESEQ_INITIAL];
    readerViewActionArg arg;
    u_result            uResult;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareaderview = gapi_dataReaderViewClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    if (datareaderview == NULL) {
        return result;
    } else if (readcondition == NULL) {
        result = GAPI_RETCODE_BAD_PARAMETER;
    } else if (readcondition->dataReaderView != datareaderview) {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    } else if ( max_samples != 0) {
        reader = u_reader(readcondition->uQuery);
        datareaderview->reader_mask = readcondition->readerMask;

        readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
        readerInfo.num_samples    = 0U;
        readerInfo.data_buffer    = data_values;
        readerInfo.info_buffer    = info_data;
        readerInfo.alloc_size     = datareaderview->datareader->allocSize;
        readerInfo.alloc_buffer   = datareaderview->datareader->allocBuffer;
        readerInfo.copy_out       = datareaderview->datareader->copy_out;
        readerInfo.copy_cache     = datareaderview->datareader->copy_cache;
        readerInfo.loan_registry  = (void**)&datareaderview->loanRegistry;

        samples._length  = 0;
        samples._maximum = V_DATAREADERSAMPLESEQ_INITIAL;
        samples._buffer  = buffer;
        samples._release = FALSE;

        arg.samples        = &samples;
        arg.max            = (gapi_unsigned_long)max_samples;
        arg.datareaderview = datareaderview;
        arg.readerInfo     = &readerInfo;
        arg.readerCopy     = datareaderview->datareader->readerCopy;
        arg.result         = GAPI_RETCODE_OK;

        uResult = u_readerTakeNextInstance(reader,
                                           a_handle,
                                           readerActionView,
                                           (c_voidp)&arg);

        result = kernelResultToApiResult(uResult);

        if ( result == GAPI_RETCODE_OK ) {
            result = arg.result;
        }
        v_readerSampleSeq_freebuf(&samples);
    } else {
        datareaderview->datareader->readerCopy(NULL, &readerInfo);
        result = GAPI_RETCODE_NO_DATA;
    }
    _EntityRelease(datareaderview);

    return result;
}

gapi_boolean
gapi_fooDataReaderView_is_loan (
    gapi_fooDataReaderView _this,
    void               *data_buffer,
    void               *info_buffer)
{
    _DataReaderView datareaderview;
    gapi_boolean result = FALSE;

    datareaderview = gapi_dataReaderViewClaim(_this, NULL);
    if ( datareaderview ) {
        if ( data_buffer && info_buffer ) {
            result = gapi_loanRegistry_is_loan(datareaderview->loanRegistry,
                                               data_buffer,
                                               info_buffer);
        }
    }
    _EntityRelease(datareaderview);

    return result;
}


gapi_returnCode_t
gapi_fooDataReaderView_get_key_value (
    gapi_fooDataReaderView _this,
    gapi_foo *key_holder,
    const gapi_instanceHandle_t handle)
{
    return GAPI_RETCODE_UNSUPPORTED;
}

typedef struct readerViewCopyInInfo_s {
    _DataReader reader;
    void *data;
} readerViewCopyInInfo;


static void
_DataReaderViewCopyIn (
    c_type type,
    void *data,
    void *to)
{
    c_base base = c_getBase(c_object(type));
    readerViewCopyInInfo *info = data;

    if (info->reader->copy_cache) {
        C_STRUCT(gapi_srcInfo) dataInfo;
        dataInfo.copyProgram = info->reader->copy_cache;
        dataInfo.src = info->data;

        info->reader->copy_in (base, &dataInfo, to);
    } else {
        info->reader->copy_in (base, info->data, to);
    }
}


/* InstanceHandle_t
 * lookup_instance(
 *     in Data instance);
 */
gapi_instanceHandle_t
gapi_fooDataReaderView_lookup_instance (
    gapi_fooDataReaderView _this,
    const gapi_foo *instance_data)
{
    _DataReaderView datareaderview;
    gapi_instanceHandle_t handle = GAPI_HANDLE_NIL;

    datareaderview = gapi_dataReaderViewClaim(_this, NULL);
    if ( datareaderview && instance_data ) {
        readerViewCopyInInfo rData;
        u_result uResult;

        rData.reader = datareaderview->datareader;
        rData.data = (void *)instance_data;

        uResult = u_dataViewLookupInstance(
                      U_DATAREADERVIEW_GET(datareaderview),
                      &rData,
                      _DataReaderViewCopyIn,
                      &handle);
    }
    _EntityRelease(datareaderview);

    return handle;
}

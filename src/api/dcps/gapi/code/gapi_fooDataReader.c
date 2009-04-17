/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
#include "gapi_fooDataReader.h"
#include "gapi_dataReader.h"
#include "gapi_qos.h"
#include "gapi_kernel.h"
#include "gapi_objManag.h"
#include "gapi_structured.h"
#include "gapi_dataReaderStatus.h"
#include "gapi_genericCopyIn.h"
#include "gapi_genericCopyOut.h"
#include "gapi_instanceHandle.h"

#include "os_heap.h"
#include "u_user.h"
#include "u_instanceHandle.h"
#include "v_kernel.h"
#include "v_state.h"
#include "v_public.h"
#include "kernelModule.h"
#include "v_dataReader.h"
#include "v_event.h"

#define INITIALBUFFER_SIZE 32
#define READBUFFERSIZE (32)

C_STRUCT(_FooDataReader) {
    C_EXTENDS(_DataReader);
};

C_CLASS(readBuffer);
C_STRUCT(readBuffer) {
        v_readerSample samples[READBUFFERSIZE];
        readBuffer next;
};

C_CLASS(readStack);
C_STRUCT (readStack) {
        c_ulong length;
        C_STRUCT(readBuffer) buffer;
        readBuffer last;
};

typedef struct readerActionArg_s {
    _DataReader         reader;
    gapi_unsigned_long  max;
    gapi_readerInfo    *readerInfo;
    readStack           samples;
    gapi_returnCode_t   result;
    gapi_dataSampleSeq  dataSamples;
} readerActionArg;

#define readStackPush(_this,sample) \
{ \
    c_long index; \
 \
    index = _this->length % READBUFFERSIZE; \
    if ((index == 0) && (_this->length > 0)) { \
        /* Need a new buffer */ \
        _this->last->next = os_malloc(sizeof(C_STRUCT(readBuffer))); \
        _this->last = _this->last->next; \
        _this->last->next = NULL; \
    } \
    _this->last->samples[index] = c_keep(sample); \
    _this->length++; \
    c_keep(sample->instance); \
}

#define readStackInit(_this) \
        { ((readStack)_this)->length = 0; \
          ((readStack)_this)->last = &((readStack)_this)->buffer; \
          ((readStack)_this)->last->next = NULL; }

v_readerSample
readStackSample(
    readStack _this,
    c_long index)
{
    readBuffer buf = &_this->buffer;
    c_long bufnum = index / READBUFFERSIZE;
    c_long n;
    for (n=0; n<bufnum; n++) { buf = buf->next; }
    return v_readerSample(buf->samples[index % READBUFFERSIZE]);
}

#define readStackFree(_this) \
{ \
    readBuffer next,del; \
 \
    next = ((readStack)_this)->buffer.next; \
    while (next) { \
        del = next; \
        next = del->next; \
        os_free(del); \
    } \
}

static void
determineSampleInfo (
     readerActionArg *info)
{
    gapi_unsigned_long  i,first,last;
    readStack samples = info->samples;
    gapi_unsigned_long  length;
    v_dataReaderInstance instance;
    v_readerSample sample;
    v_dataReaderSample lastSample;
    v_message message;
    v_state state;
    gapi_sampleInfo *to;
    gapi_unsigned_long   mrsDisposed, mrsicDisposed;
    gapi_viewStateKind   vs;
    gapi_long            count;
    gapi_instanceStateKind instance_state;

    /* The samples sequence now contains all samples read,
     * Now determine the sample info for each sample.
     */
    instance = NULL;
    length = samples->length;
    for (i=0; i<length; i++) {
        sample   = readStackSample(samples,i);
        if (instance != sample->instance) {
            /* A new instance starts with this sample.
             * Now determine the instance related sampleInfo needed for each
             * sample belonging to this instance.
             */
            first = i;
            instance = sample->instance;
            /* Now find the last sample belonging to this instance. */
            last = first;
            while ((last < (length-1)) &&
                   (instance == readStackSample(samples,(last+1))->instance))
            { last++; }
            lastSample = v_dataReaderSample(readStackSample(samples,last));

            /* Now all information is available to determine the instance related
             * sampleInfo information.
             * This information is calculated once for all samples belonging to this
             * instance.
             */
            mrsicDisposed = lastSample->disposeCount + lastSample->noWritersCount;

            state = instance->instanceState;
            instance_state = GAPI_ALIVE_INSTANCE_STATE;
            if (v_stateTest (state, L_NOWRITERS)) {
                instance_state = GAPI_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
            }
            if (v_stateTest (state, L_DISPOSED)) {
                instance_state = GAPI_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
            }
            /* determine instance view state from first sample state.
             */
            if ( v_stateTest(sample->sampleState, L_NEW) ) {
                vs = GAPI_NEW_VIEW_STATE;
            } else {
                vs = GAPI_NOT_NEW_VIEW_STATE;
            }
            mrsDisposed   = instance->disposeCount +
                            instance->noWritersCount;
        }
        state = sample->sampleState;
        message  = v_message(C_REFGET(sample,info->reader->messageOffset));
        count = v_dataReaderSample(sample)->disposeCount +
                v_dataReaderSample(sample)->noWritersCount;
        info->dataSamples._buffer[i].message = c_keep(message);
        info->dataSamples._buffer[i].data =
                         C_DISPLACE(message,info->reader->userdataOffset);

        to = &info->dataSamples._buffer[i].info;

        if (v_stateTest (state, L_READ)) {
            to->sample_state = GAPI_READ_SAMPLE_STATE;
        } else {
            to->sample_state = GAPI_NOT_READ_SAMPLE_STATE;
        }

        to->view_state                  = vs;

        to->valid_data                  = v_stateTest(state, L_VALIDDATA);

        to->instance_state              = instance_state;
        to->source_timestamp.sec        = (gapi_long)(message->writeTime.seconds);
        to->source_timestamp.nanosec    = (gapi_unsigned_long)(message->writeTime.nanoseconds);

        to->disposed_generation_count   = v_dataReaderSample(sample)->disposeCount;
        to->no_writers_generation_count = v_dataReaderSample(sample)->noWritersCount;

        to->sample_rank                 = last - i;
        to->generation_rank             = mrsicDisposed - count;
        to->absolute_generation_rank    = mrsDisposed - count;

        to->instance_handle             = gapi_instanceHandleFromHandle(v_publicHandle(v_public(instance)));
        to->publication_handle          = gapi_instanceHandleFromGID(v_dataReaderSample(sample)->publicationHandle);

        to->arrival_timestamp.sec       = (gapi_long)(v_dataReaderSample(sample)->insertTime.seconds);
        to->arrival_timestamp.nanosec   = (gapi_unsigned_long)(v_dataReaderSample(sample)->insertTime.nanoseconds);
    }
}

static c_bool
readerAction (
    c_object o,
    c_voidp copyArg)
{
    readerActionArg    *info    = (readerActionArg *) copyArg;
    readStack           samples = info->samples;
    c_bool              result  = TRUE;
    gapi_unsigned_long  length;

    length = samples->length;
    if ( o ) {
        readStackPush(samples,v_readerSample(o));
        if ( samples->length >= info->max ) {
            result = FALSE;
        }
    } else {
        if ( length > 0 ) {
            if ( length > info->dataSamples._maximum ) {
                gapi_unsigned_long size;
                size = length*sizeof(gapi_dataSample);
                info->dataSamples._buffer = (gapi_dataSample *)os_malloc(size);
                info->dataSamples._maximum = length;
            }
            if (info->dataSamples._buffer) {
                info->dataSamples._length  = length;

                determineSampleInfo(info);

            } else {
                info->result = GAPI_RETCODE_OUT_OF_RESOURCES;
            }
        } else {
            info->result = GAPI_RETCODE_NO_DATA;
        }
        result = FALSE;
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_read (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReader       datareader;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            datareader->reader_mask.sampleStateMask = 0U;
            datareader->reader_mask.viewStateMask = 0U;
            datareader->reader_mask.instanceStateMask = 0U;
            if (sample_states != GAPI_ANY_SAMPLE_STATE) {
                datareader->reader_mask.sampleStateMask = sample_states;
                reader = u_reader(datareader->uQuery);
            }
            if (view_states != GAPI_ANY_VIEW_STATE) {
                datareader->reader_mask.viewStateMask = view_states;
                reader = u_reader(datareader->uQuery);
            }
            if (instance_states != GAPI_ANY_INSTANCE_STATE) {
                datareader->reader_mask.instanceStateMask = instance_states;
                reader = u_reader(datareader->uQuery);
            }
            readStackInit(&samples);

            readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
            readerInfo.num_samples    = 0U;
            readerInfo.data_buffer    = data_values;
            readerInfo.info_buffer    = info_data;
            readerInfo.alloc_size     = datareader->allocSize;
            readerInfo.alloc_buffer   = datareader->allocBuffer;
            readerInfo.copy_out       = datareader->copy_out;
            readerInfo.copy_cache     = datareader->copy_cache;
            readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

            arg.reader         = datareader;
            arg.samples        = &samples;
            arg.max            = (gapi_unsigned_long)max_samples;
            arg.result         = GAPI_RETCODE_OK;
            {
                gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                u_result r;
                gapi_unsigned_long i;
                v_readerSample sample;

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerRead(reader, readerAction, (c_voidp)&arg);

                result = kernelResultToApiResult(r);
                if ( result == GAPI_RETCODE_OK ) {
                    if (arg.result == GAPI_RETCODE_NO_DATA) {
                        datareader->readerCopy(NULL, &readerInfo);
                    } else {
                        datareader->readerCopy(&arg.dataSamples, &readerInfo);
                    }
                    result = arg.result;
                }
                for ( i = 0; i < arg.dataSamples._length; i++ ) {
                    c_free(arg.dataSamples._buffer[i].message);
                }
                if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                    os_free(arg.dataSamples._buffer);
                }
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
            }
            readStackFree(&samples);
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_take (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReader       datareader;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            datareader->reader_mask.sampleStateMask = 0U;
            datareader->reader_mask.viewStateMask = 0U;
            datareader->reader_mask.instanceStateMask = 0U;
            if (sample_states != GAPI_ANY_SAMPLE_STATE) {
                datareader->reader_mask.sampleStateMask = sample_states;
                reader = u_reader(datareader->uQuery);
            }
            if (view_states != GAPI_ANY_VIEW_STATE) {
                datareader->reader_mask.viewStateMask = view_states;
                reader = u_reader(datareader->uQuery);
            }
            if (instance_states != GAPI_ANY_INSTANCE_STATE) {
                datareader->reader_mask.instanceStateMask = instance_states;
                reader = u_reader(datareader->uQuery);
            }
            readStackInit(&samples);

            readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
            readerInfo.num_samples    = 0U;
            readerInfo.data_buffer    = data_values;
            readerInfo.info_buffer    = info_data;
            readerInfo.alloc_size     = datareader->allocSize;
            readerInfo.alloc_buffer   = datareader->allocBuffer;
            readerInfo.copy_out       = datareader->copy_out;
            readerInfo.copy_cache     = datareader->copy_cache;
            readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

            arg.reader     = datareader;
            arg.samples    = &samples;
            arg.max        = (gapi_unsigned_long)max_samples;
            arg.result     = GAPI_RETCODE_OK;
            {
                gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                u_result r;
                gapi_unsigned_long i;
                v_readerSample sample;

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerTake(reader, readerAction, (c_voidp)&arg);

                result = kernelResultToApiResult(r);
                if ( result == GAPI_RETCODE_OK ) {
                    if (arg.result == GAPI_RETCODE_NO_DATA) {
                        datareader->readerCopy(NULL, &readerInfo);
                    } else {
                        datareader->readerCopy(&arg.dataSamples, &readerInfo);
                    }
                    result = arg.result;
                }
                for ( i = 0; i < arg.dataSamples._length; i++ ) {
                    c_free(arg.dataSamples._buffer[i].message);
                }
                if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                    os_free(arg.dataSamples._buffer);
                }
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
            }
            readStackFree(&samples);
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_read_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition)
{
    _DataReader       datareader;
    _ReadCondition    readcondition;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareader = gapi_dataReaderClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    if ( datareader != NULL ) {
        if ( readcondition == NULL ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if ((readcondition->dataReader != datareader ) ||
                   (readcondition->dataReaderView != NULL) ){
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        } else {
            reader = u_reader(readcondition->uQuery);

            readStackInit(&samples);

            readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
            readerInfo.num_samples    = 0U;
            readerInfo.data_buffer    = data_values;
            readerInfo.info_buffer    = info_data;
            readerInfo.alloc_size     = datareader->allocSize;
            readerInfo.alloc_buffer   = datareader->allocBuffer;
            readerInfo.copy_out       = datareader->copy_out;
            readerInfo.copy_cache     = datareader->copy_cache;
            readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

            arg.reader     = datareader;
            arg.samples    = &samples;
            arg.max        = (gapi_unsigned_long)max_samples;
            arg.result     = GAPI_RETCODE_OK;
            {
                gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                u_result r;
                gapi_unsigned_long i;
                v_readerSample sample;

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerRead(reader, readerAction, (c_voidp)&arg);

                result = kernelResultToApiResult(r);
                if ( result == GAPI_RETCODE_OK ) {
                    if (arg.result == GAPI_RETCODE_NO_DATA) {
                        datareader->readerCopy(NULL, &readerInfo);
                    } else {
                        datareader->readerCopy(&arg.dataSamples, &readerInfo);
                    }
                    result = arg.result;
                }
                for ( i = 0; i < arg.dataSamples._length; i++ ) {
                    c_free(arg.dataSamples._buffer[i].message);
                }
                if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                    os_free(arg.dataSamples._buffer);
                }
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
            }
            readStackFree(&samples);
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_take_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_readCondition a_condition)
{
    _DataReader       datareader;
    _ReadCondition    readcondition;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    datareader = gapi_dataReaderClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    if ( datareader != NULL ) {
        if ( readcondition == NULL ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if ((readcondition->dataReader != datareader ) ||
                   (readcondition->dataReaderView != NULL) ){
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(readcondition->uQuery);

            readStackInit(&samples);

            readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
            readerInfo.num_samples    = 0U;
            readerInfo.data_buffer    = data_values;
            readerInfo.info_buffer    = info_data;
            readerInfo.alloc_size     = datareader->allocSize;
            readerInfo.alloc_buffer   = datareader->allocBuffer;
            readerInfo.copy_out       = datareader->copy_out;
            readerInfo.copy_cache     = datareader->copy_cache;
            readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

            arg.reader     = datareader;
            arg.samples    = &samples;
            arg.max        = (gapi_unsigned_long)max_samples;
            arg.result     = GAPI_RETCODE_OK;
            {
                gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                u_result r;
                gapi_unsigned_long i;
                v_readerSample sample;

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerTake(reader, readerAction, (c_voidp)&arg);
                result = kernelResultToApiResult(r);
                if ( result == GAPI_RETCODE_OK ) {
                    if (arg.result == GAPI_RETCODE_NO_DATA) {
                        datareader->readerCopy(NULL, &readerInfo);
                    } else {
                        datareader->readerCopy(&arg.dataSamples, &readerInfo);
                    }
                    result = arg.result;
                }
                for ( i = 0; i < arg.dataSamples._length; i++ ) {
                    c_free(arg.dataSamples._buffer[i].message);
                }
                if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                    os_free(arg.dataSamples._buffer);
                }
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
            }
            readStackFree(&samples);
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_read_next_sample (
    gapi_fooDataReader _this,
    gapi_foo *data_values,
    gapi_sampleInfo *sample_info
    )
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_fooDataReader_take_next_sample (
    gapi_fooDataReader _this,
    gapi_foo *data_values,
    gapi_sampleInfo *sample_info
    )
{
    return GAPI_RETCODE_UNSUPPORTED;
}

gapi_returnCode_t
gapi_fooDataReader_read_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReader       datareader;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;
    u_instanceHandle  handle;

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (a_handle == GAPI_HANDLE_NIL) {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            result = gapi_instanceHandle_to_u_instanceHandle(a_handle, reader, &handle);
            if (result == GAPI_RETCODE_OK) {
                datareader->reader_mask.sampleStateMask = 0U;
                datareader->reader_mask.viewStateMask = 0U;
                datareader->reader_mask.instanceStateMask = 0U;
                if (sample_states != GAPI_ANY_SAMPLE_STATE) {
                    datareader->reader_mask.sampleStateMask = sample_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (view_states != GAPI_ANY_VIEW_STATE) {
                    datareader->reader_mask.viewStateMask = view_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (instance_states != GAPI_ANY_INSTANCE_STATE) {
                    datareader->reader_mask.instanceStateMask = instance_states;
                    reader = u_reader(datareader->uQuery);
                }

                readStackInit(&samples);

                readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
                readerInfo.num_samples    = 0U;
                readerInfo.data_buffer    = data_values;
                readerInfo.info_buffer    = info_data;
                readerInfo.alloc_size     = datareader->allocSize;
                readerInfo.alloc_buffer   = datareader->allocBuffer;
                readerInfo.copy_out       = datareader->copy_out;
                readerInfo.copy_cache     = datareader->copy_cache;
                readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

                arg.reader     = datareader;
                arg.samples    = &samples;
                arg.max        = (gapi_unsigned_long)max_samples;
                arg.result     = GAPI_RETCODE_OK;
                {
                    gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                    u_result r;
                    gapi_unsigned_long i;
                    v_readerSample sample;

                    arg.dataSamples._buffer  = (void *)&initialBuffer;
                    arg.dataSamples._length  = 0;
                    arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                    arg.dataSamples._release = FALSE;

                    r = u_readerReadInstance(reader,
                                             handle,
                                             readerAction,
                                             (c_voidp)&arg);

                    result = kernelResultToApiResult(r);
                    if ( result == GAPI_RETCODE_OK ) {
                        if (arg.result == GAPI_RETCODE_NO_DATA) {
                            datareader->readerCopy(NULL, &readerInfo);
                        } else {
                            datareader->readerCopy(&arg.dataSamples,
                                                   &readerInfo);
                        }
                        result = arg.result;
                    }
                    for ( i = 0; i < arg.dataSamples._length; i++ ) {
                        c_free(arg.dataSamples._buffer[i].message);
                    }
                    if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                        os_free(arg.dataSamples._buffer);
                    }
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                }
                readStackFree(&samples);
            }
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_take_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReader       datareader;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;
    u_instanceHandle  handle;

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (a_handle == GAPI_HANDLE_NIL) {
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            result = gapi_instanceHandle_to_u_instanceHandle(a_handle, reader, &handle);
            if (result == GAPI_RETCODE_OK) {
                datareader->reader_mask.sampleStateMask = 0U;
                datareader->reader_mask.viewStateMask = 0U;
                datareader->reader_mask.instanceStateMask = 0U;
                if (sample_states != GAPI_ANY_SAMPLE_STATE) {
                    datareader->reader_mask.sampleStateMask = sample_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (view_states != GAPI_ANY_VIEW_STATE) {
                    datareader->reader_mask.viewStateMask = view_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (instance_states != GAPI_ANY_INSTANCE_STATE) {
                    datareader->reader_mask.instanceStateMask = instance_states;
                    reader = u_reader(datareader->uQuery);
                }

                readStackInit(&samples);

                readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
                readerInfo.num_samples    = 0U;
                readerInfo.data_buffer    = data_values;
                readerInfo.info_buffer    = info_data;
                readerInfo.alloc_size     = datareader->allocSize;
                readerInfo.alloc_buffer   = datareader->allocBuffer;
                readerInfo.copy_out       = datareader->copy_out;
                readerInfo.copy_cache     = datareader->copy_cache;
                readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

                arg.reader     = datareader;
                arg.samples    = &samples;
                arg.max        = (gapi_unsigned_long)max_samples;
                arg.result     = GAPI_RETCODE_OK;
                {
                    gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                    u_result r;
                    gapi_unsigned_long i;
                    v_readerSample sample;

                    arg.dataSamples._buffer  = (void *)&initialBuffer;
                    arg.dataSamples._length  = 0;
                    arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                    arg.dataSamples._release = FALSE;

                    r = u_readerTakeInstance(reader,
                                             handle,
                                             readerAction,
                                             (c_voidp)&arg);

                    result = kernelResultToApiResult(r);
                    if ( result == GAPI_RETCODE_OK ) {
                        if (arg.result == GAPI_RETCODE_NO_DATA) {
                            datareader->readerCopy(NULL, &readerInfo);
                        } else {
                            datareader->readerCopy(&arg.dataSamples,
                                                   &readerInfo);
                        }
                        result = arg.result;
                    }
                    for ( i = 0; i < arg.dataSamples._length; i++ ) {
                        c_free(arg.dataSamples._buffer[i].message);
                    }
                    if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                        os_free(arg.dataSamples._buffer);
                    }
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                }
                readStackFree(&samples);
            }
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_read_next_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReader       datareader;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;
    u_instanceHandle  handle;

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            result = gapi_instanceHandle_to_u_instanceHandle(a_handle, reader, &handle);
            if (result == GAPI_RETCODE_OK) {
                datareader->reader_mask.sampleStateMask = 0U;
                datareader->reader_mask.viewStateMask = 0U;
                datareader->reader_mask.instanceStateMask = 0U;
                if (sample_states != GAPI_ANY_SAMPLE_STATE) {
                    datareader->reader_mask.sampleStateMask = sample_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (view_states != GAPI_ANY_VIEW_STATE) {
                    datareader->reader_mask.viewStateMask = view_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (instance_states != GAPI_ANY_INSTANCE_STATE) {
                    datareader->reader_mask.instanceStateMask = instance_states;
                    reader = u_reader(datareader->uQuery);
                }

                readStackInit(&samples);

                readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
                readerInfo.num_samples    = 0U;
                readerInfo.data_buffer    = data_values;
                readerInfo.info_buffer    = info_data;
                readerInfo.alloc_size     = datareader->allocSize;
                readerInfo.alloc_buffer   = datareader->allocBuffer;
                readerInfo.copy_out       = datareader->copy_out;
                readerInfo.copy_cache     = datareader->copy_cache;
                readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

                arg.reader     = datareader;
                arg.samples    = &samples;
                arg.max        = (gapi_unsigned_long)max_samples;
                arg.result     = GAPI_RETCODE_OK;
                {
                    gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                    u_result r;
                    gapi_unsigned_long i;
                    v_readerSample sample;

                    arg.dataSamples._buffer  = (void *)&initialBuffer;
                    arg.dataSamples._length  = 0;
                    arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                    arg.dataSamples._release = FALSE;

                    r = u_readerReadNextInstance(reader,
                                                 handle,
                                                 readerAction,
                                                 (c_voidp)&arg);

                    result = kernelResultToApiResult(r);
                    if ( result == GAPI_RETCODE_OK ) {
                        if (arg.result == GAPI_RETCODE_NO_DATA) {
                            datareader->readerCopy(NULL, &readerInfo);
                        } else {
                            datareader->readerCopy(&arg.dataSamples,
                                                   &readerInfo);
                        }
                        result = arg.result;
                    }
                    for ( i = 0; i < arg.dataSamples._length; i++ ) {
                        c_free(arg.dataSamples._buffer[i].message);
                    }
                    if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                        os_free(arg.dataSamples._buffer);
                    }
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                }
                readStackFree(&samples);
            }
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_take_next_instance (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_sampleStateMask sample_states,
    const gapi_viewStateMask view_states,
    const gapi_instanceStateMask instance_states)
{
    _DataReader       datareader;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;
    u_instanceHandle  handle;

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            result = gapi_instanceHandle_to_u_instanceHandle(a_handle, reader, &handle);
            if (result == GAPI_RETCODE_OK) {
                datareader->reader_mask.sampleStateMask = 0U;
                datareader->reader_mask.viewStateMask = 0U;
                datareader->reader_mask.instanceStateMask = 0U;
                if (sample_states != GAPI_ANY_SAMPLE_STATE) {
                    datareader->reader_mask.sampleStateMask = sample_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (view_states != GAPI_ANY_VIEW_STATE) {
                    datareader->reader_mask.viewStateMask = view_states;
                    reader = u_reader(datareader->uQuery);
                }
                if (instance_states != GAPI_ANY_INSTANCE_STATE) {
                    datareader->reader_mask.instanceStateMask = instance_states;
                    reader = u_reader(datareader->uQuery);
                }

                readStackInit(&samples);

                readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
                readerInfo.num_samples    = 0U;
                readerInfo.data_buffer    = data_values;
                readerInfo.info_buffer    = info_data;
                readerInfo.alloc_size     = datareader->allocSize;
                readerInfo.alloc_buffer   = datareader->allocBuffer;
                readerInfo.copy_out       = datareader->copy_out;
                readerInfo.copy_cache     = datareader->copy_cache;
                readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

                arg.reader     = datareader;
                arg.samples    = &samples;
                arg.max        = (gapi_unsigned_long)max_samples;
                arg.result     = GAPI_RETCODE_OK;
                {
                    gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                    u_result r;
                    gapi_unsigned_long i;
                    v_readerSample sample;

                    arg.dataSamples._buffer  = (void *)&initialBuffer;
                    arg.dataSamples._length  = 0;
                    arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                    arg.dataSamples._release = FALSE;

                    r = u_readerTakeNextInstance(reader,
                                                 handle,
                                                 readerAction,
                                                 (c_voidp)&arg);

                    result = kernelResultToApiResult(r);
                    if ( result == GAPI_RETCODE_OK ) {
                        if (arg.result == GAPI_RETCODE_NO_DATA) {
                            datareader->readerCopy(NULL, &readerInfo);
                        } else {
                            datareader->readerCopy(&arg.dataSamples,
                                                   &readerInfo);
                        }
                        result = arg.result;
                    }
                    for ( i = 0; i < arg.dataSamples._length; i++ ) {
                        c_free(arg.dataSamples._buffer[i].message);
                    }
                    if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                        os_free(arg.dataSamples._buffer);
                    }
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                }
                readStackFree(&samples);
            }
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_read_next_instance_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition)
{
    _DataReader       datareader;
    _ReadCondition    readcondition;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;
    u_instanceHandle  handle;

    datareader = gapi_dataReaderClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( readcondition == NULL ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if ((readcondition->dataReader != datareader ) ||
                   (readcondition->dataReaderView != NULL) ){
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            datareader->reader_mask.sampleStateMask   =
                    gapi_readCondition_get_sample_state_mask (a_condition);
            datareader->reader_mask.viewStateMask     =
                    gapi_readCondition_get_view_state_mask (a_condition);
            datareader->reader_mask.instanceStateMask =
                    gapi_readCondition_get_instance_state_mask (a_condition);

            reader = u_reader(readcondition->uQuery);
            result = gapi_instanceHandle_to_u_instanceHandle(a_handle, reader, &handle);
            if (result == GAPI_RETCODE_OK) {

                readStackInit(&samples);

                readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
                readerInfo.num_samples    = 0U;
                readerInfo.data_buffer    = data_values;
                readerInfo.info_buffer    = info_data;
                readerInfo.alloc_size     = datareader->allocSize;
                readerInfo.alloc_buffer   = datareader->allocBuffer;
                readerInfo.copy_out       = datareader->copy_out;
                readerInfo.copy_cache     = datareader->copy_cache;
                readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

                arg.reader     = datareader;
                arg.samples    = &samples;
                arg.max        = (gapi_unsigned_long)max_samples;
                arg.result     = GAPI_RETCODE_OK;
                {
                    gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                    u_result r;
                    gapi_unsigned_long i;
                    v_readerSample sample;

                    arg.dataSamples._buffer  = (void *)&initialBuffer;
                    arg.dataSamples._length  = 0;
                    arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                    arg.dataSamples._release = FALSE;

                    r = u_readerReadNextInstance(reader,
                                                 handle,
                                                 readerAction,
                                                 (c_voidp)&arg);

                    result = kernelResultToApiResult(r);
                    if ( result == GAPI_RETCODE_OK ) {
                        if (arg.result == GAPI_RETCODE_NO_DATA) {
                            datareader->readerCopy(NULL, &readerInfo);
                        } else {
                            datareader->readerCopy(&arg.dataSamples,
                                                   &readerInfo);
                        }
                        result = arg.result;
                    }
                    for ( i = 0; i < arg.dataSamples._length; i++ ) {
                        c_free(arg.dataSamples._buffer[i].message);
                    }
                    if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                        os_free(arg.dataSamples._buffer);
                    }
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                }
                readStackFree(&samples);
            }
        }
        _EntityRelease(datareader);
    }
    return result;
}

gapi_returnCode_t
gapi_fooDataReader_take_next_instance_w_condition (
    gapi_fooDataReader _this,
    void *data_values,
    void *info_data,
    const gapi_long max_samples,
    const gapi_instanceHandle_t a_handle,
    const gapi_readCondition a_condition)
{
    _DataReader       datareader;
    _ReadCondition    readcondition;
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    gapi_readerInfo   readerInfo;
    u_reader          reader;
    C_STRUCT(readStack) samples;
    readerActionArg   arg;
    u_instanceHandle  handle;

    datareader = gapi_dataReaderClaim(_this, &result);
    readcondition = _ReadConditionFromHandle(a_condition);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( readcondition == NULL ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if ((readcondition->dataReader != datareader ) ||
                   (readcondition->dataReaderView != NULL) ){
            result = GAPI_RETCODE_PRECONDITION_NOT_MET;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            datareader->reader_mask.sampleStateMask   =
                gapi_readCondition_get_sample_state_mask (a_condition);
            datareader->reader_mask.viewStateMask     =
                gapi_readCondition_get_view_state_mask (a_condition);
            datareader->reader_mask.instanceStateMask =
                gapi_readCondition_get_instance_state_mask (a_condition);

            reader = u_reader(readcondition->uQuery);
            result = gapi_instanceHandle_to_u_instanceHandle(a_handle, reader, &handle);
            if (result == GAPI_RETCODE_OK) {

                readStackInit(&samples);

                readerInfo.max_samples    = (gapi_unsigned_long)max_samples;
                readerInfo.num_samples    = 0U;
                readerInfo.data_buffer    = data_values;
                readerInfo.info_buffer    = info_data;
                readerInfo.alloc_size     = datareader->allocSize;
                readerInfo.alloc_buffer   = datareader->allocBuffer;
                readerInfo.copy_out       = datareader->copy_out;
                readerInfo.copy_cache     = datareader->copy_cache;
                readerInfo.loan_registry  = (void **)&datareader->loanRegistry;

                arg.reader     = datareader;
                arg.samples    = &samples;
                arg.max        = (gapi_unsigned_long)max_samples;
                arg.result     = GAPI_RETCODE_OK;
                {
                    gapi_dataSample initialBuffer[INITIALBUFFER_SIZE];
                    u_result r;
                    gapi_unsigned_long i;
                    v_readerSample sample;

                    arg.dataSamples._buffer  = (void *)&initialBuffer;
                    arg.dataSamples._length  = 0;
                    arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                    arg.dataSamples._release = FALSE;

                    r = u_readerTakeNextInstance(reader,
                                                 handle,
                                                 readerAction,
                                                 (c_voidp)&arg);

                    result = kernelResultToApiResult(r);
                    if ( result == GAPI_RETCODE_OK ) {
                        if (arg.result == GAPI_RETCODE_NO_DATA) {
                            datareader->readerCopy(NULL, &readerInfo);
                        } else {
                            datareader->readerCopy(&arg.dataSamples,
                                                   &readerInfo);
                        }
                        result = arg.result;
                    }
                    for ( i = 0; i < arg.dataSamples._length; i++ ) {
                        c_free(arg.dataSamples._buffer[i].message);
                    }
                    if (arg.dataSamples._buffer != (void *)&initialBuffer) {
                        os_free(arg.dataSamples._buffer);
                    }
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                }
                readStackFree(&samples);
            }
        }
        _EntityRelease(datareader);
    }
    return result;
}


gapi_returnCode_t
gapi_fooDataReader_return_loan (
    gapi_fooDataReader _this,
    void *data_buffer,
    void *info_buffer)
{
    _DataReader datareader;
    gapi_returnCode_t result = GAPI_RETCODE_OK;

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader == NULL ) {
        return result;
    } else if ( (data_buffer == NULL) && (info_buffer == NULL) ) {
        result = GAPI_RETCODE_OK;
    } else if ( (data_buffer == NULL) || (info_buffer == NULL) ) {
        result = GAPI_RETCODE_PRECONDITION_NOT_MET;
    } else {
        result = gapi_loanRegistry_deregister(datareader->loanRegistry,
                                              data_buffer,
                                              info_buffer);
    }

    _EntityRelease(datareader);

    return result;
}

gapi_boolean
gapi_fooDataReader_is_loan (
    gapi_fooDataReader _this,
    void *data_buffer,
    void *info_buffer)
{
    _DataReader datareader;
    gapi_boolean result = FALSE;

    datareader = gapi_dataReaderClaim(_this, NULL);
    if ( datareader ) {
        if ( data_buffer && info_buffer ) {
            result = gapi_loanRegistry_is_loan(datareader->loanRegistry,
                                               data_buffer,
                                               info_buffer);
        }
    }
    _EntityRelease(datareader);

    return result;
}

gapi_returnCode_t
gapi_fooDataReader_get_key_value (
    gapi_fooDataReader _this,
    gapi_foo *key_holder,
    const gapi_instanceHandle_t handle
    )
{
    return GAPI_RETCODE_UNSUPPORTED;
}

typedef struct readerCopyInInfo_s {
    _DataReader reader;
    void *data;
} readerCopyInInfo;


static void
_DataReaderCopyIn (
    c_type type,
    void *data,
    void *to)
{
    c_base base = c_getBase(c_object(type));
    readerCopyInInfo *info = data;

    if (info->reader->copy_cache) {
        C_STRUCT(gapi_srcInfo) dataInfo;

        dataInfo.copyProgram = info->reader->copy_cache;
        dataInfo.src = info->data;

        info->reader->copy_in (base, &dataInfo, to);
    } else {
        info->reader->copy_in (base, info->data, to);
    }
}

gapi_instanceHandle_t
gapi_fooDataReader_lookup_instance (
    gapi_fooDataReader _this,
    const gapi_foo *instance_data)
{
    _DataReader datareader;
    gapi_instanceHandle_t handle = GAPI_HANDLE_NIL;

    datareader = gapi_dataReaderClaim(_this, NULL);
    if ( datareader && instance_data ) {
        readerCopyInInfo rData;
        u_instanceHandle uHandle;
        u_result uResult;

        rData.reader = datareader;
        rData.data = (void *)instance_data;

        uResult = u_dataReaderLookupInstance(U_DATAREADER_GET(datareader),
                                             &rData, _DataReaderCopyIn, &uHandle);
        if ( uResult == U_RESULT_OK ) {
            handle = gapi_instanceHandleFromHandle(uHandle);
        }
    }
    _EntityRelease(datareader);

    return handle;
}

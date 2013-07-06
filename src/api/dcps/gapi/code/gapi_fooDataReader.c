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
#include "v_dataViewInstance.h"
#include "v_dataReaderInstance.h"

#include "gapi_fooDataReader.h"
#include "gapi_dataReader.h"
#include "gapi_qos.h"
#include "gapi_kernel.h"
#include "gapi_objManag.h"
#include "gapi_structured.h"
#include "gapi_genericCopyIn.h"
#include "gapi_genericCopyOut.h"

#include "os_heap.h"
#include "u_handle.h"
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

C_CLASS(instanceField);
C_STRUCT(instanceField){
    v_readerSample lastSample;
    c_ulong sampleIndex;
};

C_CLASS(instanceBuffer);
C_STRUCT(instanceBuffer){
    C_STRUCT(instanceField) instances[READBUFFERSIZE];
    instanceBuffer next;
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
        c_ulong nextReadIndex;
        readBuffer lastReadBuffer;

        c_ulong instanceLength;
        C_STRUCT(instanceBuffer) instanceBuffer;
        instanceBuffer lastInstanceBuffer;
        v_readerSample prevSample;
        v_dataReaderInstance lastInstance;
        c_ulong nextIndexInstance;
        instanceBuffer lastReadBufferInstance;
        c_ulong lastSampleIndex;
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
    c_long indexInstance; \
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
 \
    if(_this->lastInstance != sample->instance) { \
        indexInstance = _this->instanceLength % READBUFFERSIZE; \
        if ((indexInstance == 0) && (_this->instanceLength > 0)) { \
            /* Need a new buffer */ \
            _this->lastInstanceBuffer->next = os_malloc(sizeof(C_STRUCT(instanceBuffer))); \
            _this->lastInstanceBuffer = _this->lastInstanceBuffer->next; \
            _this->lastInstanceBuffer->next = NULL; \
        } \
        if(_this->prevSample){ \
            _this->lastInstanceBuffer->instances[indexInstance].lastSample = _this->prevSample; \
            _this->lastInstanceBuffer->instances[indexInstance].sampleIndex = _this->lastSampleIndex; \
            _this->instanceLength++; \
        } \
        _this->lastInstance = sample->instance; \
    } \
    _this->lastSampleIndex = _this->length - 1; \
    _this->prevSample = sample; \
}

#define readStackInit(_this) \
        { ((readStack)_this)->length = 0; \
          ((readStack)_this)->last = &((readStack)_this)->buffer; \
          ((readStack)_this)->last->next = NULL; \
          ((readStack)_this)->nextReadIndex = 0; \
          ((readStack)_this)->lastReadBuffer = NULL; \
          ((readStack)_this)->instanceLength = 0; \
          ((readStack)_this)->lastInstanceBuffer = &((readStack)_this)->instanceBuffer; \
          ((readStack)_this)->lastInstanceBuffer->next = NULL; \
          ((readStack)_this)->nextIndexInstance = 0; \
          ((readStack)_this)->prevSample = NULL; \
          ((readStack)_this)->lastInstance = NULL; \
          ((readStack)_this)->lastSampleIndex = 0; \
          ((readStack)_this)->lastReadBufferInstance = NULL; }


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

v_readerSample
readStackNextSample(
    readStack _this)
{
    v_readerSample sample;
    c_long bufnum;
    readBuffer buf = &_this->buffer;

    if(_this->lastReadBuffer == NULL){
        if(_this->length == 0){
            sample = NULL;
        } else {
            sample = v_readerSample(buf->samples[0]);
            _this->lastReadBuffer = buf;
        }
    } else {
        if(_this->nextReadIndex < _this->length){
            bufnum = _this->nextReadIndex % READBUFFERSIZE;

            if(bufnum == 0){
                _this->lastReadBuffer = _this->lastReadBuffer->next;
            }
            sample = v_readerSample(_this->lastReadBuffer->samples[bufnum]);
        } else {
            sample = NULL;
        }
    }
    _this->nextReadIndex++;

    return sample;
}

C_STRUCT(instanceField)
readStackLastSampleForInstance(
    readStack _this,
    v_dataReaderInstance instance)
{
    instanceBuffer buf;
    C_STRUCT(instanceField) field;
    c_long bufnum = _this->nextIndexInstance % READBUFFERSIZE;

    assert(_this->nextIndexInstance <= _this->instanceLength);

    if(bufnum == 0){
        if(_this->nextIndexInstance == 0){
            _this->lastReadBufferInstance = &_this->instanceBuffer;
        } else {
            _this->lastReadBufferInstance = _this->lastReadBufferInstance->next;
        }
    }
    buf = _this->lastReadBufferInstance;

    if(_this->nextIndexInstance < _this->instanceLength){
        field = buf->instances[bufnum];
        assert(field.lastSample->instance == instance);
    } else {
        assert(_this->nextIndexInstance == _this->instanceLength);
        field.lastSample = _this->prevSample;
        field.sampleIndex = _this->lastSampleIndex;
        assert(field.lastSample->instance == instance);
    }
    _this->nextIndexInstance++;

    return field;
}

#define readStackFreeContents(_this) \
{ \
    readBuffer buf = &(((readStack)_this)->buffer); \
    c_ulong curIndex = 0; \
    c_ulong index; \
 \
    while(curIndex < ((readStack)_this)->length) { \
        index = curIndex % READBUFFERSIZE; \
        if ((index == 0) && (curIndex > 0)) { \
            buf = buf->next; \
        } \
        c_free((buf->samples[index])->instance); \
        c_free(buf->samples[index]); \
        curIndex++; \
    } \
}

#define readStackFree(_this) \
{ \
    readBuffer next,del; \
    instanceBuffer nextInstance,delInstance; \
 \
    next = ((readStack)_this)->buffer.next; \
    while (next) { \
        del = next; \
        next = del->next; \
        os_free(del); \
    } \
    nextInstance = ((readStack)_this)->instanceBuffer.next; \
    while (nextInstance) { \
        delInstance = nextInstance; \
        nextInstance = delInstance->next; \
        os_free(delInstance); \
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
    C_STRUCT(instanceField) field;

    /* The samples sequence now contains all samples read,
     * Now determine the sample info for each sample.
     */
    mrsicDisposed = 0;
    mrsDisposed = 0;
    last = 0;
    vs = GAPI_NEW_VIEW_STATE;
    instance_state = GAPI_ALIVE_INSTANCE_STATE;

    instance = NULL;
    length = samples->length;
    for (i=0; i<length; i++) {
        sample = readStackNextSample(samples);

        if (instance != sample->instance) {
            /* A new instance starts with this sample.
             * Now determine the instance related sampleInfo needed for each
             * sample belonging to this instance.
             */
            first = i;
            instance = sample->instance;
            field = readStackLastSampleForInstance(samples, instance);
            lastSample = v_dataReaderSample(field.lastSample);
            last = field.sampleIndex;

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

        to->instance_handle             = u_instanceHandleNew(v_public(instance));
        to->publication_handle          = u_instanceHandleFromGID(v_dataReaderSample(sample)->publicationHandle);

        to->reception_timestamp.sec       = (gapi_long)(v_dataReaderSample(sample)->insertTime.seconds);
        to->reception_timestamp.nanosec   = (gapi_unsigned_long)(v_dataReaderSample(sample)->insertTime.nanoseconds);
    }
}

c_bool
gapi_matchesReaderMask (
    c_object o,
    c_voidp args)
{
    c_long sampleStateMask, viewStateMask, instanceStateMask;
    c_bool sampleStateFlag, viewStateFlag, instanceStateFlag;
    gapi_readerMask *mask = (gapi_readerMask *) args;
    v_dataReaderInstance instance;
    v_readerSample sample = v_readerSample(o);
    v_state instanceState;
    v_state sampleState;
    c_bool result = FALSE;

    sampleState = v_readerSampleState(sample);
    if ( v_objectKind(sample->instance) == K_DATAVIEWINSTANCE) {
        sample = v_dataViewSampleTemplate(sample)->sample;
    }
    instance = v_dataReaderInstance(sample->instance);
    instanceState = instance->instanceState;

    sampleStateMask   = (c_long)(mask->sampleStateMask);
    viewStateMask     = (c_long)(mask->viewStateMask);
    instanceStateMask = (c_long)(mask->instanceStateMask);

    sampleStateFlag = FALSE;
    if (!sampleStateMask) {
        sampleStateFlag = TRUE;
    } else {
        if (v_stateTestOr(sampleState,L_READ | L_LAZYREAD)) {
            if (v_stateTest(sampleStateMask, GAPI_READ_SAMPLE_STATE)) {
                sampleStateFlag = TRUE;
            }
        } else {
            if (v_stateTest(sampleStateMask, GAPI_NOT_READ_SAMPLE_STATE)) {
                sampleStateFlag = TRUE;
            }
        }
    }

    if (sampleStateFlag) {
        viewStateFlag = FALSE;
        if (!viewStateMask) {
            viewStateFlag = TRUE;
        } else {
            if (v_stateTest(instanceState,L_NEW)) {
                if (v_stateTest(viewStateMask, GAPI_NEW_VIEW_STATE)) {
                    viewStateFlag = TRUE;
                }
            } else {
                if (v_stateTest(viewStateMask, GAPI_NOT_NEW_VIEW_STATE)) {
                    viewStateFlag = TRUE;
                }
            }
        }

        if (viewStateFlag) {
            instanceStateFlag = FALSE;
            if (!instanceStateMask) {
                instanceStateFlag = TRUE;
            } else {
                if (v_stateTest(instanceState,L_DISPOSED)) {
                    if (v_stateTest(instanceStateMask,
                                    GAPI_NOT_ALIVE_DISPOSED_INSTANCE_STATE)) {
                        instanceStateFlag = TRUE;
                    }
                } else if (v_stateTest(instanceState,L_NOWRITERS)) {
                    if (v_stateTest(instanceStateMask,
                                    GAPI_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE)) {
                        instanceStateFlag = TRUE;
                    }
                } else {
                    if (v_stateTest(instanceStateMask,
                                    GAPI_ALIVE_INSTANCE_STATE)) {
                        instanceStateFlag = TRUE;
                    }
                }
            }

            if (instanceStateFlag) {
                result = TRUE;
            }
        }
    }
    return result;
}

static v_actionResult
readerAction (
    c_object o,
    c_voidp copyArg)
{
    readerActionArg     *info   = (readerActionArg *) copyArg;
    readStack           samples = info->samples;
    v_actionResult      result = 0;
    gapi_unsigned_long  length;

    length = samples->length;
    v_actionResultSet(result, V_PROCEED);
    if ( o ) {
        if (gapi_matchesReaderMask(o, &info->reader->reader_mask))
        {
            readStackPush(samples,v_readerSample(o));
            if ( samples->length >= info->max ) {
                v_actionResultClear(result, V_PROCEED);
            }
        } else {
            v_actionResultSet(result, V_SKIP);
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
        v_actionResultClear(result, V_PROCEED);
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
            datareader->reader_mask.sampleStateMask = sample_states;
            datareader->reader_mask.viewStateMask = view_states;
            datareader->reader_mask.instanceStateMask = instance_states;
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
                /*v_readerSample sample;*/

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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
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
            if(reader)
            {
                datareader->reader_mask.sampleStateMask = sample_states;
                datareader->reader_mask.viewStateMask = view_states;
                datareader->reader_mask.instanceStateMask = instance_states;
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
                    /*
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                    */
                    readStackFreeContents(&samples);
                }
                readStackFree(&samples);
            }else
            {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }
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
            if(reader)
            {
                datareader->reader_mask = readcondition->readerMask;
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
                    /*
                    for ( i = 0; i < samples.length; i++ ) {
                        sample = readStackSample(&samples,i);
                        c_free(sample->instance);
                        c_free(sample);
                    }
                    */
                    readStackFreeContents(&samples);
                }
                readStackFree(&samples);
            } else
            {
                result = GAPI_RETCODE_ALREADY_DELETED;
            }

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

            datareader->reader_mask = readcondition->readerMask;
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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
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

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (a_handle == GAPI_HANDLE_NIL) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            datareader->reader_mask.sampleStateMask = sample_states;
            datareader->reader_mask.viewStateMask = view_states;
            datareader->reader_mask.instanceStateMask = instance_states;
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

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerReadInstance(reader,
                                         a_handle,
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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
            }
            readStackFree(&samples);
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

    datareader = gapi_dataReaderClaim(_this, &result);

    assert(data_values);
    assert(info_data);
    assert(max_samples >= -1 );

    if ( datareader != NULL ) {
        if ( !gapi_stateMasksValid(sample_states, view_states, instance_states) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (a_handle == GAPI_HANDLE_NIL) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else if (max_samples == 0) {
            result = GAPI_RETCODE_NO_DATA;
        } else {
            reader = u_reader(U_DATAREADER_GET(datareader));
            datareader->reader_mask.sampleStateMask = sample_states;
            datareader->reader_mask.viewStateMask = view_states;
            datareader->reader_mask.instanceStateMask = instance_states;
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

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerTakeInstance(reader,
                                         a_handle,
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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
            }
            readStackFree(&samples);
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
            datareader->reader_mask.sampleStateMask = sample_states;
            datareader->reader_mask.viewStateMask = view_states;
            datareader->reader_mask.instanceStateMask = instance_states;
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

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerReadNextInstance(reader,
                                             a_handle,
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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
            }
            readStackFree(&samples);
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
            datareader->reader_mask.sampleStateMask = sample_states;
            datareader->reader_mask.viewStateMask = view_states;
            datareader->reader_mask.instanceStateMask = instance_states;
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

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerTakeNextInstance(reader,
                                             a_handle,
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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
            }
            readStackFree(&samples);
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
            reader = u_reader(readcondition->uQuery);
            datareader->reader_mask = readcondition->readerMask;

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

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerReadNextInstance(reader,
                                             a_handle,
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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
            }
            readStackFree(&samples);
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
            reader = u_reader(readcondition->uQuery);
            datareader->reader_mask = readcondition->readerMask;

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

                arg.dataSamples._buffer  = (void *)&initialBuffer;
                arg.dataSamples._length  = 0;
                arg.dataSamples._maximum = INITIALBUFFER_SIZE;
                arg.dataSamples._release = FALSE;

                r = u_readerTakeNextInstance(reader,
                                             a_handle,
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
                /*
                for ( i = 0; i < samples.length; i++ ) {
                    sample = readStackSample(&samples,i);
                    c_free(sample->instance);
                    c_free(sample);
                }
                */
                readStackFreeContents(&samples);
            }
            readStackFree(&samples);
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
    gapi_returnCode_t result = GAPI_RETCODE_OK;
    _DataReader datareader;

    datareader = gapi_dataReaderClaim(_this, &result);

    if ( datareader ) {
        if ( (key_holder == NULL) || (handle == GAPI_HANDLE_NIL) ) {
            result = GAPI_RETCODE_BAD_PARAMETER;
        } else {
            result = _DataReaderGetKeyValue(datareader, key_holder, handle);
        }
    }

    _EntityRelease(datareader);

    return result;
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
        u_result uResult;

        rData.reader = datareader;
        rData.data = (void *)instance_data;

        uResult = u_dataReaderLookupInstance(
                      U_DATAREADER_GET(datareader),
                      &rData,
                      _DataReaderCopyIn,
                      &handle);
    }
    _EntityRelease(datareader);

    return handle;
}

gapi_returnCode_t
gapi_fooDataReader_getInstanceUserData (
    gapi_fooDataReader _this,
    gapi_instanceHandle_t instance,
    c_voidp* data_out)
{
    _DataReader datareader;
    gapi_returnCode_t result;

    datareader = gapi_dataReaderClaim(_this, NULL);
    if ( datareader ) {

        result = kernelResultToApiResult (
                    u_dataReaderGetInstanceUserData(
                    U_DATAREADER_GET(datareader),
                    instance,
                    data_out));
    } else {
    	result = GAPI_RETCODE_BAD_PARAMETER;
    }

    _EntityRelease(datareader);

    return result;
}

gapi_returnCode_t
gapi_fooDataReader_setInstanceUserData (
    gapi_fooDataReader _this,
    gapi_instanceHandle_t instance,
    c_voidp data)
{
    _DataReader datareader;
    gapi_returnCode_t result;

    datareader = gapi_dataReaderClaim(_this, NULL);
    if ( datareader  ) {

        result = kernelResultToApiResult (
                    u_dataReaderSetInstanceUserData(
                    U_DATAREADER_GET(datareader),
                    instance,
                    data));
    } else {
    	result = GAPI_RETCODE_BAD_PARAMETER;
    }

    _EntityRelease(datareader);

    return result;
}

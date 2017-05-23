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

#include "c_typebase.h"
#include "cmn_samplesList.h"
#include "v_dataReaderInstance.h"
#include "v_dataReaderSample.h"
#include "v_dataViewInstance.h"
#include "v_dataViewSample.h"
#include "v_state.h"
#include "os_report.h"


#if 0
#define TRACE_SAMPLEINFO printf
#else
#define TRACE_SAMPLEINFO(...)
#endif

#define READBUFFERSIZE (32)

C_CLASS(cmn_infoField);
C_CLASS(cmn_infoBuffer);
C_CLASS(cmn_readBuffer);
C_CLASS(cmn_readList);

/* This record exists per instance that exists in the read buffer.
 * it contains the instance head, i.e. the first sample received for that instance.
 */
C_STRUCT(cmn_infoField){
    v_readerSample head;
    os_uint32 index;
};

C_STRUCT(cmn_infoBuffer){
    C_STRUCT(cmn_infoField) infoField[READBUFFERSIZE];
    cmn_infoBuffer next;
};

C_STRUCT(cmn_readBuffer) {
    v_readerSample samples[READBUFFERSIZE];
    C_STRUCT(cmn_sampleInfo) infos[READBUFFERSIZE];
    cmn_readBuffer next;
};

C_STRUCT (cmn_readList) {
    os_uint32 readBufferLength;
    C_STRUCT(cmn_readBuffer) readBuffer;
    cmn_readBuffer lastReadBuffer;

    os_uint32 infoBufferLength;
    C_STRUCT(cmn_infoBuffer) infoBuffer;
    cmn_infoBuffer lastInfoBuffer;

    v_readerSample prevSample;
    os_uint32 prevIndex;

    v_instance lastInstance;
    os_uint32 mrsDisposed;

    os_uint32 iterator;
};

C_STRUCT(cmn_samplesList) {
    C_STRUCT(cmn_readList) list;
    os_int32               maxSamples;
    os_boolean             isView;
};



static void
readListInit(
    cmn_readList _this)
{
    _this->readBufferLength = 0;
    _this->readBuffer.next = NULL;
    _this->lastReadBuffer = &_this->readBuffer;
    _this->infoBufferLength = 0;
    _this->infoBuffer.next = NULL;
    _this->lastInfoBuffer = &_this->infoBuffer;
    _this->prevSample = NULL;
    _this->prevIndex = 0;
    _this->lastInstance = NULL;
    _this->mrsDisposed = 0;
}

static void
readListFreeContents(
    cmn_readList _this)
{
    v_readerSample sample;
    cmn_readBuffer buf;
    c_ulong i, index;

    buf = &_this->readBuffer;
    for (i=0; i<_this->readBufferLength; i++) {
        index = i % READBUFFERSIZE;
        if ((index == 0) && (i != 0)) {
            buf = buf->next;
        }
        sample = buf->samples[index];
        /* The instance is kept by the cmn_samplesList_push operation
         * so that it is not freed during the take operation before the data is copied.
         * So now free the instance.
         */
        c_free(v_readerSampleInstance(sample));
        c_free(sample);
    }
    _this->readBufferLength = 0;
}

static void
readListFree(
    cmn_readList _this)
{
    cmn_readBuffer next, del;
    cmn_infoBuffer nextInstance, delInstance;

    next = _this->readBuffer.next;
    _this->readBuffer.next = NULL;
    while (next) {
        del = next;
        next = del->next;
        os_free(del);
    }
    nextInstance = _this->infoBuffer.next;
    _this->infoBuffer.next = NULL;
    while (nextInstance) {
        delInstance = nextInstance;
        nextInstance = delInstance->next;
        os_free(delInstance);
    }
}

cmn_samplesList
cmn_samplesList_new (
    os_boolean isView)
{
    cmn_samplesList list;
    list = os_malloc(sizeof(C_STRUCT(cmn_samplesList)));
    readListInit(&list->list);
    list->maxSamples = 0;
    list->isView = isView;
    return list;
}

void
cmn_samplesList_reset (
    cmn_samplesList _this,
    const os_int32 max_samples)
{
    assert(_this);
    readListFreeContents(&_this->list);
    readListFree(&_this->list);
    readListInit(&_this->list);
    _this->maxSamples = max_samples;
}

os_boolean
cmn_samplesList_insert(
    cmn_samplesList _this,
    v_readerSample sample)
{
    os_boolean result;
    c_long index;
    v_instance instance;
    cmn_infoField field;
    cmn_readList list = &_this->list;

    if ( list->readBufferLength >= (os_uint32)_this->maxSamples ) {
        result = FALSE;
    } else {
        instance = v_readerSampleInstance(sample);
        /* The instance will be used by the copy out operation later on,
         * but a take operation may delete the instance!
         * So keep the instance now and free it in the readListFreeContents
         * operation to keep it alive until the take has copied the data.
         */
         c_keep(instance);

        /*
         * Push Sample into read buffer.
         */
        index = list->readBufferLength % READBUFFERSIZE;
        if ((index == 0) &&                /* Allocate a new buffer */
            (list->readBufferLength > 0)) /* but only after the first pre allocated
                                            * buffer is filled */
        {
            /* Allocate a new buffer */
            list->lastReadBuffer->next = os_malloc(sizeof(C_STRUCT(cmn_readBuffer)));
            list->lastReadBuffer = list->lastReadBuffer->next;
            list->lastReadBuffer->next = NULL;
        }
        list->lastReadBuffer->samples[index] = c_keep(sample);
        list->readBufferLength++;


        /*
         * When a new Instance starts then push head Sample of the
         * previous instance into buffer.
         * The head sample is needed to calculate sample info ranks after the read action.
         */
        if (list->lastInstance != instance) {
            TRACE_SAMPLEINFO("New Instance (0x%x) ", instance);
            list->lastInstance = instance;
            if (list->prevSample != NULL) {       /* The previpus read sample is the head. */
                index = list->infoBufferLength % READBUFFERSIZE;
                if ((index == 0) &&                /* Allocate a new buffer */
                    (list->infoBufferLength > 0)) /* but only after the first pre allocated
                                                    * buffer is filled */
                {
                    /* Allocate a new buffer */
                    list->lastInfoBuffer->next = os_malloc(sizeof(C_STRUCT(cmn_infoBuffer)));
                    list->lastInfoBuffer = list->lastInfoBuffer->next;
                    list->lastInfoBuffer->next = NULL;
                }
                field = &list->lastInfoBuffer->infoField[index];
                field->index = list->readBufferLength-2;
                field->head = list->prevSample;
                TRACE_SAMPLEINFO("Head (0x%x) absolute index = %d\n", field->head, field->index);
                list->infoBufferLength++;
            }
        }
        TRACE_SAMPLEINFO("    Sample[%d] = 0x%x\n", list->readBufferLength-1, sample);
        list->prevSample = sample;
        list->prevIndex = list->readBufferLength-1;
        result = TRUE;
    }
    return result;
}

void
cmn_samplesList_free (
    cmn_samplesList _this)
{
    readListFreeContents(&_this->list);
    readListFree(&_this->list);
    os_free(_this);
}

/* End of the ReadList implementation *******/

static void
cmn_samplesList_finalize_reader (
    cmn_samplesList _this)
{
    os_uint32 i, infoCount, head_index;
    os_int32 mrsDisposed, mrsicDisposed;
    os_int32 count;
    v_dataReaderInstance instance;
    v_readerSample sample, head;
    v_dataReaderSample drSample, headSample;
    v_message message;
    cmn_sampleInfo sampleInfo;
    os_uint32 view_state;
    os_uint32 instance_state;
    cmn_infoBuffer ibuf;
    os_int32 ibufnum;
    cmn_readBuffer rbuf;
    os_int32 rbufnum;

    cmn_readList list = &_this->list;

    if (list->readBufferLength > 0) {

        /* The list sequence now contains all list read,
         * Now determine the sample info for each sample.
         */
        ibufnum = 0;
        ibuf = &list->infoBuffer;
        rbufnum = 0;
        rbuf = &list->readBuffer;
        instance = NULL;
        head_index = 0;
        view_state = DDS_NOT_NEW_VIEW_STATE;
        instance_state = DDS_ALIVE_INSTANCE_STATE;
        infoCount = 0;
        mrsicDisposed = 0;
        mrsDisposed   = 0;

        TRACE_SAMPLEINFO("Determine Sample Info\n");

        for (i=0; i<list->readBufferLength; i++) {
            /*
             * Get Sample from the read buffer
             * and set buffer to the next sample for the next round.
             */
            sample = rbuf->samples[rbufnum];
            sampleInfo = &rbuf->infos[rbufnum];
            rbufnum++;
            rbufnum = rbufnum % READBUFFERSIZE;
            if (rbufnum == 0) {
                rbuf = rbuf->next;
            }
            /*
             * When a new Instance starts get the head Sample from the info buffer
             * and set buffer to the next head Sample for the next round.
             */
            if (instance != v_readerSampleInstance(sample)) {
                if (infoCount < list->infoBufferLength) {
                    head = ibuf->infoField[ibufnum].head;
                    head_index = ibuf->infoField[ibufnum].index;
                } else {
                    head = list->prevSample;
                    head_index = list->prevIndex;
                }
                infoCount++;
                ibufnum++;
                ibufnum = ibufnum % READBUFFERSIZE;
                if (ibufnum == 0) {
                    ibuf = ibuf->next;
                }

                /* Now get information about this instance,
                 * i.e. relative and absolute generation counts, instance state and view state.
                 */
                instance = v_dataReaderInstance(v_readerSampleInstance(sample));

                headSample = v_dataReaderSample(head);
                /*
                 * determine generation counts.
                 */
                mrsicDisposed = headSample->disposeCount + headSample->noWritersCount;
                mrsDisposed   = instance->disposeCount + instance->noWritersCount;

                /*
                 * determine instance state from the head sample.
                 */
                instance_state = DDS_ALIVE_INSTANCE_STATE;
                if (v_dataReaderInstanceStateTest(instance, L_NOWRITERS)) {
                    instance_state = DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
                }
                if (v_dataReaderInstanceStateTest(instance, L_DISPOSED)) {
                    instance_state = DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
                }

                /*
                 * determine view state from the sample.
                 */
                if (v_readerSampleTestState(sample, L_NEW)) {
                    view_state = DDS_NEW_VIEW_STATE;
                } else {
                    view_state = DDS_NOT_NEW_VIEW_STATE;
                }
                TRACE_SAMPLEINFO("Sample[%d] Instance (0x%x) Head (0x%x) "
                                 "absolute index = %d view_state %d instance_state %d\n",
                                 i, instance, head, head_index, view_state, instance_state);
            }

            if(instance != NULL) {
                drSample = v_dataReaderSample(sample);
                /*
                 * Get the SampleInfo record from the output sequence buffer.
                 */
                if (v_readerSampleTestState(sample, L_READ)) {
                    sampleInfo->sample_state = DDS_READ_SAMPLE_STATE;
                } else {
                    sampleInfo->sample_state = DDS_NOT_READ_SAMPLE_STATE;
                }
                sampleInfo->view_state = view_state;
                sampleInfo->instance_state = instance_state;

                sampleInfo->valid_data = v_readerSampleTestState(sample, L_VALIDDATA);
                sampleInfo->disposed_generation_count = drSample->disposeCount;
                sampleInfo->no_writers_generation_count = drSample->noWritersCount;

                count = drSample->disposeCount + drSample->noWritersCount;

                sampleInfo->sample_rank = (os_int32) (head_index - i);
                sampleInfo->generation_rank = mrsicDisposed - count;
                sampleInfo->absolute_generation_rank = mrsDisposed - count;
                sampleInfo->instance_handle = u_instanceHandleNew(v_public(v_readerSampleInstance(sample)));
                sampleInfo->publication_handle = u_instanceHandleFromGID(drSample->publicationHandle);
                sampleInfo->reception_timestamp = drSample->insertTime;

                message = v_dataReaderSampleMessage(drSample);
                sampleInfo->source_timestamp = message->writeTime;

                TRACE_SAMPLEINFO("Sample[%d] sample_rank = %d - %d = %d\t",
                                 i, head_index, i, sampleInfo->sample_rank);
                TRACE_SAMPLEINFO("generation_rank = %d - %d = %d\t",
                                 mrsicDisposed, count, sampleInfo->generation_rank);
                TRACE_SAMPLEINFO("absolute_generation = %d - %d = %d\n\n",
                                 mrsDisposed, count, sampleInfo->absolute_generation_rank);
            }
        }
    }
}

static void
cmn_samplesList_finalize_view (
    cmn_samplesList _this)
{
    os_uint32 i, infoCount, head_index;
    v_dataReaderInstance drInstance;
    v_dataViewInstance instance;
    v_readerSample sample, head;
    v_dataReaderSample drSample;
    v_message message;
    cmn_sampleInfo sampleInfo;
    os_uint32 view_state;
    os_uint32 instance_state;
    cmn_infoBuffer ibuf;
    os_int32 ibufnum;
    cmn_readBuffer rbuf;
    os_int32 rbufnum;

    cmn_readList list = &_this->list;

    if (list->readBufferLength > 0) {

        /* The list sequence now contains all list read,
         * Now determine the sample info for each sample.
         */
        ibufnum = 0;
        ibuf = &list->infoBuffer;
        rbufnum = 0;
        rbuf = &list->readBuffer;
        drInstance = NULL;
        instance = NULL;
        head_index = 0;
        view_state = DDS_NOT_NEW_VIEW_STATE;
        instance_state = DDS_ALIVE_INSTANCE_STATE;
        infoCount = 0;

        TRACE_SAMPLEINFO("Determine Sample Info\n");

        for (i=0; i<list->readBufferLength; i++) {
            /*
             * Get Sample from the read buffer
             * and set buffer to the next sample for the next round.
             */
            sample = rbuf->samples[rbufnum];
            sampleInfo = &rbuf->infos[rbufnum];
            rbufnum++;
            rbufnum = rbufnum % READBUFFERSIZE;
            if (rbufnum == 0) {
                rbuf = rbuf->next;
            }
            drSample = v_dataReaderSample(v_dataViewSampleTemplate(sample)->sample);
            /*
             * When a new Instance starts get the head Sample from the info buffer
             * and set buffer to the next head Sample for the next round.
             */
            if (instance != v_readerSampleInstance(sample)) {
                if (infoCount < list->infoBufferLength) {
                    head = ibuf->infoField[ibufnum].head;
                    head_index = ibuf->infoField[ibufnum].index;
                } else {
                    head = list->prevSample;
                    head_index = list->prevIndex;
                }
                infoCount++;
                ibufnum++;
                ibufnum = ibufnum % READBUFFERSIZE;
                if (ibufnum == 0) {
                    ibuf = ibuf->next;
                }

                /* Now get information about this instance,
                 * i.e. relative and absolute generation counts, instance state and view state.
                 */
                instance = v_dataViewInstance(v_readerSampleInstance(sample));
                drInstance = v_dataReaderInstance(v_readerSampleInstance(drSample));

                /*
                 * determine instance state from the head sample.
                 */
                instance_state = DDS_ALIVE_INSTANCE_STATE;
                if (v_dataReaderInstanceStateTest(drInstance, L_NOWRITERS)) {
                    instance_state = DDS_NOT_ALIVE_NO_WRITERS_INSTANCE_STATE;
                }
                if (v_dataReaderInstanceStateTest(drInstance, L_DISPOSED)) {
                    instance_state = DDS_NOT_ALIVE_DISPOSED_INSTANCE_STATE;
                }

                /*
                 * determine view state from the sample.
                 */
                if (v_readerSampleTestState(sample, L_NEW)) {
                    view_state = DDS_NEW_VIEW_STATE;
                } else {
                    view_state = DDS_NOT_NEW_VIEW_STATE;
                }
                TRACE_SAMPLEINFO("Sample[%d] Instance (0x%x) Head (0x%x) "
                                 "absolute index = %d view_state %d instance_state %d\n",
                                 i, instance, head, head_index, view_state, instance_state);
                (void) head;
            }

            if(instance != NULL) {
                /*
                 * Get the SampleInfo record from the output sequence buffer.
                 */
                if (v_readerSampleTestState(sample, L_READ)) {
                    sampleInfo->sample_state = DDS_READ_SAMPLE_STATE;
                } else {
                    sampleInfo->sample_state = DDS_NOT_READ_SAMPLE_STATE;
                }
                sampleInfo->view_state = view_state;
                sampleInfo->instance_state = instance_state;

                sampleInfo->valid_data = v_readerSampleTestState(sample, L_VALIDDATA);
                sampleInfo->disposed_generation_count = drSample->disposeCount;
                sampleInfo->no_writers_generation_count = drSample->noWritersCount;

                sampleInfo->sample_rank = (os_int32) (head_index - i);
                sampleInfo->generation_rank = 0;
                sampleInfo->absolute_generation_rank = 0;
                sampleInfo->instance_handle = u_instanceHandleNew(v_public(v_readerSampleInstance(sample)));
                sampleInfo->publication_handle = u_instanceHandleFromGID(drSample->publicationHandle);

                sampleInfo->reception_timestamp = drSample->insertTime;

                message = v_dataReaderSampleMessage(drSample);
                sampleInfo->source_timestamp = message->writeTime;

                TRACE_SAMPLEINFO("Sample[%d] sample_rank = %d - %d = %d\t",
                                 i, head_index, i, sampleInfo->sample_rank);
                TRACE_SAMPLEINFO("generation_rank = 0\t");
                TRACE_SAMPLEINFO("absolute_generation = 0\n\n");
            }
        }
    }
}

void
cmn_samplesList_finalize (
    cmn_samplesList _this)
{
    if (_this->isView) {
        cmn_samplesList_finalize_view(_this);
    } else {
        cmn_samplesList_finalize_reader(_this);
    }
}

os_boolean
cmn_samplesList_full (
    cmn_samplesList _this)
{
    return (_this->list.readBufferLength >= (os_uint32)_this->maxSamples);
}



os_boolean
cmn_samplesList_empty (
    cmn_samplesList _this)
{
    return (_this->list.readBufferLength == 0);
}

os_uint32
cmn_samplesList_length(
    cmn_samplesList _this){
    return (_this->list.readBufferLength);
}

os_int32
cmn_samplesList_flush(
    cmn_samplesList _this,
    cmn_sampleList_copy_func copy_action,
    void *copy_arg)
{
    os_uint32 length;
    os_uint32 i;
    os_int32  r;
    cmn_readBuffer buffer;
    v_dataReaderSample sample;
    cmn_sampleInfo sampleInfo;
    v_message message;
    void *data;

    cmn_readList list = &_this->list;

    assert(copy_action);

    buffer = &list->readBuffer;
    length = list->readBufferLength;
    r = (os_int32) length;
    if (length > 0) {
        v_kernelProtectStrictReadOnlyEnter();
        for ( i = 0; i < length; i++ ) {
            os_uint32 bufnum = i % READBUFFERSIZE;
            if ((bufnum == 0) && (i > 0)) {
                buffer = buffer->next;
            }
            if (_this->isView) {
                sample = v_dataReaderSample(v_dataViewSampleTemplate(buffer->samples[bufnum])->sample);
            } else {
                sample = v_dataReaderSample(buffer->samples[bufnum]);
            }
            sampleInfo = &buffer->infos[bufnum];

            message = v_dataReaderSampleMessage(sample);
            data = C_DISPLACE(message, C_SIZEOF(v_message));

            copy_action(data, sampleInfo, copy_arg);
        }
        v_kernelProtectStrictReadOnlyExit();
        readListFreeContents(list);
        readListFree(list);
    }
    return r;
}

os_int32
cmn_samplesList_flush2(
    cmn_samplesList _this,
    cmn_sampleList_copy_func copy_action,
    void *copy_arg)
{
    os_uint32 length;
    os_uint32 i;
    cmn_readBuffer buffer;
    v_dataReaderSample sample;
    cmn_sampleInfo sampleInfo;

    cmn_readList list = &_this->list;

    assert(copy_action);

    buffer = &list->readBuffer;
    length = list->readBufferLength;
    if (length > 0) {
        for ( i = 0; i < length; i++ ) {
            os_uint32 bufnum = i % READBUFFERSIZE;
            if ((bufnum == 0) && (i > 0)) {
                buffer = buffer->next;
            }
            if (_this->isView) {
                sample = v_dataReaderSample(v_dataViewSampleTemplate(buffer->samples[bufnum])->sample);
            } else {
                sample = v_dataReaderSample(buffer->samples[bufnum]);
            }
            sampleInfo = &buffer->infos[bufnum];

            copy_action(sample, sampleInfo, copy_arg);
        }
        readListFreeContents(list);
        readListFree(list);
    }
    return (os_int32) length;
}

os_int32
cmn_samplesList_read(
    cmn_samplesList _this,
    os_uint32 index,
    cmn_sampleList_copy_func copy_action,
    void *copy_arg)
{
    os_uint32 i;
    os_uint32 bufnum;
    os_uint32 bufpos;

    v_dataReaderSample sample;
    cmn_sampleInfo sampleInfo;
    v_message message;
    void *data;

    cmn_readBuffer buffer;
    cmn_readList list = &_this->list;

    assert(copy_action);

    if (index >= list->readBufferLength) {
        /* Can not read a sample outside my buffers. */
        return 0;
    }

    v_kernelProtectStrictReadOnlyEnter();

    bufnum = index / READBUFFERSIZE;
    bufpos = index % READBUFFERSIZE;
    buffer = &list->readBuffer;
    for (i=0; i<bufnum; i++) buffer = buffer->next;

    if (_this->isView) {
        sample = v_dataReaderSample(v_dataViewSampleTemplate(buffer->samples[bufpos])->sample);
    } else {
        sample = v_dataReaderSample(buffer->samples[bufpos]);
    }
    sampleInfo = &buffer->infos[bufpos];

    message = v_dataReaderSampleMessage(sample);
    data = C_DISPLACE(message, C_SIZEOF(v_message));

    copy_action(data, sampleInfo, copy_arg);

    v_kernelProtectStrictReadOnlyExit();

    return 1;
}

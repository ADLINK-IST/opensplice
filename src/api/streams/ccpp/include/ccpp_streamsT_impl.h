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
#ifndef CCPP_STREAMS_T_IMPL_H_
#define CCPP_STREAMS_T_IMPL_H_

#include "streams_ccpp.h"
#include "ccpp_StreamDataWriter_impl.h"
#include "ccpp_StreamDataReader_impl.h"
#include "vortex_os.h"
#include <map>
#include <memory>

namespace DDS {
    namespace Streams {
        template <class T, class TStreamDwInt, class TStreamDcpsDw, class TStream>
        class TStreamDataWriter_impl :
            public virtual TStreamDwInt,
            public StreamDataWriter_impl
        {
        private:
            class WriterStreamInstance {
                public:
                    TStream *sample;
                    DDS::ULong index;
                    WriterStreamInstance(StreamId id, DDS::ULong max_length) : index(0) {
                        sample = new TStream();
                        sample->buffer.length(max_length);
                        sample->buffer.length(0);
                        sample->id = id;
                    }
                    ~WriterStreamInstance() {
                        delete sample;
                    }
            };

            class FlushThrArgs {
                public:
                    os_threadId threadId;
                    os_threadAttr threadAttr;
                    os_cond preempt;
                    bool active;
                    TStreamDataWriter_impl *streamwriter;
                    os_mutex *mtx;
                    DDS::Duration_t delay;

                    FlushThrArgs(TStreamDataWriter_impl *streamwriter) :
                        active(true), streamwriter(streamwriter), mtx(&(streamwriter->writerLock)) {
                        os_threadAttrInit(&(this->threadAttr));
                        os_condInit(&(this->preempt), this->mtx, NULL);
                    }

                    ~FlushThrArgs() {
                        os_condDestroy(&this->preempt);
                    }
            };

            typedef std::map<StreamId, WriterStreamInstance*> StreamsMap;
            os_mutex writerLock;
            TStreamDcpsDw *writer;
            StreamsMap streams;
            WriterStreamInstance *lastUsedStream;
            StreamId lastUsedId;
            FlushThrArgs *flushThread;

        public:
            TStreamDataWriter_impl(
                DDS::Publisher_ptr publisher,
                DDS::DomainId_t domainId,
                DDS::Streams::StreamDataWriterQos &qos,
                DDS::TypeSupport_ptr typeSupport,
                const char *streamName) :
                    StreamDataWriter_impl(publisher, domainId, qos, typeSupport, streamName)
            {
                DDS::Publisher_ptr myPublisher;
                DDS::DataWriter_var parentWriter;

                if (publisher != NULL) {
                    myPublisher = publisher;
                } else {
                    myPublisher = this->publisher.in();
                }
                assert(myPublisher);

                parentWriter = myPublisher->create_datawriter(this->topic,
                    DDS::Streams::default_datawriter_qos,
                    NULL, 0);
                this->writer = TStreamDcpsDw::_narrow(parentWriter.in());

                os_mutexInit(&writerLock, NULL);

                if ((this->qos.flush.max_delay.sec != DDS::DURATION_INFINITE_SEC) &&
                        (this->qos.flush.max_delay.nanosec != DDS::DURATION_INFINITE_NSEC)) {
                    flushThread = new FlushThrArgs(this);
                    os_threadCreate(&(flushThread->threadId), "timebasedflush", &(flushThread->threadAttr), flushThreadRoutine, flushThread);
                } else {
                    flushThread = NULL;
                }

                this->lastUsedStream = NULL;
                this->lastUsedId = 0;
            }

            ~TStreamDataWriter_impl()
            {
                ReturnCode_t result;
                Publisher_var tmp;
                /* To prevent data loss in the writer history when the network queue is full and
                 * the writer is deleted, wait_for_acknowledgements is used
                 */
                Duration_t ack_delay = { 60, 0 };
                bool doReport = true;
                if ((this->flushThread) && (this->flushThread->active)) {
                    this->flushThread->active = false;
                    os_condSignal(&(this->flushThread->preempt));
                    os_threadWaitExit(this->flushThread->threadId, NULL);
                }
                delete this->flushThread;

                os_mutexLock(&writerLock);
                typename StreamsMap::iterator iter = streams.begin();
                while (iter != streams.end()) {
                    delete (*iter).second;
                    iter++;
                }
                streams.clear();
                tmp = writer->get_publisher();
                /* get_publisher returns null if already deleted, which can happen if external publisher is used */
                if (tmp != NULL) {
                    result = DDS::RETCODE_TIMEOUT;
                    while(result == DDS::RETCODE_TIMEOUT) {
                        result = writer->wait_for_acknowledgments(ack_delay);
                        if (doReport && (result == DDS::RETCODE_TIMEOUT)) {
                            OS_REPORT(OS_INFO, "DDS::Streams::StreamDataWriter", result,
                                "Waiting for acknowledgements for 1 minute on StreamDataWriter");
                            doReport = false;
                        }
                    }
                    tmp->delete_datawriter(writer);
                }
                release(this->writer);
                os_mutexUnlock(&writerLock);
                os_mutexDestroy(&writerLock);
            }

            static void*
            flushThreadRoutine(void *arg) {
                assert(arg);
                os_result result;
                FlushThrArgs *ta = (FlushThrArgs*)arg;
                WriterStreamInstance *stream;
                os_duration delay;
                os_mutexLock(ta->mtx);
                while (ta->active) {
                    delay = OS_DURATION_INIT(ta->streamwriter->qos.flush.max_delay.sec,
                                             ta->streamwriter->qos.flush.max_delay.nanosec);

                    result = os_condTimedWait(&(ta->preempt), ta->mtx, delay);

                    /* Only flush all streams if delay expires. If the condition is triggered
                     * before delay expiry it means the QoS has changed and streams are already flushed.
                     */
                    if (result == os_resultTimeout) {
                        typename StreamsMap::iterator iter;
                        for (iter = ta->streamwriter->streams.begin(); iter != ta->streamwriter->streams.end(); ++iter) {
                            stream = (*iter).second;
                            if (stream->index != 0) {
                                DDS::ReturnCode_t retval;
                                retval = ta->streamwriter->doFlush((*iter).second);
                                if (retval != DDS::RETCODE_OK) {
                                    OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter", retval,
                                            "Auto-flush of stream with id %d failed", (*iter).first);
                                }
                            }
                        }
                    } else if (result != os_resultSuccess) {
                        OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter", result,
                                "Auto-flush thread error occurred (%s). Normal operation will continue but streams have not been flushed!",
                                os_resultImage(result));
                    }
                }
                os_mutexUnlock(ta->mtx);
                return NULL;
            }

            DDS::ReturnCode_t
            append(
                StreamId id,
                const T &data)
            {
                WriterStreamInstance *stream;
                ReturnCode_t result;

                /* Lock stream access */
                os_mutexLock(&writerLock);

                stream = resolveStream(id);
                assert(stream);

                /* Append data to stream */
                assert(stream->sample->buffer.length() < stream->sample->buffer.maximum());
                stream->sample->buffer.length(stream->index + 1);
                stream->sample->buffer[stream->index] = data;
                stream->index++;

                /* Check if auto-flush is required */
                if (stream->index == this->qos.flush.max_samples) {
                    result = doFlush(stream);
                } else {
                    result = DDS::RETCODE_OK;
                }

                /* Unlock stream access */
                os_mutexUnlock(&writerLock);
                return result;
            }

            /* Assumes StreamWriter mutex is locked */
            ::DDS::ReturnCode_t
            doFlush(
                WriterStreamInstance *stream)
            {
                DDS::ReturnCode_t result;

                /* Write stream sample */
                result = this->writer->write(*(stream->sample), DDS::HANDLE_NIL);
                if (result == DDS::RETCODE_OK) {
                    stream->sample->buffer.length(0);
                    stream->index = 0;
                } else if (result != DDS::RETCODE_TIMEOUT) {
                    OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter", result,
                        "Failed to flush stream with id %d", stream->sample->id);
                }
                return result;
            }

            DDS::ReturnCode_t
            flush(
                StreamId id)
            {
                DDS::ReturnCode_t result;
                WriterStreamInstance *stream;

                os_mutexLock(&writerLock);

                /* Check if stream exists */
                typename StreamsMap::iterator found = streams.find(id);
                if (found != streams.end()) {
                    stream = found->second;
                } else {
                    return DDS::RETCODE_PRECONDITION_NOT_MET;
                }

                /* Flush stream */
                result = doFlush(stream);

                os_mutexUnlock(&writerLock);
                return result;
            }

            ::DDS::ReturnCode_t
            set_qos(
                const StreamDataWriterQos &qos)
            {
                DDS::ReturnCode_t result = DDS::RETCODE_OK;
                bool flushDelayUpdated = false;
                WriterStreamInstance *stream = 0;

                /* Auto-flush delay cannot be <= 0 */
                if ((qos.flush.max_delay.sec < 0) ||
                    ((qos.flush.max_delay.sec == 0) &&
                    (qos.flush.max_delay.nanosec == 0))) {
                    return DDS::RETCODE_BAD_PARAMETER;
                }

                /* Max_samples cannot be <= 0 */
                if (qos.flush.max_samples == 0) {
                    return DDS::RETCODE_BAD_PARAMETER;
                }

                os_mutexLock(&writerLock);

                /* If QoS is changed, all streams will be flushed before applying the new QoS */
                if ((qos.flush.max_delay.sec != this->qos.flush.max_delay.sec) ||
                    (qos.flush.max_delay.nanosec != this->qos.flush.max_delay.nanosec) ||
                    (qos.flush.max_samples != this->qos.flush.max_samples)) {
                    typename StreamsMap::iterator iter;
                    for (iter = streams.begin(); (iter != streams.end()) && (result == DDS::RETCODE_OK); ++iter) {
                        stream = (*iter).second;
                        result = doFlush(stream);
                        /* update buffer length */
                        if (result == DDS::RETCODE_OK &&
                                (qos.flush.max_samples != this->qos.flush.max_samples)) {
                            stream->sample->buffer.replace(0, 0, NULL, 1);
                            stream->sample->buffer.length(qos.flush.max_samples);
                        }
                    }
                    if (result != DDS::RETCODE_OK) {
                        OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter", result,
                                "Failed to apply QoS: Stream with ID %d cannot be flushed",
                                stream->sample->id);
                        result = DDS::RETCODE_PRECONDITION_NOT_MET;
                    } else {
                        /* Determine if auto-flush thread must be created or updated */
                        if ((qos.flush.max_delay.sec != this->qos.flush.max_delay.sec) ||
                                (qos.flush.max_delay.nanosec != this->qos.flush.max_delay.nanosec)) {
                            flushDelayUpdated = true;
                        }

                        /* Apply new QoS */
                        this->qos = qos;

                        /* Update auto-flush thread */
                        if (flushDelayUpdated) {
                            if (this->flushThread) {
                                os_condSignal(&(this->flushThread->preempt));
                            } else {
                                flushThread = new FlushThrArgs(this);
                                os_threadCreate(&(flushThread->threadId), "", &(flushThread->threadAttr), flushThreadRoutine, flushThread);
                            }
                        }
                    }
                }
                os_mutexUnlock(&writerLock);

                return result;
            }

            /* Assumes StreamWriter mutex is locked */
            WriterStreamInstance*
            resolveStream(StreamId id) {
                WriterStreamInstance *stream;
                if (this->lastUsedStream && (id == this->lastUsedId)) {
                    stream = this->lastUsedStream;
                } else {
                    /* Lookup stream, lower_bound provides hint for insert if stream doesn't exist */
                    typename StreamsMap::iterator lb = streams.lower_bound(id);
                    if (lb != streams.end() && !(streams.key_comp()(id, lb->first))) {
                        stream = lb->second;
                    } else {
                        stream = new WriterStreamInstance(id, this->qos.flush.max_samples);
                        streams.insert(lb, typename StreamsMap::value_type(id, stream));
                    }
                    this->lastUsedStream = stream;
                    this->lastUsedId = id;
                }

                return stream;
            }
        };

        template <class T, class TStreamDrInt, class TStreamDcpsDr, class TStream, class TFilter, class TStreamBuf, class TStreamSeq>
        class TStreamDataReader_impl :
            public virtual TStreamDrInt,
            public StreamDataReader_impl
        {
            private:
                class ReaderStreamInstance {
                    public:
                        DDS::ULong index;
                        DDS::ConditionSeq conditionList;
                        TStreamSeq sampleCache;
                        SampleInfoSeq infoCache;
                        DDS::ReadCondition_var qc;
                        DDS::GuardCondition_var gc;
                        DDS::WaitSet_var ws;
                        DDS::InstanceHandle_t handle;
                        ReaderStreamInstance(StreamId id, TStreamDcpsDr *reader, DDS::GuardCondition_var readerGuard) :
                                index(0), conditionList(2), sampleCache(1), infoCache(1)
                        {
                            ReturnCode_t result;
                            StringSeq params;
                            params.length(0);
                            char query[255];
                            os_sprintf(query, "id = %d", id);
                            qc = reader->create_querycondition(
                                DDS::ANY_VIEW_STATE, DDS::ANY_SAMPLE_STATE, DDS::ANY_INSTANCE_STATE,
                                query, params);
                            gc = readerGuard;

                            conditionList.length(2);
                            conditionList[0] = NULL;
                            conditionList[1] = NULL;

                            ws = new WaitSet();
                            result = ws->attach_condition(gc.in());
                            if (result == DDS::RETCODE_OK) {
                                result = ws->attach_condition(qc.in());
                                if (result == DDS::RETCODE_OK) {
                                    sampleCache.length(1);
                                    infoCache.length(1);
                                    sampleCache[0].id = id;
                                    sampleCache[0].buffer.length(0);
                                    handle = reader->lookup_instance(sampleCache[0]);
                                }
                            }
                        }

                        ~ReaderStreamInstance()
                        {
                            DDS::ReturnCode_t result;
                            DataReader_var tmp;
                            tmp = qc->get_datareader();
                            result = ws->detach_condition(qc.in());
                            assert(result == DDS::RETCODE_OK || result == DDS::RETCODE_ALREADY_DELETED);

                            result = ws->detach_condition(gc.in());
                            assert(result == DDS::RETCODE_OK);
                            if (tmp) {
                                result = tmp->delete_readcondition(qc.in());
                                assert(result == DDS::RETCODE_OK);
                            }
                            /* Satisfy compiler when building release. */
                            OS_UNUSED_ARG(result);
                        }
                };

                os_mutex readerLock;
                TStreamDcpsDr *reader;
                typedef std::map<StreamId, ReaderStreamInstance*> StreamsMap;
                ReaderStreamInstance *lastUsedStream;
                StreamId lastUsedId;
                StreamsMap streams;
                DDS::GuardCondition_var gc;

            public:
                TStreamDataReader_impl(
                    DDS::Subscriber_ptr subscriber,
                    DDS::DomainId_t domainId,
                    DDS::Streams::StreamDataReaderQos &sqos,
                    DDS::TypeSupport_ptr typeSupport,
                    const char *streamName) :
                        StreamDataReader_impl(
                            subscriber, domainId, sqos, typeSupport, streamName)
                {
                    DDS::Subscriber_ptr mySubscriber;
                    DDS::DataReader_var parentReader;

                    os_mutexInit(&readerLock, NULL);

                    if (subscriber != NULL) {
                        mySubscriber = subscriber;
                    } else {
                        mySubscriber = this->subscriber.in();
                    }
                    parentReader = mySubscriber->create_datareader(
                        this->topic, DDS::Streams::default_datareader_qos, NULL, 0);
                    if(!parentReader.in()) {
                        throw StreamsException("Failed to create internal datareader", DDS::RETCODE_ERROR);
                    }
                    this->reader = TStreamDcpsDr::_narrow(parentReader.in());
                    this->lastUsedStream = NULL;
                    this->lastUsedId = 0;
                    this->gc = new DDS::GuardCondition();
                }

                ~TStreamDataReader_impl()
                {
                    Subscriber_var tmp;

                    os_mutexLock(&readerLock);
                    typename StreamsMap::iterator iter = streams.begin();
                    while (iter != streams.end()) {
                        delete (*iter).second;
                        iter++;
                    }
                    streams.clear();

                    tmp = reader->get_subscriber();
                    /* get_sublisher returns null if already deleted, which can happen if external publisher is used */
                    if (tmp != NULL) {
                        tmp->delete_datareader(reader);
                    }
                    release(this->reader);
                    os_mutexUnlock(&readerLock);
                    os_mutexDestroy(&readerLock);
                }

                DDS::ReturnCode_t
                get(
                    StreamId id,
                    TStreamBuf &data_values,
                    DDS::Long max_samples,
                    const DDS::Duration_t & timeout)
                {
                    return get_w_filter(id, data_values, max_samples, timeout, NULL);
                }

                DDS::ReturnCode_t
                get_w_filter(
                    StreamId id,
                    TStreamBuf &data_values,
                    DDS::Long max_samples,
                    const DDS::Duration_t & timeout,
                    TFilter a_filter)
                {
                    ReaderStreamInstance *stream;
                    DDS::ReturnCode_t result;
                    SampleInfoSeq sampleInfo;
                    TStreamSeq dataSeq;

                    if ((max_samples > static_cast<DDS::Long>(data_values.maximum())) ||
                        ((data_values.maximum() > 0) && (data_values.release() == FALSE))) {
                        return DDS::RETCODE_PRECONDITION_NOT_MET;
                    }

                    os_mutexLock(&readerLock);

                    /* Lookup stream, lower_bound provides hint for insert if stream doesn't exist */
                    if (this->lastUsedStream && (id == this->lastUsedId)) {
                        stream = this->lastUsedStream;
                    } else {
                        typename StreamsMap::iterator lb = streams.lower_bound(id);
                        if (lb != streams.end() && !(streams.key_comp()(id, lb->first))) {
                            stream = lb->second;
                        } else {
                            stream = new ReaderStreamInstance(id, this->reader, this->gc);
                            streams.insert(lb, typename StreamsMap::value_type(id, stream));
                        }
                    }
                    assert(stream);

                    result = DDS::RETCODE_OK;

                    /* Read a new stream sample */
                    if (stream->sampleCache[0].buffer.length() == stream->index) {
                        stream->index = 0;

                        if (stream->handle == DDS::HANDLE_NIL) {
                            /* First attempt to read this stream: wait for instance to become available */
                            result = stream->ws->wait(stream->conditionList, timeout);
                            if (result == DDS::RETCODE_OK &&
                                (stream->conditionList[0] != this->gc)) {
                                /* Lookup instance-handle, and take data */
                                stream->handle = reader->lookup_instance(stream->sampleCache[0]);
                                assert(stream->handle != DDS::HANDLE_NIL);
                                result = this->reader->take_instance(stream->sampleCache, stream->infoCache, 1, stream->handle,
                                    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                            }
                        } else {
                            /* Use instance-handle to take data if available */
                            result = this->reader->take_instance(stream->sampleCache, stream->infoCache, 1, stream->handle,
                                DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                            if (result == DDS::RETCODE_NO_DATA) {
                                /* Wait for data to become available */
                                result = stream->ws->wait(stream->conditionList, timeout);
                                if ((result == DDS::RETCODE_OK) &&
                                    (stream->conditionList[0] != this->gc)) {
                                    /* Take data */
                                    result = this->reader->take_instance(stream->sampleCache, stream->infoCache, 1, stream->handle,
                                        DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                                }
                            } else if(result == DDS::RETCODE_ALREADY_DELETED) {
                                /* If take_instance returns already deleted, the instance is possibly gone. Following code
                                 * resets the handle and the next time get_* is called the instance will be looked up
                                 * again. */
                                stream->handle = DDS::HANDLE_NIL;
                            }
                        }

                        if (result == DDS::RETCODE_TIMEOUT) {
                            result = DDS::RETCODE_NO_DATA;
                            data_values.length(0);
                            if (stream->sampleCache.length() == 0) {
                                stream->sampleCache.length(1);
                                assert(stream->infoCache.length() == 0);
                                stream->infoCache.length(1);
                            }
                            /* Forget the old buffer when the waitset timed out. */
                            stream->sampleCache[0].buffer.length(0);
                            os_mutexUnlock(&readerLock);
                            return result;
                        }
                    }

                    /* Decide on how many samples to return, based on what's available */
                    DDS::ULong max = stream->sampleCache[0].buffer.length() - stream->index;
                    if ((max_samples != DDS::LENGTH_UNLIMITED) &&
                        (max_samples < static_cast<DDS::Long>(max))) {
                        max = max_samples;
                    }

                    /* Check if the return buffer size needs to be increased */
                    if(max > data_values.length()) {
                        if (max <= data_values.maximum()) {
                            data_values.length(max);
                        } else {
                            data_values.length(data_values.maximum());
                            max = data_values.maximum();
                        }
                    }

                    DDS::ULong j = 0;
                    for (DDS::ULong i = stream->index; (j < max) && (i < stream->sampleCache[0].buffer.length()); i++) {
                        if ((a_filter == NULL) || (a_filter && a_filter->match_data(stream->sampleCache[0].buffer[i]))) {
                            data_values[j++] = stream->sampleCache[0].buffer[i];
                        }
                        stream->index++;
                    }

                    /* If a filter was used and < max samples were matched, the buffer may need to be truncated */
                    data_values.length(j);
                    if (j == 0) {
                        result = DDS::RETCODE_NO_DATA;
                    }

                    os_mutexUnlock(&readerLock);

                    return result;
                }

                DDS::ReturnCode_t
                return_loan(
                    TStreamBuf &data_values) {
                    (void)data_values;
                    return DDS::RETCODE_OK;
                }

                DDS::ReturnCode_t
                set_qos(
                    const ::DDS::Streams::StreamDataReaderQos &qos) {
                    this->qos = qos;
                    return DDS::RETCODE_OK;
                }

                DDS::ReturnCode_t
                interrupt() {
                    return this->gc->set_trigger_value(true);
                }
        };
    }
}

#endif /* CCPP_STREAMS_T_IMPL_H_ */

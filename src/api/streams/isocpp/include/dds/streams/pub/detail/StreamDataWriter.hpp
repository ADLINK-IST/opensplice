#ifndef DDS_STREAMS_PUB_DETAIL_STREAMDATAWRITER_HPP_
#define DDS_STREAMS_PUB_DETAIL_STREAMDATAWRITER_HPP_
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

/**
 * @file
 */

#include <dds/dds.hpp>
#include <dds/streams/pub/qos/StreamDataWriterQos.hpp>
#include <org/opensplice/streams/core/policy/DefaultQos.hpp>
#include <org/opensplice/streams/topic/TopicTraits.hpp>
#include <org/opensplice/core/exception_helper.hpp>
#include <map>

namespace dds
{
namespace streams
{
namespace pub
{
namespace detail
{

template <typename T>
class StreamDataWriter : public org::opensplice::core::EntityDelegate
{
public:
    typedef typename org::opensplice::streams::topic::stream_topic<T>::type StreamT;

private:
    struct stream_t
    {
        std::size_t index;
        StreamT sample;
        DDS::InstanceHandle_t handle;
    };

    enum writer_thread_state_t
    {
        WRITER_THREAD_STOPPED,
        WRITER_THREAD_STOPPING,
        WRITER_THREAD_STARTING,
        WRITER_THREAD_RUNNING
    };

public:

    StreamDataWriter(const std::string& stream_name,
                        const dds::streams::pub::qos::StreamDataWriterQos& qos)
    : thread_state(WRITER_THREAD_STOPPED), qos_(qos), dp_(org::opensplice::domain::default_id()), pub_(dp_), topic_(dds::core::null)
    {
        init(stream_name);
    }

    StreamDataWriter(uint32_t domain_id,
                        const std::string& stream_name,
                        const dds::streams::pub::qos::StreamDataWriterQos& qos)
    : thread_state(WRITER_THREAD_STOPPED), qos_(qos), dp_(domain_id), pub_(dp_), topic_(dds::core::null)
    {
        init(stream_name);
    }

    StreamDataWriter(const dds::pub::Publisher& publisher,
                        const std::string& stream_name,
                        const dds::streams::pub::qos::StreamDataWriterQos& qos)
    : thread_state(WRITER_THREAD_STOPPED), qos_(qos), dp_(org::opensplice::domain::default_id()), pub_(publisher), topic_(dds::core::null)
    {
        init(stream_name);
    }

    ~StreamDataWriter()
    {
        os_mutexLock(&mutex);
        stop_timed_flush();
        os_mutexUnlock(&mutex);
        os_mutexDestroy(&mutex);
    }

    void stream(uint32_t id)
    {
        if(streams_.count(id) == 0)
        {
            stream_ = &streams_[id];
            stream_->index = 0;
            stream_->sample.id = id;
            stream_->handle = raw_writer_->register_instance(stream_->sample);
        }
        else
        {
            stream_ = &streams_[id];
        }
    }

    uint32_t stream()
    {
        return stream_->sample.id;
    }

    void append(const T& data)
    {
        DDS::ReturnCode_t result = DDS::RETCODE_OK;

        os_mutexLock(&mutex);

        //Append the data sample to the stream
        stream_->sample.buffer.length(stream_->index + 1);
        stream_->sample.buffer[stream_->index] = data;
        stream_->index++;
        //If the max samples specified by the qos has been reached, flush the current stream
        if(stream_->sample.buffer.length() >= qos_.policy<dds::streams::core::policy::StreamFlush>().max_samples())
        {
            result = do_flush(stream_);
        }

        os_mutexUnlock(&mutex);

        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("append"));

    }

    void flush()
    {
        os_mutexLock(&mutex);

        DDS::ReturnCode_t result = do_flush(stream_);

        os_mutexUnlock(&mutex);

        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("append"));
    }



    const dds::streams::pub::qos::StreamDataWriterQos& qos() const
    {
        return qos_;
    }

    void qos(const dds::streams::pub::qos::StreamDataWriterQos& qos)
    {
        //If the qos has changed
        if(qos != qos_)
        {
            DDS::ReturnCode_t result = DDS::RETCODE_OK;
            os_mutexLock(&mutex);

            //If the max delay has changed and the thread is active, stop the thread
            if(!(qos.policy<dds::streams::core::policy::StreamFlush>().max_delay() == qos_.policy<dds::streams::core::policy::StreamFlush>().max_delay()))
            {
                stop_timed_flush();
            }

            //Flush all streams and set the new qos
            result = do_flush_all();
            if (result == DDS::RETCODE_OK)
            {
                qos_ = qos;

                //If the max delay is not infinite and the thread is not active, start the thread
                if(!(qos_.policy<dds::streams::core::policy::StreamFlush>().max_delay() == dds::core::Duration::infinite()))
                {
                    start_timed_flush();
                }
            }

            os_mutexUnlock(&mutex);

            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("set qos"));
        }
    }

private:

    DDS::ReturnCode_t do_flush(stream_t* flush_stream)
    {
        DDS::ReturnCode_t result = DDS::RETCODE_OK;

        //If the specified stream has samples, write the stream sample and clear the stream
        if(flush_stream->sample.buffer.length() > 0)
        {
            result = raw_writer_->write(flush_stream->sample, DDS::HANDLE_NIL);
            if (result == DDS::RETCODE_OK) {
                flush_stream->index = 0;
                flush_stream->sample.buffer.length(0);
            }
        }

        return result;
    }

    DDS::ReturnCode_t do_flush_all()
    {
        DDS::ReturnCode_t result = DDS::RETCODE_OK;

        //Flush all streams
        for(typename std::map<uint32_t, stream_t>::iterator iter = streams_.begin(); iter != streams_.end(); iter++)
        {
            DDS::ReturnCode_t r = do_flush(&iter->second);
            if (r != DDS::RETCODE_OK) {
                result = r;
                if (result != DDS::RETCODE_TIMEOUT) {
                    break;
                }
            }
        }

        return result;
    }

    void init(const std::string& stream_name)
    {
        dds::topic::qos::TopicQos topic_qos = org::opensplice::streams::core::policy::default_topic_qos();
        topic_ = dds::topic::Topic<StreamT>(pub_.participant(), stream_name, topic_qos, NULL, dds::core::status::StatusMask::none());

        dds::pub::qos::DataWriterQos dw_qos = org::opensplice::streams::core::policy::default_datawriter_qos();
        DDS::DataWriter_var w = pub_->pub_->create_datawriter(topic_->t_,
                                                                org::opensplice::pub::qos::convertQos(dw_qos),
                                                                NULL, 0);
        if(w.in() == 0)
            throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                OSPL_CONTEXT_LITERAL(
                    "dds::core::NullReferenceError : Unable to create DataWriter. "
                    "Nil return from ::create_datawriter")));

        raw_writer_ = org::opensplice::topic::topic_data_writer<StreamT>::type::_narrow(w.in());
        writer_ = org::opensplice::core::DDS_DW_REF(raw_writer_, org::opensplice::core::DWDeleter(pub_->pub_));
        entity_ = DDS::Entity::_narrow(raw_writer_);

        os_mutexInit(&mutex, NULL);
        os_condInit(&preempt, &mutex, NULL);

        //If a non-infinite max delay has been set, start a thread to handle the timed flush
        if(!(qos_.policy<dds::streams::core::policy::StreamFlush>().max_delay() == dds::core::Duration::infinite()))
        {
            start_timed_flush();
        }

        stream_ = &streams_[0];
        stream_->index = 0;
        stream_->sample.id = 0;
        stream_->handle = raw_writer_->register_instance(stream_->sample);
    }

    void* timed_flush()
    {
        os_duration delay;
        DDS::ReturnCode_t result = DDS::RETCODE_OK;
        bool reportTimeout = true;

        //While the qos has a non-infinite max delay set
        os_mutexLock(&mutex);
        thread_state = WRITER_THREAD_RUNNING;
        os_condBroadcast(&preempt);

        while ((thread_state == WRITER_THREAD_RUNNING) && (result == DDS::RETCODE_OK) &&
                !(qos_.template policy<dds::streams::core::policy::StreamFlush>().max_delay() == dds::core::Duration::infinite()))
        {
            //Get the max_delay duration
            delay = OS_DURATION_INIT(qos_.template policy<dds::streams::core::policy::StreamFlush>().max_delay().sec(),
                                     qos_.template policy<dds::streams::core::policy::StreamFlush>().max_delay().nanosec());

            //Wait until the max_delay duration has elapsed
            os_result ores = os_condTimedWait(&preempt, &mutex, delay);
            if ((ores == os_resultSuccess) || (ores == os_resultTimeout))
            {
                if (thread_state == WRITER_THREAD_RUNNING)
                {
                    //Flush all streams
                    result = do_flush_all();
                    if (result == DDS::RETCODE_OK)
                    {
                        reportTimeout = true;
                    }
                    else if (result == DDS::RETCODE_TIMEOUT)
                    {
                        result = DDS::RETCODE_OK;
                        if (reportTimeout)
                        {
                            reportTimeout = false;
                            OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter::timed_flush", result,
                                    "Failed to flush the stream buffers");
                        }
                    }
                }
            } else {
                OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter::timed_flush", ores,
                        "The operation os_condTimedWait failed");
            }
        }

        thread_state = WRITER_THREAD_STOPPED;
        os_condBroadcast(&preempt);
        os_mutexUnlock(&mutex);

        return 0;
    }

    static void *timed_flush_wrapper(void *arg)
    {
        dds::streams::pub::detail::StreamDataWriter<T>* writer = (dds::streams::pub::detail::StreamDataWriter<T>*)arg;
        return writer->timed_flush();
    }

    void start_timed_flush()
    {
        os_duration timeout = 200*OS_DURATION_MILLISECOND;
        int count = 10;

        while ((thread_state == WRITER_THREAD_STOPPING) && (count > 0))
        {
            (void)os_condTimedWait(&preempt, &mutex, timeout);
            count--;
        }

        if (thread_state == WRITER_THREAD_STOPPED)
        {
            os_threadAttr thread_attr;

            os_threadAttrInit(&thread_attr);

            thread_state = WRITER_THREAD_STARTING;
            os_result ores = os_threadCreate(&thread_id_, "timed_flush", &thread_attr, timed_flush_wrapper, (void*)this);
            if (ores != os_resultSuccess)
            {
                thread_state = WRITER_THREAD_STOPPED;
                OS_REPORT(OS_ERROR, "DDS::Streams::StreamDataWriter::start_timed_flush", ores,
                        "Failed to start background thread");
            }
        }

        count = 10;
        while ((thread_state == WRITER_THREAD_STARTING) && (count > 0))
        {
            (void)os_condTimedWait(&preempt, &mutex, timeout);
            count--;
        }
    }

    void stop_timed_flush()
    {
        os_duration timeout = 100*OS_DURATION_SECOND;
        int count = 10;

        while ((thread_state == WRITER_THREAD_STARTING) && (count > 0))
        {
            (void)os_condTimedWait(&preempt, &mutex, timeout);
            count--;
        }

        if (thread_state == WRITER_THREAD_RUNNING)
        {
            os_threadId tid = thread_id_;
            thread_state = WRITER_THREAD_STOPPING;
            os_condBroadcast(&preempt);
            os_mutexUnlock(&mutex);
            os_threadWaitExit(tid, 0);
            os_mutexLock(&mutex);
        }
    }

private:
    writer_thread_state_t thread_state;
    os_mutex mutex;
    os_cond preempt;
    os_threadId thread_id_;
    dds::streams::pub::qos::StreamDataWriterQos qos_;
    dds::domain::DomainParticipant dp_;
    dds::pub::Publisher pub_;
    dds::topic::Topic<StreamT> topic_;
    org::opensplice::core::DDS_DW_REF writer_;
    typename org::opensplice::topic::topic_data_writer<StreamT>::type* raw_writer_;
    std::map<uint32_t, stream_t> streams_;
    stream_t* stream_;
};

}
}
}
}

#endif /* DDS_STREAMS_PUB_DETAIL_STREAMDATAWRITER_HPP_ */

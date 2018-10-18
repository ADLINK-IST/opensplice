#ifndef DDS_STREAMS_SUB_DETAIL_STREAMDATAREADER_HPP_
#define DDS_STREAMS_SUB_DETAIL_STREAMDATAREADER_HPP_
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
#include <dds/streams/sub/qos/StreamDataReaderQos.hpp>
#include <dds/streams/sub/StreamLoanedSamples.hpp>
#include <org/opensplice/streams/topic/TopicTraits.hpp>
#include <org/opensplice/streams/core/policy/DefaultQos.hpp>
#include <map>
#include <sstream>

namespace dds
{
namespace streams
{
namespace sub
{
namespace functors
{
namespace detail
{
    class MaxSamplesManipulatorFunctor
    {
    public:
        MaxSamplesManipulatorFunctor(uint32_t n) :
        n_(n)
        {
        }

        template<typename S>
        void operator()(S& s)
        {
            s.max_samples(n_);
        }
    private:
        uint32_t n_;
    };

    class TimeoutManipulatorFunctor
    {
    public:
        TimeoutManipulatorFunctor(const dds::core::Duration& timeout) :
        timeout_(timeout)
        {
        }

        template<typename S>
        void operator()(S& s)
        {
            s.timeout(timeout_);
        }
    private:
        dds::core::Duration timeout_;
    };

    template <typename T>
    class FilterManipulatorFunctor
    {
    public:
        FilterManipulatorFunctor(bool (*filter_func)(T)) :
        filter_func_(filter_func)
        {
        }

        template<typename S>
        void operator()(S& s)
        {
            s.filter(filter_func_);
        }
    private:
        bool (*filter_func_)(T);
    };
}
}
}
}
}

namespace dds
{
namespace streams
{
namespace sub
{
namespace detail
{

template <typename T>
class StreamDataReader : public org::opensplice::core::EntityDelegate
{
public:
    typedef typename org::opensplice::streams::topic::stream_topic<T>::type StreamT;

    StreamDataReader(const std::string& stream_name,
                     const dds::streams::sub::qos::StreamDataReaderQos& qos) :
                     stream_id_(1), qos_(qos), dp_(org::opensplice::domain::default_id()), sub_(dp_), topic_(dds::core::null)
    {
        init(stream_name);
    }

    StreamDataReader(uint32_t domain_id, const std::string& stream_name,
                    const dds::streams::sub::qos::StreamDataReaderQos& qos) :
                    stream_id_(1), qos_(qos), dp_(domain_id), sub_(dp_), topic_(dds::core::null)
    {
        init(stream_name);
    }

    StreamDataReader(const dds::sub::Subscriber& subscriber, const std::string& stream_name,
                     const dds::streams::sub::qos::StreamDataReaderQos& qos) :
                     stream_id_(1), qos_(qos), dp_(subscriber.participant()), sub_(subscriber), topic_(dds::core::null)
    {
        init(stream_name);
    }

    void init(const std::string& stream_name)
    {
        dds::topic::qos::TopicQos topic_qos = org::opensplice::streams::core::policy::default_topic_qos();
        topic_ = dds::topic::Topic<StreamT>(dp_, stream_name, topic_qos, NULL, dds::core::status::StatusMask::none());

        dds::sub::qos::DataReaderQos dr_qos = org::opensplice::streams::core::policy::default_datareader_qos();
        DDS::DataReader_var r = sub_->sub_->create_datareader(topic_->t_,
                                                              org::opensplice::sub::qos::convertQos(dr_qos),
                                                              NULL,
                                                              0);

        if(r.in() == 0) throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
            OSPL_CONTEXT_LITERAL(
                "dds::core::NullReferenceError : Unable to create DataReader. "
                "Nil return from ::create_datareader")));

        raw_reader_ = org::opensplice::topic::topic_data_reader<StreamT>::type::_narrow(r.in());
        reader_ = org::opensplice::core::DDS_DR_REF(raw_reader_, org::opensplice::core::DRDeleter(sub_->sub_));
        entity_ = DDS::Entity::_narrow(raw_reader_);

        stream(0);
    }

    class Selector
    {
    public:
        Selector(StreamDataReader<T>* sdr) :
            sdr_(sdr), max_samples_(dds::core::LENGTH_UNLIMITED), timeout_(dds::core::Duration::zero()),
            filter_func_(0)
        {
        }

        Selector& max_samples(uint32_t max_samples)
        {
            max_samples_ = max_samples;
            return *this;
        }

        Selector& timeout(const dds::core::Duration& timeout)
        {
            timeout_ = timeout;
            return *this;
        }

        Selector& filter(bool (*filter_func)(T))
        {
            filter_func_ = filter_func;
            return *this;
        }

        dds::streams::sub::StreamLoanedSamples<T> get()
        {
            return sdr_->get(sdr_->stream(), max_samples_, timeout_, filter_func_);
        }

        template<typename SamplesFWIterator>
        uint32_t get(SamplesFWIterator samples, uint32_t max_samples)
        {
            dds::streams::sub::StreamLoanedSamples<T> sls = sdr_->get(sdr_->stream(), max_samples, timeout_, filter_func_);

            for(uint32_t i = 0; i < sls.length(); i++)
            {
                samples->delegate().data((sls.begin() + i)->data());
                samples++;
            }

            return 0;
        }

        template<typename SamplesBIIterator>
        uint32_t get(SamplesBIIterator samples)
        {
            dds::streams::sub::StreamLoanedSamples<T> sls = sdr_->get(sdr_->stream(), max_samples_, timeout_, filter_func_);

            dds::streams::sub::StreamSample<T> sample;
            for(uint32_t i = 0; i < sls.length(); i++)
            {
                sample.data((sls.begin() + i)->data());
                *samples = sample;
                samples++;
            }

            return sls.length();
        }

        friend class StreamDataReader;
        StreamDataReader<T>* sdr_;
        uint32_t max_samples_;
        dds::core::Duration timeout_;
        bool (*filter_func_)(T);
    };

    class ManipulatorSelector: public Selector
    {
    public:

        ManipulatorSelector(StreamDataReader<T>* sdr) : Selector(sdr)
        {
        }

        ManipulatorSelector& operator >>(dds::streams::sub::StreamLoanedSamples<T>& samples)
        {
            samples = this->Selector::get();
            return *this;
        }
    };

    void stream(uint32_t id)
    {
        if(stream_id_ != id)
        {
            stream_id_ = id;
            stream_ = &streams_[id];
            stream_->id = stream_id_;
            if(!stream_->sls.delegate()->data_info_)
            {
                DDS::StringSeq query_str;
                query_str.length(1);
                std::stringstream ss;
                ss << id;
                query_str[0] = ss.str().c_str();
                stream_->raw_reader_ = raw_reader_;
                DDS::QueryCondition_var query_cond = reader_->create_querycondition(DDS::ANY_SAMPLE_STATE,
                                                                                    DDS::ANY_VIEW_STATE,
                                                                                    DDS::ANY_INSTANCE_STATE,
                                                                                    "id=%0", query_str);
                stream_->wait_set = new DDS::WaitSet();
                stream_->handle = DDS::HANDLE_NIL;
                DDS::ReturnCode_t result = stream_->wait_set->attach_condition(query_cond.in());
                org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::attach_condition"));
            }
        }
    }

    uint32_t stream()
    {
        return stream_id_;
    }

    dds::streams::sub::StreamLoanedSamples<T> get()
    {
        return get(stream_id_, dds::core::LENGTH_UNLIMITED, dds::core::Duration::zero(), 0);
    }

    dds::streams::sub::StreamLoanedSamples<T> get(uint32_t stream_id, uint32_t max_samples,
                                                  const dds::core::Duration& timeout, bool (*filter_func)(T))
    {
        stream(stream_id);

        dds::streams::sub::StreamLoanedSamples<T> sls;
        sls.delegate()->reader_ = reader_;
        //If the stream was not found or the stream was found but stream has already been fully read
        if(!stream_->sls.delegate()->data_info_ || stream_->index >= stream_->sls.delegate()->data_info_.get()->data_.get_buffer()[0].buffer.length())
        {
            //Reset the stream index
            stream_->index = 0;

            typename dds::streams::sub::detail::StreamLoanedSamples<T>::data_info_t* data_info =
                new typename dds::streams::sub::detail::StreamLoanedSamples<T>::data_info_t;

            DDS::Duration_t dds_timeout;
            dds_timeout.sec = (DDS::Long)timeout.sec();
            dds_timeout.nanosec = (DDS::Long)timeout.nanosec();
            DDS::ConditionSeq cseq;
            DDS::ReturnCode_t result;
            //Look up the instance handle for the stream if this has not already been done
            if(stream_->handle == DDS::HANDLE_NIL)
            {
                result = stream_->wait_set->wait(cseq, dds_timeout);
                org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::wait"));
                StreamT sample;
                sample.id = stream_id;
                stream_->handle = raw_reader_->lookup_instance(sample);
                //Take data
                result = raw_reader_->take_instance(data_info->data_, data_info->info_, 1, stream_->handle,
                                                                      DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::take_instance"));
                sls.delegate()->data_info_.reset(data_info);
                stream_->sls = sls;
            }
            else
            {
                //Try to take data
                result = raw_reader_->take_instance(data_info->data_, data_info->info_, 1, stream_->handle,
                                                                                    DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                //Wait for data if none was read
                if(result == DDS::RETCODE_NO_DATA)
                {
                    result = stream_->wait_set->wait(cseq, dds_timeout);
                    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::wait"));

                    //Take data once available
                    result = raw_reader_->take_instance(data_info->data_, data_info->info_, 1, stream_->handle,
                                                        DDS::ANY_SAMPLE_STATE, DDS::ANY_VIEW_STATE, DDS::ANY_INSTANCE_STATE);
                }
                else if(result == DDS::RETCODE_ALREADY_DELETED)
                {
                    //If instance has been deleted lookup again on next get
                    stream_->handle = DDS::HANDLE_NIL;
                }
                else
                {
                    org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::take_instance"));
                }

                sls.delegate()->data_info_.reset(data_info);
                stream_->sls = sls;

                if(data_info->data_.length() == 0)
                {
                    return sls;
                }
            }
        }
        sls.delegate()->data_info_ = stream_->sls.delegate()->data_info_;

        //Resize the StreamLoanedSamples internal sample vector
        std::size_t size = stream_->sls.delegate()->data_info_.get()->data_.get_buffer()[0].buffer.length() - stream_->index < max_samples ?
            stream_->sls.delegate()->data_info_.get()->data_.get_buffer()[0].buffer.length() - stream_->index : max_samples;
        sls.delegate()->resize(size);
        sls.delegate()->stream(stream_id);

        //Iterate through StreamLoanedSamples placing samples in it
        uint32_t filter_hits = 0;
        typename dds::streams::sub::detail::StreamLoanedSamples<T>::iterator it = sls.delegate()->mbegin();
        typename dds::streams::sub::detail::StreamLoanedSamples<T>::const_iterator itend = sls.end();
        for(;it != itend && stream_->index != size; stream_->index++)
        {
            //If a filter function was not supplied or the filter function evaluates to true
            if(filter_func == 0 || filter_func(stream_->sls.delegate()->data_info_.get()->data_.get_buffer()[0].buffer[stream_->index]))
            {
                //Copy the pointer to the sample data into the StreamSharedSamples
                it->delegate().data(dynamic_cast<T*>(&stream_->sls.delegate()->data_info_.get()->data_.get_buffer()[0].buffer[stream_->index]));
                filter_hits++;
                it++;
            }
        }

        //If the filter has resulted in fewer samples being returned than expected
        if(filter_hits < size)
        {
            //Reduce the size of the StreamLoanedSamples
            sls.delegate()->resize(filter_hits);
        }

        return sls;
    }

    template<typename SamplesFWIterator>
    uint32_t get(SamplesFWIterator samples, uint32_t max_samples)
    {
        dds::streams::sub::StreamLoanedSamples<T> sls = get(stream_id_, max_samples, dds::core::Duration::zero(), 0);

        for(uint32_t i = 0; i < sls.length(); i++)
        {
            samples->delegate().data((sls.begin() + i)->data());
            samples++;
        }

        return 0;
    }

    template<typename SamplesBIIterator>
    uint32_t get(SamplesBIIterator samples)
    {
        dds::streams::sub::StreamLoanedSamples<T> sls = get(stream_id_, dds::core::LENGTH_UNLIMITED, dds::core::Duration::zero(), 0);

        dds::streams::sub::StreamSample<T> sample;
        for(uint32_t i = 0; i < sls.length(); i++)
        {
            sample.data((sls.begin() + i)->data());
            *samples = sample;
            samples++;
        }

        return sls.length();
    }

    const dds::streams::sub::qos::StreamDataReaderQos& qos() const
    {
        return qos_;
    }

    void qos(const dds::streams::sub::qos::StreamDataReaderQos& qos)
    {
        qos_ = qos;
    }

private:
    uint32_t stream_id_;
    dds::streams::sub::qos::StreamDataReaderQos qos_;
    dds::domain::DomainParticipant dp_;
    dds::sub::Subscriber sub_;
    dds::topic::Topic<StreamT> topic_;
    org::opensplice::core::DDS_DR_REF reader_;
    typename org::opensplice::topic::topic_data_reader<StreamT>::type* raw_reader_;
    class stream_t
    {
    public:
        uint32_t id;
        std::size_t index;
        DDS::WaitSet_var wait_set;
        DDS::InstanceHandle_t handle;
        dds::streams::sub::StreamLoanedSamples<T> sls;
        typename org::opensplice::topic::topic_data_reader<StreamT>::type* raw_reader_;

        ~stream_t()
        {
            if(wait_set)
            {
                DDS::ConditionSeq cseq;
                DDS::ReturnCode_t result = wait_set->get_conditions(cseq);
                org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_conditions"));
                result = wait_set->detach_condition(cseq[0]);
                org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::detach_condition"));
                result = raw_reader_->delete_readcondition(dynamic_cast<DDS::ReadCondition*>(cseq[0].in()));
                org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::delete_readcondition"));
            }
        }
    };
    std::map<uint32_t, stream_t> streams_;
    stream_t* stream_;
};

}
}
}
}

#endif /* DDS_STREAMS_SUB_DETAIL_STREAMDATAREADER_HPP_ */

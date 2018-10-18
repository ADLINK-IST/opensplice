#ifndef DDS_STREAMS_SUB_TDATAREADER_HPP_
#define DDS_STREAMS_SUB_TDATAREADER_HPP_
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

#include <spec/dds/streams/sub/TStreamDataReader.hpp>
#include <dds/streams/sub/detail/StreamDataReader.hpp>

namespace dds
{
namespace streams
{
namespace sub
{

#ifdef OSPL_2893_COMPILER_BUG
template <typename T>
class StreamDataReader <T, dds::streams::sub::detail::StreamDataReader> : public dds::core::TEntity<dds::streams::sub::detail::StreamDataReader<T> >
{
#endif

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataReader<T, DELEGATE>::Selector::Selector(StreamDataReader& sdr)
    #else
public:
    class Selector
    {
    public:
        Selector(StreamDataReader& sdr)
        #endif
        : impl_(sdr.delegate().get()) {}

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        typename StreamDataReader<T, DELEGATE>::
        #endif
        Selector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::Selector::
        #endif
        max_samples(uint32_t max_samples)
        {
            impl_.max_samples(max_samples);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        typename StreamDataReader<T, DELEGATE>::
        #endif
        Selector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::Selector::
        #endif
        timeout(const dds::core::Duration& timeout)
        {
            impl_.timeout(timeout);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        typename StreamDataReader<T, DELEGATE>::
        #endif
        Selector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::Selector::
        #endif
        filter(bool (*filter_func)(T))
        {
            impl_.filter(filter_func);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        #endif
        dds::streams::sub::StreamLoanedSamples<T>
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::Selector::
        #endif
        get()
        {
            return impl_.get();
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        #endif
        template <typename SamplesFWIterator>
        uint32_t
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::Selector::
        #endif
        get(SamplesFWIterator sfit, uint32_t max_samples)
        {
            return impl_.get(sfit, max_samples);
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        #endif
        template <typename SamplesBIIterator>
        uint32_t
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::Selector::
        #endif
        get(SamplesBIIterator sbit)
        {
            return impl_.get(sbit);
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        #else
    private:
        typename dds::streams::sub::detail::StreamDataReader<T>::Selector impl_;
    };

    class ManipulatorSelector
    {
    public:
    #endif
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::ManipulatorSelector::
        #endif
        ManipulatorSelector(StreamDataReader& sdr)
        : impl_(sdr.delegate().get()) {}

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        typename StreamDataReader<T, DELEGATE>::
        #endif
        ManipulatorSelector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::ManipulatorSelector::
        #endif
        max_samples(uint32_t max_samples)
        {
            impl_.max_samples(max_samples);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        typename StreamDataReader<T, DELEGATE>::
        #endif
        ManipulatorSelector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::ManipulatorSelector::
        #endif
        timeout(const dds::core::Duration& timeout)
        {
            impl_.timeout(timeout);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        typename StreamDataReader<T, DELEGATE>::
        #endif
        ManipulatorSelector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::ManipulatorSelector::
        #endif
        filter(bool (*filter_func)(T))
        {
            impl_.filter(filter_func);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        #endif
        #ifndef OSPL_2893_COMPILER_BUG
        typename StreamDataReader<T, DELEGATE>::
        #endif
        ManipulatorSelector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::ManipulatorSelector::
        #endif
        operator >>(dds::streams::sub::StreamLoanedSamples<T>& samples)
        {
            impl_ >> samples;
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        typename StreamDataReader<T, DELEGATE>::
        #endif
        ManipulatorSelector&
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::ManipulatorSelector::
        #endif
        operator >> (ManipulatorSelector & (manipulator)(ManipulatorSelector&))
        {
            manipulator(*this);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        #endif
        template <typename Functor>
        #ifndef OSPL_2893_COMPILER_BUG
        typename StreamDataReader<T, DELEGATE>::
        #endif
        ManipulatorSelector
        #ifndef OSPL_2893_COMPILER_BUG
        StreamDataReader<T, DELEGATE>::ManipulatorSelector::
        #endif
        operator >> (Functor f)
        {
            f(*this);
            return *this;
        }

        #ifndef OSPL_2893_COMPILER_BUG
        template <typename T, template <typename Q> class DELEGATE>
        StreamDataReader<T, DELEGATE>::
        #else
    private:
        typename dds::streams::sub::detail::StreamDataReader<T>::ManipulatorSelector impl_;
    };
    #endif
    StreamDataReader(const std::string& stream_name,
    #ifndef OSPL_2893_COMPILER_BUG
                     const dds::streams::sub::qos::StreamDataReaderQos& qos) :
    ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(stream_name, qos))
    #else
    const dds::streams::sub::qos::StreamDataReaderQos& qos = dds::streams::sub::qos::StreamDataReaderQos()) :
    ::dds::core::TEntity< dds::streams::sub::detail::StreamDataReader<T> >(new dds::streams::sub::detail::StreamDataReader<T>(stream_name, qos))
    #endif
    {

    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataReader<T, DELEGATE>::
    #endif
    StreamDataReader(uint32_t domain_id, const std::string& stream_name,
    #ifndef OSPL_2893_COMPILER_BUG
                     const dds::streams::sub::qos::StreamDataReaderQos& qos) :
    ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(domain_id, stream_name, qos))
    #else
    const dds::streams::sub::qos::StreamDataReaderQos& qos = dds::streams::sub::qos::StreamDataReaderQos()) :
    ::dds::core::TEntity< dds::streams::sub::detail::StreamDataReader<T> >(new dds::streams::sub::detail::StreamDataReader<T>(domain_id, stream_name, qos))
    #endif
    {

    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataReader<T, DELEGATE>::
    #endif
    StreamDataReader(const dds::sub::Subscriber& subscriber, const std::string& stream_name,
    #ifndef OSPL_2893_COMPILER_BUG
                     const dds::streams::sub::qos::StreamDataReaderQos& qos) :
    ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(subscriber, stream_name, qos))
    #else
    const dds::streams::sub::qos::StreamDataReaderQos& qos = dds::streams::sub::qos::StreamDataReaderQos()) :
    ::dds::core::TEntity< dds::streams::sub::detail::StreamDataReader<T> >(new dds::streams::sub::detail::StreamDataReader<T>(subscriber, stream_name, qos))
    #endif
    {

    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataReader<T, DELEGATE>&
    #else
    StreamDataReader<T, dds::streams::sub::detail::StreamDataReader>&
    #endif
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    stream(uint32_t id)
    {
        this->delegate()->stream(id);
        return *this;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    uint32_t
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    stream()
    {
        return this->delegate()->stream();
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    dds::streams::sub::StreamLoanedSamples<T>
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    get()
    {
        return this->delegate()->get();
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    template <typename SamplesFWIterator>
    uint32_t
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    get(SamplesFWIterator sfit, uint32_t max_samples)
    {
        return this->delegate()->get(sfit, max_samples);
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    template <typename SamplesBIIterator>
    uint32_t
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    get(SamplesBIIterator sbit)
    {
        return this->delegate()->get(sbit);
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename StreamDataReader<T, DELEGATE>::
    #endif
    Selector
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    select()
    {
        Selector selector(*this);
        return selector;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    StreamDataReader<T, DELEGATE>& StreamDataReader<T, DELEGATE>::
    #else
    StreamDataReader<T, dds::streams::sub::detail::StreamDataReader>&
    #endif
    operator>>(dds::streams::sub::StreamLoanedSamples<T>& sls)
    {
        sls = this->get();
        return *this;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename StreamDataReader<T, DELEGATE>::
    #endif
    ManipulatorSelector
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    operator>>(ManipulatorSelector& (manipulator)(ManipulatorSelector&))
    {
        ManipulatorSelector selector(*this);
        manipulator(selector);
        return selector;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    template <typename Functor>
    #ifndef OSPL_2893_COMPILER_BUG
    typename StreamDataReader<T, DELEGATE>::
    #endif
    ManipulatorSelector
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    operator>>(Functor f)
    {
        ManipulatorSelector selector(*this);
        f(selector);
        return selector;
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    const dds::streams::sub::qos::StreamDataReaderQos&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    qos() const
    {
        return this->delegate()->qos();
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    void
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    qos(const dds::streams::sub::qos::StreamDataReaderQos& qos)
    {
        this->delegate()->qos(qos);
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    #endif
    dds::streams::sub::qos::StreamDataReaderQos&
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    operator<<(const dds::streams::sub::qos::StreamDataReaderQos& qos)
    {
        this->delegate()->qos(qos);
        return (dds::streams::sub::qos::StreamDataReaderQos&)this->delegate()->qos();
    }

    #ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    const StreamDataReader<T, DELEGATE>&
    #else
    const StreamDataReader<T, dds::streams::sub::detail::StreamDataReader>&
    #endif
    #ifndef OSPL_2893_COMPILER_BUG
    StreamDataReader<T, DELEGATE>::
    #endif
    operator>>(dds::streams::sub::qos::StreamDataReaderQos& qos) const
    {
        qos = this->delegate()->qos();
        return *this;
    }
#ifdef OSPL_2893_COMPILER_BUG
    OMG_DDS_BASIC_REF_TYPE(StreamDataReader, dds::core::TEntity, dds::streams::sub::detail::StreamDataReader<T>)
};
#endif

}
}
}

#endif /* DDS_STREAMS_SUB_TDATAREADER_HPP_ */

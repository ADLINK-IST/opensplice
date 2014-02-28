/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2012 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef OSPL_DDS_SUB_TDATAREADER_HPP_
#define OSPL_DDS_SUB_TDATAREADER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/TDataReader.hpp>
#include <dds/sub/detail/DataReader.hpp>
#include <dds/sub/AnyDataReader.hpp>

//#include <org/opensplice/core/EntityRegistry.hpp>
// Implementation

namespace dds
{
namespace sub
{

class AnyDataReader;
//--------------------------------------------------------------------------------
//  DATAREADER
//--------------------------------------------------------------------------------
#ifdef OSPL_2893_COMPILER_BUG
template <typename T>
class DataReader <T, dds::sub::detail::DataReader> : public dds::core::TEntity<dds::sub::detail::DataReader<T> >
{
#endif

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::Selector::Selector(DataReader& dr)
#else
public:
    typedef T                                        DataType;
    typedef ::dds::sub::DataReaderListener<T>        Listener;
    //--------------------------------------------------------------------------------
    //  DATAREADER::SELECTOR
    //--------------------------------------------------------------------------------
    class Selector
    {
    public:
        Selector(DataReader& dr)
#endif
        : impl_(dr.delegate().get()) {}
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    Selector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    instance(const dds::core::InstanceHandle& h)
    {
        impl_.instance(h);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    Selector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    state(const dds::sub::status::DataState& s)
    {
        impl_.filter_state(s);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    Selector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    content(const dds::sub::Query& query)
    {
        impl_.filter_content(query);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    Selector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    max_samples(uint32_t n)
    {
        impl_.max_samples(n);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::sub::LoanedSamples<T>
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    read()
    {
        return impl_.read();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::sub::LoanedSamples<T>
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    take()
    {
        return impl_.take();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesFWIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    read(SamplesFWIterator sfit, uint32_t max_samples)
    {
        return impl_.read(sfit, max_samples);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesFWIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    take(SamplesFWIterator sfit,    uint32_t max_samples)
    {
        return impl_.take(sfit, max_samples);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesBIIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    read(SamplesBIIterator sbit)
    {
        return impl_.read(sbit);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesBIIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::Selector::
#endif
    take(SamplesBIIterator sbit)
    {
        return impl_.take(sbit);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#else
private:
    typename dds::sub::detail::DataReader<T>::Selector impl_;
};
class ManipulatorSelector
{
public:
#endif

    //--------------------------------------------------------------------------------
    //  DATAREADER::MANIPULATORSELECTOR
    //--------------------------------------------------------------------------------
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    ManipulatorSelector(DataReader& dr)
        : impl_(dr.delegate().get()) {}

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    bool
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    read_mode()
    {
        return impl_.read_mode();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    read_mode(bool b)
    {
        impl_.read_mode(b);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    instance(const dds::core::InstanceHandle& h)
    {
        impl_.instance(h);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    next_instance(const dds::core::InstanceHandle& h)
    {
        impl_.next_instance(h);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
#ifndef OSPL_2893_COMPILER_BUG
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    operator >>(dds::sub::LoanedSamples<T>& samples)
    {
        impl_ >> samples;
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
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
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    operator >> (Functor f)
    {
        f(*this);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    state(const dds::sub::status::DataState& s)
    {
        impl_.filter_state(s);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    content(const dds::sub::Query& query)
    {
        impl_.filter_content(query);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::ManipulatorSelector::
#endif
    max_samples(uint32_t n)
    {
        impl_.max_samples(n);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::
#else
private:
    typename dds::sub::detail::DataReader<T>::ManipulatorSelector impl_;
};
#endif
    DataReader(const dds::sub::Subscriber& sub,
               const dds::topic::Topic<T>& topic)
        :
#ifndef OSPL_2893_COMPILER_BUG
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic))
#else
        ::dds::core::TEntity< dds::sub::detail::DataReader<T> >(new dds::sub::detail::DataReader<T>(sub, topic))
#endif
    {
#ifndef OSPL_2893_COMPILER_BUG
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, DELEGATE> >::insert(this->delegate()->get_raw_reader(), *this);
#else
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, dds::sub::detail::DataReader > >::insert(this->delegate()->get_raw_reader(), *this);
#endif
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::
#endif
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::Topic<T>& topic,
               const dds::sub::qos::DataReaderQos& qos,
#                                       ifndef OSPL_2893_COMPILER_BUG
               dds::sub::DataReaderListener<T>* listener,
               const dds::core::status::StatusMask& mask)
#                                       else
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all())
#                                       endif
        :
#ifndef OSPL_2893_COMPILER_BUG
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(
#else
        ::dds::core::TEntity< dds::sub::detail::DataReader<T> >(new dds::sub::detail::DataReader<T>(
#endif
                                                sub,
                                                topic,
                                                qos))
    {
        if(listener)
        {
            dds::sub::detail::EventHandler<DataReader, T>* h = new dds::sub::detail::EventHandler<DataReader, T>(*this, listener);
            this->delegate()->event_handler(h, mask);
        }

#ifndef OSPL_2893_COMPILER_BUG
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, DELEGATE> >::insert(this->delegate()->get_raw_reader(), *this);
#else
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, dds::sub::detail::DataReader > >::insert(this->delegate()->get_raw_reader(), *this);
#endif
    }

#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::
#endif

    DataReader(const dds::sub::Subscriber& sub,
               const dds::topic::ContentFilteredTopic<T>& topic)
        :
#ifndef OSPL_2893_COMPILER_BUG
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic))
#else
        ::dds::core::TEntity< dds::sub::detail::DataReader<T> >(new dds::sub::detail::DataReader<T>(sub, topic))
#endif
    {
#ifndef OSPL_2893_COMPILER_BUG
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, DELEGATE> >::insert(this->delegate()->get_raw_reader(), *this);
#else
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, dds::sub::detail::DataReader > >::insert(this->delegate()->get_raw_reader(), *this);
#endif
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::
#endif
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::ContentFilteredTopic<T>& topic,
               const dds::sub::qos::DataReaderQos& qos,
#ifndef OSPL_2893_COMPILER_BUG
               dds::sub::DataReaderListener<T>* listener,
               const dds::core::status::StatusMask& mask)
#else
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all())
#endif
        :
#ifndef OSPL_2893_COMPILER_BUG
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic, qos))
#else
        ::dds::core::TEntity< dds::sub::detail::DataReader<T> >(new dds::sub::detail::DataReader<T>(sub, topic, qos))
#endif

    {
        if(listener)
        {
            dds::sub::detail::EventHandler<DataReader, T>* h = new dds::sub::detail::EventHandler<DataReader, T>(*this, listener);
            this->delegate()->event_handler(h, mask);
        }

#ifndef OSPL_2893_COMPILER_BUG
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, DELEGATE> >::insert(this->delegate()->get_raw_reader(), *this);
#else
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, dds::sub::detail::DataReader > >::insert(this->delegate()->get_raw_reader(), *this);
#endif
    }
#endif /* OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT */

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::
#endif
    DataReader(const dds::sub::Subscriber& sub,
               const dds::topic::MultiTopic<T>& topic)
        :
#       ifndef OSPL_2893_COMPILER_BUG
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic))
#       else
        ::dds::core::TEntity< dds::sub::detail::DataReader<T> >(new dds::sub::detail::DataReader<T>(sub, topic))
#       endif
    {
#ifndef OSPL_2893_COMPILER_BUG
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, DELEGATE> >::insert(this->delegate()->get_raw_reader(), *this);
#else
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, dds::sub::detail::DataReader > >::insert(this->delegate()->get_raw_reader(), *this);
#endif
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::
#endif
    DataReader(const dds::sub::Subscriber& sub,
               const ::dds::topic::MultiTopic<T>& topic,
               const dds::sub::qos::DataReaderQos& qos,
#                                       ifndef OSPL_2893_COMPILER_BUG
               dds::sub::DataReaderListener<T>* listener,
               const dds::core::status::StatusMask& mask)
#                                       else
               dds::sub::DataReaderListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all())
#                                       endif
        :
#                                       ifndef OSPL_2893_COMPILER_BUG
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic, qos))
#                                       else
        ::dds::core::TEntity< dds::sub::detail::DataReader<T> >(new dds::sub::detail::DataReader<T>(sub, topic, qos))
#                                       endif
    {
        if(listener)
        {
            dds::sub::detail::EventHandler<DataReader, T>* h = new dds::sub::detail::EventHandler<DataReader, T>(*this, listener);
            this->delegate()->event_handler(h, mask);
        }

#ifndef OSPL_2893_COMPILER_BUG
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, DELEGATE> >::insert(this->delegate()->get_raw_reader(), *this);
#else
        org::opensplice::core::EntityRegistry<DDS::DataReader_ptr, DataReader<T, dds::sub::detail::DataReader > >::insert(this->delegate()->get_raw_reader(), *this);
#endif
    }
#endif /* OMG_DDS_MULTI_TOPIC_SUPPORT */

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>::
#else
    virtual
#endif
    ~DataReader() { }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::sub::status::DataState&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    default_filter_state()
    {
        return this->delegate()->default_status_filter();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::
#else
    DataReader<T, dds::sub::detail::DataReader>&
#endif
    default_filter_state(const dds::sub::status::DataState& status)
    {
        this->delegate()->default_status_filter(status);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::
#else
    DataReader<T, dds::sub::detail::DataReader>&
#endif
    operator >>(dds::sub::LoanedSamples<T>& ls)
    {
        ls = this->read();
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    operator >> (ManipulatorSelector& (manipulator)(ManipulatorSelector&))
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
    typename DataReader<T, DELEGATE>::
#endif
    ManipulatorSelector
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    operator >> (Functor f)
    {
        ManipulatorSelector selector(*this);
        f(selector);
        return selector;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    LoanedSamples<T>
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    read()
    {
        return this->delegate()->read();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    LoanedSamples<T>
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    take()
    {
        return this->delegate()->take();
    }


#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesFWIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    read(SamplesFWIterator sfit, uint32_t max_samples)
    {
        return this->delegate()->read(sfit, max_samples);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesFWIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    take(SamplesFWIterator sfit, uint32_t max_samples)
    {
        return this->delegate()->take(sfit, max_samples);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesBIIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    read(SamplesBIIterator sbit)
    {
        return this->delegate()->read(sbit);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesBIIterator>
    uint32_t
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    take(SamplesBIIterator sbit)
    {
        return this->delegate()->take(sbit);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    Selector
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    select()
    {
        Selector selector(*this);
        return selector;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::topic::TopicInstance<T>
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    key_value(const dds::core::InstanceHandle& h)
    {
        return this->delegate()->key_value(h);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    T&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    key_value(T& sample, const dds::core::InstanceHandle& h)
    {
        return this->delegate()->key_value(sample, h);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::InstanceHandle
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    lookup_instance(const T& key) const
    {
        return this->delegate()->lookup_instance(key);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::topic::TopicDescription<T>
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    topic_description() const
    {
        return this->delegate()->topic_description();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::sub::Subscriber&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    subscriber() const
    {
        return this->delegate()->subscriber();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    listener(Listener* listener,
             const dds::core::status::StatusMask& event_mask)
    {
        dds::sub::detail::EventHandler<DataReader, T>* h = 0;
        if(listener)
        {
            h = new dds::sub::detail::EventHandler<DataReader, T>(*this, listener);
        }
        this->delegate()->event_handler(h, event_mask);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    typename DataReader<T, DELEGATE>::
#endif
    Listener*
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    listener() const
    {
        return this->delegate()->listener();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::sub::qos::DataReaderQos&

#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
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
    DataReader<T, DELEGATE>::
#endif
    qos(const dds::sub::qos::DataReaderQos& qos)
    {
        this->delegate()->qos(qos);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::sub::qos::DataReaderQos&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    operator << (const dds::sub::qos::DataReaderQos& qos)
    {
        this->delegate()->qos(qos);
        return (dds::sub::qos::DataReaderQos&)this->delegate()->qos();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
    const DataReader<T, DELEGATE>&
#else
    const DataReader<T, dds::sub::detail::DataReader>&
#endif
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    operator >> (dds::sub::qos::DataReaderQos& qos) const
    {
        qos = this->delegate()->qos();
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    wait_for_historical_data(const dds::core::Duration& timeout)
    {
        this->delegate()->wait_for_historical_data(timeout);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::status::LivelinessChangedStatus&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    liveliness_changed_status()
    {
        return this->delegate()->liveliness_changed_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::status::SampleRejectedStatus&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    sample_rejected_status()
    {
        return this->delegate()->sample_rejected_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::status::SampleLostStatus&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    sample_lost_status()
    {
        return this->delegate()->sample_lost_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::status::RequestedDeadlineMissedStatus&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    requested_deadline_missed_status()
    {
        return this->delegate()->requested_deadline_missed_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::status::RequestedIncompatibleQosStatus&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    requested_incompatible_qos_status()
    {
        return this->delegate()->requested_incompatible_qos_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::core::status::SubscriptionMatchedStatus&
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    subscription_matched_status()
    {
        return this->delegate()->subscription_matched_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    close()
    {
        try
        {
            this->delegate()->close();
            dds::sub::AnyDataReader adr(*this);
            org::opensplice::core::retain_remove<dds::sub::AnyDataReader>(adr);
        }
        catch(int i)
        {
            (void)i;
        }
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataReader<T, DELEGATE>::
#endif
    retain()
    {
        this->delegate()->retain();
        dds::sub::AnyDataReader adr(*this);
        org::opensplice::core::retain_add<dds::sub::AnyDataReader>(adr);
    }
#ifdef OSPL_2893_COMPILER_BUG
public:
    OMG_DDS_REF_TYPE(DataReader, dds::core::TEntity, dds::sub::detail::DataReader<T>)
};
#endif
}
}

// End of implementation

#endif /* OSPL_DDS_SUB_TDATAREADER_HPP_ */

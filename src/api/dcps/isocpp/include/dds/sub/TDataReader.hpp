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

// Implementation

namespace dds
{
namespace sub
{

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::Selector::Selector(DataReader& dr)
    : impl_(dr.delegate().get()) {}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector& DataReader<T, DELEGATE>::Selector::instance(const dds::core::InstanceHandle& h)
{
    impl_.instance(h);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector& DataReader<T, DELEGATE>::Selector::state(const dds::sub::status::DataState& s)
{
    impl_.filter_state(s);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector& DataReader<T, DELEGATE>::Selector::content(const dds::sub::Query& query)
{
    impl_.filter_content(query);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
dds::sub::LoanedSamples<T>
DataReader<T, DELEGATE>::Selector::read()
{
    return impl_.read();
}

template <typename T, template <typename Q> class DELEGATE>
dds::sub::LoanedSamples<T>
DataReader<T, DELEGATE>::Selector::take()
{
    return impl_.take();
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesFWIterator>
uint32_t
DataReader<T, DELEGATE>::Selector::read(SamplesFWIterator sfit, uint32_t max_samples)
{
    return impl_.read(sfit, max_samples);
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesFWIterator>
uint32_t
DataReader<T, DELEGATE>::Selector::take(SamplesFWIterator sfit,    uint32_t max_samples)
{
    return impl_.take(sfit, max_samples);
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesBIIterator>
uint32_t
DataReader<T, DELEGATE>::Selector::read(SamplesBIIterator sbit)
{
    return impl_.read(sbit);
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesBIIterator>
uint32_t
DataReader<T, DELEGATE>::Selector::take(SamplesBIIterator sbit)
{
    return impl_.take(sbit);
}

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::ManipulatorSelector::ManipulatorSelector(DataReader& dr)
    : impl_(dr.delegate().get()) {}

template <typename T, template <typename Q> class DELEGATE>
bool DataReader<T, DELEGATE>::ManipulatorSelector::read_mode()
{
    return impl_.read_mode();
}

template <typename T, template <typename Q> class DELEGATE>
void DataReader<T, DELEGATE>::ManipulatorSelector::read_mode(bool b)
{
    impl_.read_mode(b);
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::instance(const dds::core::InstanceHandle& h)
{
    impl_.instance(h);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::next_instance(const dds::core::InstanceHandle& h)
{
    impl_.next_instance(h);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector&
DataReader<T, DELEGATE>::ManipulatorSelector::operator >>(dds::sub::LoanedSamples<T>& samples)
{
    impl_ >> samples;
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector&
DataReader<T, DELEGATE>::ManipulatorSelector::operator >> (ManipulatorSelector& (manipulator)(ManipulatorSelector&))
{
    manipulator(*this);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
template <typename Functor>
typename DataReader<T, DELEGATE>::ManipulatorSelector
DataReader<T, DELEGATE>::ManipulatorSelector::operator >> (Functor f)
{
    f(*this);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector& DataReader<T, DELEGATE>::ManipulatorSelector::state(const dds::sub::status::DataState& s)
{
    impl_.filter_state(s);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(const dds::sub::Subscriber& sub,
                                    const dds::topic::Topic<T>& topic)
    : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic))
{ }

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(const dds::sub::Subscriber& sub,
                                    const ::dds::topic::Topic<T>& topic,
                                    const dds::sub::qos::DataReaderQos& qos,
                                    dds::sub::DataReaderListener<T>* listener,
                                    const dds::core::status::StatusMask& mask)
    : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub,
                                          topic,
                                          qos))
{
    if (listener)
    {
        dds::sub::detail::EventHandler<DataReader, T>* h = new dds::sub::detail::EventHandler<DataReader, T>(*this, listener);
        this->delegate()->event_handler(h, mask);
    }
}

#ifdef OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT
template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(const dds::sub::Subscriber& sub,
                                    const dds::topic::ContentFilteredTopic<T>& topic)
    : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic))
{ }

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(const dds::sub::Subscriber& sub,
                                    const ::dds::topic::ContentFilteredTopic<T>& topic,
                                    const dds::sub::qos::DataReaderQos& qos,
                                    dds::sub::DataReaderListener<T>* listener,
                                    const dds::core::status::StatusMask& mask)
                                    : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic, qos))
{
    if (listener)
    {
        dds::sub::detail::EventHandler<DataReader, T>* h = new dds::sub::detail::EventHandler<DataReader, T>(*this, listener);
        this->delegate()->event_handler(h, mask);
    }
}
#endif /* OMG_DDS_CONTENT_SUBSCRIPTION_SUPPORT */

#ifdef OMG_DDS_MULTI_TOPIC_SUPPORT
template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(const dds::sub::Subscriber& sub,
                                    const dds::topic::MultiTopic<T>& topic)
    : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic))
{ }

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::DataReader(const dds::sub::Subscriber& sub,
                                    const ::dds::topic::MultiTopic<T>& topic,
                                    const dds::sub::qos::DataReaderQos& qos,
                                    dds::sub::DataReaderListener<T>* listener,
                                    const dds::core::status::StatusMask& mask)
                                    : ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(sub, topic, qos))
{
    if (listener)
    {
        dds::sub::detail::EventHandler<DataReader, T>* h = new dds::sub::detail::EventHandler<DataReader, T>(*this, listener);
        this->delegate()->event_handler(h, mask);
    }
}
#endif /* OMG_DDS_MULTI_TOPIC_SUPPORT */

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>::~DataReader() { }

template <typename T, template <typename Q> class DELEGATE>
const dds::sub::status::DataState& DataReader<T, DELEGATE>::default_filter_state()
{
    return this->delegate()->default_status_filter();
}

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::default_filter_state(const dds::sub::status::DataState& status)
{
    this->delegate()->default_status_filter(status);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::operator >>(dds::sub::LoanedSamples<T>& ls)
{
    ls = this->read();
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::ManipulatorSelector
DataReader<T, DELEGATE>::operator >> (ManipulatorSelector& (manipulator)(ManipulatorSelector&))
{
    ManipulatorSelector selector(*this);
    manipulator(selector);
    return selector;
}

template <typename T, template <typename Q> class DELEGATE>
template <typename Functor>
typename DataReader<T, DELEGATE>::ManipulatorSelector DataReader<T, DELEGATE>::operator >> (Functor f)
{
    ManipulatorSelector selector(*this);
    f(selector);
    return selector;
}

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T> DataReader<T, DELEGATE>::read()
{
    return this->delegate()->read();
}

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T> DataReader<T, DELEGATE>::take()
{
    return this->delegate()->take();
}


template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesFWIterator>
uint32_t
DataReader<T, DELEGATE>::read(SamplesFWIterator sfit, uint32_t max_samples)
{
    return this->delegate()->read(sfit, max_samples);
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesFWIterator>
uint32_t
DataReader<T, DELEGATE>::take(SamplesFWIterator sfit, uint32_t max_samples)
{
    return this->delegate()->take(sfit, max_samples);
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesBIIterator>
uint32_t
DataReader<T, DELEGATE>::read(SamplesBIIterator sbit)
{
    return this->delegate()->read(sbit);
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesBIIterator>
uint32_t
DataReader<T, DELEGATE>::take(SamplesBIIterator sbit)
{
    return this->delegate()->take(sbit);
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Selector DataReader<T, DELEGATE>::select()
{
    Selector selector(*this);
    return selector;
}

template <typename T, template <typename Q> class DELEGATE>
dds::topic::TopicInstance<T> DataReader<T, DELEGATE>::key_value(const dds::core::InstanceHandle& h)
{
    return this->delegate()->key_value(h);
}

template <typename T, template <typename Q> class DELEGATE>
T& DataReader<T, DELEGATE>::key_value(T& sample, const dds::core::InstanceHandle& h)
{
    return this->delegate()->key_value(sample, h);
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::InstanceHandle DataReader<T, DELEGATE>::lookup_instance(const T& key) const
{
    return this->delegate()->lookup_instance(key);
}

template <typename T, template <typename Q> class DELEGATE>
dds::topic::TopicDescription<T> DataReader<T, DELEGATE>::topic_description() const
{
    return this->delegate()->topic_description();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::sub::Subscriber& DataReader<T, DELEGATE>::subscriber() const
{
    return this->delegate()->subscriber();
}

template <typename T, template <typename Q> class DELEGATE>
void DataReader<T, DELEGATE>::listener(Listener* the_listener,
                                       const dds::core::status::StatusMask& event_mask)
{
    dds::sub::detail::EventHandler<DataReader, T>* h = 0;
    if (the_listener)
        h = new dds::sub::detail::EventHandler<DataReader, T>(*this, the_listener);
    this->delegate()->event_handler(h, event_mask);
}

template <typename T, template <typename Q> class DELEGATE>
typename DataReader<T, DELEGATE>::Listener* DataReader<T, DELEGATE>::listener() const
{
    return this->delegate()->listener();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::sub::qos::DataReaderQos& DataReader<T, DELEGATE>::qos() const
{
    return this->delegate()->qos();
}

template <typename T, template <typename Q> class DELEGATE>
void DataReader<T, DELEGATE>::qos(const dds::sub::qos::DataReaderQos& the_qos)
{
    this->delegate()->qos(the_qos);
}

template <typename T, template <typename Q> class DELEGATE>
DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::operator << (const dds::sub::qos::DataReaderQos& the_qos)
{
    this->delegate()->qos(the_qos);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
const DataReader<T, DELEGATE>& DataReader<T, DELEGATE>::operator >> (dds::sub::qos::DataReaderQos& the_qos) const
{
    the_qos = this->delegate()->qos();
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
void DataReader<T, DELEGATE>::wait_for_historical_data(const dds::core::Duration& timeout)
{
    this->delegate()->wait_for_historical_data(timeout);
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::status::LivelinessChangedStatus&
DataReader<T, DELEGATE>::liveliness_changed_status()
{
    return this->delegate()->liveliness_changed_status();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::status::SampleRejectedStatus&
DataReader<T, DELEGATE>::sample_rejected_status()
{
    return this->delegate()->sample_rejected_status();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::status::SampleLostStatus&
DataReader<T, DELEGATE>::sample_lost_status()
{
    return this->delegate()->sample_lost_status();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::status::RequestedDeadlineMissedStatus&
DataReader<T, DELEGATE>::requested_deadline_missed_status()
{
    return this->delegate()->requested_deadline_missed_status();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::status::RequestedIncompatibleQosStatus&
DataReader<T, DELEGATE>::requested_incompatible_qos_status()
{
    return this->delegate()->requested_incompatible_qos_status();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::core::status::SubscriptionMatchedStatus&
DataReader<T, DELEGATE>::subscription_matched_status()
{
    return this->delegate()->subscription_matched_status();
}

}
}

// End of implementation

#endif /* OSPL_DDS_SUB_TDATAREADER_HPP_ */

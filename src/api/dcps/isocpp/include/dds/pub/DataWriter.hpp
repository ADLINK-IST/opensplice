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
#ifndef OSPL_DDS_PUB_DATAWRITER_HPP_
#define OSPL_DDS_PUB_DATAWRITER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/DataWriter.hpp>
// Implementation
#include <dds/pub/detail/DataWriter.hpp>

namespace dds
{
namespace pub
{

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>::DataWriter(const dds::pub::Publisher& pub, const dds::topic::Topic<T>& topic) :
    ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(pub,
                                        topic,
                                        pub.default_writer_qos(),
                                        dds::core::status::StatusMask::all())) { }

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>::DataWriter(const dds::pub::Publisher& pub,
                                    const ::dds::topic::Topic<T>& topic,
                                    const dds::pub::qos::DataWriterQos& qos,
                                    dds::pub::DataWriterListener<T>* listener,
                                    const dds::core::status::StatusMask& mask) :
    ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(pub,
                                        topic,
                                        qos,
                                        mask))
{
    if (listener)
    {
        dds::pub::detail::EventHandler<DataWriter, T>* h = new dds::pub::detail::EventHandler<DataWriter, T>(*this, listener);
        this->delegate()->event_handler(h, mask);
    }
}


template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>::~DataWriter() { }

//==========================================================================
//== Write API
template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(const T& sample)
{
    this->delegate()->write(sample);
}

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(const T& sample, const dds::core::Time& timestamp)
{
    this->delegate()->write(sample, timestamp);
}

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(const T& data, const ::dds::core::InstanceHandle& instance)
{
    this->delegate()->write(data, instance);
}

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(const T& data,
                                    const ::dds::core::InstanceHandle& instance,
                                    const dds::core::Time& timestamp)
{
    this->delegate()->write(data, instance, timestamp);
}

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(const dds::topic::TopicInstance<T>& i)
{
    this->delegate()->write(i);
}

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::write(const dds::topic::TopicInstance<T>& i,
                                    const dds::core::Time& timestamp)
{
    this->delegate()->write(i, timestamp);
}

template <typename T, template <typename Q> class DELEGATE>
template <typename FWIterator>
void DataWriter<T, DELEGATE>::write(const FWIterator& begin, const FWIterator& end)
{
    FWIterator b = begin;
    while (b != end)
    {
        this->delegate()->write(*b);
        ++b;
    }
}

template <typename T, template <typename Q> class DELEGATE>
template <typename FWIterator>
void DataWriter<T, DELEGATE>::write(const FWIterator& begin, const FWIterator& end,
                                    const dds::core::Time& timestamp)
{
    FWIterator b = begin;
    while (b != end)
    {
        this->delegate()->write(*b, timestamp);
        ++b;
    }
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesFWIterator, typename HandlesFWIterator>
void DataWriter<T, DELEGATE>::write(const SamplesFWIterator& data_begin,
                                    const SamplesFWIterator& data_end,
                                    const HandlesFWIterator& handle_begin,
                                    const HandlesFWIterator& handle_end)
{
    SamplesFWIterator data = data_begin;
    HandlesFWIterator handle = handle_begin;

    while (data != data_end && handle != handle_end)
    {
        this->delegate()->write(*data, *handle);
        ++data;
        ++handle;
    }
}

template <typename T, template <typename Q> class DELEGATE>
template <typename SamplesFWIterator, typename HandlesFWIterator>
void DataWriter<T, DELEGATE>::write(const SamplesFWIterator& data_begin,
                                    const SamplesFWIterator& data_end,
                                    const HandlesFWIterator& handle_begin,
                                    const HandlesFWIterator& handle_end,
                                    const dds::core::Time& timestamp)
{
    SamplesFWIterator data = data_begin;
    HandlesFWIterator handle = handle_begin;

    while (data != data_end && handle != handle_end)
    {
        this->delegate()->write(*data, *handle, timestamp);
        ++data;
        ++handle;
    }
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator << (const T& data)
{
    this->write(data);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator << (const std::pair<T, dds::core::Time>& data)
{
    this->write(data.first, data.second);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator << (const std::pair<T, ::dds::core::InstanceHandle>& data)
{
    this->write(data.first, data.second);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator <<(DataWriter& (*manipulator)(DataWriter&))
{
    return manipulator(*this);
}

//==========================================================================
//== Instance Management
template <typename T, template <typename Q> class DELEGATE>
const ::dds::core::InstanceHandle DataWriter<T, DELEGATE>::register_instance(const T& key)
{
    return this->delegate()->register_instance(key);
}

template <typename T, template <typename Q> class DELEGATE>
const ::dds::core::InstanceHandle DataWriter<T, DELEGATE>::register_instance(const T& key, const dds::core::Time& ts)
{
    return this->delegate()->register_instance(key, ts);
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(const ::dds::core::InstanceHandle& i)
{
    this->delegate()->unregister_instance(i);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::unregister_instance(const ::dds::core::InstanceHandle& i, const dds::core::Time& ts)
{
    this->delegate()->unregister_instance(i,ts);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(const ::dds::core::InstanceHandle& i)
{
    this->delegate()->dispose_instance(i);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::dispose_instance(const ::dds::core::InstanceHandle& i, const dds::core::Time& ts)
{
    this->delegate()->dispose_instance(i,ts);
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
dds::topic::TopicInstance<T>& DataWriter<T, DELEGATE>::key_value(dds::topic::TopicInstance<T>& i, const ::dds::core::InstanceHandle& h)
{
    return this->delegate()->key_value(i, h);
}

template <typename T, template <typename Q> class DELEGATE>
T& DataWriter<T, DELEGATE>::key_value(T& sample, const ::dds::core::InstanceHandle& h)
{
    return this->delegate()->key_value(sample, h);
}

template <typename T, template <typename Q> class DELEGATE>
dds::core::InstanceHandle DataWriter<T, DELEGATE>::lookup_instance(const T& key)
{
    return this->delegate()->lookup_instance(key);
}

//==========================================================================
//== QoS Management
template <typename T, template <typename Q> class DELEGATE>
const ::dds::pub::qos::DataWriterQos& DataWriter<T, DELEGATE>::qos() const
{
    return this->delegate()->qos();
}

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::qos(const dds::pub::qos::DataWriterQos& the_qos)
{
    this->delegate()->qos(the_qos);
}

template <typename T, template <typename Q> class DELEGATE>
const DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator >> (::dds::pub::qos::DataWriterQos& the_qos) const
{
    the_qos = this->delegate()->qos();
    return *this;
}

template <typename T, template <typename Q> class DELEGATE>
DataWriter<T, DELEGATE>& DataWriter<T, DELEGATE>::operator << (const ::dds::pub::qos::DataWriterQos& the_qos)
{
    this->delegate()->qos(the_qos);
    return *this;
}

//==========================================================================
//== Entity Navigation
template <typename T, template <typename Q> class DELEGATE>
const dds::topic::Topic<T>& DataWriter<T, DELEGATE>::topic() const
{
    return this->delegate()->topic();
}

template <typename T, template <typename Q> class DELEGATE>
const dds::pub::Publisher& DataWriter<T, DELEGATE>::publisher() const
{
    return this->delegate()->publisher();
}

//==========================================================================
//== ACKs

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::wait_for_acknowledgments(const dds::core::Duration& timeout)
{
    this->delegate()->wait_for_acknowledgments(timeout);
}

//==========================================================================
//== Listeners Management

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::listener(DataWriterListener<T>* the_listener,
                                       const ::dds::core::status::StatusMask& mask)
{
    dds::pub::detail::EventHandler<DataWriter, T>* h = new dds::pub::detail::EventHandler<DataWriter, T>(*this, the_listener);
    this->delegate()->event_handler(h, mask);

    //this->delegate()->listener(the_listener, mask);
}

template <typename T, template <typename Q> class DELEGATE>
DataWriterListener<T>* DataWriter<T, DELEGATE>::listener() const
{
    return this->delegate()->listener();
}

//==========================================================================
//== Status Management

template <typename T, template <typename Q> class DELEGATE>
const ::dds::core::status::LivelinessLostStatus DataWriter<T, DELEGATE>::liveliness_lost_status()
{
    return this->delegate()->liveliness_lost_status();
}

template <typename T, template <typename Q> class DELEGATE>
const ::dds::core::status::OfferedDeadlineMissedStatus
DataWriter<T, DELEGATE>::offered_deadlined_missed_status()
{
    return this->delegate()->offered_deadline_missed_status();
}

template <typename T, template <typename Q> class DELEGATE>
const ::dds::core::status::OfferedIncompatibleQosStatus
DataWriter<T, DELEGATE>::offered_incompatible_qos_status()
{
    return this->delegate()->offered_incompatible_qos_status();
}

template <typename T, template <typename Q> class DELEGATE>
const ::dds::core::status::PublicationMatchedStatus
DataWriter<T, DELEGATE>::publication_matched_status()
{
    return this->delegate()->publication_matched_status();
}

//==========================================================================
//== Liveliness Management

template <typename T, template <typename Q> class DELEGATE>
void DataWriter<T, DELEGATE>::assert_liveliness()
{
    this->delegate()->assert_liveliness();
}

}
}

// End of implementation

#endif /* OSPL_DDS_PUB_DATAWRITER_HPP_ */

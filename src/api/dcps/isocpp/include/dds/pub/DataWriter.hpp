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

// Implementation
#include <spec/dds/pub/DataWriter.hpp>
#include <dds/pub/detail/DataWriter.hpp>
#include <dds/pub/AnyDataWriter.hpp>


namespace dds
{
namespace pub
{

class AnyDataWriter;
#ifdef OSPL_2893_COMPILER_BUG
#define DELEGATE dds::pub::detail::DataWriter
template <typename T>
class DataWriter<T, dds::pub::detail::DataWriter> : public dds::core::TEntity< dds::pub::detail::DataWriter<T> >
{
public:
    typedef dds::pub::DataWriterListener<T>              Listener;
    OMG_DDS_REF_TYPE(DataWriter, ::dds::core::TEntity, DELEGATE<T>)

#endif

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif

#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    DataWriter(const dds::pub::Publisher& pub, const dds::topic::Topic<T>& topic) :
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(pub,
                                            topic,
                                            pub.default_datawriter_qos(),
                                            dds::core::status::StatusMask::all()))
    {
        org::opensplice::core::EntityRegistry<DDS::DataWriter_ptr, DataWriter<T, DELEGATE> >::insert(this->delegate()->get_raw_writer(), *this);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif

#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    DataWriter(const dds::pub::Publisher& pub,
               const ::dds::topic::Topic<T>& topic,
               const dds::pub::qos::DataWriterQos& qos,
#                                   ifndef OSPL_2893_COMPILER_BUG
               dds::pub::DataWriterListener<T>* listener,
               const dds::core::status::StatusMask& mask
#                                   else
               dds::pub::DataWriterListener<T>* listener = NULL,
               const dds::core::status::StatusMask& mask = ::dds::core::status::StatusMask::all()
#                                   endif
              ) :
        ::dds::core::TEntity< DELEGATE<T> >(new DELEGATE<T>(pub,
                                            topic,
                                            qos,
                                            mask))
    {
        if(listener)
        {
            dds::pub::detail::EventHandler<DataWriter, T>* h = new dds::pub::detail::EventHandler<DataWriter, T>(*this, listener);
            this->delegate()->event_handler(h, mask);
        }

        org::opensplice::core::EntityRegistry<DDS::DataWriter_ptr, DataWriter<T, DELEGATE> >::insert(this->delegate()->get_raw_writer(), *this);
    }


#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif

#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#else
    virtual
#endif
    ~DataWriter() { }

    //==========================================================================
    //== Write API
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const T& sample)
    {
        this->delegate()->write(sample);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const T& sample, const dds::core::Time& timestamp)
    {
        this->delegate()->write(sample, timestamp);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const T& data, const ::dds::core::InstanceHandle& instance)
    {
        this->delegate()->write(data, instance);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const T& data,
          const ::dds::core::InstanceHandle& instance,
          const dds::core::Time& timestamp)
    {
        this->delegate()->write(data, instance, timestamp);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const dds::topic::TopicInstance<T>& i)
    {
        this->delegate()->write(i);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const dds::topic::TopicInstance<T>& i,
          const dds::core::Time& timestamp)
    {
        this->delegate()->write(i, timestamp);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename FWIterator>
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const FWIterator& begin, const FWIterator& end)
    {
        FWIterator b = begin;
        while(b != end)
        {
            this->delegate()->write(*b);
            ++b;
        }
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename FWIterator>
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const FWIterator& begin, const FWIterator& end,
          const dds::core::Time& timestamp)
    {
        FWIterator b = begin;
        while(b != end)
        {
            this->delegate()->write(*b, timestamp);
            ++b;
        }
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const SamplesFWIterator& data_begin,
          const SamplesFWIterator& data_end,
          const HandlesFWIterator& handle_begin,
          const HandlesFWIterator& handle_end)
    {
        SamplesFWIterator data = data_begin;
        HandlesFWIterator handle = handle_begin;

        while(data != data_end && handle != handle_end)
        {
            this->delegate()->write(*data, *handle);
            ++data;
            ++handle;
        }
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    template <typename SamplesFWIterator, typename HandlesFWIterator>
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    write(const SamplesFWIterator& data_begin,
          const SamplesFWIterator& data_end,
          const HandlesFWIterator& handle_begin,
          const HandlesFWIterator& handle_end,
          const dds::core::Time& timestamp)
    {
        SamplesFWIterator data = data_begin;
        HandlesFWIterator handle = handle_begin;

        while(data != data_end && handle != handle_end)
        {
            this->delegate()->write(*data, *handle, timestamp);
            ++data;
            ++handle;
        }
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    operator << (const T& data)
    {
        this->write(data);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    operator << (const std::pair<T, dds::core::Time>& data)
    {
        this->write(data.first, data.second);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    operator << (const std::pair<T, ::dds::core::InstanceHandle>& data)
    {
        this->write(data.first, data.second);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    operator <<(DataWriter & (*manipulator)(DataWriter&))
    {
        return manipulator(*this);
    }

    //==========================================================================
    //== Instance Management
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const ::dds::core::InstanceHandle
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    register_instance(const T& key)
    {
        return this->delegate()->register_instance(key);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const ::dds::core::InstanceHandle
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    register_instance(const T& key, const dds::core::Time& ts)
    {
        return this->delegate()->register_instance(key, ts);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    unregister_instance(const ::dds::core::InstanceHandle& i)
    {
        this->delegate()->unregister_instance(i);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    unregister_instance(const ::dds::core::InstanceHandle& i, const dds::core::Time& ts)
    {
        this->delegate()->unregister_instance(i, ts);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    dispose_instance(const ::dds::core::InstanceHandle& i)
    {
        this->delegate()->dispose_instance(i);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    dispose_instance(const ::dds::core::InstanceHandle& i, const dds::core::Time& ts)
    {
        this->delegate()->dispose_instance(i, ts);
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::topic::TopicInstance<T>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    key_value(dds::topic::TopicInstance<T>& i, const ::dds::core::InstanceHandle& h)
    {
        return this->delegate()->key_value(i, h);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    T&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    key_value(T& sample, const ::dds::core::InstanceHandle& h)
    {
        return this->delegate()->key_value(sample, h);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::core::InstanceHandle
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    lookup_instance(const T& key)
    {
        return this->delegate()->lookup_instance(key);
    }

    //==========================================================================
    //== QoS Management
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const ::dds::pub::qos::DataWriterQos&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
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
    DataWriter<T, DELEGATE>::
#endif
    qos(const dds::pub::qos::DataWriterQos& qos)
    {
        this->delegate()->qos(qos);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const DataWriter<T, DELEGATE>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    operator >> (::dds::pub::qos::DataWriterQos& qos) const
    {
        qos = this->qos();
        return *this;
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    dds::pub::qos::DataWriterQos&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    operator << (const ::dds::pub::qos::DataWriterQos& qos)
    {
        this->qos(qos);
        return (dds::pub::qos::DataWriterQos&)this->qos();
    }

    //==========================================================================
    //== Entity Navigation
#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::topic::Topic<T>&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    topic() const
    {
        return this->delegate()->topic();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const dds::pub::Publisher&
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    publisher() const
    {
        return this->delegate()->publisher();
    }

    //==========================================================================
    //== ACKs

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    wait_for_acknowledgments(const dds::core::Duration& timeout)
    {
        this->delegate()->wait_for_acknowledgments(timeout);
    }

    //==========================================================================
    //== Listeners Management

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    listener(DataWriterListener<T>* listener,
             const ::dds::core::status::StatusMask& mask)
    {
        dds::pub::detail::EventHandler<DataWriter, T>* h = new dds::pub::detail::EventHandler<DataWriter, T>(*this, listener);
        this->delegate()->event_handler(h, mask);

        //this->delegate()->listener(listener, mask);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    DataWriterListener<T>*
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    listener() const
    {
        return this->delegate()->listener();
    }

    //==========================================================================
    //== Status Management

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const ::dds::core::status::LivelinessLostStatus
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    liveliness_lost_status()
    {
        return this->delegate()->liveliness_lost_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const ::dds::core::status::OfferedDeadlineMissedStatus

#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    offered_deadline_missed_status()
    {
        return this->delegate()->offered_deadline_missed_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const ::dds::core::status::OfferedIncompatibleQosStatus

#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    offered_incompatible_qos_status()
    {
        return this->delegate()->offered_incompatible_qos_status();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    const ::dds::core::status::PublicationMatchedStatus

#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    publication_matched_status()
    {
        return this->delegate()->publication_matched_status();
    }

    //==========================================================================
    //== Liveliness Management

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    assert_liveliness()
    {
        this->delegate()->assert_liveliness();
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    close()
    {
        this->delegate()->close();
        dds::pub::AnyDataWriter adw(*this);
        org::opensplice::core::retain_remove<dds::pub::AnyDataWriter>(adw);
    }

#ifndef OSPL_2893_COMPILER_BUG
    template <typename T, template <typename Q> class DELEGATE>
#endif
    void
#ifndef OSPL_2893_COMPILER_BUG
    DataWriter<T, DELEGATE>::
#endif
    retain()
    {
        this->delegate()->retain();
        dds::pub::AnyDataWriter adr(*this);
        org::opensplice::core::retain_add<dds::pub::AnyDataWriter>(adr);
    }

#ifdef OSPL_2893_COMPILER_BUG
#undef DELEGATE
};
#endif

}
}

// End of implementation

#endif /* OSPL_DDS_PUB_DATAWRITER_HPP_ */

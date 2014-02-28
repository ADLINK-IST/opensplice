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
#ifndef OSPL_DDS_PUB_DETAIL_DATAWRITER_HPP_
#define OSPL_DDS_PUB_DETAIL_DATAWRITER_HPP_

/**
 * @file
 */

// Implementation

#include <dds/topic/Topic.hpp>
#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/topic/TopicTraits.hpp>
#include <org/opensplice/core/memory.hpp>
#include <org/opensplice/pub/qos/QosConverter.hpp>
#include <org/opensplice/core/exception_helper.hpp>
#include <dds/pub/Publisher.hpp>
#include <dds/core/status/Status.hpp>
#include <org/opensplice/core/StatusConverter.hpp>

namespace dds
{
namespace pub
{
template <typename T>
class DataWriterListener;

namespace detail
{
template <typename T>
class DataWriter;
}
}
}

namespace dds
{
namespace pub
{
namespace detail
{
//////////////////////////////////////////////////////////////////////
// Listener
//////////////////////////////////////////////////////////////////////
template <typename T>
class DataWriterListener;

template <typename T>
class DataWriterEventHandler
{
public:
    virtual ~DataWriterEventHandler() { }

public:
    virtual void
    on_offered_deadline_missed(const dds::core::status::OfferedDeadlineMissedStatus& status) = 0;

    virtual void
    on_offered_incompatible_qos(const dds::core::status::OfferedIncompatibleQosStatus& status) = 0;

    virtual void
    on_liveliness_lost(const dds::core::status::LivelinessLostStatus& status) = 0;

    virtual void
    on_publication_matched(const dds::core::status::PublicationMatchedStatus& status) = 0;

    virtual dds::pub::DataWriterListener<T>* listener() = 0;
    virtual void listener(dds::pub::DataWriterListener<T>* l) = 0;
};

template<typename T>
class DataWriterEventForwarder: public DDS::DataWriterListener
{
public:
    DataWriterEventForwarder(
        dds::pub::detail::DataWriterEventHandler<T>* handler) :
        handler_(handler)
    {
    }
    virtual ~DataWriterEventForwarder()
    {
        delete handler_;
    }
public:
    virtual void on_offered_deadline_missed(
        DDS::DataWriter_ptr reader,
        const DDS::OfferedDeadlineMissedStatus& status)
    {
        OMG_DDS_LOG("EVT", "on_offered_deadline_missed");
        dds::core::status::OfferedDeadlineMissedStatus nstatus;
        org::opensplice::core::convertStatus(status, nstatus);
        handler_->on_offered_deadline_missed(nstatus);
    }
    virtual void on_offered_incompatible_qos(
        DDS::DataWriter_ptr reader,
        const DDS::OfferedIncompatibleQosStatus& status)
    {
        OMG_DDS_LOG("EVT", "on_offered_incompatible_qos");
        dds::core::status::OfferedIncompatibleQosStatus nstatus;
        org::opensplice::core::convertStatus(status, nstatus);
        handler_->on_offered_incompatible_qos(nstatus);
    }
    virtual void on_liveliness_lost(DDS::DataWriter_ptr reader,
                                    const DDS::LivelinessLostStatus& status)
    {
        OMG_DDS_LOG("EVT", "on_liveliness_lost");
        dds::core::status::LivelinessLostStatus nstatus;
        org::opensplice::core::convertStatus(status, nstatus);
        handler_->on_liveliness_lost(nstatus);
    }
    virtual void on_publication_matched(DDS::DataWriter_ptr reader,
                                        const DDS::PublicationMatchedStatus& status)
    {
        OMG_DDS_LOG("EVT", "on_publication_matched");
        dds::core::status::PublicationMatchedStatus nstatus;
        org::opensplice::core::convertStatus(status, nstatus);
        handler_->on_publication_matched(nstatus);
    }

    dds::pub::detail::DataWriterEventHandler<T>* handler()
    {
        return handler_;
    }

    dds::pub::detail::DataWriterEventHandler<T>* handler_;
};

template<typename T>
class DataWriter;

template <typename DW, typename T>
class EventHandler : public dds::pub::detail::DataWriterEventHandler<T>
{
public:
    EventHandler(const DW& dw,
                 dds::pub::DataWriterListener<T>* l)
        : dw_(dw),
          listener_(l)
    { }
public:
    virtual void
    on_offered_deadline_missed(const dds::core::status::OfferedDeadlineMissedStatus& status)
    {
        if(listener_ != 0)
        {
            listener_->on_offered_deadline_missed(dw_, status);
        }
    }

    virtual void
    on_offered_incompatible_qos(const dds::core::status::OfferedIncompatibleQosStatus& status)
    {
        if(listener_ != 0)
        {
            listener_->on_offered_incompatible_qos(dw_, status);
        }
    }


    virtual void
    on_liveliness_lost(const dds::core::status::LivelinessLostStatus& status)
    {
        if(listener_ != 0)
        {
            listener_->on_liveliness_lost(dw_, status);
        }
    }

    virtual void
    on_publication_matched(const dds::core::status::PublicationMatchedStatus& status)
    {
        if(listener_ != 0)
        {
            listener_->on_publication_matched(dw_, status);
        }
    }

    dds::pub::DataWriterListener<T>*
    listener()
    {
        return listener_;
    }

    void listener(dds::pub::DataWriterListener<T>* l)
    {
        listener_ = l;
    }


private:
    DW dw_;
    dds::pub::DataWriterListener<T>* listener_;
};

}
}
}

template <typename T>
class dds::pub::detail::DataWriter : public  org::opensplice::core::EntityDelegate
{
public:
    typedef typename org::opensplice::topic::topic_data_writer<T>::type DW;

public:
    DataWriter(const ::dds::pub::Publisher& pub,
               const dds::topic::Topic<T>& topic,
               const ::dds::pub::qos::DataWriterQos& qos,
               const dds::core::status::StatusMask& mask)
        :   pub_(pub),
            topic_(topic),
            qos_(qos),
            event_forwarder_(0),
            mask_(mask)
    {
        DDS::DataWriterQos dwqos = convertQos(qos);

        DDS::DataWriter_var w =
            pub_->pub_->create_datawriter(topic_->t_,
                                          dwqos,
                                          event_forwarder_,
                                          mask.to_ulong());
        if(w.in() == 0)
            throw dds::core::NullReferenceError(org::opensplice::core::exception_helper(
                                                    OSPL_CONTEXT_LITERAL(
                                                            "dds::core::NullReferenceError : Unable to create DataWriter. "
                                                            "Nil return from ::create_datawriter")));

        raw_writer_ = DW::_narrow(w.in());

        org::opensplice::core::DDS_DW_REF tmp = org::opensplice::core::DDS_DW_REF(raw_writer_, org::opensplice::core::DWDeleter(pub_->pub_));
        writer_ = tmp;
        entity_ = DDS::Entity::_narrow(raw_writer_);
    }


    virtual ~DataWriter()
    {
        if(event_forwarder_ != 0)
        {
            DDS::ReturnCode_t result = raw_writer_->set_listener(0, DDS::STATUS_MASK_NONE);
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_listener"));
            DDS::release(event_forwarder_);
        }
    }


public:
    void write(const T& sample)
    {
        DDS::ReturnCode_t result = raw_writer_->write(sample, DDS::HANDLE_NIL);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::write"));
    }

    void write(const T& sample, const dds::core::Time& timestamp)
    {
        DDS::Time_t ddsTime;
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddsTime.sec = static_cast<DDS::Long>(timestamp.sec());
        ddsTime.nanosec = timestamp.nanosec();

        DDS::ReturnCode_t result = raw_writer_->write_w_timestamp(sample, DDS::HANDLE_NIL, ddsTime);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::write_w_timestamp"));
    }

    void write(const T& sample, const ::dds::core::InstanceHandle& instance)
    {
        DDS::ReturnCode_t result = raw_writer_->write(sample, instance->handle());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::write"));
    }

    void write(const T& sample,
               const ::dds::core::InstanceHandle& instance,
               const dds::core::Time& timestamp)
    {
        DDS::Time_t ddsTime;
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddsTime.sec = static_cast<DDS::Long>(timestamp.sec());
        ddsTime.nanosec = timestamp.nanosec();

        DDS::ReturnCode_t result = raw_writer_->write_w_timestamp(sample, instance->handle(), ddsTime);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::write_w_timestamp"));
    }

    void write(const dds::topic::TopicInstance<T>& i)
    {
        DDS::ReturnCode_t result = raw_writer_->write(i.sample(), i.handle()->handle());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::write"));
    }

    void write(const dds::topic::TopicInstance<T>& i,
               const dds::core::Time& timestamp)
    {
        DDS::Time_t ddsTime;
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddsTime.sec = static_cast<DDS::Long>(timestamp.sec());
        ddsTime.nanosec = timestamp.nanosec();

        DDS::ReturnCode_t result = raw_writer_->write_w_timestamp(i.sample(), i.handle()->handle(), ddsTime);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::write_w_timestamp"));
    }

    const ::dds::core::InstanceHandle register_instance(const T& key)
    {
        return dds::core::InstanceHandle(raw_writer_->register_instance(key));
    }

    const ::dds::core::InstanceHandle register_instance(const T& key, const dds::core::Time& ts)
    {
        DDS::Time_t ddsTime;
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddsTime.sec = static_cast<DDS::Long>(ts.sec());
        ddsTime.nanosec = ts.nanosec();

        return dds::core::InstanceHandle(raw_writer_->register_instance_w_timestamp(key, ddsTime));
    }

    void unregister_instance(const ::dds::core::InstanceHandle& i)
    {
        T sample;
        raw_writer_->get_key_value(sample, i->handle());
        raw_writer_->unregister_instance(sample, i->handle());
    }

    void unregister_instance(const ::dds::core::InstanceHandle& i, const dds::core::Time& ts)
    {
        DDS::Time_t ddsTime;
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddsTime.sec = static_cast<DDS::Long>(ts.sec());
        ddsTime.nanosec = ts.nanosec();

        T sample;
        raw_writer_->get_key_value(sample, i->handle());

        raw_writer_->unregister_instance_w_timestamp(sample, i->handle(), ddsTime);
    }

    void dispose_instance(const ::dds::core::InstanceHandle& i)
    {
        T sample;
        raw_writer_->get_key_value(sample, i->handle());

        raw_writer_->dispose(sample, i->handle());
    }

    void dispose_instance(const ::dds::core::InstanceHandle& i, const dds::core::Time& ts)
    {
        DDS::Time_t ddsTime;
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddsTime.sec = static_cast<DDS::Long>(ts.sec());
        ddsTime.nanosec = ts.nanosec();

        T sample;
        raw_writer_->get_key_value(sample, i->handle());

        raw_writer_->dispose_w_timestamp(sample, i->handle(), ddsTime);
    }

    dds::topic::TopicInstance<T>& key_value(dds::topic::TopicInstance<T>& i, const ::dds::core::InstanceHandle& h)
    {
        T sample;
        raw_writer_->get_key_value(sample, h->handle());

        i.handle(h);
        i.sample(sample);

        return i;
    }

    T& key_value(T& sample, const ::dds::core::InstanceHandle& h)
    {
        raw_writer_->get_key_value(sample, h->handle());
        return sample;
    }

    dds::core::InstanceHandle lookup_instance(const T& key)
    {
        return dds::core::InstanceHandle(raw_writer_->lookup_instance(key));
    }

    const ::dds::pub::qos::DataWriterQos& qos() const
    {
        return qos_;
    }

    void qos(const dds::pub::qos::DataWriterQos& qos)
    {
        DDS::ReturnCode_t result = raw_writer_->set_qos(org::opensplice::pub::qos::convertQos(qos));
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_qos"));

        qos_ = qos;
    }

    const dds::topic::Topic<T>& topic() const
    {
        return topic_;
    }

    const dds::pub::Publisher& publisher() const
    {
        return pub_;
    }

    void wait_for_acknowledgments(const ::dds::core::Duration& timeout)
    {
        DDS::Duration_t ddsTime;
        /** @internal @bug OSPL-2308 RTF Time-ish coercion issue
            @see http://jira.prismtech.com:8080/browse/OSPL-2308 */
        ddsTime.sec = static_cast<DDS::Long>(timeout.sec());
        ddsTime.nanosec = timeout.nanosec();

        DDS::ReturnCode_t result = raw_writer_->wait_for_acknowledgments(ddsTime);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::wait_for_acknowledgments"));
    }

    void event_handler(dds::pub::detail::DataWriterEventHandler<T>* handler,
                       const dds::core::status::StatusMask& mask)
    {
        if(handler == 0)
        {
            if(event_forwarder_)
            {
                DDS::release(event_forwarder_);
                event_forwarder_ = 0;
            }
        }
        else if(event_forwarder_ == 0)
        {
            event_forwarder_ =
                new DataWriterEventForwarder<T>(handler);
        }
        else
        {
            event_forwarder_->handler_ = handler;
        }
        DDS::ReturnCode_t result = raw_writer_->set_listener(event_forwarder_, mask.to_ulong());
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::set_listener"));
    }

    dds::pub::DataWriterListener<T>* listener()
    {
        return event_forwarder_->handler()->listener();
    }

    //========================================================================
    //== Status API

    const dds::core::status::LivelinessLostStatus&
    liveliness_lost_status()
    {
        DDS::LivelinessLostStatus status;
        DDS::ReturnCode_t result = raw_writer_->get_liveliness_lost_status(status);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_liveliness_lost_status"));

        org::opensplice::core::convertStatus(status, liveliness_lost_status_);

        return liveliness_lost_status_;
    }

    const dds::core::status::OfferedDeadlineMissedStatus&
    offered_deadline_missed_status()
    {
        DDS::OfferedDeadlineMissedStatus status;
        DDS::ReturnCode_t result = raw_writer_->get_offered_deadline_missed_status(status);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_offered_deadline_missed_status"));

        org::opensplice::core::convertStatus(status, offered_deadline_missed_status_);

        return offered_deadline_missed_status_;
    }

    const dds::core::status::OfferedIncompatibleQosStatus&
    offered_incompatible_qos_status()
    {
        DDS::OfferedIncompatibleQosStatus status;
        DDS::ReturnCode_t result = raw_writer_->get_offered_incompatible_qos_status(status);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_offered_incompatible_qos_status"));

        org::opensplice::core::convertStatus(status, offered_incompatible_qos_status_);

        return offered_incompatible_qos_status_;
    }

    const dds::core::status::PublicationMatchedStatus&
    publication_matched_status()
    {
        DDS::PublicationMatchedStatus status;
        DDS::ReturnCode_t result = raw_writer_->get_publication_matched_status(status);
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::get_publication_matched_status"));

        org::opensplice::core::convertStatus(status, publication_matched_status_);

        return publication_matched_status_;
    }

    //==========================================================================
    //== Liveliness Management

    void assert_liveliness()
    {
        raw_writer_->assert_liveliness();
    }

    DW* get_raw_writer()
    {
        return raw_writer_;
    }

    void close()
    {
        org::opensplice::core::DWDeleter* d = OSPL_CXX11_STD_MODULE::get_deleter<org::opensplice::core::DWDeleter>(writer_);
        if(d)
        {
            d->close(raw_writer_);
        }
    }

    void retain()
    {

    }

private:
    ::dds::pub::Publisher             pub_;
    dds::topic::Topic<T>            topic_;
    ::dds::pub::qos::DataWriterQos    qos_;
    DataWriterEventForwarder<T>* event_forwarder_;
    dds::core::status::StatusMask     mask_;

    org::opensplice::core::DDS_DW_REF writer_;
    DW* raw_writer_;

    dds::core::status::LivelinessLostStatus liveliness_lost_status_;
    dds::core::status::OfferedDeadlineMissedStatus offered_deadline_missed_status_;
    dds::core::status::OfferedIncompatibleQosStatus offered_incompatible_qos_status_;
    dds::core::status::PublicationMatchedStatus publication_matched_status_;
};


// End of implementation

#endif /* OSPL_DDS_PUB_DETAIL_DATAWRITER_HPP_ */

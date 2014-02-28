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


/**
 * @file
 */

#ifndef ORG_OPENSPLICE_SUB_SUBSCRIBER_DELEGATE_HPP_
#define ORG_OPENSPLICE_SUB_SUBSCRIBER_DELEGATE_HPP_


#include <org/opensplice/core/EntityDelegate.hpp>
#include <dds/core/ref_traits.hpp>
#include <dds/core/status/State.hpp>
#include <dds/core/cond/StatusCondition.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/domain/DomainParticipant.hpp>

namespace dds
{
namespace sub
{
class SubscriberListener;
}
}

namespace org
{
namespace opensplice
{
namespace sub
{

class OSPL_ISOCPP_IMPL_API SubscriberDelegate : public org::opensplice::core::EntityDelegate
{
public:

public:
    SubscriberDelegate(const dds::domain::DomainParticipant& dp,
                       const dds::sub::qos::SubscriberQos& qos,
                       const dds::core::status::StatusMask& event_mask);

    /**
     *  @internal This function initialises the delegate as the built in subscriber
     */
    void init_builtin(DDS::Subscriber_ptr sub);

    /**
     *  @internal Checks if the subscriber is the built in subscriber
     */
    bool is_builtin() const;

    virtual ~SubscriberDelegate();


public:
    /**
     *  @internal This operation invokes the operation on_data_available on the
     * DataReaderListener objects attached to contained DataReader
     * entities with a DATA_AVAILABLE status that is considered changed
     * as described in Section 7.1.4.2.2, Changes in Read Communication
     * Statuses.
     */
    void notify_datareaders();

    const dds::domain::DomainParticipant& participant() const;

    const dds::sub::qos::SubscriberQos& qos() const;

    void qos(const dds::sub::qos::SubscriberQos& sqos);

    dds::sub::qos::DataReaderQos default_datareader_qos();

    void default_datareader_qos(const dds::sub::qos::DataReaderQos& qos);

    void event_forwarder(dds::sub::SubscriberListener* listener,
                         const dds::core::smart_ptr_traits<DDS::SubscriberListener>::ref_type& forwarder,
                         const dds::core::status::StatusMask& event_mask);

    dds::sub::SubscriberListener* listener() const;

    void close();

    /** @internal @todo OSPL-1944 Subscriber Listener should return list of affected DataReaders (on_data_on_readers) **/
    //dds::sub::AnyDataReader get_datareaders();

    dds::domain::DomainParticipant dp_;
private:
    dds::sub::qos::SubscriberQos qos_;
    dds::sub::SubscriberListener* listener_;
    dds::core::status::StatusMask mask_;
    dds::sub::qos::DataReaderQos default_dr_qos_;

public:
    dds::core::smart_ptr_traits<DDS::Subscriber>::ref_type sub_;
    dds::core::smart_ptr_traits<DDS::SubscriberListener>::ref_type sub_event_forwarder_;
};

}
}
}

#endif /* ORG_OPENSPLICE_SUB_SUBSCRIBER_DELEGATE_HPP_ */

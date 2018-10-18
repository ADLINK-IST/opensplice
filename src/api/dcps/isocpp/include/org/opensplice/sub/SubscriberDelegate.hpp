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

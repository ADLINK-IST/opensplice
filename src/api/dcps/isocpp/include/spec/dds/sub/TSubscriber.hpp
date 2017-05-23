#ifndef OMG_TDDS_SUB_SUBSCRIBER_HPP_
#define OMG_TDDS_SUB_SUBSCRIBER_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <dds/core/TEntity.hpp>
#include <dds/core/cond/StatusCondition.hpp>
#include <dds/domain/DomainParticipant.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>

namespace dds
{
namespace sub
{
template <typename DELEGATE>
class TSubscriber;

class SubscriberListener;
}
}

/**
 * A Subscriber is the object responsible for the actual reception of the
 * data resulting from its subscriptions.
 *
 * A Subscriber acts on the behalf of one or several DataReader objects
 * that are related to it. When it receives data (from the other parts of
 * the system), it builds the list of concerned DataReader objects, and then
 * indicates to the application that data is available, through its listener
 * or by enabling related conditions. The application can access the list of
 * concerned DataReader objects through the operation get_datareaders and
 * then access the data available through operations on the DataReader.
 *
 *  See \ref DCPS_Modules_Subscriber "Subscriber" for more information
 */
template <typename DELEGATE>
class dds::sub::TSubscriber : public dds::core::TEntity<DELEGATE>
{
public:
    OMG_DDS_REF_TYPE(TSubscriber, dds::core::TEntity, DELEGATE)

public:
    typedef dds::sub::SubscriberListener                 Listener;

public:
    /**
     * Create a Subscriber attached to the given domain participant.
     * The subscriber QoS will be set to the default as provided by the domain
     * participant.
     *
     * @param dp the domain participant that will own this subscriber
     */
    TSubscriber(const ::dds::domain::DomainParticipant& dp);

    /**
     * Create a Subscriber attached to the given domain participant.
     *
     * @param dp the domain participant that will own this subscriber
     * @param qos the subscriber qos
     * @param listener the subscriber listener
     * @param mask the listener event mask
     */
    TSubscriber(const ::dds::domain::DomainParticipant& dp,
                const dds::sub::qos::SubscriberQos& qos,
                dds::sub::SubscriberListener* listener = NULL,
                const dds::core::status::StatusMask& mask = dds::core::status::StatusMask::none());

public:
    virtual ~TSubscriber();

public:
    /**
     * This operation invokes the operation on_data_available on the
     * DataReaderListener objects attached to contained DataReader
     * entities with a data_available status that is considered changed
     * as described in Section 7.1.4.2.2, Changes in Read Communication
     * Statuses.
     */
    void notify_datareaders();

    /**
     * By virtue of extending Entity, a Subscriber can be attached to a
     * Listener at creation time or later by using the set_listener
     * operation. The Listener attached must extend SubscriberListener.
     * Listeners are described in Section 7.1.4, “Listeners, Conditions,
     * and Wait-sets,”.
     *
     * @param listener the listener
     * @param event_mask the event mask for the listener
     */
    void listener(Listener* listener,
                  const dds::core::status::StatusMask& event_mask);

    /**
     * Get the Subscriber listener.
     *
     * @return the Subscriber listener
     */
    Listener* listener() const;

    /**
     * Get the Subscriber QoS.
     *
     * @return the Subscriber QoS
     */
    const dds::sub::qos::SubscriberQos& qos() const;

    /**
     * Set the new qos policies for this subscriber.
     *
     * @param sqos the new subscriber QoS
     */
    void qos(const dds::sub::qos::SubscriberQos& sqos);

    /**
     * Get the default DataReader QoS.
     *
     * @return the default DataReader QoS
     */
    dds::sub::qos::DataReaderQos default_datareader_qos() const;

    /**
     * Set the default DataReader QoS.
     *
     * @param qos the default DataReader QoS.
     */
    TSubscriber& default_datareader_qos(const dds::sub::qos::DataReaderQos& qos);

    /**
     * Return the DomainParticipant that owns this Subscriber.
     *
     * @return the DomainParticipant that owns this Subscriber
     */
    const dds::domain::DomainParticipant& participant() const;

    /**
     *
     *  allows the application to access the DataReader objects that contain samples with the
     *  specified sample_states, view_states, and instance_states.
     *
     *  The PRESENTATION QosPolicy of the Subscriber affects the order and occurrences of readers
     *  returned by this operation:
     *  If the PRESENTATION QosPolicy of the Subscriber to which the DataReader belongs
     *  has the access_scope set to 'GROUP'. This operation should only be invoked inside a
     *  begin_access/end_access block. Otherwise it will return the error PRECONDITION_NOT_MET.
     *
     *  Depending on the setting of the PRESENTATION QoS policy the returned collection of DataReader
     *  objects may be a 'set' containing each DataReader at most once in no specified order, or a
     *  'list' containing each DataReader one or more times in a specific order.
     *  - If PRESENTATION access_scope is INSTANCE or TOPIC, the returned collection is a 'set'.
     *  - If PRESENTATION access_scope is GROUP and ordered_access is set to TRUE, then the returned
     *    collection is a 'list'. This difference is due to the fact that, in the second situation
     *    it is required to access samples belonging to different DataReader objects in a particular
     *    order. In this case, the application should process each DataReader in the same order it
     *    appears in the 'list' and read or take exactly one sample from each DataReader.
     *
     * @return list of data reaader
     */
    //dds::sub::AnyDataReader get_datareaders();

    /**
     * Set the QoS associated with this Subscriber.
     *
     * @param qos the new Subscriber QoS
     */
    dds::sub::qos::SubscriberQos& operator << (const dds::sub::qos::SubscriberQos& qos);

    /**
     * Get the QoS associated with this Subscriber.
     *
     * @param qos the current Subscriber QoS
     */
    const TSubscriber& operator >> (dds::sub::qos::SubscriberQos& qos) const;

    /**
     * This function closes the entity and releases all resources associated with
     * DDS, such as threads, sockets, buffers, etc. Any attempt to invoke
     * functions on a closed entity will raise an exception.
     */
    virtual void close();

    /**
     * Indicates that references to this object may go out of scope but that
     * the application expects to look it up again later. Therefore, the
     * Service must consider this object to be still in use and may not
     * close it automatically.
     */
    virtual void retain();
};


#endif /* OMG_TDDS_SUB_SUBSCRIBER_HPP_ */

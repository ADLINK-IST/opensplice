/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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
#ifndef OSPL_DDS_PUB_TPUBLISHER_HPP_
#define OSPL_DDS_PUB_TPUBLISHER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/pub/TPublisher.hpp>

// Implementation
#include <org/opensplice/pub/PublisherEventForwarder.hpp>
#include <org/opensplice/core/Retain.hpp>
#include <org/opensplice/core/EntityRegistry.hpp>

namespace dds
{
namespace pub
{

template <typename DELEGATE>
TPublisher<DELEGATE>::TPublisher(const dds::domain::DomainParticipant& dp)
    :   ::dds::core::TEntity<DELEGATE>(new DELEGATE(dp,
                                       dp.default_publisher_qos(),
                                       dds::core::status::StatusMask::none()))
{
    org::opensplice::core::EntityRegistry<DDS::Publisher_ptr, dds::pub::TPublisher<DELEGATE> >::insert(this->delegate()->pub_.get(), *this);
}

template <typename DELEGATE>
TPublisher<DELEGATE>::TPublisher(const dds::domain::DomainParticipant& dp,
                                 const dds::pub::qos::PublisherQos& qos,
                                 dds::pub::PublisherListener* listener,
                                 const dds::core::status::StatusMask& mask)
    :   ::dds::core::TEntity<DELEGATE>(new DELEGATE(dp, qos, mask))
{
    if(listener)
    {
        dds::core::smart_ptr_traits<DDS::PublisherListener>::ref_type h(new org::opensplice::pub::PublisherEventForwarder<TPublisher>(*this, listener));
        this->delegate()->event_forwarder(listener, h, mask);
    }
    org::opensplice::core::EntityRegistry<DDS::Publisher_ptr, dds::pub::TPublisher<DELEGATE> >::insert(this->delegate()->pub_.get(), *this);
}

template <typename DELEGATE>
TPublisher<DELEGATE>::~TPublisher() { }

template <typename DELEGATE>
const dds::pub::qos::PublisherQos& TPublisher<DELEGATE>::qos() const
{
    return this->delegate()->qos();
}

template <typename DELEGATE>
void TPublisher<DELEGATE>::qos(const dds::pub::qos::PublisherQos& pqos)
{
    this->delegate()->qos(pqos);
}

template <typename DELEGATE>
dds::pub::qos::PublisherQos& TPublisher<DELEGATE>::operator <<(const dds::pub::qos::PublisherQos& qos)
{
    this->qos(qos);
    return (dds::pub::qos::PublisherQos&)this->qos();
}

template <typename DELEGATE>
TPublisher<DELEGATE>& TPublisher<DELEGATE>::operator >> (dds::pub::qos::PublisherQos& qos)
{
    qos = this->qos();
    return *this;
}

template <typename DELEGATE>
TPublisher<DELEGATE>& TPublisher<DELEGATE>::default_datawriter_qos(const dds::pub::qos::DataWriterQos& dwqos)
{
    this->delegate()->default_datawriter_qos(dwqos);
    return *this;
}

template <typename DELEGATE>
dds::pub::qos::DataWriterQos TPublisher<DELEGATE>::default_datawriter_qos() const
{
    return this->delegate()->default_datawriter_qos();
}

template <typename DELEGATE>
void TPublisher<DELEGATE>::listener(Listener* plistener, const dds::core::status::StatusMask& event_mask)
{

    dds::core::smart_ptr_traits<DDS::PublisherListener>::ref_type h(new org::opensplice::pub::PublisherEventForwarder<TPublisher>(*this, plistener));
    this->delegate()->event_forwarder(plistener, h, event_mask);

    //this->delegate()->listener(plistener, event_mask);
}

template <typename DELEGATE>
typename TPublisher<DELEGATE>::Listener* TPublisher<DELEGATE>::listener() const
{
    return this->delegate()->listener();
}

template <typename DELEGATE>
void TPublisher<DELEGATE>::wait_for_acknowledgments(const dds::core::Duration& timeout)
{
    this->delegate()->wait_for_acknowledgments(timeout);
}

template <typename DELEGATE>
const dds::domain::DomainParticipant& TPublisher<DELEGATE>::participant() const
{
    return this->delegate()->participant();
}

template <typename DELEGATE>
void
TPublisher<DELEGATE>::close()
{
    this->delegate()->close();
    org::opensplice::core::retain_remove<TPublisher<DELEGATE> >(*this);
}

template <typename DELEGATE>
void
TPublisher<DELEGATE>::retain()
{
    this->delegate()->retain();
    org::opensplice::core::retain_add<TPublisher<DELEGATE> >(*this);
}

}
}

// End of implementation

#endif /* OSPL_DDS_PUB_TPUBLISHER_HPP_ */

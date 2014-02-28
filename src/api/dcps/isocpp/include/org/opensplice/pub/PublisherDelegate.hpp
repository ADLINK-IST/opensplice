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

#ifndef OMG_IDDS_PUB_PUBLISHER_IMPL_HPP_
#define OMG_IDDS_PUB_PUBLISHER_IMPL_HPP_

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

#include <dds/core/types.hpp>
#include <dds/core/ref_traits.hpp>
#include <dds/core/Duration.hpp>
#include <dds/core/status/State.hpp>
#include <dds/core/cond/StatusCondition.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/domain/DomainParticipant.hpp>

#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/core/config.hpp>


namespace org
{
namespace opensplice
{
namespace pub
{
class PublisherDelegate;
}
}
}

namespace dds
{
namespace pub
{
class PublisherListener;
}
}

namespace org
{
namespace opensplice
{
namespace pub
{

class OSPL_ISOCPP_IMPL_API PublisherDelegate : public org::opensplice::core::EntityDelegate
{
public:

    PublisherDelegate(const dds::domain::DomainParticipant& dp,
                      const dds::pub::qos::PublisherQos& qos,
                      const dds::core::status::StatusMask& event_mask);

    virtual ~PublisherDelegate();
    const dds::pub::qos::PublisherQos& qos() const;

    void qos(const dds::pub::qos::PublisherQos& pqos);

    void
    wait_for_acknowledgments(const dds::core::Duration& max_wait);

    const dds::domain::DomainParticipant& participant() const;


    bool suspend_publications();

    bool resume_publications();

    void
    begin_coherent_changes();

    void
    end_coherent_changes();

    void close();

    void retain();

    void default_datawriter_qos(const dds::pub::qos::DataWriterQos& dwqos);

    dds::pub::qos::DataWriterQos default_datawriter_qos();

    void event_forwarder(dds::pub::PublisherListener* listener,
                         const dds::core::smart_ptr_traits<DDS::PublisherListener>::ref_type& forwarder,
                         const dds::core::status::StatusMask& event_mask);

    dds::pub::PublisherListener* listener() const;

private:
    dds::domain::DomainParticipant dp_;
    dds::pub::qos::PublisherQos qos_;
    dds::pub::PublisherListener* listener_;
    dds::core::status::StatusMask mask_;
    dds::pub::qos::DataWriterQos default_dwqos_;

public:
    dds::core::smart_ptr_traits<DDS::Publisher>::ref_type pub_;
    dds::core::smart_ptr_traits<DDS::PublisherListener>::ref_type pub_event_forwarder_;
};
}
}
}

#endif /* OMG_IDDS_PUB_PUBLISHER_IMPL_HPP_ */

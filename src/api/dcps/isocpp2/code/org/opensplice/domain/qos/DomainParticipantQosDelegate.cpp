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


/**
 * @file
 */

#include <org/opensplice/core/ReportUtils.hpp>
#include <org/opensplice/domain/qos/DomainParticipantQosDelegate.hpp>

#include "u_participantQos.h"

#include "dds_namedQosTypesSplType.h"

namespace org
{
namespace opensplice
{
namespace domain
{
namespace qos
{

DomainParticipantQosDelegate::DomainParticipantQosDelegate()
{
    this->defaults();
}

DomainParticipantQosDelegate::DomainParticipantQosDelegate(
    const DomainParticipantQosDelegate& other)
    :  user_data_(other.user_data_),
       entity_factory_(other.entity_factory_),
       listener_scheduling_(other.listener_scheduling_),
       watchdog_scheduling_(other.watchdog_scheduling_)
{
}

DomainParticipantQosDelegate::~DomainParticipantQosDelegate()
{
}

void
DomainParticipantQosDelegate::policy(const dds::core::policy::UserData& user_data)
{
    user_data.delegate().check();
    user_data_ = user_data;
}

void
DomainParticipantQosDelegate::policy(const dds::core::policy::EntityFactory& entity_factory)
{
    entity_factory.delegate().check();
    entity_factory_ = entity_factory;
}

void
DomainParticipantQosDelegate::policy(const org::opensplice::core::policy::ListenerScheduling& listener_scheduling)
{
    listener_scheduling.delegate().check();
    listener_scheduling_ = listener_scheduling;
}

void
DomainParticipantQosDelegate::policy(const org::opensplice::core::policy::WatchdogScheduling& watchdog_scheduling)
{
    watchdog_scheduling.delegate().check();
    watchdog_scheduling_ = watchdog_scheduling;
}

u_participantQos
DomainParticipantQosDelegate::u_qos() const
{
    u_participantQos qos = u_participantQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    qos->userData           = user_data_.delegate().v_policyI();
    qos->entityFactory      = entity_factory_.delegate().v_policyI();
    qos->watchdogScheduling = watchdog_scheduling_.delegate().v_policyI();
    /* u_participantQos does not contain a listenerScheduling. */
    return qos;
}

void
DomainParticipantQosDelegate::u_qos(const u_participantQos qos)
{
    assert(qos);
    user_data_.delegate().v_policyI(qos->userData);
    entity_factory_.delegate().v_policyI(qos->entityFactory);
    watchdog_scheduling_.delegate().v_policyI(qos->watchdogScheduling);
    /* u_participantQos does not contain a listenerScheduling. So, use the default.  */
    listener_scheduling_ = org::opensplice::core::policy::ListenerScheduling();
}

void
DomainParticipantQosDelegate::named_qos(const struct _DDS_NamedDomainParticipantQos &qos)
{
    /* We only need the QoS part of the named QoS. */
    const struct _DDS_DomainParticipantQos *q = &qos.domainparticipant_qos;
    /* The idl policies are aligned the same as the kernel/builtin representation.
     * So, cast and use the kernel policy translations (or builtin when available). */
    user_data_          .delegate().v_policy((v_builtinUserDataPolicy&)(q->user_data          ));
    entity_factory_     .delegate().v_policy((v_entityFactoryPolicy&)  (q->entity_factory     ));
    watchdog_scheduling_.delegate().v_policyI((v_schedulePolicyI&)     (q->watchdog_scheduling));
    listener_scheduling_.delegate().v_policyI((v_schedulePolicyI&)     (q->listener_scheduling));
}

void
DomainParticipantQosDelegate::check() const
{
    /* Policies are checked when set.
     * No consistency check between policies needed. */
}

bool
DomainParticipantQosDelegate::operator ==(const DomainParticipantQosDelegate& other) const
{
    return other.user_data_           == user_data_ &&
           other.entity_factory_      == entity_factory_ &&
           other.listener_scheduling_ == listener_scheduling_ &&
           other.watchdog_scheduling_ == watchdog_scheduling_;

}

DomainParticipantQosDelegate&
DomainParticipantQosDelegate::operator = (const DomainParticipantQosDelegate& other)
{
    user_data_           = other.user_data_;
    entity_factory_      = other.entity_factory_;
    listener_scheduling_ = other.listener_scheduling_;
    watchdog_scheduling_ = other.watchdog_scheduling_;
    return *this;
}

void
DomainParticipantQosDelegate::defaults()
{
    /* Get default QoS from userlayer and copy result. */
    u_participantQos qos = u_participantQosNew(NULL);
    if (!qos) {
        ISOCPP_THROW_EXCEPTION(ISOCPP_OUT_OF_RESOURCES_ERROR, "Could not create internal QoS.");
    }
    this->u_qos(qos);
    u_participantQosFree(qos);
}

}
}
}
}

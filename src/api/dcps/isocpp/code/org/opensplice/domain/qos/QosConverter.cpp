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

#include <org/opensplice/domain/qos/QosConverter.hpp>
#include <org/opensplice/core/policy/PolicyConverter.hpp>

using namespace org::opensplice::core::policy;

dds::domain::qos::DomainParticipantQos
org::opensplice::domain::qos::convertQos(const DDS::DomainParticipantQos& from)
{
    dds::domain::qos::DomainParticipantQos to;
    to = to << convertPolicy(from.user_data) << convertPolicy(from.entity_factory);
    return to;
}

DDS::DomainParticipantQos
org::opensplice::domain::qos::convertQos(const dds::domain::qos::DomainParticipantQos& from)
{
    DDS::DomainParticipantQos to;

    DDS::UserDataQosPolicy user_data = convertPolicy(from.policy<dds::core::policy::UserData>());
    to.user_data.value = user_data.value;

    DDS::EntityFactoryQosPolicy entity_factory = convertPolicy(from.policy<dds::core::policy::EntityFactory>());
    to.entity_factory.autoenable_created_entities = entity_factory.autoenable_created_entities;

    to.watchdog_scheduling.scheduling_class.kind = DDS::SCHEDULE_DEFAULT;
    to.watchdog_scheduling.scheduling_priority_kind.kind = DDS::PRIORITY_RELATIVE;
    to.watchdog_scheduling.scheduling_priority = 0;

    to.listener_scheduling.scheduling_class.kind = DDS::SCHEDULE_DEFAULT;
    to.listener_scheduling.scheduling_priority_kind.kind = DDS::PRIORITY_RELATIVE;
    to.listener_scheduling.scheduling_priority = 0;

    return to;
}

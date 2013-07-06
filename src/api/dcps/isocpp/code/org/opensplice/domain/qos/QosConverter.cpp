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

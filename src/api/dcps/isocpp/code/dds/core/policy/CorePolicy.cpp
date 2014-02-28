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

#include <dds/core/detail/conformance.hpp>
#include <dds/core/policy/CorePolicy.hpp>

OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::UserData,           "UserData")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Durability,         "Durability")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Presentation,       "Presentation")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Deadline,           "Deadline")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::LatencyBudget,      "LatencyBudget")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::TimeBasedFilter,    "TimeBasedFilter")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Ownership,          "Ownership")

#ifdef  OMG_DDS_OWNERSHIP_SUPPORT
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::OwnershipStrength,  "OwnershipStrength")
#endif  // OMG_DDS_OWNERSHIP_SUPPORT

OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Liveliness,         "Liveliness")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Partition,          "Partition")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Reliability,        "Reliability")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::DestinationOrder,   "DestinationOrder")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::History,            "History")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::ResourceLimits,     "ResourceLimits")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::EntityFactory,      "EntityFactory")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::WriterDataLifecycle, "WriterDataLifecycle")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::ReaderDataLifecycle, "ReaderDataLifecycle")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::TopicData,          "TopicData")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::GroupData,          "GroupData")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::TransportPriority,  "TransportPriority")
OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::Lifespan,           "Lifespan")

#ifdef  OMG_DDS_PERSISTENCE_SUPPORT

OMG_DDS_DEFINE_POLICY_TRAITS(dds::core::policy::DurabilityService,  "DurabilityService")

#endif  // OMG_DDS_PERSISTENCE_SUPPORT

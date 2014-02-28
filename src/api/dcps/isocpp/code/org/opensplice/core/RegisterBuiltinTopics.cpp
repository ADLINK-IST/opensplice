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

#include <org/opensplice/core/RegisterBuiltinTopics.hpp>
#include <org/opensplice/core/EntityRegistry.hpp>

INSTANTIATE_TYPED_REGISTRIES(::DDS::ParticipantBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::TopicBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::PublicationBuiltinTopicData)

INSTANTIATE_TYPED_REGISTRIES(::DDS::SubscriptionBuiltinTopicData)
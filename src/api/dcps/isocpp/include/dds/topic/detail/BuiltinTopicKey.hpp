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
#ifndef OSPL_DDS_TOPIC_DETAIL_BUILTINTOPICKEY_HPP_
#define OSPL_DDS_TOPIC_DETAIL_BUILTINTOPICKEY_HPP_

/**
 * @file
 */

// Implementation

#include <org/opensplice/topic/BuiltinTopicKeyImpl.hpp>
#include <dds/topic/TBuiltinTopicKey.hpp>

namespace dds
{
namespace topic
{
namespace detail
{
typedef dds::topic::TBuiltinTopicKey<org::opensplice::topic::BuiltinTopicKeyImpl> BuiltinTopicKey;
}
}
}

// End of implementation

#endif /* OSPL_DDS_TOPIC_DETAIL_BUILTINTOPICKEY_HPP_ */

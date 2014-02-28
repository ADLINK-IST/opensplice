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

#ifndef ORG_OPENSPLICE_TOPIC_QOS_QOSCONVERTER_HPP_
#define ORG_OPENSPLICE_TOPIC_QOS_QOSCONVERTER_HPP_

#include <dds/core/types.hpp>
#include <dds/topic/qos/TopicQos.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace topic
{
namespace qos
{
dds::topic::qos::TopicQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::TopicQos& from);

DDS::TopicQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::topic::qos::TopicQos& from);
}
}
}
}

#endif /* ORG_OPENSPLICE_TOPIC_QOS_QOSCONVERTER_HPP_ */

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

#ifndef ORG_OPENSPLICE_PUB_QOS_QOSCONVERTER_HPP_
#define ORG_OPENSPLICE_PUB_QOS_QOSCONVERTER_HPP_

#include <dds/core/types.hpp>
#include <dds/pub/qos/DataWriterQos.hpp>
#include <dds/pub/qos/PublisherQos.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
namespace qos
{
dds::pub::qos::DataWriterQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::DataWriterQos& from);

DDS::DataWriterQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::pub::qos::DataWriterQos& from);

dds::pub::qos::PublisherQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::PublisherQos& from);

DDS::PublisherQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::pub::qos::PublisherQos& from);
}
}
}
}

#endif /* ORG_OPENSPLICE_PUB_QOS_QOSCONVERTER_HPP_ */

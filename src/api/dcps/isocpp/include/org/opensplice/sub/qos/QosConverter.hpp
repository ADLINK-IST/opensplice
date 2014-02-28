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

#ifndef ORG_OPENSPLICE_SUB_QOS_QOSCONVERTER_HPP_
#define ORG_OPENSPLICE_SUB_QOS_QOSCONVERTER_HPP_

#include <dds/core/types.hpp>
#include <dds/sub/qos/DataReaderQos.hpp>
#include <dds/sub/qos/SubscriberQos.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{
namespace qos
{
dds::sub::qos::DataReaderQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::DataReaderQos& from);

DDS::DataReaderQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::sub::qos::DataReaderQos& from);

dds::sub::qos::SubscriberQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::SubscriberQos& from);

DDS::SubscriberQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::sub::qos::SubscriberQos& from);
}
}
}
}

#endif /* ORG_OPENSPLICE_SUB_QOS_QOSCONVERTER_HPP_ */

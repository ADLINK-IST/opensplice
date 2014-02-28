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

#ifndef ORG_OPENSPLICE_DOMAIN_QOS_QOSCONVERTER_HPP_
#define ORG_OPENSPLICE_DOMAIN_QOS_QOSCONVERTER_HPP_

#include <dds/core/types.hpp>
#include <dds/domain/qos/DomainParticipantQos.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{
namespace qos
{
dds::domain::qos::DomainParticipantQos
OSPL_ISOCPP_IMPL_API convertQos(const DDS::DomainParticipantQos& from);

DDS::DomainParticipantQos
OSPL_ISOCPP_IMPL_API convertQos(const dds::domain::qos::DomainParticipantQos& from);
}
}
}
}

#endif /* ORG_OPENSPLICE_DOMAIN_QOS_QOSCONVERTER_HPP_ */

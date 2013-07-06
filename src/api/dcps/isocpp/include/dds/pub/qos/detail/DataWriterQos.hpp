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
#ifndef OSPL_DDS_PUB_QOS_DETAIL_DATAWRITERQOS_HPP_
#define OSPL_DDS_PUB_QOS_DETAIL_DATAWRITERQOS_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/TEntityQos.hpp>
#include <org/opensplice/pub/qos/DataWriterQosImpl.hpp>

namespace dds
{
namespace pub
{
namespace qos
{
namespace detail
{
typedef dds::core::TEntityQos<org::opensplice::pub::qos::DataWriterQosImpl> DataWriterQos;
}
}
}
}

// End of implementation

#endif /* OSPL_DDS_PUB_QOS_DETAIL_DATAWRITERQOS_HPP_ */

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
#ifndef OSPL_DDS_CORE_DETAIL_INSTANCEHANDLE_HPP_
#define OSPL_DDS_CORE_DETAIL_INSTANCEHANDLE_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/TInstanceHandle.hpp>
#include <org/opensplice/core/InstanceHandleImpl.hpp>

namespace dds
{
namespace core
{
namespace detail
{
typedef dds::core::TInstanceHandle<org::opensplice::core::InstanceHandleImpl> InstanceHandle;
}
}
}

// End of implementation

#endif /* OSPL_DDS_CORE_DETAIL_INSTANCEHANDLE_HPP_ */

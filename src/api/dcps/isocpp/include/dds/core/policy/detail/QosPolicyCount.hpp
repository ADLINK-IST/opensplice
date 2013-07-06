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
#ifndef OSPL_DDS_CORE_POLICY_DETAIL_QOSPOLICYCOUNT_HPP_
#define OSPL_DDS_CORE_POLICY_DETAIL_QOSPOLICYCOUNT_HPP_

/**
 * @file
 */

// Implementation

#include <dds/core/policy/TQosPolicyCount.hpp>
#include <org/opensplice/core/policy/QosPolicyCountImpl.hpp>

namespace dds
{
namespace core
{
namespace policy
{
namespace detail
{
typedef dds::core::policy::TQosPolicyCount<org::opensplice::core::policy::QosPolicyCountImpl> QosPolicyCount;
}
}
}
}

// End of implementation

#endif /* OSPL_DDS_CORE_POLICY_DETAIL_QOSPOLICYCOUNT_HPP_ */

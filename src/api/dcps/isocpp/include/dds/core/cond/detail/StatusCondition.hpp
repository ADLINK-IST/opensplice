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
#ifndef OSPL_DDS_CORE_COND_DETAIL_STATUSCONDITION_HPP_
#define OSPL_DDS_CORE_COND_DETAIL_STATUSCONDITION_HPP_

/**
 * @file
 */

// Implementation


#include <dds/core/cond/TStatusCondition.hpp>
#include <org/opensplice/core/StatusConditionImpl.hpp>

namespace dds
{
namespace core
{
namespace cond
{
namespace detail
{
typedef dds::core::cond::TStatusCondition<org::opensplice::core::StatusConditionImpl> StatusCondition;
}
}
}
}

// End of implementation

#endif /* OSPL_DDS_CORE_COND_DETAIL_STATUSCONDITION_HPP_ */

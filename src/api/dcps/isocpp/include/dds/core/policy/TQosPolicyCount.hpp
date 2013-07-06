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
#ifndef OSPL_DDS_CORE_POLICY_TQOSPOLICYCOUNT_HPP_
#define OSPL_DDS_CORE_POLICY_TQOSPOLICYCOUNT_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/policy/TQosPolicyCount.hpp>

// Implementation

namespace dds
{
namespace core
{
namespace policy
{

template <typename D>
TQosPolicyCount<D>::TQosPolicyCount(QosPolicyId policy_id, int32_t count) : dds::core::Value<D>(policy_id, count) { }

template <typename D>
TQosPolicyCount<D>::TQosPolicyCount(const TQosPolicyCount& other) : dds::core::Value<D>(other.policy_id(), other.count()) { }

template <typename D> QosPolicyId TQosPolicyCount<D>::policy_id() const
{
    return this->delegate().policy_id();
}

template <typename D>
int32_t TQosPolicyCount<D>::count() const
{
    return this->delegate().count();
}

}
}
}

// End of implementation

#endif /* OSPL_DDS_CORE_POLICY_TQOSPOLICYCOUNT_HPP_ */

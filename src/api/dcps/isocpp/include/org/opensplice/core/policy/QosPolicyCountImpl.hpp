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

#ifndef ORG_OPENSPLICE_CORE_POLICY_QOS_POLICY_COUNT_HPP_
#define ORG_OPENSPLICE_CORE_POLICY_QOS_POLICY_COUNT_HPP_

#include <dds/core/types.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
namespace policy
{

class QosPolicyCountImpl
{
public:
    QosPolicyCountImpl(dds::core::policy::QosPolicyId id, uint32_t count)
        : policy_id_(id),
          count_(count)
    { }

    QosPolicyCountImpl(const QosPolicyCountImpl& other)
        : policy_id_(other.policy_id()),
          count_(other.count())
    { }

public:
    dds::core::policy::QosPolicyId policy_id() const
    {
        return policy_id_;
    }
    void policy_id(dds::core::policy::QosPolicyId id)
    {
        policy_id_ = id;
    }

    uint32_t count() const
    {
        return count_;
    }
    void count(uint32_t c)
    {
        count_ = c;
    }

    bool operator ==(const QosPolicyCountImpl& other) const
    {
        return other.policy_id_ == policy_id_ &&
               other.count_ == count_;
    }


private:
    dds::core::policy::QosPolicyId policy_id_;
    uint32_t count_;
};

}
}
}
}

#endif /* ORG_OPENSPLICE_CORE_POLICY_QOS_POLICY_COUNT_HPP_ */

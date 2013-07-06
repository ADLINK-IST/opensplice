#ifndef ORG_OPENSPLICE_CORE_CONDITION_IMPL_HPP_
#define ORG_OPENSPLICE_CORE_CONDITION_IMPL_HPP_
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

#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
class ConditionImpl;
}
}
}

class OSPL_ISOCPP_IMPL_API org::opensplice::core::ConditionImpl
{
public:
    virtual ~ConditionImpl();

    virtual void dispatch() = 0;

    bool get_trigger_value()
    {
        /** @internal @bug OSPL-918 DDS::Boolean is not (yet!) a bool
        @todo Remove fudge when OSPL-918 fixed
        @see http://jira.prismtech.com:8080/browse/OSPL-918 */
        return condition_->get_trigger_value() ? true : false;
    }

    inline DDS::Condition_ptr get_dds_condition() const
    {
        return condition_;
    }

protected:
    ConditionImpl();
    DDS::Condition_ptr condition_;
};


#endif /* ORG_OPENSPLICE_CORE_CONDITION_IMPL_HPP_ */

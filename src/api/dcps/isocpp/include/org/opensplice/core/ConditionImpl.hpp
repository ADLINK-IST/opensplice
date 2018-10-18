#ifndef ORG_OPENSPLICE_CORE_CONDITION_IMPL_HPP_
#define ORG_OPENSPLICE_CORE_CONDITION_IMPL_HPP_
/*
*                         Vortex OpenSplice
*
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
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

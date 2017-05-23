/*
*                         OpenSplice DDS
*
 *   This software and documentation are Copyright 2006 to TO_YEAR PrismTech
 *   Limited, its affiliated companies and licensors. All rights reserved.
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

#ifndef ORG_OPENSPLICE_SUB_COHERENT_ACCESS_IMPL_HPP_
#define ORG_OPENSPLICE_SUB_COHERENT_ACCESS_IMPL_HPP_

#include <org/opensplice/core/exception_helper.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{

class CoherentAccessImpl
{
public:
    CoherentAccessImpl() : sub(dds::core::null), ended(true) {}

    CoherentAccessImpl(const dds::sub::Subscriber sub) : sub(sub), ended(false)
    {
        DDS::ReturnCode_t result = sub->sub_->begin_access();
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::begin_access"));
    }

    void end()
    {
        if(!ended)
        {
            DDS::ReturnCode_t result = sub->sub_->end_access();
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::end_access"));
            ended = true;
        }
    }

    bool operator==(const CoherentAccessImpl& other) const
    {
        return sub == other.sub && ended == other.ended;
    }

    dds::sub::Subscriber sub;
    bool ended;
};

}
}
}

#endif /* ORG_OPENSPLICE_SUB_COHERENT_ACCESS_IMPL_HPP_ */

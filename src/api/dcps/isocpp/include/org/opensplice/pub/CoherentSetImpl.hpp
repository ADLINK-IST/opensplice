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

#ifndef ORG_OPENSPLICE_PUB_COHERENT_SET_IMPL_HPP_
#define ORG_OPENSPLICE_PUB_COHERENT_SET_IMPL_HPP_

#include <iostream>
#include <org/opensplice/core/exception_helper.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
class CoherentSetImpl;
}
}
}

class org::opensplice::pub::CoherentSetImpl
{
public:
	CoherentSetImpl() : pub(dds::core::null), ended(true) {}

    CoherentSetImpl(const dds::pub::Publisher& pub) : pub(pub), ended(false)
    {
        DDS::ReturnCode_t result = pub->pub_->begin_coherent_changes();
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::begin_coherent_changes"));
    }

    void end()
    {
        if(!ended)
        {
            DDS::ReturnCode_t result = pub->pub_->end_coherent_changes();
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::end_coherent_changes"));
            ended = true;
        }
    }

    ~CoherentSetImpl()
    {
        this->end();
    }

    bool operator ==(const CoherentSetImpl& other) const
    {
        return pub == other.pub && ended == other.ended;
    }

private:
    dds::pub::Publisher pub;
    bool ended;
};

#endif /* OMG_IDDS_PUB_COHERENT_SET_IMPL_HPP_ */

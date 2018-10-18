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

#ifndef ORG_OPENSPLICE_PUB_SUSPENDED_PUBBLICATIONS_IMPL_HPP_
#define ORG_OPENSPLICE_PUB_SUSPENDED_PUBBLICATIONS_IMPL_HPP_

#include <iostream>
#include <org/opensplice/core/exception_helper.hpp>

namespace org
{
namespace opensplice
{
namespace pub
{
class SuspendedPublicationImpl;
}
}
}

class org::opensplice::pub::SuspendedPublicationImpl
{
public:
	SuspendedPublicationImpl() : pub(dds::core::null), resumed(false) {}

    SuspendedPublicationImpl(const dds::pub::Publisher& pub) : pub(pub), resumed(false)
    {
        DDS::ReturnCode_t result = pub->pub_->suspend_publications();
        org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::suspend_publications"));
    }

    void resume()
    {
        if(!resumed)
        {
            DDS::ReturnCode_t result = pub->pub_->resume_publications();
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::resume_publications"));
        }
    }

    ~SuspendedPublicationImpl()
    {
        this->resume();
    }

    bool operator ==(const SuspendedPublicationImpl& other) const
    {
        return pub == other.pub && resumed == other.resumed;
    }

private:
    dds::pub::Publisher pub;
    bool resumed;
};

#endif /* ORG_OPENSPLICE_PUB_SUSPENDED_PUBBLICATIONS_IMPL_HPP_ */

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

#ifndef ORG_OPENSPLICE_SUB_COHERENT_ACCESS_DELEGATE_HPP_
#define ORG_OPENSPLICE_SUB_COHERENT_ACCESS_DELEGATE_HPP_

#include <dds/sub/Subscriber.hpp>

namespace org
{
namespace opensplice
{
namespace sub
{

class OMG_DDS_API CoherentAccessDelegate
{
public:
    CoherentAccessDelegate(const dds::sub::Subscriber sub);
    ~CoherentAccessDelegate();

    void end();

    bool operator==(const CoherentAccessDelegate& other) const;

    dds::sub::Subscriber get_subscriber() const {
        return this->sub;
    }

private:
    dds::sub::Subscriber sub;
    bool ended;
};

}
}
}

#endif /* ORG_OPENSPLICE_SUB_COHERENT_ACCESS_DELEGATE_HPP_ */

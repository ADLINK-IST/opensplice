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

#include <org/opensplice/sub/CoherentAccessDelegate.hpp>
#include <org/opensplice/sub/SubscriberDelegate.hpp>


namespace org
{
namespace opensplice
{
namespace sub
{

CoherentAccessDelegate::CoherentAccessDelegate(const dds::sub::Subscriber sub) : sub(sub), ended(false)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(sub);

    sub.delegate()->begin_coherent_access();
}

CoherentAccessDelegate::~CoherentAccessDelegate()
{
    /* The wrapper destructor already ended the set. Nothing to do here. */
}

void
CoherentAccessDelegate::end()
{
    if(!ended)
    {
        sub.delegate()->end_coherent_access();
        ended = true;
    }
}

bool
CoherentAccessDelegate::operator==(const CoherentAccessDelegate& other) const
{
    return sub == other.sub && ended == other.ended;
}

}
}
}

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

#include <org/opensplice/pub/SuspendedPublicationDelegate.hpp>
#include <org/opensplice/pub/PublisherDelegate.hpp>


namespace org
{
namespace opensplice
{
namespace pub
{

SuspendedPublicationDelegate::SuspendedPublicationDelegate(const dds::pub::Publisher& pub) : pub(pub), resumed(false)
{
    ISOCPP_REPORT_STACK_DDS_BEGIN(pub);

    pub.delegate()->suspend_publications();
}

SuspendedPublicationDelegate::~SuspendedPublicationDelegate()
{
    /* The wrapper destructor already resumed the publication. Nothing to do here. */
}

void SuspendedPublicationDelegate::resume()
{
    if(!resumed)
    {
        pub.delegate()->resume_publications();
        resumed = true;
    }
}

bool SuspendedPublicationDelegate::operator ==(const SuspendedPublicationDelegate& other) const
{
    return pub == other.pub && resumed == other.resumed;
}

}
}
}

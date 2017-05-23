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

#include <dds/pub/discovery.hpp>

namespace dds
{
    namespace pub
    {

        void ignore(const dds::domain::DomainParticipant& dp,
                    const dds::core::InstanceHandle& handle)
        {
            DDS::ReturnCode_t result = ((dds::domain::DomainParticipant)dp)->dp_->ignore_subscription(handle->handle());
            org::opensplice::core::check_and_throw(result, OSPL_CONTEXT_LITERAL("Calling ::ignore_subscription"));
        }

    }
}


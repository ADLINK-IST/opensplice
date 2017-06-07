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


#include <dds/sub/find.hpp>

namespace dds
{
namespace sub
{

const dds::sub::Subscriber builtin_subscriber(const dds::domain::DomainParticipant& dp)
{
    DDS::Subscriber_ptr key = dp->dp_->get_builtin_subscriber();
    dds::sub::Subscriber sub = dds::sub::Subscriber(dp);
	org::opensplice::core::EntityRegistry<DDS::Subscriber_ptr, dds::sub::Subscriber>::remove(sub->sub_.get());
	sub->init_builtin(key);

	org::opensplice::core::EntityRegistry<DDS::Subscriber_ptr, dds::sub::Subscriber>::insert(key, sub);

    return sub;
}

}
}

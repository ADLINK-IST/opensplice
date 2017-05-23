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

#include <sstream>

#include <dds/core/Exception.hpp>
#include <dds/core/InstanceHandle.hpp>

#include <org/opensplice/core/EntityDelegate.hpp>
#include <org/opensplice/core/exception_helper.hpp>


volatile unsigned int org::opensplice::core::EntityDelegate::entityID_ = 0;

org::opensplice::core::EntityDelegate::EntityDelegate()
    : enabled_(true)
{ }

org::opensplice::core::EntityDelegate::~EntityDelegate()
{ }

void
org::opensplice::core::EntityDelegate::enable()
{
    org::opensplice::core::check_and_throw(entity_.in()->enable(), OSPL_CONTEXT_LITERAL("Calling ::enable()"));
    enabled_ = true;
}

void
org::opensplice::core::EntityDelegate::close()
{

}

void
org::opensplice::core::EntityDelegate::retain()
{

}

const dds::core::status::StatusMask
org::opensplice::core::EntityDelegate::status_changes()
{
    return dds::core::status::StatusMask(entity_.in()->get_status_changes());
}

const dds::core::InstanceHandle
org::opensplice::core::EntityDelegate::instance_handle() const
{
    return dds::core::InstanceHandle(entity_.in()->get_instance_handle());
}

DDS::Entity_ptr org::opensplice::core::EntityDelegate::get_dds_entity()
{
    return entity_.in();
}

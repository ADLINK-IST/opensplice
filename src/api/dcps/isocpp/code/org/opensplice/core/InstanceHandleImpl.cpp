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

#include <org/opensplice/core/InstanceHandleImpl.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

InstanceHandleImpl::InstanceHandleImpl() : handle_(0)
{
    // empty
}

InstanceHandleImpl::InstanceHandleImpl(DDS::InstanceHandle_t h) : handle_(h) { }

InstanceHandleImpl::InstanceHandleImpl(const dds::core::null_type& src)
    : handle_(0)
{
    (void)src;
}

InstanceHandleImpl::~InstanceHandleImpl() { }

InstanceHandleImpl::InstanceHandleImpl(const InstanceHandleImpl& other)
    : handle_(other.handle_)
{ }

InstanceHandleImpl&
InstanceHandleImpl::operator=(const dds::core::null_type& src)
{
    handle_ = 0;
    return *this;
}

bool
InstanceHandleImpl::is_nil() const
{
    return (handle_ == 0);
}

bool
InstanceHandleImpl::operator==(const InstanceHandleImpl& that) const
{
    return (this->handle_ == that.handle_);
}

bool
InstanceHandleImpl::operator<(const InstanceHandleImpl& that) const
{
    return (this->handle_ < that.handle_);
}

bool
InstanceHandleImpl::operator>(const InstanceHandleImpl& that) const
{
    return (this->handle_ > that.handle_);
}

}
}
}

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

#include <org/opensplice/core/InstanceHandleDelegate.hpp>
#include <org/opensplice/core/ReportUtils.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

InstanceHandleDelegate::InstanceHandleDelegate() : handle_(U_INSTANCEHANDLE_NIL)
{
    // empty
}

InstanceHandleDelegate::InstanceHandleDelegate(u_instanceHandle h) : handle_(h)
{
}

InstanceHandleDelegate::InstanceHandleDelegate(v_handle vHandle)
{
    if (v_handleIsNil(vHandle)) {
        handle_ = U_INSTANCEHANDLE_NIL;
    } else {
        v_handleResult result;
        v_object instance;

        result = v_handleClaim(vHandle, &instance);
        if (result != V_HANDLE_OK) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not claim v_handle when converting to InstanceHandle");
        }
        handle_ = u_instanceHandleNew(v_public(instance));
        result = v_handleRelease(vHandle);
        if (result != V_HANDLE_OK) {
            ISOCPP_THROW_EXCEPTION(ISOCPP_ERROR, "Could not release v_handle when converting to InstanceHandle");
        }
    }
}

InstanceHandleDelegate::InstanceHandleDelegate(const dds::core::null_type& src)
    : handle_(U_INSTANCEHANDLE_NIL)
{
    (void)src;
}

InstanceHandleDelegate::~InstanceHandleDelegate() { }

InstanceHandleDelegate::InstanceHandleDelegate(const InstanceHandleDelegate& other)
    : handle_(other.handle_)
{
    this->handle_ = other.handle_;
}

u_instanceHandle
InstanceHandleDelegate::handle() const
{
    return handle_;
}


InstanceHandleDelegate&
InstanceHandleDelegate::operator=(const dds::core::null_type& src)
{
    handle_ = U_INSTANCEHANDLE_NIL;
    return *this;
}

bool
InstanceHandleDelegate::is_nil() const
{
    return (handle_ == U_INSTANCEHANDLE_NIL);
}

bool
InstanceHandleDelegate::operator==(const InstanceHandleDelegate& that) const
{
    return (this->handle_ == that.handle_);
}

bool
InstanceHandleDelegate::operator<(const InstanceHandleDelegate& that) const
{
    return (this->handle_ < that.handle_);
}

bool
InstanceHandleDelegate::operator>(const InstanceHandleDelegate& that) const
{
    return (this->handle_ > that.handle_);
}

}
}
}

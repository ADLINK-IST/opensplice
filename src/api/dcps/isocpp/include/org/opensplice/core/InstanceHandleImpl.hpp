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

#ifndef ORG_OPENSPLICE_CORE_INSTANCE_HANDLE_HPP_
#define ORG_OPENSPLICE_CORE_INSTANCE_HANDLE_HPP_

#include <dds/core/types.hpp>
#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace core
{
class InstanceHandleImpl;
}
}
}

class OSPL_ISOCPP_IMPL_API org::opensplice::core::InstanceHandleImpl
{
public:
    InstanceHandleImpl();
    InstanceHandleImpl(DDS::InstanceHandle_t handle);
    ~InstanceHandleImpl();
public:
    InstanceHandleImpl(const dds::core::null_type& src);
    InstanceHandleImpl(const InstanceHandleImpl& other);


public:
    bool operator==(const InstanceHandleImpl& that) const;

    bool operator<(const InstanceHandleImpl& that) const;

    bool operator>(const InstanceHandleImpl& that) const;

    InstanceHandleImpl& operator=(const dds::core::null_type& src);
    bool is_nil() const;

public:
    DDS::InstanceHandle_t handle() const
    {
        return handle_;
    }

private:
    DDS::InstanceHandle_t handle_;
};

inline std::ostream&
operator << (std::ostream& os,
             const org::opensplice::core::InstanceHandleImpl& h)
{
    os << h.handle();
    return os;
}
#endif /* ORG_OPENSPLICE_CORE_INSTANCE_HANDLE_HPP_ */

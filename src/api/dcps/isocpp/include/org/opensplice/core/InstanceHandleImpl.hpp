/*
*                         OpenSplice DDS
*
*   This software and documentation are Copyright 2006 to 2012 PrismTech
*   Limited and its licensees. All rights reserved. See file:
*
*                     $OSPL_HOME/LICENSE
*
*   for full copyright notice and license terms.
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

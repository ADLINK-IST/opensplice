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

#include <org/opensplice/core/InstanceHandleImpl.hpp>

namespace org
{
namespace opensplice
{
namespace core
{

InstanceHandleImpl::InstanceHandleImpl() : handle_(-1)
{
    // empty
}

InstanceHandleImpl::InstanceHandleImpl(DDS::InstanceHandle_t h) : handle_(h) { }

InstanceHandleImpl::InstanceHandleImpl(const dds::core::null_type& src)
    : handle_(-1)
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
    handle_ = -1;
    return *this;
}

bool
InstanceHandleImpl::is_nil() const
{
    return (handle_ == -1);
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

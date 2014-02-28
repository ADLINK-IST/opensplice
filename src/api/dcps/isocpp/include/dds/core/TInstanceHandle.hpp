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
#ifndef OSPL_DDS_CORE_TINSTANCEHANDLE_HPP_
#define OSPL_DDS_CORE_TINSTANCEHANDLE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/TInstanceHandle.hpp>

// Implementation
namespace dds
{
namespace core
{

template <typename DELEGATE>
TInstanceHandle<DELEGATE>::TInstanceHandle() { }

template <typename DELEGATE>
template <typename ARG0>
TInstanceHandle<DELEGATE>::TInstanceHandle(const ARG0& arg0) : dds::core::Value<DELEGATE>(arg0) { }

template <typename DELEGATE>
TInstanceHandle<DELEGATE>::TInstanceHandle(const dds::core::null_type& nullHandle) : dds::core::Value<DELEGATE>(nullHandle) { }

template <typename DELEGATE>
TInstanceHandle<DELEGATE>::TInstanceHandle(const TInstanceHandle& other): dds::core::Value<DELEGATE>(other.delegate()) { }

template <typename DELEGATE>
TInstanceHandle<DELEGATE>::~TInstanceHandle() { }

template <typename DELEGATE>
TInstanceHandle<DELEGATE>& TInstanceHandle<DELEGATE>::operator=(const TInstanceHandle& that)
{
    if(this != &that)
    {
        this->delegate() = that.delegate();
    }
    return *this;
}

template <typename DELEGATE>
bool TInstanceHandle<DELEGATE>::operator==(const TInstanceHandle& that) const
{
    return this->delegate() == that.delegate();
}

template <typename DELEGATE>
bool TInstanceHandle<DELEGATE>::operator<(const TInstanceHandle& that) const
{
    return this->delegate() < that.delegate();
}

template <typename DELEGATE>
bool TInstanceHandle<DELEGATE>::operator>(const TInstanceHandle& that) const
{
    return this->delegate() > that.delegate();
}

template <typename DELEGATE>
const TInstanceHandle<DELEGATE> TInstanceHandle<DELEGATE>::nil()
{
    dds::core::null_type nt;
    static TInstanceHandle nil_handle(nt);
    return nil_handle;
}

template <typename DELEGATE>
bool TInstanceHandle<DELEGATE>::is_nil() const
{
    return this->delegate().is_nil();
}
}
}
// End of implementation

#endif /* OSPL_DDS_CORE_TINSTANCEHANDLE_HPP_ */

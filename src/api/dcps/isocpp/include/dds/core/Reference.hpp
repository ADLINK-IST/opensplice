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
#ifndef OSPL_DDS_CORE_REFERENCE_HPP_
#define OSPL_DDS_CORE_REFERENCE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/Reference.hpp>

// Implementation
/** @internal @todo Commented OMG_DDS_LOG to remove?? **/
template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(dds::core::null_type&) : impl_()
{
    //OMG_DDS_LOG("MM", "Reference(dds::core::null_type&)");
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(const Reference& ref) : impl_(ref.delegate())
{
    //OMG_DDS_LOG("MM", "Reference(const Reference& ref)");
}

template <typename DELEGATE>
template <typename D>
dds::core::Reference<DELEGATE>::Reference(const Reference<D>& ref)
{
    OMG_DDS_STATIC_ASSERT((dds::core::is_base_of<DELEGATE_T,  D>::value));
    //OMG_DDS_LOG("MM", "Reference(const Reference<D>& ref)");
    impl_ = ref.delegate();
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(DELEGATE_T* p) : impl_(p)
{
    //OMG_DDS_LOG("MM", "Reference(DELEGATE_T* p)");
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::Reference(const DELEGATE_REF_T& p) : impl_(p)
{
    //OMG_DDS_LOG("MM", "Reference(DELEGATE_REF_T& p)");
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::~Reference()
{
    // OMG_DDS_LOG("MM", "~Reference()");
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::operator DELEGATE_REF_T() const
{
    return impl_;
}

template <typename DELEGATE>
template <typename R>
bool
dds::core::Reference<DELEGATE>::operator==(const R& ref) const
{
    return this->delegate() == ref.delegate();
}

template <typename DELEGATE>
template <typename R>
bool
dds::core::Reference<DELEGATE>::operator!=(const R& ref) const
{
    return this->delegate() != ref.delegate();
}

template <typename DELEGATE>
template <typename D>
dds::core::Reference<DELEGATE>&
dds::core::Reference<DELEGATE>::operator=(const Reference<D>& that)
{
    OMG_DDS_STATIC_ASSERT((dds::core::is_base_of<DELEGATE_T, D>::value));
    if(this != (Reference*)&that)
    {
        *this = Reference<DELEGATE_T>(that);
    }
    return *this;
}

template <typename DELEGATE>
template <typename R>
dds::core::Reference<DELEGATE>&
dds::core::Reference<DELEGATE>::operator=(const R& rhs)
{
    OMG_DDS_STATIC_ASSERT((dds::core::is_base_of< DELEGATE_T, typename R::DELEGATE_T>::value));
    if(this != (Reference*)&rhs)
    {
        *this = Reference<DELEGATE_T>(rhs);
    }
    return *this;
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>&
dds::core::Reference<DELEGATE>::operator=(const null_type)
{
    DELEGATE_REF_T tmp;
    impl_ = tmp;
    return *this;
}

template <typename DELEGATE>
bool
dds::core::Reference<DELEGATE>::is_nil() const
{
    return this->delegate().get() == 0;
}

template <typename DELEGATE>
bool
dds::core::Reference<DELEGATE>::operator==(const null_type) const
{
    return this->is_nil();
}

template <typename DELEGATE>
bool
dds::core::Reference<DELEGATE>::operator!=(const null_type) const
{
    return !(this->is_nil());
}

template <typename DELEGATE>
const typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T&
dds::core::Reference<DELEGATE>::delegate() const
{
    return impl_;
}

template <typename DELEGATE>
typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T&
dds::core::Reference<DELEGATE>::delegate()
{
    return impl_;
}

template <typename DELEGATE>
DELEGATE*
dds::core::Reference<DELEGATE>::operator->()
{
    return impl_.get();
}

template <typename DELEGATE>
const DELEGATE*
dds::core::Reference<DELEGATE>::operator->() const
{
    return impl_.get();
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::operator
const typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T& () const
{
    return impl_;
}

template <typename DELEGATE>
dds::core::Reference<DELEGATE>::operator
typename dds::core::Reference<DELEGATE>::DELEGATE_REF_T& ()
{
    return impl_;
}

template <class D> bool operator == (dds::core::null_type, const dds::core::Reference<D>& r)
{
    return r.is_nil();
}

template <class D> bool operator != (dds::core::null_type, const dds::core::Reference<D>& r)
{
    return !r.is_nil();
}

// End of implementation

#endif /* OSPL_DDS_CORE_REFERENCE_HPP_ */

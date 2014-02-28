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
#ifndef OSPL_DDS_SUB_LOANEDSAMPLES_HPP_
#define OSPL_DDS_SUB_LOANEDSAMPLES_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/LoanedSamples.hpp>

// Implementation

namespace dds
{
namespace sub
{

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T, DELEGATE>::LoanedSamples() : delegate_(new DELEGATE<T>()) { }

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T, DELEGATE>::~LoanedSamples() {  }

template <typename T, template <typename Q> class DELEGATE>
LoanedSamples<T, DELEGATE>::LoanedSamples(const LoanedSamples& other)
{
    delegate_ = other.delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
typename LoanedSamples<T, DELEGATE>::const_iterator LoanedSamples<T, DELEGATE>::begin() const
{
    return delegate()->begin();
}

template <typename T, template <typename Q> class DELEGATE>
typename LoanedSamples<T, DELEGATE>::const_iterator LoanedSamples<T, DELEGATE>::end() const
{
    return delegate()->end();
}

template <typename T, template <typename Q> class DELEGATE>
const typename LoanedSamples<T, DELEGATE>::DELEGATE_REF_T& LoanedSamples<T, DELEGATE>::delegate() const
{
    return delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
typename LoanedSamples<T, DELEGATE>::DELEGATE_REF_T& LoanedSamples<T, DELEGATE>::delegate()
{
    return delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
uint32_t LoanedSamples<T, DELEGATE>::length() const
{
    return delegate_->length();
}

}
}

// End of implementation

#endif /* OSPL_DDS_SUB_LOANEDSAMPLES_HPP_ */

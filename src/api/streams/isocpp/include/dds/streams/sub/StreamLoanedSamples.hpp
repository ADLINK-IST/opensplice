#ifndef DDS_STREAM_SUB_STREAMLOANEDSAMPLES_HPP_
#define DDS_STREAM_SUB_STREAMLOANEDSAMPLES_HPP_
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

#include <spec/dds/streams/sub/StreamLoanedSamples.hpp>

namespace dds
{
namespace streams
{
namespace sub
{

template <typename T, template <typename Q> class DELEGATE>
StreamLoanedSamples<T, DELEGATE>::StreamLoanedSamples() : delegate_(new DELEGATE<T>()) { }

template <typename T, template <typename Q> class DELEGATE>
StreamLoanedSamples<T, DELEGATE>::~StreamLoanedSamples() {  }

template <typename T, template <typename Q> class DELEGATE>
StreamLoanedSamples<T, DELEGATE>::StreamLoanedSamples(const StreamLoanedSamples& other)
{
    delegate_ = other.delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
typename StreamLoanedSamples<T, DELEGATE>::const_iterator StreamLoanedSamples<T, DELEGATE>::begin() const
{
    return delegate()->begin();
}

template <typename T, template <typename Q> class DELEGATE>
typename StreamLoanedSamples<T, DELEGATE>::const_iterator StreamLoanedSamples<T, DELEGATE>::end() const
{
    return delegate()->end();
}

template <typename T, template <typename Q> class DELEGATE>
const typename StreamLoanedSamples<T, DELEGATE>::DELEGATE_REF_T& StreamLoanedSamples<T, DELEGATE>::delegate() const
{
    return delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
typename StreamLoanedSamples<T, DELEGATE>::DELEGATE_REF_T& StreamLoanedSamples<T, DELEGATE>::delegate()
{
    return delegate_;
}

template <typename T, template <typename Q> class DELEGATE>
uint32_t StreamLoanedSamples<T, DELEGATE>::length() const
{
    return delegate_->length();
}

template <typename T, template <typename Q> class DELEGATE>
uint32_t StreamLoanedSamples<T, DELEGATE>::stream() const
{
    return delegate_->stream();
}

}
}
}

#endif /* DDS_STREAM_SUB_STREAMLOANEDSAMPLES_HPP_ */

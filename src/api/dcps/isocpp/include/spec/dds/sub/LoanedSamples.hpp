/* Copyright 2010, Object Management Group, Inc.
* Copyright 2010, PrismTech, Corp.
* Copyright 2010, Real-Time Innovations, Inc.
* All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef OMG_DDS_SUB_TLOANED_SAMPLES_HPP_
#define OMG_DDS_SUB_TLOANED_SAMPLES_HPP_

#include <dds/core/ref_traits.hpp>
#include <dds/sub/Sample.hpp>

#include <dds/sub/detail/LoanedSamples.hpp>

namespace dds
{
namespace sub
{
template <typename T,
          template <typename Q> class DELEGATE = dds::sub::detail::LoanedSamples>
class LoanedSamples;

// Used by C++11 compilers to allow for using LoanedSamples
// and SharedSamples in a range-based for-loop.
template <typename T> typename T::const_iterator cbegin(const T& t);
template <typename T> typename T::const_iterator cend(const T& t);
}

}

/**
 * This class encapsulates and automates the management of loaned samples.
 *
 * It is a container which is used to hold samples which have been read
 * or taken by the DataReader. Samples are effectively "loaned" from the
 * DataReader to avoid the need to copy the data. When the LoanedSamples
 * container goes out of scope the loan is automatically returned.
 *
 * LoanedSamples maintains a ref count so that the loan will only be
 * returned once all copies of the same LoanedSamples have been destroyed.
 */
template <typename T,
          template <typename Q> class DELEGATE>
class dds::sub::LoanedSamples
{
public:
    typedef T                     DataType;
    typedef typename DELEGATE<T>::const_iterator        const_iterator;

    typedef typename dds::core::smart_ptr_traits< DELEGATE<T> >::ref_type DELEGATE_REF_T;

public:
    /**
     * Constructs a LoanedSamples instance.
     */
    LoanedSamples();

    /**
     * Implicitly return the loan.
     */
    ~LoanedSamples();

    /**
     * Copies a LoanedSamples instance.
     */
    LoanedSamples(const LoanedSamples& other);


public:
    /**
     * Gets an iterator pointing to the first sample in the container.
     *
     * @return an iterator pointing to the first sample
     */
    const_iterator begin() const;

    /**
     * Gets an iterator pointing to the end of the container.
     *
     * @return an iterator pointing to the end of the container
     */
    const_iterator end() const;

    /**
     * Gets the delegate.
     *
     * @return the delegate
     */
    const DELEGATE_REF_T& delegate() const;

    /**
     * Gets the delegate.
     *
     * @return the delegate
     */
    DELEGATE_REF_T& delegate();

    /**
     * Gets the number of samples.
     *
     * @return the number of samples
     */
    uint32_t length() const;

private:
    DELEGATE_REF_T delegate_;
};

namespace dds
{
namespace sub
{
template <typename T, template <typename Q> class D>
LoanedSamples<T, D >
move(LoanedSamples<T, D >& a);
}
}
#endif /* OMG_DDS_SUB_TLOANED_SAMPLES_HPP_ */

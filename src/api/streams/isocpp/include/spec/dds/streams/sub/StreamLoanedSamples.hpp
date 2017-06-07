#ifndef SPEC_DDS_STREAMS_SUB_STREAMLOANEDSAMPLES_HPP_
#define SPEC_DDS_STREAMS_SUB_STREAMLOANEDSAMPLES_HPP_
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

#include <dds/core/ref_traits.hpp>
#include <dds/streams/sub/StreamSample.hpp>

#include <dds/streams/sub/detail/StreamLoanedSamples.hpp>

namespace dds
{
namespace streams
{
namespace sub
{
    template <typename T,
    template <typename Q> class DELEGATE = dds::streams::sub::detail::StreamLoanedSamples>
    class StreamLoanedSamples;

    // Used by C++11 compilers to allow for using StreamLoanedSamples
    // and SharedSamples in a range-based for-loop.
    template <typename T> typename T::const_iterator cbegin(const T& t);
    template <typename T> typename T::const_iterator cend(const T& t);
}
}
}

/**
 * This class encapsulates and automates the management of loaned samples.
 *
 * It is a container which is used to hold samples which have been read
 * or taken by the StreamDataReader. Samples are effectively "loaned" from the
 * DataReader to avoid the need to copy the data. When the StreamLoanedSamples
 * container goes out of scope the loan is automatically returned.
 *
 * StreamLoanedSamples maintains a ref count so that the loan will only be
 * returned once all copies of the same StreamLoanedSamples have been destroyed.
 */
template <typename T,
template <typename Q> class DELEGATE>
class dds::streams::sub::StreamLoanedSamples
{
public:
    typedef T                     DataType;
    typedef typename DELEGATE<T>::const_iterator        const_iterator;

    typedef typename dds::core::smart_ptr_traits< DELEGATE<T> >::ref_type DELEGATE_REF_T;

public:
    /**
     * Constructs a StreamLoanedSamples instance.
     *
     * As loaned samples are not currently fully supported, this constructor
     * must be used in conjunction with the datareader get function.
     */
    StreamLoanedSamples();

    /**
     * Implicitly return the loan.
     */
    ~StreamLoanedSamples();

    /**
     * Copies a StreamLoanedSamples instance.
     */
    StreamLoanedSamples(const StreamLoanedSamples& other);


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

    /**
     * Gets the id of the stream the samples were taken from.
     *
     * @return the stream id
     */
    uint32_t stream() const;

private:
    DELEGATE_REF_T delegate_;
};

namespace dds
{
namespace streams
{
namespace sub
{
template <typename T, template <typename Q> class D>
StreamLoanedSamples<T, D >
move(StreamLoanedSamples<T, D >& a);
}
}
}

#endif /* SPEC_DDS_STREAMS_SUB_STREAMLOANEDSAMPLES_HPP_ */

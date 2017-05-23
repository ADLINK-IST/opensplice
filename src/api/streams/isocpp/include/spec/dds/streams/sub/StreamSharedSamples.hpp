#ifndef SPEC_DDS_STREAMS_SUB_STREAMS_SHARED_SAMPLES_HPP_
#define SPEC_DDS_STREAMS_SUB_STREAMS_SHARED_SAMPLES_HPP_
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

#include <dds/core/Reference.hpp>
#include <dds/streams/sub/StreamSample.hpp>
#include <dds/streams/sub/StreamLoanedSamples.hpp>
#include <dds/streams/sub/detail/StreamSharedSamples.hpp>

namespace dds
{
namespace streams
{
namespace sub
{
template <typename T,
template <typename Q> class DELEGATE = detail::StreamSharedSamples>
class StreamSharedSamples;
}
}
}

/**
 * The class StreamSharedSamples is used as a container safe
 * and sharable version of the StreamLoanedSamples class.
 *
 *
 */
template <typename T,
template <typename Q> class DELEGATE>
class dds::streams::sub::StreamSharedSamples
{
public:
    typedef T                     DataType;
    typedef typename DELEGATE<T>::const_iterator        const_iterator;

    typedef typename dds::core::smart_ptr_traits< DELEGATE<T> >::ref_type DELEGATE_REF_T;

public:
    /**
     * Constructs a StreamSharedSamples instance.
     */
    StreamSharedSamples();

    /**
     * Constructs an instance of StreamSharedSamples and
     * removes the ownership of the loan from the StreamLoanedSamples.
     * As a result the destruction of the StreamLoanedSamples object
     * will have no effect on loaned data. Loaned data will be returned
     * automatically once the delegate of this reference type will have a
     * zero reference count.
     *
     * @param ls the loaned samples
     *
     */
    StreamSharedSamples(dds::streams::sub::StreamLoanedSamples<T> ls);

    /**
     * Copies a StreamSharedSamples instance.
     */
    StreamSharedSamples(const StreamSharedSamples& other);

    ~StreamSharedSamples();

public:
    /**
     * Gets an iterator pointing to the beginning of the samples.
     *
     * @return an iterator pointing to the beginning of the samples
     */
    const_iterator begin() const;

    /**
     * Gets an iterator pointing to the beginning of the samples.
     *
     * @return an iterator pointing to the beginning of the samples
     */
    const_iterator  end() const;

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
     * Returns the number of samples.
     *
     * @return the number of samples
     */
    uint32_t length() const;

private:
    DELEGATE_REF_T delegate_;
};

#endif /* SPEC_DDS_STREAMS_SUB_STREAMS_SHARED_SAMPLES_HPP_ */

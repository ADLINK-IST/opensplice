#ifndef SPEC_DDS_STREAMS_SUB_TSTREAMSAMPLE_HPP_
#define SPEC_DDS_STREAMS_SUB_TSTREAMSAMPLE_HPP_
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

#include <dds/core/Value.hpp>

namespace dds
{
namespace streams
{
namespace sub
{
template <typename T, template <typename Q> class DELEGATE>
class StreamSample;
}
}
}

/**
 * This class encapsulates the data associated with stream samples.
 */
template <typename T, template <typename Q> class DELEGATE>
class dds::streams::sub::StreamSample : public dds::core::Value< DELEGATE<T> >
{
public:
    typedef T DataType;

public:
    /**
     * Create a stream sample with invalid data.
     */
    StreamSample();

    /**
     * Creates a StreamSample instance.
     *
     * @param data the data
     */
    StreamSample(const T& data);

    /**
     * Copies a stream sample instance.
     *
     * @param other the stream sample instance to copy
     */
    StreamSample(const StreamSample& other);

    /**
     * Gets the data.
     *
     * @return the data
     */
    const DataType& data() const;

    /**
     * Sets the data.
     *
     * @param data the data
     */
    void data(const DataType& data);
};

#endif /* SPEC_DDS_STREAMS_SUB_TSTREAMSAMPLE_HPP_ */

#ifndef DDS_STREAMS_SUB_TSTREAMSAMPLE_HPP_
#define DDS_STREAMS_SUB_TSTREAMSAMPLE_HPP_
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

#include <spec/dds/streams/sub/TStreamSample.hpp>

namespace dds
{
namespace streams
{
namespace sub
{

template <typename T, template <typename Q> class DELEGATE>
StreamSample<T, DELEGATE>::StreamSample() : dds::core::Value< DELEGATE<T> >() {}

template <typename T, template <typename Q> class DELEGATE>
StreamSample<T, DELEGATE>::StreamSample(const T& data) : dds::core::Value< DELEGATE<T> >(data) { }

template <typename T, template <typename Q> class DELEGATE>
StreamSample<T, DELEGATE>::StreamSample(const StreamSample& other) : dds::core::Value< DELEGATE<T> >(other.delegate()) { }

template <typename T, template <typename Q> class DELEGATE>
const typename StreamSample<T, DELEGATE>::DataType& StreamSample<T, DELEGATE>::data() const
{
    return this->delegate().data();
}

template <typename T, template <typename Q> class DELEGATE>
void StreamSample<T, DELEGATE>::data(const DataType& d)
{
    this->delegate().data(d);
}

}
}
}

#endif /* DDS_STREAMS_SUB_TSTREAMSAMPLE_HPP_ */

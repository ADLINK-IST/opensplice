#ifndef SPEC_DDS_STREAMS_SUB_STREAMDATAREADER_HPP_
#define SPEC_DDS_STREAMS_SUB_STREAMDATAREADER_HPP_
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

#include <dds/streams/sub/detail/StreamDataReader.hpp>

namespace dds
{
namespace streams
{
namespace sub
{
template < typename T, template <typename Q> class DELEGATE = dds::streams::sub::detail::StreamDataReader >
class StreamDataReader;
}
}
}

#include <dds/streams/sub/TStreamDataReader.hpp>

// = Manipulators
namespace dds
{
namespace streams
{
namespace sub
{
namespace functors
{
typedef dds::streams::sub::functors::detail::MaxSamplesManipulatorFunctor MaxSamplesManipulatorFunctor;
typedef dds::streams::sub::functors::detail::TimeoutManipulatorFunctor TimeoutManipulatorFunctor;
template <typename T>
class FilterManipulatorFunctor : dds::streams::sub::functors::detail::FilterManipulatorFunctor<T> {};
}
}
}
}

namespace dds
{
namespace streams
{
namespace sub
{

template <typename SELECTOR>
SELECTOR& get(SELECTOR& selector);

inline dds::streams::sub::functors::MaxSamplesManipulatorFunctor
max_samples(uint32_t max_samples);

inline dds::streams::sub::functors::TimeoutManipulatorFunctor
timeout(const dds::core::Duration& timeout);

template <typename T>
inline dds::streams::sub::functors::FilterManipulatorFunctor<T>
filter(bool (*filter_func)(T));

}
}
}

#endif /* SPEC_DDS_STREAMS_SUB_STREAMDATAREADER_HPP_ */

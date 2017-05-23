#ifndef DDS_STREAMS_SUB_DATAREADER_HPP_
#define DDS_STREAMS_SUB_DATAREADER_HPP_
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

#include <spec/dds/streams/sub/StreamDataReader.hpp>

namespace dds
{
namespace streams
{
namespace sub
{

template <typename SELECTOR>
SELECTOR& get(SELECTOR& selector)
{
    selector.get();
    return selector;
}

inline dds::streams::sub::functors::MaxSamplesManipulatorFunctor
max_samples(uint32_t max_samples)
{
    return dds::streams::sub::functors::MaxSamplesManipulatorFunctor(max_samples);
}

inline dds::streams::sub::functors::TimeoutManipulatorFunctor
timeout(const dds::core::Duration& timeout)
{
    return dds::streams::sub::functors::TimeoutManipulatorFunctor(timeout);
}

template <typename T>
inline dds::streams::sub::functors::FilterManipulatorFunctor<T>
filter(bool (*filter_func)(T))
{
    return dds::streams::sub::functors::FilterManipulatorFunctor<T>(filter_func);
}

}
}
}

#endif /* DDS_STREAMS_SUB_DATAREADER_HPP_ */

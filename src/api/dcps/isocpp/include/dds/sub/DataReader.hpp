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
#ifndef OSPL_DDS_SUB_DATAREADER_HPP_
#define OSPL_DDS_SUB_DATAREADER_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/DataReader.hpp>

// Implementation

namespace dds
{
namespace sub
{

template <typename SELECTOR>
SELECTOR& read(SELECTOR& selector)
{
    selector.read_mode(true);
    return selector;
}

template <typename SELECTOR>
SELECTOR& take(SELECTOR& selector)
{
    selector.read_mode(false);
    return selector;
}

inline dds::sub::functors::ContentFilterManipulatorFunctor
content(const dds::sub::Query& query)
{
    return dds::sub::functors::ContentFilterManipulatorFunctor(query);
}


inline dds::sub::functors::StateFilterManipulatorFunctor
state(const dds::sub::status::DataState& s)
{
    return dds::sub::functors::StateFilterManipulatorFunctor(s);
}

inline dds::sub::functors::InstanceManipulatorFunctor
instance(const dds::core::InstanceHandle& h)
{
    return dds::sub::functors::InstanceManipulatorFunctor(h);
}

inline dds::sub::functors::NextInstanceManipulatorFunctor
next_instance(const dds::core::InstanceHandle& h)
{
    return dds::sub::functors::NextInstanceManipulatorFunctor(h);
}

}
}

// End of implementation

#endif /* OSPL_DDS_SUB_DATAREADER_HPP_ */

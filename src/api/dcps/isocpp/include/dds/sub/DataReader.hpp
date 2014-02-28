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

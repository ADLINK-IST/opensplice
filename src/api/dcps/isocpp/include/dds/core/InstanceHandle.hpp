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
#ifndef OSPL_DDS_CORE_INSTANCEHANDLE_HPP_
#define OSPL_DDS_CORE_INSTANCEHANDLE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/core/InstanceHandle.hpp>

// Implementation

inline std::ostream& operator << (std::ostream& os, const dds::core::InstanceHandle& h)
{
    os << h.delegate();
    return os;
}

// End of implementation

#endif /* OSPL_DDS_CORE_INSTANCEHANDLE_HPP_ */

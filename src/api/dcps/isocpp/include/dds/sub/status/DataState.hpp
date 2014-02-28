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
#ifndef OSPL_DDS_SUB_STATUS_DATASTATE_HPP_
#define OSPL_DDS_SUB_STATUS_DATASTATE_HPP_

/**
 * @file
 */

/*
 * OMG PSM class declaration
 */
#include <spec/dds/sub/status/DataState.hpp>

// Implementation

namespace dds
{
namespace sub
{
namespace status
{

inline const SampleState SampleState::read()
{
    return SampleState(0x0001 << 0u);
}

inline const SampleState SampleState::not_read()
{
    return SampleState(0x0001 << 1u);
}

inline const SampleState SampleState::any()
{
    return SampleState(~0u);
}

inline const ViewState ViewState::new_view()
{
    return ViewState(0x0001 << 0u);
}

inline const ViewState ViewState::not_new_view()
{
    return ViewState(0x0001 << 1u);
}

inline const ViewState ViewState::any()
{
    return ViewState(~0u);
}

inline const InstanceState InstanceState::alive()
{
    return InstanceState(0x0001 << 0u);
}

inline const InstanceState InstanceState::not_alive_disposed()
{
    return InstanceState(0x0001 << 1u);
}

inline const InstanceState InstanceState::not_alive_no_writers()
{
    return InstanceState(0x0001 << 2u);
}

inline const InstanceState InstanceState::not_alive_mask()
{
    return not_alive_disposed() | not_alive_no_writers();
}

inline const InstanceState InstanceState::any()
{
    return InstanceState(~0u);
}

} /* namespace status */
} /* namespace sub */
} /* namespace dds */

// End of implementation

#endif /* OSPL_DDS_SUB_STATUS_DATASTATE_HPP_ */

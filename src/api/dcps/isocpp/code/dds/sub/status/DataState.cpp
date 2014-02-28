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


/**
 * @file
 */

#ifndef OSPL_DDS_SUB_STATUS_DATASTATE_CPP_
#define OSPL_DDS_SUB_STATUS_DATASTATE_CPP_

#include <dds/sub/status/DataState.hpp>

// Implementation

namespace dds
{
namespace sub
{
namespace status
{

SampleState::SampleState() : MaskType() { }
SampleState::SampleState(uint32_t i) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
     * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (i)) { }
SampleState::SampleState(const SampleState& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
     * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }
SampleState::SampleState(const MaskType& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }

ViewState::ViewState() : MaskType() { }
ViewState::ViewState(uint32_t m) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (m)) { }
ViewState::ViewState(const ViewState& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }
ViewState::ViewState(const MaskType& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }

InstanceState::InstanceState(uint32_t m) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (m)) { }
InstanceState::InstanceState() : MaskType() { }
InstanceState::InstanceState(const InstanceState& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }
InstanceState::InstanceState(const MaskType& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }

} /* namespace status */
} /* namespace sub */
} /* namespace dds */

// End of implementation

#endif /* OSPL_DDS_SUB_STATUS_DATASTATE_HPP_ */

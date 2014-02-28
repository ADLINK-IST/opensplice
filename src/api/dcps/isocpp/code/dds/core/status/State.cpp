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

#ifndef OMG_DDS_CORE_STATUS_STATE_CPP_
#define OMG_DDS_CORE_STATUS_STATE_CPP_

#include <dds/core/status/State.hpp>

namespace dds
{
namespace core
{
namespace status
{

SampleRejectedState::SampleRejectedState() : MaskType() { }

SampleRejectedState::SampleRejectedState(const SampleRejectedState& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }

SampleRejectedState::SampleRejectedState(const MaskType& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }

SampleRejectedState::SampleRejectedState(uint32_t s)
    : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (s))
{ }

StatusMask::StatusMask() { }

StatusMask::StatusMask(uint32_t mask) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (mask)) { }

StatusMask::StatusMask(const StatusMask& other) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (other.to_ulong())) { }

StatusMask::~StatusMask() { }


}
}
} /* namespace dds / core / status*/


#endif /* OMG_DDS_CORE_STATUS_STATE_HPP_ */

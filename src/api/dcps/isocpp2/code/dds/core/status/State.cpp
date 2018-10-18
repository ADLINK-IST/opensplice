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
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC, using int in Windows.
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }

SampleRejectedState::SampleRejectedState(const MaskType& src) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC, using int in Windows.
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (src.to_ulong())) { }

SampleRejectedState::SampleRejectedState(uint32_t s)
    : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC, using int in Windows.
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (s))
{ }

StatusMask::StatusMask() { }

StatusMask::StatusMask(uint32_t mask) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC, using int in Windows.
    * @see http://connect.microsoft.com/VisualStudio/feedback/details/532897 */
#if (defined OSPL_USE_CXX11 && _MSC_VER == 1600)
        static_cast<int>
#endif
        (mask)) { }

StatusMask::StatusMask(const StatusMask& other) : MaskType(
    /** @internal @note MSVC bug: Problems constructing a bitset from an unsigned long in the VC RC, using int in Windows.
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

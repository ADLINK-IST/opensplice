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

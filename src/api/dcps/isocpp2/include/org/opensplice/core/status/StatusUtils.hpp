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

#ifndef ORG_OPENSPLICE_CORE_STATUS_UTILS_HPP_
#define ORG_OPENSPLICE_CORE_STATUS_UTILS_HPP_

#include <dds/core/status/State.hpp>
#include <org/opensplice/core/config.hpp>

#include "u_user.h"

namespace org
{
namespace opensplice
{
namespace core
{
namespace utils
{

/*
 * Status conversions
 */
dds::core::status::StatusMask
vEventMaskToStatusMask (
    const v_eventMask vMask,
    const v_kind      vKind);

v_eventMask
vEventMaskFromStatusMask (
    const dds::core::status::StatusMask& mask);

}
}
}
}

#endif /* ORG_OPENSPLICE_CORE_STATUS_UTILS_HPP_ */

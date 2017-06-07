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
 *
 * Unfortunately, we need this because we can not change the Duration.hpp and Time.hpp
 * and we don't want this piece of code repeated in every function within their
 * implementation files.
 */

#ifndef ORG_OPENSPLICE_CORE_TIME_UTILS_HPP_
#define ORG_OPENSPLICE_CORE_TIME_UTILS_HPP_

#include <dds/core/Time.hpp>
#include <dds/core/Duration.hpp>
#include <org/opensplice/core/ReportUtils.hpp>



namespace org
{
namespace opensplice
{
namespace core
{
namespace timeUtils
{

os_timeW
convertTime(
    const dds::core::Time& time,
    os_int64 maxSupportedSeconds);

os_duration
convertDuration(
    const dds::core::Duration& duration);

}
}
}
}

#endif /* ORG_OPENSPLICE_CORE_TIME_HELPER_HPP_ */

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

#ifndef ORG_OPENSPLICE_DOMAIN_DOMAIN_HPP_
#define ORG_OPENSPLICE_DOMAIN_DOMAIN_HPP_

#include <org/opensplice/core/config.hpp>

namespace org
{
namespace opensplice
{
namespace domain
{
inline uint32_t any_id()
{
    return 0;
}

inline uint32_t default_id()
{
    return DDS::DOMAIN_ID_DEFAULT;
}

OSPL_ISOCPP_IMPL_API void configure(uint32_t, int, char*[]);
}
}
}

#endif /* ORG_OPENSPLICE_DOMAIN_DOMAIN_HPP_ */

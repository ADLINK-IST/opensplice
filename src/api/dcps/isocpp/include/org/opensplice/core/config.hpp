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

#ifndef ORG_OPENSPLICE_CORE_CONFIG_HPP_
#define ORG_OPENSPLICE_CORE_CONFIG_HPP_

#ifndef OPENSPLICE_ISOCXX_PSM
#define OPENSPLICE_ISOCXX_PSM
#endif

/* Includes necessary for pulling in legacy C++ API */
#include <ccpp_dds_dcps.h>

#include <dds/core/macros.hpp>
#include <dds/core/types.hpp>

/** @internal Using a separate macro for org::opensplice in case we want to separate libs
later */
#define OSPL_ISOCPP_IMPL_API OMG_DDS_API

#endif /* ORG_OPENSPLICE_CORE_CONFIG_HPP_ */

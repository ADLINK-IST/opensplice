#ifndef OMG_DDS_CORE_DETAIL_MACROS_HPP_
#define OMG_DDS_CORE_DETAIL_MACROS_HPP_

/* Copyright 2010, Object Management Group, Inc.
 * Copyright 2010, PrismTech, Corp.
 * Copyright 2010, Real-Time Innovations, Inc.
 * All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <boost/static_assert.hpp>
#include <string.h>

// == Constants
#define OMG_DDS_DEFAULT_STATE_BIT_COUNT_DETAIL (size_t)16
#define OMG_DDS_DEFAULT_STATUS_COUNT_DETAIL    (size_t)16
// ==========================================================================

// == Static Assert
#define OMG_DDS_STATIC_ASSERT_DETAIL BOOST_STATIC_ASSERT
// ==========================================================================

// Logging Macros
#define OMG_DDS_LOG_DETAIL(kind, msg) \
    std::cout << "[" << kind << "]: " << msg << std::endl;
// ==========================================================================


// DLL Export Macros

#ifdef _WIN32 // This is defined for 32/64 bit Windows
#  define OMG_DDS_API_DETAIL __declspec(dllexport)
#else
#  define OMG_DDS_API_DETAIL
#endif

// ==========================================================================



#endif /* OMG_DDS_CORE_MACROS_HPP_*/

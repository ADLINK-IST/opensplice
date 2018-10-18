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
#ifndef VORTEX_FACE_MACROS_HPP_
#define VORTEX_FACE_MACROS_HPP_

#include "os_if.h"
#include "dds/core/ddscore.hpp"

#ifdef OS_API
#undef OS_API
#endif

#ifdef BUILD_VORTEX_FACE_API
#define VORTEX_FACE_API OS_API_EXPORT
#else
#define VORTEX_FACE_API OS_API_IMPORT
#endif

namespace Vortex {
namespace FACE {

template <typename T>
struct smart_ptr_traits
{
    /* Re-use isocpp2 to get shared pointers. */
    typedef OSPL_CXX11_STD_MODULE::shared_ptr<T> shared_ptr;
    typedef OSPL_CXX11_STD_MODULE::weak_ptr<T>   weak_ptr;
};

};
};

#endif /* VORTEX_FACE_MACROS_HPP_ */

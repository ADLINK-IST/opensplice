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
#ifndef _FACE_COMMON_H_
#define _FACE_COMMON_H_

#include <dds/core/ddscore.hpp>

#ifdef NO_ERROR
/* This windows socket NO_ERROR return value can
 * clash with the FACE return code enum. */
#undef NO_ERROR
#endif

namespace FACE
{
    typedef int64_t SYSTEM_TIME_TYPE;

    const ::FACE::SYSTEM_TIME_TYPE INF_TIME_VALUE = (-1LL);

    typedef std::string CONFIGURATION_RESOURCE;

    enum RETURN_CODE_TYPE {
        NO_ERROR,
        NO_ACTION,
        NOT_AVAILABLE,
        ADDR_IN_USE,
        INVALID_PARAM,
        INVALID_CONFIG,
        PERMISSION_DENIED,
        INVALID_MODE,
        TIMED_OUT,
        MESSAGE_STALE,
        CONNECTION_IN_PROGRESS,
        CONNECTION_CLOSED,
        DATA_BUFFER_TOO_SMALL
    };

    typedef std::string SYSTEM_ADDRESS_TYPE;

    typedef ::FACE::SYSTEM_TIME_TYPE TIMEOUT_TYPE;

    typedef int32_t MESSAGE_RANGE_TYPE;

}

#endif /* _FACE_COMMON_H_ */

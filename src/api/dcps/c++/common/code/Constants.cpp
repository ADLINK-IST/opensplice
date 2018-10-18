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
#include "Constants.h"
#include "os_if.h"

namespace DDS
{
    const ::DDS::Duration_t DURATION_ZERO         = {0L,0U};
    const ::DDS::Duration_t DURATION_INFINITE     = {DURATION_INFINITE_SEC, DURATION_INFINITE_NSEC};

    const ::DDS::Time_t TIMESTAMP_INVALID         = {TIMESTAMP_INVALID_SEC, TIMESTAMP_INVALID_NSEC};
    const ::DDS::Time_t TIMESTAMP_CURRENT         = {TIMESTAMP_INVALID_SEC, TIMESTAMP_INVALID_NSEC-1};

    // Note: ANY_STATUS is deprecated, please use spec version specific constants.
    const ::DDS::StatusKind ANY_STATUS            = 0x7FFF;
    // STATUS_MASK_ANY_V1_2 is all standardised status bits as of V1.2 of the
    // specification.
    const ::DDS::StatusKind STATUS_MASK_ANY_V1_2  = 0x7FFF;
    const ::DDS::StatusKind STATUS_MASK_ANY       = 0xFFFFFFFF;
    const ::DDS::StatusKind STATUS_MASK_NONE      = 0x0;
}

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
#pragma once

#if defined(LITE) && !defined(OS__SYNC_NO_SHARED)
# define OS__SYNC_NO_SHARED 1
#endif

#include "os_sync.h"

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct os_os_mutex_s {
    os_syncLock lock;
    os_scopeAttr scope;
#ifndef NDEBUG
    UINT32 signature; /* Set to OS__MUTEX_SIG if initialised */
#endif
} os_os_mutex;

#if defined (__cplusplus)
}
#endif

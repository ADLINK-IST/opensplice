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

/**
 * This file exposes two external structs with the right size for the os__sync implementation,
 * without including Windows.h. This file can thus be included in public header files. Assumptions
 * on the size are asserted in the sync-API implementation at compile-time.
 *
 * External structs:
 * - os_syncLock
 * - os_syncCond
 *
 * Build-flags:
 * OS__SYNC_NOSHARED:
 *                    The sync API by default implements process shared (shared memory) mutexes and
 *                    conditions. If these are not needed one can specify -DOS__SYNC_NOSHARED. The
 *                    initialization structs will then be only the size needed for process private
 *                    locks and condition vairables.
 */
#if defined (__cplusplus)
extern "C" {
#endif

typedef void * os__syncLockPrivateSRWLOCK;

#if _WIN32_WINNT < 0x0601 /* _WIN32_WINNT_WIN7 == 0x0601 */
# pragma pack(push, 8)
typedef struct os__syncLockPrivateCRITSEC_s {
    void *a;
    long b, c;
    void *d, *e, *f;
} os__syncLockPrivateCRITSEC;
# pragma pack(pop)

typedef struct os__syncLockPrivateEVENT_s {
    long a;
    void *b;
} os__syncLockPrivateEVENT;

/* A struct with at least enough size to hold the native lock on this platform.  Ensure enough
 * space is available to store a CRITICAL_SECTION, event-based lock or a SRWLOCK. */
struct os__syncLockPrivate_s {
    union {
        os__syncLockPrivateCRITSEC opaque_critsec;
        os__syncLockPrivateSRWLOCK opaque_srwlock;
        os__syncLockPrivateEVENT opaque_eventlock;
    };
};
#else
struct os__syncLockPrivate_s {
    /* If Windows 7 is targeted at compile-time, no CRITICAL_SECTIONS are used. SRWLOCK has the size
     * of a pointer according to MSDN, so reserve space for the srwlock using the pointer-type. */
    os__syncLockPrivateSRWLOCK opaque;
};
#endif /* _WIN32_WINNT < 0x0601 */

typedef struct os__syncLockPrivate_s os__syncLockPrivate;
#if !OS__SYNC_NOSHARED
typedef long os__syncLockShared;
#endif

/* A struct large enough to store a lock used by the sync-API. */
typedef union os__syncLock_u {
    os__syncLockPrivate priv;
#if !OS__SYNC_NOSHARED
    os__syncLockShared sha;
#endif
} os_syncLock;

typedef struct os_condPrivateCONDVAR_s {
    void *a;
} os__syncCondPrivateCONDVAR;
/* A struct with at least enough size to hold the native condition on this platform. In order not to
 * include Windows.h in this public header, this will be set to something with the right size which
 * is asserted at compile time in the implementation. */
#if _WIN32_WINNT < 0x0601 /* _WIN32_WINNT_WIN7 == 0x0601 */
typedef struct os_condPrivateSEMCOND_s {
    void *a, *b;
    long c;
} os__syncCondPrivateSEMCOND;

struct os__syncCondPrivate_s {
    union {
        os__syncCondPrivateSEMCOND opaque_semcond;
        os__syncCondPrivateCONDVAR opaque_condvar;
    };
};
#else
struct os__syncCondPrivate_s {
    /* If Windows 7 is targeted at compile-time CONDITION_VARIABLEs are used. CONDITION_VARIABLE has
     * the size of a pointer, so reserve space for the CONDITION_VARIABLE using the pointer-type. */
    os__syncCondPrivateCONDVAR opaque_condvar;
};
#endif /* _WIN32_WINNT < 0x0601 */

typedef struct os__syncCondPrivate_s os__syncCondPrivate;
#if !OS__SYNC_NOSHARED
typedef long os__syncCondShared;
#endif

/* A struct large enough to store a condition variable used by the sync-API. */
typedef union os__syncCond_u {
    os__syncCondPrivate priv;
#if !OS__SYNC_NOSHARED
    os__syncCondShared sha;
#endif
} os_syncCond;

#if defined (__cplusplus)
}
#endif

/* (progn (c-set-style "k&r") (setq c-basic-offset 4)) */

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

#ifndef C_MM_H
#define C_MM_H

#include "c_typebase.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

/* this will make mmState return the exact amount of memory used
 * only use this in tests as this is lock intrusive*/
#define C_MM_STATS 2

#define C_MM_RESERVATION_NO_CHECK    ((os_address)-1)
#define C_MM_RESERVATION_ZERO        (0)
#define C_MM_RESERVATION_LOW         (10000)
#define C_MM_RESERVATION_HIGH        (100000)


typedef enum c_mm_mode {
    MM_SHARED,  /* really using shared memory */
    MM_PRIVATE, /* this allocator in process-private memory */
    MM_HEAP     /* forward to os_malloc/os_free */
} c_mm_mode;

typedef struct c_mm_s       *c_mm;
typedef struct c_mmStatus_s c_mmStatus;

struct c_mmStatus_s {
    c_size size;
    c_size used;
    c_size maxUsed;
    c_size garbage;
    c_longlong count;
    c_ulonglong fails;
    /* The cached field will be filled with the amount of memory allocated for
     * caches (including all headers). */
    c_size cached;
    /* The preallocated field will be filled with the amount of memory that is
     * preallocated in caches, but is not in use. So in order to retain the
     * total amount of memory in use:
     *      totalInUse = used - preallocated;
     * And in order to get all free memory (including allocated, but available
     * in caches):
     *      totalFree = size - totalInUse */
    c_size preallocated;
    /* The mmMode field indicates if the memory map is in shared memory, private
     * memory or heap memory. */
    c_mm_mode mmMode;
};

OS_API c_mm c_mmCreate (void *address, c_size size, c_size threshold);
OS_API c_mmStatus c_mmState (c_mm mm, c_ulong flags);
OS_API c_mm_mode c_mmMode (c_mm mm);

OS_API os_int64 c_mmGetUsedMem (c_mm mm);

OS_API void c_mmSuspend(c_mm mm);
OS_API int c_mmResume(c_mm mm);

OS_API void *c_mmCheckPtr(c_mm mm, void *ptr);
OS_API c_size c_mmSize (c_mm mm);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_MM_H */

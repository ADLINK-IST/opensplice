/* -*- mode: c; c-file-style: "k&r"; c-basic-offset: 4; -*- */

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

#ifndef USE_ADV_MEM_MNG

#include "c__mmbase.h"
#include "c_mmbase.h"
#include "os_heap.h"

struct c_mm_s {
    int dummy;
};

c_mm
c_mmCreate(
    void *address,
    c_size size,
    c_size threshold)
{
    c_mm mm;
    OS_UNUSED_ARG(address);
    OS_UNUSED_ARG(size);
    OS_UNUSED_ARG(threshold);
    mm = os_malloc(sizeof(*mm));
    mm->dummy = 0;
    return mm;
}

c_mmStatus
c_mmState (
    c_mm mm,
    c_ulong flags)
{
    c_mmStatus s;
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(flags);

    s.size = 0;
    s.used = 0;
    s.maxUsed = 0;
    s.garbage = 0;
    s.count = 0;
    s.fails = 0;
    s.cached = 0;
    s.preallocated = 0;
    s.mmMode = MM_HEAP;

    return s;
}

c_mm_mode
c_mmMode (
    c_mm mm)
{
    return MM_HEAP;
}

os_int64
c_mmGetUsedMem (
    c_mm mm)
{
    OS_UNUSED_ARG(mm);
    return 0;
}

void
c_mmDestroy(
    c_mm mm)
{
    os_free(mm);
}

void *
c_mmAddress(
    c_mm mm)
{
    return mm;
}


void *
c_mmMalloc(
    c_mm mm,
    os_size_t size)
{
    void* retVal;
    OS_UNUSED_ARG(mm);

    if(size != 0)
    {
        retVal = os_malloc(size);
    } else
    {
        retVal = NULL;
    }
    return retVal;
}

void *
c_mmMallocThreshold (
    c_mm mm,
    os_size_t size)
{
    return c_mmMalloc(mm, size);
}

void
c_mmFree(
    c_mm mm,
    void *memory)
{
    OS_UNUSED_ARG(mm);
    if(memory != NULL)
    {
        os_free(memory);
    }
}


void *
c_mmBind(
    c_mm mm,
    const c_char *name,
    void *memory)
{
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(name);
    return memory;
}

void
c_mmUnbind(
    c_mm mm,
    const c_char *name)
{
    assert(0);
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(name);
}

void *
c_mmLookup(
    c_mm mm,
    const c_char *name)
{
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(name);
    return NULL;
}

void
c_mmSuspend(
    c_mm mm)
{
    OS_UNUSED_ARG(mm);
    /* do nothing */
}

int
c_mmResume(
    c_mm mm)
{
    OS_UNUSED_ARG(mm);
    return 0;
}

c_memoryThreshold
c_mmbaseGetMemThresholdStatus(
    c_mm mm)
{
    OS_UNUSED_ARG(mm);
    return C_MEMTHRESHOLD_OK;
}

c_bool
c_mmbaseMakeReservation (
    c_mm mm,
    os_address amount)
{
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(amount);
    return 1;
}

void
c_mmbaseReleaseReservation (
    c_mm mm,
    os_address amount)
{
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(amount);
}

void *
c_mmCheckPtr(
    c_mm mm,
    void *ptr)
{
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(ptr);
    return NULL;
}

c_size
c_mmSize (c_mm mm)
{
    OS_UNUSED_ARG(mm);
    return 0;
}

void c_mmTrackObject (struct c_mm_s *mm, const void *ptr, os_uint32 code)
{
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(ptr);
    OS_UNUSED_ARG(code);
}

void c_mmPrintObjectHistory(FILE *fp, c_mm mm, void *ptr)
{
    OS_UNUSED_ARG(mm);
    OS_UNUSED_ARG(ptr);
    fprintf (fp, "no object history tracing available\n");
}
#else
/*
 * warning: ISO C forbids an empty translation unit [-pedantic]
 */
typedef int EhWasHere; /* Avoid compiler warning for empty file. */
#endif /* !defined USE_ADV_MEM_MNG */

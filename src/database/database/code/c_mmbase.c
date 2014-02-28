/* -*- mode: c; c-file-style: "k&r"; c-basic-offset: 4; -*- */

/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2013 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
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
    if ((mm = os_malloc(sizeof(*mm))) != NULL) {
        mm->dummy = 0;
    }
    return mm;
}

c_mmStatus
c_mmState (
    c_mm mm,
    c_ulong flags)
{
    c_mmStatus s;

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

os_int64
c_mmGetUsedMem (
    c_mm mm)
{
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
    c_long size)
{
    void* retVal;

    if(size != 0)
    {
        retVal = os_malloc(size);
    } else
    {
        retVal = NULL;
    }
    return retVal;
}

void
c_mmFree(
    c_mm mm,
    void *memory)
{
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
    return NULL;
}

void
c_mmSuspend(
    c_mm mm)
{
    /* do nothing */
}

int
c_mmResume(
    c_mm mm)
{
    return 0;
}

c_memoryThreshold
c_mmbaseGetMemThresholdStatus(
    c_mm mm)
{
    return C_MEMTHRESHOLD_OK;
}

void *
c_mmCheckPtr(
    c_mm mm,
    void *ptr)
{
    return NULL;
}

void c_mmTrackObject (struct c_mm_s *mm, const void *ptr, os_uint32 code)
{
}

void c_mmPrintObjectHistory(FILE *fp, c_mm mm, void *ptr)
{
    fprintf (fp, "no object history tracing available\n");
}
#else
typedef int EhWasHere; /* Avoid compiler warning for empty file. */
#endif /* !defined USE_ADV_MEM_MNG */

/*
 *                         Vortex OpenSplice
 *
 *   This software and documentation are Copyright 2006 to TO_YEAR ADLINK
 *   Technology Limited, its affiliated companies and licensors. All rights
 *   reserved.
 *
 *   Licensed under the ADLINK Software License Agreement Rev 2.7 2nd October
 *   2014 (the "License"); you may not use this file except in compliance with
 *   the License.
 *   You may obtain a copy of the License at:
 *                      $OSPL_HOME/LICENSE
 *
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
/* (progn (c-set-style "k&r") (setq c-basic-offset 4)) */

#ifndef C_MMHEAP_H
#define C_MMHEAP_H

#if defined (__cplusplus)
extern "C" {
#endif
#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif

#include "os_defs.h"
#include "os_mutex.h"

#define C_MMHEAP_SHARED 1u

struct c_mmheap_tree;
struct c_mmheap_list;

struct c_mmheap_region {
    os_address off, size;
    void *base;
    struct c_mmheap_region *next;
};

struct c_mmheap {
    os_mutex lock;
    struct c_mmheap_tree *free;
    struct c_mmheap_list *free1;
    struct c_mmheap_list *free2;
    os_uint32 flags;
    int dump;
    int check;
    os_uint32 heap_check_serial;
    os_address n_free_bytes;
    os_address n_free_blocks;
    os_address n_allocated_blocks;
    os_address n_failed_allocations;
    struct c_mmheap_region heap_region;
};

struct c_mmheapStats {
    os_address nused;
    os_address nfails;
    os_address totfree;
};

int c_mmheapInit (struct c_mmheap *heap, os_address off, os_address size, unsigned flags);
void c_mmheapFini (struct c_mmheap *heap);
int c_mmheapAddRegion (struct c_mmheap *heap, void *block, os_address size);
void c_mmheapDropRegion (struct c_mmheap *heap, os_address minfree, os_address minsize, os_address align, void (*dropped_cb) (void *arg, void *addr, os_address size), void *arg);
void *c_mmheapMalloc (struct c_mmheap *heap, os_address size);
void c_mmheapFree (struct c_mmheap *heap, void *b);
void c_mmheapStats (struct c_mmheap *heap, struct c_mmheapStats *st);
void *c_mmheapCheckPtr (struct c_mmheap *heap, void *ptr);
os_address c_mmheapLargestAvailable (struct c_mmheap *heap);

#undef OS_API
#if defined (__cplusplus)
}
#endif
#endif /* C_MMHEAP_H */

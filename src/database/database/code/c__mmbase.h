/* (progn (c-set-style "k&r") (setq c-basic-offset 4)) */

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

#ifndef C_MM__H
#define C_MM__H

#include "c_typebase.h"
#include "c_mmbase.h"
#include "os.h"

#if defined (__cplusplus)
extern "C" {
#endif

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define C_MMTRACKOBJECT_CODE_MIN 2

void  c_mmDestroy (c_mm mm);
void *c_mmAddress (c_mm mm);

void *c_mmMalloc  (c_mm mm, c_long   size);
void  c_mmFree    (c_mm mm, void *memory);
void c_mmTrackObject (struct c_mm_s *mm, const void *ptr, os_uint32 code);
OS_API void c_mmPrintObjectHistory(FILE *fp, c_mm mm, void *ptr);

void *c_mmBind    (c_mm mm, const c_char *name, void *memory);
void  c_mmUnbind  (c_mm mm, const c_char *name);
void *c_mmLookup  (c_mm mm, const c_char *name);

c_memoryThreshold
c_mmbaseGetMemThresholdStatus(
    c_mm mm);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_MM__H */

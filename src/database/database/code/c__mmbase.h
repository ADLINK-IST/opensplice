/* (progn (c-set-style "k&r") (setq c-basic-offset 4)) */

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

#ifndef C_MM__H
#define C_MM__H

#include "c_typebase.h"
#include "c_mmbase.h"
#include "vortex_os.h"

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

void *c_mmMalloc  (c_mm mm, os_size_t size);
void *c_mmMallocThreshold (c_mm mm, os_size_t size);
void  c_mmFree    (c_mm mm, void *memory);
void c_mmTrackObject (struct c_mm_s *mm, const void *ptr, os_uint32 code);
OS_API void c_mmPrintObjectHistory(FILE *fp, c_mm mm, void *ptr);

void *c_mmBind    (c_mm mm, const c_char *name, void *memory);
void  c_mmUnbind  (c_mm mm, const c_char *name);
void *c_mmLookup  (c_mm mm, const c_char *name);

c_memoryThreshold
c_mmbaseGetMemThresholdStatus(
    c_mm mm);

c_bool
c_mmbaseMakeReservation (
    c_mm mm,
    os_address amount);

void
c_mmbaseReleaseReservation (
    c_mm mm,
    os_address amount);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* C_MM__H */

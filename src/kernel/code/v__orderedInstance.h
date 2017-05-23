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
#ifndef V__ORDERED_INSTANCE_H
#define V__ORDERED_INSTANCE_H

#include "kernelModule.h"
#include "v_dataReader.h"
#include "v_dataViewSample.h"

#if defined (__cplusplus)
extern "C" {
#endif
#include "os_if.h"

#ifdef OSPL_BUILD_CORE
#define OS_API OS_API_EXPORT
#else
#define OS_API OS_API_IMPORT
#endif
/* !!!!!!!!NOTE From here no more includes are allowed!!!!!!! */

#define v_orderedInstance(o) (C_CAST(o,v_orderedInstance))

OS_API v_orderedInstance
v_orderedInstanceNew(
    v_entity entity,
    v_presentationKind presentation,
    v_orderbyKind orderby);

OS_API void
v_orderedInstanceReset (
    v_orderedInstance _this);

OS_API void
v_orderedInstanceRemove (
    v_orderedInstance _this,
    v_entity entity);

OS_API c_iter
v_orderedInstanceGetDataReaders (
    v_orderedInstance _this,
    v_sampleMask mask);

OS_API v_writeResult
v_orderedInstanceWrite (
    v_orderedInstance _this,
    v_readerSample sample);

OS_API v_dataReaderSample
v_orderedInstanceFirstSample (
    v_orderedInstance _this);

OS_API v_dataReaderSample
v_orderedInstanceReadSample (
    v_orderedInstance _this,
    v_sampleMask mask);

#if 0
OS_API void
v_orderedInstanceUnreadSample (
    v_orderedInstance _this,
    v_dataReaderSample sample);
#endif

/* If presentation access scope is set to group, the read/take operations must
   be aligned to the list of data readers returned by get_datareaders. In case
   an unauthorized read/take is registered the list is invalidated and the
   read/take operation should fallback to unordered mode. */
OS_API c_bool
v_orderedInstanceIsAligned (
    v_orderedInstance _this);

/* If a reader reads a sample that does not belong to itself, it must
   invalidate the list returned by get_datareaders by invoking this function.
   This will effectively mark the ordered instance unaligned and reset the
   bookmark. */
OS_API void
v_orderedInstanceUnaligned (
    v_orderedInstance _this);

#undef OS_API

#if defined (__cplusplus)
}
#endif

#endif /* V__ORDERED_INSTANCE_H */

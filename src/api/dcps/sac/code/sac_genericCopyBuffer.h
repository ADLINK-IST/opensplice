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
#ifndef DDS_GENERICCOPYBUFFER_H
#define DDS_GENERICCOPYBUFFER_H

#include "dds_dcps.h"
#include "sac_genericCopyCache.h"

typedef struct {
    DDS_unsigned_long  _maximum;
    DDS_unsigned_long  _length;
    void               *_buffer;
    DDS_boolean        _release;
} DDSSequenceType;

#define DDS_SEQUENCE_CORRECTION (2 * sizeof(DDS_unsigned_long) + sizeof(void*))

void
DDS_genericCopyBufferFreeType (
    DDSCopyHeader *ch,
    void           *ptr);

DDS_ReturnCode_t
DDS_genericCopyBufferFree (
    void *buffer);

void *
DDS_genericCopyBufferAlloc (
    DDS_copyCache      copyCache,
    DDSCopyHeader     *copyRoutine,
    DDS_unsigned_long  size,
    DDS_unsigned_long  count);

void *
DDS_genericCopyBufferAllocSeqBuffer (
    DDS_copyCache      copyCache,
    DDSCopyHeader     *copyRoutine,
    DDS_unsigned_long  size,
    DDS_unsigned_long  count);

#endif /* DDS_GENERICCOPYBUFFER_H */

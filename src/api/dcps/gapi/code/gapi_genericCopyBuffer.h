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
#ifndef GAPI_GENERICCOPYBUFFER_H
#define GAPI_GENERICCOPYBUFFER_H

#include "gapi.h"
#include "gapi_genericCopyCache.h"

typedef struct {
    gapi_unsigned_long  _maximum;
    gapi_unsigned_long  _length;
    void               *_buffer;
    gapi_boolean        _release;
} gapiSequenceType;

#define GAPI_SEQUENCE_CORRECTION (2 * sizeof(gapi_unsigned_long) + sizeof(void*))

void
gapi_genericCopyBufferFreeType (
    gapiCopyHeader *ch,
    void           *ptr);

gapi_boolean
gapi_genericCopyBufferFree (
    void *buffer);

void *
gapi_genericCopyBufferAlloc (
    gapi_copyCache      copyCache,
    gapiCopyHeader     *copyRoutine,
    gapi_unsigned_long  size,
    gapi_unsigned_long  count);

void *
gapi_genericCopyBufferAllocSeqBuffer (
    gapi_copyCache      copyCache,
    gapiCopyHeader     *copyRoutine,
    gapi_unsigned_long  size,
    gapi_unsigned_long  count);

#endif /* GAPI_GENERICCOPYBUFFER_H */

#ifndef GAPI_GENERICCOPYBUFFER_H
#define GAPI_GENERICCOPYBUFFER_H

#include "gapi.h"
#include "gapi_genericCopyCache.h"

void
gapi_genericCopyBufferFreeType (
    gapiCopyHeader *ch,
    void           *ptr);

void
gapi_genericCopyBufferFree (
    void *buffer);

void *
gapi_genericCopyBufferAlloc (
    gapiCopyHeader     *copyProgram,
    gapiCopyHeader     *copyRoutine,
    gapi_unsigned_long  size,
    gapi_unsigned_long  count);

void *
gapi_genericCopyBufferAllocSeqBuffer (
    gapiCopyHeader     *copyProgram,
    gapiCopyHeader     *copyRoutine,
    gapi_unsigned_long  size,
    gapi_unsigned_long  count);

#endif /* GAPI_GENERICCOPYBUFFER_H */

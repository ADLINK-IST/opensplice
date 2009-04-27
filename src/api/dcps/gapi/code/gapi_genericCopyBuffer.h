/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
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

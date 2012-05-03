/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/* Interface */
#include "nw_writerAsync.h"

/* Base classes */
#include "nw__writer.h"
#include "nw__runnable.h"

/* Implementation */
#include "nw__confidence.h"
#include "nw_commonTypes.h"
#include "nw_configurationDefs.h"
#include "nw_report.h"
#include "c_sync.h"
#include "c_time.h"
#include "v_kernel.h"
#include "v_networkReaderEntry.h"
#include "os.h"

/* _MI_ = Multiple Insertion threads.
 * Only enable if lock contention problems are solved.
 */
/*#define _MI_*/

NW_CLASS(nw_ringBufferEntry);
NW_STRUCT(nw_ringBufferEntry) {
    v_networkReaderEntry entry;
    v_message message;
    c_ulong messageId;
    v_gid   sender;
    c_bool  sendTo;
    v_gid   receiver; /* Valid only if sendTo == TRUE */
};

NW_CLASS(nw_ringBuffer);
NW_STRUCT(nw_ringBuffer) {
    os_uint32 nofEntries;
    nw_ringBufferEntry *entries;
    os_uint32 head;
    os_uint32 tail;
    os_uint32 currentSize;
};

#define NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, index)             \
        (ringBuffer->nofEntries > index)

#define NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, index)             \
        (nw_ringBufferEntry)(ringBuffer->entries[index])

#define NW_RINGBUFFER_INC(ringBuffer)                               \
        ringBuffer->head++;                                         \
        if (ringBuffer->head >= ringBuffer->nofEntries) {           \
            ringBuffer->head = 0;                                   \
        }                                                           \
        ringBuffer->currentSize++;

#define NW_RINGBUFFER_DEC(ringBuffer)                               \
        ringBuffer->tail++;                                         \
        if (ringBuffer->tail >= ringBuffer->nofEntries) {           \
            ringBuffer->tail = 0;                                   \
        }                                                           \
        NW_CONFIDENCE(ringBuffer->currentSize > 0);                 \
        ringBuffer->currentSize--;

#define NW_RINGBUFFER_IS_FULL(ringBuffer)                           \
        (ringBuffer->currentSize == ringBuffer->nofEntries)

#define NW_RINGBUFFER_IS_EMPTY(ringBuffer)                          \
        (ringBuffer->currentSize == 0)


static nw_ringBuffer
nw_ringBufferNew(
    os_uint32 nofEntries)
{
    nw_ringBuffer result;
    os_uint32 i;

    result = (nw_ringBuffer)os_malloc(sizeof(*result));
    if (result) {
        result->nofEntries = nofEntries;
        result->entries = (nw_ringBufferEntry *)os_malloc(nofEntries * sizeof(*(result->entries)));
        for (i=0; i<nofEntries; i++) {
            result->entries[i] = (nw_ringBufferEntry)os_malloc(sizeof(*(result->entries[0])));
        }
        result->head = 0;
        result->tail = 0;
        result->currentSize = 0;
    }
    return result;
}

static void
nw_ringBufferFree(
    nw_ringBuffer this)
{
    os_uint32 i;

    if (this) {
        if (this->entries) {
            for (i=0; i<this->nofEntries; i++) {
                os_free(this->entries[i]);
            }
            os_free(this->entries);
        }
        os_free(this);
    }
}

static void
nw_ringBufferPostEntry(
    nw_ringBuffer ringBuffer,
    v_networkReaderEntry entry,
    v_message message,
    c_ulong messageId,
    v_gid   sender,
    c_bool  sendTo,
    v_gid   receiver)
{
    nw_ringBufferEntry bufferEntry;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->head));

    bufferEntry = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->head);
    bufferEntry->entry = entry;
    bufferEntry->message = c_keep(message);
    bufferEntry->messageId = messageId;
    bufferEntry->sender = sender;
    bufferEntry->sendTo = sendTo;
    bufferEntry->receiver = receiver;
    NW_RINGBUFFER_INC(ringBuffer);
}

static nw_bool
nw_ringBufferProcessEntry(
    nw_ringBuffer ringBuffer,
    v_networkReaderEntry *entry,
    v_message *message,
    c_ulong *messageId,
    v_gid   *sender,
    c_bool  *sendTo,
    v_gid   *receiver)
{
    nw_ringBufferEntry bufferEntry;
    nw_bool result;

    NW_CONFIDENCE(ringBuffer != NULL);
    NW_CONFIDENCE(NW_RINGBUFFER_INDEX_IS_VALID(ringBuffer, ringBuffer->tail));

    if (!NW_RINGBUFFER_IS_EMPTY(ringBuffer)) {
        bufferEntry = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->tail);
        *entry = bufferEntry->entry;
        *message = bufferEntry->message;
        *messageId = bufferEntry->messageId;
        *sender = bufferEntry->sender;
        *sendTo = bufferEntry->sendTo;
        *receiver = bufferEntry->receiver;
#ifndef NDEBUG
        memset(bufferEntry, 0, sizeof(NW_STRUCT(nw_ringBufferEntry)));
#endif
        NW_RINGBUFFER_DEC(ringBuffer);
        result = TRUE;
    } else {
        result = FALSE;
    }
    return result;
}


/* -----------------------------------------------------------------------------
 * - the actual writerAsync class
 * -------------------------------------------------------------------------- */
NW_STRUCT(nw_writerAsync) {
    struct nw_writer_s writer; /* Multiple inheritance */
    struct nw_runnable_s runnable; /* Multiple inheritance */
#ifdef _MI_
    struct nw_runnable_s runnable2; /* Multiple inheritance */
#endif
    nw_ringBuffer ringBuffer;
    c_mutex mutex;
    c_cond condition_full;
    nw_bool blocking_full;
    c_cond condition_empty;
    nw_bool blocking_empty;
};

static void *nw_writerAsyncMain(
    nw_runnable runnable, c_voidp arg);

static nw_bool nw_writerAsyncWriteMessage(
    nw_writer writer,
    v_networkReaderEntry entry,
    v_message message,
    c_ulong messageId,
    v_gid sender,
    c_bool sendTo,
    v_gid receiver);

static void nw_writerAsyncFinalize(
    nw_writer writer);

#define BUFFERSIZE (1000)

nw_writer
nw_writerAsyncNew(
    const char *parentPathName,
    const char *pathName)
{
    nw_writer result = NULL;
    nw_writerAsync writerAsync;
    nw_runnable runnable;
#ifdef _MI_
    nw_runnable runnable2;
#endif
    char *tmpPath;
    size_t tmpPathSize;

    writerAsync = (nw_writerAsync)os_malloc(sizeof(*writerAsync));

    if (writerAsync != NULL) {
        /* Initialize parent */
        runnable = (nw_runnable)&(writerAsync->runnable);
#ifdef _MI_
        runnable2 = (nw_runnable)&(writerAsync->runnable2);
#endif
        /* First determine its parameter path */
        if(nw_configurationElementHasChildElementWithName(pathName, NWCF_ROOT(Scheduling)))
        {
            tmpPathSize = strlen(pathName) +  strlen(NWCF_SEP) +
                strlen(NWCF_ROOT(Scheduling)) + 1 /* '\0' */;
            tmpPath = os_malloc(tmpPathSize);
            os_sprintf(tmpPath, "%s%s%s", pathName, NWCF_SEP, NWCF_ROOT(Scheduling));
        } else
        {
            tmpPathSize = strlen(parentPathName) + strlen(NWCF_SEP) +
                strlen(NWCF_ROOT(Rx)) + strlen(NWCF_SEP) +
                strlen(NWCF_ROOT(Scheduling)) + 1 /* '\0' */;
            tmpPath = os_malloc(tmpPathSize);
            os_sprintf(tmpPath, "%s%s%s%s%s", parentPathName, NWCF_SEP, NWCF_ROOT(Rx),
                NWCF_SEP, NWCF_ROOT(Scheduling));
        }

        result = &(writerAsync->writer);
        nw_runnableInitialize(runnable,
                              pathName /* Use pathName as name */,
                              tmpPath,
                              nw_writerAsyncMain, result,
                              NULL, NULL);
#ifdef _MI_
        nw_runnableInitialize(runnable2,
                              pathName /* Use pathName as name */,
                              tmpPath,
                              nw_writerAsyncMain, result,
                              NULL, NULL);
#endif

        /* Initialize other parent */
        nw_writerInitialize(result, nw_writerAsyncWriteMessage, nw_writerAsyncFinalize);

        /* Init self */
        c_mutexInit(&writerAsync->mutex, PRIVATE_MUTEX);
        c_condInit(&writerAsync->condition_full, &writerAsync->mutex, PRIVATE_COND);
        writerAsync->blocking_full = FALSE;
        c_condInit(&writerAsync->condition_empty, &writerAsync->mutex, PRIVATE_COND);
        writerAsync->blocking_empty = FALSE;
        writerAsync->ringBuffer = nw_ringBufferNew(BUFFERSIZE);
        /* Start the thread */
        nw_runnableStart(runnable);
#ifdef _MI_
        nw_runnableStart(runnable2);
#endif
    }

    return result;
}

static nw_bool
nw_writerAsyncWriteMessage(
    nw_writer writer,
    v_networkReaderEntry entry,
    v_message message,
    c_ulong messageId,
    v_gid   sender,
    c_bool  sendTo,
    v_gid   receiver)
{
    nw_writerAsync this = (nw_writerAsync)writer;
    c_time timeOut = {0, 20000000};
    nw_bool result = FALSE;

    if (this != NULL) {
        c_mutexLock(&this->mutex);
        /* if the ringbuffer is full, wait max 20 ms until there is more room */
        /* if still no room, return false */
        if (NW_RINGBUFFER_IS_FULL(this->ringBuffer)) {
            NW_TRACE(Receive, 4, "Block on Full buffer. ");
            this->blocking_full = TRUE;
            c_condTimedWait(&this->condition_full, &this->mutex, timeOut);
            this->blocking_full = FALSE;
        }

        if (! NW_RINGBUFFER_IS_FULL(this->ringBuffer)) {
            nw_ringBufferPostEntry(this->ringBuffer,
                entry, message, messageId, sender, sendTo, receiver);
            if (this->blocking_empty) {
                c_condBroadcast(&this->condition_empty);
            }
            result = TRUE;
        }
        c_mutexUnlock(&this->mutex);
    }
    return result;
}

static void *
nw_writerAsyncMain(
    nw_runnable runnable,
    c_voidp arg)
{
    nw_writerAsync this = (nw_writerAsync)arg;
    c_time timeOut = {0, 200000000};
    nw_bool messageAvailable = FALSE;
    v_networkReaderEntry entry = NULL;
    v_writeResult write_result = V_WRITE_SUCCESS;
    v_message message = NULL;
    c_ulong messageId = 0;
    v_gid   sender = {0,0,0};
    c_bool  sendTo = FALSE;
    c_time sleep = {0,1000000};
    v_gid   receiver = {0,0,0};

    c_ulong level = BUFFERSIZE/2;

    nw_runnableSetRunState(runnable, rsRunning);
    while (!nw_runnableTerminationRequested(runnable)) {
        do {
            c_mutexLock(&this->mutex);
            messageAvailable = nw_ringBufferProcessEntry(this->ringBuffer,
                &entry, &message, &messageId, &sender, &sendTo, &receiver);
            /* we just created some room in the ringbuffer, is another thread
             * was waiting for some room and enough is available, then notify it!
             */
            if (this->blocking_full && (this->ringBuffer->currentSize < level)) {
                c_condBroadcast(&this->condition_full);
            }
            c_mutexUnlock(&this->mutex);
            if (messageAvailable) {
                write_result = v_networkReaderEntryReceive(entry, message, messageId, sender, sendTo, receiver);

                while ( (write_result == V_WRITE_REJECTED) && !nw_runnableTerminationRequested(runnable)) {
                    c_timeNanoSleep(sleep);
                    write_result = v_networkReaderEntryReceive(entry, message, messageId, sender, sendTo, receiver);
                }
                c_free(message);
            }
        } while ((messageAvailable) && !nw_runnableTerminationRequested(runnable));

        c_mutexLock(&this->mutex);
        /* only block if the ringbuffer is still empty! */
        if (NW_RINGBUFFER_IS_EMPTY(this->ringBuffer)) {
            this->blocking_empty = TRUE;
            c_condTimedWait(&this->condition_empty, &this->mutex, timeOut);
            this->blocking_empty = FALSE;
        }
        c_mutexUnlock(&this->mutex);
    }
    nw_runnableSetRunState(runnable, rsTerminated);

    return NULL;
}

static void
nw_writerAsyncFinalize(
    nw_writer writer)
{
    nw_writerAsync this = (nw_writerAsync)writer;

    if (this) {
        /* Finalize runnable parent */
        nw_runnableStop(&(this->runnable)); /* this will actually stop the loop in nw_writerAsyncMain */
        nw_runnableFinalize(&(this->runnable));
#ifdef _MI_
        nw_runnableStop(&(this->runnable2)); /* this will actually stop the loop in nw_writerAsyncMain */
        nw_runnableFinalize(&(this->runnable2));
#endif
        /* Finalize nw_writer parent */
        /* (Nothing) */
        /* Finalize self */
        nw_ringBufferFree(this->ringBuffer);
        c_mutexDestroy(&this->mutex);
        c_condDestroy(&this->condition_full);
        c_condDestroy(&this->condition_empty);
    }
}

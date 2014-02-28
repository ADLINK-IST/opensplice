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

/* Interface */
#include "v_networkReaderEntry.h"

/* Implementation */
#include "os_heap.h"
#include "c_misc.h"
#include "v__entry.h"
#include "v__reader.h"        /* for v_reader()        */
#include "v__writer.h"        /* for v_writer()        */
#include "v_writerSample.h"
#include "v_group.h"         /* for v_group()         */
#include "v_message.h"       /* for v_message()       */
#include "v_entity.h"        /* for v_entity()        */
#include "v_handle.h"
#include "v__networkReader.h" /* Friend class */
#include "v_public.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_writerInstance.h"

#ifdef __VERBOSE__
#include "os_time.h"
#define PRINT_TIME                                        \
    {                                                     \
       os_time t = os_timeGet();                          \
       printf("%5d.%3.3d ", t.tv_sec, t.tv_nsec/1000000); \
    }
#define __PRINT__(msg) \
    PRINT_TIME         \
    printf(msg "\n");
#define __PRINT_1__(msg, a1) \
    PRINT_TIME               \
    printf(msg "\n", a1);
#define __PRINT_2__(msg, a1, a2) \
    PRINT_TIME                   \
    printf(msg "\n", a1, a2);
#define __PRINT_3__(msg, a1, a2, a3) \
    PRINT_TIME                       \
    printf(msg "\n", a1, a2, a3);
#else /* __VERBOSE__ */
#define __PRINT__(msg)
#define __PRINT_1__(msg, a1)
#define __PRINT_2__(msg, a1, a2)
#define __PRINT_3__(msg, a1, a2, a3)
#endif /* __VERBOSE__ */



/* ----------------------------- v_networkReaderEntry ----------------------- */


#define NW_ROT_CHAR(val, rot) (((val) << (rot)) + ((val) >> (8-(rot))))

static v_networkHashValue
v_networkReaderEntryCalculateHashValue(
    v_networkReaderEntry entry)
{
    v_networkHashValue result = {0xa0, 0x22, 0x8d, 0x07};

    const char *partitionName;
    const char *topicName;
    const char *currentPtr;

    partitionName = v_partitionName(v_groupPartition(entry->group));
    topicName = v_topicName(v_groupTopic(entry->group));
    currentPtr = partitionName;

    while (*currentPtr != '\0') {
        /* gcc2.96 gave internal compile errrors (with optimisation enabled)
         * when compiling the NW_ROT_CHAR macro twice in same command :
         * these assignments are deliberatly split over 2 lines as workaround.
         */

        result.h1 = NW_ROT_CHAR(result.h1, 1);
        result.h1 += NW_ROT_CHAR(*currentPtr, 4);

        result.h2 = NW_ROT_CHAR(result.h2, 2);
        result.h2 += NW_ROT_CHAR(*currentPtr, 7);

        result.h3 = NW_ROT_CHAR(result.h3, 3);
        result.h3 += NW_ROT_CHAR(*currentPtr, 1);

        result.h4 = NW_ROT_CHAR(result.h4, 4);
        result.h4 += NW_ROT_CHAR(*currentPtr, 5);

        currentPtr = &(currentPtr[1]);
    }

    currentPtr = topicName;
    while (*currentPtr != '\0') {
        result.h1 = NW_ROT_CHAR(result.h1, 4);
        result.h1 += NW_ROT_CHAR(*currentPtr, 7);

        result.h2 = NW_ROT_CHAR(result.h2, 3);
        result.h2 += NW_ROT_CHAR(*currentPtr, 1);

        result.h3 = NW_ROT_CHAR(result.h3, 2);
        result.h3 += NW_ROT_CHAR(*currentPtr, 5);

        result.h4 = NW_ROT_CHAR(result.h4, 1);
        result.h4 += NW_ROT_CHAR(*currentPtr, 4);

        currentPtr = &(currentPtr[1]);
    }
    return result;
}


static void
v_networkReaderEntryInit(
    v_networkReaderEntry entry,
    v_networkReader reader,
    v_group group,
    v_networkId networkId,
    c_ulong channelsToConnect,
    v_networkPartitionId networkPartitionId,
    c_bool isRouting)
{
    v_networkReaderEntry found;

    v_entryInit(v_entry(entry),v_reader(reader));

    entry->group = c_keep(group);
    entry->networkId = networkId;
    entry->channelCountdown = channelsToConnect;
    c_mutexInit(&entry->channelCountdownMutex, SHARED_MUTEX);
    entry->networkPartitionId = networkPartitionId;
    entry->hashValue = v_networkReaderEntryCalculateHashValue(entry);
    entry->isRouting = isRouting;

    found = v_networkReaderEntry(v_readerAddEntry(v_reader(reader), v_entry(entry)));
    assert(found == entry);
    c_free(found);
}

/* Protected constructor */

v_networkReaderEntry
v_networkReaderEntryNew(
    v_networkReader reader,
    v_group group,
    v_networkId networkId,
    c_ulong channelsToConnect,
    v_networkPartitionId networkPartitionId,
    c_bool isRouting)
{
    v_kernel kernel;
    v_networkReaderEntry result;

    assert(C_TYPECHECK(reader, v_networkReader));
    assert(C_TYPECHECK(group, v_group));

    kernel = v_objectKernel(reader);
    result = v_networkReaderEntry(v_objectNew(kernel,K_NETWORKREADERENTRY));
    v_networkReaderEntryInit(result, reader, group, networkId,
        channelsToConnect, networkPartitionId, isRouting);

    return result;
}

void
v_networkReaderEntryNotifyConnected(
    v_networkReaderEntry entry,
    const c_char* serviceName)
{
    c_bool allChannelsConnected = FALSE;

    c_mutexLock(&entry->channelCountdownMutex);

    assert (entry->channelCountdown > 0);
    entry->channelCountdown--;
    if (entry->channelCountdown == 0) {
        allChannelsConnected = TRUE;
    }

    c_mutexUnlock(&entry->channelCountdownMutex);

    if (allChannelsConnected) {
        v_groupAddEntry(v_group(entry->group), v_entry(entry));
        v_groupNotifyAwareness(v_group(entry->group),serviceName,TRUE);
        v_groupGetHistoricalData(v_group(entry->group), v_entry(entry));
    }
}


c_bool
v_networkReaderEntryIsRouting(
    v_networkReaderEntry entry)
{
    assert(C_TYPECHECK(entry, v_networkReaderEntry));

    return entry->isRouting;
}

void
v_networkReaderEntryFree(
    v_networkReaderEntry e)
{
    c_free(c_object(e));
}


v_writeResult
v_networkReaderEntryWrite(
    v_networkReaderEntry entry,
    v_message message,
    v_networkId writingNetworkId)
{
    v_writeResult result = V_WRITE_SUCCESS;
    c_bool writeSucceeded;
    static v_gid zeroAddressee = {0,0,0};

    assert(C_TYPECHECK(entry, v_networkReaderEntry));
    assert(message != NULL);

    /* First check if there is any remote interest at all */

    if (v_networkReader(v_entry(entry)->reader)->remoteActivity) {
        /* Only forward messages that come from this kernel */
        if (writingNetworkId == V_NETWORKID_LOCAL || entry->isRouting) {
            /* OK, message is from this kernel or this is a routing entry. Now
             * attach the correct fields if needed */

            /* TODO: For networking services that support routing perhaps
             * messages shouldn't be forwarded to 'self' (e.g., echo cancellation
             * may be needed). For R&R this modus is fine. */
            writeSucceeded = v_networkReaderWrite(
                                  v_networkReader(v_entry(entry)->reader),
                                  message, entry, 0, message->writerGID,
                                  FALSE /* no p2p */, zeroAddressee);
            if (writeSucceeded) {
                result = V_WRITE_SUCCESS;
            } else {
                result = V_WRITE_REJECTED;
            }
        }
    }

    return result;
}

v_writeResult
v_networkReaderEntryReceive(
    v_networkReaderEntry entry,
    v_message message,
    c_ulong sequenceNumber,
    v_gid sender,
    c_bool sendTo, /* for p2p writing */
    v_gid receiver)
{
    /* Prepared for p2p sending but not yet implemented */
    /* Ignore result */

    v_resendScope resendScope = V_RESEND_NONE; /* resendScope not yet used here beyond this function */

    OS_UNUSED_ARG(sequenceNumber);
    OS_UNUSED_ARG(sender);
    OS_UNUSED_ARG(sendTo);
    OS_UNUSED_ARG(receiver);

    return v_groupWriteCheckSampleLost(entry->group, message,
        NULL /* no instance pointer */, entry->networkId, &resendScope);
}






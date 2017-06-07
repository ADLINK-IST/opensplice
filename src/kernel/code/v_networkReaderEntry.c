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
       os_timeW t = os_timeWGet();                        \
       printf(" %"PA_PRItime" ", OS_TIMEW_PRINT(t)); \
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


#define NW_ROT_CHAR(val, rot) ((c_octet) (((val) << (rot)) + ((val) >> (8-(rot)))))

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
        result.h1 = (c_octet) (NW_ROT_CHAR(result.h1, 1) + NW_ROT_CHAR(*currentPtr, 4));
        result.h2 = (c_octet) (NW_ROT_CHAR(result.h2, 2) + NW_ROT_CHAR(*currentPtr, 7));
        result.h3 = (c_octet) (NW_ROT_CHAR(result.h3, 3) + NW_ROT_CHAR(*currentPtr, 1));
        result.h4 = (c_octet) (NW_ROT_CHAR(result.h4, 4) + NW_ROT_CHAR(*currentPtr, 5));
        currentPtr++;
    }

    currentPtr = topicName;
    while (*currentPtr != '\0') {
        result.h1 = (c_octet) (NW_ROT_CHAR(result.h1, 4) + NW_ROT_CHAR(*currentPtr, 7));
        result.h2 = (c_octet) (NW_ROT_CHAR(result.h2, 3) + NW_ROT_CHAR(*currentPtr, 1));
        result.h3 = (c_octet) (NW_ROT_CHAR(result.h3, 2) + NW_ROT_CHAR(*currentPtr, 5));
        result.h4 = (c_octet) (NW_ROT_CHAR(result.h4, 1) + NW_ROT_CHAR(*currentPtr, 4));
        currentPtr++;
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
    v_networkRoutingMode routing)
{
    v_networkReaderEntry found;

    v_entryInit(v_entry(entry),v_reader(reader));

    entry->group = c_keep(group);
    entry->networkId = networkId;
    entry->channelCountdown = channelsToConnect;
    c_mutexInit(c_getBase(entry), &entry->channelCountdownMutex);
    entry->networkPartitionId = networkPartitionId;
    entry->hashValue = v_networkReaderEntryCalculateHashValue(entry);
    entry->routing = routing;

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
    v_networkRoutingMode routing)
{
    v_kernel kernel;
    v_networkReaderEntry result;

    assert(C_TYPECHECK(reader, v_networkReader));
    assert(C_TYPECHECK(group, v_group));

    kernel = v_objectKernel(reader);
    result = v_networkReaderEntry(v_objectNew(kernel,K_NETWORKREADERENTRY));
    v_networkReaderEntryInit(result, reader, group, networkId, channelsToConnect, networkPartitionId, routing);

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
    }
}

c_bool
v_networkReaderEntryIsRouting(
    v_networkReaderEntry entry)
{
    assert(C_TYPECHECK(entry, v_networkReaderEntry));

    return entry->routing >= V_NETWORKROUTING_ROUTING;
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
    v_networkId writingNetworkId,
    c_bool groupRoutingEnabled)
{
    static const v_gid zeroAddressee = {0, 0, 0};

    assert(C_TYPECHECK(entry, v_networkReaderEntry));
    assert(message != NULL);

    if (!v_networkReader(v_entry(entry)->reader)->remoteActivity) {
        return V_WRITE_SUCCESS;
    }

    if (writingNetworkId != V_NETWORKID_LOCAL)
    {
        switch (entry->routing)
        {
            case V_NETWORKROUTING_NONE:
                return V_WRITE_SUCCESS;
            case V_NETWORKROUTING_FROM_GROUP:
                if (writingNetworkId == entry->networkId || !groupRoutingEnabled) {
                    return V_WRITE_SUCCESS;
                }
                break;
            case V_NETWORKROUTING_ROUTING:
                if (writingNetworkId == entry->networkId) {
                    return V_WRITE_SUCCESS;
                }
                break;
            case V_NETWORKROUTING_ECHO:
                break;
        }
    }

    if (v_networkReaderWrite(v_networkReader(v_entry(entry)->reader), message, entry, 0, message->writerGID, FALSE /* no p2p */, zeroAddressee)) {
        return V_WRITE_SUCCESS;
    } else {
        return V_WRITE_REJECTED;
    }
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






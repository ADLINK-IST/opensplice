/* Interface */
#include "nw_discovery.h"

/* Implementation */
#include "os_stdlib.h"
#include "os_heap.h"
#include "c_typebase.h"
#include "c_sync.h"
#include "c_time.h"
#include "v_time.h" /* for v_timeGet */
#include "nw__confidence.h"
#include "nw__runnable.h"
#include "nw_plugTypes.h"
#include "nw_plugNetwork.h"
#include "nw__plugNetwork.h"
#include "nw__plugChannel.h"
#include "nw_plugSendChannel.h"
#include "nw_plugReceiveChannel.h"
#include "nw_bridge.h" /* for v_gidToGlobalId */
#include "nw_report.h"
#include "kernelModule.h"
#include "u_networkReader.h"
#include "v_networkReader.h"

NW_STRUCT(nw_discovery) {
	NW_EXTENDS(nw_runnable);
    nw_plugNetwork network;
};
NW_CLASS(nw_discovery);

/* -------------------------- Baseclass (abstract) -------------------------- */

static void
nw_discoveryInitialize(
    nw_discovery discovery,
    nw_networkId networkId,
    const char *name,
    const char *pathName,
    const nw_runnableMainFunc mainFunc,
    const nw_runnableTriggerFunc triggerFunc,
    const nw_runnableFinalizeFunc finalizeFunc)
{
	NW_CONFIDENCE(discovery != NULL);

    /* Initialize parent */
    nw_runnableInitialize((nw_runnable)discovery, name, pathName,
        mainFunc, NULL, triggerFunc, finalizeFunc);

    /* Initialize self */
	discovery->network = nw_plugNetworkIncarnate(networkId);
}

static void
nw_discoveryFinalize(
    nw_runnable runnable)
{
	nw_discovery discovery;

	discovery = (nw_discovery)runnable;
	NW_CONFIDENCE(discovery != NULL);

	/* Finalize parent */
	nw_runnableFinalize(runnable);

    /* Finalize self */
	nw_plugNetworkExcarnate(discovery->network);
}


/* --------------------- discoveryWriter (concrete) ------------------------- */

static void nw_discoveryWriterTrigger(nw_runnable runnable);

NW_STRUCT(nw_discoveryWriter) {
    NW_EXTENDS(nw_discovery);
    /* NetworkId of this network */
    v_networkId networkId;
    /* Immediate interaction with the networking plug */
    nw_plugChannel sendChannel;
    /* trigger for responding to new nodes */
    nw_bool respondToNode;
    /* Lock and condVar for inter-thread communication */
    c_mutex mutex;
    c_cond condition;
};

static void
nw_discoveryWriterWriteMessageToNetwork(
    nw_discoveryWriter discoveryWriter,
    nw_data message,
    nw_length length)
{
    nw_data messageBuffer;
    nw_length bufferLength;
    nw_signedLength maxBytes;
    nw_bool result;

    NW_TRACE(Discovery, 6, "Sending heartbeat to the network");
    /* First retrieve a buffer to write in */
    maxBytes = 0; /* don't flush. */
    nw_plugSendChannelMessageStart(discoveryWriter->sendChannel, &messageBuffer, &bufferLength, 0, &maxBytes);
    /* Copy the message into the buffer */
    NW_CONFIDENCE(bufferLength >= length);
    memcpy((void *)messageBuffer, message, length);
    /* Update buffer position and size */
    messageBuffer = &(messageBuffer[length]);
    /* Do write and flush immediately */
    nw_plugSendChannelMessageEnd(discoveryWriter->sendChannel, messageBuffer);
    maxBytes = bufferLength;
    result = nw_plugSendChannelMessagesFlush(discoveryWriter->sendChannel,
                                             TRUE, &maxBytes);
    NW_CONFIDENCE(result);
}

#define NW_DISCOVERY_STARTING_SIGN 'a'
#define NW_DISCOVERY_ALIVE_SIGN    'l'
#define NW_DISCOVERY_STOPPING_SIGN 'z'
#define NW_DISCOVERY_SIGN_TO_STATE(sign)                         \
    (sign == NW_DISCOVERY_STARTING_SIGN ? "<Starting>" :         \
        (sign == NW_DISCOVERY_ALIVE_SIGN ? "<Alive>" :           \
            (sign == NW_DISCOVERY_STOPPING_SIGN ? "<Stopping>" : \
                "<Invalid>")))


/* Important note: make sure that this message has the same memory layout on all
 *                 supported platforms. endianness may differ but alignment
 *                 needs to match */
NW_STRUCT(nw_discoveryMessage) {
    v_networkId networkId;
    c_time heartbeatInterval;
    c_char sign;
};
NW_CLASS(nw_discoveryMessage);

#define NW_DISCOVERY_MESSAGE_SIZE ((unsigned int)sizeof(NW_STRUCT(nw_discoveryMessage)))

#define NW_HEARTBEATINTERVAL_TO_SLEEPTIME(time, interval, factor) \
    time.seconds = (c_long)((factor) * (interval)) / 1000U;                \
    time.nanoseconds = (1000000U * ((c_ulong)((factor) * (interval)) % 1000U))

static void *
nw_discoveryWriterMain(
    nw_runnable runnable,
    c_voidp arg)
{
    nw_discoveryWriter discoveryWriter = (nw_discoveryWriter)runnable;
    c_bool terminationRequested;
    unsigned int heartbeatInterval;
    float safetyFactor;
    unsigned int salvoSize;
    unsigned int respondCount;
    static c_time regularSleepTime;
    static c_time startStopSleepTime;
    static c_time heartbeatTime;
    NW_STRUCT(nw_discoveryMessage) discoveryMessage;
    unsigned int i;

    c_char* path;

    nw_runnableSetRunState(runnable, rsRunning);

    /* several parameters */
    path = os_malloc(strlen(runnable->name) + strlen(NWCF_SEP) + strlen(NWCF_ROOT(Tx)) + 1);
    sprintf(path, "%s%s%s", runnable->name, NWCF_SEP, NWCF_ROOT(Tx));
    heartbeatInterval = NWCF_SIMPLE_PARAM(ULong, path, Interval);
    if (heartbeatInterval < NWCF_MIN(Interval)) {
        NW_REPORT_WARNING_2("retrieving discovery sending parameters",
            "specified Interval %u too small, "
            "switching to %u",
            heartbeatInterval, NWCF_MIN(Interval));
        heartbeatInterval = NWCF_MIN(Interval);
    }

    safetyFactor = NWCF_SIMPLE_PARAM(Float, path, SafetyFactor);
    if (safetyFactor < NWCF_MIN(SafetyFactor)) {
        NW_REPORT_WARNING_2("retrieving discovery sending parameters",
            "specified SafetyFactor %f too small, "
            "switching to %f",
            safetyFactor, NWCF_MIN(SafetyFactor));
        safetyFactor = NWCF_MIN(SafetyFactor);
    }

    salvoSize = NWCF_SIMPLE_PARAM(ULong, path, SalvoSize);
    if (salvoSize < NWCF_MIN(SalvoSize)) {
        NW_REPORT_WARNING_2("retrieving discovery sending parameters",
            "specified SalvoSize %u too small, "
            "switching to %u",
            salvoSize, NWCF_MIN(SalvoSize));
        salvoSize = NWCF_MIN(SalvoSize);
    }

    os_free(path);

    NW_HEARTBEATINTERVAL_TO_SLEEPTIME(regularSleepTime, heartbeatInterval, safetyFactor);
    NW_HEARTBEATINTERVAL_TO_SLEEPTIME(startStopSleepTime, heartbeatInterval, (0.1*safetyFactor));

    discoveryMessage.networkId = nw_plugHostToNetwork(discoveryWriter->networkId);
    NW_HEARTBEATINTERVAL_TO_SLEEPTIME(heartbeatTime, heartbeatInterval, 1.0);
    discoveryMessage.heartbeatInterval.seconds = nw_plugHostToNetwork(heartbeatTime.seconds);
    discoveryMessage.heartbeatInterval.nanoseconds = nw_plugHostToNetwork(heartbeatTime.nanoseconds);

    c_mutexLock(&discoveryWriter->mutex);

    /* First give a salvo of starting messages so everybody knows about me */
    discoveryMessage.sign = NW_DISCOVERY_STARTING_SIGN;
    NW_TRACE_1(Discovery, 2, "Liveliness state set to %s",
        NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));
    terminationRequested = (int)nw_runnableTerminationRequested(runnable);
    for (i=0; (i<salvoSize) && (!terminationRequested); i++) {
        nw_discoveryWriterWriteMessageToNetwork(discoveryWriter,
            (nw_data)&discoveryMessage, NW_DISCOVERY_MESSAGE_SIZE);
        c_condTimedWait(&discoveryWriter->condition, &discoveryWriter->mutex,
            startStopSleepTime);
        terminationRequested = (int)nw_runnableTerminationRequested(runnable);
    }

    /* Now go into regular state */
    discoveryMessage.sign = NW_DISCOVERY_ALIVE_SIGN;
    respondCount = 0;
    NW_TRACE_1(Discovery, 2, "Liveliness state set to %s",
        NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));
    while (!terminationRequested) {
        nw_discoveryWriterWriteMessageToNetwork(discoveryWriter,
            (nw_data)&discoveryMessage, NW_DISCOVERY_MESSAGE_SIZE);
        if (discoveryWriter->respondToNode) {
            respondCount = salvoSize;
            discoveryWriter->respondToNode = FALSE;
        }

        if (respondCount == 0) {
            c_condTimedWait(&discoveryWriter->condition, &discoveryWriter->mutex,
                regularSleepTime);
        } else {
            /* Somebody is currently starting so respond quickly with a alive salvo */
            respondCount--;
            c_condTimedWait(&discoveryWriter->condition, &discoveryWriter->mutex,
                startStopSleepTime);
        }
        /* Check if we have to stop */
        terminationRequested = (int)nw_runnableTerminationRequested(runnable);
    }

    /* Send a salvo of stopping messages so everybody can react quickly */
    discoveryMessage.sign = NW_DISCOVERY_STOPPING_SIGN;
    NW_TRACE_1(Discovery, 2, "Liveliness state set to %s",
        NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));
    for (i=0; i<salvoSize; i++) {
        nw_discoveryWriterWriteMessageToNetwork(discoveryWriter,
            (nw_data)&discoveryMessage, NW_DISCOVERY_MESSAGE_SIZE);
        c_condTimedWait(&discoveryWriter->condition, &discoveryWriter->mutex,
            startStopSleepTime);
        terminationRequested = (int)nw_runnableTerminationRequested(runnable);
    }

    nw_runnableSetRunState(runnable, rsTerminated);
    c_mutexUnlock(&discoveryWriter->mutex);

    return NULL;
}

nw_discoveryWriter
nw_discoveryWriterNew(
    v_networkId networkId,
    const char *name)
{
    nw_discoveryWriter result;
    size_t schedPathNameSize;
    c_char * schedPath;

    result = (nw_discoveryWriter)os_malloc(sizeof(*result));
    if (result != NULL) {
    	schedPathNameSize = strlen(name) + strlen(NWCF_SEP) + strlen(NWCF_ROOT(Tx)) +
                + strlen(NWCF_SEP) + strlen(NWCF_ROOT(Scheduling)) + 1 /* '\0' */;
        schedPath = os_malloc(schedPathNameSize);
        snprintf(schedPath, schedPathNameSize, "%s%s%s%s%s", name, NWCF_SEP,
            NWCF_ROOT(Tx), NWCF_SEP, NWCF_ROOT(Scheduling));
        /* Initialize parent */
        nw_discoveryInitialize((nw_discovery)result, networkId,
            name, schedPath,
            nw_discoveryWriterMain,     /* my own main func */
            nw_discoveryWriterTrigger,  /* my trigger function to wake up */
            nw_discoveryFinalize);      /* my own finalize func */
        os_free(schedPath);
        /* Initialize self */
        result->networkId = networkId;
        result->sendChannel = nw_plugNetworkNewSendChannel(
            ((nw_discovery)result)->network, name,NULL,NULL);
        result->respondToNode = FALSE;
        c_mutexInit(&result->mutex, PRIVATE_MUTEX);
        c_condInit(&result->condition, &result->mutex, PRIVATE_COND);
        /* Do not start in the constructor, but let somebody else start me */
	}

	return result;
}

void
nw_discoveryWriterRespondToStartingNode(
    nw_discoveryWriter discoveryWriter)
{
    c_mutexLock(&discoveryWriter->mutex);
    discoveryWriter->respondToNode = TRUE;
    c_condBroadcast(&discoveryWriter->condition);
    c_mutexUnlock(&discoveryWriter->mutex);
}


static void
nw_discoveryWriterTrigger(
    nw_runnable runnable)
{
    nw_discoveryWriter discoveryWriter = (nw_discoveryWriter)runnable;

    if (discoveryWriter != NULL) {
        /* Wake up the writer from its blocking action */
        c_mutexLock(&discoveryWriter->mutex);
        c_condBroadcast(&discoveryWriter->condition);
        c_mutexUnlock(&discoveryWriter->mutex);
    }
}

/* --------------------- discoveryReader (concrete) ------------------------- */

static void nw_discoveryReaderFinalize(nw_runnable runnable);

NW_CLASS(nw_aliveNodesHashItem);
NW_STRUCT(nw_aliveNodesHashItem) {
    v_networkId networkId;
    nw_address address;
    c_time maxHeartbeatIntervalAllowed;
    c_time lastHeartbeatReceivedTime;
    nw_bool hasDied;
    nw_aliveNodesHashItem next;
};


NW_STRUCT(nw_discoveryReader) {
    NW_EXTENDS(nw_discovery);
    /* Action routines and parameter */
    nw_discoveryAction startedAction;
    nw_discoveryAction stoppedAction;
    nw_discoveryAction diedAction;
    nw_discoveryMsgArg arg;
    /* Discovery behaviour */
    nw_seqNr deathDetectionCount;
    /* Keep administration of alive nodes */
    nw_seqNr aliveNodesCount;
    nw_seqNr diedNodesCount;
    nw_seqNr aliveNodesHashSize;
    nw_aliveNodesHashItem *aliveNodesHash;
    /* Request for alive-check has been submitted */
    nw_bool checkRequested;
    /* Immediate interaction with the networking plug */
    nw_plugChannel receiveChannel;
};

static void
nw_discoveryReaderTrigger(
    nw_runnable runnable)
{
    nw_discoveryReader discoveryReader = (nw_discoveryReader)runnable;

	/* Wake up the reader from its blocking action */
	nw_plugReceiveChannelWakeUp(discoveryReader->receiveChannel);
}

#define NW_ALIVENODESHASH_HASHVALUE(reader, value) \
    ((value) % (reader)->aliveNodesHashSize)
#define NW_ALIVENODESHASH_INDEXISVALID(reader, index) \
    (index < (reader)->alivedNodesHashSize)
#define NW_ALIVENODESHASH_ITEMBYINDEX(reader, index) \
    (reader)->aliveNodesHash[(index) % (reader)->aliveNodesHashSize]
#define NW_ALIVENODESHASH_ITEMBYVALUE(reader, value) \
    NW_ALIVENODESHASH_ITEMBYINDEX(reader, NW_ALIVENODESHASH_HASHVALUE(reader, value))


static void
nw_discoveryReaderRemoveNode(
    nw_discoveryReader discoveryReader,
    v_networkId networkId,
    nw_address address,
    nw_aliveNodesHashItem *itemFound)
{
    nw_aliveNodesHashItem currentItem;
    nw_aliveNodesHashItem *currentItemPtr;
    nw_bool found = FALSE;
    nw_bool done = FALSE;

    *itemFound = NULL;
    currentItemPtr = &(NW_ALIVENODESHASH_ITEMBYVALUE(discoveryReader, networkId));
    currentItem = *currentItemPtr;
    while ((currentItem != NULL) && (!found) && (!done)) {
        if (currentItem->networkId < networkId) {
            currentItemPtr = &(currentItem->next);
            currentItem = *currentItemPtr;
        } else if (currentItem->networkId == networkId) {
            found = TRUE;
            NW_CONFIDENCE(currentItem->address == address);
            *itemFound = currentItem;
            *currentItemPtr = currentItem->next;
            currentItem->next = NULL;
        } else {
            /* Not found and will never find because the list is ordered */
            done = TRUE;
        }
    }
}


static void
nw_discoveryReaderLookupOrCreateNode(
    nw_discoveryReader discoveryReader,
    v_networkId networkId,
    nw_address address,
    nw_aliveNodesHashItem *item,
    nw_bool *wasCreated)
{
    nw_aliveNodesHashItem currentItem;
    nw_aliveNodesHashItem *currentItemPtr;
    nw_aliveNodesHashItem newItem;
    nw_bool found = FALSE;
    nw_bool insertionNeeded = FALSE;

    currentItemPtr = &(NW_ALIVENODESHASH_ITEMBYVALUE(discoveryReader, networkId));
    currentItem = *currentItemPtr;
    while ((currentItem != NULL) && (!found) && (!insertionNeeded)) {
        if (currentItem->networkId < networkId) {
            currentItemPtr = &(currentItem->next);
            currentItem = *currentItemPtr;
        } else if (currentItem->networkId == networkId) {
            found = TRUE;
            NW_CONFIDENCE(currentItem->address == address);
        } else {
            insertionNeeded = TRUE;
        }
    }
    if (!found) {
        newItem = (nw_aliveNodesHashItem)os_malloc(sizeof(*newItem));
        newItem->networkId = networkId;
        newItem->address = address;
        newItem->next = currentItem;
        *currentItemPtr = newItem;
    }

    *item = *currentItemPtr;
    *wasCreated = !found;
}

static void
nw_aliveNodesHashItemFree(
    nw_aliveNodesHashItem hashItem)
{
    NW_CONFIDENCE(hashItem != NULL);

    os_free(hashItem);
}

static void
nw_discoveryReaderReceivedHeartbeat(
    nw_discoveryReader discoveryReader,
    nw_data messageBuffer,
    nw_length bufferLength,
    nw_address address,
    c_time receivedTime)
{
    NW_STRUCT(nw_discoveryMessage) discoveryMessage;
    v_networkId networkId;
    c_time heartbeatInterval;
    c_time diffTime;
    c_equality eq;
    nw_aliveNodesHashItem itemFound;
    nw_bool wasCreated;
    unsigned int i;

    NW_CONFIDENCE(messageBuffer != NULL);
    NW_CONFIDENCE(bufferLength == NW_DISCOVERY_MESSAGE_SIZE);

    memcpy(&discoveryMessage, messageBuffer, NW_DISCOVERY_MESSAGE_SIZE);
    networkId = nw_plugNetworkToHost(discoveryMessage.networkId);
    NW_TRACE_2(Discovery, 6, "Received heartbeat from node with id 0x%x, "
        "state is %s", networkId, NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));
    switch (discoveryMessage.sign) {
        case NW_DISCOVERY_STOPPING_SIGN:
            nw_discoveryReaderRemoveNode(discoveryReader, networkId, address,
                &itemFound);
            if (itemFound != NULL) {
                /* Take action if node has not died before */
                if (!itemFound->hasDied) {
                    discoveryReader->aliveNodesCount--;
                    NW_TRACE_1(Discovery, 2, "Removed alive node 0x%x because it has stoppend", networkId);
                    if (discoveryReader->stoppedAction != NULL) {
                        discoveryReader->stoppedAction(networkId, address,
                            receivedTime, discoveryReader->aliveNodesCount,
                            discoveryReader->arg);
                    }
                } else {
                    discoveryReader->diedNodesCount--;
                    NW_TRACE_1(Discovery, 2, "Removed dead node 0x%x because it has stopped", networkId);
                }
                nw_aliveNodesHashItemFree(itemFound);
            }
            break;
        case NW_DISCOVERY_STARTING_SIGN:
        case NW_DISCOVERY_ALIVE_SIGN:
            heartbeatInterval.seconds = nw_plugNetworkToHost(discoveryMessage.heartbeatInterval.seconds);
            heartbeatInterval.nanoseconds = nw_plugNetworkToHost(discoveryMessage.heartbeatInterval.nanoseconds);
            nw_discoveryReaderLookupOrCreateNode(discoveryReader, networkId,
                address, &itemFound, &wasCreated);
            NW_CONFIDENCE(itemFound != NULL);
            if (wasCreated) {
                /* New item, initialize it */
                itemFound->maxHeartbeatIntervalAllowed = heartbeatInterval;
                for (i=1; i<discoveryReader->deathDetectionCount; i++) {
                    itemFound->maxHeartbeatIntervalAllowed =
                        c_timeAdd(itemFound->maxHeartbeatIntervalAllowed,
                                  heartbeatInterval);
                }
                itemFound->lastHeartbeatReceivedTime = receivedTime;
                itemFound->hasDied = FALSE;
                discoveryReader->aliveNodesCount++;
                NW_TRACE_1(Discovery, 2, "New node detected with id 0x%x", networkId);
                NW_TRACE_2(Discovery, 3, "Node has heartbeatInterval %d.%3.3u",
                    heartbeatInterval.seconds, heartbeatInterval.nanoseconds/1000000U);
                NW_TRACE_2(Discovery, 3, "Node has maxInterval %d.%3.3u",
                    itemFound->maxHeartbeatIntervalAllowed.seconds,
                    itemFound->maxHeartbeatIntervalAllowed.nanoseconds/1000000U);
                if (discoveryReader->startedAction != NULL) {
                    discoveryReader->startedAction(networkId, address,
                        receivedTime, discoveryReader->aliveNodesCount,
                        discoveryReader->arg);
                }
            } else {
                if (!itemFound->hasDied) {
                    diffTime = c_timeSub(receivedTime,
                        itemFound->lastHeartbeatReceivedTime);
                    eq = c_timeCompare(diffTime, itemFound->maxHeartbeatIntervalAllowed);
                    if (eq == C_GT) {
                        /* It has taken too long for this node to send a heartbeat */
                        itemFound->hasDied = TRUE;
                        discoveryReader->aliveNodesCount--;
                        discoveryReader->diedNodesCount++;
                        NW_TRACE_1(Discovery, 2, "Declared node 0x%x dead; "
                            "heartbeat received but too late", networkId);
                        if (discoveryReader->diedAction != NULL) {
                            discoveryReader->diedAction(networkId, address,
                                receivedTime, discoveryReader->aliveNodesCount,
                                discoveryReader->arg);
                        }
                    } else {
                        itemFound->lastHeartbeatReceivedTime = receivedTime;
                    }
                } else {
                    NW_TRACE_1(Discovery, 4, "Ignoring message from dead node 0x%x", networkId);
                }
            }
            break;
        default:
            NW_CONFIDENCE(FALSE);
            break;
    }
}


static void
nw_discoveryReaderCheckForDiedNodes(
    nw_discoveryReader discoveryReader,
    c_time checkTime)
{
    nw_seqNr i;
    nw_aliveNodesHashItem hashItem;
    c_time diffTime;
    c_equality eq;

    NW_TRACE(Discovery, 5, "Checking liveliness of nodes");
    for (i=0; i<discoveryReader->aliveNodesHashSize; i++) {
        hashItem = NW_ALIVENODESHASH_ITEMBYINDEX(discoveryReader, i);
        while (hashItem != NULL) {
            if (!hashItem->hasDied) {
                diffTime = c_timeSub(checkTime, hashItem->lastHeartbeatReceivedTime);
                NW_TRACE_3(Discovery, 5, "No heartbeat received from alive node 0x%x  during interval %d.%3.3u",
                    hashItem->networkId, diffTime.seconds, diffTime.nanoseconds/1000000U);
                eq = c_timeCompare(diffTime, hashItem->maxHeartbeatIntervalAllowed);
                if (eq == C_GT) {
                    /* It has taken too long for this node to send a heartbeat */
                    hashItem->hasDied = TRUE;
                    discoveryReader->aliveNodesCount--;
                    discoveryReader->diedNodesCount++;
                    NW_TRACE_3(Discovery, 2, "Declared node 0x%x dead, "
                        "no heartbeat received during interval %d.%3.3u", hashItem->networkId,
                        diffTime.seconds, diffTime.nanoseconds/1000000);
                    if (discoveryReader->diedAction != NULL) {
                        discoveryReader->diedAction(hashItem->networkId,
                            hashItem->address, checkTime,
                            discoveryReader->aliveNodesCount,
                            discoveryReader->arg);
                    }
                }
            } else {
                NW_TRACE_1(Discovery, 5, "Ignoring dead node 0x%x", hashItem->networkId);
            }
            hashItem = hashItem->next;
        }
    }
}


static void *
nw_discoveryReaderMain(
    nw_runnable runnable,
    c_voidp arg)
{
    nw_discoveryReader discoveryReader = (nw_discoveryReader)runnable;
    c_bool terminationRequested;
    nw_data messageBuffer;
    nw_length bufferLength;
    nw_address senderAddress;
    c_time now;

    nw_runnableSetRunState(runnable, rsRunning);

    terminationRequested = (int)nw_runnableTerminationRequested(runnable);
    while (!terminationRequested) {
    	/* First retrieve a buffer to write in */
        nw_plugReceiveChannelMessageStart(discoveryReader->receiveChannel,
            &messageBuffer, &bufferLength, &senderAddress);
        if (messageBuffer != NULL) {
            nw_plugReceiveChannelMessageEnd(discoveryReader->receiveChannel);
        }

        /* Check if we have to stop */
        terminationRequested = (int)nw_runnableTerminationRequested(runnable);
        if (!terminationRequested) {
            now = v_timeGet();
            if (messageBuffer != NULL) {
                /* Determine the contents of the message and update the admin */
                nw_discoveryReaderReceivedHeartbeat(discoveryReader, messageBuffer,
                    bufferLength, senderAddress, now);
            }
            if (discoveryReader->checkRequested) {
                nw_discoveryReaderCheckForDiedNodes(discoveryReader, now);
                discoveryReader->checkRequested = FALSE;
            }
        }
    }

    nw_runnableSetRunState(runnable, rsTerminated);

    return NULL;
}

#undef NW_MESSAGE
#undef NW_MESSAGE_SIZE

#define NW_ALIVENODESHASH_SIZE        (256)

nw_discoveryReader
nw_discoveryReaderNew(
    v_networkId networkId,
    const char *name,
    nw_discoveryAction startedAction,
    nw_discoveryAction stoppedAction,
    nw_discoveryAction diedAction,
    nw_discoveryMsgArg arg)
{
    nw_discoveryReader result = NULL;
    nw_seqNr i;
    char *pathName;
    char *schedPathName;
    size_t schedPathNameSize;

    result = (nw_discoveryReader)os_malloc((os_uint32)sizeof(*result));
    if (result != NULL) {
        /* Initialize parent */
        /* First determine its parameter path */
        schedPathNameSize = strlen(name) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(Rx)) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(Scheduling))+ 1;
        schedPathName = os_malloc(schedPathNameSize);
        sprintf(schedPathName, "%s%s%s%s%s", name, NWCF_SEP, NWCF_ROOT(Rx),
            NWCF_SEP, NWCF_ROOT(Scheduling));
        nw_discoveryInitialize((nw_discovery)result, networkId,
            name, schedPathName,
            nw_discoveryReaderMain,      /* my own main */
            nw_discoveryReaderTrigger,   /* my own trigger */
            nw_discoveryReaderFinalize); /* my own finalize */
        os_free(schedPathName);

        /* Initialize myself */
        /* Set the private members */
        result->startedAction = startedAction;
        result->stoppedAction = stoppedAction;
        result->diedAction = diedAction;
        result->arg = arg;
        result->checkRequested = FALSE;
        result->receiveChannel = nw_plugNetworkNewReceiveChannel(
            ((nw_discovery)result)->network, name, NULL,NULL);


        pathName = os_malloc(strlen(name) + strlen(NWCF_SEP) + strlen(NWCF_ROOT(Rx)) + 1) ;
        sprintf(pathName, "%s%s%s", name, NWCF_SEP, NWCF_ROOT(Rx));

        /* death detection of other nodes */
        result->deathDetectionCount = NWCF_SIMPLE_PARAM(ULong, pathName, DeathDetectionCount);
        if (result->deathDetectionCount < NWCF_MIN(DeathDetectionCount)) {
            NW_REPORT_WARNING_2("nw_discoveryReaderNew",
                "specified DeathDetectionCount %u too small, "
                "switching to %u",
                result->deathDetectionCount, NWCF_MIN(DeathDetectionCount));
            result->deathDetectionCount = NWCF_MIN(DeathDetectionCount);
        }
        os_free(pathName);
        /* node admin */
        result->aliveNodesCount = 0;
        result->diedNodesCount = 0;
        result->aliveNodesHashSize = NW_ALIVENODESHASH_SIZE;
        result->aliveNodesHash =
            (nw_aliveNodesHashItem *)os_malloc(result->aliveNodesHashSize *
                sizeof(*result->aliveNodesHash));
        for (i=0; i<result->aliveNodesHashSize; i++) {
            NW_ALIVENODESHASH_ITEMBYINDEX(result, i) = NULL;
        }
        /* Do not start in the constructor, but let somebody else start me */
    }

    return result;
}

static void
nw_discoveryReaderFinalize(
    nw_runnable runnable)
{
    nw_discoveryReader discoveryReader;
    nw_aliveNodesHashItem *hashItemPtr;
    nw_aliveNodesHashItem toFree;
    nw_seqNr i;

    discoveryReader = (nw_discoveryReader)runnable;
    NW_CONFIDENCE(discoveryReader != NULL);

    /* Finalize self */

    for (i=0; i<discoveryReader->aliveNodesHashSize; i++) {
        hashItemPtr = &(NW_ALIVENODESHASH_ITEMBYINDEX(discoveryReader, i));
        while (*hashItemPtr != NULL) {
            toFree = *hashItemPtr;
            *hashItemPtr = toFree->next;
            nw_aliveNodesHashItemFree(toFree);
        }
    }
    os_free(discoveryReader->aliveNodesHash);

    /* Finalize parent */
    nw_discoveryFinalize(runnable);
}

void
nw_discoveryReaderInitiateCheck(
    nw_discoveryReader discoveryReader)
{
    discoveryReader->checkRequested = TRUE;
    NW_TRACE(Discovery, 5, "Triggering discoveryReader for liveliness checking");
    nw_discoveryReaderTrigger((nw_runnable)discoveryReader);
}

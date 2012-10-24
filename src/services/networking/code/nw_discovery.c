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
#include "nw_discovery.h"

/* Implementation */
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_socket.h"
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
#include "nw_stringList.h"
#include "nw_bridge.h" /* for v_gidToGlobalId */
#include "nw_report.h"
#include "kernelModule.h"
#include "u_networkReader.h"
#include "v_networkReader.h"


NW_CLASS(nw_discovery);
/**
* @extends nw_runnable
*/
NW_STRUCT(nw_discovery) {
    NW_EXTENDS(nw_runnable);
    nw_plugNetwork network;
};


typedef enum nw_discoveryState_e {
    NW_DISCSTATE_KNOWN,
    NW_DISCSTATE_ALIVE,
    NW_DISCSTATE_FULL
} nw_discoveryStateType;

NW_CLASS(nw_aliveNodesHashItem);
NW_STRUCT(nw_aliveNodesHashItem) {
    v_networkId networkId;
    os_sockaddr_storage address;
    nw_name role;
    nw_discoveryStateType state;
    c_time maxHeartbeatIntervalAllowed;
    c_time lastHeartbeatReceivedTime;
    nw_bool hasDied;
    nw_aliveNodesHashItem next;
};


NW_CLASS(nw_nodeListItem);
NW_STRUCT(nw_nodeListItem) {
    v_networkId networkId;
    os_sockaddr_storage address;
    nw_nodeListItem next;
};

static nw_nodeListItem
nw_nodeListAdd(nw_nodeListItem next, v_networkId networkId, os_sockaddr_storage address)
{
    nw_nodeListItem result = os_malloc(sizeof(*result));
    if ( result ) {
        result->networkId = networkId;
        result->address = address;
        result->next = next;
    }

    return result;
}

/* --------------------- helper class: nw_messageBox ------------------------ */
typedef enum nw_discMessageType_e {
    NW_DISCBOX_DOREQ,
    NW_DISCBOX_SENDLIST,
    NW_DISCBOX_RESPOND
} nw_discMessageType;


NW_CLASS(nw_discMessage);
NW_STRUCT(nw_discMessage) {
    nw_networkId networkId ;
    os_sockaddr_storage address ;
    nw_name expression;
    nw_nodeListItem nodeList;
    nw_discMessage next;
    nw_discMessageType messageType;
};

NW_CLASS(nw_discMessageBox);
NW_STRUCT(nw_discMessageBox) {
    c_mutex mutex;
    /* fifo queue */
    nw_discMessage firstMessage;
    nw_discMessage lastMessage;
};

static nw_discMessageBox
nw_messageBoxNew(
    void)
{
    nw_discMessageBox result;

    result = (nw_discMessageBox)os_malloc(sizeof(*result));
    if (result != NULL) {
        c_mutexInit(&result->mutex, PRIVATE_MUTEX);
        result->firstMessage = NULL;
        result->lastMessage = NULL;
    }
    return result;
}

static void
nw_messageBoxFree(
    nw_discMessageBox messageBox)
{
    nw_discMessage toFree;

    if (messageBox != NULL) {
        while (messageBox->firstMessage != NULL) {
            toFree = messageBox->firstMessage;
            messageBox->firstMessage = toFree->next;
            os_free(toFree);
        }
        os_free(messageBox);
    }
}

static void
nw_messageBoxPushMessage(
    nw_discMessageBox messageBox,
    nw_networkId networkId,
    os_sockaddr_storage address,
    nw_name expression,
    nw_nodeListItem nodeList,
    nw_discMessageType messageType)
{
    nw_discMessage newMessage;
    nw_discMessage *prevNextPtr;

    c_mutexLock(&messageBox->mutex);
    newMessage = (nw_discMessage)os_malloc(sizeof(*newMessage));
    newMessage->networkId = networkId; /* transfer ownership */
    newMessage->address = address;
    newMessage->expression = expression; /* transfer ownership */
    newMessage->messageType = messageType;
    newMessage->nodeList = nodeList; /* transfer ownership */
    if (messageBox->firstMessage == NULL) {
        prevNextPtr = &(messageBox->firstMessage);
    } else {
        prevNextPtr = &(messageBox->lastMessage->next);
    }
    newMessage->next = NULL;
    *prevNextPtr = newMessage;
    messageBox->lastMessage = newMessage;
    c_mutexUnlock(&(messageBox->mutex));
}

static nw_discMessage
nw_messageBoxPopMessage(
    nw_discMessageBox messageBox)
{
    nw_discMessage result = NULL;

    c_mutexLock(&(messageBox->mutex));
    if (messageBox->firstMessage != NULL) {
        result = messageBox->firstMessage;
        messageBox->firstMessage = result->next;
        if (messageBox->firstMessage == NULL) {
            messageBox->lastMessage = NULL;
        }
    }
    c_mutexUnlock(&(messageBox->mutex));

    return result;
}



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

static nw_bool
nw_rolescopeMatching(
    c_string str,
    c_string pattern )

{
    c_bool   stop = FALSE;
    nw_bool  match =  TRUE;
    c_string strRef = NULL;
    c_string patternRef = NULL;

    while ((*str != 0) && (*pattern != 0) && (stop == FALSE)) {
        if (*pattern == '*') {
            pattern++;
            while ((*str != 0) && (*str != *pattern)) {
                str++;
            }
            if (*str != 0) {
                strRef = str+1; /* just behind the matching char */
                patternRef = pattern-1; /* on the '*' */
            }
        } else if (*pattern == '?') {
            pattern++;
            str++;
        } else if (*pattern++ != *str++) {
            if (strRef == NULL) {
                match = FALSE;
                stop = TRUE;
            } else {
                str = strRef;
                pattern = patternRef;
                strRef = NULL;
            }
        }
    }
    if ((*str == (char)0) && (stop == FALSE)) {
        while (*pattern == '*') {
            pattern++;
        }
        if (*pattern != (char)0) {
             match = FALSE;
        }
    } else {
        match = FALSE;
    }
    /*
    NW_REPORT_WARNING_3("match result",
                "Pattern: %s String: %s Match: %d",
                t2, t1, match);*/
    return match;
}


/* Role matches the scope if the Role is a wildcard match with
   any of the comma seperated parts of the scope expression  */
static nw_bool
nw_RoleScopeMatch(
    c_string Role,
    nw_stringList scopeList)
{
    const char *partscope = "";
    unsigned int size, i;
    nw_bool result = FALSE;

    size = nw_stringListGetSize(scopeList);
    i = 0;
    while ( i<size && !result ) {
        partscope = nw_stringListGetValue(scopeList, i);
        result = nw_rolescopeMatching(Role,(c_string)partscope);
        i++;
    }
    NW_TRACE_3(
        Discovery, 6,
        "nw_RoleScopeMatch: Role %s, partscope %s, match %u ",
        Role,partscope,result);


    return result;
}


/* --------------------- discoveryWriter (concrete) ------------------------- */

static void nw_discoveryWriterTrigger(nw_runnable runnable);
/**
* @extends nw_discovery
*/
NW_STRUCT(nw_discoveryWriter) {
    NW_EXTENDS(nw_discovery);
    /* NetworkId of this network */
    v_networkId networkId;
    /* Immediate interaction with the networking plug */
    nw_plugChannel sendChannel;
    /* trigger for responding to new nodes */
    nw_bool respondToNode;
    nw_name role;
    nw_name scope;
    nw_name probeList;
    nw_discMessageBox mbox;
    /* Lock and condVar for inter-thread communication */
    c_mutex mutex;
    c_cond condition;
};

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

#define NW_DISCOVERY_MESSAGE_SIZE \
        ((os_uint)sizeof(NW_STRUCT(nw_discoveryMessage)))

#define NW_HEARTBEATINTERVAL_TO_SLEEPTIME(time, interval, factor) \
        time.seconds = (c_long)((factor) * (interval)) / 1000U;                \
        time.nanoseconds = (1000000U * ((c_ulong)((factor) * (interval)) % 1000U))


/* tags used for the optional parts of the discovery messages:
 * These are encode as:
 * a byte representing the tag
 * a byte representing the length os the content
 * the indicated number of bytes worth of content
 */
#define NW_DISCTAG_SENTINEL     (0)
#define NW_DISCTAG_ROLE         (1)
#define NW_DISCTAG_REQ          (2)
#define NW_DISCTAG_IPV4LIST     (3)
#define NW_DISCTAG_IPV6LIST     (4)

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
    nw_length len;

    NW_TRACE_1(Discovery, 6, "Sending heartbeat to the network with role \"%s\"",discoveryWriter->role);
    /* First retrieve a buffer to write in */
    maxBytes = 0; /* don't flush. */
    result = nw_plugSendChannelMessageStart(discoveryWriter->sendChannel,
                                            &messageBuffer,
                                            &bufferLength,
                                            0,
                                            &maxBytes,
                                            NULL);
    if (result) {
        /* Copy the message into the buffer */
        NW_CONFIDENCE(bufferLength >= length);
        memcpy((void *)messageBuffer, message, length);
        /* Update buffer position and size */
        messageBuffer = &(messageBuffer[length]);
        bufferLength -= length;
        /* insert role item in message */
        len = strlen(discoveryWriter->role);
        NW_CONFIDENCE(bufferLength >= (len+3));
        *(messageBuffer++) = NW_DISCTAG_ROLE;
        *(messageBuffer++) = (os_uchar)len/256;
        *(messageBuffer++) = (os_uchar)(len&0xff);
        memcpy(messageBuffer,discoveryWriter->role,len);
        messageBuffer += len;
        bufferLength -= (len+3);
        *(messageBuffer++) = NW_DISCTAG_SENTINEL;
        bufferLength -= 1;

        /* Do write and flush immediately */
        nw_plugSendChannelMessageEnd(discoveryWriter->sendChannel, messageBuffer, NULL);
        maxBytes = bufferLength;
        result = nw_plugSendChannelMessagesFlush(discoveryWriter->sendChannel,
                                                 TRUE, &maxBytes, NULL);
    }
    NW_CONFIDENCE(result);
}


static void
nw_discoveryWriterWriteReqToDestination(
    nw_discoveryWriter discoveryWriter,
    nw_data message,
    nw_length length,
    nw_networkId destination,
    nw_name expression
    )
{
    nw_data messageBuffer;
    nw_length bufferLength;
    nw_signedLength maxBytes;
    nw_bool result;
    nw_length len;

    NW_TRACE_2(Discovery, 2, "Sending discoveryrequest \"%s\" to node 0x%x", expression, destination);
    /* First retrieve a buffer to write in */
    maxBytes = 0; /* don't flush. */
    result = nw_plugSendChannelWriteToMessageStart(discoveryWriter->sendChannel,
                                            &messageBuffer,
                                            &bufferLength,
                                            0,
                                            destination,
                                            &maxBytes,
                                            NULL);
    if (result) {
        /* Copy the message into the buffer */
        NW_CONFIDENCE(bufferLength >= length);
        memcpy((void *)messageBuffer, message, length);
        /* Update buffer position and size */
        messageBuffer = &(messageBuffer[length]);
        bufferLength -= length;
        /* insert role item in message */
        len = strlen(discoveryWriter->role);
        NW_CONFIDENCE(bufferLength >= (len+3));
        *(messageBuffer++) = NW_DISCTAG_ROLE;
        *(messageBuffer++) = (os_uchar)len/256;
        *(messageBuffer++) = (os_uchar)(len&0xff);
        memcpy(messageBuffer,discoveryWriter->role,len);
        messageBuffer += len;
        bufferLength -= (len+3);
        /* insert req item in message */
        len = strlen(expression);
        NW_CONFIDENCE(bufferLength >= (len+3));
        *(messageBuffer++) = NW_DISCTAG_REQ;
        *(messageBuffer++) = (os_uchar)len/256;
        *(messageBuffer++) = (os_uchar)(len&0xff);
        memcpy(messageBuffer,expression,len);
        messageBuffer += len;
        bufferLength -= (len+3);
        *(messageBuffer++) = NW_DISCTAG_SENTINEL;
        bufferLength -= 1;

        /* Do write and flush immediately */
        nw_plugSendChannelMessageEnd(discoveryWriter->sendChannel, messageBuffer, NULL);
        maxBytes = bufferLength;
        result = nw_plugSendChannelMessagesFlush(discoveryWriter->sendChannel,
                                                 TRUE, &maxBytes, NULL);
    }
    NW_CONFIDENCE(result);
}

static void
nw_discoveryWriterWriteNodelistToDestination(
    nw_discoveryWriter discoveryWriter,
    nw_data message,
    nw_length length,
    nw_networkId destination,
    nw_nodeListItem NodeList)
{
    nw_data messageBuffer;
    nw_length bufferLength;
    nw_signedLength maxBytes;
    nw_bool result;
    nw_data len_field;
    os_uint32 v4Address;
    nw_networkId hostId;
    nw_length len;
    nw_nodeListItem oldItem;
    char addressStr[INET6_ADDRSTRLEN];

    NW_TRACE_1(Discovery, 2, "Sending matching nodelist to node 0x%x", destination);
    /* First retrieve a buffer to write in */
    maxBytes = 0; /* don't flush. */
    result = nw_plugSendChannelWriteToMessageStart(discoveryWriter->sendChannel,
                                            &messageBuffer,
                                            &bufferLength,
                                            0,
                                            destination,
                                            &maxBytes,
                                            NULL);
    if (result) {
        /* Copy the message into the buffer */
        NW_CONFIDENCE(bufferLength >= length);
        memcpy((void *)messageBuffer, message, length);
        /* Update buffer position and size */
        messageBuffer = &(messageBuffer[length]);
        bufferLength -= length;
        /* insert role item in message */
        len = strlen(discoveryWriter->role);
        NW_CONFIDENCE(bufferLength >= (len+3));
        *(messageBuffer++) = NW_DISCTAG_ROLE;
        *(messageBuffer++) = (os_uchar)len/256;
        *(messageBuffer++) = (os_uchar)(len&0xff);
        memcpy(messageBuffer,discoveryWriter->role,len);
        messageBuffer += len;
        bufferLength -= (len+3);
        /* insert nodelist in message */
        /* @todo dds#2523 Can simplify & remove repetition below. Want to see it work 1st */
        if (nw_configurationGetIsIPv6())
        {
            /* IPv6 */
            NW_CONFIDENCE(bufferLength >= 3);
            *(messageBuffer++) = NW_DISCTAG_IPV6LIST;
            len_field = messageBuffer; /* length placeholder */
            messageBuffer += 2;
            bufferLength -= 3;
            len = 0;
            NW_TRACE(Discovery, 4, "Sending IPv6 List start ");
            while (NodeList != NULL ) {
                os_sockaddr_in6* in6Address = (os_sockaddr_in6*) &NodeList->address;
                NW_CONFIDENCE(bufferLength >= 20);
                memcpy(messageBuffer,&in6Address->sin6_addr.s6_addr, 16);
                messageBuffer += 16;
                hostId = nw_plugHostToNetwork(NodeList->networkId);
                memcpy(messageBuffer,&hostId, 4);
                messageBuffer += 4;
                bufferLength -= 20;
                len += 8;
                oldItem = NodeList;
                NodeList = NodeList->next;
                os_free(oldItem);

                NW_TRACE_2(Discovery, 4, "Sending IPv6 List Item 0x%x  address: %s ",
                                         hostId,
                                         os_sockaddrAddressToString((os_sockaddr*) in6Address,
                                                                    addressStr,
                                                                    sizeof(addressStr)));
            }
        }
        else
        {
            /* IPv4 */
            NW_CONFIDENCE(bufferLength >= 3);
            *(messageBuffer++) = NW_DISCTAG_IPV4LIST;
            len_field = messageBuffer; /* length placeholder */
            messageBuffer += 2;
            bufferLength -= 3;
            len = 0;
            NW_TRACE(Discovery, 4, "Sending List start ");
            while (NodeList != NULL ) {
                os_sockaddr_in* in4Addr = (os_sockaddr_in*) &NodeList->address;
                NW_CONFIDENCE(bufferLength >= 8);
                v4Address = nw_plugHostToNetwork(in4Addr->sin_addr.s_addr);
                memcpy(messageBuffer,&v4Address, 4);
                messageBuffer += 4;
                hostId = nw_plugHostToNetwork(NodeList->networkId);
                memcpy(messageBuffer,&hostId, 4);
                messageBuffer += 4;
                bufferLength -= 8;
                len += 8;
                oldItem = NodeList;
                NodeList = NodeList->next;
                os_free(oldItem);

                NW_TRACE_2(Discovery, 4, "Sending List Item 0x%x  address: 0x%x ", hostId, in4Addr->sin_addr.s_addr );
            }
        }
        /* now fill in the length */
        *(len_field++) = (os_uchar)len/256;
        *(len_field++) = (os_uchar)(len&0xff);
        *(messageBuffer++) = NW_DISCTAG_SENTINEL;
        bufferLength -= 1;


        /* Do write and flush immediately */
        nw_plugSendChannelMessageEnd(discoveryWriter->sendChannel, messageBuffer, NULL);
        maxBytes = bufferLength;
        result = nw_plugSendChannelMessagesFlush(discoveryWriter->sendChannel,
                                                 TRUE, &maxBytes, NULL);
    }
    NW_CONFIDENCE(result);
}





static void *
nw_discoveryWriterMain(
    nw_runnable runnable,
    c_voidp arg)
{
    nw_discoveryWriter discoveryWriter = (nw_discoveryWriter)runnable;
    c_bool terminationRequested;
    os_uint32 heartbeatInterval;
    float safetyFactor;
    os_uint32 salvoSize;
    os_uint32 respondCount;
    static c_time regularSleepTime;
    static c_time startStopSleepTime;
    static c_time heartbeatTime;
    NW_STRUCT(nw_discoveryMessage) discoveryMessage;
    os_uint32 i;
    nw_discMessage msg;

    c_char* path;

    nw_runnableSetRunState(runnable, rsRunning);

    /* several parameters */
    path = os_malloc(strlen(runnable->name) + strlen(NWCF_SEP) + strlen(NWCF_ROOT(Tx)) + 1);
    os_sprintf(path, "%s%s%s", runnable->name, NWCF_SEP, NWCF_ROOT(Tx));
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
        NW_REPORT_WARNING_2(
            "retrieving discovery sending parameters",
            "specified SafetyFactor %f too small, "
            "switching to %f",
            safetyFactor, NWCF_MIN(SafetyFactor));
        safetyFactor = NWCF_MIN(SafetyFactor);
    }

    salvoSize = NWCF_SIMPLE_PARAM(ULong, path, SalvoSize);
    if (salvoSize < NWCF_MIN(SalvoSize)) {
        NW_REPORT_WARNING_2(
            "retrieving discovery sending parameters",
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

    if ( discoveryWriter->scope ) {
        nw_plugChannelNotifyGpAddList(nw_plugChannel(discoveryWriter->sendChannel), discoveryWriter->probeList);
    }

    c_mutexLock(&discoveryWriter->mutex);

    /* First give a salvo of starting messages so everybody knows about me */
    discoveryMessage.sign = NW_DISCOVERY_STARTING_SIGN;

    NW_TRACE_1(
        Discovery, 2,
        "Liveliness state set to %s",
        NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));

    terminationRequested = (os_int)nw_runnableTerminationRequested(runnable);
    for (i=0; (i<salvoSize) && (!terminationRequested); i++) {
        nw_discoveryWriterWriteMessageToNetwork(discoveryWriter,
            (nw_data)&discoveryMessage, NW_DISCOVERY_MESSAGE_SIZE);
        c_condTimedWait(&discoveryWriter->condition, &discoveryWriter->mutex,
            startStopSleepTime);
        terminationRequested = (os_int)nw_runnableTerminationRequested(runnable);
    }

    /* Now go into regular state */
    discoveryMessage.sign = NW_DISCOVERY_ALIVE_SIGN;
    respondCount = 0;

    NW_TRACE_1(
        Discovery, 2,
        "Liveliness state set to %s",
        NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));

    while (!terminationRequested) {
        while ( (msg = nw_messageBoxPopMessage(discoveryWriter->mbox)) != NULL ) {
            switch (msg->messageType) {
                case NW_DISCBOX_DOREQ:
                    /* send request to the indicated network _id , with the given expression */
                    nw_plugChannelNotifyGpAdd(discoveryWriter->sendChannel, msg->networkId, msg->address);
                    nw_discoveryWriterWriteReqToDestination(discoveryWriter,
                        (nw_data)&discoveryMessage, NW_DISCOVERY_MESSAGE_SIZE,msg->networkId, msg->expression);
                break;
                case NW_DISCBOX_SENDLIST:
                    /* send the provide list to the indicated network _id  */
                    nw_discoveryWriterWriteNodelistToDestination(discoveryWriter,
                        (nw_data)&discoveryMessage, NW_DISCOVERY_MESSAGE_SIZE,msg->networkId, msg->nodeList);
                break;
                case NW_DISCBOX_RESPOND:
                    discoveryWriter->respondToNode = TRUE;
                break;
            }
            os_free(msg);
        }

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
        terminationRequested = (os_int)nw_runnableTerminationRequested(runnable);
    }

    /* Send a salvo of stopping messages so everybody can react quickly */
    discoveryMessage.sign = NW_DISCOVERY_STOPPING_SIGN;

    NW_TRACE_1(
        Discovery, 2,
        "Liveliness state set to %s",
        NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));

    for (i=0; i<salvoSize; i++) {
        nw_discoveryWriterWriteMessageToNetwork(discoveryWriter,
            (nw_data)&discoveryMessage, NW_DISCOVERY_MESSAGE_SIZE);
        c_condTimedWait(&discoveryWriter->condition, &discoveryWriter->mutex,
            startStopSleepTime);
        terminationRequested = (os_int)nw_runnableTerminationRequested(runnable);
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

        result->scope = NWCF_DEFAULTED_ATTRIB(String, name, Scope, "","");
        if (*(result->scope) == '\0') {
            result->scope = NULL;
            NW_TRACE(Discovery, 2, "Scope W#1 (null)");
        } else {
            NW_TRACE_1(Discovery, 2, "Scope W#1 %s",result->scope);
        }
        result->role = nw_configurationGetDomainRole();
        result->probeList = NWCF_DEFAULTED_PARAM(String, name, ProbeList, "");


        result->mbox = nw_messageBoxNew();
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

void
nw_discoveryWriterRespondToStartingNode(
    nw_discoveryWriter discoveryWriter,
    nw_networkId networkId)
{
    os_sockaddr_storage zeroAddress;
    memset (&zeroAddress, 0, sizeof(zeroAddress));
    nw_messageBoxPushMessage(discoveryWriter->mbox, networkId, zeroAddress, NULL, NULL, NW_DISCBOX_RESPOND);
    nw_discoveryWriterTrigger((nw_runnable)discoveryWriter);
}

void
nw_discoveryWriterSendRequest(
    nw_discoveryWriter discoveryWriter,
    nw_networkId networkId,
    os_sockaddr_storage address,
    nw_name expression)
{
    nw_messageBoxPushMessage(discoveryWriter->mbox, networkId, address, expression, NULL, NW_DISCBOX_DOREQ);
    nw_discoveryWriterTrigger((nw_runnable)discoveryWriter);
}

void
nw_discoveryWriterSendList(
    nw_discoveryWriter discoveryWriter,
    nw_networkId networkId,
    nw_nodeListItem nodeList
    )
{
    os_sockaddr_storage zeroAddress;
    memset (&zeroAddress, 0, sizeof(zeroAddress));
    nw_messageBoxPushMessage(discoveryWriter->mbox, networkId, zeroAddress, NULL, nodeList, NW_DISCBOX_SENDLIST);
    nw_discoveryWriterTrigger((nw_runnable)discoveryWriter);
}


/* --------------------- discoveryReader (concrete) ------------------------- */

static void nw_discoveryReaderFinalize(nw_runnable runnable);


/**
* @extends nw_discovery
*/
NW_STRUCT(nw_discoveryReader) {
    NW_EXTENDS(nw_discovery);
    /* Action routines and parameter */
    nw_discoveryAction startedAction;
    nw_discoveryAction stoppedAction;
    nw_discoveryAction diedAction;
    nw_discoveryAction gpAddAction;
    nw_discoveryAction gpRemoveAction;
    nw_discoveryWriter discoveryWriter;
    nw_discoveryMsgArg arg;
    /* Discovery behaviour */
    nw_seqNr deathDetectionCount;
    /* Keep administration of alive nodes */
    nw_name role;
    nw_name scope;
    nw_stringList scopeList;
    nw_seqNr aliveNodesCount;
    nw_seqNr diedNodesCount;
    nw_seqNr aliveNodesHashSize;
    nw_aliveNodesHashItem *aliveNodesHash;
    /* Request for alive-check has been submitted */
    nw_bool checkRequested;
    os_mutex checkRequestedMtx;
    /* Immediate interaction with the networking plug */
    nw_plugChannel receiveChannel;
    /* Allow reconnection of nodes that stopped or died earlier */
    nw_bool reconnectAllowed;
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
    os_sockaddr_storage address,
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
            NW_CONFIDENCE(os_sockaddrIPAddressEqual((os_sockaddr*)&currentItem->address, (os_sockaddr*)&address));
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
    os_sockaddr_storage address,
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
        } else {
            insertionNeeded = TRUE;
        }
    }
    if (!found) {
        newItem = (nw_aliveNodesHashItem)os_malloc(sizeof(*newItem));
        newItem->address = address;
        newItem->networkId = networkId;
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

static nw_nodeListItem
nw_discoveryReaderBuildNodeList(
    nw_discoveryReader discoveryReader,
    nw_name scope)
{
    nw_seqNr i;
    nw_aliveNodesHashItem hashItem;
    nw_stringList scopeList;
    nw_nodeListItem result = NULL;

    NW_TRACE_1(Discovery, 5, "building nodelist for scope %s",scope);


    scopeList = nw_stringListNew(scope, ";, ");
    for (i=0; i<discoveryReader->aliveNodesHashSize; i++) {
        hashItem = NW_ALIVENODESHASH_ITEMBYINDEX(discoveryReader, i);
        while (hashItem != NULL) {
            if (!hashItem->hasDied) {
                if (nw_RoleScopeMatch(hashItem->role,scopeList)) {

                    NW_TRACE_2(Discovery, 4, "adding to list host 0x%x address 0x%x",hashItem->networkId,hashItem->address);
                    result = nw_nodeListAdd(result,hashItem->networkId,hashItem->address);
                }
            }
            hashItem = hashItem->next;
        }
    }
    return result;
}



/**
* @param address The heartbeat sender's IP address
*/
static void
nw_discoveryReaderReceivedHeartbeat(
    nw_discoveryReader discoveryReader,
    nw_data messageBuffer,
    nw_length bufferLength,
    os_sockaddr_storage address,
    c_time receivedTime)
{
    NW_STRUCT(nw_discoveryMessage) discoveryMessage;
    v_networkId networkId;
    c_time heartbeatInterval;
    c_time diffTime;
    c_equality eq;
    nw_aliveNodesHashItem itemFound;
    nw_bool wasCreated;
    nw_bool endReached = FALSE;
    os_uint32 i;
    nw_name  no_role = "";
    nw_name  role = no_role;
    nw_name  reqscope = NULL;
    nw_data  Nodelist = NULL;
    nw_seqNr NodelistLen = 0;
    os_uchar tag;
    os_uint32 len;
    char addressStr[INET6_ADDRSTRLEN];

    NW_CONFIDENCE(messageBuffer != NULL);
    NW_CONFIDENCE(bufferLength >= NW_DISCOVERY_MESSAGE_SIZE);

    memcpy(&discoveryMessage, messageBuffer, NW_DISCOVERY_MESSAGE_SIZE);
    messageBuffer += NW_DISCOVERY_MESSAGE_SIZE;
    bufferLength -= NW_DISCOVERY_MESSAGE_SIZE;
    networkId = nw_plugNetworkToHost(discoveryMessage.networkId);

    while (!endReached && bufferLength >= 3 ) {
        tag = *(messageBuffer++);
        len =(*(messageBuffer))*256 + *(messageBuffer+1);
        messageBuffer += 2;
        bufferLength -= 3;

        switch (tag) {
            case NW_DISCTAG_SENTINEL:
                endReached = TRUE;
            break;
            case NW_DISCTAG_ROLE:
                role = os_malloc(len+1);
                memcpy(role,messageBuffer,len);
                role[len] = 0;
            break;
            case NW_DISCTAG_REQ:
                reqscope = os_malloc(len+1);
                memcpy(reqscope,messageBuffer,len);
                reqscope[len] = 0;
            break;
            case NW_DISCTAG_IPV4LIST:
                if (! nw_configurationGetIsIPv6())
                {
                    Nodelist = messageBuffer;
                    NodelistLen = len;
                }
                else
                {
                    NW_REPORT_ERROR_2("nw_discoveryReaderReceivedHeartbeat",
                                      "IPv4 heartbeat received from network ID 0x%x, host %s in IPv6 networking service",
                                      networkId,
                                      os_sockaddrAddressToString((os_sockaddr*) &address,
                                                                    addressStr,
                                                                    sizeof(addressStr)));
                    return;
                }
            break;
            case NW_DISCTAG_IPV6LIST:
                if (nw_configurationGetIsIPv6())
                {
                    Nodelist = messageBuffer;
                    NodelistLen = len;
                }
                else
                {
                    NW_REPORT_ERROR_2("nw_discoveryReaderReceivedHeartbeat",
                                      "IPv6 heartbeat received from network ID 0x%x, host %s in IPv4 networking service",
                                      networkId,
                                      os_sockaddrAddressToString((os_sockaddr*) &address,
                                                                 addressStr,
                                                                 sizeof(addressStr)));
                    return;
                }
            break;
        }
        messageBuffer += len;
        bufferLength -= len;
    }


    /* filter out heartbeats that are not in our scope,
     * if we have defined one.
     */
    if ((!discoveryReader->scope) ||
        nw_RoleScopeMatch(role,discoveryReader->scopeList)) {

        NW_TRACE_4(
            Discovery, 6,
            "Received heartbeat from node with id 0x%x, address 0x%x, role \"%s\" "
            "state is %s",
            networkId,
            os_sockaddrAddressToString((os_sockaddr*) &address,
                                     addressStr,
                                     sizeof(addressStr)),
            role,
            NW_DISCOVERY_SIGN_TO_STATE(discoveryMessage.sign));

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
                        if (discoveryReader->scope &&
                            discoveryReader->gpRemoveAction!= NULL) {
                            discoveryReader->gpRemoveAction(networkId, address,
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
                nw_discoveryReaderLookupOrCreateNode(discoveryReader,
                    networkId,address, &itemFound, &wasCreated);
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
                    itemFound->role = os_strdup(role);
                    itemFound->networkId = networkId;

                    discoveryReader->aliveNodesCount++;
                    NW_TRACE_3(Discovery, 2, "New node detected with id 0x%x, address %s, role \"%s\"",
                                                networkId,
                                                os_sockaddrAddressToString((os_sockaddr*) &address,
                                                                            addressStr,
                                                                            sizeof(addressStr)),
                                                role);
                    NW_TRACE_2(Discovery, 3, "Node has heartbeatInterval %d.%3.3u",
                        heartbeatInterval.seconds, heartbeatInterval.nanoseconds/1000000U);
                    NW_TRACE_2(Discovery, 3, "Node has maxInterval %d.%3.3u",
                        itemFound->maxHeartbeatIntervalAllowed.seconds,
                        itemFound->maxHeartbeatIntervalAllowed.nanoseconds/1000000U);
                    if (discoveryReader->scope &&
                        discoveryReader->gpAddAction!= NULL) {
                        discoveryReader->gpAddAction(networkId, address,
                            receivedTime, discoveryReader->aliveNodesCount,
                            discoveryReader->arg);
                    }
                    if ( Nodelist || (discoveryReader->scope == NULL) ) {
                        itemFound->state = NW_DISCSTATE_FULL;
                    } else {
                        itemFound->state = NW_DISCSTATE_ALIVE;
                        /* request a nodelist */
                        nw_discoveryWriterSendRequest(discoveryReader->discoveryWriter, networkId, address, discoveryReader->scope);
                    }
                    if (discoveryReader->startedAction != NULL) {
                        discoveryReader->startedAction(networkId, address,
                            receivedTime, discoveryReader->aliveNodesCount,
                            discoveryReader->arg);
                    }

                } else {
                    if( itemFound->state == NW_DISCSTATE_KNOWN ) {
                        /* New known item, initialize it */
                        itemFound->maxHeartbeatIntervalAllowed = heartbeatInterval;
                        for (i=1; i<discoveryReader->deathDetectionCount; i++) {
                            itemFound->maxHeartbeatIntervalAllowed =
                                c_timeAdd(itemFound->maxHeartbeatIntervalAllowed,
                                          heartbeatInterval);
                        }
                        itemFound->lastHeartbeatReceivedTime = receivedTime;
                        itemFound->hasDied = FALSE;
                        itemFound->role = os_strdup(role);
                        itemFound->networkId = networkId;
                        if ( Nodelist ) {
                            itemFound->state = NW_DISCSTATE_FULL;
                        } else {
                            itemFound->state = NW_DISCSTATE_ALIVE;
                        }

                        discoveryReader->aliveNodesCount++;
                        NW_TRACE_3(Discovery, 2, "New node detected with id 0x%x, address %s, role \"%s\"",
                                                    networkId,
                                                    os_sockaddrAddressToString((os_sockaddr*) &address,
                                                                                 addressStr,
                                                                                 sizeof(addressStr)),
                                                     role);
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
                        if ( Nodelist ) {
                            itemFound->state = NW_DISCSTATE_FULL;
                        }
                    }

                    if (!itemFound->hasDied) {
                        diffTime = c_timeSub(receivedTime,
                            itemFound->lastHeartbeatReceivedTime);
                        eq = c_timeCompare(diffTime, itemFound->maxHeartbeatIntervalAllowed);

                        if (eq == C_GT && !discoveryReader->reconnectAllowed) {
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
                            if (discoveryReader->scope &&
                                discoveryReader->gpRemoveAction!= NULL) {
                                discoveryReader->gpRemoveAction(networkId, address,
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

        /* lookup or insert nodes for the nodelist.
         * if new set them to NW_DISCSTATE_KNOWN.
         */
        while ( discoveryReader->scope && Nodelist && NodelistLen > 0 ) {
            os_sockaddr_storage nodeAddress;
            os_uint32 nodeId;

            memset(&nodeAddress, 0, sizeof(nodeAddress));
            if (nw_configurationGetIsIPv6())
            {
                /* IPv6 */
                os_sockaddr_in6* in6Address = (os_sockaddr_in6*) &nodeAddress;
                in6Address->sin6_family = AF_INET6;
                memcpy(&in6Address->sin6_addr.s6_addr,Nodelist,16);
                Nodelist += 16;
                memcpy(&nodeId,Nodelist,4);
                nodeId = nw_plugNetworkToHost(nodeId);
                Nodelist += 4;
                NodelistLen -= 20;
                NW_TRACE_2(Discovery, 4, "Processing Incoming IPv6 Nodelistitem Id 0x%x, address %s",
                                            nodeId,
                                            os_sockaddrAddressToString((os_sockaddr*) in6Address,
                                                                        addressStr,
                                                                        sizeof(addressStr)));
                if (nodeId != discoveryReader->receiveChannel->nodeId) {
                    nw_discoveryReaderLookupOrCreateNode(discoveryReader,
                        nodeId,nodeAddress,&itemFound, &wasCreated);
                    if( wasCreated ) {
                        NW_TRACE_3(Discovery, 2, "Received from hostid:0x%x: New IPv6 NodeId:0x%x, address:%s",
                                                 networkId,
                                                 nodeId,
                                                 os_sockaddrAddressToString((os_sockaddr*) in6Address,
                                                                        addressStr,
                                                                        sizeof(addressStr)));
                        itemFound->state = NW_DISCSTATE_KNOWN;
                        itemFound->networkId = nodeId;
                        itemFound->hasDied = TRUE;
                        if (discoveryReader->scope &&
                            discoveryReader->gpAddAction!= NULL) {
                            discoveryReader->gpAddAction(nodeId, nodeAddress,
                                receivedTime, discoveryReader->aliveNodesCount,
                                discoveryReader->arg);
                        }

                        nw_discoveryWriterSendRequest(discoveryReader->discoveryWriter, nodeId, nodeAddress, discoveryReader->scope);
                    }
                }
            }
            else
            {
                os_sockaddr_in* in4Addr = (os_sockaddr_in*) &nodeAddress;
                in4Addr->sin_family = AF_INET;
                memcpy(&in4Addr->sin_addr.s_addr,Nodelist,4);
                Nodelist += 4;
                memcpy(&nodeId,Nodelist,4);
                nodeId = nw_plugNetworkToHost(nodeId);
                in4Addr->sin_addr.s_addr = nw_plugNetworkToHost(in4Addr->sin_addr.s_addr);
                Nodelist += 4;
                NodelistLen -= 8;
                NW_TRACE_2(Discovery, 4, "Processing Incoming Nodelistitem Id 0x%x, address 0x%x", nodeId, in4Addr->sin_addr.s_addr);
                if (nodeId != discoveryReader->receiveChannel->nodeId) {
                    nw_discoveryReaderLookupOrCreateNode(discoveryReader,
                        nodeId,nodeAddress,&itemFound, &wasCreated);
                    if( wasCreated ) {
                        NW_TRACE_3(Discovery, 2, "Received from hostid:0x%x: New NodeId:0x%x, address:0x%x",
                                                    networkId,
                                                    nodeId,
                                                    in4Addr->sin_addr.s_addr);
                        itemFound->state = NW_DISCSTATE_KNOWN;
                        itemFound->networkId = nodeId;
                        itemFound->hasDied = TRUE;
                        if (discoveryReader->scope &&
                            discoveryReader->gpAddAction!= NULL) {
                            discoveryReader->gpAddAction(nodeId, nodeAddress,
                                receivedTime, discoveryReader->aliveNodesCount,
                                discoveryReader->arg);
                        }

                        nw_discoveryWriterSendRequest(discoveryReader->discoveryWriter, nodeId, nodeAddress, discoveryReader->scope);
                    }
                }
            }
        }

        /* create new nodelist based on scope expression
         * and send it.
         */
        if ( discoveryReader->scope && reqscope ) {
            /* Create a copy list which has the scope applied */
            nw_nodeListItem list = nw_discoveryReaderBuildNodeList(discoveryReader,reqscope);

            nw_discoveryWriterSendList(discoveryReader->discoveryWriter,networkId,list); /* transfer ownership of list */
            reqscope = NULL;
        }
    } else {
        NW_TRACE_1(Discovery, 4, "Ignoring out of scope role \"%s\"", role);
    }

    if ( role != no_role) {

        os_free(role);
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
    nw_aliveNodesHashItem itemFound = NULL;
    nw_bool reconnectFree = FALSE;

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
                    if ( discoveryReader->reconnectAllowed) {
                        nw_discoveryReaderRemoveNode(discoveryReader, hashItem->networkId, hashItem->address,&itemFound);
                        NW_CONFIDENCE(itemFound != NULL);
                    } else {
                        hashItem->hasDied = TRUE;
                        discoveryReader->diedNodesCount++;
                    }
                    /* Take action if node has not died before */
                    if (hashItem->state != NW_DISCSTATE_KNOWN ) {
                        discoveryReader->aliveNodesCount--;
                    }
                    NW_TRACE_3(Discovery, 2, "Declared node 0x%x dead, "
                        "no heartbeat received during interval %d.%3.3u", hashItem->networkId,
                        diffTime.seconds, diffTime.nanoseconds/1000000);
                    if (discoveryReader->diedAction != NULL) {
                        discoveryReader->diedAction(hashItem->networkId,
                            hashItem->address, checkTime,
                            discoveryReader->aliveNodesCount,
                            discoveryReader->arg);
                    }
                    if (discoveryReader->scope &&
                        discoveryReader->gpRemoveAction!= NULL) {
                        discoveryReader->gpRemoveAction(hashItem->networkId,
                            hashItem->address, checkTime,
                            discoveryReader->aliveNodesCount,
                            discoveryReader->arg);
                    }
                    if (discoveryReader->reconnectAllowed) {
                        reconnectFree = TRUE;
                    }
                }
            } else {
                NW_TRACE_1(Discovery, 5, "Ignoring dead node 0x%x", hashItem->networkId);
            }

            if ( discoveryReader->scope && hashItem->state != NW_DISCSTATE_FULL ) {
                nw_discoveryWriterSendRequest(discoveryReader->discoveryWriter, hashItem->networkId, hashItem->address, discoveryReader->scope);
            }
            hashItem = hashItem->next;

            if (discoveryReader->reconnectAllowed && reconnectFree) {
                nw_aliveNodesHashItemFree(itemFound);
            }
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
    struct nw_senderInfo_s sender;
    c_time now;

    nw_runnableSetRunState(runnable, rsRunning);
    memset(&sender, 0, sizeof(sender));

    terminationRequested = (os_int)nw_runnableTerminationRequested(runnable);
    while (!terminationRequested) {
        /* First retrieve a buffer to write in */
        nw_plugReceiveChannelMessageStart(discoveryReader->receiveChannel,
            &messageBuffer, &bufferLength, &sender, NULL);

        if (messageBuffer != NULL) {
            nw_plugReceiveChannelMessageEnd(discoveryReader->receiveChannel, NULL);
        }

        /* Check if we have to stop */
        terminationRequested = (os_int)nw_runnableTerminationRequested(runnable);
        if (!terminationRequested) {
            now = v_timeGet();
            if (messageBuffer != NULL) {
                /* Determine the contents of the message and update the admin */
                nw_discoveryReaderReceivedHeartbeat(discoveryReader, messageBuffer,
                                                    bufferLength,
                                                    sender.ipAddress,
                                                    now);
            }
        os_mutexLock( &discoveryReader->checkRequestedMtx );
            if (discoveryReader->checkRequested) {
                nw_discoveryReaderCheckForDiedNodes(discoveryReader, now);
                discoveryReader->checkRequested = FALSE;

            }
        os_mutexUnlock( &discoveryReader->checkRequestedMtx );
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
    nw_discoveryAction gpAddAction,
    nw_discoveryAction gpRemoveAction,
    nw_discoveryWriter discoveryWriter,
    nw_discoveryMsgArg arg)
{
    nw_discoveryReader result = NULL;
    nw_seqNr i;
    char *pathName;
    char *schedPathName;
    size_t schedPathNameSize;
    os_mutexAttr checkReqAttr;
    nw_aliveNodesHashItem itemFound;
    nw_bool wasCreated;
    c_time heartbeatInterval;
    c_time receivedTime = v_timeGet();

    result = (nw_discoveryReader)os_malloc((os_uint32)sizeof(*result));
    if (result != NULL) {
        os_sockaddr_storage dummy_address;
        os_sockaddr_in6 sin6;

        sin6.sin6_family = AF_INET6;
        sin6.sin6_addr = os_in6addr_any;
        memcpy(&dummy_address, &sin6, sizeof(sin6));

        /* Initialize parent */
        /* First determine its parameter path */
        schedPathNameSize = strlen(name) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(Rx)) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(Scheduling))+ 1;
        schedPathName = os_malloc(schedPathNameSize);
        os_sprintf(schedPathName, "%s%s%s%s%s", name, NWCF_SEP, NWCF_ROOT(Rx),
            NWCF_SEP, NWCF_ROOT(Scheduling));
        nw_discoveryInitialize((nw_discovery)result, networkId,
            name, schedPathName,
            nw_discoveryReaderMain,      /* my own main */
            nw_discoveryReaderTrigger,   /* my own trigger */
            nw_discoveryReaderFinalize); /* my own finalize */
        os_free(schedPathName);

        /* Initialize myself */
        /* Set the private members */

        result->role = nw_configurationGetDomainRole();
        result->scope = NWCF_DEFAULTED_ATTRIB(String, name, Scope, "","");
        if (*(result->scope) == '\0') {
            result->scope = NULL;
        }
        result->scopeList = nw_stringListNew(result->scope, ";, ");

        result->startedAction = startedAction;
        result->stoppedAction = stoppedAction;
        result->diedAction = diedAction;
        result->gpAddAction = gpAddAction;
        result->gpRemoveAction = gpRemoveAction;
        result->discoveryWriter  = discoveryWriter;
        result->arg = arg;
        result->checkRequested = FALSE;
        os_mutexAttrInit(&checkReqAttr);
        checkReqAttr.scopeAttr = OS_SCOPE_PRIVATE;
        os_mutexInit(&result->checkRequestedMtx, &checkReqAttr);
        result->receiveChannel = nw_plugNetworkNewReceiveChannel(
            ((nw_discovery)result)->network, name, NULL,NULL);


        pathName = os_malloc(strlen(name) + strlen(NWCF_SEP) + strlen(NWCF_ROOT(Rx)) + 1) ;
        os_sprintf(pathName, "%s%s%s", name, NWCF_SEP, NWCF_ROOT(Rx));

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

        result->reconnectAllowed = NWCF_SIMPLE_ATTRIB(Bool,NWCF_ROOT(General) NWCF_SEP NWCF_NAME(Reconnection),allowed);
        /* Do not start in the constructor, but let somebody else start me */

        /*
         *  the networking service will only send data to the network if there's at least one other running node
         *  (referred to as 'remote activity'). During start-up, the service temporarily sends no data even though
         *  there is remote activity, because it simply hasn't detected it yet. For built-in topics this leads to a
         *  bootstrap issue, causing some of them not to be transmitted to remote nodes.
         *  This causes your application to fail.
         *  0x0100007F = 127.0.0.1
         *  12345 = networkId
         *
         *  solved by creating a fake node at startup.
         */
        if (!nw_configurationGetIsIPv6())
        {
            /* dummy_address was already initialised to IPv6 loopback at declaration */
            os_sockaddr_in* in4Addr = (os_sockaddr_in*) &dummy_address;
            in4Addr->sin_addr.s_addr = 0x0100007F;
            in4Addr->sin_family = AF_INET;
        }

        heartbeatInterval.seconds = 1;
        heartbeatInterval.nanoseconds = 0;
        nw_discoveryReaderLookupOrCreateNode(result,
                           12345,dummy_address, &itemFound, &wasCreated);
        if (wasCreated) {
            /* New item, initialize it */
            itemFound->maxHeartbeatIntervalAllowed = heartbeatInterval;
            for (i=1; i<result->deathDetectionCount; i++) {
                itemFound->maxHeartbeatIntervalAllowed =
                    c_timeAdd(itemFound->maxHeartbeatIntervalAllowed,
                              heartbeatInterval);
            }
            itemFound->lastHeartbeatReceivedTime = receivedTime;
            itemFound->hasDied = FALSE;
            itemFound->networkId = 12345;
            itemFound->state = NW_DISCSTATE_ALIVE;

            result->aliveNodesCount++;

            if (result->startedAction != NULL) {
                result->startedAction(12345, dummy_address,
                    receivedTime, result->aliveNodesCount,
                    result->arg);
            }
        } else {
            NW_REPORT_WARNING("nw_discoveryReaderNew",
                        "failed to create fake node"
                       );
        }
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

    os_mutexDestroy(&discoveryReader->checkRequestedMtx);
    /* Finalize parent */
    nw_discoveryFinalize(runnable);
}

void
nw_discoveryReaderInitiateCheck(
    nw_discoveryReader discoveryReader)
{
    os_mutexLock( &discoveryReader->checkRequestedMtx );
    discoveryReader->checkRequested = TRUE;
    os_mutexUnlock( &discoveryReader->checkRequestedMtx );
    NW_TRACE(Discovery, 5, "Triggering discoveryReader for liveliness checking");
    nw_discoveryReaderTrigger((nw_runnable)discoveryReader);
}

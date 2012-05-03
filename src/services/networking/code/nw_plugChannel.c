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
/* interface */
#include "nw_plugChannel.h"
#include "nw__plugChannel.h"

/* implementation */
#include "c_sync.h" /* For the mutex */
#include "nw__plugSendChannel.h"
#include "nw__plugReceiveChannel.h"
#include "nw__plugDataBuffer.h"
#include "nw_commonTypes.h"
#include "nw_misc.h"
#include "nw_configuration.h"
#include "nw_configurationDefs.h"
#include "nw_report.h"
#include "nw__confidence.h"
#include "nw_security.h"
/* For socket constructors */
#include "nw_socketBroadcast.h"
#include "nw_socketMulticast.h"
#include "nw_socketLoopback.h"

/* --------------------- helper class: nw_messageBox ------------------------ */

NW_CLASS(nw_messageBoxMessage);
NW_STRUCT(nw_messageBoxMessage) {
    nw_networkId networkId;
    os_sockaddr_storage address;
    c_string list;
    nw_messageBoxMessageType messageType;
    nw_messageBoxMessage next;
};

NW_STRUCT(nw_messageBox) {
    c_mutex mutex;
    /* fifo queue */
    nw_messageBoxMessage firstMessage;
    nw_messageBoxMessage lastMessage;
};

static nw_messageBox
nw_messageBoxNew(
    void)
{
    nw_messageBox result;

    result = (nw_messageBox)os_malloc(sizeof(*result));
    if (result != NULL) {
        c_mutexInit(&result->mutex, PRIVATE_MUTEX);
        result->firstMessage = NULL;
        result->lastMessage = NULL;
    }
    return result;
}

static void
nw_messageBoxFree(
    nw_messageBox messageBox)
{
    nw_messageBoxMessage toFree;

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
    nw_messageBox messageBox,
    nw_networkId networkId,
    os_sockaddr_storage address,
    c_string list,
    nw_messageBoxMessageType messageType)
{
    nw_messageBoxMessage newMessage;
    nw_messageBoxMessage *prevNextPtr;

    c_mutexLock(&messageBox->mutex);
    newMessage = (nw_messageBoxMessage)os_malloc(sizeof(*newMessage));
    newMessage->networkId = networkId;
    newMessage->address = address;
    newMessage->list = list;
    newMessage->messageType = messageType;

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

static nw_messageBoxMessage
nw_messageBoxPopMessage(
    nw_messageBox messageBox)
{
    nw_messageBoxMessage result = NULL;

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


/* PlugChannel class operations */

/* Private operations */

/* Protected operations */

void
nw_plugChannelInitialize(
    nw_plugChannel channel,
    nw_seqNr seqNr,
    nw_networkId nodeId,
    nw_communicationKind communication,
    nw_plugPartitions partitions,
    nw_userData *userDataPtr,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    nw_size fragmentLength;
    nw_bool reliable;
    nw_bool controlNeeded;
    static sk_portNr sendingPortNr = NWCF_DEF(PortNr);
    static sk_portNr receivingPortNr = NWCF_DEF(PortNr);
    sk_portNr newPortNr;
    nw_plugInterChannel *interChannelPtr = (nw_plugInterChannel *)userDataPtr;
    char *defaultPartitionAddress;

    /* Simple attributes */
    channel->name = nw_stringDup(pathName);
    channel->Id = seqNr;
    channel->nodeId = nodeId;
    channel->communication = communication;
    channel->partitions = partitions;

    /* Attributes to be read from config */
    /* QoS-es*/
    reliable = NWCF_SIMPLE_ATTRIB(Bool, pathName, reliable);
    if (reliable) {
        channel->reliabilityOffered = NW_REL_RELIABLE;
        controlNeeded = TRUE;
        /* Create object for inter-channel communication */
        nw_plugInterChannelIncarnate(interChannelPtr, pathName);
        channel->interChannelComm = *interChannelPtr;
    } else {
        channel->reliabilityOffered = NW_REL_BEST_EFFORT;
        controlNeeded = FALSE;
        /* NO object needed for inter-channel communication */
        channel->interChannelComm = NULL;
    }
    /* Default, to be implemented */
    channel->priorityOffered = NW_PRIORITY_UNDEFINED;
    channel->latencyBudgetOffered = NW_LATENCYBUDGET_UNDEFINED;

    /* Network fragment length */
    fragmentLength = (nw_size)NWCF_SIMPLE_PARAM(Size, pathName, FragmentSize);

    /* CHECKME, NWCF_MIN(FragmentSize) must be larger dealing with encryption */
    if (fragmentLength < NWCF_MIN(FragmentSize)) {
        NW_REPORT_WARNING_3("initializing network",
            "Channel \"%s\": requested value %u for fragment size is too small, "
            "using %u instead",
            pathName, fragmentLength, NWCF_MIN(FragmentSize));
        fragmentLength = NWCF_MIN(FragmentSize);
    }
    else if(fragmentLength > NWCF_MAX(FragmentSize)) {
        NW_REPORT_WARNING_3("initializing network",
            "Channel \"%s\": requested value " PA_SIZEFMT " for fragment size is too big, "
            "using %u instead",
            pathName, fragmentLength, NWCF_MAX(FragmentSize));
        fragmentLength = NWCF_MAX(FragmentSize);
    }
    /* FIXME, this rounds up to multiple of 4, but it should round down to
     * meet network constraints (??) */
    /* round to lowest NW_FRAG_BOUNDARY multiplication higher than
     * fragmentLength */
    channel->fragmentLength =
    NW_ALIGN(NW_PLUGDATABUFFER_ALIGNMENT, (nw_length)fragmentLength);

    /* What is the base adress of the socket wee need ? */
    nw_plugPartitionsGetDefaultPartition(partitions, &defaultPartitionAddress, NULL /* SecurityProfile not of interest */ );

    switch (communication) {
    case NW_COMM_SEND:
        newPortNr = NWCF_DEFAULTED_PARAM(ULong, pathName, PortNr, sendingPortNr);
        if (newPortNr == sendingPortNr) {
            sendingPortNr+=2;
        }
        channel->socket = nw_socketSendNew(defaultPartitionAddress,
            newPortNr, controlNeeded, pathName);
    break;
    case NW_COMM_RECEIVE:
        newPortNr = NWCF_DEFAULTED_PARAM(
            ULong, pathName, PortNr, receivingPortNr);
        if (newPortNr == receivingPortNr) {
            receivingPortNr+=2;
        }
        channel->socket = nw_socketReceiveNew(defaultPartitionAddress, newPortNr,
            controlNeeded, pathName);
    break;
    default:
        NW_CONFIDENCE(FALSE);
    break;
    }

    channel->messageBox = nw_messageBoxNew();
    channel->onFatal = onFatal;
    channel->onFatalUsrData = onFatalUsrData;


    channel->reconnectAllowed = NWCF_SIMPLE_ATTRIB(Bool,NWCF_ROOT(General) NWCF_SEP NWCF_NAME(Reconnection),allowed);
    channel->crc = ut_crcNew(UT_CRC_KEY);
}


void
nw_plugChannelFinalize(
    nw_plugChannel channel)
{
    if (channel) {
        if (channel->reliabilityOffered == NW_REL_RELIABLE) {
            nw_plugInterChannelExcarnate(&channel->interChannelComm);
        }
        nw_messageBoxFree(channel->messageBox);
        nw_socketFree(channel->socket);
        os_free(channel->name);
    }
}


void
nw_plugChannelFree(
    nw_plugChannel channel)
{
    if (channel) {
        switch (channel->communication) {
            case NW_COMM_SEND:
                nw_plugSendChannelFree(channel);
            break;
            case NW_COMM_RECEIVE:
                nw_plugReceiveChannelFree(channel);
            break;
        }
    }
}


nw_bool
nw_plugChannelProcessMessageBox(
    nw_plugChannel channel,
    nw_networkId *networkId /* out */,
    os_sockaddr_storage *address,
    c_string *list,
    nw_messageBoxMessageType *messageType /* out */)
{
    nw_bool result = FALSE;
    nw_messageBoxMessage message;

    message = nw_messageBoxPopMessage(channel->messageBox);
    if (message != NULL) {
        result = TRUE;
        *networkId = message->networkId;
        *messageType = message->messageType;
        *address = message->address;
        *list = message->list;
        os_free(message);
    }
    return result;
}


/* -------------------------------- Public ---------------------------------- */

nw_seqNr
nw_plugChannelGetId(
    nw_plugChannel channel)
{
    nw_seqNr result = NW_ID_UNDEFINED;

    NW_CONFIDENCE(channel);

    if (channel) {
        result = channel->Id;
    }

    return result;
}


nw_reliabilityKind
nw_plugChannelGetReliabilityOffered(
    nw_plugChannel channel)
{
    nw_reliabilityKind result = NW_RELIABILITY_UNDEFINED;

    NW_CONFIDENCE(channel);

    if (channel) {
        result = channel->reliabilityOffered;
    }

    return result;
}


nw_priorityKind
nw_plugChannelGetPriorityOffered(
    nw_plugChannel channel)
{
    nw_priorityKind result = NW_PRIORITY_UNDEFINED;

    NW_CONFIDENCE(channel);

    if (channel) {
        result = channel->priorityOffered;
    }

    return result;
}


void
nw_plugChannelNotifyNodeStarted(
    nw_plugChannel channel,
    nw_networkId networkId,
    os_sockaddr_storage address)
{
    nw_messageBoxPushMessage(channel->messageBox, networkId, address, NULL, NW_MBOX_NODE_STARTED);
}

void
nw_plugChannelNotifyNodeStopped(
    nw_plugChannel channel,
    nw_networkId networkId,
    os_sockaddr_storage address)
{
    nw_messageBoxPushMessage(channel->messageBox, networkId, address, NULL, NW_MBOX_NODE_STOPPED);
}

void
nw_plugChannelNotifyNodeDied(
    nw_plugChannel channel,
    nw_networkId networkId,
    os_sockaddr_storage address)
{
    nw_messageBoxPushMessage(channel->messageBox, networkId, address, NULL, NW_MBOX_NODE_DIED);
}

void
nw_plugChannelNotifyGpAdd(
    nw_plugChannel channel,
    nw_networkId networkId,
    os_sockaddr_storage address)
{
    nw_messageBoxPushMessage(channel->messageBox, networkId, address, NULL, NW_MBOX_GP_ADD);
}

void
nw_plugChannelNotifyGpAddList(
    nw_plugChannel channel,
    c_string probelist)
{
    os_sockaddr_storage dummy;
    memset(&dummy, 0, sizeof(dummy));
    NW_TRACE_1(
        Discovery, 2,
        "Adding ProbeList %s",probelist);
    nw_messageBoxPushMessage(channel->messageBox, 0, dummy, probelist, NW_MBOX_GP_ADDLIST);
}

void
nw_plugChannelNotifyGpRemove(
    nw_plugChannel channel,
    nw_networkId networkId,
    os_sockaddr_storage address)
{
    nw_messageBoxPushMessage(channel->messageBox, networkId, address, NULL, NW_MBOX_GP_REMOVE);
}


void
nw_plugChannelGetPartition(
    nw_plugChannel channel,
    nw_partitionId partitionId,
    nw_bool *found,
    nw_partitionAddress *partitionAddress,
    nw_networkSecurityPolicy *securityPolicy,
    nw_bool *connected,
    nw_bool *compression,
    os_uint32 *hash,
    c_ulong *mTTL)
{
    nw_plugPartitionsGetPartition(channel->partitions, partitionId, found,
        partitionAddress, securityPolicy, connected, compression, hash, mTTL);

}


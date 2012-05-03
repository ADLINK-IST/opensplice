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

#include "nw__plugSendChannel.h"
#include "nw_plugSendChannel.h"

/* implementation */
#include "os_stdlib.h"
#include "os_heap.h"
#include "nw_misc.h"
#include "nw_commonTypes.h"
#include "nw__plugDataBuffer.h"
#include "nw__plugControlBuffer.h"
#include "nw__plugChannel.h"
#include "nw_plugInterChannel.h"
#include "nw__confidence.h"
#include "nw_configuration.h" /* For NW_DEBUGGING */
#include "nw_report.h"
#include "nw_socketMisc.h" /* For helper function */
#include "nw_security.h"
#ifdef NW_DEBUGGING
#include "os_time.h"
#endif

#define sendChannelStatisticsCounterInc(fieldName, entity, value) \
        v_fullCounterSetValue(&(entity->fieldName),value);

#define NW_PARTITIONID_DEFAULT (0)
#define NW_PARTITIONID_NONE    (0xffffffff)

#define NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel) \
        /* The extra one is the default partition */ \
        nw_plugPartitionsGetNofPartitions(nw_plugChannel(sendChannel)->partitions)

#define NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, id) \
        sendChannel->receivingPartitions[id]

#define SC_PRINT_EVERY (5)

#define NW_MAXBURSTSIZE_UNLIMITED (0x0fffffff)
#define NW_MAXBURSTSIZE_NONE      NW_MAXBURSTSIZE_UNLIMITED

#define NW_RECEIVING_NODES_HASHSIZE (256)

#define nw_plugDataBufferReliabilityAdminFree(admin) os_free(admin)

/* helper function */

#define ADDR_SIZE (16)

static void nw_periodicCheckMessageBox( nw_plugChannel channel );


/* ------------------- nw_plugDataBufferReliabilityAdmin ----------------------- */

/* Wrapper around plugDataBuffer for maintaining reliability administration */

NW_CLASS(nw_plugDataBufferReliabilityAdmin);
NW_STRUCT(nw_plugDataBufferReliabilityAdmin) {
    nw_plugDataBufferReliabilityAdmin prev;
    nw_plugDataBufferReliabilityAdmin next;
    nw_plugSendChannel owningChannel;
    nw_seqNr usedCount;
    /* c_octet data[fragmentSize] */
};


#define REL_ADMIN_LENGTH \
        (UI(sizeof(NW_STRUCT(nw_plugDataBufferReliabilityAdmin))))

#define REL_ADMIN(ptr) \
        (((ptr) != NULL)? \
          (nw_plugDataBufferReliabilityAdmin)(UI(ptr) - REL_ADMIN_LENGTH): \
           NULL)

#define REL_DATA(ptr) \
        (((ptr) != NULL)? \
          (nw_plugDataBuffer)(UI(ptr) + REL_ADMIN_LENGTH): \
           NULL)

static nw_plugDataBufferReliabilityAdmin
    nw_plugSendChannelDataBufferReliabilityAdminCreate(
        nw_plugSendChannel sendChannel);

static void
    nw_plugDataBufferReliabilityAdminInsert(
        nw_plugDataBufferReliabilityAdmin admin,
        nw_plugDataBufferReliabilityAdmin *prevNext,
        nw_plugDataBufferReliabilityAdmin *nextPrev);

static void
    nw_plugDataBufferReliabilityAdminKeep(
        nw_plugDataBufferReliabilityAdmin admin);

static nw_bool
    nw_plugDataBufferReliabilityAdminRelease(
        nw_plugDataBufferReliabilityAdmin admin);


/* --------- administration concerning flushable buffers ---------------- */

static void
    nw_plugSendChannelPushCurrentWriteBuffer(
        nw_plugSendChannel sendChannel,
        nw_length dataLength);

static nw_plugDataBufferReliabilityAdmin
    nw_plugSendChannelFlushableBufferPop(
        nw_plugSendChannel sendChannel);


/* ------------------- forward declarations ----------------------- */

NW_CLASS(nw_resendItem);
NW_CLASS(nw_ackMessage);
NW_CLASS(nw_plugPartitionNode);
NW_CLASS(nw_plugReceivingPartition);
NW_CLASS(nw_plugReceivingNode);

/* --------------------------- ReceivingNode ----------------------- */

#define NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, id) \
    receivingNode->partitionNodes[id]

typedef enum nw_plugReceivingNodeState_e {
    NW_NODESTATE_UNDEFINED,
    NW_NODESTATE_RESPONDING,
    NW_NODESTATE_NOT_RESPONDING,
    NW_NODESTATE_STOPPED,
    NW_NODESTATE_RISEN, /* Not (yet) used */
    NW_NODESTATE_UNKNOWN
} nw_plugReceivingNodeState;


/* The nw-plugReceivingNode class is introduced for node-specific actions.
 * For example, resending is done point to point to a node so networking
 * will walk over the node list */

NW_STRUCT(nw_plugReceivingNode) {
    nw_plugSendChannel owner;
    nw_plugReceivingNodeState state;
    nw_seqNr nodeId;
    os_sockaddr_storage address;

    /* Array of partition-nodes owned by this node */
    nw_plugPartitionNode *partitionNodes;
    /* For double-checking only */
    nw_seqNr nofPartitions;

    /* Nodes are stored in a hash table */
    nw_plugReceivingNode nextInHash;
    /* Nodes are stored in a linked list in insertion order */
    nw_plugReceivingNode nextInList;
};


static nw_plugReceivingNode
nw_plugReceivingNodeNew(
    nw_plugSendChannel owner,
    nw_seqNr receivingNodeId,
    os_sockaddr_storage receivingAddress,
    nw_partitionId nofPartitions,
    nw_plugReceivingNode *prevNextInHash,
    nw_plugReceivingNode *prevNextInList);

static void
nw_plugReceivingNodeFree(
    nw_plugReceivingNode plugReceivingNode);

static nw_plugPartitionNode
nw_plugReceivingNodeCreatePartitionNode(
    nw_plugReceivingNode receivingNode,
    nw_partitionId partitionId);


static void
nw_plugReceivingNodeNotResponding(
    nw_plugReceivingNode plugReceivingNode);

/* --------------------------- ReceivingPartition --------------------------- */

/* The nw_plugReceivingPartition class is introduced for partition-specific
 * actions. Data is usually sent to a partition so reliability administration
 * is initially created and updated from this partition point of view.
 * Every partition has its own administration of sequence numbering and
 * fragmentation numering */

NW_STRUCT(nw_plugReceivingPartition) {

    /* The id of this partition */
    nw_partitionId partitionId;
    os_uint32 partitionHash;

    /* Data fieds for properly filling all sequence numbers for this
     * networking partition */
    /* Total number of fragmented messages upto now */
    nw_seqNr fragmentedSeqNr;
    /* SeqNr of last packet sent reliably to this partition */
    nw_seqNr packetSeqNr;

    /* A list of writing buffers that possibly have to be resent. The buffers
     * are created by the sendChannel */
    /*    nw_plugDataBufferReliabilityAdmin resendBufferList; */

    /* A list of resendItems that is to be used for quick allocation and
     * deallocation by partitionNodes */
    nw_resendItem freeResendList;
    /* The total number of resendItems ever allocated */
    nw_length totalResendCount;

    /* A list of resend items that makes it possible to resend data to a
     * node that was not yet known before. This can be seen as temporarily
     * transient data to ensure that any holes occuring during startup
     * can be recovered */
    nw_resendItem lateJoiningResendListHead;
    nw_resendItem lateJoiningResendListTail;

    /* Structural members */
    /* List of partition-nodes in this partition */
    nw_plugPartitionNode partitionNodesHead;
    nw_plugPartitionNode partitionNodesTail;
};

static nw_plugReceivingPartition
nw_plugReceivingPartitionNew(
    nw_partitionId partitionId,
    os_uint32 hash);

static void
nw_plugReceivingPartitionFree(
    nw_plugReceivingPartition receivingPartition);

static void
nw_plugReceivingPartitionAddPartitionNode(
    nw_plugReceivingPartition receivingPartition,
    nw_plugPartitionNode plugPartitionNode);

static void
nw_plugReceivingPartitionResendItemCreate(
    nw_plugReceivingPartition receivingPartition,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev,
    nw_length bufferLength);

/* --------------------- PartitionNode ---------------------- */

NW_STRUCT(nw_plugPartitionNode) {

    /* Possible resends for this partition-node */
    /* The total number of possible resends */
    nw_length pendingResendCount;
    /* Start of list containing all possible resends */
    nw_resendItem pendingResendListHead;
    /* End of list containing all possible resends */
    nw_resendItem pendingResendListTail;

    /* List of acks that have to be sent to the network */
    nw_length pendingAckCount;
    /* The total number of ack messages that have been allocated */
    nw_length totalAckCount;
    /* The list of acks to be processed */
    nw_ackMessage pendingAckList;
    /* The list of unused ack messages that have been allocated */
    nw_ackMessage freeAckList;
    /* Keep administration about first ack received */
    nw_seqNr firstStartingNr;
    /* Keep administration about first ack sended */
    nw_seqNr firstSendAck;
    nw_seqNr repeatFirstAckCntDwn;

    /* Status: number of fragment buffers in use at the remote node */
    nw_length remoteRecvBuffer;
    nw_bool   remoteRecvBufferRefreshed;

    /* Structural relationships */
    nw_plugReceivingNode owner;
    nw_plugReceivingPartition receivingPartition;
    /* Linked list within partition */
    nw_plugPartitionNode nextInPartition;
};

static nw_plugPartitionNode
nw_plugPartitionNodeNew(
    nw_plugReceivingNode owner);

static void
nw_plugPartitionNodeFree(
    nw_plugPartitionNode plugPartitionNode);

static void
nw_plugPartitionNodeSetPartition(
    nw_plugPartitionNode plugPartitionNode,
    nw_plugReceivingPartition receivingPartition,
    nw_plugPartitionNode *prevNextInPartition);

static void
nw_plugPartitionNodeResendItemCreate(
    nw_plugPartitionNode partitionNode,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev,
    nw_length bufferLength);

static void
nw_plugPartitionNodeResendItemClone(
    nw_plugPartitionNode partitionNode,
    nw_resendItem resendItem,
    nw_resendItem *prevNextPtr,
    nw_resendItem *nextPrevPtr);

static void
nw_plugPartitionNodeDataSentReliably(
    nw_plugPartitionNode partitionNode,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_length bufferLength);

static void
nw_plugPartitionNodeDataReceivedReliably(
    nw_plugChannel channel,
    nw_plugPartitionNode partitionNode,
    nw_seqNr packetNr,
    nw_bool reconnectAllowed);

static void
nw_plugPartitionNodeAckMessageCreate(
    nw_plugPartitionNode partitionNode,
    nw_seqNr startingNr,
    nw_ackMessage *prevNext);

static void
nw_plugPartitionNodeAckMessageRelease(
    nw_plugPartitionNode partitionNode,
    nw_ackMessage ackMessage,
    nw_ackMessage *prevNext);

static void
nw_plugPartitionNodeSerializeAndReleaseAcks(
    nw_plugPartitionNode partitionNode,
    nw_plugControlMessage *controlMessages,
    nw_length maxMsgCount,
    nw_length *actualMsgCount,
    nw_bool *more);

/* ---------------------------- ResendItem ---------------------------------- */

NW_STRUCT(nw_resendItem) {
    nw_seqNr packetNr;
    nw_partitionId partitionId;  /* never used ?? */
    nw_seqNr controlFlushPass;
    nw_plugDataBuffer buffer;
    nw_resendItem next;
    nw_resendItem prev;
    nw_length bufferLength;
};
static nw_resendItem
nw_resendItemNew(void);

static void
nw_resendItemInit(
    nw_resendItem item,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev,
    nw_length bufferLength);

static void
nw_resendItemFree(
    nw_resendItem resendItem);



/* ---------------------------- Ack message --------------------------------- */

NW_STRUCT(nw_ackMessage) {
    nw_seqNr startingNr;
    nw_seqNr closingNr;
    nw_ackMessage next;
};

static nw_ackMessage
nw_ackMessageNew();

static void
nw_ackMessageInit(
    nw_ackMessage msg,
    nw_seqNr startinNr,
    nw_ackMessage *prevNext);

static void
nw_ackMessageFree(
    nw_ackMessage ackMessage);


/* ---------------------------- nw_plugSendChannel -------------------------- */

#define nw_plugSendChannel(o) ((nw_plugSendChannel)(o))

#define SET_INUSE(sendChannel, value) sendChannel->inUse = value


NW_STRUCT(nw_plugSendChannel) {
    NW_EXTENDS(nw_plugChannel);

    /** inUse indicates that message sending has started buf not flushed */
    nw_bool inUse;
    /** processingMessage indicates that we are between a Start and End */
    nw_bool processingMessage;
    /** Boolean is true if currently fragmenting a message */
    nw_bool fragmenting;

    /* Information concerning the current networking action */
    /** Number of fragments in current fragmentation action */
    nw_seqNr currentFragmentSeqNr;
    /** Number of messages currently in this fragment */
    nw_seqNr currentFragmentsInMsgCount;
    /** Number of messages after the last flush */
    nw_seqNr currentMsgCount;
    /** The length of the fragment currently being filled */
    nw_seqNr currentFragmentLength;

    /** Pointer to the last messageHolder being returned */
    nw_messageHolder lastMessage;

    /** Total number of messages sent by this channel */
    nw_seqNr totalMsgCount;
    /** The dataBuffer currently used for writing */
    nw_plugDataBuffer currentWriteBuffer;
    /** Caching the end of the buffer for fast calculation of remaining space */
    nw_data currentWriteBufferEnd;
    /** Caching the partition we are currently writing to */
    nw_plugReceivingPartition currentPartition;
    /** Caching the destination address we are currently writing to (if any) */
    nw_networkId currentDestination;
    /** A scratch buffer for writing control messages (acks) */
    nw_plugControlBuffer controlWriteBuffer;
    /** Caching the end of the buffer for fast calculation of remaining space */
    nw_data controlWriteBufferEnd; /* obsolete? */

    /** A count for the number of free buffers in the pool */
    nw_seqNr nofFreeRelBuffers;
    /** A count of all reliability admin buffers  */
    nw_seqNr totNofRelBuffers;
    /** A list of free buffers to avoid exhaustive allocation and de-allocation */
    nw_plugDataBufferReliabilityAdmin freeRelBufferList;

    /** A list of buffers waiting to be flushed to the network */
    nw_plugDataBufferReliabilityAdmin flushableOldest;
    nw_plugDataBufferReliabilityAdmin flushableNewest;

    /* Relilability administration starts here */
    /** Hashtable containing all known remote nodes */
    nw_seqNr receivingNodesHashSize;
    nw_plugReceivingNode *receivingNodesHash;
    /** Linked list containing all known remote nodes */
    nw_plugReceivingNode receivingNodesHead;
    nw_plugReceivingNode receivingNodesTail;

    /** Array containing all known networking partitions */
    nw_plugReceivingPartition *receivingPartitions;
    /** For double checking only */
    nw_seqNr nofReceivingPartitions;

    /** Parameters for resending behavious */
    nw_seqNr recoveryFactor;
    nw_seqNr maxRetries;

    /** Current number of fragment buffers in use in the recv-thread */
    nw_length RecvBufferInUse;

    /** Parameters for traffic limitation */
    nw_length currentPeriodBudget;
    nw_length upperThrottleLimit;
    nw_length lowerThrottleLimit;
    nw_length throttleThreshold;
    nw_length throttleParamP;
    nw_length throttleParamD;
    nw_length throttlevalueHist1;
    nw_length throttleValueHist2;
    nw_length throttleMedian;
    nw_signedLength lastPeriodCredits;

    /** Resending statistics */
    nw_seqNr totalResendCount;
    nw_seqNr reportInterval;
    plugSendStatistics pss;
    nw_bool crc_check;

#ifdef NW_DEBUGGING
    os_time nextPrintTime;
#endif

    NW_SECURITY_ENCODER_HOOK_DECL
};


/* static */
/* nw_data */
/* nw_plugSendChannelBufferEnd(nw_plugDataBuffer buffer,  */
/*              nw_plugSendChannel sendChannel, */
/*              ) */
/* { */

/* } */

static void
nw_plugSendChannelInitializeDataBuffer(
    nw_plugSendChannel sendChannel,
    nw_plugDataBuffer buffer);

static void
nw_plugSendChannelCreateReceivingPartitions(
    nw_plugSendChannel sendChannel,
    nw_plugPartitions partitions);

nw_plugChannel
nw_plugSendChannelNew(
    nw_seqNr seqNr,
    nw_networkId nodeId,
    nw_plugPartitions partitions,
    nw_userData *userDataPtr,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    nw_plugSendChannel result;
    os_uint32 i;
    c_char * tmpPath;
    nw_length dataLength;
    nw_plugDataBufferReliabilityAdmin admin;
    nw_size currentPeriodBudget;

    result = (nw_plugSendChannel)os_malloc(sizeof(*result));
    if (result != NULL) {
        /* Initialize parent */
        nw_plugChannelInitialize(nw_plugChannel(result), seqNr, nodeId,
            NW_COMM_SEND, partitions, userDataPtr, pathName,onFatal,onFatalUsrData);

        /* Initialize self */
        SET_INUSE(result, FALSE);
        result->processingMessage = FALSE;
        result->fragmenting = FALSE;

        result->currentFragmentSeqNr = 0;
        result->currentFragmentsInMsgCount = 0;
        result->currentMsgCount = 0;
        result->currentFragmentLength = 0;
        result->lastMessage = NULL;

        result->totalMsgCount = 0;

        result->nofFreeRelBuffers = 0;
        result->totNofRelBuffers = 0;
        result->freeRelBufferList = NULL;

        result->flushableNewest = NULL;
        result->flushableOldest = NULL;

        NW_SECURITY_ENCODER_INIT(result, nodeId, partitions);

        result->controlWriteBuffer =
            (nw_plugControlBuffer)os_malloc(nw__plugChannelGetFragmentLength(nw_plugChannel(result)));
        /* Same as writeBuffer, reserve space at end of buffer to hold any size
         * of configured security header */
        result->controlWriteBufferEnd = (nw_data)(UI(result->controlWriteBuffer) +
            nw__plugChannelGetFragmentLength(nw_plugChannel(result)) -
            NW_SECURITY_RESERVED_BUFFER_SPACE(result));

        result->receivingNodesHashSize = NW_RECEIVING_NODES_HASHSIZE;
        result->receivingNodesHash = (nw_plugReceivingNode *)os_malloc(
            result->receivingNodesHashSize * sizeof(*result->receivingNodesHash));
        for (i=0; i<result->receivingNodesHashSize; i++) {
            result->receivingNodesHash[i] = NULL;
        }
        result->receivingNodesHead = NULL;
        result->receivingNodesTail = NULL;

        nw_plugSendChannelCreateReceivingPartitions(result, partitions);

        tmpPath = os_malloc(strlen(pathName) + strlen(NWCF_SEP) +
                            strlen(NWCF_ROOT(Tx)) + 1);
        os_sprintf(tmpPath, "%s%s%s", pathName, NWCF_SEP, NWCF_ROOT(Tx));
        result->recoveryFactor = NWCF_SIMPLE_PARAM(ULong, tmpPath, RecoveryFactor);
        result->maxRetries = NWCF_SIMPLE_PARAM(ULong, tmpPath, MaxRetries);
        result->reportInterval = NWCF_SIMPLE_PARAM(ULong, tmpPath, ReportInterval);
        if (result->reportInterval < NWCF_MIN(ReportInterval)) {
            /* Do not report anything, silently change value */
            result->reportInterval = NWCF_MIN(ReportInterval);
        }
        result->totalResendCount = 0;

        /*
         * There use to be different default values for MaxBurstSize for reliable and non-reliable channels.
         * Now both type default to the same value, which is supposed to be an "unlimited" value.
         */
        currentPeriodBudget = (nw_size)NWCF_SIMPLE_PARAM(Size, tmpPath, MaxBurstSize);
        if (currentPeriodBudget > NWCF_MAX(MaxBurstSize)) {
            result->currentPeriodBudget = NWCF_MAX(MaxBurstSize);
        }
        else if (currentPeriodBudget < NWCF_MIN(MaxBurstSize)) {
            result->currentPeriodBudget = NWCF_MIN(MaxBurstSize);
        }
        else
        {
            result->currentPeriodBudget = (nw_length)currentPeriodBudget;
        }


        result->throttleThreshold = NWCF_SIMPLE_PARAM(ULong, tmpPath, ThrottleThreshold);
        result->lowerThrottleLimit = NWCF_SIMPLE_PARAM(ULong, tmpPath, ThrottleLimit);
        if (result->lowerThrottleLimit <= NWCF_MIN(ThrottleLimit)) {
            result->lowerThrottleLimit = NWCF_MIN(ThrottleLimit);
        }
        if ( result->lowerThrottleLimit > result->currentPeriodBudget ) {
            result->lowerThrottleLimit = result->currentPeriodBudget; /* disabling throttling*/
        }

        result->upperThrottleLimit = result->currentPeriodBudget;

        result->throttleParamP = 100;  /* Hard-coded control-factor*/
        result->throttleParamD = 5;    /* Hard-coded control-factor*/
        result->throttleMedian = 0;
        result->throttlevalueHist1 = 0;
        result->throttleValueHist2 = 0;
        result->lastPeriodCredits = 0;
        os_free(tmpPath);

        result->RecvBufferInUse = 0;

        result->currentPartition = NULL;
        result->currentDestination = 0;

        admin = nw_plugSendChannelDataBufferReliabilityAdminCreate(result);
        result->currentWriteBuffer = REL_DATA(admin);

        dataLength = nw__plugChannelGetFragmentLength(nw_plugChannel(result)) -
                                NW_SECURITY_RESERVED_BUFFER_SPACE(result);
        result->currentWriteBufferEnd = (nw_data)(UI(result->currentWriteBuffer) + dataLength);

        /* crc check */
        result->crc_check = NWCF_DEFAULTED_SUBPARAM(Bool, pathName, Tx, CrcCheck,NWCF_DEF_CrcCheck);


#ifdef NW_DEBUGGING
        result->nextPrintTime = os_timeGet();
        result->nextPrintTime.tv_sec += SC_PRINT_EVERY;
#endif
    }

    return nw_plugChannel(result);
}


void
nw_plugSendChannelFree(
    nw_plugChannel channel)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugReceivingNode receivingNode;
    nw_partitionId partitionId;
    nw_plugDataBufferReliabilityAdmin relAdmin;

    if (channel != NULL) {
        /* own finalization */

        /* Unused buffers for resending */
        while (sendChannel->freeRelBufferList != NULL) {
            relAdmin = sendChannel->freeRelBufferList;
            sendChannel->freeRelBufferList = relAdmin->next;
            nw_plugDataBufferReliabilityAdminFree(relAdmin);
        }

        /* Fixed buffer */
        nw_plugDataBufferReliabilityAdminFree(REL_ADMIN(sendChannel->currentWriteBuffer));

        /* List with receiving nodes */
        while (sendChannel->receivingNodesHead != NULL) {
            receivingNode = sendChannel->receivingNodesHead;
            sendChannel->receivingNodesHead = receivingNode->nextInList;
            nw_plugReceivingNodeFree(receivingNode);
        }
        /* Hashtable */
        os_free(sendChannel->receivingNodesHash);

        /* Receiving partitions */
        for (partitionId = 0;
             partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
             partitionId++) {
            nw_plugReceivingPartitionFree(
                NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, partitionId));
        }
        os_free(sendChannel->receivingPartitions);

        NW_SECURITY_ENCODER_DEINIT(sendChannel);

        /* inherited finalization */
        nw_plugChannelFinalize(channel);
        os_free(channel);
    }
}


static void
nw_plugSendChannelCreateReceivingPartitions(
    nw_plugSendChannel sendChannel,
    nw_plugPartitions partitions)
{
    nw_partitionId partitionId;
    nw_partitionAddress partitionAddress;
    nw_bool found;
    nw_bool connected;
    nw_bool compression;
    os_uint32 hash;
    c_ulong mTTL;

    NW_CONFIDENCE(sendChannel != NULL);

    sendChannel->receivingPartitions = os_malloc(
        NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel) *
        sizeof(*sendChannel->receivingPartitions));
    for (partitionId = 0;
         partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
         partitionId++) {
        nw_plugPartitionsGetPartition(partitions, partitionId,
            &found, &partitionAddress, NULL, &connected, &compression, &hash, &mTTL);

        NW_CONFIDENCE(found);
        if (found) {
            NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, partitionId) =
                nw_plugReceivingPartitionNew(partitionId, hash);
            nw_socketAddPartition(nw__plugChannelGetSocket(
                nw_plugChannel(sendChannel)), partitionId, partitionAddress,
                connected, compression, FALSE, mTTL);
        } else {
            NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, partitionId) = NULL;
        }
    }
}

#ifdef NW_DEBUGGING
#define SC_PRINT_ATTR_GEN(format, a, attrName) \
        printf("  "#attrName": "format"\n", a->attrName)

#define SC_PRINT_ATTR_UINT(a, attrName) \
        SC_PRINT_ATTR_GEN("%u", a, attrName)

#define SC_PRINT_ATTR(type, a, attrName) \
        SC_PRINT_ATTR_##type(a, attrName)

#endif


static nw_plugReceivingNode
nw_plugSendChannelLookupReceivingNode(
    nw_plugSendChannel sendChannel,
    nw_seqNr sendingNodeId)
{
    nw_plugReceivingNode result = NULL;
    nw_plugReceivingNode current;
    nw_plugReceivingNode *currentPtr;
    nw_bool done = FALSE;

    currentPtr = &(sendChannel->receivingNodesHash[sendingNodeId % sendChannel->receivingNodesHashSize]);
    current = *currentPtr;
    while (!done && (current != NULL)) {
        if (current->nodeId > sendingNodeId) {
            /* Not found yet, look further */
            currentPtr = &(current->nextInHash);
            current = *currentPtr;
        } else if (current->nodeId == sendingNodeId) {
            /* Found! */
            result = current;
            done = TRUE;
        } else {
            /* Not found but new node to be inserted because the list is ordered */
            done = TRUE;
        }
    }

    return result;
}


static nw_plugReceivingNode
nw_plugSendChannelCreateReceivingNode(
    nw_plugSendChannel sendChannel,
    nw_seqNr sendingNodeId,
    os_sockaddr_storage sendingAddress,
    nw_plugReceivingNode *prevNextInHash,
    nw_plugReceivingNode *prevNextInList)
{
    nw_plugReceivingNode result;
    nw_plugReceivingPartition partition;
    nw_partitionId partitionId;
    nw_plugPartitionNode partitionNode;

    result = nw_plugReceivingNodeNew(sendChannel,sendingNodeId, sendingAddress,
        NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel),
        prevNextInHash, prevNextInList);

    if (sendChannel->pss != NULL) {
        if (sendChannel->pss->enabled) {
            sendChannel->pss->numberOfKnownNodes++;
        }
    }

    for (partitionId = 0;
         partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
         partitionId++) {
        partition = NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, partitionId);
        if (partition != NULL) {
            partitionNode = nw_plugReceivingNodeCreatePartitionNode(result, partitionId);
            nw_plugReceivingPartitionAddPartitionNode(partition, partitionNode);
        }
    }

    return result;
}

static nw_plugReceivingNode
nw_plugSendChannelLookupOrCreateReceivingNode(
    nw_plugSendChannel sendChannel,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingAddress)
{
    nw_plugReceivingNode current;
    nw_plugReceivingNode *currentPtr;
    nw_plugReceivingNode *prevNextPtr;
    nw_plugReceivingNode result = NULL;
    nw_bool done = FALSE;

    currentPtr = &(sendChannel->receivingNodesHash[sendingNodeId % sendChannel->receivingNodesHashSize]);
    current = *currentPtr;
    while (!done && (current != NULL)) {
        if (current->nodeId > sendingNodeId) {
            /* Not found yet, look further */
            currentPtr = &(current->nextInHash);
            current = *currentPtr;
        } else if (current->nodeId == sendingNodeId) {
            /* Found! */
            result = current;
            done = TRUE;
        } else {
            /* Not found but new node to be inserted because the list is ordered */
            done = TRUE;
        }
    }

    if (result == NULL) {
        /* Not found, insert or append new receivingNode */
        NW_CONFIDENCE(result == NULL);
        if (sendChannel->receivingNodesHead == NULL) {
            prevNextPtr = &sendChannel->receivingNodesHead;
        } else {
            NW_CONFIDENCE(sendChannel->receivingNodesTail != NULL);
            prevNextPtr = &(sendChannel->receivingNodesTail->nextInList);
        }
        result = nw_plugSendChannelCreateReceivingNode(sendChannel,
            sendingNodeId, sendingAddress, currentPtr, prevNextPtr);

        if (prevNextPtr == &sendChannel->receivingNodesHead) {
            sendChannel->receivingNodesTail = result;
            NW_CONFIDENCE(sendChannel->receivingNodesHead == sendChannel->receivingNodesTail);
        }

    }

    NW_CONFIDENCE(result != NULL);

    return result;
}


static nw_plugPartitionNode
nw_plugSendChannelLookupOrCreatePartitionNode(
    nw_plugSendChannel sendChannel,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    os_sockaddr_storage sendingAddress)
{
    nw_plugPartitionNode result;
    nw_plugReceivingNode receivingNode;

    receivingNode = nw_plugSendChannelLookupOrCreateReceivingNode(sendChannel,
        sendingNodeId, sendingPartitionId, sendingAddress);

    /* Once the receivingNode is found or created, the plugPartitionNode
     * is just an entry in an array */
    result = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, sendingPartitionId);

    return result;
}


static void
nw_plugReceivingPartitionDataSentReliably(
    nw_plugReceivingPartition receivingPartition,
    nw_seqNr packetSeqNr,
    nw_plugDataBuffer dataBuffer,
    nw_length dataBufferLength)
{
    nw_plugPartitionNode currentNode;
    nw_resendItem* prevNextPtr;
    nw_seqNr receivingNodeId;

    /* Store the buffer in the list of items that possibly have to be
     * resent. */
    if (receivingPartition->lateJoiningResendListTail == NULL) {
        NW_CONFIDENCE(receivingPartition->lateJoiningResendListHead == NULL);
        prevNextPtr = &receivingPartition->lateJoiningResendListHead;
    } else {
        NW_CONFIDENCE(receivingPartition->lateJoiningResendListHead != NULL);
        prevNextPtr = &receivingPartition->lateJoiningResendListTail->next;
    }
    nw_plugReceivingPartitionResendItemCreate(receivingPartition, packetSeqNr,
        dataBuffer, prevNextPtr, &receivingPartition->lateJoiningResendListTail,
        dataBufferLength);

    receivingNodeId = nw_plugBufferGetReceivingNodeId(nw_plugBuffer(dataBuffer));

    /* Walk over all receiving partitionnodes and insert buffer */
    currentNode = receivingPartition->partitionNodesHead;
    NW_TRACE_1(Test, 6, "Packet %x reliably sent", packetSeqNr);
    while (currentNode != NULL) {
        if ( receivingNodeId == 0 || receivingNodeId == currentNode->owner->nodeId){
            /* if this is a "write_to" packet, only add it for the intended node */
            if (currentNode->firstStartingNr != 0) {
                /* Only administrate this if the node is known to be
                  connected to this partition */
                nw_plugPartitionNodeDataSentReliably(currentNode,
             packetSeqNr, dataBuffer, dataBufferLength);
            }
        }
        currentNode = currentNode->nextInPartition;
    }
}


static void
nw_plugSendChannelInitializeDataBuffer(
    nw_plugSendChannel sendChannel,
    nw_plugDataBuffer buffer)
{
    nw_bool reliable;

#ifdef NW_DEBUGGING
    /* initialize buffer with known value, so that one can tell in visual debugger the
     * undefined space. */
    memset((void*) buffer, 'X', nw__plugChannelGetFragmentLength(nw_plugChannel(sendChannel)));
#endif

    nw_plugBufferSetVersion(nw_plugBuffer(buffer),NW_CURRENT_PROTOCOL_VERSION);
    nw_plugBufferClearFlags(nw_plugBuffer(buffer));
    nw_plugBufferSetSendingNodeId(nw_plugBuffer(buffer),
        nw__plugChannelGetNodeId(nw_plugChannel(sendChannel)));
    reliable = (nw__plugChannelGetReliabilityOffered(nw_plugChannel(sendChannel)) == NW_REL_RELIABLE);
    nw_plugBufferSetReliabilityFlag(nw_plugBuffer(buffer), reliable);
    /* For now, set these to 0 because the corresponding QoS-es are not
     * yet implemented */
    nw_plugBufferSetReceivingNodeId(nw_plugBuffer(buffer), 0);
    nw_plugBufferSetP2PFlag(nw_plugBuffer(buffer), FALSE);
    nw_plugBuffer(buffer)->crc=0;

#ifdef NW_DEBUGGING
    nw_plugDataBufferSetPacketNr(buffer, NW_VALUE_DONT_CARE);
    nw_plugDataBufferSetFragmentedMsgNr(buffer, NW_VALUE_DONT_CARE);
    nw_plugDataBufferSetFragmentNr(buffer, NW_VALUE_DONT_CARE);
    nw_plugDataBufferSetTerminatedMsgNr(buffer, NW_VALUE_DONT_CARE);
    nw_plugDataBufferSetTerminatingFragmentNr(buffer, NW_VALUE_DONT_CARE);
#endif
}

static void
nw_plugSendChannelInitializeControlBuffer(
    nw_plugSendChannel sendChannel,
    nw_plugControlBuffer buffer)
{
    nw_plugBufferSetVersion(nw_plugBuffer(buffer),NW_CURRENT_PROTOCOL_VERSION);
    nw_plugBufferClearFlags(nw_plugBuffer(buffer));
    nw_plugBufferSetSendingNodeId(nw_plugBuffer(buffer),
        nw__plugChannelGetNodeId(nw_plugChannel(sendChannel)));
    nw_plugBufferSetReliabilityFlag(nw_plugBuffer(buffer), NW_REL_BEST_EFFORT);
    nw_plugBufferSetControlFlag(nw_plugBuffer(buffer), TRUE);

    nw_plugControlBufferSetRecvBufferInUse(buffer,sendChannel->RecvBufferInUse);
}

/* Currently being used for encryption, but could also be used for
   scrambling the buffer by permutations to protect against
   re-engineering */
static nw_bool
nw_plugSendChannelBufferEncoding(nw_plugChannel channel,
                 nw_partitionId partitionId,
                 void *mesg,
                 nw_length *mesgLength) /* in/out */
{
    /* read fragment length from channel-attribute */
    nw_bool succeeded = OS_TRUE;

    NW_SECURITY_ENCODER_PROCESS(succeeded, nw_plugSendChannel(channel), partitionId, mesg, mesgLength, nw__plugChannelGetFragmentLength(channel));

    return succeeded;
}

/* Do a flush of data messages to the network
 * The function returns true if all bytes were sent, false otherwise */
nw_bool
nw_plugSendChannelMessagesFlush(
    nw_plugChannel channel,
    nw_bool all,
    nw_signedLength *bytesLeft, /* in/out */
    plugSendStatistics pss)
{
    nw_bool result = FALSE;
    c_ulong bytesSent;
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_bool reliable;
    nw_plugDataBufferReliabilityAdmin currentBuffer;
    nw_plugReceivingPartition partition;
    nw_seqNr packetSeqNr;
    nw_seqNr receivingNodeId;
    nw_plugReceivingNode receivingNode;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_SEND);
    NW_CONFIDENCE(!sendChannel->processingMessage);


    /* Only do something if we have written any messages */
    if (all && (sendChannel->currentMsgCount > 0) && sendChannel->inUse) {
        nw_plugSendChannelPushCurrentWriteBuffer(sendChannel,
                                                 sendChannel->currentFragmentLength);
        SET_INUSE(sendChannel, FALSE);
        sendChannel->lastMessage = NULL;
    }

    /* Walk over all buffers in the flushable list and send them to the network */
    reliable = (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE);

    if (*bytesLeft > 0) {
        bytesSent = 0;
        currentBuffer = nw_plugSendChannelFlushableBufferPop(sendChannel);
        while ((currentBuffer != NULL) && (*bytesLeft > 0)) {

            nw_plugDataBuffer buffer = REL_DATA(currentBuffer);

            /* Read attributes from buffer before the buffer is encoded */

            NW_STAMP(buffer,NW_BUF_TIMESTAMP_FLUSH);
            bytesSent = nw_plugBufferGetLength(nw_plugBuffer(buffer));

            /** The packet sequence number must be read from byte buffer
             * before encrypted. */
            packetSeqNr = nw_plugDataBufferGetPacketNr(buffer);

            /* read partitionId from buffer, it could be that there are still serialized buffers on
             * the send stack that have not yet been sent, but that the channel is already serializing a message
             * for another partition after it. */
            partition = NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel,nw_plugDataBufferGetPartitionId(buffer));

             /*partition id name to hash*/
             nw_plugDataBufferSetPartitionId(buffer,partition->partitionHash);

             /* Valid Partition so Send data to network */
             receivingNodeId = nw_plugBufferGetReceivingNodeId(nw_plugBuffer(buffer));

             /* crc check */
             if (sendChannel->crc_check) {
                 nw_plugBuffer(buffer)->crc = ut_crcCalculate(channel->crc,((os_char*)buffer)+sizeof(NW_STRUCT(nw_plugBuffer)),bytesSent-sizeof(NW_STRUCT(nw_plugBuffer)));
             }

            /* Encode the buffer before sent to network. If the encoding fails due to malicious configuration
             * the message will be dropped. If dealing with reliability, the message-buffer will not be resent. */
            if (!nw_plugSendChannelBufferEncoding(channel, partition->partitionId, buffer, /* (void*) */
                                                  (nw_length*)&bytesSent)) /* in/out, will increment length by header-size */
            {
               /* silent dropping of buffer */
               NW_REPORT_WARNING("nw_plugSendChannelFlushSingleWriteBuffer",
                     "encoding failure, dropping message buffer");
               bytesSent = 0;
            } else {
                /* never reached, if encoding failed */

                if (!(os_int)nw_configurationLoseSentMessage()) {

                    /*
                    if (nw_plugDataBufferGetPartitionId(buffer) !=
                        partition->partitionId)
                    {
                        NW_REPORT_ERROR_2("nw_plugSendChannelMessagesFlush",
                                "Partition Id of Channel (%u) differs from buffer (%u)",
                                 partition->partitionId,
                                 nw_plugDataBufferGetPartitionId(buffer));
                    }*/



                    if ( receivingNodeId == 0 ){
                        nw_periodicCheckMessageBox(channel); /* make sure to have processed all messages, as they might contain the mapping for this write_to*/
                        nw_socketSendDataToPartition(
                            nw__plugChannelGetSocket(channel),
                            partition->partitionId,
                            pss,
                            buffer,
                            (sk_length*)&bytesSent); /* in/out, might decrement due to compression */
                    } else {
                        nw_periodicCheckMessageBox(channel); /* make sure to have processed all messages, as they might contain the mapping for this write_to*/
                        receivingNode = nw_plugSendChannelLookupReceivingNode(
                                                             sendChannel,receivingNodeId);
                        if (receivingNode != NULL ) {
                            nw_socketSendDataTo(nw__plugChannelGetSocket(channel),
                                receivingNode->address,
                                buffer,
                                bytesSent);
                        } else {

                            NW_TRACE_1(Send, 3, "nw_plugSendChannelMessagesFlush: Trying to send data to an unknown hostId 0x%x",receivingNodeId);
                            NW_REPORT_WARNING_1("nw_plugSendChannelMessagesFlush",
                                                "Trying to send data to an unknown hostId 0x%x",
                                                 receivingNodeId);
                        }
                    }
                    if (pss != NULL) {
                        if (pss->enabled) {
                            pss->numberOfPacketsSent++;
                            pss->numberOfBytesSent += bytesSent;
                        }
                    }

                }
                /* only insert data in the reliability admin if it one of our own.
                 * data resend on behalf of another died node, is not inserted
                 */
                if (reliable  &&
                      (nw__plugChannelGetNodeId(nw_plugChannel(sendChannel)) ==
                       nw_plugBufferGetSendingNodeId(nw_plugBuffer(buffer)))) {

                    /* Insert data into resend admin */
                    nw_plugReceivingPartitionDataSentReliably(partition,
                                                              packetSeqNr,
                                                              buffer,
                                                              bytesSent);
                }
            }

            /* Release current buffer */
            nw_plugDataBufferReliabilityAdminRelease(currentBuffer);

            *bytesLeft -= bytesSent;
            if (*bytesLeft > 0) {
                currentBuffer = nw_plugSendChannelFlushableBufferPop(sendChannel);
            }
        }
        /* POST: bytesLeft <= 0 || currentBuffer == NULL */

        if (*bytesLeft > 0) {
            result = TRUE;
        } else {
            result = (sendChannel->flushableNewest == NULL);
        }
    }

    if (result) {
        sendChannel->currentPartition = NULL;
        sendChannel->currentDestination = 0;
    }

    return result;
}



/* Starting the writing of a message with a given destination node,
 * returns a buffer of length to write into */
nw_bool
nw_plugSendChannelWriteToMessageStart(
    nw_plugChannel channel,
    nw_data *buffer,
    nw_length *length,
    nw_partitionId partitionId,
    nw_networkId destination,
    nw_signedLength *bytesLeft, /* in/out */
    plugSendStatistics pss)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugReceivingPartition partition;
    nw_length sendLength;
    nw_bool result = TRUE;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_SEND);
    NW_CONFIDENCE(!sendChannel->processingMessage);
    NW_CONFIDENCE(buffer != NULL);
    NW_CONFIDENCE(length != NULL);

    sendChannel->pss = pss;


    NW_TRACE_1(Send, 5, "nw_plugSendChannelWriteToMessageStart: Sending data to  hostId 0x%x",destination);

    if (sendChannel->currentPartition != NULL) {
        if ((partitionId != sendChannel->currentPartition->partitionId) ||
            (destination != sendChannel->currentDestination)){
            /* Channel contains message(s) for another partition.
             * Flush the buffer and set new partition Id.
             */
            nw_plugSendChannelMessagesFlush(channel,TRUE,bytesLeft, pss);
            partition = NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, partitionId);
            if (partition == NULL) {
                result = FALSE; /* no existing partition. */
            } else {
                sendChannel->currentPartition = partition;
                sendChannel->currentDestination = destination;
            }
        }
    } else {
        partition = NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, partitionId);
        if (partition == NULL) {
            result = FALSE; /* no existing partition. */
        } else {
            sendChannel->currentPartition = partition;
            sendChannel->currentDestination = destination;
        }
    }

    if (result == TRUE) {
        sendChannel->processingMessage = TRUE;
        if (!sendChannel->inUse) {
            SET_INUSE(sendChannel, TRUE);

            sendChannel->currentMsgCount = 0;
            sendChannel->currentFragmentsInMsgCount = 1;
            sendChannel->lastMessage =
                NW_PLUGDATABUFFER_FIRSTMESSAGE(sendChannel->currentWriteBuffer);
            sendChannel->currentFragmentLength =
                NW_PLUGDATABUFFER_DIFF(sendChannel->currentWriteBuffer,
                                       NW_MESSAGEHOLDER_DATA(sendChannel->lastMessage));
        } else {
            NW_CONFIDENCE(sendChannel->lastMessage != NULL);
            *length = NW_PLUGDATABUFFER_DIFF(sendChannel->lastMessage,
                                             sendChannel->currentWriteBufferEnd);

            /* 44 = message header size */
            if (*length < 44) {
                nw_plugBufferSetLength(nw_plugBuffer(
                     sendChannel->currentWriteBuffer),
                     sendChannel->currentFragmentLength);
                nw_plugDataBufferSetNrOfMessages(
                     sendChannel->currentWriteBuffer,
                     sendChannel->currentFragmentsInMsgCount);

                sendLength = nw__plugChannelGetFragmentLength(channel) -
                             *length -
                             NW_SECURITY_RESERVED_BUFFER_SPACE(sendChannel);

                nw_plugSendChannelPushCurrentWriteBuffer(sendChannel, sendLength);

                /* Sending done, now initialize the fragment for reuse */
                sendChannel->currentFragmentsInMsgCount = 1;
                sendChannel->lastMessage =
                    NW_PLUGDATABUFFER_FIRSTMESSAGE(sendChannel->currentWriteBuffer);
                sendChannel->currentFragmentLength =
                    NW_PLUGDATABUFFER_DIFF(sendChannel->currentWriteBuffer,
                                           NW_MESSAGEHOLDER_DATA(sendChannel->lastMessage));
            } else {
                sendChannel->currentFragmentsInMsgCount++;
                sendChannel->currentFragmentLength += NW_MESSAGEHOLDER_SIZE;
                if (sendChannel->pss != NULL) {
                    if (sendChannel->pss->enabled) {
                        sendChannel->pss->numberOfMessagesPacked++;
                    }
                }
            }
        }
        NW_CONFIDENCE(sendChannel->lastMessage != NULL);
        *buffer = NW_MESSAGEHOLDER_DATA(sendChannel->lastMessage);
        *length = NW_PLUGDATABUFFER_DIFF(*buffer, sendChannel->currentWriteBufferEnd);
    } else {
        NW_REPORT_WARNING_1("nw_plugSendChannelMessageStart",
                            "Trying to send data to disconnected partition %u",
                             partitionId);
    }
    return result;
}

/* #define nw_plugSendChannelMessageStart(c,b,l,p,bl,p) nw_plugSendChannelWriteToMessageStart(c,b,l,p,0,bl,p);
*/

/* Starting the writing of a message, returns a buffer of length to write into */
nw_bool
nw_plugSendChannelMessageStart(
    nw_plugChannel channel,
    nw_data *buffer,
    nw_length *length,
    nw_partitionId partitionId,
    nw_signedLength *bytesLeft, /* in/out */
    plugSendStatistics pss)
{
    return nw_plugSendChannelWriteToMessageStart(channel,buffer,length, partitionId,0,bytesLeft,pss);
}


/* Retrieve a buffer for copying data into if the previous buffer was not
 * sufficiently long. NOTE: This function currently flushes the available
 * data to the network */
void
nw_plugSendChannelGetNextFragment(
    nw_plugChannel channel,
    nw_data *buffer,
    nw_length *length) /* remaining Length in buffer */
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_length messageLength;
    nw_length sendLength;
    nw_bool reliable;
    nw_plugReceivingPartition partition;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_SEND);
    NW_CONFIDENCE(sendChannel->inUse);
    NW_CONFIDENCE(sendChannel->processingMessage);
    NW_CONFIDENCE(sendChannel->lastMessage != NULL);

    partition = sendChannel->currentPartition;
    NW_CONFIDENCE(partition != NULL);


    /* First fill the last length field */
    messageLength = NW_PLUGDATABUFFER_DIFF(
        NW_MESSAGEHOLDER_DATA(sendChannel->lastMessage), *buffer);
    /*nw_messageHolderSetLength(sendChannel->lastMessage, messageLength);*/

    /* Note: Do not add padding bytes by aligning the messageLength value to
     * next higher multiple of 4, as this will cause faults on receiver side
     * deserializing fragmented strings */
    nw_messageHolderSetLength(sendChannel->lastMessage, messageLength);

    if (!sendChannel->fragmenting) {
        /* Not currently fragmenting a message */
        /* Increase fragmented seqnr for this partition */
        partition->fragmentedSeqNr++;
        /* Update current information for sendChannel */
        sendChannel->currentFragmentSeqNr = 1;
        sendChannel->fragmenting = TRUE;
        if (sendChannel->pss != NULL) {
              if (sendChannel->pss->enabled) {
                  sendChannel->pss->numberOfMessagesFragmented++;
              }
        }
    } else {
        NW_CONFIDENCE(sendChannel->currentFragmentSeqNr > 0);
        sendChannel->currentFragmentSeqNr++;
    }

    /* Note: Do not add padding bytes by aligning the messageLength value to
     * next higher multiple of 4, as this will cause faults on receiver side
     * deserializing fragmented strings */
    sendChannel->currentFragmentLength =
        sendChannel->currentFragmentLength + messageLength;

    nw_plugBufferSetLength(
        nw_plugBuffer(sendChannel->currentWriteBuffer),
        sendChannel->currentFragmentLength);

    nw_plugDataBufferSetNrOfMessages(
        sendChannel->currentWriteBuffer,
        sendChannel->currentFragmentsInMsgCount);

    nw_plugBufferSetFragmentedFlag(
        nw_plugBuffer(sendChannel->currentWriteBuffer),
        TRUE);

    nw_plugDataBufferSetFragmentedMsgNr(
        sendChannel->currentWriteBuffer,
        partition->fragmentedSeqNr);

    nw_plugDataBufferSetFragmentNr(
        sendChannel->currentWriteBuffer,
        sendChannel->currentFragmentSeqNr);

    reliable = (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE);
    sendLength = nw__plugChannelGetFragmentLength(channel) -
                 *length -
                 NW_SECURITY_RESERVED_BUFFER_SPACE(sendChannel);

    nw_plugSendChannelPushCurrentWriteBuffer(sendChannel, sendLength);

    /* Sending done, now initialize the fragment for reuse */
    sendChannel->currentFragmentsInMsgCount = 1;
    sendChannel->lastMessage =
        NW_PLUGDATABUFFER_FIRSTMESSAGE(sendChannel->currentWriteBuffer);

    sendChannel->currentFragmentLength =
        NW_PLUGDATABUFFER_DIFF(sendChannel->currentWriteBuffer,
                               NW_MESSAGEHOLDER_DATA(sendChannel->lastMessage));

    NW_CONFIDENCE(sendChannel->lastMessage != NULL);

    *buffer = NW_MESSAGEHOLDER_DATA(sendChannel->lastMessage);
    *length = NW_PLUGDATABUFFER_DIFF(*buffer, sendChannel->currentWriteBufferEnd);
}


/** Indicate the end of a message */
void
nw_plugSendChannelMessageEnd(
    nw_plugChannel channel,
    nw_data buffer,
    plugSendStatistics pss)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_length messageLength;
    nw_plugReceivingPartition partition;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_SEND);
    NW_CONFIDENCE(sendChannel->inUse);
    NW_CONFIDENCE(sendChannel->processingMessage);
    NW_CONFIDENCE(sendChannel->lastMessage != NULL);
    NW_CONFIDENCE(sendChannel->pss == pss);

    partition = sendChannel->currentPartition;
    NW_CONFIDENCE(partition != NULL);

    sendChannel->processingMessage = FALSE;
    sendChannel->currentMsgCount++;
    sendChannel->totalMsgCount++;

    /* First fill the last length field */
    messageLength = NW_PLUGDATABUFFER_DIFF(
        NW_MESSAGEHOLDER_DATA(sendChannel->lastMessage), buffer);
    /*nw_messageHolderSetLength(sendChannel->lastMessage, messageLength);*/

    nw_messageHolderSetLength(sendChannel->lastMessage,
            NW_ALIGN(NW_PLUGDATABUFFER_DATA_ALIGNMENT, messageLength));

    if (sendChannel->fragmenting) {
        /* This message data crosses the boundary of one or more fragments but
         * is now finished. Therefore, set the terminator flag and fill the
         * corresponding properties */
        NW_CONFIDENCE(sendChannel->currentFragmentSeqNr > 0);
        sendChannel->currentFragmentSeqNr++;
        nw_plugBufferSetTerminatorFlag(nw_plugBuffer(sendChannel->currentWriteBuffer), TRUE);
        nw_plugDataBufferSetTerminatedMsgNr(sendChannel->currentWriteBuffer, partition->fragmentedSeqNr);
        nw_plugDataBufferSetTerminatingFragmentNr(sendChannel->currentWriteBuffer, sendChannel->currentFragmentSeqNr);
        sendChannel->fragmenting = FALSE;
    }
    /* Set pointer to last message. Make sure that it is aligned correctly */

    sendChannel->lastMessage =
        (nw_messageHolder)NW_ALIGN(NW_PLUGDATABUFFER_DATA_ALIGNMENT, buffer);
    sendChannel->currentFragmentLength = sendChannel->currentFragmentLength +
        NW_ALIGN(NW_PLUGDATABUFFER_DATA_ALIGNMENT, messageLength);

    if (sendChannel->pss != NULL) {
        if (sendChannel->pss->enabled) {
            sendChannel->pss->numberOfPacketsInResendBuffer = sendChannel->totNofRelBuffers- sendChannel->nofFreeRelBuffers;
            sendChannel->pss->numberOfBytesInResendBuffer = sendChannel->pss->numberOfPacketsInResendBuffer * nw__plugChannelGetFragmentLength(channel);
        }
    }
    sendChannel->pss = NULL;
}


/* ------------------- nw_plugDataBufferReliabilityAdmin -------------------- */

static nw_plugDataBufferReliabilityAdmin
nw_plugSendChannelDataBufferReliabilityAdminCreate(
    nw_plugSendChannel sendChannel)
{
    nw_plugDataBufferReliabilityAdmin admin = NULL;
    nw_length dataLength;

    NW_CONFIDENCE(sendChannel != NULL);
    NW_CONFIDENCE((sendChannel->nofFreeRelBuffers == 0) ==
                  (sendChannel->freeRelBufferList == NULL));

    dataLength = nw__plugChannelGetFragmentLength(nw_plugChannel(sendChannel)) -
                 NW_SECURITY_RESERVED_BUFFER_SPACE(sendChannel);

    if (sendChannel->freeRelBufferList != NULL) {
        admin = sendChannel->freeRelBufferList;
        sendChannel->freeRelBufferList = admin->next;
        sendChannel->nofFreeRelBuffers--;
    } else {
        admin = (nw_plugDataBufferReliabilityAdmin)os_malloc(dataLength + REL_ADMIN_LENGTH + NW_SECURITY_RESERVED_BUFFER_SPACE(sendChannel));
        sendChannel->totNofRelBuffers++;
    }

    if (admin != NULL) {
        admin->prev = NULL;
        admin->next = NULL;
        admin->owningChannel = sendChannel;
        admin->usedCount = 1;
    }
    nw_plugSendChannelInitializeDataBuffer(sendChannel,
            REL_DATA(admin));

    return admin;
}

static void
nw_plugDataBufferReliabilityAdminInsert(
    nw_plugDataBufferReliabilityAdmin admin,
    nw_plugDataBufferReliabilityAdmin *prevNext,
    nw_plugDataBufferReliabilityAdmin *nextPrev)
{
    NW_CONFIDENCE(admin != NULL);

    if (prevNext != NULL) {
        admin->next = *prevNext;
        *prevNext = admin;
    } else {
        admin->next = NULL;
    }
    if (nextPrev != NULL) {
        admin->prev = *nextPrev;
        *nextPrev = admin;
    } else {
        admin->prev = NULL;
    }
}


static void
nw_plugDataBufferReliabilityAdminKeep(
    nw_plugDataBufferReliabilityAdmin admin)
{
    if (admin != NULL) {
        admin->usedCount++;
    }
}


static nw_bool
nw_plugDataBufferReliabilityAdminRelease(
    nw_plugDataBufferReliabilityAdmin admin)
{
    nw_bool result = FALSE;
    nw_plugSendChannel sendChannel;

    NW_CONFIDENCE(admin != NULL);

    if (admin != NULL) {
       admin->usedCount--;
       if (admin->usedCount == 0) {
           sendChannel = admin->owningChannel;
           NW_CONFIDENCE(sendChannel != NULL);
           if (sendChannel->freeRelBufferList != NULL) {
               nw_plugDataBufferReliabilityAdminInsert(admin,
                   &sendChannel->freeRelBufferList,
                   &sendChannel->freeRelBufferList->prev);
           } else {
               nw_plugDataBufferReliabilityAdminInsert(admin,
                   &sendChannel->freeRelBufferList, NULL);
           }
           result = TRUE;
           sendChannel->nofFreeRelBuffers++;
       }
    }
    return result;
}


/* ------------------- nw_plugDataBufferFlushable -------------------- */

static void
nw_plugSendChannelPushCurrentWriteBuffer(
    nw_plugSendChannel sendChannel,
    nw_length dataLength)
{
    nw_plugReceivingPartition partition;
    nw_plugDataBuffer buffer;
    nw_plugDataBufferReliabilityAdmin admin;
    nw_plugDataBufferReliabilityAdmin *prevNext;
    nw_plugDataBufferReliabilityAdmin *nextPrev;

    partition = sendChannel->currentPartition;
    NW_CONFIDENCE(partition != NULL);

    buffer = sendChannel->currentWriteBuffer;

    /* Fill in the remaining fields */
    partition->packetSeqNr++;
    nw_plugDataBufferSetPacketNr(buffer, partition->packetSeqNr);
    nw_plugDataBufferSetNrOfMessages(buffer, sendChannel->currentFragmentsInMsgCount);
    nw_plugBufferSetLength(nw_plugBuffer(buffer), dataLength);
    NW_STAMP(buffer,NW_BUF_TIMESTAMP_FILLED);

    nw_plugDataBufferSetPartitionId(buffer, partition->partitionId);
    nw_plugBufferSetReceivingNodeId(nw_plugBuffer(buffer), sendChannel->currentDestination);

    /* Move buffer into flushable buffers list */
    NW_CONFIDENCE((sendChannel->flushableNewest == NULL) ==
                  (sendChannel->flushableOldest == NULL));

    prevNext = &sendChannel->flushableNewest;
    if (sendChannel->flushableNewest == NULL) {
        nextPrev = &sendChannel->flushableOldest;
    } else {
        nextPrev = &(sendChannel->flushableNewest->prev);
    }
    nw_plugDataBufferReliabilityAdminInsert(REL_ADMIN(buffer), prevNext, nextPrev);

    /* Create new buffer for scratch */
    admin = nw_plugSendChannelDataBufferReliabilityAdminCreate(sendChannel);
    sendChannel->currentWriteBuffer = REL_DATA(admin);

    dataLength = nw__plugChannelGetFragmentLength(nw_plugChannel(sendChannel)) -
                                    NW_SECURITY_RESERVED_BUFFER_SPACE(sendChannel);
    sendChannel->currentWriteBufferEnd = (nw_data)(UI(sendChannel->currentWriteBuffer) + dataLength);
}

static void
nw_plugSendChannelPushCurrentBackupBuffer(
    nw_plugSendChannel sendChannel,
    nw_length dataLength,
    nw_plugDataBuffer buffer)
{
    nw_plugDataBufferReliabilityAdmin *prevNext;
    nw_plugDataBufferReliabilityAdmin *nextPrev;

    /* Move buffer into flushable buffers list */
    NW_CONFIDENCE((sendChannel->flushableNewest == NULL) ==
                  (sendChannel->flushableOldest == NULL));

    prevNext = &sendChannel->flushableNewest;
    if (sendChannel->flushableNewest == NULL) {
        nextPrev = &sendChannel->flushableOldest;
    } else {
        nextPrev = &(sendChannel->flushableNewest->prev);
    }
    nw_plugDataBufferReliabilityAdminInsert(REL_ADMIN(buffer), prevNext, nextPrev);

}

static nw_plugDataBufferReliabilityAdmin
nw_plugSendChannelFlushableBufferPop(
    nw_plugSendChannel sendChannel)
{
    nw_plugDataBufferReliabilityAdmin result = NULL;

    result = sendChannel->flushableOldest;
    if (result != NULL) {
        NW_CONFIDENCE(result->next == NULL);
        if (result->prev == NULL) {
            sendChannel->flushableNewest = NULL;
        } else {
            result->prev->next = NULL;
        }
        sendChannel->flushableOldest = result->prev;
    }

    return result;
}

/* --------------------------- nw_plugReceivingNode ------------------------- */


static nw_plugReceivingNode
nw_plugReceivingNodeNew(
    nw_plugSendChannel owner,
    nw_seqNr receivingNodeId,
    os_sockaddr_storage receivingAddress,
    nw_partitionId nofPartitions,
    nw_plugReceivingNode *prevNextInHash,
    nw_plugReceivingNode *prevNextInList)
{
    nw_plugReceivingNode result;
    char *addressString;
    c_ulong memSize;
    nw_partitionId partitionId;

    NW_CONFIDENCE(prevNextInHash != NULL);
    NW_CONFIDENCE(prevNextInList != NULL);

    result = (nw_plugReceivingNode)os_malloc(sizeof(*result));
    if (result != NULL) {
        result->owner = owner;
        result->state = NW_NODESTATE_RESPONDING;
        result->nodeId = receivingNodeId;
        result->address = receivingAddress;
        result->nextInHash = *prevNextInHash;
        *prevNextInHash = result;
        result->nextInList = *prevNextInList;
        *prevNextInList = result;

        addressString = os_sockaddrAddressToString((struct sockaddr*) &receivingAddress,
                                                    (char*) os_malloc(INET6_ADDRSTRLEN),
                                                    INET6_ADDRSTRLEN);

        /* Create the corresponding partitionNodes */
        result->nofPartitions = nofPartitions;
        memSize = result->nofPartitions * sizeof(*result->partitionNodes);
        result->partitionNodes = os_malloc(memSize);
        memset(result->partitionNodes, 0, memSize);
        for (partitionId = 0; partitionId < result->nofPartitions; partitionId++) {
            NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(result, partitionId) =
                nw_plugPartitionNodeNew(result);
        }

        NW_TRACE_2(Send, 1, "Created receiving node 0x%x (address %s)",
            receivingNodeId, addressString);
        os_free(addressString);
    }

    return result;
}


static nw_plugPartitionNode
nw_plugReceivingNodeCreatePartitionNode(
    nw_plugReceivingNode receivingNode,
    nw_partitionId partitionId)
{
    nw_plugPartitionNode result = NULL;

    result = nw_plugPartitionNodeNew(receivingNode);
    NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, partitionId) = result;

    return result;
}


static void
nw_plugReceivingNodeRemove(
    nw_plugReceivingNode receivingNode)
{
    nw_partitionId partitionId;
    nw_plugPartitionNode partitionNode;

    NW_CONFIDENCE(receivingNode != NULL);

    for (partitionId = 0; partitionId < receivingNode->nofPartitions; partitionId++) {
        partitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, partitionId);
        nw_plugPartitionNodeFree(partitionNode);
    }
}

static void
nw_plugReceivingNodeStopped(
    nw_plugReceivingNode receivingNode)
{
    char *addressString;

    NW_CONFIDENCE(receivingNode != NULL);

    if ((receivingNode != NULL) && (receivingNode->state == NW_NODESTATE_RESPONDING)) {
        receivingNode->state = NW_NODESTATE_STOPPED;
        addressString = os_sockaddrAddressToString((struct sockaddr*) &receivingNode->address,
                                                    (char*) os_malloc(INET6_ADDRSTRLEN),
                                                    INET6_ADDRSTRLEN);
        NW_TRACE_2(Send, 1,
            "Node 0x%x (address %s) stopped, removing it from the reliable protocol",
            receivingNode->nodeId, addressString);
        os_free(addressString);

        nw_plugReceivingNodeRemove(receivingNode);
    }
}

static void
nw_plugReceivingNodeNotResponding(
    nw_plugReceivingNode receivingNode)
{
    char *addressString;

    NW_CONFIDENCE(receivingNode != NULL);

    if ((receivingNode != NULL) && (receivingNode->state == NW_NODESTATE_RESPONDING)) {
        receivingNode->state = NW_NODESTATE_NOT_RESPONDING;
        addressString = os_sockaddrAddressToString((struct sockaddr*) &receivingNode->address,
                                                    (char*) os_malloc(INET6_ADDRSTRLEN),
                                                    INET6_ADDRSTRLEN);
        if (receivingNode->nodeId != 12345) { /*TODO neat fix for ignore ghost node 12345*/
            NW_REPORT_WARNING_2("reliable protocol",
                "Node 0x%x (address %s) not responding or no heartbeats received, removing it from the reliable protocol",
                receivingNode->nodeId, addressString);
            NW_TRACE_2(Send, 1,
                "Node 0x"PA_ADDRFMT" (address %s) no heartbeats received or not responding, removing it from the reliable protocol",
                /*receivingNode->nodeId*/(PA_ADDRCAST)receivingNode, addressString);
        }
        os_free(addressString);

        nw_plugReceivingNodeRemove(receivingNode);
    }
}


static void
nw_plugReceivingNodeFree(
    nw_plugReceivingNode receivingNode)
{
    nw_plugPartitionNode partitionNode;
    nw_partitionId partitionId;

    for (partitionId = 0; partitionId < receivingNode->nofPartitions; partitionId++) {
        partitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, partitionId);
        nw_plugPartitionNodeFree(partitionNode);
    }

    os_free(receivingNode->partitionNodes);
    os_free(receivingNode);
}

static void
nw_plugPartitionNodeDataSentReliably(
    nw_plugPartitionNode partitionNode,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_length bufferLength)
{
    nw_plugReceivingNode receivingNode;
    nw_resendItem *currentItemPtr;
    nw_resendItem currentItem;

    receivingNode = partitionNode->owner;
    NW_CONFIDENCE(receivingNode != NULL);
    /* Only execute action if this receiving node is still responding */
    if (receivingNode->state == NW_NODESTATE_RESPONDING) {
        /* Find the item to insert before */
        /* Since we expect this to be an append, start at tail */
        currentItemPtr = &partitionNode->pendingResendListTail;
        currentItem = *currentItemPtr;
        if (currentItem != NULL) {
            if (currentItem->packetNr < packetNr) {
                /* Yes, we have found it at the end */
                nw_plugPartitionNodeResendItemCreate(partitionNode, packetNr,
                    buffer, &currentItem->next,  currentItemPtr, bufferLength);
            } else {
                /* This should be impossible with the current design. */
                NW_CONFIDENCE(FALSE);
            }
        } else {
            NW_CONFIDENCE(partitionNode->pendingResendListHead == NULL);
            nw_plugPartitionNodeResendItemCreate(partitionNode, packetNr, buffer,
                &partitionNode->pendingResendListHead, currentItemPtr, bufferLength);

        }
    }
}

static void
nw_plugPartitionNodeDataReceivedReliably(
    nw_plugChannel channel,
    nw_plugPartitionNode partitionNode,
    nw_seqNr packetNr,
    nw_bool reconnectAllowed)
{
    c_bool found = FALSE;
    nw_plugReceivingNode receivingNode;
    nw_ackMessage *currentMsgPtr;
    nw_ackMessage currentMsg;

    receivingNode = partitionNode->owner;
    NW_CONFIDENCE(receivingNode != NULL);

    if ((partitionNode->owner->state == NW_NODESTATE_NOT_RESPONDING)||
        (partitionNode->owner->state == NW_NODESTATE_STOPPED)){

        if (reconnectAllowed) {
            /* We received Data from the node, so we will mark it as responding again. */
            partitionNode->owner->state = NW_NODESTATE_RESPONDING;

            {
                nw_partitionId partitionId;
                nw_plugPartitionNode locPartitionNode;

                for (partitionId = 0; partitionId < partitionNode->owner->nofPartitions; partitionId++) {
                    locPartitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(partitionNode->owner, partitionId);
                    locPartitionNode->firstStartingNr = 0;
                }
            }
        }
    }

    if (receivingNode->state == NW_NODESTATE_RESPONDING) {
        /* Data has been received, insert an ack message into the list for this partition-node */
        currentMsgPtr = &partitionNode->pendingAckList;
        currentMsg = *currentMsgPtr;
        while ((currentMsg != NULL) && (!found)){
            if (packetNr > (currentMsg->closingNr + 1)) {
                currentMsgPtr = &(currentMsg->next);
                currentMsg = currentMsg->next;
            } else if (packetNr == (currentMsg->closingNr + 1)) {
                /* Ack fits into this message */
                currentMsg->closingNr = packetNr;
                if (currentMsg->next != NULL) {
                    /* Check if this message and next one can be linked together */
                    if (currentMsg->next->startingNr == (packetNr + 1)) {
                        /* Yes, they can */
                        currentMsg->closingNr = currentMsg->next->closingNr;
                        /* Remove the next message because it has been combined into one */
                        nw_plugPartitionNodeAckMessageRelease(partitionNode,
                            currentMsg->next, &currentMsg->next);
                    }
                }
                found = TRUE;
            } else if (packetNr >= currentMsg->startingNr) {
                NW_CONFIDENCE(packetNr <= currentMsg->closingNr);
                /* Do nothing, somehow the ack was already in the acklist */
                found = TRUE;
            } else if (packetNr == (currentMsg->startingNr-1)) {
                /* Fits into this message */
                currentMsg->startingNr = packetNr;
                found = TRUE;
            } else {
                NW_CONFIDENCE(packetNr < (currentMsg->startingNr-1));
                /* Create new message and insert before current */
                nw_plugPartitionNodeAckMessageCreate(partitionNode, packetNr,
                    currentMsgPtr);
                found = TRUE;
            }
        }
        if (!found) {
            NW_CONFIDENCE(currentMsg == NULL);
            nw_plugPartitionNodeAckMessageCreate(partitionNode, packetNr,
                currentMsgPtr);
        }
    }
}


static void
nw_plugPartitionNodeAckReceived(
    nw_plugPartitionNode partitionNode,
    nw_seqNr startingNr,
    nw_seqNr closingNr,
    nw_length remoteRecvBuffer,
    nw_bool reconnectAllowed)
{
    nw_resendItem currentItem;
    nw_resendItem firstItem;
    nw_resendItem lastItem;
    nw_resendItem *prevNext;
    nw_resendItem *nextPrev;
    nw_bool found = FALSE;
    nw_bool done = FALSE;
    nw_plugReceivingPartition partition;
    nw_seqNr firstKnownPacketNr;
    char *addressString;

    NW_CONFIDENCE(partitionNode != NULL);
    NW_CONFIDENCE(startingNr > 0);
    NW_CONFIDENCE(closingNr >= startingNr);

    if ((partitionNode->owner->state == NW_NODESTATE_NOT_RESPONDING)||
        (partitionNode->owner->state == NW_NODESTATE_STOPPED)){

        if (reconnectAllowed) {
            /* We received an ACK from the node, so we will mark it as responding again. */
            partitionNode->owner->state = NW_NODESTATE_RESPONDING;
            {
                nw_partitionId partitionId;
                nw_plugPartitionNode tmpPartitionNode;

                for (partitionId = 0; partitionId < partitionNode->owner->nofPartitions; partitionId++) {
                    tmpPartitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(partitionNode->owner, partitionId);
                    tmpPartitionNode->firstStartingNr = 0;
                }
            }
        }
    }

    if (partitionNode->owner->state == NW_NODESTATE_RESPONDING) {
        if (partitionNode->firstStartingNr == 0) {
            /* First Ack for this partitionNode
             * Administer this, and copy from the lateJoiningResendList
             */

            partitionNode->firstStartingNr = startingNr;

            /* First look for the resendItem to start copying */
            if (partitionNode->pendingResendListHead != NULL) {
                firstKnownPacketNr = partitionNode->pendingResendListHead->packetNr;
                currentItem = partitionNode->receivingPartition->lateJoiningResendListTail;
                while ((currentItem != NULL) &&
                       (currentItem->packetNr >= firstKnownPacketNr)) {
                    currentItem = currentItem->prev;
                }
            } else {
                currentItem = partitionNode->receivingPartition->lateJoiningResendListTail;
            }
            /* Item to start copying from is found, now do the copy */
            if (partitionNode->pendingResendListHead != NULL) {
                nextPrev = &partitionNode->pendingResendListHead->prev;
            } else {
                nextPrev = &partitionNode->pendingResendListTail;
            }
            addressString = os_sockaddrAddressToString((struct sockaddr*) &partitionNode->owner->address,
                                                        (char*) os_malloc(INET6_ADDRSTRLEN),
                                                        INET6_ADDRSTRLEN);
            NW_TRACE_5(Send, 2,
                "plugChannel %s: First ACK from node 0x"PA_ADDRFMT" (address %s) : starting: %d ,closing: %d",
                 nw__plugChannelGetName(nw_plugChannel(partitionNode->owner->owner)),
                 partitionNode->owner->nodeId, addressString, startingNr,closingNr);

            while ((currentItem != NULL) &&
                   (currentItem->packetNr >= startingNr)) {

                NW_TRACE_4(Send, 3,
                    "plugChannel %s: Cloning from latejoininglist for node 0x"PA_ADDRFMT" (address %s) : %d",
                    nw__plugChannelGetName(nw_plugChannel(partitionNode->owner->owner)),
                    partitionNode->owner->nodeId, addressString, currentItem->packetNr);

                nw_plugPartitionNodeResendItemClone(partitionNode, currentItem,
                    &partitionNode->pendingResendListHead, nextPrev);
                nextPrev = &((*nextPrev)->prev);
                currentItem = currentItem->prev;
            }
            os_free(addressString);


        }

        /* The last received remoteRecvBuffer is always the one to use */
        partitionNode->remoteRecvBuffer = remoteRecvBuffer;
        partitionNode->remoteRecvBufferRefreshed = TRUE;

        /* Remove the corresponding buffers from the resend list */
        currentItem = partitionNode->pendingResendListHead;

        /* Find the first buffer to remove */
        while ((currentItem != NULL) && (!found)) {
            if (currentItem->packetNr >= startingNr) {
                found = TRUE;
            } else {
                currentItem = currentItem->next;
            }
        }

        if (found) {

             /* NOTE: This implements a shortcut in stead of using
             *       nw_plugpartitionNodeResendItemRelease.
             *       First release all admin buffers manually and
             *       then remove a complete list of resendItems
             *       from the pendingResendList */

            firstItem = currentItem;
            lastItem = NULL;
            do {
                if (currentItem->packetNr <= closingNr) {
                    lastItem = currentItem;
                    nw_plugDataBufferReliabilityAdminRelease(
                        REL_ADMIN(lastItem->buffer));
                    partitionNode->pendingResendCount--;
                    currentItem = currentItem->next;
                } else {
                    done = TRUE;
                }
            } while ((currentItem != NULL) && !done);
            /* move all items to freelist */
            if (lastItem != NULL) {
                if (firstItem == partitionNode->pendingResendListHead) {
                    prevNext = &partitionNode->pendingResendListHead;
                } else {
                    NW_CONFIDENCE(firstItem->prev != NULL);
                    NW_CONFIDENCE(firstItem->prev->next == firstItem);
                    prevNext = &firstItem->prev->next;
                }
                if (lastItem == partitionNode->pendingResendListTail) {
                    nextPrev = &partitionNode->pendingResendListTail;
                } else {
                    NW_CONFIDENCE(lastItem->next != NULL);
                    NW_CONFIDENCE(lastItem->next->prev == lastItem);
                    nextPrev = &lastItem->next->prev;
                }
                *prevNext = lastItem->next;
                *nextPrev = firstItem->prev;
                /* Move the items to the free list in the partition */
                partition = partitionNode->receivingPartition;
                lastItem->next = partition->freeResendList;
                partition->freeResendList = firstItem;
            }
        } else {
            if ( partitionNode->firstStartingNr == 0 ||
                 closingNr < partitionNode->firstStartingNr ){
                    addressString = os_sockaddrAddressToString((struct sockaddr*) &partitionNode->owner->address,
                                                               (char*) os_malloc(INET6_ADDRSTRLEN),
                                                               INET6_ADDRSTRLEN);
                    NW_REPORT_WARNING_4("reliable protocol",
                        "ACK received from node 0x"PA_ADDRFMT" (address %s) for packet %d, while only packets from %d "
                        "onwards are available for resend reliable operation of this channel is no longer guaranteed",
                        partitionNode->owner->nodeId, addressString, closingNr, partitionNode->firstStartingNr);
            os_free (addressString);
            }
        }
    }
}


/* ------------------------- nw_plugReceivingNodePartition ------------------ */


static nw_plugReceivingPartition
nw_plugReceivingPartitionNew(
    nw_partitionId partitionId,
    os_uint32 hash)
{
    nw_plugReceivingPartition result = NULL;

    result = os_malloc(sizeof(*result));
    if (result != NULL) {
        result->partitionId = partitionId;
        result->partitionHash = hash;
        result->fragmentedSeqNr = 0;
        result->packetSeqNr = 0;

        result->lateJoiningResendListHead = NULL;
        result->lateJoiningResendListTail = NULL;
        result->freeResendList = NULL;

        result->totalResendCount = 0;

        result->partitionNodesHead = NULL;
        result->partitionNodesTail = NULL;
    }
    return result;
}

static void
nw_plugReceivingPartitionFree(
    nw_plugReceivingPartition receivingPartition)
{
    nw_resendItem resendItem;

    if (receivingPartition != NULL) {

        /* Global resendItems list */
        while (receivingPartition->lateJoiningResendListHead != NULL) {
            resendItem = receivingPartition->lateJoiningResendListHead;
            receivingPartition->lateJoiningResendListHead = resendItem->next;
            nw_resendItemFree(resendItem);
        }

        /* Global resendItems pool */
        while (receivingPartition->freeResendList != NULL) {
            resendItem = receivingPartition->freeResendList;
            receivingPartition->freeResendList = resendItem->next;
            nw_resendItemFree(resendItem);
        }

        os_free(receivingPartition);
    }
}

static void
nw_plugReceivingPartitionAddPartitionNode(
    nw_plugReceivingPartition receivingPartition,
    nw_plugPartitionNode plugPartitionNode)
{
    nw_plugPartitionNode *prevNext;

    NW_CONFIDENCE(receivingPartition != NULL);
    NW_CONFIDENCE(plugPartitionNode != NULL);

    if (receivingPartition->partitionNodesHead == NULL) {
        prevNext = &receivingPartition->partitionNodesHead;
    } else {
        prevNext = &receivingPartition->partitionNodesTail->nextInPartition;
    }
    nw_plugPartitionNodeSetPartition(plugPartitionNode, receivingPartition, prevNext);
    if (receivingPartition->partitionNodesTail == NULL) {
        receivingPartition->partitionNodesTail = receivingPartition->partitionNodesHead;
    }
}

static void
nw_plugReceivingPartitionResendItemCreate(
    nw_plugReceivingPartition receivingPartition,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev,
    nw_length bufferLength)
{
    nw_resendItem item;

    NW_CONFIDENCE(prevNext != NULL);
    NW_CONFIDENCE(nextPrev != NULL);

    if (receivingPartition->freeResendList != NULL) {
        item = receivingPartition->freeResendList;
        receivingPartition->freeResendList = item->next;
    } else {
        item = nw_resendItemNew();
        receivingPartition->totalResendCount++;
    }
    if (item != NULL) {
        nw_resendItemInit(item, packetNr, buffer, prevNext, nextPrev, bufferLength);
    }
}


static void
nw_plugReceivingPartitionResendItemRelease(
    nw_plugReceivingPartition receivingPartition,
    nw_resendItem resendItem,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev)
{
    NW_CONFIDENCE(receivingPartition != NULL);
    NW_CONFIDENCE(prevNext != NULL);
    NW_CONFIDENCE(nextPrev != NULL);

    nw_plugDataBufferReliabilityAdminRelease(REL_ADMIN(resendItem->buffer));
    *prevNext = resendItem->next;
    *nextPrev = resendItem->prev;
    resendItem->next = receivingPartition->freeResendList;
    receivingPartition->freeResendList = resendItem;
}


/* --------------------- PartitionNode ---------------------- */

static nw_plugPartitionNode
nw_plugPartitionNodeNew(
    nw_plugReceivingNode owner)
{
    nw_plugPartitionNode result;

    result = os_malloc(sizeof(*result));
    if (result != NULL) {

        result->pendingResendCount = 0;
        result->pendingResendListHead = NULL;
        result->pendingResendListTail = NULL;

        result->pendingAckCount = 0;
        result->pendingAckList = NULL;
        result->totalAckCount = 0;
        result->freeAckList = NULL;
        result->firstStartingNr = 0;
        result->firstSendAck = 0;
        result->repeatFirstAckCntDwn = 0;
        result->remoteRecvBuffer = 0;
        result->remoteRecvBufferRefreshed = FALSE;

        result->owner = owner;

        /* nextInPartition has to be set by the method SetPartition */
        result->nextInPartition = NULL;
    }
    return result;
}

static void
nw_plugPartitionNodeFree(
    nw_plugPartitionNode partitionNode)
{
    nw_ackMessage ackMessage;
    nw_resendItem resendItem;

    if (partitionNode != NULL) {
        /* Free pending ack messages */
        while (partitionNode->pendingAckList != NULL) {
            ackMessage = partitionNode->pendingAckList;
            partitionNode->pendingAckList = ackMessage->next;
            partitionNode->pendingAckCount--;
            partitionNode->totalAckCount--;
            nw_ackMessageFree(ackMessage);
        }
        /* Free ack messages pool */
        while (partitionNode->freeAckList != NULL) {
            ackMessage = partitionNode->freeAckList;
            partitionNode->freeAckList = ackMessage->next;
            partitionNode->totalAckCount--;
            nw_ackMessageFree(ackMessage);
        }
        /* Free pending resend items messages */
        while (partitionNode->pendingResendListHead != NULL) {
            resendItem = partitionNode->pendingResendListHead;
            partitionNode->pendingResendListHead = resendItem->next;
            partitionNode->pendingResendCount--;
            nw_resendItemFree(resendItem);
        }
        partitionNode->pendingResendListTail = NULL;
        partitionNode->pendingAckCount = 0;
        partitionNode->totalAckCount = 0;
        partitionNode->pendingResendCount = 0;
        partitionNode->firstStartingNr = 0;
    }
}

static void
nw_plugPartitionNodeSetPartition(
    nw_plugPartitionNode plugPartitionNode,
    nw_plugReceivingPartition receivingPartition,
    nw_plugPartitionNode *prevNextInPartition)
{
    if (plugPartitionNode != NULL) {
        plugPartitionNode->receivingPartition = receivingPartition;
        plugPartitionNode->nextInPartition = *prevNextInPartition;
        *prevNextInPartition = plugPartitionNode;
    }
}

static void
nw_plugPartitionNodeResendItemCreate(
    nw_plugPartitionNode partitionNode,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev,
    nw_length bufferLength)
{
    /* The partitionNode at this moment does not contain its own freeResendList.
     * If that would be the case, the item would have been created here.
     * For now, just forward this call to the owning receivingPartition. */
    nw_plugReceivingPartition receivingPartition;

    receivingPartition = partitionNode->receivingPartition;
    NW_CONFIDENCE(receivingPartition != NULL);
    nw_plugReceivingPartitionResendItemCreate(receivingPartition, packetNr,
        buffer, prevNext, nextPrev, bufferLength);
    partitionNode->pendingResendCount++;
}

static void
nw_plugPartitionNodeResendItemClone(
    nw_plugPartitionNode partitionNode,
    nw_resendItem resendItem,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev)
{
    nw_plugReceivingPartition receivingPartition;

    NW_CONFIDENCE(prevNext != NULL);
    NW_CONFIDENCE(nextPrev != NULL);
    NW_CONFIDENCE(partitionNode != NULL);
    NW_CONFIDENCE(partitionNode->owner->state == NW_NODESTATE_RESPONDING);

    receivingPartition = partitionNode->receivingPartition;
    NW_CONFIDENCE(receivingPartition != NULL);
    nw_plugReceivingPartitionResendItemCreate(receivingPartition,
        resendItem->packetNr, resendItem->buffer,
        prevNext, nextPrev, resendItem->bufferLength);
    partitionNode->pendingResendCount++;
}

static void
nw_plugPartitionNodeAckMessageCreate(
    nw_plugPartitionNode partitionNode,
    nw_seqNr startingNr,
    nw_ackMessage *prevNext)
{
    nw_ackMessage msg;

    NW_CONFIDENCE(prevNext != NULL);
    NW_CONFIDENCE(partitionNode != NULL);

    if (partitionNode->freeAckList != NULL) {
        msg = partitionNode->freeAckList;
        partitionNode->freeAckList = msg->next;
    } else {
        msg = nw_ackMessageNew();
        partitionNode->totalAckCount++;
    }
    if (msg != NULL) {
        nw_ackMessageInit(msg, startingNr, prevNext);
        partitionNode->pendingAckCount++;
    }
}

static void
nw_plugPartitionNodeAckMessageRelease(
    nw_plugPartitionNode partitionNode,
    nw_ackMessage ackMessage,
    nw_ackMessage *prevNext)
{
    NW_CONFIDENCE(prevNext != NULL);
    NW_CONFIDENCE(partitionNode != NULL);

    *prevNext = ackMessage->next;
    ackMessage->next = partitionNode->freeAckList;
    partitionNode->freeAckList = ackMessage;
    partitionNode->pendingAckCount--;
}

static void
nw_plugPartitionNodeSerializeAndReleaseAcks(
    nw_plugPartitionNode partitionNode,
    nw_plugControlMessage *controlMessages,
    nw_length maxMsgCount,
    nw_length *actualMsgCount,
    nw_bool *more)
{
    NW_STRUCT(nw_plugControlMessage) *currentControlMessage;
    nw_length count = 0;
    char *addressString;

    NW_CONFIDENCE((partitionNode->pendingAckCount == 0) ==
                  (partitionNode->pendingAckList == NULL));
        /* During the maximum retry period (recoveryfactor*maxretries), include the
         * first ever ack for this PartitionNode. This is done, so that the sender
         * will always know which data should be resend to this node.
         */
        currentControlMessage = controlMessages[0];
        if (partitionNode->firstSendAck > 0 &&
        partitionNode->repeatFirstAckCntDwn > 0 &&
        ((partitionNode->repeatFirstAckCntDwn % partitionNode->owner->owner->recoveryFactor) == 0 || partitionNode->pendingAckCount > 0) ){
            nw_plugControlMessageSetPartitionId(currentControlMessage,
                partitionNode->receivingPartition->partitionHash);
            nw_plugControlMessageSetStartingNr(currentControlMessage,
                partitionNode->firstSendAck);
            nw_plugControlMessageSetClosingNr(currentControlMessage,
                partitionNode->firstSendAck);
            currentControlMessage = &(currentControlMessage[1]);
        count++;
    }
    if (partitionNode->pendingAckCount > 0) {
        NW_CONFIDENCE(partitionNode->owner->state == NW_NODESTATE_RESPONDING);

        /* First ack to be send out. Store the sequence number and intialize the countdown,
         * that determines how long this first ack number will be included in every ack-message
         */
        if (partitionNode->firstSendAck == 0 ){
            partitionNode->firstSendAck = partitionNode->pendingAckList->startingNr;
            partitionNode->repeatFirstAckCntDwn = partitionNode->owner->owner->maxRetries *
                                                  partitionNode->owner->owner->recoveryFactor;

            addressString = os_sockaddrAddressToString((struct sockaddr*) &partitionNode->owner->address,
                                                        (char*) os_malloc(INET6_ADDRSTRLEN),
                                                        INET6_ADDRSTRLEN);
            NW_TRACE_5(Send, 2,
                "plugChannel %s: First ACK to node 0x"PA_ADDRFMT" (address %s) : %d,%d",
                 nw__plugChannelGetName(nw_plugChannel(partitionNode->owner->owner)),
                 partitionNode->owner->nodeId, addressString,
                 partitionNode->pendingAckList->startingNr,
                 partitionNode->pendingAckList->closingNr);
            os_free(addressString);
        }
        while ((partitionNode->pendingAckList != NULL) && (count < maxMsgCount)) {

            nw_plugControlMessageSetPartitionId(currentControlMessage,
                partitionNode->receivingPartition->partitionHash);
            nw_plugControlMessageSetStartingNr(currentControlMessage,
                partitionNode->pendingAckList->startingNr);
            nw_plugControlMessageSetClosingNr(currentControlMessage,
                partitionNode->pendingAckList->closingNr);
            nw_plugPartitionNodeAckMessageRelease(partitionNode,
                partitionNode->pendingAckList, &partitionNode->pendingAckList);

            currentControlMessage = &(currentControlMessage[1]);
            count++;
        }
        *controlMessages = currentControlMessage;
    }
    *actualMsgCount = count;
    *more = (partitionNode->pendingAckList != NULL);
}

/* --------------------------- nw_resendItem -------------------------------- */

/* No constructors here becuse receivingPartition is the factory */

static nw_resendItem
nw_resendItemNew(void)
{
    nw_resendItem result;

    result = os_malloc(sizeof(*result));

    return result;
}

static void
nw_resendItemInit(
    nw_resendItem item,
    nw_seqNr packetNr,
    nw_plugDataBuffer buffer,
    nw_resendItem *prevNext,
    nw_resendItem *nextPrev,
    nw_length bufferLength)
{
    NW_CONFIDENCE(item != NULL);
    NW_CONFIDENCE(prevNext != NULL);
    NW_CONFIDENCE(nextPrev != NULL);

    item->packetNr = packetNr;
    item->controlFlushPass = 0;
    nw_plugDataBufferReliabilityAdminKeep(REL_ADMIN(buffer));
    item->buffer = buffer;
    item->next = *prevNext;
    *prevNext = item;
    item->prev = *nextPrev;
    *nextPrev = item;
    item->bufferLength = bufferLength;
}

static void
nw_resendItemFree(
    nw_resendItem resendItem)
{
    NW_CONFIDENCE(resendItem != NULL);

    os_free(resendItem);
}

/* --------------------------- nw_ackMessage -------------------------------- */

static nw_ackMessage
nw_ackMessageNew()
{
    nw_ackMessage result;

    result = (nw_ackMessage)os_malloc(sizeof(*result));

    return result;
}

static void
nw_ackMessageInit(
    nw_ackMessage msg,
    nw_seqNr startingNr,
    nw_ackMessage *prevNext)
{
    msg->startingNr = startingNr;
    msg->closingNr = startingNr;
    msg->next = *prevNext;
    *prevNext = msg;
}

static void
nw_ackMessageFree(
    nw_ackMessage ackMessage)
{
    os_free(ackMessage);
}

/* ---------------------------- Control (acks sending and resending) ------- */

static void
nw_periodicCheckMessageBox( nw_plugChannel channel )
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_messageBoxMessageType messageType;
    nw_bool messageReceived;
    os_sockaddr_storage sendingAddress;
    nw_seqNr sendingNodeId;
    nw_plugReceivingNode receivingNode;
    char *addressString;
    c_string list;


    /* Walk over all messages in the messageBox */
    messageReceived = nw_plugChannelProcessMessageBox(channel,
        &sendingNodeId, &sendingAddress, &list, &messageType);
    while (messageReceived) {
        /* Only take action in case of reliability */
        if (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE) {
            switch (messageType) {
                case NW_MBOX_NODE_STARTED:
                    receivingNode = nw_plugSendChannelLookupOrCreateReceivingNode(
                        sendChannel, sendingNodeId, 0, sendingAddress);
                    NW_CONFIDENCE(receivingNode != NULL);

                break;
                case NW_MBOX_NODE_STOPPED:
                    receivingNode = nw_plugSendChannelLookupReceivingNode(
                        sendChannel, sendingNodeId);
                    if (receivingNode != NULL) {
                        NW_CONFIDENCE(os_sockaddrIPAddressEqual((os_sockaddr*)&receivingNode->address, (os_sockaddr*)&sendingAddress));
                        nw_plugReceivingNodeStopped(receivingNode);
                        if (sendChannel->pss != NULL) {
                           if (sendChannel->pss->enabled) {
                               sendChannel->pss->numberOfKnownNodes--;
                           }
                        }
                    }
                break;
                case NW_MBOX_NODE_DIED:
                    receivingNode = nw_plugSendChannelLookupReceivingNode(
                        sendChannel, sendingNodeId);
                    if (receivingNode != NULL) {
                        NW_CONFIDENCE(os_sockaddrIPAddressEqual((os_sockaddr*)&receivingNode->address, (os_sockaddr*)&sendingAddress));
                        addressString = os_sockaddrAddressToString((struct sockaddr*) &receivingNode->address,
                                                                    (char*) os_malloc(INET6_ADDRSTRLEN),
                                                                    INET6_ADDRSTRLEN);
                        if (receivingNode->nodeId != 12345) { /*TODO neat fix for ignore ghost node 12345*/
                            NW_REPORT_WARNING_2("reliable protocol",
                                "No heartbeats received from Node 0x%x (address %s)",
                                receivingNode->nodeId, addressString);
                        }
                        nw_plugReceivingNodeNotResponding(receivingNode);
                        if (sendChannel->pss != NULL) {
                           if (sendChannel->pss->enabled) {
                               sendChannel->pss->numberOfKnownNodes--;
                           }
                        }
                        os_free(addressString);
                    }

                break;
                case NW_MBOX_GP_ADD:
                case NW_MBOX_GP_ADDLIST:
                case NW_MBOX_GP_REMOVE:
                break;
                case NW_MBOX_UNDEFINED:
                    NW_CONFIDENCE(messageType != NW_MBOX_UNDEFINED);
                break;
            }
        }
        switch (messageType) {
            char* addressString;
            case NW_MBOX_GP_ADD:
                receivingNode = nw_plugSendChannelLookupOrCreateReceivingNode(
                    sendChannel, sendingNodeId, 0, sendingAddress);
                NW_CONFIDENCE(receivingNode != NULL);
                /* @todo dds#2523 - Suspect the below had / still has leak (leaks?)? The address to string call(s) allocate.
                Need to drill into nw__plugChannelGetName & nw_socketAddPartition to see if take ownership */
                addressString = os_sockaddrAddressToString((struct sockaddr*) &sendingAddress,
                                                               (char*) os_malloc(INET6_ADDRSTRLEN),
                                                               INET6_ADDRSTRLEN);
                NW_TRACE_3(Send, 5, "plugChannel %s: sendthread: Adding to GP: %x with address %s",
                    nw__plugChannelGetName(channel),sendingNodeId,addressString);
                /* extend the global partition with this address */
                nw_socketAddPartition(nw__plugChannelGetSocket(channel), 0,
                                      addressString,
                                      TRUE, FALSE, TRUE, 0);

            break;
            case NW_MBOX_GP_ADDLIST:
                /* extend the global partition with this address */
                NW_TRACE_2(Send, 5, "plugChannel %s: sendthread: Adding list to GP: %s",nw__plugChannelGetName(channel),list);
                nw_socketAddPartition(nw__plugChannelGetSocket(channel), 0,
                                      list,
                                      TRUE, FALSE, TRUE, 0);
            break;
            case NW_MBOX_GP_REMOVE:
                receivingNode = nw_plugSendChannelLookupReceivingNode(
                    sendChannel, sendingNodeId);


                /* PW: TODO remove address from global partition */

            break;

            case NW_MBOX_NODE_STARTED:
            case NW_MBOX_NODE_STOPPED:
            case NW_MBOX_NODE_DIED:
            break;
            case NW_MBOX_UNDEFINED:
                NW_CONFIDENCE(messageType != NW_MBOX_UNDEFINED);
            break;
        }


        messageReceived = nw_plugChannelProcessMessageBox(channel,
            &sendingNodeId, &sendingAddress, &list, &messageType);
    }
}

static void
nw_periodicProcessIncomingAcks( nw_plugChannel channel )
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugInterChannel interChannel;
    nw_bool done;
    nw_seqNr sendingNodeId;
    nw_partitionId sendingPartitionId;
    os_sockaddr_storage sendingAddress;
    nw_seqNr startingNr;
    nw_seqNr closingNr;
    nw_length remoteRecvBuffer;
    nw_plugPartitionNode partitionNode;
    c_ulong ackcounter;

    /* First get all ack messages from the buffer and update
     * the administratin correspondingly */
    interChannel = nw__plugChannelGetInterChannelComm(channel);
    NW_CONFIDENCE(interChannel != NULL);

    ackcounter =0;
    do {
        done = !nw_plugInterChannelProcessAckReceivedMessage(interChannel,
                &sendingNodeId, &sendingPartitionId, &sendingAddress,
                &startingNr, &closingNr, &remoteRecvBuffer);
        if (!done) {
            ackcounter++;
            /* Lookup receivingNode object from hash */
            partitionNode = nw_plugSendChannelLookupOrCreatePartitionNode(
                sendChannel, sendingNodeId, sendingPartitionId,
                sendingAddress);
            NW_CONFIDENCE(partitionNode != NULL);
            nw_plugPartitionNodeAckReceived(partitionNode,
                startingNr, closingNr, remoteRecvBuffer,channel->reconnectAllowed);

            NW_TRACE_2(Send, 4, "plugChannel %s: sendthread: recvACK with ThrottleValue: %d",
                nw__plugChannelGetName(channel),remoteRecvBuffer);
        }
    } while (!done);
    if (sendChannel->pss != NULL) {
        if (sendChannel->pss->enabled) {
            sendChannelStatisticsCounterInc(adminQueueAcks,sendChannel->pss,ackcounter);
        }
    }
    /* teller add adminQueueAcks */
}

static void
nw_autoThrottle( nw_plugChannel channel,nw_signedLength credits )
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugReceivingNode receivingNode;
    nw_partitionId partitionId;
    nw_plugPartitionNode partitionNode;
    nw_length remoteRecvBuffer = 0;
    nw_length ThrottleValue = 0;
    nw_length ThrottleValuePrev = 0;
    nw_signedLength maxBurstSigned;
    nw_length* determiningValuePtr = NULL;
    nw_length ActualData = 0;
    nw_bool   FreshThrottlevalue = FALSE;


    /* determine max remoteRecvBuffers for throttling */
    /* Walk over all partition nodes, to determine the maximum remoteRecvBuffer */
    receivingNode = sendChannel->receivingNodesHead;
    while (receivingNode != NULL) {
        if (receivingNode->state == NW_NODESTATE_RESPONDING) {
            for (partitionId = 0;
                 partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
                 partitionId++) {
                partitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, partitionId);
                if ( partitionNode->remoteRecvBuffer > remoteRecvBuffer ) {
                    remoteRecvBuffer = partitionNode->remoteRecvBuffer;
                    FreshThrottlevalue = partitionNode->remoteRecvBufferRefreshed;
                    determiningValuePtr = &partitionNode->remoteRecvBuffer;
                }
                partitionNode->remoteRecvBufferRefreshed = FALSE;
            }
        }
        receivingNode = receivingNode->nextInList;
    }


    /*
     * Throttle the maxBurstSize based on remoteRecvBuffer
     */
    ThrottleValue = remoteRecvBuffer;
    ThrottleValuePrev = sendChannel->throttleMedian;

    /* calculate median over the current and the previous 2 ThrottleValue values.*/
    if (sendChannel->throttleValueHist2 >= sendChannel->throttlevalueHist1) {
        if (ThrottleValue >= sendChannel->throttleValueHist2 ){
            sendChannel->throttleMedian = sendChannel->throttleValueHist2;
        } else if (ThrottleValue <= sendChannel->throttlevalueHist1 ){
            sendChannel->throttleMedian = sendChannel->throttlevalueHist1;
        } else {
            sendChannel->throttleMedian = ThrottleValue;
        }
    } else {
        if (ThrottleValue >= sendChannel->throttlevalueHist1 ){
            sendChannel->throttleMedian = sendChannel->throttlevalueHist1;
        } else if (ThrottleValue <= sendChannel->throttleValueHist2 ){
            sendChannel->throttleMedian = sendChannel->throttleValueHist2;
        } else {
            sendChannel->throttleMedian = ThrottleValue;
        }
    }
    sendChannel->throttleValueHist2 = sendChannel->throttlevalueHist1;
    sendChannel->throttlevalueHist1 = ThrottleValue;

    maxBurstSigned = (nw_signedLength)sendChannel->currentPeriodBudget;
    /* D-Action */
    maxBurstSigned +=  (ThrottleValuePrev - sendChannel->throttleMedian) *
                                  (nw__plugChannelGetFragmentLength(channel)/sendChannel->throttleParamD);

    /* P-Action (supressed if not reaching maxburstsize currently */
    if ( (credits) <= 0 ) {
        maxBurstSigned += (sendChannel->throttleThreshold - sendChannel->throttleMedian) *
                                         (nw__plugChannelGetFragmentLength(channel)/sendChannel->throttleParamP);
    } else {
        if (sendChannel->throttleThreshold < sendChannel->throttleMedian) {
            /* If remote queue is bigger than threshold, but we didn't even use the
             * maximum Throughput yet, correct the maximum with the amount we didn't even use the last period.
             */
            maxBurstSigned -= credits;
        }
    }

    /* Calculate actual amount of data send in the last resolution period, for tracing */
    ActualData = sendChannel->currentPeriodBudget - credits;

    /* Check Limits */
    if (maxBurstSigned < (nw_signedLength)sendChannel->lowerThrottleLimit) {
        sendChannel->currentPeriodBudget = sendChannel->lowerThrottleLimit;
    }else if (maxBurstSigned > (nw_signedLength)sendChannel->upperThrottleLimit) {
        sendChannel->currentPeriodBudget = sendChannel->upperThrottleLimit;
    }else {
        sendChannel->currentPeriodBudget = (nw_length)maxBurstSigned;
    }

    NW_TRACE_5(Send, 5, "plugChannel %s: Throttling: ThrottleValue: %d, TV_updated:%d, maxBurstSize:%d, actualData:%d",
        nw__plugChannelGetName(channel),ThrottleValue,FreshThrottlevalue, sendChannel->currentPeriodBudget,ActualData);

}


static void
nw_periodicProcessReceivedData( nw_plugChannel channel )
{
    nw_bool done;
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugInterChannel interChannel;
    nw_seqNr sendingNodeId;
    nw_partitionId sendingPartitionId;
    os_sockaddr_storage sendingAddress;
    nw_seqNr packetNr;
    nw_length localRecvBuffer;
    nw_plugPartitionNode partitionNode;
    c_ulong datacounter;

    interChannel = nw__plugChannelGetInterChannelComm(channel);
    NW_CONFIDENCE(interChannel != NULL);
    /* Now find out which data has been received and send the corresponding
     * acks to the sending node */
    /* teller 0 */
    datacounter =0;
    do {
        done = !nw_plugInterChannelProcessDataReceivedMessage(interChannel,
                &sendingNodeId, &sendingPartitionId, &sendingAddress, &packetNr, &localRecvBuffer);
        if (!done) {
            datacounter++;
            /* Lookup receivingNode object from hash */
            partitionNode = nw_plugSendChannelLookupOrCreatePartitionNode(
                sendChannel, sendingNodeId, sendingPartitionId,
                sendingAddress);
            NW_CONFIDENCE(partitionNode != NULL);
            nw_plugPartitionNodeDataReceivedReliably(channel, partitionNode, packetNr,channel->reconnectAllowed);
            sendChannel->RecvBufferInUse = localRecvBuffer;
        }

    } while (!done);
    /* add adminQueueData fullcounters */
    if (sendChannel->pss != NULL) {
        if (sendChannel->pss->enabled) {
            sendChannelStatisticsCounterInc(adminQueueData,sendChannel->pss,datacounter);
        }
    }

    NW_TRACE_2(Send, 4, "plugChannel %s: sendthread: sendACK MAX output: %d",
        nw__plugChannelGetName(channel),sendChannel->RecvBufferInUse);
}

static void
nw_periodicProcessReceivedBackup( nw_plugChannel channel )
{
    nw_bool done;
    nw_plugInterChannel interChannel;
    nw_seqNr sendingNodeId;
    nw_partitionId sendingPartitionId;
    os_sockaddr_storage sendingAddress;
    nw_plugDataBuffer backupBuffer;
    nw_length localRecvBuffer;
    c_ulong datacounter;

    interChannel = nw__plugChannelGetInterChannelComm(channel);
    NW_CONFIDENCE(interChannel != NULL);
    /* Now find out which data has been received and send the corresponding*/
    /* teller 0 */
    datacounter =0;
    do {
        done = !nw_plugInterChannelProcessBackupReceivedMessage(interChannel,
                &sendingNodeId, &sendingPartitionId, &sendingAddress, &backupBuffer, &localRecvBuffer);
        if ( !done) {
            /* Write the buffer to the socket*/
            NW_TRACE_4(Send, 4, "plugChannel %s: RUPC:sendthread: Resend BACKUP buffer to 0x%x part 0x%x message %d",
                nw__plugChannelGetName(channel),sendingNodeId,sendingPartitionId,nw_plugDataBufferGetPacketNr(backupBuffer));

            nw_socketSendDataTo(nw__plugChannelGetSocket(channel),
                sendingAddress,
                backupBuffer,
                nw_plugBufferGetLength(nw_plugBuffer(backupBuffer)));

        }



    } while (!done);
}

/*
 * Build one or more messages that contain all the announcements and broadcast them on the default partition.
 *
 */
static void
nw_periodicProcessDataAnnounce( nw_plugChannel channel )
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugInterChannel interChannel;
    nw_plugControlBuffer controlBuffer;
    nw_plugControlAltMessage currentMessage;
    nw_length remainingSize;
    nw_length maxNofMessages;
    nw_length nofMessages;
    nw_length length;
    nw_bool done;
    nw_seqNr diedNodeId;
    nw_partitionId partitionId;
    nw_seqNr firstNr;
    nw_seqNr lastNr;

    controlBuffer = nw_plugControlBuffer(sendChannel->controlWriteBuffer);
    nw_plugSendChannelInitializeControlBuffer(sendChannel,controlBuffer);
    currentMessage = NW_PLUGCONTROLBUFFER_FIRSTALTMESSAGE(controlBuffer);
    remainingSize = UI(sendChannel->controlWriteBufferEnd) - UI(currentMessage);
    maxNofMessages = remainingSize / NW_CONTROL_ALTMESSAGE_SIZE;
    nofMessages = 0;

    /* Read all the announces from the ringbuffer and put them in the message */
    interChannel = nw__plugChannelGetInterChannelComm(channel);
    NW_CONFIDENCE(interChannel != NULL);

    do {
        done = !nw_plugInterChannelProcessDataAnnounceMessage(interChannel,
                &diedNodeId, &partitionId,
                &firstNr, &lastNr);
        if (!done) {
            /* Add announce to the message */
            nw_plugControlAltMessageSetDiedNodeId(currentMessage,diedNodeId);
            nw_plugControlAltMessageSetPartitionId(currentMessage,sendChannel->receivingPartitions[partitionId]->partitionHash);
            nw_plugControlAltMessageSetFirstNr(currentMessage,firstNr);
            nw_plugControlAltMessageSetLastNr(currentMessage,lastNr);

            currentMessage = &(currentMessage[1]);
            nofMessages++;

            NW_TRACE_5(Send, 4, "plugChannel %s: RUPC:sendthread: generate Data Announce node %x part %d first %d last %d",
                nw__plugChannelGetName(channel),diedNodeId,partitionId,firstNr,lastNr);

            if (nofMessages == maxNofMessages) {
                nw_plugControlBufferSetNrOfMessages(controlBuffer,
                    nofMessages);
                length = nofMessages * NW_CONTROL_ALTMESSAGE_SIZE +
                    NW_PLUGCONTROLBUFFER_HEADERSIZE;

                /*crc check*/
                if (sendChannel->crc_check) {
                    nw_plugBuffer(controlBuffer)->crc = ut_crcCalculate(channel->crc,((os_char*)controlBuffer)+sizeof(NW_STRUCT(nw_plugBuffer)),length-sizeof(NW_STRUCT(nw_plugBuffer)));
                }
                nw_plugBufferSetLength(nw_plugBuffer(controlBuffer),length);
                nw_plugControlBufferSetRecvBufferInUse(controlBuffer,NW_CONTROL_TAG_DATA_ANNOUNCE);
                nw_socketSendControl(nw__plugChannelGetSocket(channel),controlBuffer, length);
                currentMessage = NW_PLUGCONTROLBUFFER_FIRSTALTMESSAGE(controlBuffer);
                remainingSize = UI(sendChannel->controlWriteBufferEnd) -
                    UI(currentMessage);
                maxNofMessages = remainingSize / NW_CONTROL_ALTMESSAGE_SIZE;
                nofMessages = 0;
            }
        }
    } while (!done);

    if( nofMessages > 0 ) {
        nw_plugControlBufferSetNrOfMessages(controlBuffer,
            nofMessages);
        length = nofMessages * NW_CONTROL_ALTMESSAGE_SIZE + NW_PLUGCONTROLBUFFER_HEADERSIZE;

        /* crc check */
        if (sendChannel->crc_check) {
            nw_plugBuffer(controlBuffer)->crc = ut_crcCalculate(channel->crc,((os_char*)controlBuffer)+sizeof(NW_STRUCT(nw_plugBuffer)),length-sizeof(NW_STRUCT(nw_plugBuffer)));
        }

        nw_plugBufferSetLength(nw_plugBuffer(controlBuffer),length);
        nw_plugControlBufferSetRecvBufferInUse(controlBuffer,NW_CONTROL_TAG_DATA_ANNOUNCE);
        nw_socketSendControl(nw__plugChannelGetSocket(channel),controlBuffer, length);
    }
}

/*
 * build a message for each request and send it to the intended node
 */
static void
nw_periodicProcessDataRequest( nw_plugChannel channel )
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugInterChannel interChannel;
    nw_plugControlBuffer controlBuffer;
    nw_plugControlAltMessage currentMessage;
    nw_length length;
    nw_bool done;
    nw_seqNr servingNodeId;
    os_sockaddr_storage servingNodeAddress;
    nw_seqNr diedNodeId;
    nw_partitionId partitionId;
    nw_seqNr firstNr;
    nw_seqNr lastNr;

    /* Iterate over all data requests in the ringbuffer */
    interChannel = nw__plugChannelGetInterChannelComm(channel);
    NW_CONFIDENCE(interChannel != NULL);

    do {
        done = !nw_plugInterChannelProcessDataRequestMessage(interChannel,
                &servingNodeId, &servingNodeAddress, &diedNodeId, &partitionId,
                &firstNr, &lastNr);
        if (!done) {

            NW_TRACE_7(Send, 4, "plugChannel %s: RUPC:sendthread: generate Data Request to 0x%x (IP 0x%x) deadnode %x part %d first %d last %d",
                nw__plugChannelGetName(channel),servingNodeId,servingNodeAddress,diedNodeId,partitionId,firstNr,lastNr);


            /* build and send a seperate message for each request */
            controlBuffer = nw_plugControlBuffer(sendChannel->controlWriteBuffer);
            nw_plugSendChannelInitializeControlBuffer(sendChannel,controlBuffer);
            currentMessage = NW_PLUGCONTROLBUFFER_FIRSTALTMESSAGE(controlBuffer);

            nw_plugControlAltMessageSetDiedNodeId(currentMessage,diedNodeId);
            nw_plugControlAltMessageSetPartitionId(currentMessage,sendChannel->receivingPartitions[partitionId]->partitionHash);
            nw_plugControlAltMessageSetFirstNr(currentMessage,firstNr);
            nw_plugControlAltMessageSetLastNr(currentMessage,lastNr);

            nw_plugControlBufferSetNrOfMessages(controlBuffer,1);
            length = NW_CONTROL_ALTMESSAGE_SIZE + NW_PLUGCONTROLBUFFER_HEADERSIZE;

            /* crc check */
            if (sendChannel->crc_check) {
                nw_plugBuffer(controlBuffer)->crc = ut_crcCalculate(channel->crc,((os_char*)controlBuffer)+sizeof(NW_STRUCT(nw_plugBuffer)),length-sizeof(NW_STRUCT(nw_plugBuffer)));
            }

            nw_plugBufferSetLength(nw_plugBuffer(controlBuffer),length);
            nw_plugControlBufferSetRecvBufferInUse(controlBuffer,NW_CONTROL_TAG_DATA_REQUEST);
            nw_socketSendControlTo(nw__plugChannelGetSocket(channel),servingNodeAddress, controlBuffer, length);

        }
    } while (!done);
}



/* As each network partitions are used to seperate information flow
   from each other and per network partition a unique encrpytion key
   may be used, the Acks must only be send according the network
   partition the sender resides on. Send Acks per network-partitions,
   do not bundle Acks for different network partitions into a single
   messsage

   This operation differs Acks that belong to different partitions. The Acks
   that belong to the same partition are bundled, encoded with the specific
   security-profile and pushed to the network socket.
   In contrast the other nw_periodicSendAcksByBundled bundles Acks that belong
   to the same receiving node, no matter what partition they belong to.
*/

static void
nw_periodicSendAcksBySecureFlows( nw_plugChannel channel,
                  nw_plugReceivingNode receivingNode)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_partitionId partitionId;
    nw_plugPartitionNode partitionNode;
    nw_plugControlBuffer controlBuffer;
    nw_plugControlMessage firstMessage;
    nw_length length;
    nw_bool more;
    nw_bool result;

    /* Serialize all acks for this node and send them to the network */
    /* printf("%d ack messages to be sent for receivingNode 0x%x\n",
                   receivingNode->pendingAckCount, (unsigned int)receivingNode);
     */
    controlBuffer = nw_plugControlBuffer(sendChannel->controlWriteBuffer);
    nw_plugSendChannelInitializeControlBuffer(sendChannel,
        controlBuffer);
    firstMessage = NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(controlBuffer);

    for (partitionId = 0;
         partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
         partitionId++) {
        partitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, partitionId);

        NW_CONFIDENCE((partitionNode->pendingAckList == NULL) ==
                      (partitionNode->pendingAckCount == 0));
        if (partitionNode->pendingAckCount > 0) {
            nw_length remainingSize;
            nw_length maxNofMessages;
            nw_length totNofMessages;

            remainingSize = UI(sendChannel->controlWriteBufferEnd) -
              UI(firstMessage);
            maxNofMessages = remainingSize / NW_CONTROL_MESSAGE_SIZE;
            totNofMessages = 0;

            do {
                nw_length nofMessages;
#ifdef NW_DEBUGGING
                unsigned int pendingAckCount = partitionNode->pendingAckCount;
#endif
                nw_plugPartitionNodeSerializeAndReleaseAcks(partitionNode,
                    &firstMessage, maxNofMessages,
                    &nofMessages, &more);
                NW_CONFIDENCE(nofMessages > 0);
#ifdef NW_DEBUGGING
                NW_CONFIDENCE(nofMessages <= pendingAckCount);
#endif
                totNofMessages += nofMessages;
                maxNofMessages -= nofMessages;

                /* Either the buffer is full or no more ACKs for this
                 * network partition have got to be sent, in any case
                 * push the buffer to the socket  */
                {
                    nw_plugControlBufferSetNrOfMessages(controlBuffer,
                        totNofMessages);
                    length = totNofMessages * NW_CONTROL_MESSAGE_SIZE +
                        NW_PLUGCONTROLBUFFER_HEADERSIZE;
                    NW_CONFIDENCE(sizeof(receivingNode->address) == sizeof(os_sockaddr_storage));
                    nw_plugBufferSetLength(nw_plugBuffer(controlBuffer),length);

                    result = nw_plugSendChannelBufferEncoding(channel,partitionId,controlBuffer,&length);

                    if (result) {
                         /* only reached if encoding has been successful,
                          * the buffer contains the encoded data now and may be
                          * written to network  */
                         nw_socketSendControlTo(nw__plugChannelGetSocket(channel),
                                    receivingNode->address,
                                    controlBuffer, length);
                    } else {
                        NW_REPORT_WARNING("nw_plugSendChannelControlFlush","dropping control message");
                    }
                    /* reset the control-buffer with plain text data */
                    nw_plugSendChannelInitializeControlBuffer(sendChannel,
                        controlBuffer);

                    firstMessage = NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(controlBuffer);
                    remainingSize = UI(sendChannel->controlWriteBufferEnd) -
                        UI(firstMessage);
                    maxNofMessages = remainingSize / NW_CONTROL_MESSAGE_SIZE;
                    totNofMessages = 0;
                }
            } while (more);
#ifdef NW_DEBUGGING
            NW_CONFIDENCE(partitionNode->pendingAckCount == 0);
            NW_CONFIDENCE(partitionNode->pendingAckList == NULL);
#endif
        /* The control-write-buffer must not contain unsent messages */
        NW_CONFIDENCE(totNofMessages == 0);
        }
    }
}

static void
nw_periodicSendAcksBundled( nw_plugChannel channel,
                     nw_plugReceivingNode receivingNode)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_partitionId partitionId;
    nw_plugPartitionNode partitionNode;
    nw_plugControlBuffer controlBuffer;
    nw_plugControlMessage firstMessage;
    nw_length remainingSize;
    nw_length nofMessages;
    nw_length maxNofMessages;
    nw_length totNofMessages;
    nw_length length;
    nw_bool more;

    /* Serialize all acks for this node and send them to the network */
    /* printf("%d ack messages to be sent for receivingNode 0x%x\n",
                   receivingNode->pendingAckCount, (unsigned int)receivingNode);
     */
    controlBuffer = nw_plugControlBuffer(sendChannel->controlWriteBuffer);
    nw_plugSendChannelInitializeControlBuffer(sendChannel,
        controlBuffer);
    firstMessage = NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(controlBuffer);
    remainingSize = UI(sendChannel->controlWriteBufferEnd) -
        UI(firstMessage);
    maxNofMessages = remainingSize / NW_CONTROL_MESSAGE_SIZE;
    totNofMessages = 0;
    for (partitionId = 0;
         partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
         partitionId++) {
        partitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, partitionId);

        /* Every resolutiontick, lower the counter that controls the retransmission of the
         * initial ack for this partitionNode.
         */
        if ( partitionNode->repeatFirstAckCntDwn > 0 ) {
            partitionNode->repeatFirstAckCntDwn--;
        }

        NW_CONFIDENCE((partitionNode->pendingAckList == NULL) ==
                      (partitionNode->pendingAckCount == 0));
            do {
                nw_plugPartitionNodeSerializeAndReleaseAcks(partitionNode,
                    &firstMessage, maxNofMessages,
                    &nofMessages, &more);
                totNofMessages += nofMessages;
                maxNofMessages -= nofMessages;
                if (maxNofMessages == 0) {
                    nw_plugControlBufferSetNrOfMessages(controlBuffer,
                        totNofMessages);
                    length = totNofMessages * NW_CONTROL_MESSAGE_SIZE +
                        NW_PLUGCONTROLBUFFER_HEADERSIZE;
                    NW_CONFIDENCE(sizeof(receivingNode->address) == sizeof(os_sockaddr_storage));

                    /*crc check*/
                    if (sendChannel->crc_check) {
                        nw_plugBuffer(controlBuffer)->crc = ut_crcCalculate(channel->crc,((os_char*)controlBuffer)+sizeof(NW_STRUCT(nw_plugBuffer)),length-sizeof(NW_STRUCT(nw_plugBuffer)));
                    }
                    nw_plugBufferSetLength(nw_plugBuffer(controlBuffer),length);
                    nw_socketSendControlTo(nw__plugChannelGetSocket(channel),
                                            receivingNode->address,
                                            controlBuffer, length);
                    if (sendChannel->pss != NULL) {
                        if (sendChannel->pss->enabled) {
                            sendChannel->pss->numberOfAcksSent++;
                        }
                    }
                    firstMessage = NW_PLUGCONTROLBUFFER_FIRSTMESSAGE(controlBuffer);
                    remainingSize = UI(sendChannel->controlWriteBufferEnd) -
                        UI(firstMessage);
                    maxNofMessages = remainingSize / NW_CONTROL_MESSAGE_SIZE;
                    totNofMessages = 0;
                }
            } while (more);
    }

    if (totNofMessages > 0) {
        nw_plugControlBufferSetNrOfMessages(controlBuffer,
            totNofMessages);
        length = totNofMessages * NW_CONTROL_MESSAGE_SIZE +
            NW_PLUGCONTROLBUFFER_HEADERSIZE;
        NW_CONFIDENCE(sizeof(receivingNode->address) == sizeof(os_sockaddr_storage));

        /* crc check */
        if (sendChannel->crc_check) {
            nw_plugBuffer(controlBuffer)->crc = ut_crcCalculate(channel->crc,((os_char*)controlBuffer)+sizeof(NW_STRUCT(nw_plugBuffer)),length-sizeof(NW_STRUCT(nw_plugBuffer)));
        }

        nw_plugBufferSetLength(nw_plugBuffer(controlBuffer),length);
        nw_socketSendControlTo(nw__plugChannelGetSocket(channel),
            receivingNode->address,
            controlBuffer, length);
        if (sendChannel->pss != NULL) {
            if (sendChannel->pss->enabled) {
               sendChannel->pss->numberOfAcksSent++;
            }
        }
    }
}

static void nw_periodicSendAcks(nw_plugChannel channel,
                                nw_plugReceivingNode receivingNode) {

    if (!NW_SECURITY_DISABLED && NW_SECURITY_ENCODER_HOOK(nw_plugSendChannel(channel))) { /* Quick check for security module */
        nw_periodicSendAcksBySecureFlows(channel, receivingNode);
    } else {
        /* else-branch is the fast path */
        nw_periodicSendAcksBundled(channel, receivingNode);
    }
}

static void
nw_periodicResend( nw_plugChannel channel,
                   nw_plugReceivingNode receivingNode,
                   nw_signedLength *credits)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_length nofPostponedResends;
    nw_partitionId partitionId;
    nw_plugPartitionNode partitionNode;
    nw_resendItem resendItem;
    nw_plugDataBuffer dataBuffer;
    nw_length length;
    char *addressString;

    /* Now walk over all plugPartitionNodes in order to find out which
     * data to resend and do the actual resend */

    nofPostponedResends = 0;
    for (partitionId = 0;
         partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
         partitionId++) {
        partitionNode = NW_RECEIVINGNODE_PARTITIONNODE_BY_ID(receivingNode, partitionId);

        NW_CONFIDENCE((partitionNode->pendingResendListHead == NULL) ==
                      (partitionNode->pendingResendCount == 0));
        if (partitionNode->pendingResendCount > 0) {
            resendItem = partitionNode->pendingResendListHead;
            if (resendItem->controlFlushPass >
                sendChannel->recoveryFactor * sendChannel->maxRetries) {

                if (receivingNode->nodeId != 12345) { /*TODO neat fix for ignore ghost node 12345*/
                    addressString = os_sockaddrAddressToString((struct sockaddr*) &receivingNode->address,
                                                               (char*) os_malloc(INET6_ADDRSTRLEN),
                                                               INET6_ADDRSTRLEN);
                    NW_REPORT_WARNING_5("reliable protocol",
                        "plugChannel %s: Node 0x%x (address %s) has not acked message (id=%d) after %d retries",
                        nw__plugChannelGetName(channel),
                        receivingNode->nodeId,
                        addressString,
                        resendItem->packetNr,
                        sendChannel->maxRetries);
                    os_free(addressString);
                }
                nw_plugReceivingNodeNotResponding(receivingNode);
            } else {
                do {
                    resendItem->controlFlushPass++;
                    if ((resendItem->controlFlushPass % sendChannel->recoveryFactor) == 0) {
                        /* the buffer is encoded,
                         *  we must not read or write the buffer anymore */
                        dataBuffer = resendItem->buffer;
                        /*length = nw_plugBufferGetLength(nw_plugBuffer(dataBuffer));*/
                        length = resendItem->bufferLength;

                        if ( *credits > 0 ){
                             *credits -= length;
                            if (!(os_int)nw_configurationLoseSentMessage()) {
                                /* Send data to network */
                                NW_CONFIDENCE(sizeof(receivingNode->address) == sizeof(os_sockaddr_storage));

                                /* data buffer has been attached to
                                 * resend-queue being encoded */

                                nw_socketSendDataTo(nw__plugChannelGetSocket(channel),
                                    receivingNode->address,
                                    dataBuffer, length);

                                sendChannel->totalResendCount++;
                                if ((sendChannel->totalResendCount % sendChannel->reportInterval) == 0) {
                                    NW_TRACE_2(Send, 2, "plugChannel %s: %u fragments resent",
                                        nw__plugChannelGetName(channel),
                                        sendChannel->totalResendCount);
                                }


                                addressString = os_sockaddrAddressToString((struct sockaddr*) &receivingNode->address,
                                                                          (char*) os_malloc(INET6_ADDRSTRLEN),
                                                                          INET6_ADDRSTRLEN);
                                NW_TRACE_5(Send, 3, "plugChannel %s: Resending packet (id=%d,count=%d) to Node 0x%x (address %s)",
                                    nw__plugChannelGetName(channel),
                                    resendItem->packetNr,
                                    resendItem->controlFlushPass,
                                    receivingNode->nodeId,
                                    addressString);
                                os_free(addressString);

                                if (sendChannel->pss != NULL) {
                                    if (sendChannel->pss->enabled) {
                                        sendChannel->pss->numberOfBytesResent+= length;
                                        sendChannel->pss->numberOfPacketsResent++;
                                    }
                                }
                            }
                        } else {
                            /* Scheduled for retry now, but this is postponed due to resent budget */
                            resendItem->controlFlushPass--;
                            nofPostponedResends++;
                        }
                    }
                    resendItem = resendItem->next;
                } while (resendItem != NULL);
            }
        }
    }
    if ( nofPostponedResends > 0 ) {
        NW_TRACE_3(Send, 4, "plugChannel %s: postponed resending of %d fragments to 0x%x",
                nw__plugChannelGetName(channel),
                nofPostponedResends,
                receivingNode->address);
    }
}


static void
nw_periodicClearLateJoiningLists( nw_plugChannel channel )
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_partitionId partitionId;
    nw_plugReceivingPartition receivingPartition;
    nw_resendItem resendItem;
    nw_resendItem* nextPrevPtr;

    /* Now walk over all plugPartitions and remove the old resendItems
     * from the LateJoining lists */
    for (partitionId = 0;
         partitionId < NW_SENDCHANNEL_NOF_PARTITIONS(sendChannel);
         partitionId++) {
        receivingPartition = NW_SENDCHANNEL_PARTITION_BY_ID(sendChannel, partitionId);
        if (receivingPartition != NULL) {
            resendItem = receivingPartition->lateJoiningResendListHead;
            while ((resendItem != NULL) &&
                   (resendItem->controlFlushPass >
                       (sendChannel->maxRetries * sendChannel->recoveryFactor))) {
                if (resendItem->next != NULL) {
                    nextPrevPtr = &(resendItem->next->prev);
                } else {
                    nextPrevPtr = &(receivingPartition->lateJoiningResendListTail);
                }
                nw_plugReceivingPartitionResendItemRelease(receivingPartition, resendItem,
                    &receivingPartition->lateJoiningResendListHead, nextPrevPtr);
                resendItem = receivingPartition->lateJoiningResendListHead;
            }
            while (resendItem != NULL) {
                resendItem->controlFlushPass++;
                resendItem = resendItem->next;
            }
        }
    }
}

#define NW_RESEND_BANDWIDTH_DIVISOR (3)

static nw_signedLength
nw_updateCredits(
    nw_plugChannel channel,
    nw_signedLength *credits)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_signedLength resend_credits;

    resend_credits = sendChannel->currentPeriodBudget;
    *credits += resend_credits;
    if (*credits > (nw_signedLength)(sendChannel->currentPeriodBudget)) {
        *credits = sendChannel->currentPeriodBudget;
    }

    return sendChannel->currentPeriodBudget - resend_credits;
}


/* Perform periodic actions:
  *  - send control messages  (reliable only)
  *  - resend data (reliable only)
  *  - process discovery messages (reliable only)
  *  - throttle maxBurstSize (reliable only)
  *  - Handout Credits for the coming period (reliable and best effort)
  *
  * credits is an In/Out parameter:
  * In: Leftover credits from the last period
  * Out: credits for the coming period available for new data
  */
void
nw_plugSendChannelPeriodicAction(
    nw_plugChannel channel,
    nw_signedLength *credits,
    plugSendStatistics pss)
{
    nw_plugSendChannel sendChannel = nw_plugSendChannel(channel);
    nw_plugReceivingNode receivingNode;
    nw_signedLength post_resend_credits;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_SEND);
    NW_CONFIDENCE(!sendChannel->inUse);

    sendChannel->pss = pss;

    /* Check for new messages from the network-discovery */
    nw_periodicCheckMessageBox(channel);


    if (nw_plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE) {

        /* Process Ack and Data notifications from the interchannel-queue*/
        nw_periodicProcessIncomingAcks(channel);
        nw_periodicProcessReceivedData(channel);

        /* process multi-node reliability messages from the interchannel-queue*/
        nw_periodicProcessReceivedBackup(channel);
        nw_periodicProcessDataRequest(channel);
        nw_periodicProcessDataAnnounce(channel);

        /* Adapt outgoing Throughput limit (currentPeriodBudget), based on the status of remote nodes */
        nw_autoThrottle( channel, *credits );

        /* Update credits, based on the currentPeriodBudget */
        post_resend_credits = nw_updateCredits(channel,credits);

        /* Walk over all receiving nodes, to send Acks and resends */
        receivingNode = sendChannel->receivingNodesHead;
        while (receivingNode != NULL) {
            if (receivingNode->state == NW_NODESTATE_RESPONDING) {
                nw_periodicSendAcks(channel,receivingNode);
                nw_periodicResend( channel,receivingNode,credits);
            }
            receivingNode = receivingNode->nextInList;
        }

        /* Clear old items from the resend list for late joining nodes */
        nw_periodicClearLateJoiningLists(channel);

        /* Add the remaining budget to Credits for the coming period.*/
        *credits += post_resend_credits;
        if (*credits > (nw_signedLength)(sendChannel->currentPeriodBudget)) {
            *credits = sendChannel->currentPeriodBudget;
        }
        sendChannel->lastPeriodCredits = *credits;
        /* trigger the receive channel */
        nw_plugInterChannelSetTrigger(nw__plugChannelGetInterChannelComm(channel));
    } else {
         /* For best effort channels, the maxBurstSize budget is completely available for new data */
        *credits = sendChannel->currentPeriodBudget;
    }
    sendChannel->pss = NULL;

}

#undef NW_RECEIVING_NODES_HASHSIZE
#undef NW_CONTROL_MESSAGE_SIZE


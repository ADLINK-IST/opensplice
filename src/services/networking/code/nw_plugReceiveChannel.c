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
/* interface */
#include "nw__plugReceiveChannel.h"

/* implementation */
#include "os_heap.h"
#include "nw_misc.h"
#include "nw_commonTypes.h"
#include "nw__plugChannel.h"
#include "nw__plugDataBuffer.h"
#include "nw__plugControlBuffer.h"
#include "nw__confidence.h"
#include "nw_configuration.h"
#include "nw_report.h"

/* --------------------------- nw_plugReceivChannel ----------------------- */

NW_CLASS(nw_plugBufferDefragAdmin);
NW_CLASS(nw_msgHolderPtr);
NW_CLASS(nw_plugSendingPartitionNode);

NW_STRUCT(nw_plugReceiveChannel) {
    NW_EXTENDS(nw_plugChannel);
    /* Reading and defragmentation stuff */
    /* Boolean indicating if anybody wants to interrupt */
    nw_bool wakeupRequested;
    /* Linked list of known sending nodes */
    nw_plugSendingPartitionNode sendingPartitionNodes;
    /* Linked list of free buffers not in use */
    nw_plugBufferDefragAdmin freeBuffers;
    /* Linked list of data buffers waiting to be processed */
    nw_plugBufferDefragAdmin waitingDataBuffersHead;
    nw_plugBufferDefragAdmin waitingDataBuffersTail;
    /* Linked list of pointer objects to message starting points */
    nw_msgHolderPtr freeMsgHolderPtrs;
    nw_msgHolderPtr readyMsgHolderPtrsHead; /* For taking */
    nw_msgHolderPtr readyMsgHolderPtrsTail; /* For appending */
    nw_plugBufferDefragAdmin lastReturnedBuffer;
    nw_messageHolder lastReturnedHolder;
    nw_msgHolderPtr lastReturnedMsgHolderPtr;
    /* Currently for confidence only */
    nw_seqNr nofFreeBuffers;
    nw_seqNr nofUsedBuffers;
    nw_seqNr totNofBuffers;
    nw_seqNr maxNofBuffers;
    nw_seqNr maxReliabBacklog;
    nw_seqNr fragmentsOutOfOrderDropped;
    nw_seqNr lastReceivedFragment;
#ifdef NW_TRACING
    nw_seqNr buffersAllocatedTrace;
    nw_seqNr buffersUsedTrace;
    nw_seqNr buffersUsedTraceBottom;
#endif
    /* Boolean for confidence checking */
    nw_bool inUse;
};



/* ------------------------ nw_plugBufferDefragAdmin -------------------- */

NW_STRUCT(nw_plugBufferDefragAdmin) {
    nw_plugBufferDefragAdmin prev;
    nw_plugBufferDefragAdmin next;
    nw_address senderAddress;
    nw_seqNr usedCount;
};

#define UI(val)       ((nw_length)(val))
#define UI_DEREF(ptr) (*(nw_length *)(ptr))

#define DF_ADMIN_LENGTH (UI(sizeof(NW_STRUCT(nw_plugBufferDefragAdmin))))
#define DF_ADMIN(ptr)   (((ptr) != NULL)?(nw_plugBufferDefragAdmin)(UI(ptr) - DF_ADMIN_LENGTH): NULL)
#define DF_BUFFER(ptr)  ((((ptr) != NULL)?nw_plugBuffer(UI(ptr) + DF_ADMIN_LENGTH):NULL))
#define DF_DATA(ptr)    (nw_plugDataBuffer(DF_BUFFER(ptr)))
#define DF_CONTROL(ptr) (nw_plugControlBuffer(DF_BUFFER(ptr)))

#if 0
#define PRINT_USEDCOUNT(admin)       \
    printf("%s:%d (0x%x) usedCount = %d\n", __FILE__, __LINE__, (unsigned int)admin, admin->usedCount)
#else
#define PRINT_USEDCOUNT(admin)
#endif

#define SET_USEDCOUNT(admin, value) admin->usedCount = value; PRINT_USEDCOUNT(admin)
#define INC_USEDCOUNT(admin)        pa_increment(admin->usedCount++);       PRINT_USEDCOUNT(admin)
#define DEC_USEDCOUNT(admin)        pa_decrement(admin->usedCount--);       PRINT_USEDCOUNT(admin)

static nw_plugBufferDefragAdmin
nw_plugBufferDefragAdminCreate(
    nw_plugReceiveChannel receiveChannel)
{
    nw_plugBufferDefragAdmin admin = NULL;

    if (receiveChannel->freeBuffers != NULL) {
        /* Get a buffer from the buffer pool */
        admin = receiveChannel->freeBuffers;
        receiveChannel->freeBuffers = admin->next;
        receiveChannel->nofUsedBuffers++;
        receiveChannel->nofFreeBuffers--;
    } else {
        /* Allocate a new buffer from heap */
        NW_CONFIDENCE(receiveChannel->nofFreeBuffers == 0);
        if ((receiveChannel->maxNofBuffers == 0) ||
            (receiveChannel->totNofBuffers < receiveChannel->maxNofBuffers)) {
            admin = (nw_plugBufferDefragAdmin)os_malloc(
                nw__plugChannelGetFragmentLength(nw_plugChannel(receiveChannel)) +
                DF_ADMIN_LENGTH);
            receiveChannel->nofUsedBuffers++;
            receiveChannel->totNofBuffers++;
#ifdef NW_TRACING
                    if (receiveChannel->totNofBuffers == 2*receiveChannel->buffersAllocatedTrace) {
                        NW_TRACE_2(Receive, 2, "Total number of defragmentation buffers "
                            "for channel \"%s\" has climbed to %u",
                            nw__plugChannelGetName(nw_plugChannel(receiveChannel)),
                            receiveChannel->totNofBuffers);
                        receiveChannel->buffersAllocatedTrace = 2*receiveChannel->buffersAllocatedTrace;
                    }
#endif
        }
    }

    if (admin != NULL) {
        admin->prev = NULL;
        admin->next = NULL;
        admin->senderAddress = 0;
        SET_USEDCOUNT(admin, 1);
    }

    return admin;
}


#define nw_plugBufferDefragAdminFree(admin) os_free(admin)
#define nw_plugBufferDefragAdminKeep(admin) INC_USEDCOUNT(admin)

static void
nw_plugBufferDefragAdminRelease(
    nw_plugReceiveChannel receiveChannel,
    nw_plugBufferDefragAdmin admin)
{
    NW_CONFIDENCE(admin->usedCount > 0);

    DEC_USEDCOUNT(admin);
    if (admin->usedCount == 0) {
        if (admin->prev != NULL) {
            admin->prev->next = admin->next;
            if(admin->next != NULL)
            {
                admin->next->prev = admin->prev;
            }
        }
        if (receiveChannel->freeBuffers != NULL) {
            receiveChannel->freeBuffers->prev = admin;
        }
        admin->next = receiveChannel->freeBuffers;
        admin->prev = NULL;
        receiveChannel->freeBuffers = admin;
        receiveChannel->nofUsedBuffers--;
        receiveChannel->nofFreeBuffers++;
        if ((receiveChannel->nofUsedBuffers > receiveChannel->buffersUsedTraceBottom) &&
                (receiveChannel->nofUsedBuffers == receiveChannel->buffersUsedTrace/2)) {
            NW_TRACE_2(Receive, 2, "Number of defragmentation buffers in use "
                "for channel \"%s\" has dropped to %u",
                nw__plugChannelGetName(nw_plugChannel(receiveChannel)),
                receiveChannel->nofUsedBuffers);
            receiveChannel->buffersUsedTrace = receiveChannel->buffersUsedTrace / 2;
        }
    }
}

/* --------------------------- Defragmentation ------------------------------ */

NW_STRUCT(nw_msgHolderPtr) {
    nw_messageHolder messageHolder;
    nw_plugBufferDefragAdmin firstAdmin;
    nw_plugBufferDefragAdmin lastAdmin;
    nw_seqNr fragmentedMsgNr;
    nw_seqNr totNrOfFragmentsContained;
    nw_seqNr totNrOfFragmentsNeeded;
    nw_msgHolderPtr prev;
    nw_msgHolderPtr next;
};


#define nw_msgHolderPtrIsComplete(msgHolderPtr) \
    ((msgHolderPtr)->totNrOfFragmentsContained == \
     (msgHolderPtr)->totNrOfFragmentsNeeded)



static nw_msgHolderPtr
nw_msgHolderPtrCreate(
    nw_plugReceiveChannel receiveChannel,
    nw_msgHolderPtr *prevNext,
    nw_msgHolderPtr *nextPrev)
{
    nw_msgHolderPtr result;

    if (receiveChannel->freeMsgHolderPtrs == NULL) {
        result = (nw_msgHolderPtr)os_malloc(sizeof(*result));
    } else {
        result = receiveChannel->freeMsgHolderPtrs;
        receiveChannel->freeMsgHolderPtrs = result->next;
    }
    NW_CONFIDENCE(result != NULL);

    result->messageHolder = NULL;
    result->firstAdmin = NULL;
    result->lastAdmin = NULL;
    result->totNrOfFragmentsContained = 0;
    result->totNrOfFragmentsNeeded = 0;
    result->next = *prevNext;
    *prevNext = result;
    result->prev = *nextPrev;
    *nextPrev = result;

    return result;
}

#define nw_msgHolderPtrFree(msgHolderPtr) os_free(msgHolderPtr)

static void
nw_msgHolderPtrRelease(
    nw_plugReceiveChannel receiveChannel,
    nw_msgHolderPtr msgHolderPtr)
{
    receiveChannel->readyMsgHolderPtrsHead = msgHolderPtr->next;
    if (msgHolderPtr->next != NULL) {
        msgHolderPtr->next->prev = NULL;
    }
    if (receiveChannel->readyMsgHolderPtrsHead == NULL) {
        receiveChannel->readyMsgHolderPtrsTail = NULL;
    }
    msgHolderPtr->next = receiveChannel->freeMsgHolderPtrs;
    if (msgHolderPtr->next != NULL) {
        msgHolderPtr->next->prev = msgHolderPtr;
    }
    msgHolderPtr->prev = NULL;
    receiveChannel->freeMsgHolderPtrs = msgHolderPtr;
}

static void
nw_msgHolderPtrAddCompleteMessage(
    nw_msgHolderPtr msgHolderPtr,
    nw_plugBufferDefragAdmin admin,
    nw_messageHolder messageHolder)
{
    NW_CONFIDENCE(msgHolderPtr->firstAdmin == NULL);
    NW_CONFIDENCE(msgHolderPtr->lastAdmin == NULL);

    msgHolderPtr->firstAdmin = admin;
    msgHolderPtr->lastAdmin = admin;
    msgHolderPtr->messageHolder = messageHolder;
    msgHolderPtr->fragmentedMsgNr = 0;
    msgHolderPtr->totNrOfFragmentsContained = 1;
    msgHolderPtr->totNrOfFragmentsNeeded = 1;
    /* One for the head */
    nw_plugBufferDefragAdminKeep(admin);
    /* One for the tail */
    nw_plugBufferDefragAdminKeep(admin);
}

static void
nw_msgHolderPtrAddHeadAdmin(
    nw_msgHolderPtr msgHolderPtr,
    nw_plugBufferDefragAdmin admin,
    nw_messageHolder messageHolder)
{
    NW_CONFIDENCE(msgHolderPtr->firstAdmin == NULL);

    msgHolderPtr->firstAdmin = admin;
    msgHolderPtr->totNrOfFragmentsContained++;
    msgHolderPtr->messageHolder = messageHolder;
    /* Keep because of firstAdmin */
    nw_plugBufferDefragAdminKeep(admin);
}

static void
nw_msgHolderPtrAddMiddleAdmin(
    nw_msgHolderPtr msgHolderPtr,
    nw_plugBufferDefragAdmin admin)
{
    msgHolderPtr->totNrOfFragmentsContained++;
    /* Keep because of firstAdmin */
    nw_plugBufferDefragAdminKeep(admin);
}

static void
nw_msgHolderPtrAddTailAdmin(
    nw_msgHolderPtr msgHolderPtr,
    nw_plugBufferDefragAdmin admin,
    nw_seqNr nrOfFragmentsNeeded)
{
    NW_CONFIDENCE(msgHolderPtr->lastAdmin == NULL);

    msgHolderPtr->lastAdmin = admin;
    msgHolderPtr->totNrOfFragmentsContained++;
    msgHolderPtr->totNrOfFragmentsNeeded = nrOfFragmentsNeeded;
    /* Keep because of firstAdmin */
    nw_plugBufferDefragAdminKeep(admin);
    /* Keep because of lastAdmin */
    nw_plugBufferDefragAdminKeep(admin);
}




/* ---------------------------- nw_plugSendingPartitionNode -------------------------- */

NW_STRUCT(nw_plugSendingPartitionNode) {
    nw_seqNr nodeId;
    nw_partitionId partitionId;
    nw_bool active;
    nw_seqNr packetNrWaitingFor;
    NW_STRUCT(nw_plugBufferDefragAdmin) outOfOrderAdminsHead;
    NW_STRUCT(nw_plugBufferDefragAdmin) outOfOrderAdminsTail;
    NW_STRUCT(nw_plugBufferDefragAdmin) defragAdminsHead;
    NW_STRUCT(nw_plugBufferDefragAdmin) defragAdminsTail;
    nw_msgHolderPtr msgHolderPtrsHead;
    nw_msgHolderPtr msgHolderPtrsTail;
    nw_plugSendingPartitionNode next;
};

static nw_plugSendingPartitionNode
nw_plugSendingPartitionNodeNew(
    nw_seqNr nodeId,
    nw_partitionId partitionId,
    nw_plugSendingPartitionNode *prevNext)
{
    nw_plugSendingPartitionNode result;

    result = (nw_plugSendingPartitionNode)os_malloc(sizeof(*result));

    if (result != NULL) {
        result->nodeId = nodeId;
        result->partitionId = partitionId;
        result->active = TRUE;
        result->packetNrWaitingFor = 0; /* First packet can be any packet */
        result->msgHolderPtrsHead = NULL;
        result->msgHolderPtrsTail = NULL;
        result->outOfOrderAdminsHead.prev = NULL;
        result->outOfOrderAdminsHead.next = &result->outOfOrderAdminsTail;
        result->outOfOrderAdminsTail.prev = &result->outOfOrderAdminsHead;
        result->outOfOrderAdminsTail.next = NULL;
        result->defragAdminsHead.prev = NULL;
        result->defragAdminsHead.next = &result->defragAdminsTail;
        result->defragAdminsTail.prev = &result->defragAdminsHead;
        result->defragAdminsTail.next = NULL;
        result->next = *prevNext;
        *prevNext = result;
    }

    return result;
}

static void
nw_plugSendingPartitionNodeFree(
    nw_plugSendingPartitionNode sendingPartitionNode)
{
    nw_plugBufferDefragAdmin admin;
    nw_msgHolderPtr msgHolderPtr;

    admin = sendingPartitionNode->outOfOrderAdminsHead.next;
    while (admin->next != NULL) {
        sendingPartitionNode->outOfOrderAdminsHead.next = admin->next;
        nw_plugBufferDefragAdminFree(admin);
        admin = sendingPartitionNode->outOfOrderAdminsHead.next;
    }

    admin = sendingPartitionNode->defragAdminsHead.next;
    while (admin->next != NULL) {
        sendingPartitionNode->defragAdminsHead.next = admin->next;
        nw_plugBufferDefragAdminFree(admin);
        admin = sendingPartitionNode->defragAdminsHead.next;
    }

    while (sendingPartitionNode->msgHolderPtrsHead != NULL) {
        msgHolderPtr = sendingPartitionNode->msgHolderPtrsHead;
        sendingPartitionNode->msgHolderPtrsHead = msgHolderPtr->next;
        nw_msgHolderPtrFree(msgHolderPtr);
    }
}

static void
nw_plugSendingPartitionNodeDeclareMsgReady(
    nw_plugReceiveChannel receiveChannel,
    nw_plugSendingPartitionNode sendingPartitionNode,
    nw_msgHolderPtr msgHolderPtr)
{
    nw_msgHolderPtr *prevNext;
    nw_msgHolderPtr *nextPrev;


    /* First remove */
    if (sendingPartitionNode->msgHolderPtrsHead == msgHolderPtr) {
        NW_CONFIDENCE(msgHolderPtr->prev == NULL);
        prevNext = &sendingPartitionNode->msgHolderPtrsHead;
    } else {
        prevNext = &msgHolderPtr->prev->next;
    }
    if (sendingPartitionNode->msgHolderPtrsTail == msgHolderPtr) {
        NW_CONFIDENCE(msgHolderPtr->next == NULL);
        nextPrev = &sendingPartitionNode->msgHolderPtrsTail;
    } else {
        nextPrev = &msgHolderPtr->next->prev;
    }

    *prevNext = msgHolderPtr->next;
    *nextPrev = msgHolderPtr->prev;

    /* Then append */
    if (receiveChannel->readyMsgHolderPtrsHead == NULL) {
        prevNext = &receiveChannel->readyMsgHolderPtrsHead;
    } else {
        prevNext = &receiveChannel->readyMsgHolderPtrsTail->next;
    }

    NW_CONFIDENCE(*prevNext == NULL);
    msgHolderPtr->next = NULL;
    *prevNext = msgHolderPtr;
    msgHolderPtr->prev = receiveChannel->readyMsgHolderPtrsTail;
    receiveChannel->readyMsgHolderPtrsTail = msgHolderPtr;
}

static nw_bool
nw_plugSendingPartitionNodeIsActive(
    nw_plugSendingPartitionNode sendingPartitionNode)
{
    return sendingPartitionNode->active;
}

static void
nw_receiveChannelCreateMsgReady(
    nw_plugReceiveChannel receiveChannel,
    nw_plugBufferDefragAdmin admin,
    nw_messageHolder messageHolder)
{
    nw_msgHolderPtr *prevNext;
    nw_msgHolderPtr created;


    /* Create at the end of the list */
    /* Then append */
    if (receiveChannel->readyMsgHolderPtrsHead == NULL) {
        prevNext = &receiveChannel->readyMsgHolderPtrsHead;
    } else {
        prevNext = &receiveChannel->readyMsgHolderPtrsTail->next;
    }

    NW_CONFIDENCE(*prevNext == NULL);
    created = nw_msgHolderPtrCreate(receiveChannel, prevNext,
        &receiveChannel->readyMsgHolderPtrsTail);
    nw_msgHolderPtrAddCompleteMessage(created, admin, messageHolder);
}

static nw_msgHolderPtr
nw_plugSendingPartitionNodeLookupOrCreateMsgHolderPtr(
    nw_plugReceiveChannel receiveChannel,
    nw_plugSendingPartitionNode sendingPartitionNode,
    nw_seqNr fragmentedMsgNumberLookingFor)
{
    nw_msgHolderPtr result;
    nw_msgHolderPtr msgHolderPtr;
    nw_msgHolderPtr *prevNext = (nw_msgHolderPtr *)0xBADCAFE;
    nw_msgHolderPtr *nextPrev = (nw_msgHolderPtr *)0xBADCAFE;
    nw_bool creationNeeded;

    /* Look for the msgHolderPtr with this message number. Start at the end
     * because it is likely that we receive data in order. */
    msgHolderPtr = sendingPartitionNode->msgHolderPtrsTail;
    result = NULL;
    creationNeeded = FALSE;
    while ((msgHolderPtr != NULL) && (result == NULL) && !creationNeeded) {
        if (msgHolderPtr->fragmentedMsgNr < fragmentedMsgNumberLookingFor) {
            prevNext = &msgHolderPtr->next;
            if (msgHolderPtr == sendingPartitionNode->msgHolderPtrsTail) {
                nextPrev = &sendingPartitionNode->msgHolderPtrsTail;
            } else {
                nextPrev = &msgHolderPtr->next->prev;
            }
            creationNeeded = TRUE;
        } else if (msgHolderPtr->fragmentedMsgNr == fragmentedMsgNumberLookingFor) {
            result = msgHolderPtr;
        } else {
            msgHolderPtr = msgHolderPtr->prev;
        }
    }
    if ((result == NULL) && (!creationNeeded)) {
        prevNext = &sendingPartitionNode->msgHolderPtrsHead;
        if (sendingPartitionNode->msgHolderPtrsHead == NULL) {
            NW_CONFIDENCE(sendingPartitionNode->msgHolderPtrsTail == NULL);
            nextPrev = &sendingPartitionNode->msgHolderPtrsTail;
        } else {
            nextPrev = &sendingPartitionNode->msgHolderPtrsHead->prev;
        }
        creationNeeded = TRUE;
    }
    if (creationNeeded == TRUE) {
        result = nw_msgHolderPtrCreate(receiveChannel, prevNext, nextPrev);
        result->fragmentedMsgNr = fragmentedMsgNumberLookingFor;
    }
    NW_CONFIDENCE(result != NULL);
    return result;
}

static nw_plugBufferDefragAdmin
nw_plugSendingPartitionNodeOutOfOrderListTake(
    nw_plugSendingPartitionNode sendingPartitionNode,
    nw_bool orderPreservation)
{
    nw_plugBufferDefragAdmin result = NULL;
    nw_seqNr firstWaitingPacketNr;
    nw_bool waiting = FALSE;

    NW_CONFIDENCE((sendingPartitionNode->outOfOrderAdminsHead.next->next == NULL) ==
                  (sendingPartitionNode->outOfOrderAdminsTail.prev->prev == NULL));
    if (sendingPartitionNode->outOfOrderAdminsHead.next->next != NULL) {
        if (orderPreservation) {
            firstWaitingPacketNr = nw_plugDataBufferGetPacketNr(
                DF_DATA(sendingPartitionNode->outOfOrderAdminsHead.next));
            if (firstWaitingPacketNr < sendingPartitionNode->packetNrWaitingFor) {
                waiting = TRUE;
            }
        } else {
            NW_CONFIDENCE(sendingPartitionNode->packetNrWaitingFor == 0);
            waiting = TRUE;
        }
        if (waiting) {
            result = sendingPartitionNode->outOfOrderAdminsHead.next;
            sendingPartitionNode->outOfOrderAdminsHead.next = result->next;
            result->next->prev = result->prev;
        }
    }
    NW_CONFIDENCE((sendingPartitionNode->outOfOrderAdminsHead.next->next == NULL) ==
                  (sendingPartitionNode->outOfOrderAdminsTail.prev->prev == NULL));
    return result;
}

static nw_plugBufferDefragAdmin
nw_plugSendingPartitionNodeOutOfOrderListInsert(
    nw_plugReceiveChannel receiveChannel,
    nw_plugSendingPartitionNode sendingPartitionNode,
    nw_plugBufferDefragAdmin admin,
    nw_bool orderPreservation,
    nw_bool isReinsert,
    nw_bool *isDroppedReliably)
{
    nw_plugBufferDefragAdmin result;
    nw_seqNr packetNrLookingFor;
    nw_bool isOutOfOrder;
    nw_bool isOld;
    nw_bool insertionNeeded;
    nw_seqNr packetNrCurrent;
    nw_seqNr packetNrPrevious;
    nw_plugBufferDefragAdmin currentAdmin;
    nw_bool holeFound;
    nw_seqNr OutOfOrderCount;

    packetNrLookingFor = nw_plugDataBufferGetPacketNr(DF_DATA(admin));

    result = NULL;
    *isDroppedReliably = FALSE;
    isOutOfOrder = FALSE;
    isOld = FALSE;
    OutOfOrderCount = 0;
    if (orderPreservation) {
        /* If this packet was received out of order, insert in into the out of
         * order list or discard it if it is old */
        if (sendingPartitionNode->packetNrWaitingFor == 0) {
            sendingPartitionNode->packetNrWaitingFor = packetNrLookingFor;
        } else {
            if (packetNrLookingFor == sendingPartitionNode->packetNrWaitingFor) {
            } else if (packetNrLookingFor > sendingPartitionNode->packetNrWaitingFor) {
                isOutOfOrder = TRUE;
            } else {
                isOld = TRUE;
            }
        }
    }

    if (isReinsert || !isOld) {
        /* Only packets not considered old need to be inserted */
        currentAdmin = sendingPartitionNode->outOfOrderAdminsTail.prev;
#ifndef NW_DEBUGGING
        /* Doublecheck if not outOfOrder indeed means appending after the last buffer */
        if (!isOutOfOrder) {
            if (currentAdmin->prev != NULL) {
                packetNrCurrent = nw_plugDataBufferGetPacketNr(DF_DATA(currentAdmin));
                NW_CONFIDENCE(packetNrCurrent+1 == packetNrLookingFor);
            }
        }
#endif
        insertionNeeded = FALSE;
        while ((currentAdmin->prev != NULL) &&
               (result == NULL) && !insertionNeeded) {
            packetNrCurrent = nw_plugDataBufferGetPacketNr(DF_DATA(currentAdmin));
            if (packetNrCurrent == packetNrLookingFor) {
                result = currentAdmin;
                if (orderPreservation) {
                    *isDroppedReliably = TRUE;
                }
            } else if (packetNrCurrent < packetNrLookingFor) {
                insertionNeeded = TRUE;
            } else {
                currentAdmin = currentAdmin->prev;
            }
            OutOfOrderCount++;
        }
        if (result == NULL) {
            NW_CONFIDENCE(insertionNeeded || (currentAdmin->prev == NULL));
            admin->next = currentAdmin->next;
            admin->prev = currentAdmin;
            admin->next->prev = admin;
            admin->prev->next = admin;
            nw_plugBufferDefragAdminKeep(admin);
            if (!isOutOfOrder) {
                result = admin;
                if (orderPreservation) {
		  /* Now find the first missing packetnumber.*/
             
                    packetNrPrevious = nw_plugDataBufferGetPacketNr(DF_DATA(admin));
                    currentAdmin = admin->next;
                    holeFound = FALSE;
                    while ((currentAdmin->next != NULL) && !holeFound) {
                        packetNrCurrent = nw_plugDataBufferGetPacketNr(DF_DATA(currentAdmin));
                        NW_CONFIDENCE(packetNrCurrent > packetNrPrevious);
                        if (packetNrCurrent != packetNrPrevious + 1) {
                            holeFound = TRUE;
                        } else {
                            currentAdmin = currentAdmin->next;
                            packetNrPrevious = packetNrCurrent;
                        }
                    }
                    sendingPartitionNode->packetNrWaitingFor = packetNrPrevious+1;
                }
            }
        }
        /*
         * The reliability backlog had grown beyond acceptable limit.
         * The sendingPartitionNode is removed from the reliability protocol
         * for this channel.
         * All packetbuffers that are in use for this SendingPartitionNode will
         * be freed.
         */
        if (OutOfOrderCount > receiveChannel->maxReliabBacklog ){
            NW_REPORT_WARNING_1("reliable protocol",
               "Node 0x%x did not resend a missing packet in time, removing that node from the reliable protocol.",
               sendingPartitionNode->nodeId);
            
            sendingPartitionNode->active = FALSE;
          
            //release all admins for this PartitionNode
            nw_plugSendingPartitionNodeFree(sendingPartitionNode);          
        } 
        

        /* Todo: recheck this assert NW_CONFIDENCE((result == NULL) == isOutOfOrder); */
    } else {
        /* Do not insert anywhere, but prepare for correct release */
        admin->prev = NULL;
        admin->next = NULL;
    }
    return result;
}


static void
nw_plugSendingPartitionNodeDefragListInsert(
    nw_plugSendingPartitionNode sendingPartitionNode,
    nw_plugBufferDefragAdmin admin)
{
    nw_plugBufferDefragAdmin result;
    nw_seqNr packetNrLookingFor;
    nw_plugBufferDefragAdmin currentAdmin;
    nw_bool insertionNeeded;
    nw_seqNr packetNrCurrent;
    nw_bool found;

    packetNrLookingFor = nw_plugDataBufferGetPacketNr(DF_DATA(admin));

    result = NULL;
    currentAdmin = sendingPartitionNode->defragAdminsTail.prev;

    insertionNeeded = FALSE;
    found = FALSE;
    while ((currentAdmin->prev != NULL) &&
           !found && !insertionNeeded) {
        packetNrCurrent = nw_plugDataBufferGetPacketNr(DF_DATA(currentAdmin));
        if (packetNrCurrent == packetNrLookingFor) {
            /* Very very strange, data received twice?? */
            NW_TRACE(Receive, 1, "Received non-reliable data twice. Ignoring.");
            found = TRUE;
        } else if (packetNrCurrent < packetNrLookingFor) {
            insertionNeeded = TRUE;
        } else {
            currentAdmin = currentAdmin->prev;
        }
    }
    if (insertionNeeded || (currentAdmin->prev == NULL)) {
        admin->next = currentAdmin->next;
        admin->prev = currentAdmin;
        admin->next->prev = admin;
        admin->prev->next = admin;
        nw_plugBufferDefragAdminKeep(admin);
    }
}

static nw_plugBufferDefragAdmin
nw_plugSendingPartitionNodeInsertAdmin(
    nw_plugReceiveChannel receiveChannel,
    nw_plugSendingPartitionNode sendingPartitionNode,
    nw_plugBufferDefragAdmin admin,
    nw_bool isReinsert)
{
    nw_plugBufferDefragAdmin insertedAdmin;
    nw_bool orderPreservation;
    nw_bool isDropped;
    nw_msgHolderPtr msgHolderPtr;
    nw_plugDataBuffer dataBuffer;
    nw_messageHolder messageHolder;
    nw_seqNr nrOfMessages;
    nw_bool isFragmented;
    nw_seqNr fragmentedMsgNr;
    nw_seqNr terminatedMsgNr;
    nw_seqNr spareMessages;

    /* Now insert buffer in list */
    orderPreservation =
        (nw__plugChannelGetReliabilityOffered(nw_plugChannel(receiveChannel)) == NW_REL_RELIABLE);
    insertedAdmin = nw_plugSendingPartitionNodeOutOfOrderListInsert(receiveChannel,sendingPartitionNode, admin,
        orderPreservation, isReinsert, &isDropped);
    if (isDropped) {
        receiveChannel->fragmentsOutOfOrderDropped++;
        NW_TRACE_1(Receive, 3, "Channel %s: reliable fragment dropped because it "
            "has been received twice or it is too old",
            nw__plugChannelGetName(nw_plugChannel(receiveChannel)));        
    }
    /* Execute the following action only if the admin is directly available */
    if (insertedAdmin == admin) {
        insertedAdmin = nw_plugSendingPartitionNodeOutOfOrderListTake(sendingPartitionNode, orderPreservation);
        NW_CONFIDENCE(insertedAdmin == admin);
        nw_plugSendingPartitionNodeDefragListInsert(sendingPartitionNode, admin);
        /* Release the buffer taken from the outOfOrderList */
        nw_plugBufferDefragAdminRelease(receiveChannel, admin);
        dataBuffer = nw_plugDataBuffer(DF_DATA(admin));
        nrOfMessages = nw_plugDataBufferGetNrOfMessages(dataBuffer);
        NW_CONFIDENCE(nrOfMessages > 0);
        msgHolderPtr = NULL;
        messageHolder = NULL;
        isFragmented = nw_plugBufferGetFragmentedFlag(nw_plugBuffer(dataBuffer));

        if (nw_plugBufferGetTerminatorFlag(nw_plugBuffer(dataBuffer))) {
            /* This is a terminator message so look for corresponding msgHolderPtr
             * and insert the databuffer at the end */
            terminatedMsgNr = nw_plugDataBufferGetTerminatedMsgNr(dataBuffer);
            messageHolder = nw_plugDataBufferGetNextMessageHolder(dataBuffer,
                messageHolder, FALSE);
            NW_CONFIDENCE(messageHolder != NULL);
            msgHolderPtr = nw_plugSendingPartitionNodeLookupOrCreateMsgHolderPtr(
                receiveChannel, sendingPartitionNode, terminatedMsgNr);
            /* Found or created a msgHolderPtr, now insert the admin */
            nw_msgHolderPtrAddTailAdmin(msgHolderPtr, admin,
                nw_plugDataBufferGetTerminatingFragmentNr(dataBuffer));
            nrOfMessages--;
            if (nw_msgHolderPtrIsComplete(msgHolderPtr)) {
                nw_plugSendingPartitionNodeDeclareMsgReady(receiveChannel, sendingPartitionNode, msgHolderPtr);
            }
        }

        if (isFragmented) {
            spareMessages = 1;
        } else {
            spareMessages = 0;
        }
        while (nrOfMessages > spareMessages) {
            /* These are complete messages, immediately insert them into the
             * list of waiting complete messages */
            messageHolder = nw_plugDataBufferGetNextMessageHolder(dataBuffer,
                messageHolder, FALSE);
            NW_CONFIDENCE(messageHolder != NULL);
            nw_receiveChannelCreateMsgReady(receiveChannel, admin, messageHolder);
            nrOfMessages--;
        }

        if (isFragmented) {
            NW_CONFIDENCE(nrOfMessages == 1);
            /* This is a fragmented message so look for corresponding msgHolderPtr
             * and insert the databuffer at the start */
            fragmentedMsgNr = nw_plugDataBufferGetFragmentedMsgNr(dataBuffer);
            messageHolder = nw_plugDataBufferGetNextMessageHolder(dataBuffer,
                messageHolder, FALSE);
            NW_CONFIDENCE(messageHolder != NULL);
            msgHolderPtr = nw_plugSendingPartitionNodeLookupOrCreateMsgHolderPtr(
                receiveChannel, sendingPartitionNode, fragmentedMsgNr);
            /* Found or created a msgHolderPtr, now insert the admin */
            fragmentedMsgNr = nw_plugDataBufferGetFragmentNr(dataBuffer);
            if (fragmentedMsgNr == 1) {
                nw_msgHolderPtrAddHeadAdmin(msgHolderPtr, admin, messageHolder);
            } else {
                nw_msgHolderPtrAddMiddleAdmin(msgHolderPtr, admin);
            }
            /* Not very likely, but the message might be complete because fragments
             * have been received out of order */
            if (nw_msgHolderPtrIsComplete(msgHolderPtr)) {
                nw_plugSendingPartitionNodeDeclareMsgReady(receiveChannel, sendingPartitionNode, msgHolderPtr);
            }
        }
    }
    return insertedAdmin;
}

static nw_bool
nw_plugSendingPartitionDropMsgHolderPtr(
    nw_plugReceiveChannel receiveChannel,
    nw_plugSendingPartitionNode sendingPartitionNode)
{
    nw_plugBufferDefragAdmin admin;
    nw_bool found = FALSE;
    nw_bool result = FALSE;
    nw_msgHolderPtr msgHolderPtr = sendingPartitionNode->msgHolderPtrsHead;

    /* Check wether this is the most recent msgHolderptr, we always keep that one */
    if ( msgHolderPtr->next != NULL) {

        /* This is the oldest MsgHolderPtr and it's not the last one, so we're releasing its buffers */
        /* If FirstAdmin is set, we use that to locate the buffers. If not we're locating them */
        /* based on the fragmented message number */
        NW_CONFIDENCE( !nw_msgHolderPtrIsComplete(msgHolderPtr));
        NW_CONFIDENCE( msgHolderPtr->totNrOfFragmentsContained != 0 );
        admin = msgHolderPtr->firstAdmin;

        if ( admin == NULL ){
            /* it can't always be easy */
            admin = sendingPartitionNode->defragAdminsHead.next;
            if(admin)
            {
                while( (!found) && (admin->next != NULL)) {
                    admin = admin->next;
                    found = msgHolderPtr->fragmentedMsgNr == nw_plugDataBufferGetFragmentedMsgNr(DF_DATA(admin));
                }
            }
        }

        if(msgHolderPtr->lastAdmin != NULL) {
            nw_plugBufferDefragAdminRelease(receiveChannel,msgHolderPtr->lastAdmin);
        }

        /* the msgHolderPtr is there for a reason, so there should be a corresponding admin */
        NW_CONFIDENCE(admin);/*let this be, inconsistency detected which should not occur*/
        if (admin)  { /*apparently the inconsistency occurs, avoid core dump.*/
            while ((msgHolderPtr->totNrOfFragmentsContained > 0) &&
                       (admin->next != NULL) ){
                nw_plugBufferDefragAdmin tmp = admin->next;
                /* release from this msgHolderPtr->firstAdmin */
                nw_plugBufferDefragAdminRelease(receiveChannel,admin);
                /* release from the list */
                nw_plugBufferDefragAdminRelease(receiveChannel,admin);
                admin = tmp;
                msgHolderPtr->totNrOfFragmentsContained--;
            }
        }

        /* remove from this msgHolderPtrList */
        sendingPartitionNode->msgHolderPtrsHead = msgHolderPtr->next;
        if ( msgHolderPtr->next != NULL ) {
          msgHolderPtr->next->prev = NULL;
        }

        /* insert into freeMsgHolderPtrs List */
        msgHolderPtr->next = receiveChannel->freeMsgHolderPtrs;
        if (msgHolderPtr->next != NULL) {
            msgHolderPtr->next->prev = msgHolderPtr;
        }
        msgHolderPtr->prev = NULL;
        receiveChannel->freeMsgHolderPtrs = msgHolderPtr;

        result = TRUE;
    }
    return result;
}

/* -------------------------- nw_plugReceiveChannel ------------------------- */

#define nw_plugReceiveChannel(o) ((nw_plugReceiveChannel)(o))


/* -------------------------------- Public ---------------------------------- */

/* Function used during construction */

static void
nw_plugReceiveChannelCreateSendingPartitions(
    nw_plugReceiveChannel receiveChannel,
    nw_plugPartitions partitions)
{
    nw_partitionId partitionId;
    nw_partitionAddress partitionAddress;
    nw_bool found;
    nw_bool connected;

    NW_CONFIDENCE(receiveChannel != NULL);

    for (partitionId = 0;
         partitionId < nw_plugPartitionsGetNofPartitions(partitions);
         partitionId++) {
        nw_plugPartitionsGetPartition(partitions, partitionId,
            &found, &partitionAddress, &connected);
        NW_CONFIDENCE(found);
        if (found) {
            nw_socketAddPartition(nw__plugChannelGetSocket(
                nw_plugChannel(receiveChannel)), partitionId, partitionAddress,
                connected);
        }
    }
}


#define DF_BUFUSE_MONITORING_START (10)

nw_plugChannel
nw_plugReceiveChannelNew(
    nw_seqNr seqNr,
    nw_networkId nodeId,
    nw_plugPartitions partitions,
    nw_userData *userDataPtr,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    nw_plugReceiveChannel result;
    nw_seqNr DefDefragBuffersSize;

    result = (nw_plugReceiveChannel)os_malloc(sizeof(*result));
    if (result != NULL) {
        nw_plugChannelInitialize(nw_plugChannel(result), seqNr, nodeId,
            NW_COMM_RECEIVE, partitions, userDataPtr, pathName,onFatal,onFatalUsrData);
        result->wakeupRequested = FALSE;
        result->freeBuffers = NULL;
        result->sendingPartitionNodes = NULL;
        result->waitingDataBuffersHead = NULL;
        result->waitingDataBuffersTail = NULL;
        result->freeMsgHolderPtrs = NULL;
        result->readyMsgHolderPtrsHead = NULL;
        result->readyMsgHolderPtrsTail = NULL;
        result->lastReturnedBuffer = NULL;
        result->lastReturnedHolder = NULL;
        result->lastReturnedMsgHolderPtr = NULL;
        result->nofFreeBuffers = 0;
        result->nofUsedBuffers = 0;
        result->totNofBuffers = 0;
        result->fragmentsOutOfOrderDropped = 0;
        result->lastReceivedFragment = 0;
        result->inUse = FALSE;

        if (nw_plugChannelGetReliabilityOffered(nw_plugChannel(result)) == NW_REL_RELIABLE) {
            DefDefragBuffersSize = NWCF_DEF_DefragBufferSize;
        } else {
            DefDefragBuffersSize = NWCF_DEF_DefragBufferSizeBestEffort;
        }

        result->maxNofBuffers = NWCF_DEFAULTED_SUBPARAM(ULong, pathName, Rx, DefragBufferSize,DefDefragBuffersSize);
        result->maxReliabBacklog = NWCF_DEFAULTED_SUBPARAM(ULong, pathName, Rx, MaxReliabBacklog,NWCF_DEF_MaxReliabBacklog);
#ifdef NW_TRACING
        result->buffersAllocatedTrace = DF_BUFUSE_MONITORING_START;
        result->buffersUsedTrace = DF_BUFUSE_MONITORING_START;
        result->buffersUsedTraceBottom = result->buffersUsedTrace;
#endif
        nw_plugReceiveChannelCreateSendingPartitions(result, partitions);
    }

    return nw_plugChannel(result);
}

void
nw_plugReceiveChannelFree(
    nw_plugChannel channel)
{
    nw_plugReceiveChannel receiveChannel = nw_plugReceiveChannel(channel);
    nw_plugBufferDefragAdmin admin;
    nw_msgHolderPtr msgHolderPtr;
    nw_plugSendingPartitionNode sendingPartitionNode;
    unsigned int i;

    /* own finalization */
    /* List with free buffers */

    if (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE) {
        NW_TRACE_2(Receive, 3, "Channel %s: %u reliable fragments dropped because they "
            "have been received twice or were too old",
            nw__plugChannelGetName(channel), receiveChannel->fragmentsOutOfOrderDropped);
    }

    i = 0;
    while (receiveChannel->freeBuffers != NULL) {
        admin = receiveChannel->freeBuffers;
        receiveChannel->freeBuffers = admin->next;
        nw_plugBufferDefragAdminFree(admin);
        i++;
    }
    NW_CONFIDENCE(i == receiveChannel->nofFreeBuffers);

    while (receiveChannel->waitingDataBuffersHead != NULL) {
        admin = receiveChannel->waitingDataBuffersHead;
        receiveChannel->waitingDataBuffersHead = admin->next;
        nw_plugBufferDefragAdminFree(admin);
    }
    while (receiveChannel->freeMsgHolderPtrs != NULL) {
        msgHolderPtr = receiveChannel->freeMsgHolderPtrs;
        receiveChannel->freeMsgHolderPtrs = msgHolderPtr->next;
        nw_msgHolderPtrFree(msgHolderPtr);
    }

    while (receiveChannel->readyMsgHolderPtrsHead != NULL) {
        msgHolderPtr = receiveChannel->readyMsgHolderPtrsHead;
        receiveChannel->readyMsgHolderPtrsHead = msgHolderPtr->next;
        nw_msgHolderPtrFree(msgHolderPtr);
    }

    while (receiveChannel->sendingPartitionNodes != NULL) {
        sendingPartitionNode = receiveChannel->sendingPartitionNodes;
        receiveChannel->sendingPartitionNodes = sendingPartitionNode->next;
        nw_plugSendingPartitionNodeFree(sendingPartitionNode);
    }

    /* inherited finalization */
    nw_plugChannelFinalize(channel);

    /* The actual free */
    os_free(channel);
}

static nw_bool
nw_plugReceiveChannelRemoveDefragmentationGarbage(
    nw_plugReceiveChannel receiveChannel)
{
    nw_plugSendingPartitionNode sendingPartitionNode;
    nw_bool result = FALSE;
    static nw_bool reported = FALSE;
    static nw_bool reported_garbage_error = FALSE;

    if (nw_plugChannelGetReliabilityOffered(nw_plugChannel(receiveChannel)) != NW_REL_RELIABLE) {
        sendingPartitionNode = receiveChannel->sendingPartitionNodes;
        while ((sendingPartitionNode != NULL) && (!result)) {
            if ( sendingPartitionNode->msgHolderPtrsHead != NULL)  {
                result = nw_plugSendingPartitionDropMsgHolderPtr(receiveChannel,sendingPartitionNode);
            }
            sendingPartitionNode = sendingPartitionNode->next;
        }
        if (result ) {
            if ( ! reported ) {
                NW_REPORT_WARNING_2("receiving data from the network",
                    "Channel \"%s\" exceeded maximum defragmentation buffers %u, "
                    "messages have been dropped",
                    nw__plugChannelGetName(nw_plugChannel(receiveChannel)),
                    receiveChannel->maxNofBuffers);
                reported = TRUE;
            }
        }else if(!reported_garbage_error){/* Listen very carefully, I shall only say this once allo allo */
            NW_REPORT_WARNING_2("receiving data from the network",
                "Channel \"%s\" exceeded maximum defragmentation buffers %u, "
                "But unable to garbage collect buffers",
                nw__plugChannelGetName(nw_plugChannel(receiveChannel)),
                receiveChannel->maxNofBuffers);
            reported_garbage_error = TRUE;
        }
    }
    return result;
}

static void
nw_plugReceiveChannelDropDefragmentationBuffers(
    nw_plugReceiveChannel receiveChannel)
{
    nw_plugSendingPartitionNode sendingPartitionNode = receiveChannel->sendingPartitionNodes;
    nw_plugBufferDefragAdmin admin;
    nw_plugBufferDefragAdmin insertedAdmin;
    nw_seqNr packetNrPrevious;
    nw_seqNr packetNrCurrent;
    nw_bool holeFound;


    if (nw_plugChannelGetReliabilityOffered(nw_plugChannel(receiveChannel)) == NW_REL_RELIABLE) {

        NW_REPORT_ERROR_2("receiving data from the network",
            "Channel \"%s\" exceeded maximum defragmentation buffers %u, "
            "messages have been dropped",
            nw__plugChannelGetName(nw_plugChannel(receiveChannel)),
            receiveChannel->maxNofBuffers);

        while (sendingPartitionNode != NULL)  {
            /* First check if there are any buffers in the out_of_order list for this PartitionNode*/
            admin = sendingPartitionNode->outOfOrderAdminsHead.next;
            if ( admin->next != NULL ) {
                /* Buffers found, Now find the first missing packetnumber */
                packetNrPrevious = nw_plugDataBufferGetPacketNr(DF_DATA(admin));
                admin = admin->next;
                holeFound = FALSE;
                while ((admin->next != NULL) && !holeFound) {
                    packetNrCurrent = nw_plugDataBufferGetPacketNr(DF_DATA(admin));
                    NW_CONFIDENCE(packetNrCurrent > packetNrPrevious);
                    if (packetNrCurrent != packetNrPrevious + 1) {
                        holeFound = TRUE;
                    } else {
                        admin = admin->next;
                        packetNrPrevious = packetNrCurrent;
                    }
                }
                sendingPartitionNode->packetNrWaitingFor = packetNrPrevious+1;
                /* by resetting packetNrWaitingFor, we've basicly dropped the fragment(s) we missed,
                    as we will not be waiting for it anymore, and will not accept it */


                /* now process the fragments that have come available */
                admin = nw_plugSendingPartitionNodeOutOfOrderListTake(sendingPartitionNode,
                    TRUE);
                while (admin != NULL) {
                    insertedAdmin = nw_plugSendingPartitionNodeInsertAdmin(receiveChannel,
                        sendingPartitionNode, admin, TRUE);
                    NW_CONFIDENCE(insertedAdmin == admin);
                    /* Release this reference, buffer is refcounted by the list */
                    nw_plugBufferDefragAdminRelease(receiveChannel, admin);
                    admin = nw_plugSendingPartitionNodeOutOfOrderListTake(sendingPartitionNode, TRUE);
                }
            }
            sendingPartitionNode = sendingPartitionNode->next;
        }
    }
}


static nw_plugBuffer
nw_plugReceiveChannelCreateBuffer(
    nw_plugReceiveChannel channel)
{
    nw_plugBufferDefragAdmin admin;
    nw_plugBuffer result = NULL;


    admin = nw_plugBufferDefragAdminCreate(channel);
    if( !admin){
        /* Try to clean up defragmentation garbage */
        if (nw_plugReceiveChannelRemoveDefragmentationGarbage(channel)) {
            /* only if we successfully cleaned up some buffers, continue
             * otherwise, drop the buffers
             */
            if(channel->freeBuffers != NULL)
            {
                admin = nw_plugBufferDefragAdminCreate(channel);
            }
        } else {
            /* No buffers available; this will cause dropping of buffers even if we are reliable */
        }

    }

    if ( admin ) {
        result = DF_BUFFER(admin);
    }

    return result;
}

#define nw_plugReceiveChannelReleaseBuffer(channel, buffer) \
    nw_plugBufferDefragAdminRelease(channel, DF_ADMIN(buffer))


static void
nw_plugReceiveChannelTakeFirstMessageHolder(
    nw_plugReceiveChannel receiveChannel,
    nw_messageHolder *messageHolder,
    nw_address *senderAddress)
{
    nw_msgHolderPtr msgHolderPtr;
    nw_plugBufferDefragAdmin lastReturnedBuffer;
    nw_messageHolder lastReturnedHolder;
    nw_msgHolderPtr lastReturnedMsgHolderPtr;
    nw_bool wasLastAdmin;

    NW_CONFIDENCE(receiveChannel != NULL);

    if (receiveChannel->lastReturnedBuffer != NULL) {
        lastReturnedBuffer = receiveChannel->lastReturnedBuffer;
        lastReturnedHolder = receiveChannel->lastReturnedHolder;
        lastReturnedMsgHolderPtr = receiveChannel->lastReturnedMsgHolderPtr;
        NW_CONFIDENCE(lastReturnedHolder != NULL);
        NW_CONFIDENCE(lastReturnedMsgHolderPtr != NULL);
        NW_CONFIDENCE(lastReturnedBuffer == lastReturnedMsgHolderPtr->firstAdmin);
        NW_CONFIDENCE(lastReturnedHolder == lastReturnedMsgHolderPtr->messageHolder);
        NW_CONFIDENCE(lastReturnedMsgHolderPtr == receiveChannel->readyMsgHolderPtrsHead);

        /* Was this the last admin of this msgHolderPtr? */
        wasLastAdmin = (lastReturnedMsgHolderPtr->firstAdmin ==
                        lastReturnedMsgHolderPtr->lastAdmin);
        if (wasLastAdmin) {
            /* Yes, it was, so get the next message */
            /* Also make sure to release the buffer because of the lastAdmin ptr */
            nw_plugBufferDefragAdminRelease(receiveChannel, lastReturnedBuffer);
            nw_msgHolderPtrRelease(receiveChannel, lastReturnedMsgHolderPtr);
        } else {
            /* No, it was not, so move to the next fragment */
            lastReturnedMsgHolderPtr->firstAdmin =
                lastReturnedMsgHolderPtr->firstAdmin->next;
            lastReturnedMsgHolderPtr->messageHolder = nw_plugDataBufferGetNextMessageHolder(
                DF_DATA(lastReturnedMsgHolderPtr->firstAdmin), NULL, FALSE);
        }

        /* Free the last returned buffer because of the firstAdmin ptr */
        nw_plugBufferDefragAdminRelease(receiveChannel, lastReturnedBuffer);

        /* need to determine if the admin is no longer used by any message
         * holder pointer, which can be detected by checking the usedCount of
         * the admin. If this usedCount is 1, then only the list holds the
         * reference.
         * If this is the case, then we must remove it from the
         * list. We accomplish this by calling the defrag admin release
         * function which removes it from the list and places it in the
         * freebufferslist by lowering the usercount to 0.
         */
        if (lastReturnedBuffer->usedCount == 1) {
            /*this admin buffer is no longer used, remove it from the list*/
            nw_plugBufferDefragAdminRelease(receiveChannel, lastReturnedBuffer);
        }
        receiveChannel->lastReturnedBuffer = NULL;
        receiveChannel->lastReturnedHolder = NULL;
        receiveChannel->lastReturnedMsgHolderPtr = NULL;
    }

    msgHolderPtr = receiveChannel->readyMsgHolderPtrsHead;
    if (msgHolderPtr != NULL) {
        /* Fill the result */
        *messageHolder = msgHolderPtr->messageHolder;
        *senderAddress = msgHolderPtr->firstAdmin->senderAddress;
        receiveChannel->lastReturnedBuffer = msgHolderPtr->firstAdmin;
        receiveChannel->lastReturnedHolder = msgHolderPtr->messageHolder;
        receiveChannel->lastReturnedMsgHolderPtr = msgHolderPtr;
    } else {
        *messageHolder = NULL;
        *senderAddress = 0;
    }
}

static void
nw_plugReceiveChannelIgnoreLastReturnedMessage(
    nw_plugReceiveChannel receiveChannel)
{
    nw_plugBufferDefragAdmin lastReturnedBuffer;
    nw_messageHolder lastReturnedHolder;
    nw_msgHolderPtr lastReturnedMsgHolderPtr;
    nw_bool wasLastAdmin = FALSE;

    NW_CONFIDENCE(receiveChannel != NULL);
    NW_CONFIDENCE(receiveChannel->lastReturnedBuffer != NULL);

    lastReturnedMsgHolderPtr = receiveChannel->lastReturnedMsgHolderPtr;
    NW_CONFIDENCE(lastReturnedMsgHolderPtr != NULL);

    do {
        lastReturnedBuffer = lastReturnedMsgHolderPtr->firstAdmin;
        lastReturnedHolder = lastReturnedMsgHolderPtr->messageHolder;
        NW_CONFIDENCE(lastReturnedMsgHolderPtr == receiveChannel->readyMsgHolderPtrsHead);
        /* Was this the last admin of this msgHolderPtr? */
        wasLastAdmin = (lastReturnedMsgHolderPtr->firstAdmin ==
                        lastReturnedMsgHolderPtr->lastAdmin);
        if (wasLastAdmin) {
            /* Yes, it was, so get the next message */
            /* Also make sure to release the buffer because of the lastAdmin ptr */
            nw_plugBufferDefragAdminRelease(receiveChannel, lastReturnedBuffer);
            nw_msgHolderPtrRelease(receiveChannel, lastReturnedMsgHolderPtr);
        } else {
            /* No, it was not, so move to the next fragment */
            lastReturnedMsgHolderPtr->firstAdmin =
                lastReturnedMsgHolderPtr->firstAdmin->next;
            lastReturnedMsgHolderPtr->messageHolder = nw_plugDataBufferGetNextMessageHolder(
                DF_DATA(lastReturnedMsgHolderPtr->firstAdmin), NULL, FALSE);
        }

        /* Free the last returned buffer because of the firstAdmin ptr */
        nw_plugBufferDefragAdminRelease(receiveChannel, lastReturnedBuffer);

        /* need to determine if the admin is no longer used by any message
         * holder pointer, which can be detected by checking the usedCount of
         * the admin. If this usedCount is 1, then only the list holds the
         * reference.
         * If this is the case, then we must remove it from the
         * list. We accomplish this by calling the defrag admin release
         * function which removes it from the list and places it in the
         * freebufferslist by lowering the usercount to 0.
         */
        if (lastReturnedBuffer->usedCount == 1) {
            /*this admin buffer is no longer used, remove it from the list*/
            nw_plugBufferDefragAdminRelease(receiveChannel, lastReturnedBuffer);
        }
    } while (!wasLastAdmin);

    receiveChannel->lastReturnedBuffer = NULL;
    receiveChannel->lastReturnedHolder = NULL;
    receiveChannel->lastReturnedMsgHolderPtr = NULL;
}


static nw_bool
nw_plugReceiveChannelGetFragmentInstantly(
    nw_plugReceiveChannel channel,
    nw_data *data,
    nw_length *length,
    nw_address *senderAddress)
{
    nw_bool result = FALSE;
    nw_messageHolder messageHolder = NULL;

    nw_plugReceiveChannelTakeFirstMessageHolder(channel, &messageHolder, senderAddress);
    if (messageHolder != NULL) {
        *data = NW_MESSAGEHOLDER_DATA(messageHolder);
        *length = nw_messageHolderGetLength(messageHolder);
        result = TRUE;
    } else {
        *data = NULL;
        *length = 0;
        result =FALSE;
    }

    return result;
}


static void
nw_plugReceiveChannelCommunicateControlReceived(
    nw_plugReceiveChannel channel,
    nw_seqNr sendingNodeId,
    nw_address sendingAddress,
    nw_plugControlBuffer buffer)
{
    nw_plugInterChannel interChannel;
    nw_plugControlMessage message;
    nw_bool more;

    NW_CONFIDENCE(nw__plugChannelGetReliabilityOffered(nw_plugChannel(channel)) == NW_REL_RELIABLE);

    interChannel = nw__plugChannelGetInterChannelComm(nw_plugChannel(channel));
    NW_CONFIDENCE(interChannel != NULL);


    NW_TRACE_2(Receive, 4, "plugChannel %s: Recvthread: recvACK : %d",
        nw__plugChannelGetName(nw_plugChannel(channel)),nw_plugControlBufferGetRecvBufferInUse(buffer));

    message = NULL;
    do {
        message = nw_plugControlBufferGetNextMessage(buffer, message, &more);
        NW_CONFIDENCE(message != NULL);
        nw_plugInterChannelPostAckReceivedMessage(interChannel,
            sendingNodeId,
            nw_plugControlMessageGetPartitionId(message),
            sendingAddress,
            nw_plugControlMessageGetStartingNr(message),
            nw_plugControlMessageGetClosingNr(message),
            nw_plugControlBufferGetRecvBufferInUse(buffer));
    } while (more);
}

static void
nw_plugReceiveChannelCommunicateDataReceivedReliably(
    nw_plugReceiveChannel channel,
    nw_address sendingAddress,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    nw_plugDataBuffer buffer)
{
    nw_plugInterChannel interChannel;

    NW_CONFIDENCE(nw__plugChannelGetReliabilityOffered(nw_plugChannel(channel)) == NW_REL_RELIABLE);

    interChannel = nw__plugChannelGetInterChannelComm(nw_plugChannel(channel));
    NW_CONFIDENCE(interChannel != NULL);

    nw_plugInterChannelPostDataReceivedMessage(interChannel,
        sendingNodeId, sendingPartitionId, sendingAddress,
        nw_plugDataBufferGetPacketNr(buffer),channel->nofUsedBuffers);

    NW_TRACE_2(Receive, 4, "plugChannel %s: Recvthread: sendACK : %d",
        nw__plugChannelGetName(nw_plugChannel(channel)),channel->nofUsedBuffers);
}

static void
nw_plugReceiveChannelInsertDataReceived(
    nw_plugReceiveChannel receiveChannel,
    nw_seqNr sendingNodeId,
    nw_partitionId sendingPartitionId,
    nw_plugDataBuffer buffer,
    nw_bool orderPreservation)
{
    nw_plugSendingPartitionNode currentNode;
    nw_plugBufferDefragAdmin admin;
    nw_plugBufferDefragAdmin insertedAdmin;
    nw_bool found;

    /* First lookup or create the node that has sent this message */
    NW_CONFIDENCE(sendingNodeId == nw_plugBufferGetSendingNodeId(nw_plugBuffer(buffer)));
    currentNode = receiveChannel->sendingPartitionNodes;
    found = FALSE;
    while ((currentNode != NULL) && !found) {
        found = (
            (currentNode->nodeId == sendingNodeId) &&
            (currentNode->partitionId == sendingPartitionId));
        if (!found) {
            currentNode = currentNode->next;
        }
    }
    if (!found) {
        currentNode = nw_plugSendingPartitionNodeNew(sendingNodeId, sendingPartitionId,
            &receiveChannel->sendingPartitionNodes);
    }
    NW_CONFIDENCE(currentNode != NULL);

    
    if ( nw_plugSendingPartitionNodeIsActive(currentNode) ) {
        /* Insert the admin into the defrag of out of order list */
        admin = DF_ADMIN(buffer);
        insertedAdmin = nw_plugSendingPartitionNodeInsertAdmin(receiveChannel, currentNode,
            admin, FALSE);
        /* Release this reference, buffer is refcounted by the list, if inserted */
        nw_plugBufferDefragAdminRelease(receiveChannel, admin);
        if (insertedAdmin == admin) {
            /* Check if any buffers have become not out of order */
            admin = nw_plugSendingPartitionNodeOutOfOrderListTake(currentNode,
                orderPreservation);
            while (admin != NULL) {
                insertedAdmin = nw_plugSendingPartitionNodeInsertAdmin(receiveChannel,
                    currentNode, admin, TRUE);
                NW_CONFIDENCE(insertedAdmin == admin);
                /* Release this reference, buffer is refcounted by the list */
                nw_plugBufferDefragAdminRelease(receiveChannel, admin);
                admin = nw_plugSendingPartitionNodeOutOfOrderListTake(currentNode,
                        orderPreservation);
            }
        }
    } else {
        NW_TRACE_1(Receive, 5, "Channel %s: packet received from inactive node,"
            " that has been removed from the protocol for this channel",
            nw__plugChannelGetName(nw_plugChannel(receiveChannel)));
    }
}

static nw_bool
nw_plugReceiveChannelReadSocket(
    nw_plugReceiveChannel channel,
    os_time *timeOut,
    nw_bool *dataRead)
{
    sk_length readLength;
    nw_plugBufferDefragAdmin admin;
    nw_plugBuffer buffer;
    nw_bool result = TRUE;
    nw_seqNr sendingNodeId;
    nw_bool reliable;
    sk_length dataLength;
    nw_partitionId sendingPartitionId;
    nw_bool found;
    char *addressString;
    nw_bool connected;

    *dataRead = TRUE;
    NW_CONFIDENCE(sizeof(sk_address) == sizeof(admin->senderAddress));
    buffer = nw_plugReceiveChannelCreateBuffer(channel);
    if (buffer) {
        admin = DF_ADMIN(buffer);
        readLength = nw_socketReceive(
            nw__plugChannelGetSocket(nw_plugChannel(channel)),
            (sk_address *)&admin->senderAddress, buffer,
            nw__plugChannelGetFragmentLength(nw_plugChannel(channel)), timeOut);
        if (readLength > 0) {
            if (nw_plugBufferCheckVersion(buffer, NW_CURRENT_PROTOCOL_VERSION)){
                sendingNodeId = nw_plugBufferGetSendingNodeId(buffer);
                if (nw_plugBufferGetControlFlag(buffer)) {
                    /* communicate incoming control message to sending thread */
                    nw_plugReceiveChannelCommunicateControlReceived(
                        nw_plugReceiveChannel(channel), sendingNodeId,
                        admin->senderAddress, nw_plugControlBuffer(buffer));
                    nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                } else {
                    dataLength = nw_plugBufferGetLength(buffer);
                    if (dataLength > 0) {
                        NW_STAMP(nw_plugDataBuffer(buffer),NW_BUF_TIMESTAMP_RECEIVE);
                        sendingNodeId = nw_plugBufferGetSendingNodeId(buffer);
                        sendingPartitionId = nw_plugDataBufferGetPartitionId(nw_plugDataBuffer(buffer));
                        connected = FALSE;
                        nw_plugChannelGetPartition(nw_plugChannel(channel), sendingPartitionId,&found, &addressString, &connected);
                        if (connected) {
                            /* communicate incoming data message to sending thread
                             * for reliable channels (for acking) */
                            reliable = (nw__plugChannelGetReliabilityOffered(nw_plugChannel(channel)) == NW_REL_RELIABLE);
                            NW_CONFIDENCE(nw_plugBufferGetReliabilityFlag(buffer) == reliable);
                            if (reliable) {
                                nw_plugReceiveChannelCommunicateDataReceivedReliably(
                                    channel, admin->senderAddress,
                                    sendingNodeId, sendingPartitionId,
                                    nw_plugDataBuffer(buffer));
                            }
                            /* Append at tail of doubly linked list */
                            admin->next = NULL;
                            if (channel->waitingDataBuffersHead == NULL) {
                                channel->waitingDataBuffersHead = admin;
                            } else {
                                NW_CONFIDENCE(channel->waitingDataBuffersTail != NULL);
                                (channel->waitingDataBuffersTail)->next = admin;
                            }
                            admin->prev = channel->waitingDataBuffersTail;
                            channel->waitingDataBuffersTail = admin;
                        } else {
                            /* This is a strange situation: data received for
                             * a disconnected partition. Either the switch
                             * does not implement multicast filtering or the
                             * application does not fit with the configuration */
                            NW_REPORT_WARNING_1("receive data",
                                 "received data for disconnected network partition (id=%d)",
                                 sendingPartitionId);
                            nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                        }
                    } else {
                        char *str;
                        /* DataLength <= 0, so invalid data */
                        NW_REPORT_WARNING("receive data",
                             "received data with corrupted content");
                        str = nw_plugDataBufferToString(nw_plugDataBuffer(buffer));
                        NW_REPORT_WARNING_1("receive data",
                             "corrupted content: %s", str);
                        os_free(str);
                        nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                    }
                }
            } else {
                /* Received data was not a valid ospl packet */
                NW_REPORT_WARNING("receive data",
                     "received data with an incorrect protocol-version id");
                nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                *dataRead = FALSE;
            }
        } else {
            /* ReadLength <= 0, so no data */
            nw_plugReceiveChannelReleaseBuffer(channel, buffer);
            *dataRead = FALSE;
        }
    } else {
        result = FALSE;
        *dataRead = FALSE;
    }
    return result;
}



static nw_plugBufferDefragAdmin
nw_plugReceiveChannelTakeWaitingDataBuffer(
    nw_plugReceiveChannel channel)
{
    nw_plugBufferDefragAdmin admin = NULL;

    if (channel->waitingDataBuffersHead) {
        admin = channel->waitingDataBuffersHead;
        /* set new head to the next of the current head */
        channel->waitingDataBuffersHead = channel->waitingDataBuffersHead->next;
        /* If we have a new head, then update it's prev pointer to NULL,
         * else if we didnt have a new head, then we removed the last admin
         * and thus have to update the tail as well.
         */
        if (channel->waitingDataBuffersHead != NULL) {
            channel->waitingDataBuffersHead->prev = NULL;
        } else {
            channel->waitingDataBuffersTail = NULL;
        }
    }
    return admin;
}

/*
 * Read data from the socket as long as there is something to read
 * and communicate incoming data and control messages to the send-thread
 */
void
nw_plugReceiveChannelProcessIncoming(nw_plugChannel channel)
{
    static os_time zeroTime = {0,0};
    nw_bool dataRead = TRUE;
    nw_bool result = TRUE;

    while (result && dataRead ) {
        result = nw_plugReceiveChannelReadSocket(nw_plugReceiveChannel(channel), &zeroTime, &dataRead);
    }
}

c_bool
nw_plugReceiveChannelMessageStart(
    nw_plugChannel channel,
    nw_data *data,
    nw_length *length,
    nw_address *senderAddress)
{
    nw_plugReceiveChannel  receiveChannel = nw_plugReceiveChannel(channel);
    nw_bool messageWaiting;
    nw_plugBuffer buffer;
    nw_plugBufferDefragAdmin admin;
    nw_seqNr sendingNodeId;
    nw_partitionId sendingPartitionId;
    nw_bool reliable;
    nw_bool read_result = TRUE;
    nw_bool dummy;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);
    NW_CONFIDENCE(data != NULL);
    NW_CONFIDENCE(length != NULL);
    NW_CONFIDENCE(senderAddress != NULL);
    NW_CONFIDENCE(sizeof(nw_address) == sizeof(sk_address));
    NW_CONFIDENCE(!receiveChannel->inUse);

    *data = NULL;
    *length = 0;
    *senderAddress = 0;
    if (!receiveChannel->wakeupRequested) {
            /* Check if any message is waiting to be read.
             * This can be, for example, a message in a previously received
             * buffer. */
        messageWaiting = nw_plugReceiveChannelGetFragmentInstantly(
            receiveChannel, data, length, senderAddress);
        do {
            /* First read from the socket in order to avoid rxbuffer overflow */
            nw_plugReceiveChannelProcessIncoming(channel);
            if (!messageWaiting) {
                /* First handle any waiting buffers */
                admin = nw_plugReceiveChannelTakeWaitingDataBuffer(receiveChannel);
                if (admin != NULL) {
                    buffer = DF_BUFFER(admin);
                    NW_STAMP(nw_plugDataBuffer(buffer),NW_BUF_TIMESTAMP_HANDLE);
                    sendingNodeId = nw_plugBufferGetSendingNodeId(buffer);
                    sendingPartitionId = nw_plugDataBufferGetPartitionId(nw_plugDataBuffer(buffer));
                    reliable = (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE);

                    nw_plugReceiveChannelInsertDataReceived(
                        receiveChannel, sendingNodeId, sendingPartitionId,
                        nw_plugDataBuffer(buffer), reliable);
                    messageWaiting = nw_plugReceiveChannelGetFragmentInstantly(
                        receiveChannel, data, length, senderAddress);

                    /* Do not release here, but keep the refcount to 1 */
                    /* nw_plugReceiveChannelReleaseBuffer(receiveChannel, buffer); */
                } else {
                    os_time timeout = {0, 200000000}; /* 200 milli seconds */
                    /* No data waiting to be processed, so wait for data in socket */
                    read_result = nw_plugReceiveChannelReadSocket(receiveChannel, &timeout, &dummy);
                    if (!read_result) {
                        /* Data could not be read from socket, because we're out of defrag buffers
                          * which could not be garbage collected. This is a Fatal error.
                          * Inform the higher powers
                          */
                        if ((channel->onFatal != NULL) &&
                            (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE))
                        {
                            NW_REPORT_ERROR_2("receiving data from the network",
                                "Channel \"%s\" reached maximum defragmentation buffers %u, "
                                "terminating Networking service",
                                nw__plugChannelGetName(channel),
                                receiveChannel->maxNofBuffers);
                            channel->onFatal(channel->onFatalUsrData);
                            channel->onFatal = NULL;
                        }
                        receiveChannel->wakeupRequested = TRUE;
                    }
                }
            }
            NW_CONFIDENCE(messageWaiting == ((*data != NULL) && (*length != 0) && (*senderAddress != 0)));
        } while (!messageWaiting && !receiveChannel->wakeupRequested);
    }

    /* Always reset wakeup-flag */
    receiveChannel->wakeupRequested = FALSE;
    receiveChannel->inUse = (*data != NULL);
    return receiveChannel->inUse;
}

void
nw_plugReceiveChannelGetNextFragment(
    nw_plugChannel channel,
    nw_data *data,
    nw_length *length)
{
    nw_plugReceiveChannel receiveChannel = nw_plugReceiveChannel(channel);
    nw_address senderAddress;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);
    NW_CONFIDENCE(data != NULL);
    NW_CONFIDENCE(length != NULL);
    NW_CONFIDENCE(receiveChannel->inUse);
    /* We only arrive here if a new fragment is indeed available. */
    nw_plugReceiveChannelGetFragmentInstantly(receiveChannel, data, length,
        &senderAddress);
    NW_CONFIDENCE((*data != NULL) && (*length != 0));
}

void
nw_plugReceiveChannelMessageIgnore(
    nw_plugChannel channel)
{
    nw_plugReceiveChannel receiveChannel = (nw_plugReceiveChannel)channel;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);
    NW_CONFIDENCE(receiveChannel->inUse);

    /* Release all buffers related to the last returned message */
    nw_plugReceiveChannelIgnoreLastReturnedMessage(receiveChannel);

    receiveChannel->inUse = FALSE;
}


void
nw_plugReceiveChannelMessageEnd(
    nw_plugChannel channel)
{
    nw_plugReceiveChannel receiveChannel = (nw_plugReceiveChannel)channel;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);
/*    NW_CONFIDENCE(receiveChannel->inUse);*/

    receiveChannel->inUse = FALSE;
}

void
nw_plugReceiveChannelWakeUp(
    nw_plugChannel channel)
{
    nw_plugReceiveChannel receiveChannel = nw_plugReceiveChannel(channel);

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);

    receiveChannel->wakeupRequested = TRUE;
}


#ifdef NW_TIMESTAMP
os_time *
nw_plugReceiveChannelLastHolderTimestamps (
    nw_plugChannel channel)
{
    if (channel == NULL) return NULL;
return NULL;
/*    return nw_plugReceiveChannel(channel)->lastReturnedHolder->timestamp;*/
}

os_time *
nw_plugReceiveChannelLastBufferTimestamps (
    nw_plugChannel channel)
{
    if (channel == NULL) return NULL;
    return nw_plugDataBuffer(DF_BUFFER(nw_plugReceiveChannel(channel)->lastReturnedBuffer))->timestamp;
}
#endif


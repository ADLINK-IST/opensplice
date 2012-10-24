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
#include "nw__plugReceiveChannel.h"
#include <time.h>
#include <math.h>

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
#include "nw_security.h"
#include "ut_collection.h"

/*
 * This define flag can be used to diagnose leakage of defragbuffers/admins
 * When it is set, all allocated buffers are kept linked in the adminPool field of the receiveChannel.
 * Also each admin wil contain a linked-list "trail" of all the changes of the usedcount.
 */
#undef NW_BUFFER_TRAILS

/* --------------------------- nw_plugReceiveChannel ----------------------- */

NW_CLASS(nw_plugBufferDefragAdmin);
NW_CLASS(nw_plugBufferBackupAdmin);
NW_CLASS(nw_msgHolderPtr);
NW_CLASS(nw_plugSendingPartitionNode);

/**
* @extends nw_plugChannel_s
*/
NW_STRUCT(nw_plugReceiveChannel) {
    NW_EXTENDS(nw_plugChannel);
    /* Reading and defragmentation stuff */
    /** Boolean indicating if anybody wants to interrupt */
    nw_bool wakeupRequested;
    os_mutex wakeupMtx;
    /** Linked list of known sending nodes */
    nw_plugSendingPartitionNode sendingPartitionNodes;
    /** Linked list of free buffers not in use */
    nw_plugBufferDefragAdmin freeBuffers;
    /** Linked list of data buffers waiting to be processed */
    nw_plugBufferDefragAdmin waitingDataBuffersHead;
    nw_plugBufferDefragAdmin waitingDataBuffersTail;
    /** Linked list of pointer objects to message starting points */
    nw_msgHolderPtr freeMsgHolderPtrs;
    nw_msgHolderPtr readyMsgHolderPtrsHead; /* For taking */
    nw_msgHolderPtr readyMsgHolderPtrsTail; /* For appending */
    nw_plugBufferDefragAdmin lastReturnedBuffer;
    nw_messageHolder lastReturnedHolder;
    nw_msgHolderPtr lastReturnedMsgHolderPtr;
    /** Currently for confidence only */
    nw_seqNr nofFreeBuffers;
    nw_seqNr nofUsedBuffers;
    nw_seqNr nofBackupBuffers;
    nw_seqNr totNofBuffers;
    nw_seqNr maxNofBuffers;
    nw_seqNr maxReliabBacklog;
    nw_seqNr fragmentsOutOfOrderDropped;
    nw_seqNr lastReceivedFragment;
    nw_seqNr nofBuffersWaiting;
    nw_seqNr nofBuffersOutOfOrder;
#ifdef NW_TRACING
    nw_seqNr buffersAllocatedTrace;
    nw_seqNr buffersUsedTrace;
    nw_seqNr buffersUsedTraceBottom;
#endif
    /** Boolean for confidence checking */
    nw_bool inUse;
    plugReceiveStatistics prs;
    nw_bool crc_check;
    ut_collection partionIdToHashAdmin;
    nw_bool fragmentSizeErrorLog;
    nw_plugBufferBackupAdmin backupBufferTail;
    nw_plugBufferBackupAdmin backupBufferHead;
    os_time packetRetentionPeriod;
    os_time ReliabilityRecoveryPeriod;
    nw_bool backupActive;
    nw_seqNr periodicTriggerCount;
    /*nw_seqNr backupBufferCounter;*/

    NW_SECURITY_DECODER_HOOK_DECL
#ifdef NW_BUFFER_TRAILS
    nw_plugBufferDefragAdmin adminPool;
#endif
};

#ifdef NW_BUFFER_TRAILS
NW_CLASS(nw_plugBufferTrail);
NW_STRUCT(nw_plugBufferTrail) {
    nw_plugBufferTrail prev;
    nw_seqNr line;
    nw_seqNr usedCount;
};
#endif

/* ------------------------ nw_plugBufferDefragAdmin -------------------- */

NW_STRUCT(nw_plugBufferDefragAdmin) {
    nw_plugBufferDefragAdmin prev;
    nw_plugBufferDefragAdmin next;
    NW_STRUCT(nw_senderInfo) sender;
    nw_seqNr usedCount;
#ifdef NW_BUFFER_TRAILS
    nw_plugBufferDefragAdmin link;
    nw_plugBufferTrail trail;
#endif
};

/* */
NW_STRUCT(nw_plugBufferBackupAdmin) {
    nw_plugBufferBackupAdmin prev;
    nw_plugBufferBackupAdmin next;
    nw_plugBufferDefragAdmin admin;
    os_time time;
 #ifdef NW_BUFFER_TRAILS
    nw_plugBufferDefragAdmin link;
    nw_plugBufferTrail trail;
#endif
};

#define UI(val)       ((os_address)(val))
#define UI_DEREF(ptr) (*(os_address *)(ptr))

#define DF_ADMIN_LENGTH (UI(sizeof(NW_STRUCT(nw_plugBufferDefragAdmin))))
#define DF_ADMIN(ptr)   (((ptr) != NULL)?(nw_plugBufferDefragAdmin)(UI(ptr) - DF_ADMIN_LENGTH): NULL)
#define DF_BUFFER(ptr)  ((((ptr) != NULL)?nw_plugBuffer(UI(ptr) + DF_ADMIN_LENGTH):NULL))
#define DF_DATA(ptr)    (nw_plugDataBuffer(DF_BUFFER(ptr)))
#define DF_CONTROL(ptr) (nw_plugControlBuffer(DF_BUFFER(ptr)))


#if NW_BUFFER_TRAILS
static void
nw_plugBufferDefragAdminAddTrail(nw_plugBufferDefragAdmin admin, nw_seqNr line)
{
    nw_plugBufferTrail prev;
    prev = admin->trail;
    admin->trail = (nw_plugBufferTrail)os_malloc(UI(sizeof(NW_STRUCT(nw_plugBufferTrail))));
    admin->trail->usedCount = admin->usedCount;
    admin->trail->line = line;
    admin->trail->prev = prev;
}

static void
nw_plugBufferDefragAdminEmptyTrails(nw_plugBufferDefragAdmin admin)
{
    nw_plugBufferTrail prev, current;
    current = admin->trail;
    while ( current != NULL ) {
        prev = current->prev;
        os_free(current);
        current = prev;
    }
    admin->trail = NULL;
}
#endif

#if NW_BUFFER_TRAILS
#define USEDCOUNT_TRAIL(admin) \
    nw_plugBufferDefragAdminAddTrail(admin,__LINE__)
#else
#define USEDCOUNT_TRAIL(admin)
#endif

#if 0
#define PRINT_USEDCOUNT(admin)       \
    printf("%s:%d (0x%x) usedCount = %d\n", __FILE__, __LINE__, (os_address)admin, admin->usedCount)
#else
#define PRINT_USEDCOUNT(admin)
#endif

#define SET_USEDCOUNT(admin, value) do { admin->usedCount = value;        USEDCOUNT_TRAIL(admin); PRINT_USEDCOUNT(admin); } while (0)
#define INC_USEDCOUNT(admin)        do { pa_increment(&admin->usedCount); USEDCOUNT_TRAIL(admin); PRINT_USEDCOUNT(admin); } while (0)
#define DEC_USEDCOUNT(admin)        do { pa_decrement(&admin->usedCount); USEDCOUNT_TRAIL(admin); PRINT_USEDCOUNT(admin); } while (0)
#define TRANSFER_USEDCOUNT(admin)   do {                                  USEDCOUNT_TRAIL(admin); PRINT_USEDCOUNT(admin); } while (0)

static os_equality
compareLeafs (
    void *o1,
    void *o2,
    void *args
    )
{
    if (o1 < o2) {
        return C_LT;
    } else if (o1 > o2) {
        return C_GT;
    }
    return C_EQ;
}


/* ---------------------------- nw_plugSendingPartitionNode -------------------------- */

NW_STRUCT(nw_plugSendingPartitionNode) {
    nw_seqNr nodeId;
    nw_partitionId partitionId;
    nw_bool active;
    nw_bool announcing;
    nw_seqNr packetNrWaitingFor;
    nw_seqNr outOfOrderCount;
    NW_STRUCT(nw_plugBufferDefragAdmin) outOfOrderAdminsHead;
    NW_STRUCT(nw_plugBufferDefragAdmin) outOfOrderAdminsTail;
    NW_STRUCT(nw_plugBufferDefragAdmin) defragAdminsHead;
    NW_STRUCT(nw_plugBufferDefragAdmin) defragAdminsTail;
    nw_msgHolderPtr msgHolderPtrsHead;
    nw_msgHolderPtr msgHolderPtrsTail;
    nw_plugSendingPartitionNode next;
    os_time timeOfDead;
    nw_seqNr firstNr;
    nw_seqNr lastNr;
};


static nw_bool
nw_plugBufferDefragBackupAdminRelease(
        nw_plugReceiveChannel receiveChannel)
{
    nw_bool result = FALSE;
    os_time timetemp;
    nw_plugBufferBackupAdmin ba;

    if (receiveChannel->backupBufferTail != NULL) {
        ba = receiveChannel->backupBufferTail;
        timetemp = os_hrtimeGet();
        /* set time for all buffers */
        for (ba = receiveChannel->backupBufferTail;ba != NULL && ba->time.tv_sec == 0 && ba->time.tv_nsec == 0;ba = ba->next) {
            ba->time = timetemp;
        }
        timetemp = os_timeSub(timetemp,receiveChannel->backupBufferTail->time);
        if (os_timeCompare(receiveChannel->packetRetentionPeriod, timetemp) == OS_LESS) {
            /*buffer can be freed*/
            result = TRUE;
            /*NW_REPORT_WARNING_1("backupbuffer","Backupbuffer can be freed counter: %d", receiveChannel->backupBufferCounter);*/

        } else {
            /*buffer needs to be kept*/
            /*NW_REPORT_WARNING_1("backupbuffer","Backupbuffer must be kept counter %d", receiveChannel->backupBufferCounter);*/
        }
    }
    return result;
}

static nw_plugBufferDefragAdmin
nw_plugBufferDefragAdminCreate(
    nw_plugReceiveChannel receiveChannel)
{
    nw_plugBufferDefragAdmin admin = NULL;
    nw_plugBufferBackupAdmin temptail = NULL;

    if (receiveChannel->freeBuffers != NULL) {
        /* Get a buffer from the buffer pool */

        admin = receiveChannel->freeBuffers;
        receiveChannel->freeBuffers = admin->next;
        receiveChannel->nofUsedBuffers++;
        receiveChannel->nofFreeBuffers--;

        if (admin->usedCount>=1) {
            /* the reference count should be 0 */
            NW_REPORT_WARNING("receive data", "receive-buffer in pool has unexpected reference count");
        }
#if NW_BUFFER_TRAILS
        nw_plugBufferDefragAdminEmptyTrails(admin);
#endif
    } else {
        /* Allocate a new buffer from heap */
        NW_CONFIDENCE(receiveChannel->nofFreeBuffers == 0);
        /* look if backupAdmin has expired buffers if so use them else create a new buffer in separate function*/
        if (!receiveChannel->backupActive || !nw_plugBufferDefragBackupAdminRelease(receiveChannel)) {

            if ((receiveChannel->maxNofBuffers == 0) ||
                (receiveChannel->totNofBuffers < receiveChannel->maxNofBuffers)) {
                admin = (nw_plugBufferDefragAdmin)os_malloc(
                    nw__plugChannelGetFragmentLength(nw_plugChannel(receiveChannel)) +
                    DF_ADMIN_LENGTH);
                receiveChannel->nofUsedBuffers++;
                receiveChannel->totNofBuffers++;

                SET_USEDCOUNT(admin, 1); /* to supress valgrind warnings */

#if NW_BUFFER_TRAILS
                admin->link = receiveChannel->adminPool;
                receiveChannel->adminPool = admin;
                admin->trail = NULL;
#endif
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
        } else {
            /*adminBuffer can be used*/

            admin = receiveChannel->backupBufferTail->admin;
            temptail = receiveChannel->backupBufferTail;

            receiveChannel->backupBufferTail = receiveChannel->backupBufferTail->next;
            if ( receiveChannel->backupBufferTail ) {
                receiveChannel->backupBufferTail->prev = NULL;
            } else {
                receiveChannel->backupBufferHead = NULL;
            }
            receiveChannel->nofBackupBuffers--;

            NW_CONFIDENCE((receiveChannel->backupBufferTail == NULL) ==
                          (receiveChannel->nofBackupBuffers == 0));


            /* Adapt firstNr for the partitionnode
             */
            {
                nw_plugBuffer buffer;
                nw_seqNr NodeId;
                nw_partitionId PartitionId;
                nw_plugSendingPartitionNode currentNode = NULL;
                nw_bool found;

                buffer = DF_BUFFER(admin);
                NodeId = nw_plugBufferGetSendingNodeId(buffer);
                PartitionId = nw_plugDataBufferGetPartitionId(nw_plugDataBuffer(buffer));
                currentNode = receiveChannel->sendingPartitionNodes;
                found = FALSE;
                while ((currentNode != NULL) && !found) {
                    found = (
                        (currentNode->nodeId == NodeId) &&
                        (currentNode->partitionId == PartitionId));
                    if (!found) {
                        currentNode = currentNode->next;
                    }
                }
                if (found) {
                    currentNode->firstNr++;
                }
            }


            os_free(temptail);
            /*receiveChannel->backupBufferCounter--;*/

            /*NW_REPORT_WARNING("backupbuffer","backupbuffer REMOVED");*/
        }
    }
    if (admin != NULL) {
        admin->prev = NULL;
        admin->next = NULL;
        memset(&(admin->sender), 0, sizeof(NW_STRUCT(nw_senderInfo))); /* zero out */
        SET_USEDCOUNT(admin, 1);
    }

    return admin;
}


#ifdef NW_BUFFER_TRAILS
#define nw_plugBufferDefragAdminFree(admin) nw_plugBufferDefragAdminEmptyTrails(admin); os_free(admin)
#else
#define nw_plugBufferDefragAdminFree(admin) os_free(admin)
#endif
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
    /* Keep on behalf of this msgHolderPtr */
    nw_plugBufferDefragAdminKeep(admin);
}

static void
nw_msgHolderPtrAddHeadAdmin(
    nw_msgHolderPtr msgHolderPtr,
    nw_plugBufferDefragAdmin admin,
    nw_messageHolder messageHolder)
{
    msgHolderPtr->firstAdmin = admin;
    if ( !msgHolderPtr->lastAdmin) {
        msgHolderPtr->lastAdmin = admin;
    }
    msgHolderPtr->totNrOfFragmentsContained++;
    msgHolderPtr->messageHolder = messageHolder;
    /* Keep on behalf of this msgHolderPtr */
    nw_plugBufferDefragAdminKeep(admin);
}

static void
nw_msgHolderPtrAddMiddleAdmin(
    nw_msgHolderPtr msgHolderPtr,
    nw_plugBufferDefragAdmin admin)
{
    msgHolderPtr->totNrOfFragmentsContained++;
    if ((!msgHolderPtr->firstAdmin) ||
         msgHolderPtr->firstAdmin->prev == admin){
       msgHolderPtr->firstAdmin = admin;
    }
    if ((!msgHolderPtr->lastAdmin) ||
         msgHolderPtr->lastAdmin->next == admin){
       msgHolderPtr->lastAdmin = admin;
    }
    /* Keep on behalf of this msgHolderPtr */
    nw_plugBufferDefragAdminKeep(admin);
}

static void
nw_msgHolderPtrAddTailAdmin(
    nw_msgHolderPtr msgHolderPtr,
    nw_plugBufferDefragAdmin admin,
    nw_seqNr nrOfFragmentsNeeded)
{
    if ( !msgHolderPtr->firstAdmin) {
        msgHolderPtr->firstAdmin = admin;
    }
    msgHolderPtr->lastAdmin = admin;
    msgHolderPtr->totNrOfFragmentsContained++;
    msgHolderPtr->totNrOfFragmentsNeeded = nrOfFragmentsNeeded;
    /* Keep on behalf of this msgHolderPtr */
    nw_plugBufferDefragAdminKeep(admin);
}




/* ---------------------------- nw_plugSendingPartitionNode -------------------------- */


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
        result->announcing = FALSE;
        result->packetNrWaitingFor = 0; /* First packet can be any packet */
        result->outOfOrderCount =0;
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
        result->timeOfDead.tv_nsec = 0;
        result->timeOfDead.tv_sec = 0;
        result->firstNr = 0;
        *prevNext = result;
    }

    return result;
}

static void
nw_plugSendingPartitionNodeFree(
    nw_plugReceiveChannel receiveChannel,
    nw_plugSendingPartitionNode sendingPartitionNode)
{
    nw_plugBufferDefragAdmin admin;
    nw_msgHolderPtr msgHolderPtr;

    while (sendingPartitionNode->msgHolderPtrsHead != NULL) {
        msgHolderPtr = sendingPartitionNode->msgHolderPtrsHead;
        sendingPartitionNode->msgHolderPtrsHead = msgHolderPtr->next;

        admin = msgHolderPtr->firstAdmin;
        while (msgHolderPtr->firstAdmin != msgHolderPtr->lastAdmin) {
            msgHolderPtr->firstAdmin = admin->next;

            nw_plugBufferDefragAdminRelease(receiveChannel,admin);
            if (admin->usedCount == 1) {
                /*this admin buffer is no longer used, remove it from the list*/
                nw_plugBufferDefragAdminRelease(receiveChannel, admin);
            }
            admin = msgHolderPtr->firstAdmin;
        }

        nw_plugBufferDefragAdminRelease(receiveChannel,admin); /* for first */
        if (admin->usedCount == 1) {
            /*this admin buffer is no longer used, remove it from the list*/
            nw_plugBufferDefragAdminRelease(receiveChannel, admin);
        }

        nw_msgHolderPtrFree(msgHolderPtr);
    }
    admin = sendingPartitionNode->outOfOrderAdminsHead.next;
    while (admin->next != NULL) {
        sendingPartitionNode->outOfOrderAdminsHead.next = admin->next;
        nw_plugBufferDefragAdminRelease(receiveChannel,admin);
        admin = sendingPartitionNode->outOfOrderAdminsHead.next;
    }

    admin = sendingPartitionNode->defragAdminsHead.next;
    while (admin->next != NULL) {
        sendingPartitionNode->defragAdminsHead.next = admin->next;
        nw_plugBufferDefragAdminRelease(receiveChannel,admin);
        admin = sendingPartitionNode->defragAdminsHead.next;
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
    nw_plugReceiveChannel receiveChannel,
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

    if (result) {
        NW_CONFIDENCE(sendingPartitionNode->outOfOrderCount > 0);
        sendingPartitionNode->outOfOrderCount--;
        NW_CONFIDENCE(receiveChannel->nofBuffersOutOfOrder > 0);
        receiveChannel->nofBuffersOutOfOrder--;
    }

    NW_CONFIDENCE(((sendingPartitionNode->outOfOrderAdminsHead.next->next == NULL) ==
                   (sendingPartitionNode->outOfOrderAdminsTail.prev->prev == NULL)) &&
                  ((sendingPartitionNode->outOfOrderAdminsHead.next->next == NULL) ==
                   (sendingPartitionNode->outOfOrderCount == 0)));
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


    packetNrLookingFor = nw_plugDataBufferGetPacketNr(DF_DATA(admin));
    TRANSFER_USEDCOUNT(admin);

    result = NULL;
    *isDroppedReliably = FALSE;
    isOutOfOrder = FALSE;
    isOld = FALSE;

    if (orderPreservation) {
        /* If this packet was received out of order, insert in into the out of
         * order list or discard it if it is old */
        if (sendingPartitionNode->packetNrWaitingFor == 0) {
            sendingPartitionNode->packetNrWaitingFor = packetNrLookingFor;
            sendingPartitionNode->firstNr = packetNrLookingFor;
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

        }
        if (result == NULL) {

            sendingPartitionNode->outOfOrderCount++;
            receiveChannel->nofBuffersOutOfOrder++;

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
        if (sendingPartitionNode->outOfOrderCount > receiveChannel->maxReliabBacklog ){
            NW_REPORT_WARNING_1("reliable protocol",
               "Node 0x%x did not resend a missing packet in time, removing that node from the reliable protocol.",
               sendingPartitionNode->nodeId);

            sendingPartitionNode->active = FALSE;
            sendingPartitionNode->announcing = TRUE;
            sendingPartitionNode->timeOfDead = os_timeGet();

            /*release all admins for this PartitionNode*/
            nw_plugSendingPartitionNodeFree(receiveChannel,sendingPartitionNode);
        }

        if (result == NULL) {
            NW_TRACE_3(Send, 2,
                "Waiting for packet %d from node 0x%x, received packet %d",
                 sendingPartitionNode->packetNrWaitingFor, sendingPartitionNode->nodeId,packetNrLookingFor);
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
    TRANSFER_USEDCOUNT(admin);

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

    TRANSFER_USEDCOUNT(admin);

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
        insertedAdmin = nw_plugSendingPartitionNodeOutOfOrderListTake(receiveChannel,sendingPartitionNode, orderPreservation);
        TRANSFER_USEDCOUNT(admin);

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
    nw_bool result = FALSE;
    nw_msgHolderPtr msgHolderPtr = sendingPartitionNode->msgHolderPtrsHead;

    /* Check wether this is the most recent msgHolderptr, we always keep that one */
    if ( msgHolderPtr->next != NULL) {

        /* This is the oldest MsgHolderPtr and it's not the last one, so we're releasing its buffers */
        NW_CONFIDENCE( !nw_msgHolderPtrIsComplete(msgHolderPtr));
        NW_CONFIDENCE( msgHolderPtr->totNrOfFragmentsContained != 0 );
        admin = msgHolderPtr->firstAdmin;

        /* the msgHolderPtr is there for a reason, so there should be a corresponding admin */
        NW_CONFIDENCE(admin);/*let this be, inconsistency detected which should not occur*/
        if (admin)  {
            /*apparently the inconsistency occurs, avoid core dump.*/
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

/*
 * Checks if there are any partition nodes that need their status announced
 * and check if they have expired.
 * Must be executed periodicly.
 */
static void
nw_CheckAnnounce( nw_plugChannel channel )
{
    nw_plugReceiveChannel ReceiveChannel = nw_plugReceiveChannel(channel);
    nw_plugInterChannel interChannel;
    nw_plugSendingPartitionNode currentNode;
    os_time now;
    os_time age;

    currentNode = ReceiveChannel->sendingPartitionNodes;
    while ((currentNode != NULL)) {
        if (!currentNode->active && currentNode->announcing) {
            if ( currentNode->packetNrWaitingFor > 0 ) {
                /* send announce message */
                interChannel = nw__plugChannelGetInterChannelComm(nw_plugChannel(channel));
                NW_CONFIDENCE(interChannel != NULL);

                nw_plugInterChannelPostDataAnnounceMessage(interChannel,
                    currentNode->nodeId, currentNode->partitionId, currentNode->firstNr,currentNode->packetNrWaitingFor-1);

                NW_TRACE_5(Receive, 4, "plugChannel %s: RUCP receive: sendannounce : node %x partition %d first %d last %d",
                    nw__plugChannelGetName(nw_plugChannel(channel)),
                    currentNode->nodeId, currentNode->partitionId, currentNode->firstNr,currentNode->packetNrWaitingFor-1);
            }

            /* Check for end of announce period */
            now = os_timeGet();
            age = os_timeSub(now, currentNode->timeOfDead);
            if ( os_timeCompare(age, ReceiveChannel->ReliabilityRecoveryPeriod) == OS_MORE) {
                /* clear announcing flag and release all admins for this PartitionNode */
                currentNode->announcing = FALSE;
                currentNode->timeOfDead = os_timeGet();
                nw_plugSendingPartitionNodeFree(ReceiveChannel,currentNode);
                NW_TRACE_3(Receive, 4, "plugChannel %s: RUCP receive: Announcing stops for node %x partition %d",
                    nw__plugChannelGetName(nw_plugChannel(channel)),
                    currentNode->nodeId, currentNode->partitionId);
            }
        }
        currentNode = currentNode->next;
    }

}


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
    nw_bool compression;
    nw_networkSecurityPolicy securityPolicy;
    os_uint32 hash;
    c_ulong mTTL;


    NW_CONFIDENCE(receiveChannel != NULL);

    for (partitionId = 0;
         partitionId < nw_plugPartitionsGetNofPartitions(partitions);
         partitionId++) {
        nw_plugPartitionsGetPartition(partitions, partitionId,
            &found, &partitionAddress, &securityPolicy, &connected, &compression, &hash, &mTTL);
        NW_CONFIDENCE(found);

        /*add hash with partitionid value to the hashTree*/
        ut_tableInsert((ut_table)receiveChannel->partionIdToHashAdmin,&hash,&partitionId);
        if (found) {
            nw_socketAddPartition(nw__plugChannelGetSocket(
                nw_plugChannel(receiveChannel)), partitionId, partitionAddress,
                connected, compression, TRUE, mTTL);
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
    os_mutexAttr wakeAttr;
    nw_seqNr packetRetentionPeriodTemp;
    nw_seqNr ReliabilityRecoveryPeriodTemp;
    os_timeSec sectemp;
    os_int32 nsectemp;

    result = (nw_plugReceiveChannel)os_malloc(sizeof(*result));
    if (result != NULL) {
        nw_plugChannelInitialize(nw_plugChannel(result), seqNr, nodeId,
            NW_COMM_RECEIVE, partitions, userDataPtr, pathName,onFatal,onFatalUsrData);
        result->wakeupRequested = FALSE;
    os_mutexAttrInit( &wakeAttr);
        wakeAttr.scopeAttr = OS_SCOPE_PRIVATE;
    os_mutexInit( &result->wakeupMtx, &wakeAttr );
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
        result->nofBackupBuffers = 0;
        result->totNofBuffers = 0;
        result->fragmentsOutOfOrderDropped = 0;
        result->lastReceivedFragment = 0;
        result->nofBuffersWaiting = 0;
        result->nofBuffersOutOfOrder = 0;
        result->inUse = FALSE;
        result->backupBufferTail = NULL;
        result->backupBufferHead = NULL;
        result->periodicTriggerCount = 0;

#ifdef NW_BUFFER_TRAILS
        result->adminPool = NULL;
#endif

        result->fragmentSizeErrorLog = FALSE;

        if (nw_plugChannelGetReliabilityOffered(nw_plugChannel(result)) == NW_REL_RELIABLE) {
            DefDefragBuffersSize = NWCF_DEF_DefragBufferSize;
        } else {
            DefDefragBuffersSize = NWCF_DEF_DefragBufferSizeBestEffort;
        }
        /* translate backup time to os_time struct */
        packetRetentionPeriodTemp = NWCF_SIMPLE_SUBPARAM(ULong, pathName, Rx, PacketRetentionPeriod);
        sectemp = floor (packetRetentionPeriodTemp/1000);
        nsectemp = ((packetRetentionPeriodTemp-sectemp*1000)*10e6);
        result->packetRetentionPeriod.tv_sec = sectemp;
        result->packetRetentionPeriod.tv_nsec= nsectemp;

        result->backupActive = (packetRetentionPeriodTemp > 0 );

        /* translate backup time to os_time struct */
        ReliabilityRecoveryPeriodTemp = NWCF_SIMPLE_SUBPARAM(ULong, pathName, Rx, ReliabilityRecoveryPeriod);
        sectemp = floor (ReliabilityRecoveryPeriodTemp/1000);
        nsectemp = ((ReliabilityRecoveryPeriodTemp-sectemp*1000)*10e6);
        result->ReliabilityRecoveryPeriod.tv_sec = sectemp;
        result->ReliabilityRecoveryPeriod.tv_nsec= nsectemp;

        result->crc_check = NWCF_DEFAULTED_SUBPARAM(Bool, pathName, Rx, CrcCheck,NWCF_DEF_CrcCheck);

        result->maxNofBuffers = NWCF_DEFAULTED_SUBPARAM(ULong, pathName, Rx, DefragBufferSize,DefDefragBuffersSize);
        result->maxReliabBacklog = NWCF_DEFAULTED_SUBPARAM(ULong, pathName, Rx, MaxReliabBacklog,NWCF_DEF_MaxReliabBacklog);
#ifdef NW_TRACING
        result->buffersAllocatedTrace = DF_BUFUSE_MONITORING_START;
        result->buffersUsedTrace = DF_BUFUSE_MONITORING_START;
        result->buffersUsedTraceBottom = result->buffersUsedTrace;
#endif

        NW_SECURITY_DECODER_INIT(result, nodeId, partitions);


        result->partionIdToHashAdmin =  ut_tableNew (compareLeafs, NULL);
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
    os_uint32 i;

    /* own finalization */
    /* List with free buffers */

    if (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE) {
        NW_TRACE_2(Receive, 3, "Channel %s: %u reliable fragments dropped because they "
            "have been received twice or were too old",
            nw__plugChannelGetName(channel), receiveChannel->fragmentsOutOfOrderDropped);
    }

    while (receiveChannel->sendingPartitionNodes != NULL) {
        sendingPartitionNode = receiveChannel->sendingPartitionNodes;
        receiveChannel->sendingPartitionNodes = sendingPartitionNode->next;
        nw_plugSendingPartitionNodeFree(receiveChannel,sendingPartitionNode);
    }

    while (receiveChannel->readyMsgHolderPtrsHead != NULL) {
        msgHolderPtr = receiveChannel->readyMsgHolderPtrsHead;
        receiveChannel->readyMsgHolderPtrsHead = msgHolderPtr->next;
        nw_msgHolderPtrFree(msgHolderPtr);
    }

    while (receiveChannel->freeMsgHolderPtrs != NULL) {
        msgHolderPtr = receiveChannel->freeMsgHolderPtrs;
        receiveChannel->freeMsgHolderPtrs = msgHolderPtr->next;
        nw_msgHolderPtrFree(msgHolderPtr);
    }

    while (receiveChannel->waitingDataBuffersHead != NULL) {
        admin = receiveChannel->waitingDataBuffersHead;
        receiveChannel->waitingDataBuffersHead = admin->next;
        nw_plugBufferDefragAdminFree(admin);
    }

    i = 0;
    while (receiveChannel->freeBuffers != NULL) {
        admin = receiveChannel->freeBuffers;
        receiveChannel->freeBuffers = admin->next;
        nw_plugBufferDefragAdminFree(admin);
        i++;
    }
    NW_CONFIDENCE(i == receiveChannel->nofFreeBuffers);
    NW_SECURITY_DECODER_DEINIT(receiveChannel);

    os_mutexDestroy( &receiveChannel->wakeupMtx );
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
    nw_plugChannel channel = nw_plugChannel(receiveChannel);
    nw_bool result = FALSE;
    static nw_bool reported_garbage_error = FALSE;
    os_time timeout = {0, 200000000}; /* 200 milli seconds */

    if (nw_plugChannelGetReliabilityOffered(nw_plugChannel(receiveChannel)) != NW_REL_RELIABLE) {
        sendingPartitionNode = receiveChannel->sendingPartitionNodes;
        while ((sendingPartitionNode != NULL) && (!result)) {
            if ( sendingPartitionNode->msgHolderPtrsHead != NULL)  {
                result = nw_plugSendingPartitionDropMsgHolderPtr(receiveChannel,sendingPartitionNode);
            }
            sendingPartitionNode = sendingPartitionNode->next;
        }
        if ((!result )&& (!reported_garbage_error)){
            /* Listen very carefully, I shall only say this once */
            NW_REPORT_WARNING_2("receiving data from the network",
                "Channel \"%s\" exceeded maximum defragmentation buffers %u, "
                "But unable to garbage collect buffers",
                nw__plugChannelGetName(nw_plugChannel(receiveChannel)),
                receiveChannel->maxNofBuffers);
            reported_garbage_error = TRUE;
        }
        if (!result) {
            if ( receiveChannel->waitingDataBuffersHead == NULL )
            {
                /* no buffers to garbage collect and no buffers left to process.
                 * there's no other option than to just terminate the networking service.
                 */

                if ((channel->onFatal != NULL) &&
                    (nw__plugChannelGetReliabilityOffered(channel) != NW_REL_RELIABLE))
                {
                    NW_REPORT_ERROR_1("receiving data from the network",
                        "Channel \"%s\" has no more fragmentbuffers available, "
                        "An incoming fragmented message is too large to fit in "
                        "the configured fragmentbuffers. Increase the size of the "
                        "fragmentbuffers (config://NetworkService/Channels/Channel/FramentSize)"
                        "and/or the maximum number of fragmentbuffers to use "
                        "(config://NetworkService>Channels>Channel>Receiving>DefragBufferSize). "
                        "Terminating Networking service",
                        nw__plugChannelGetName(channel));
                    channel->onFatal(channel->onFatalUsrData);
                    channel->onFatal = NULL;
                }
                os_nanoSleep(timeout); /* Give network mainthread chance to process the callback */
        os_mutexLock( &receiveChannel->wakeupMtx );
                receiveChannel->wakeupRequested = TRUE;
        os_mutexUnlock( &receiveChannel->wakeupMtx );
            }
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
                admin = nw_plugSendingPartitionNodeOutOfOrderListTake(receiveChannel,sendingPartitionNode,
                    TRUE);
                while (admin != NULL) {
                    insertedAdmin = nw_plugSendingPartitionNodeInsertAdmin(receiveChannel,
                        sendingPartitionNode, admin, TRUE);
                    NW_CONFIDENCE(insertedAdmin == admin);
                    /* Release this reference, buffer is refcounted by the list */
                    nw_plugBufferDefragAdminRelease(receiveChannel, admin);
                    admin = nw_plugSendingPartitionNodeOutOfOrderListTake(receiveChannel,sendingPartitionNode, TRUE);
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
nw_plugReceiveChannelInsertBackupAdmin(
    nw_plugReceiveChannel receiveChannel,
    nw_plugBufferDefragAdmin lastReturnedBuffer)
{

    /*create backupAdmin */
    nw_plugBufferBackupAdmin backupBuffer = (nw_plugBufferBackupAdmin) os_malloc(sizeof(NW_STRUCT(nw_plugBufferBackupAdmin)));
    /*receiveChannel->backupBufferCounter++;*/
    /* first node in the list */
    if (receiveChannel->backupBufferTail == NULL && receiveChannel->backupBufferHead == NULL) {
            backupBuffer->admin = lastReturnedBuffer;
            backupBuffer->time.tv_sec = 0;
            backupBuffer->time.tv_nsec = 0;
            backupBuffer->next = NULL;
            backupBuffer->prev = NULL;

            /* tail and head point to the same backupAdmin*/
            receiveChannel->backupBufferHead = backupBuffer;
            receiveChannel->backupBufferTail = backupBuffer;
            /*NW_REPORT_WARNING("backupbuffer","Backupbuffer FIRST BUFFER");*/
    } else {
        /* add new node at the end of the linkedlist head buffer changes*/

        backupBuffer->admin = lastReturnedBuffer;
        backupBuffer->time.tv_sec = 0;
        backupBuffer->time.tv_nsec = 0;

        /*current next will be the new backupBuffer*/
        receiveChannel->backupBufferHead->next = backupBuffer;

        /* new prev will be current head*/
        backupBuffer->prev = receiveChannel->backupBufferHead;

        /* next is unknown*/
        backupBuffer->next = NULL;

        /* current head will be new backupBuffer*/
        receiveChannel->backupBufferHead = backupBuffer;

    }
    receiveChannel->nofBackupBuffers++;
}

static void
nw_plugReceiveChannelTakeFirstMessageHolder(
    nw_plugReceiveChannel receiveChannel,
    nw_messageHolder *messageHolder)
{
    /** (FR) The message-queueing requires some refactoring */

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
            /*this lastReturnedBuffer buffer is no longer used, remove it from the list*/
            if (receiveChannel->backupActive) {
                if (lastReturnedBuffer->prev != NULL) {
                    lastReturnedBuffer->prev->next = lastReturnedBuffer->next;
                    if(lastReturnedBuffer->next != NULL) {
                        lastReturnedBuffer->next->prev = lastReturnedBuffer->prev;
                    }
                }
                nw_plugReceiveChannelInsertBackupAdmin(receiveChannel, lastReturnedBuffer);
            } else {
               /* release when backupBuffer is freed  */
               nw_plugBufferDefragAdminRelease(receiveChannel, lastReturnedBuffer);
           }

        }
        receiveChannel->lastReturnedBuffer = NULL;
        receiveChannel->lastReturnedHolder = NULL;
        receiveChannel->lastReturnedMsgHolderPtr = NULL;
    }

    msgHolderPtr = receiveChannel->readyMsgHolderPtrsHead;
    if (msgHolderPtr != NULL) {
        /* Fill the result */
        *messageHolder = msgHolderPtr->messageHolder;
        receiveChannel->lastReturnedBuffer = msgHolderPtr->firstAdmin;
        receiveChannel->lastReturnedHolder = msgHolderPtr->messageHolder;
        receiveChannel->lastReturnedMsgHolderPtr = msgHolderPtr;
    } else {
        *messageHolder = NULL;
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
    nw_senderInfo sender)
{
    nw_bool result = FALSE;
    nw_messageHolder messageHolder = NULL;

    nw_plugReceiveChannelTakeFirstMessageHolder(channel, &messageHolder);
    if (messageHolder != NULL) {
        *data = NW_MESSAGEHOLDER_DATA(messageHolder);
        *length = nw_messageHolderGetLength(messageHolder);
        NW_CONFIDENCE(*data!=NULL); /* if one of these two checks fails, */
        NW_CONFIDENCE(*length>0);    /* something must be wrong with the queueing */

        if (sender != NULL) {
            /* so far this copies a 4-octet integer, later with IPv6 it
             * will be necessary to copy the 12-octet IPv6 address */
            sender->ipAddress = channel->lastReturnedBuffer->sender.ipAddress;
            /* no strdup required here, sender does not take ownership */
            sender->dn = channel->lastReturnedBuffer->sender.dn;
        }
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
    os_sockaddr_storage sendingAddress,
    nw_plugControlBuffer buffer)
{
    nw_plugInterChannel interChannel;
    nw_plugControlMessage message;
    nw_bool more;
    nw_partitionId partitionHashToId;
    os_int32 partitionHash;

    NW_CONFIDENCE(nw__plugChannelGetReliabilityOffered(nw_plugChannel(channel)) == NW_REL_RELIABLE);

    interChannel = nw__plugChannelGetInterChannelComm(nw_plugChannel(channel));
    NW_CONFIDENCE(interChannel != NULL);


    NW_TRACE_2(Receive, 4, "plugChannel %s: Recvthread: recvACK : %d",
        nw__plugChannelGetName(nw_plugChannel(channel)),nw_plugControlBufferGetRecvBufferInUse(buffer));

    message = NULL;
    do {
        message = nw_plugControlBufferGetNextMessage(buffer, message, &more);
        NW_CONFIDENCE(message != NULL);
        partitionHash = nw_plugControlMessageGetPartitionId(message);
        partitionHashToId = nw_plugPartitionsGetPartitionIdByHash(nw_plugChannel(channel)->partitions,partitionHash);

        nw_plugInterChannelPostAckReceivedMessage(interChannel,
            sendingNodeId,
            partitionHashToId,
            sendingAddress,
            nw_plugControlMessageGetStartingNr(message),
            nw_plugControlMessageGetClosingNr(message),
            nw_plugControlBufferGetRecvBufferInUse(buffer));
    } while (more);
}

static void
nw_plugReceiveChannelCommunicateDataReceivedReliably(
    nw_plugReceiveChannel channel,
    os_sockaddr_storage sendingAddress,
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
        nw_plugDataBufferGetPacketNr(buffer),channel->nofBuffersWaiting + channel->nofBuffersOutOfOrder);

    NW_TRACE_2(Receive, 4, "plugChannel %s: Recvthread: sendACK : %d",
        nw__plugChannelGetName(nw_plugChannel(channel)),
        channel->nofBuffersWaiting + channel->nofBuffersOutOfOrder);
}

static void
nw_plugReceiveChannelProcessDataAnnounce(
    nw_plugReceiveChannel channel,
    nw_seqNr sendingNodeId,
    os_sockaddr_storage sendingAddress,
    nw_plugControlBuffer buffer)
{
    nw_plugInterChannel interChannel;
    nw_plugControlAltMessage message;
    nw_partitionId partitionHashToId;
    os_int32 partitionHash;
    nw_seqNr nodeId;
    nw_bool more;
    nw_plugSendingPartitionNode currentNode = NULL;
    nw_bool found;

    NW_CONFIDENCE(nw__plugChannelGetReliabilityOffered(nw_plugChannel(channel)) == NW_REL_RELIABLE);

    interChannel = nw__plugChannelGetInterChannelComm(nw_plugChannel(channel));
    NW_CONFIDENCE(interChannel != NULL);

    /* For each message in the announce
     *  find the partition node
     *  if inactive and annouced contains message >= packetwaitiung for
     *     send request
     */
    NW_TRACE_2(Receive, 5, "Channel %s: RUPC,"
        "Receiving announce from 0x%x",
        nw__plugChannelGetName(nw_plugChannel(channel)),sendingNodeId);

    message = NULL;
    do {
        message = nw_plugControlBufferGetNextAltMessage(buffer, message, &more);
        NW_CONFIDENCE(message != NULL);
        nodeId = nw_plugControlAltMessageGetDiedNodeId(message);
        partitionHash = nw_plugControlAltMessageGetPartitionId(message);
        partitionHashToId = nw_plugPartitionsGetPartitionIdByHash(nw_plugChannel(channel)->partitions,partitionHash);

        NW_TRACE_4(Receive, 5, "Channel %s: RUPC,"
            "announce message # node_id 0x%x part_hash 0x%x part_id 0x%x",
            nw__plugChannelGetName(nw_plugChannel(channel)), nodeId, partitionHash,partitionHashToId);

        currentNode = channel->sendingPartitionNodes;
        found = FALSE;
        while ((currentNode != NULL) && !found) {
            NW_TRACE_3(Receive, 5, "Channel %s: RUPC,"
                "announce message # checking against node 0x%x part 0x%x",
                nw__plugChannelGetName(nw_plugChannel(channel)), currentNode->nodeId, currentNode->partitionId);
            found = (
                (currentNode->nodeId == nodeId) &&
                ((currentNode->partitionId) == partitionHashToId));
            if (!found) {
                currentNode = currentNode->next;
            }
        }
        if (found) {
            NW_TRACE_5(Receive, 5, "Channel %s: RUPC,"
                "announce message #found partnode  active %d waitfor  %d first %d last %d",
                nw__plugChannelGetName(nw_plugChannel(channel)), currentNode->active, currentNode->packetNrWaitingFor, nw_plugControlAltMessageGetFirstNr(message),nw_plugControlAltMessageGetLastNr(message));
            if( !currentNode->active &&
                currentNode->packetNrWaitingFor >=  nw_plugControlAltMessageGetFirstNr(message) &&
                currentNode->packetNrWaitingFor <=  nw_plugControlAltMessageGetLastNr(message) ) {


                NW_TRACE_1(Receive, 5, "Channel %s: RUPC,"
                    "sending Request message",
                    nw__plugChannelGetName(nw_plugChannel(channel)));


               nw_plugInterChannelPostDataRequestMessage(interChannel,
                                                         sendingNodeId,
                                                         sendingAddress,
                                                         nodeId,
                                                         partitionHashToId,
                                                         currentNode->packetNrWaitingFor,
                                                         nw_plugControlAltMessageGetLastNr(message));
            } else {
                NW_TRACE_1(Receive, 5, "Channel %s: RUPC,"
                    "Up to date, no request needed",
                    nw__plugChannelGetName(nw_plugChannel(channel)));
            }
        } else {
            NW_TRACE_1(Receive, 5, "Channel %s: RUPC,"
                "announce message partition-node not found",
                nw__plugChannelGetName(nw_plugChannel(channel)));
        }

    } while (more);


    NW_TRACE_1(Receive, 4, "plugChannel %s: Recvthread: announce received ",
        nw__plugChannelGetName(nw_plugChannel(channel)));
}


static void
nw_plugReceiveChannelProcessDataRequest(
    nw_plugReceiveChannel channel,
    nw_seqNr sendingNodeId,
    os_sockaddr_storage sendingAddress,
    nw_plugControlBuffer buffer)
{
    nw_plugInterChannel interChannel;
    nw_bool found;
    nw_partitionId partitionHashToId;
    os_int32 partitionHash;
    nw_partitionId thisPartitionId;
    os_int32 thisPartitionHash;
    nw_plugControlAltMessage message = NULL;
    nw_seqNr nodeId;
    nw_seqNr thisNodeId;
    nw_seqNr Nr;
    nw_plugBufferBackupAdmin currentBackBuffer;
    nw_plugBuffer thisBuffer;

    NW_CONFIDENCE(nw__plugChannelGetReliabilityOffered(nw_plugChannel(channel)) == NW_REL_RELIABLE);

    interChannel = nw__plugChannelGetInterChannelComm(nw_plugChannel(channel));
    NW_CONFIDENCE(interChannel != NULL);

    NW_TRACE_2(Receive, 5, "Channel %s: RUPC,"
        "Receiving request from 0x%x",
        nw__plugChannelGetName(nw_plugChannel(channel)),sendingNodeId);

    message = nw_plugControlBufferGetNextAltMessage(buffer, message, &found);
    NW_CONFIDENCE(message != NULL);
    nodeId = nw_plugControlAltMessageGetDiedNodeId(message);
    partitionHash = nw_plugControlAltMessageGetPartitionId(message);
    partitionHashToId = nw_plugPartitionsGetPartitionIdByHash(nw_plugChannel(channel)->partitions,partitionHash);
    NW_TRACE_5(Receive, 5, "Channel %s: RUPC,"
        "Requested: deadnode 0x%x part 0x%x first %d last %d",
        nw__plugChannelGetName(nw_plugChannel(channel)),nodeId,partitionHashToId,nw_plugControlAltMessageGetFirstNr(message),nw_plugControlAltMessageGetLastNr(message));
    currentBackBuffer = channel->backupBufferHead;
    found = FALSE;
    while (currentBackBuffer && !found ) {

        thisBuffer = DF_BUFFER(currentBackBuffer->admin);

        thisNodeId = nw_plugBufferGetSendingNodeId(thisBuffer);
        thisPartitionHash = nw_plugDataBufferGetPartitionId(nw_plugDataBuffer(thisBuffer));
        thisPartitionId = nw_plugPartitionsGetPartitionIdByHash(nw_plugChannel(channel)->partitions,thisPartitionHash);



        Nr = nw_plugDataBufferGetPacketNr(nw_plugDataBuffer(thisBuffer));
        NW_TRACE_4(Receive, 5, "Channel %s: RUPC,"
            "Browsing backups # node_id 0x%x part 0x%x message %d",
            nw__plugChannelGetName(nw_plugChannel(channel)),thisNodeId,thisPartitionId,Nr);
        if (nodeId == thisNodeId &&
            partitionHashToId == thisPartitionId &&
            Nr >= nw_plugControlAltMessageGetFirstNr(message) &&
            Nr <= nw_plugControlAltMessageGetLastNr(message) ) {

            NW_TRACE_2(Receive, 5, "Channel %s: RUPC,"
                "MATCH: sending backup to 0x%x ",
                nw__plugChannelGetName(nw_plugChannel(channel)),sendingNodeId);

            nw_plugInterChannelPostBackupReceivedMessage(interChannel,
                                                         sendingNodeId,
                                                         thisPartitionId,
                                                         sendingAddress,
                                                         nw_plugDataBuffer(thisBuffer),
                                                         0);
        }
        if (nodeId == thisNodeId &&
            partitionHashToId == thisPartitionId &&
            Nr < nw_plugControlAltMessageGetFirstNr(message)) {
            /* reached start of requested range, so qe can quit */
            found = TRUE;
        }
        currentBackBuffer = currentBackBuffer->prev;
    }



    NW_TRACE_1(Receive, 4, "plugChannel %s: Recvthread: request received ",
        nw__plugChannelGetName(nw_plugChannel(channel)));
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
    if ( ! nw_plugSendingPartitionNodeIsActive(currentNode) &&
           nw_plugChannel(receiveChannel)->reconnectAllowed         ) {

        /* mark them as active again and Initialize */
        currentNode->active = TRUE;
        currentNode->announcing = FALSE;
        currentNode->packetNrWaitingFor = 0; /* Next packet can be any packet */
        currentNode->firstNr = 0; /* Next packet can be any packet */
        currentNode->outOfOrderCount =0;
        currentNode->msgHolderPtrsHead = NULL;
        currentNode->msgHolderPtrsTail = NULL;
        currentNode->outOfOrderAdminsHead.prev = NULL;
        currentNode->outOfOrderAdminsHead.next = &currentNode->outOfOrderAdminsTail;
        currentNode->outOfOrderAdminsTail.prev = &currentNode->outOfOrderAdminsHead;
        currentNode->outOfOrderAdminsTail.next = NULL;
        currentNode->defragAdminsHead.prev = NULL;
        currentNode->defragAdminsHead.next = &currentNode->defragAdminsTail;
        currentNode->defragAdminsTail.prev = &currentNode->defragAdminsHead;
        currentNode->defragAdminsTail.next = NULL;
    }


    if ( nw_plugSendingPartitionNodeIsActive(currentNode) ||
         (receiveChannel->backupActive && currentNode->announcing)) {

        if (receiveChannel->backupActive && currentNode->announcing) {
            NW_TRACE_1(Receive, 5, "Channel %s: packet received from an announcing node",
                nw__plugChannelGetName(nw_plugChannel(receiveChannel)));
        }

        /* Insert the admin into the defrag of out of order list */
        admin = DF_ADMIN(buffer);

        TRANSFER_USEDCOUNT(admin);

        insertedAdmin = nw_plugSendingPartitionNodeInsertAdmin(receiveChannel, currentNode,
            admin, FALSE);
        /* Release this reference, buffer is refcounted by the list, if inserted */
        nw_plugBufferDefragAdminRelease(receiveChannel, admin);
        if (insertedAdmin == admin) {
            /* Check if any buffers have become not out of order */
            admin = nw_plugSendingPartitionNodeOutOfOrderListTake(receiveChannel,currentNode,
                orderPreservation);
            while (admin != NULL) {
                TRANSFER_USEDCOUNT(admin);
                insertedAdmin = nw_plugSendingPartitionNodeInsertAdmin(receiveChannel,
                    currentNode, admin, TRUE);
                NW_CONFIDENCE(insertedAdmin == admin);
                /* Release this reference, buffer is refcounted by the list */
                nw_plugBufferDefragAdminRelease(receiveChannel, admin);
                admin = nw_plugSendingPartitionNodeOutOfOrderListTake(receiveChannel,currentNode,
                        orderPreservation);
            }
        }
    } else {
        NW_TRACE_4(Receive, 5, "Channel %s: packet received from inactive node 0x%x part 0x%x Announcing %d,"
            " that has been removed from the protocol for this channel",
            nw__plugChannelGetName(nw_plugChannel(receiveChannel)),
            currentNode->nodeId,currentNode->partitionId,currentNode->announcing);
        nw_plugReceiveChannelReleaseBuffer(receiveChannel, buffer);
    }
}

nw_partitionId
nw_plugReceiveChannelLookUpPartitionHash(
        nw_plugReceiveChannel channel,
        nw_partitionId partitionHash)
{
    nw_partitionId partitionHashToId;


    partitionHashToId = nw_plugPartitionsGetPartitionIdByHash(nw_plugChannel(channel)->partitions,partitionHash);


    return partitionHashToId;
}


static nw_bool
nw_plugReceiveChannelReadSocket(
    nw_plugReceiveChannel channel,
    os_time *timeOut,
    nw_bool *dataRead)
{
    sk_length readLength;
    sk_length bufferLength;
    nw_plugBufferDefragAdmin admin;
    nw_plugBuffer buffer;
    nw_bool result = OS_TRUE;
    nw_seqNr sendingNodeId;
    nw_bool reliable;
    sk_length dataLength;
    nw_partitionId sendingPartitionId;
    nw_bool found;
    char *addressString;
    nw_networkSecurityPolicy networkSecurityPolicy;
    nw_bool connected;
    nw_bool compression;
    nw_bool crc_correct;
    os_uint32 crc;
    os_uint32 partitionHash;
    c_ulong mTTL;

    *dataRead = OS_TRUE; /* nw_bool is typedef-ed os_boolean */
    NW_CONFIDENCE(sizeof(os_sockaddr_storage) == sizeof(admin->sender.ipAddress));

    buffer = nw_plugReceiveChannelCreateBuffer(channel);
    if (buffer) {
        const nw_length fragmentLength =
            nw__plugChannelGetFragmentLength(nw_plugChannel(channel));

        admin = DF_ADMIN(buffer);

        TRANSFER_USEDCOUNT(admin);

        readLength = nw_socketReceive(
            nw__plugChannelGetSocket(nw_plugChannel(channel)),
            (os_sockaddr_storage *)&admin->sender.ipAddress, buffer,
            fragmentLength, timeOut, channel->prs);

        NW_SECURITY_DECODER_PROCESS(bufferLength, channel, &(admin->sender), buffer, readLength, fragmentLength);

        /* if security is enabled the readLength includes the security header
         * the bufferLength is the original readLength without the security header */
        readLength = bufferLength;

        if (readLength >= OS_SIZEOF(nw_plugBuffer)) {

            if (nw_plugBufferCheckVersion(buffer, NW_CURRENT_PROTOCOL_VERSION)){

                dataLength = nw_plugBufferGetLength(buffer);

                /* check if received length is equal to the length field of the message*/
                if (readLength == dataLength) {

                    /* skip crc calculation if crc_check is not set in the configfile*/
                    if (channel->crc_check) {
                        crc = ut_crcCalculate(nw_plugChannel(channel)->crc,
                                  ((os_char*)buffer)+sizeof(NW_STRUCT(nw_plugBuffer)),
                                  readLength-sizeof(NW_STRUCT(nw_plugBuffer)));
                        crc_correct = (crc == nw_plugBuffer(buffer)->crc);
                    } else {
                        crc = 0;
                        crc_correct = 1;
                    }

                    if (crc_correct) {
                        sendingNodeId = nw_plugBufferGetSendingNodeId(buffer);
                        if (nw_plugBufferGetControlFlag(buffer)) {

                            switch (nw_plugControlBufferGetRecvBufferInUse(nw_plugControlBuffer(buffer))) {
                                case NW_CONTROL_TAG_DATA_ANNOUNCE:
                                    /* process the incoming data announce message */
                                    nw_plugReceiveChannelProcessDataAnnounce(
                                        nw_plugReceiveChannel(channel), sendingNodeId,
                                                admin->sender.ipAddress, nw_plugControlBuffer(buffer));
                                    break;
                                case NW_CONTROL_TAG_DATA_REQUEST:
                                    /* process the incoming data request message */
                                    nw_plugReceiveChannelProcessDataRequest(
                                        nw_plugReceiveChannel(channel), sendingNodeId,
                                                admin->sender.ipAddress, nw_plugControlBuffer(buffer));
                                    break;
                                default:
                                    /* communicate incoming control message to sending thread */
                                    nw_plugReceiveChannelCommunicateControlReceived(
                                        nw_plugReceiveChannel(channel), sendingNodeId,
                                                admin->sender.ipAddress, nw_plugControlBuffer(buffer));
                            }
                            nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                        } else {

                            if (dataLength > 0) {
                                if (channel->prs != NULL) {
                                    if (channel->prs->enabled) {
                                        channel->prs->numberOfBytesReceived += dataLength;
                                        channel->prs->numberOfPacketsReceived++;
                                    }
                                }

                                NW_STAMP(nw_plugDataBuffer(buffer),NW_BUF_TIMESTAMP_RECEIVE);

                                partitionHash = nw_plugDataBufferGetPartitionId(nw_plugDataBuffer(buffer));
                                sendingPartitionId = nw_plugPartitionsGetPartitionIdByHash(nw_plugChannel(channel)->partitions,partitionHash);


                                connected = FALSE;
                                nw_plugChannelGetPartition(nw_plugChannel(channel), sendingPartitionId,&found, &addressString, &networkSecurityPolicy, &connected, &compression, &partitionHash, &mTTL);
#ifdef FILTER_ON_SENDING_PARTITION
                                if (connected) {
#endif
                                    /* communicate incoming data message to sending thread
                                     * for reliable channels (for acking) */
                                    reliable = (nw__plugChannelGetReliabilityOffered(nw_plugChannel(channel)) == NW_REL_RELIABLE);
                                    NW_CONFIDENCE(nw_plugBufferGetReliabilityFlag(buffer) == reliable);
                                    if (reliable) {
                                        nw_plugReceiveChannelCommunicateDataReceivedReliably(
                                            channel, admin->sender.ipAddress,
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
                                    channel->nofBuffersWaiting++;

#ifdef FILTER_ON_SENDING_PARTITION
                                    TRANSFER_USEDCOUNT(admin);
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
#endif
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
                        NW_REPORT_WARNING_3("receive data",
                             "received data with an incorrect calculated crc %x receive crc %x packet length: %d",
                             crc,
                             nw_plugBuffer(buffer)->crc,
                             readLength-sizeof(NW_STRUCT(nw_plugBuffer)));


                        nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                        *dataRead = FALSE;
                    }
                } else {
                    if (readLength == nw__plugChannelGetFragmentLength(nw_plugChannel(channel))) {
                        if (!channel->fragmentSizeErrorLog) {
                            NW_REPORT_WARNING_2("receive data",
                                "received data with an incorrect fragment length due to inconsistent fragmentsize configuration (received:%d expected: %d)",
                                readLength,dataLength);
                            channel->fragmentSizeErrorLog = TRUE;
                        }
                    } else {
                        NW_REPORT_WARNING_2("receive data",
                            "received data with an incorrect fragment length (received:%d expected: %d)",
                            readLength,dataLength);

                    }
                    nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                    *dataRead = OS_FALSE;
                }
            } else {
                /* Received data was not a valid ospl packet */
                NW_REPORT_WARNING("receive data",
                     "received data with an incorrect protocol-version id");
                nw_plugReceiveChannelReleaseBuffer(channel, buffer);
                *dataRead = FALSE;
            }
        } else {
            /* ReadLength < plugbufferheader, so no valid data */

            nw_plugReceiveChannelReleaseBuffer(channel, buffer);
            *dataRead = FALSE;
        }
    } else {
        result = OS_FALSE;
        *dataRead = OS_FALSE;
    }
    return result;
}




static nw_plugBufferDefragAdmin
nw_plugReceiveChannelTakeWaitingDataBuffer(
    nw_plugReceiveChannel channel)
{
    nw_plugBufferDefragAdmin admin = NULL;

    if (channel->waitingDataBuffersHead) {
        NW_CONFIDENCE(channel->nofBuffersWaiting > 0 );
        admin = channel->waitingDataBuffersHead;
        TRANSFER_USEDCOUNT(admin);
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
        channel->nofBuffersWaiting--;
    }
    return admin;
}

/*
 * This operation checks the messageBox and frees the
 * administration and buffers for node that have stopped or died.
 */

static void
nw_CheckMessageBox( nw_plugChannel channel )
{
    nw_plugReceiveChannel ReceiveChannel = nw_plugReceiveChannel(channel);
    nw_messageBoxMessageType messageType;
    nw_bool messageReceived;
    os_sockaddr_storage sendingAddress;
    nw_seqNr sendingNodeId;
    nw_plugSendingPartitionNode currentNode;
    c_string list;

    /* Walk over all messages in the messageBox */
    messageReceived = nw_plugChannelProcessMessageBox(channel,
        &sendingNodeId, &sendingAddress, &list, &messageType);
    while (messageReceived) {
        switch (messageType) {
            case NW_MBOX_NODE_STARTED:
                if (channel->reconnectAllowed) {
                    /* Now walk over all partitionNodes to find the ones that match this NodeId*/
                    currentNode = ReceiveChannel->sendingPartitionNodes;
                    while ((currentNode != NULL)) {
                        if (currentNode->nodeId == sendingNodeId) {
                            /* mark them as active again and Initialize */
                            currentNode->active = TRUE;
                            currentNode->announcing = FALSE;
                            currentNode->packetNrWaitingFor = 0; /* Next packet can be any packet */
                            currentNode->outOfOrderCount =0;
                            currentNode->msgHolderPtrsHead = NULL;
                            currentNode->msgHolderPtrsTail = NULL;
                            currentNode->outOfOrderAdminsHead.prev = NULL;
                            currentNode->outOfOrderAdminsHead.next = &currentNode->outOfOrderAdminsTail;
                            currentNode->outOfOrderAdminsTail.prev = &currentNode->outOfOrderAdminsHead;
                            currentNode->outOfOrderAdminsTail.next = NULL;
                            currentNode->defragAdminsHead.prev = NULL;
                            currentNode->defragAdminsHead.next = &currentNode->defragAdminsTail;
                            currentNode->defragAdminsTail.prev = &currentNode->defragAdminsHead;
                            currentNode->defragAdminsTail.next = NULL;
                        }
                        currentNode = currentNode->next;
                    }
                }
            break;
            case NW_MBOX_NODE_STOPPED:
            case NW_MBOX_NODE_DIED:
                NW_TRACE_2(Receive, 5, "Channel %s: RUPC,"
                    " node reported dead 0x%x",
                    nw__plugChannelGetName(nw_plugChannel(ReceiveChannel)),sendingNodeId);
                /* Now walk over all partitionNodes to find the ones that match this NodeId*/
                currentNode = ReceiveChannel->sendingPartitionNodes;
                while ((currentNode != NULL)) {
                    NW_TRACE_2(Receive, 5, "Channel %s: RUPC,"
                        " checking against 0x%x",
                        nw__plugChannelGetName(nw_plugChannel(ReceiveChannel)),currentNode->nodeId);
                    if (currentNode->nodeId == sendingNodeId) {
                        /* mark as inactive and start announcing or release all admins for this PartitionNode */
                        currentNode->active = FALSE;
                        if (ReceiveChannel->backupActive ) {
                            NW_TRACE_2(Receive, 5, "Channel %s: RUPC,"
                                " Match... sending a announce (backup_active = %d)",
                                nw__plugChannelGetName(nw_plugChannel(ReceiveChannel)),ReceiveChannel->backupActive);
                            currentNode->announcing = TRUE;
                            currentNode->timeOfDead = os_timeGet();
                            nw_CheckAnnounce(channel);
                        } else {
                            nw_plugSendingPartitionNodeFree(ReceiveChannel,currentNode);
                        }
                    }
                    currentNode = currentNode->next;
                }
            break;
            case NW_MBOX_GP_ADD:
            case NW_MBOX_GP_ADDLIST:
            case NW_MBOX_GP_REMOVE:
                /* Don't handle here */
            break;
            case NW_MBOX_UNDEFINED:
                NW_CONFIDENCE(messageType != NW_MBOX_UNDEFINED);
            break;
        }
        messageReceived = nw_plugChannelProcessMessageBox(channel,
            &sendingNodeId, &sendingAddress, &list, &messageType);
    }

    if (ReceiveChannel->backupActive) {
        if (nw_plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE &&
            nw_plugInterChannelGetTrigger(nw__plugChannelGetInterChannelComm(channel))) {
            ReceiveChannel->periodicTriggerCount++;
            /* Evaluate announcements every 5 resolution ticks, HARDCODED for now */
            if ( ReceiveChannel->periodicTriggerCount > 5 ){
                ReceiveChannel->periodicTriggerCount = 0;
                nw_CheckAnnounce(channel);
            }
        }
    }
}

/*
 * Read data from the socket as long as there is something to read
 * and communicate incoming data and control messages to the send-thread
 */
void
nw_plugReceiveChannelProcessIncoming(nw_plugChannel channel)
{
    static os_time zeroTime = {0,0}; /* TODO should not be static, but constant , static causes memory region shared by all concurrent threads*/
    nw_bool dataRead = OS_TRUE;
    nw_bool result = OS_TRUE;

    nw_CheckMessageBox(channel);
    while (result && dataRead ) {
        dataRead = OS_FALSE; /* reset */
        result = nw_plugReceiveChannelReadSocket(nw_plugReceiveChannel(channel), &zeroTime, &dataRead);
   }
}

c_bool
nw_plugReceiveChannelMessageStart(
    nw_plugChannel channel,
    nw_data *data,
    nw_length *length,
    nw_senderInfo sender,
    plugReceiveStatistics prs)

{
    nw_plugReceiveChannel  receiveChannel = nw_plugReceiveChannel(channel);
    nw_bool messageWaiting;
    nw_plugBuffer buffer;
    nw_plugBufferDefragAdmin admin;
    nw_seqNr sendingNodeId;
    nw_partitionId sendingPartitionIdHash;
    nw_partitionId partitionHashToId;
    nw_bool reliable;
    nw_bool read_result = TRUE;
    nw_bool dummy;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);
    NW_CONFIDENCE(data != NULL);
    NW_CONFIDENCE(length != NULL);
    NW_CONFIDENCE(sender != NULL);
    NW_CONFIDENCE(sizeof(os_sockaddr_storage) == sizeof(os_sockaddr_storage));
    NW_CONFIDENCE(!receiveChannel->inUse);

   receiveChannel->prs = prs;

    *data = NULL;
    *length = 0;

    NW_REPORT_INFO(5, "nw_plugReceiveChannelMessageStart");

    os_mutexLock(&receiveChannel->wakeupMtx);
    if (!receiveChannel->wakeupRequested) {
        os_mutexUnlock(&receiveChannel->wakeupMtx);
            /* Check if any message is waiting to be read.
             * This can be, for example, a message in a previously received
             * buffer. */
        messageWaiting = nw_plugReceiveChannelGetFragmentInstantly(
            receiveChannel, data, length, sender);
        os_mutexLock(&receiveChannel->wakeupMtx);
        do {
            os_mutexUnlock(&receiveChannel->wakeupMtx);
            /* First read from the socket in order to avoid rxbuffer overflow */
            nw_plugReceiveChannelProcessIncoming(channel);

            if (!messageWaiting) {
                /* First handle any waiting buffers */
                admin = nw_plugReceiveChannelTakeWaitingDataBuffer(receiveChannel);
                if (admin != NULL) {
                    buffer = DF_BUFFER(admin);

                    NW_STAMP(nw_plugDataBuffer(buffer),NW_BUF_TIMESTAMP_HANDLE);
                    sendingNodeId = nw_plugBufferGetSendingNodeId(buffer);
                    sendingPartitionIdHash = nw_plugDataBufferGetPartitionId(nw_plugDataBuffer(buffer));
                    partitionHashToId = nw_plugPartitionsGetPartitionIdByHash(nw_plugChannel(channel)->partitions,sendingPartitionIdHash);
                    reliable = (nw__plugChannelGetReliabilityOffered(channel) == NW_REL_RELIABLE);

                    nw_plugReceiveChannelInsertDataReceived(
                        receiveChannel, sendingNodeId, partitionHashToId,
                        nw_plugDataBuffer(buffer), reliable);
                    messageWaiting = nw_plugReceiveChannelGetFragmentInstantly(
                        receiveChannel, data, length, sender);

                    /* Do not release here, but keep the refcount to 1 */
                    /* nw_plugReceiveChannelReleaseBuffer(receiveChannel, buffer); */
                } else {
                    os_time timeout = {0, 200000000}; /* 200 milli seconds */
                    if (receiveChannel->backupActive) {
                        timeout.tv_nsec = 10000000; /* 10 millisec, to ensure regular mailbox check */
                    }
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
                            os_nanoSleep(timeout); /* Give network mainthread chance to process the callback */
                        }
                     os_mutexLock(&receiveChannel->wakeupMtx);
                         receiveChannel->wakeupRequested = TRUE;
                         os_mutexUnlock(&receiveChannel->wakeupMtx);
                     }
                }
            }
            NW_CONFIDENCE(messageWaiting == ((*data != NULL) && (*length != 0)));
        os_mutexLock(&receiveChannel->wakeupMtx);

        } while (!messageWaiting && !receiveChannel->wakeupRequested);
    }

    /* Always reset wakeup-flag */
    receiveChannel->wakeupRequested = FALSE;
    os_mutexUnlock(&receiveChannel->wakeupMtx);
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

    NW_STRUCT(nw_senderInfo) sender;
    memset(&sender, 0, sizeof(sender));

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);
    NW_CONFIDENCE(data != NULL);
    NW_CONFIDENCE(length != NULL);
    NW_CONFIDENCE(receiveChannel->inUse);

    /* We only arrive here if a new fragment is indeed available. */
    nw_plugReceiveChannelGetFragmentInstantly(receiveChannel, data, length, &sender);

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
    nw_plugChannel channel,
    plugReceiveStatistics prs)
{
    nw_plugReceiveChannel receiveChannel = (nw_plugReceiveChannel)channel;

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);
/*    NW_CONFIDENCE(receiveChannel->inUse);*/
    NW_CONFIDENCE(receiveChannel->prs == prs);
    if(receiveChannel->prs != NULL) {
        if (receiveChannel->prs->enabled) {
            receiveChannel->prs->nofUsedPacketBuffers = receiveChannel->nofUsedBuffers;
            receiveChannel->prs->nofFreePacketBuffers = receiveChannel->nofFreeBuffers;
        }
    }
    receiveChannel->inUse = FALSE;
    receiveChannel->prs = NULL;
}

void
nw_plugReceiveChannelWakeUp(
    nw_plugChannel channel)
{
    nw_plugReceiveChannel receiveChannel = nw_plugReceiveChannel(channel);

    NW_CONFIDENCE(channel != NULL);
    NW_CONFIDENCE(nw__plugChannelGetCommunication(channel) == NW_COMM_RECEIVE);

    os_mutexLock(&receiveChannel->wakeupMtx);
    receiveChannel->wakeupRequested = TRUE;
    os_mutexUnlock(&receiveChannel->wakeupMtx);
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


/* Interface */
#include "nw_channelUser.h"
#include "nw__channelUser.h"

/* Implementation */
#include "os_heap.h"
#include "v_entity.h"         /* for v_entity() */
#include "v_group.h"          /* for v_group() */
#include "v_topic.h"
#include "v_domain.h"
#include "v_networkReader.h"
#include "nw__confidence.h"
#include "nw_commonTypes.h"
#include "nw_report.h"

/* -------------------------------- AdminMessage ---------------------------- */

NW_CLASS(nw_adminMessage);

typedef enum nw_adminMessageKind_e {
    NW_MESSAGE_NEW_GROUP
} nw_adminMessageKind;

NW_STRUCT(nw_adminMessage) {
    nw_adminMessageKind kind;
    v_networkReaderEntry entry;
};


static nw_adminMessage
nw_adminMessageNew(
    nw_adminMessageKind kind,
    v_networkReaderEntry entry)
{
    nw_adminMessage result = NULL;
    
    result = (nw_adminMessage)os_malloc((os_uint32)sizeof(*result));
    
    if (result != NULL) {
        result->kind = kind;
        result->entry = entry; /* Intentionally no keep, we are in an action routine */
    }
    
    return result;
}


static void
nw_adminMessageFree(
    nw_adminMessage message)
{
    if (message) {
        os_free(message);
    }
}


/* --------------------------------- Ringbuffer ----------------------------- */

NW_STRUCT(nw_ringBuffer) {
    unsigned int nofEntries;
    nw_adminMessage *entries /* [nofEntries] */;
    unsigned int head;
    unsigned int tail;
};


#define NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, index) \
            (ringBuffer->entries[index])
#define NW_RINGBUFFER_INDEX_INC(ringBuffer, index)     \
        index++;                                       \
        if (index >= ringBuffer->nofEntries) {         \
            index = 0;                                 \
        }

/** \TODO Review the thread-safety of this class */                       

static void
nw_ringBufferSetEntryByIndex(
    nw_ringBuffer ringBuffer,
    nw_adminMessage message,
    unsigned int index)
{
    if (ringBuffer) {
        nw_adminMessageFree(NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, index));
        NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, index) = message;
    }
}


static nw_ringBuffer
nw_ringBufferNew(
    unsigned int nofEntries)
{
    nw_ringBuffer result = NULL;
    unsigned int i;
    
    result = (nw_ringBuffer)os_malloc((os_uint32)sizeof(*result));
    
    if (result != NULL) {
        result->entries = (nw_adminMessage *)os_malloc(
                              nofEntries * (os_uint32)sizeof(*result->entries));
        if (result->entries) {
            result->nofEntries = nofEntries;
            /* Initialize the buffer entries */
            for (i=0; i<nofEntries; i++) {
                NW_RINGBUFFER_ENTRY_BY_INDEX(result, i) = NULL;
            }
        } else {
            result->nofEntries = 0;
        }
        result->head = 0;
        result->tail = 0;
    }
    
    return result;
}


static void
nw_ringBufferFree(
    nw_ringBuffer ringBuffer)
{
    unsigned int i;
    
    if (ringBuffer) {
        if (ringBuffer->entries) {
            for (i=0; i<ringBuffer->nofEntries; i++) {
                nw_ringBufferSetEntryByIndex(ringBuffer, NULL, i);
            }
            os_free(ringBuffer->entries);
        }
        os_free(ringBuffer);
    }
}    


static void
nw_ringBufferPostMessage(
    nw_ringBuffer ringBuffer,
    nw_adminMessage message)
{
    if (ringBuffer) {
        nw_ringBufferSetEntryByIndex(ringBuffer, message, ringBuffer->head);
        NW_RINGBUFFER_INDEX_INC(ringBuffer, 
                                ringBuffer->head);
        if (ringBuffer->head == ringBuffer->tail) {           
           NW_REPORT_WARNING_2("posting administrative message", 
               "Administration buffer full, messages for partition "
               "%s of topic \"%s\" will be ignored",
               v_partitionName(v_groupPartition(message->entry->group)),
               v_topicName(v_groupTopic(message->entry->group)));
           NW_RINGBUFFER_INDEX_INC(ringBuffer,
               ringBuffer->tail);
        }
    }
}


static nw_adminMessage
nw_ringBufferProcessMessage(
    nw_ringBuffer ringBuffer)
{
    nw_adminMessage result = NULL;
    
    if (ringBuffer &&
        (ringBuffer->tail != ringBuffer->head)) {
        result = NW_RINGBUFFER_ENTRY_BY_INDEX(ringBuffer, ringBuffer->tail);
        NW_RINGBUFFER_INDEX_INC(ringBuffer,
                                ringBuffer->tail);
    }
    
    return result;
}    


/* --------------------------------- ChannelUser ---------------------------- */



/* Protected members */
void
nw_channelUserInitialize(
    nw_channelUser channelUser,
    const char *name,
    const char *pathName,
    u_networkReader reader,
    const nw_runnableMainFunc runnableMainFunc,
    const nw_runnableTriggerFunc runnableTriggerFunc,
    const nw_runnableFinalizeFunc runnableFinalizeFunc)
{
    c_ulong groupQueueSize;
    /* Initialize parent */
    nw_runnableInitialize((nw_runnable)channelUser, name, pathName,
                          runnableMainFunc, NULL, runnableTriggerFunc,
                          runnableFinalizeFunc);

    if (channelUser) {
        channelUser->reader = reader;
        groupQueueSize = NWCF_SIMPLE_PARAM(ULong, name, GroupQueueSize);
	    if (groupQueueSize < NWCF_MIN(GroupQueueSize)) {
	        NW_REPORT_WARNING_2("retrieving  channel parameters",
	            "specified GroupQueueSize %u too small, "
	            "switching to %u",
	            groupQueueSize, NWCF_MIN(GroupQueueSize));
	        groupQueueSize = NWCF_MIN(GroupQueueSize);
	    }
        channelUser->messageBuffer = nw_ringBufferNew(groupQueueSize);
    }
}
#undef NW_DEFAULT_RINGBUFFER_SIZE


c_bool
nw_channelUserRetrieveNewGroup(
    nw_channelUser channelUser,
    v_networkReaderEntry *entry)
{
    c_bool result = FALSE;
    nw_adminMessage message;
    
    if (channelUser) {
        message = nw_ringBufferProcessMessage(channelUser->messageBuffer);
        if (message) {
            result = TRUE;
            *entry = message->entry;
        }
    }
    
    return result;
}

/* Protected */
                
void
nw_channelUserFinalize(
    nw_channelUser channelUser)
{
    /* Finalize self */
    if (channelUser) {
        nw_ringBufferFree(channelUser->messageBuffer);

    /* Finalize parent */
    nw_runnableFinalize((nw_runnable)channelUser);
    
    }
}


/* Public members */

struct onNewGroupArg {
    nw_channelUser channelUser;
    v_networkReaderEntry entry;
};

static void
onNewGroup(
    v_entity e,
    c_voidp arg)
{
    v_networkReader reader;
    struct onNewGroupArg *onNewGroupArg;
    nw_channelUser channelUser;
    nw_adminMessage toPost;
    v_networkReaderEntry entry;
    
    reader = v_networkReader(e);
    NW_CONFIDENCE(reader);
    onNewGroupArg = (struct onNewGroupArg *)arg;
    entry = onNewGroupArg->entry;
    NW_CONFIDENCE(entry);
    channelUser = onNewGroupArg->channelUser;
    NW_CONFIDENCE(channelUser);
    
    toPost = nw_adminMessageNew(NW_MESSAGE_NEW_GROUP, entry);
    if (toPost) {
        /* Post the message in the buffer */
        nw_ringBufferPostMessage(channelUser->messageBuffer, toPost);
        /* Wake up the channelUser for processing this message */
        nw_runnableTrigger((nw_runnable)channelUser);
    }
}



void
nw_channelUserNotifyNewGroup(
    nw_channelUser channelUser,
    v_networkReaderEntry entry)
{
    struct onNewGroupArg onNewGroupArg;
    
    if (channelUser) {
        onNewGroupArg.entry = entry;
        onNewGroupArg.channelUser = channelUser;
        u_entityAction(u_entity(channelUser->reader), onNewGroup, &onNewGroupArg);
    }
}

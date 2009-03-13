/* Interface */
#include "nw_channelReader.h"

/* Implementation */
#include "os_heap.h"
#include "os_time.h"
#include "c_base.h"
#include "nw__confidence.h"
#include "nw__channelUser.h"
#include "nw__runnable.h"
#include "nw_writerSync.h"
#include "nw_writerAsync.h"
#include "kernelModule.h"
#include "v_entity.h"      /* for v_entity()     */
#include "v_group.h"       /* for v_groupWrite() */
#include "v_topic.h"
#include "v_domain.h"
#include "u_networkReader.h"
#include "v_networkReader.h"
#include "v_networkReaderEntry.h"
#include "nw_report.h"


/* ------------------------------- Private ---------------------------------- */

C_STRUCT(nw_channelReader){
    C_EXTENDS(nw_channelUser);
    /* Networking reader */
    u_networkReader reader;
    /* Networking channel */
    nw_receiveChannel receiveChannel;
    /* Network writer that writes into the kernel */
    nw_writer writer;
    /* Tracing parameter */
    c_ulong reportInterval;
};

typedef c_voidp nw_channelOnNewGroupArg;
typedef void (*nw_channelOnNewGroupAction)(
    v_networkReaderEntry entry,
    nw_channelOnNewGroupArg arg);

static c_bool
nw_channelReaderConnectToNewGroups(
    nw_channelReader channelReader,
    nw_channelOnNewGroupAction action,
    nw_channelOnNewGroupArg arg)
{
    c_bool result = FALSE;
    c_bool newGroupAvailable;
    v_networkReaderEntry entry;

    /* Check if any new groups need to be added */
    newGroupAvailable = nw_channelUserRetrieveNewGroup(
        (nw_channelUser)channelReader, &entry);

    while (newGroupAvailable) {

        result = TRUE;
        /* Create a new channel on the network */
        nw_receiveChannelAddGroup(channelReader->receiveChannel, entry);
        /* Call the action routine if needed */
        if (action != NULL) {
        	action(entry, arg);
        }
        /* Any more new groups? */
        newGroupAvailable = nw_channelUserRetrieveNewGroup(
            (nw_channelUser)channelReader, &entry);
    }

    return result;
}

NW_STRUCT(nw_newEntryArg) {
	v_networkHashValue hashValue;
	const char *partitionName;
	const char *topicName;
	v_networkReaderEntry entryFound;
};
NW_CLASS(nw_newEntryArg);

static void
onNewGroupAction (
    v_networkReaderEntry entry,
    nw_channelOnNewGroupArg arg)
{
	nw_newEntryArg newEntryArg = (nw_newEntryArg)arg;

	NW_CONFIDENCE(newEntryArg != NULL);

	if ((entry->hashValue.h1 == newEntryArg->hashValue.h1) &&
	    (entry->hashValue.h2 == newEntryArg->hashValue.h2) &&
	    (entry->hashValue.h3 == newEntryArg->hashValue.h3) &&
	    (entry->hashValue.h4 == newEntryArg->hashValue.h4) &&
	    (strcmp(v_partitionName(v_groupPartition(entry->group)),
	           newEntryArg->partitionName) == 0) &&
	    (strcmp(v_topicName(v_groupTopic(entry->group)),
	           newEntryArg->topicName) == 0)) {
        NW_CONFIDENCE(newEntryArg->entryFound == NULL);
	    newEntryArg->entryFound = entry;
	}
}

static v_networkReaderEntry
onLookupEntryAction(
    v_networkHashValue hashValue,
    const char *partitionName,
    const char *topicName,
    nw_entryLookupArg arg)
{
	v_networkReaderEntry result;
    nw_channelReader channelReader;
    NW_STRUCT(nw_newEntryArg) newEntryArg;

    channelReader = (nw_channelReader)arg;
    newEntryArg.hashValue = hashValue;
    newEntryArg.partitionName = partitionName;
    newEntryArg.topicName = topicName;
    newEntryArg.entryFound = NULL;

    nw_channelReaderConnectToNewGroups(channelReader, onNewGroupAction, &newEntryArg);
    result = newEntryArg.entryFound;

    return result;
}


static void
nw_channelReaderMain(
    v_entity e,
    c_voidp arg)
{
    v_networkReader reader;
    nw_channelReader channelReader;
    v_networkReaderEntry entry = NULL;
    v_message message = NULL;
    struct nw_endpointInfo endpointInfo = {0, {0,0,0}, FALSE, {0,0,0}}; /*p2p preparation*/
    c_ulong messagesReceived;
    c_ulong messagesReceivedReport;

    reader = v_networkReader(e);
    channelReader = (nw_channelReader)arg;
    messagesReceived = 0;
    messagesReceivedReport = 0;

    while (!(int)nw_runnableTerminationRequested((nw_runnable)channelReader)) {

        /* Read messages from the network */
        nw_receiveChannelRead(channelReader->receiveChannel,
                              &message,
                              &entry,
                              onLookupEntryAction,
                              channelReader);
        if (entry != NULL) {
#define NW_IS_BUILTIN_DOMAINNAME(name) ((int)(name)[0] == (int)'_')

            /* Do not trace for internal partitions */
            if (!NW_IS_BUILTIN_DOMAINNAME(
                v_partitionName(v_groupPartition(entry->group)))) {
#undef NW_IS_BUILTIN_DOMAINNAME
                messagesReceived++;
                messagesReceivedReport++;
                if (messagesReceivedReport == channelReader->reportInterval) {
                    NW_TRACE_2(Receive, 3, "Channel %s: %u messages "
                        "taken from network and written to queue",
                        ((nw_runnable)channelReader)->name,
                        messagesReceived);
                    messagesReceivedReport = 0;
                }
                NW_TRACE_3(Receive, 4,
                    "Channel %s: data message taken from network and "
                    "written to queue (partition = %s, topic = %s)",
                    ((nw_runnable)channelReader)->name,
                    v_partitionName(v_groupPartition(entry->group)),
                    v_topicName(v_groupTopic(entry->group)));
            }

            /* Send the message to the network writer */
            if ( nw_writerWriteMessage(channelReader->writer,
                    entry, message,
                    endpointInfo.messageId, endpointInfo.sender,
                    endpointInfo.sendTo, endpointInfo.receiver) )
            {
                c_free(message);
                message = NULL;
                entry = NULL;
            }
        }
    }
    NW_TRACE_2(Receive, 2, "Channel %s: %d messages taken from network and "
        "written to queue", ((nw_runnable)channelReader)->name, messagesReceived);
}


static void *
nw_channelReaderMainFunc(
    nw_runnable runnable,
    c_voidp arg)
{
    u_result result;
    nw_channelReader channelReader = (nw_channelReader)runnable;

    nw_runnableSetRunState(runnable, rsRunning);
    result = u_entityAction(u_entity(channelReader->reader),
                            nw_channelReaderMain, channelReader);
    nw_runnableSetRunState(runnable, rsTerminated);

    return NULL;
}


static void
nw_channelReaderTrigger(
    nw_runnable runnable)
{
    nw_channelReader channelReader = (nw_channelReader)runnable;
    if (channelReader != NULL) {
        nw_receiveChannelTrigger(channelReader->receiveChannel);
    }
}


static void
nw_channelReaderFinalize(
    nw_runnable runnable)
{
    nw_channelReader channelReader = (nw_channelReader)runnable;
    /* Finalize self */
    /* Finalize members of self */
    nw_writerFree(channelReader->writer);
    nw_receiveChannelFree(channelReader->receiveChannel);

    /* Finalize parent */
    nw_channelUserFinalize((nw_channelUser)channelReader);
}


/* ------------------------------- Public ----------------------------------- */

nw_channelReader
nw_channelReaderNew(
    const char *pathName,
    nw_receiveChannel receiveChannel,
    u_networkReader reader)
{
    nw_channelReader result = NULL;
    char *tmpPath;
    size_t tmpPathSize;
    nw_bool doSMPOptimization;

    result = (nw_channelReader)os_malloc((os_uint32)sizeof(*result));

    if (result != NULL) {
        /* Initialize parent */
        /* First determine its parameter path */
        tmpPathSize = strlen(pathName) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(Rx)) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(Scheduling)) + 1 /* '\0' */;
        tmpPath = os_malloc(tmpPathSize);
        sprintf(tmpPath, "%s%s%s%s%s", pathName, NWCF_SEP, NWCF_ROOT(Rx),
            NWCF_SEP, NWCF_ROOT(Scheduling));

        nw_channelUserInitialize((nw_channelUser)result,
            pathName /* use pathName as name */, tmpPath, reader,
            nw_channelReaderMainFunc, nw_channelReaderTrigger,
            nw_channelReaderFinalize);
        os_free(tmpPath);

        /* Initialize myself */
        tmpPathSize =  strlen(pathName) + strlen(NWCF_SEP) + strlen(NWCF_ROOT(Tx)) + 1;
        tmpPath = os_malloc(tmpPathSize);
        sprintf(tmpPath, "%s%s%s", pathName, NWCF_SEP, NWCF_ROOT(Tx));
        result->reportInterval = NWCF_SIMPLE_PARAM(ULong, tmpPath, ReportInterval);
        if (result->reportInterval < NWCF_MIN(ReportInterval)) {
            NW_REPORT_WARNING_3("initializing network",
                "Requested value %d for sending report interval for channel \"%s\" is "
                "too small, using %d instead",
                result->reportInterval, pathName, NWCF_MIN(ReportInterval));
            result->reportInterval = NWCF_MIN(ReportInterval);
        }
        os_free(tmpPath);

        tmpPathSize =  strlen(pathName) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(Rx)) + strlen(NWCF_SEP) +
            strlen(NWCF_ROOT(SMPOptimization)) + 1;
        tmpPath = os_malloc(tmpPathSize);
        sprintf(tmpPath, "%s%s%s%s%s", pathName, NWCF_SEP, NWCF_ROOT(Rx), NWCF_SEP, NWCF_ROOT(SMPOptimization));
        /* Check for the node on SMPOptimization, default to enabled = FALSE if there is no
         * such node, enable = TRUE if the  node is available but does not have the
         * enabled attribute */
        doSMPOptimization = NWCF_DEFAULTED_ATTRIB(Bool, tmpPath, enabled, FALSE, TRUE);

        if (doSMPOptimization) {
            result->writer = nw_writerAsyncNew(pathName, tmpPath);
        } else {
            result->writer = nw_writerSyncNew();
        }
        os_free(tmpPath);

        /* Store the channel to read from */
        result->receiveChannel = receiveChannel;
        /* Set the networkReader */
        result->reader = reader;
        /* Do not start in the constructor, but let somebody else start me */
    }

    return result;
}



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
#include "nw_channelWriter.h"

/* Implementation */
#include "os_stdlib.h"
#include "os_heap.h"
#include "os_time.h"
#include "nw__channelUser.h"
#include "nw__runnable.h"
#include "kernelModule.h"
#include "v_entity.h" /* for casting */
#include "v_entry.h" /* for casting */
#include "v_group.h"
#include "v_topic.h"
#include "v_partition.h"
#include "v_networkQueue.h"
#include "v_networkReader.h"
#include "v_networkReaderEntry.h"
#include "u_networkReader.h"
#include "nw_configuration.h"
#include "nw_report.h"
#include "nw__confidence.h"
#include "v_networking.h"
#include "v_subscriber.h"
#include "v_networkingStatistics.h"
#include "nw_statistics.h"
#include "nw_plugSendChannel.h"
#include "nw_security.h"
#include "nw_statistics.h"

#ifdef NW_LOOPBACK
#include "v_group.h"/* for casting */
#endif

/* ----------------------------------- Private ------------------------------ */

/**
* @extends nw_channelUser_s
*/
NW_STRUCT(nw_channelWriter){
    NW_EXTENDS(nw_channelUser);
    /* Networking channel to write to */
    nw_sendChannel sendChannel;
    /* Id of networking queue to read from */
    c_ulong queueId;
    /* Tracing parameter */
    c_ulong reportInterval; /* In messages */
    c_char* serviceName;
    c_ulong stat_channel_id;
    nw_SendChannelStatistics scs;
};


#define NW_READER_ID_IS_VALID(channelReader, id) \
        (id < channelReader->nofGroups)

#define NW_READER_BY_GROUP_ID(channelReader, id) \
        (channelReader->proxyReaders[id])

void nw_updatePlugSendStatistics(plugSendStatistics pss, nw_channelWriter channelWriter) {
	channelWriter->scs->numberOfBytesInResendBuffer = pss->numberOfBytesInResendBuffer;
	channelWriter->scs->numberOfBytesResent = pss->numberOfBytesResent;
	channelWriter->scs->numberOfKnownNodes = pss->numberOfKnownNodes;
	channelWriter->scs->numberOfMessagesFragmented = pss->numberOfMessagesFragmented;
	channelWriter->scs->numberOfMessagesPacked = pss->numberOfMessagesPacked;
	channelWriter->scs->numberOfPacketsInResendBuffer = pss->numberOfPacketsInResendBuffer;
	channelWriter->scs->numberOfPacketsResent = pss->numberOfPacketsResent;
	channelWriter->scs->numberOfPacketsSent = pss->numberOfPacketsSent;
	channelWriter->scs->numberOfBytesSent = pss->numberOfBytesSent;
	channelWriter->scs->numberOfAcksSent = pss->numberOfAcksSent;
	channelWriter->scs->adminQueueAcks = pss->adminQueueAcks;
	channelWriter->scs->nofBytesBeforeCompression = pss->nofBytesBeforeCompression;
	channelWriter->scs->nofBytesAfterCompression = pss->nofBytesAfterCompression;
}


static void
nw_channelWriterMain(
    v_entity e,
    c_voidp arg)
{
    v_networkReader reader;
    nw_channelWriter channelWriter;
    c_bool newGroupAvailable;
    v_networkQueue qosQueue;

    v_networkReaderWaitResult waitResult;
    c_long bytesWritten;
    /* Total number of messages sent to the network */
    c_ulong writtenCountMessages;
    /* Total number of bytes sent to the network */
    c_ulong writtenCountBytes;
    /* Total number of bytes sent to the network during one burst */
    c_ulong writtenCountBytesThisBurst;
    /* Number of message sent to the network after last report */
    c_ulong writtenCountMessagesReport;

    v_message message;
    v_networkReaderEntry entry;
    c_ulong sequenceNumber;
    v_gid sender;
    c_bool sendTo;
    v_gid receiver;
    c_time sendBefore;
    c_ulong priority;
    c_bool more = TRUE;
    c_bool slowingDown;
    c_ulong timeoutCount;
    nw_signedLength credits;
    v_networking n;
    NW_STRUCT(plugSendStatistics) pss = {0,0,0,0,0,0,0,0,0,0,
                                         {0,
                                          {{0,0},
                                           0},
                                          {{0,0},
                                           0},
                                          {0.0,0}},
                                         {0,
                                          {{0,0},
                                           0},
                                          {{0,0},
                                           0},
                                          {0.0,0}},
                                         0,0,0};
    v_fullCounterInit(&(pss.adminQueueAcks));
    v_fullCounterInit(&(pss.adminQueueData));

    reader = v_networkReader(e);
    channelWriter = (nw_channelWriter)arg;


    /* This line is needed as long as the discovery channel is not yet implemented */
    writtenCountBytesThisBurst = 0;
    writtenCountMessages = 0;
    writtenCountBytes = 0;
    writtenCountMessagesReport = 0;
    slowingDown = FALSE;
    timeoutCount = 0;
    credits = 0;

    while (!(int)nw_runnableTerminationRequested((nw_runnable)channelWriter)) {

        /* Wait for data on the reader */
        if (!slowingDown) {
            waitResult = v_networkReaderWait(reader,
                                             channelWriter->queueId,
                                             &qosQueue);
        } else {
            waitResult = v_networkReaderWaitDelayed(reader,
                 channelWriter->queueId, &qosQueue);

            NW_CONFIDENCE(waitResult & (V_WAITRESULT_TIMEOUT | V_WAITRESULT_TRIGGERED));
        }

        if ((waitResult & V_WAITRESULT_TRIGGERED) &&
            (!nw_runnableTerminationRequested((nw_runnable)channelWriter))) {
            /* If necessary, check if any new groups need to be added */
            newGroupAvailable = nw_channelUserRetrieveNewGroup(
                (nw_channelUser)channelWriter, &entry);

            while (newGroupAvailable) {
                /* Create a new channel on the network */
                /* No, do not call any function. With the new design,
                 * a channelWriter does not need to know anything about this
                 * new group. Maybe at a later stage. */
                /* nw_channelAddGroup((nw_channel)channelWriter->sendChannel, entry); */
                /* And notify we are connected */
                v_networkReaderEntryNotifyConnected(entry,channelWriter->serviceName);

                newGroupAvailable = nw_channelUserRetrieveNewGroup(
                     (nw_channelUser)channelWriter, &entry);
            }
        }

        /* Resend data should also obey max_burst_size
         * Each clocktick, the remaining credits of the last period and a
         * third of the new max_burst_size budget may be used for resend data.
         * The rest of the budget is assigned after that, and can be used to
         * flush stored buffers of send fresh data.
         */
        if (waitResult & V_WAITRESULT_TIMEOUT) {

            /*
             * The periodic action is needed for every clocktick.
             * This will also update the credits, for the amount of bandwidth
             * available in the coming period.
             */
            /*stat update routine */
            n = v_networking(v_subscriber(v_reader(reader)->subscriber)->participant);
            /* update statistics */
            if (v_entity(n)->statistics) {
                if (!pss.enabled) {
                    pss.enabled =1;
                }
                /* sync plug stats */
                nw_updatePlugSendStatistics(&pss,channelWriter);
                nw_SendChannelUpdate(v_networkingStatistics(v_entity(n)->statistics)->channels[channelWriter->stat_channel_id],channelWriter->scs);
            }

            nw_sendChannelPeriodicAction(channelWriter->sendChannel,&credits,&pss); /*struc call*/
            /* A flush is needed if we are slowing down. */
            if (slowingDown) {
                /* The return value is true is all data has been sent.
                 * Afterwards, credits will contain the new amount of allowed
                 * bytes.
                 * We are using a special flush function here that flushes full
                 * buffers only */
                slowingDown = !nw_sendChannelFlush(channelWriter->sendChannel,
                                                   FALSE, &credits, &pss);
            }
        }
        if ((waitResult & V_WAITRESULT_MSGWAITING) && !slowingDown) {
            /* Messages are waiting... */
            writtenCountBytesThisBurst = 0;
            more= TRUE;
            while (more &&
                  ((nw_signedLength)writtenCountBytesThisBurst <= credits))
            {
                /* Take any new messages */
                v_networkQueueTakeFirst(qosQueue, &message, &entry,
                    &sequenceNumber, &sender, &sendTo, &receiver,
                    &sendBefore, &priority, &more);
                NW_CONFIDENCE(message != NULL);
                NW_CONFIDENCE(entry != NULL);


		if (!(NW_SECURITY_CHECK_FOR_PUBLISH_PERMISSION_OF_SENDER_ON_SENDER_SIDE(entry))) {
				bytesWritten = 0; /* indicates that nothing has been written */

				NW_REPORT_WARNING_2(
						"nw_channelWriterMain",
						"Channel \"%s\" could not deliver message 0x%x : message dropped!",
						((nw_runnable)channelWriter)->name,
						message);
		} else {
				bytesWritten = nw_sendChannelWrite(channelWriter->sendChannel,
                                                   entry, message, &credits ,&pss); /* stat struc plug vars */
                if (bytesWritten == 0) {
                    NW_REPORT_WARNING_2(
                        "nw_channelWriterMain",
                        "Channel \"%s\" could not deliver message 0x%x : message dropped!",
                         ((nw_runnable)channelWriter)->name,
                         message);
                }
                /*numberOfMessagesSent stats*/
                if (pss.enabled) {
                    channelWriter->scs->numberOfMessagesSent++;
                }
                assert( bytesWritten > 0); /* if permission grantedm the value must be greater 0 */

                }

                writtenCountBytesThisBurst += bytesWritten;

#define NW_IS_BUILTIN_DOMAINNAME(name) ((int)(name)[0] == (int)'_')

                /* Do not trace for internal partitions */
                if (bytesWritten>0 && /* might be 0 if access control refuses write permission */
                	!NW_IS_BUILTIN_DOMAINNAME(
                    v_partitionName(v_groupPartition(entry->group)))) {
#undef NW_IS_BUILTIN_DOMAINNAME
                    writtenCountBytes += bytesWritten;
                    writtenCountMessages++;
                    writtenCountMessagesReport++;
                    if (writtenCountMessagesReport == channelWriter->reportInterval) {
                        NW_TRACE_3(Send, 3,
                            "Channel %s: %u messages (%u bytes) "
                            "taken from queue and written to network",
                            ((nw_runnable)channelWriter)->name,
                            writtenCountMessages, writtenCountBytes);
                        writtenCountMessagesReport = 0;
                    }
                    NW_TRACE_3(Send, 4,
                        "Channel %s: data message taken from queue, "
                        "and written to network (partition = %s, topic = %s)",
                        ((nw_runnable)channelWriter)->name,
                        v_partitionName(v_groupPartition(entry->group)),
                        v_topicName(v_groupTopic(entry->group)));
                }
                c_free(message);
                c_free(entry);
            }
            slowingDown = !nw_sendChannelFlush(channelWriter->sendChannel,
                                               TRUE, &credits, &pss);
        }
    }
    NW_TRACE_3(Send, 2,
               "Channel %s: %u messages (%u bytes) taken from queue and "
               "written to network", ((nw_runnable)channelWriter)->name,
                writtenCountMessages, writtenCountBytes);
}

static void *
nw_channelWriterMainFunc(
    nw_runnable runnable,
    c_voidp arg)
{
    u_result result;
    nw_channelWriter channelWriter = (nw_channelWriter)runnable;

    nw_runnableSetRunState(runnable, rsRunning);
    result = u_entityAction(u_entity(((nw_channelUser)channelWriter)->reader),
                            nw_channelWriterMain,channelWriter);
    nw_runnableSetRunState(runnable, rsTerminated);

    return NULL;
}


static void
nw_channelWriterTrigger(
    nw_runnable runnable)
{
    nw_channelWriter channelWriter = (nw_channelWriter)runnable;
    /* Trigger if in the wait */
    u_networkReaderTrigger(((nw_channelUser)channelWriter)->reader,
                           channelWriter->queueId);
}


static void
nw_channelWriterFinalize(
    nw_runnable runnable)
{
    nw_channelWriter channelWriter = (nw_channelWriter)runnable;
    /* Finalize self */
    nw_sendChannelFree(channelWriter->sendChannel);

    /* Finalize parent */
    nw_channelUserFinalize((nw_channelUser)channelWriter);
}


/* ----------------------------------- Public ------------------------------- */


nw_channelWriter
nw_channelWriterNew(
    const char *serviceName,
    const char *pathName,
    nw_sendChannel sendChannel,
    u_networkReader reader,
    c_ulong stat_channel_id)
{
    nw_channelWriter result = NULL;
    c_ulong queueSize;
    c_ulong priority;
    c_bool reliable;
    c_bool useAsDefault;
    static c_bool defaultDefined = FALSE;
    u_result ures;
    c_ulong resolutionMsecs;
    c_time resolution;
    char *tmpPath;
    size_t tmpPathSize;
    char* name;

    result = (nw_channelWriter)os_malloc((os_uint32)sizeof(*result));

    if (result != NULL) {
        /* Initialize parent */
        /* First determine its parameter path */
        tmpPathSize = strlen(pathName) + strlen(NWCF_SEP) +
                      strlen(NWCF_ROOT(Tx)) + strlen(NWCF_SEP) +
                      strlen(NWCF_ROOT(Scheduling)) + 1 /* '\0' */;
        tmpPath = os_malloc(tmpPathSize);
        os_sprintf(tmpPath, "%s%s%s%s%s", pathName, NWCF_SEP, NWCF_ROOT(Tx),
                                       NWCF_SEP, NWCF_ROOT(Scheduling));
        nw_channelUserInitialize((nw_channelUser)result,
            pathName /* use pathName as name */, tmpPath, reader,
            nw_channelWriterMainFunc, nw_channelWriterTrigger,
            nw_channelWriterFinalize);
        os_free(tmpPath);

        /* Own initialization */
        result->serviceName = os_strdup(serviceName);
        /* Store the channel to write to */
        result->sendChannel = sendChannel;
        /* Create a new networking queue */
        /* First read the corresponding options */
        queueSize = NWCF_SIMPLE_SUBPARAM(ULong, pathName, Tx, QueueSize);
        priority = NWCF_SIMPLE_ATTRIB(ULong, pathName, priority);
        reliable = NWCF_SIMPLE_ATTRIB(Bool, pathName, reliable);
        useAsDefault = NWCF_SIMPLE_ATTRIB(Bool, pathName, default);
        name = NWCF_DEFAULTED_ATTRIB(String, pathName, ChannelName,"unnamed","unnamed");
        if (useAsDefault) {
            if (defaultDefined) {
                NW_REPORT_WARNING_1(
                    "initializing network",
                    "default channel redefined by channel \"%s\"",
                     pathName);
                useAsDefault = FALSE;
            } else {
                defaultDefined = TRUE;
            }
        }
        result->scs = nw_SendChannelStatisticsNew();
        result->stat_channel_id = stat_channel_id;
        resolutionMsecs = NWCF_SIMPLE_PARAM(ULong, pathName, Resolution);
        if (resolutionMsecs < NWCF_MIN(Resolution)) {
            NW_REPORT_WARNING_3(
                "initializing network",
                "Requested value %d for resolution period for channel \"%s\" is "
                "too small, using %d instead",
                resolutionMsecs, pathName, NWCF_MIN(Resolution));
            resolutionMsecs = NWCF_MIN(Resolution);
        }
        resolution.seconds = resolutionMsecs/1000;
        resolution.nanoseconds = 1000000*(resolutionMsecs % 1000);

        tmpPathSize = strlen(pathName) +
                      strlen(NWCF_SEP) +
                      strlen(NWCF_ROOT(Tx)) + 1;

        tmpPath = os_malloc(tmpPathSize);
        os_sprintf(tmpPath, "%s%s%s", pathName, NWCF_SEP, NWCF_ROOT(Tx));

        result->reportInterval = NWCF_SIMPLE_PARAM(ULong, tmpPath, ReportInterval);

        if (result->reportInterval < NWCF_MIN(ReportInterval)) {
            NW_REPORT_WARNING_3(
                "initializing network",
                "Requested report interval value %d for channel \"%s\" is "
                "too small, using %d instead",
                result->reportInterval, pathName, NWCF_MIN(ReportInterval));
            result->reportInterval = NWCF_MIN(ReportInterval);
        }
        os_free(tmpPath);

        ures = u_networkReaderCreateQueue(reader,
                                          queueSize,
                                          priority,
                                          reliable,
                                          FALSE,
                                          resolution,
                                          useAsDefault,
                                          &result->queueId,
                                          name);
        os_free(name);
        if (ures != U_RESULT_OK) {
            NW_REPORT_ERROR_1(
                "initializing network",
                "creation of network queue failed for channel \"%s\"",
                 pathName);
        }

    }

    return result;
}


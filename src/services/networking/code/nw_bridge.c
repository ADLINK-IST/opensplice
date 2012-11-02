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
#include "nw_bridge.h"

/* implementation */
#include <ctype.h> /* isupper, islower */
#include "os_heap.h"
#include "os_time.h"
#include "c_serialize.h"
#include "nw__confidence.h"
#include "nw__channel.h"
#include "nw_commonTypes.h"
#include "nw_plugTypes.h"
#include "nw_plugNetwork.h"
#include "nw_plugSendChannel.h"
#include "nw_plugReceiveChannel.h"
#include "nw__plugDataBuffer.h"
#include "v_entity.h" /* for v_entity() */
#include "v_group.h"  /* for v_group() */
#include "v_public.h" /* for gid stuff */
#include "v_message.h" /* for allocTime */
#include "v_networkReaderEntry.h" /* for v_networkReaderEntry() */
#include "nw_configuration.h"
#include "nw_report.h"
#include "nw_profiling.h"
#include "nw_stream.h"
#include "nw_security.h"


/* Bridge class definition */

#define NW_CHANNEL_ID_IS_VALID(bridge,id) \
        (id < bridge->nofChannels)

#define NW_STREAM_BY_ID(bridge,id) \
        bridge->streams[id]

#define NW_CHANNEL_BY_ID(bridge,id) \
        nw_stream_channel(NW_STREAM_BY_ID(bridge,id))

NW_STRUCT(nw_bridge) {
    nw_plugNetwork plugNetwork;
    nw_seqNr nofChannels;
    nw_stream *streams;
};

/* Bridge class operations */

/* Helpers */

nw_globalId
v_gidToGlobalId(
    v_gid gid)
{
    nw_globalId result;

    result.nodeId = v_gidSystemId(gid);
    result.localId1 = v_gidLocalId(gid);
    result.localId2 = v_gidLifecycleId(gid);

    return result;
}

v_gid
v_gidFromGlobalId(
    nw_globalId globalId)
{
    v_gid result;

    result.systemId = globalId.nodeId;
    result.localId = globalId.localId1;
    result.serial = globalId.localId2;

    return result;
}

/* --------------------------------- Public --------------------------------- */

nw_bridge
nw_bridgeNew(
    v_networkId nodeId)
{
    nw_bridge result;
    nw_seqNr i;

    result = (nw_bridge)os_malloc((os_uint32)sizeof(*result));

    if (result) {
        result->plugNetwork = nw_plugNetworkIncarnate((nw_networkId)nodeId);
        result->nofChannels = nw_plugNetworkGetMaxChannelId(result->plugNetwork);
        result->streams = (nw_stream *)os_malloc(
            result->nofChannels * (os_uint32)sizeof(*result->streams));
        NW_CONFIDENCE(result->streams);
        if (result->streams) {
            for (i=0; i<result->nofChannels; i++) {
                NW_STREAM_BY_ID(result, i) = NULL;
            }
        } else {
            result->nofChannels = 0;
        }
    }

    return result;
}


void
nw_bridgeFree(
    nw_bridge bridge)
{
    if (bridge != NULL) {

        /* Release the plug */
        nw_plugNetworkExcarnate(bridge->plugNetwork);

        /* Release allocated mem */
        os_free(bridge->streams);
        os_free(bridge);
    }
}

typedef struct nw_outOfBuffersArg_s {
    nw_plugChannel plugChannel;
    c_ulong byteCount;
} *nw_outOfBuffersArg;

static void
onSerializerOutOfBuffers(
    c_octet **buffer,
    c_ulong *length,
    c_serializeActionArg arg)
{
    nw_data *networkBuffer = (nw_data *)buffer;
    nw_length *networkBufferLength = (nw_length *)length;
    nw_outOfBuffersArg outOfBuffersArg = (nw_outOfBuffersArg)arg;
    nw_plugChannel plugChannel = outOfBuffersArg->plugChannel;

    NW_CONFIDENCE(networkBuffer != NULL);
    NW_CONFIDENCE(networkBufferLength != NULL);
    NW_CONFIDENCE(plugChannel != NULL);

    outOfBuffersArg->byteCount -= *networkBufferLength;
    nw_plugSendChannelGetNextFragment(plugChannel, networkBuffer, networkBufferLength);
    outOfBuffersArg->byteCount += *networkBufferLength;
}

static void
writeStringToPlugChannel(
    nw_plugChannel plugChannel,
    nw_data *networkBuffer,
    nw_length *networkBufferLength,
    const char *string)
{
    const char *srcPtr;
    char *dstPtr;
    c_bool done = FALSE;

    srcPtr = string;
    dstPtr = (char *)(*networkBuffer);

    while (!done) {
        if (*networkBufferLength == 0) {
            *networkBuffer = (nw_data)dstPtr;
            nw_plugSendChannelGetNextFragment(plugChannel, networkBuffer,
                networkBufferLength);
            NW_CONFIDENCE(*networkBuffer != NULL);
            NW_CONFIDENCE(*networkBufferLength > 0);
            dstPtr = (char *)(*networkBuffer);
        }
        *dstPtr = *srcPtr;
        done = (*srcPtr == '\0');
        srcPtr = &(srcPtr[1]);
        dstPtr = &(dstPtr[1]);
        (*networkBufferLength)--;
    }
    *networkBuffer = (nw_data)dstPtr;
}

void
nw_bridgePeriodicAction(
    nw_bridge bridge,
    nw_seqNr channelId,
    nw_signedLength *bytesLeft,
    plugSendStatistics pss)
{
    nw_plugChannel plugChannel;

    NW_CONFIDENCE(bridge != NULL);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

    if ((bridge != NULL) && NW_CHANNEL_ID_IS_VALID(bridge, channelId)) {

        /* Find the channel to read from */
        plugChannel = NW_CHANNEL_BY_ID(bridge, channelId);
        NW_CONFIDENCE(plugChannel != NULL);

        nw_plugSendChannelPeriodicAction(plugChannel,bytesLeft, pss);
    }
}


c_ulong
nw_bridgeWrite(
    nw_bridge bridge,
    nw_seqNr channelId,
    v_networkPartitionId partitionId,
    v_message message,
    v_networkHashValue hashValue,
    const c_char *partitionName,
    const c_char *topicName,
    nw_signedLength *bytesLeft,
    plugSendStatistics pss)
{
    c_ulong result = 0;
    nw_bool valid = TRUE;
    nw_stream stream;

    NW_CONFIDENCE(bridge != NULL);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

    stream = NW_STREAM_BY_ID(bridge, channelId);

    valid = nw_stream_writeBegin(stream, partitionId, bytesLeft, pss);
    if (valid) {
        result  = nw_stream_writeOpaq(stream,sizeof(hashValue),(c_voidp)&hashValue);
        result += nw_stream_writeString(stream,NULL,(c_voidp)&partitionName);
        result += nw_stream_writeString(stream,NULL,(c_voidp)&topicName);
        result += nw_stream_write(stream,message);
        nw_stream_writeEnd(stream, pss);
    }
    return result;
}

nw_bool
nw_bridgeFlush(
    nw_bridge bridge,
    nw_seqNr channelId,
    nw_bool all,
    nw_signedLength *bytesLeft,
    plugSendStatistics pss)
{
    nw_bool result = TRUE;
    nw_plugChannel plugChannel;

#ifdef NW_DEBUGGING
    if (!nw_configurationNoPacking()) {
#endif

    NW_CONFIDENCE(bridge != NULL);

    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

    plugChannel = NW_CHANNEL_BY_ID(bridge, channelId);
    NW_CONFIDENCE(plugChannel != NULL);

    result = nw_plugSendChannelMessagesFlush(plugChannel, all, bytesLeft, pss);

#ifdef NW_DEBUGGING
    }
#endif
    return result;
}


static void
onDeserializerOutOfBuffers(
    c_octet **buffer,
    c_ulong *length,
    c_serializeActionArg arg)
{
    nw_data *networkBuffer = (nw_data *)buffer;
    nw_length *networkBufferLength = (nw_length *)length;
    nw_plugChannel plugChannel = (nw_plugChannel)arg;

    NW_CONFIDENCE(networkBuffer != NULL);
    NW_CONFIDENCE(networkBufferLength != NULL);
    NW_CONFIDENCE(plugChannel != NULL);

    nw_plugReceiveChannelGetNextFragment(plugChannel,
        networkBuffer, networkBufferLength);
}

static c_char *
getStringFromPlugChannel(
    nw_plugChannel plugChannel,
    nw_data *networkBuffer,
    nw_length *networkBufferLength)
{
    const char *copyPtr;
    nw_length len;
    nw_length saveLen;
    const char *stringSegStart;
    char *saveString = NULL;
    char *newString = NULL;
    c_bool copyNeeded = FALSE;
    c_bool done = FALSE;

    copyPtr = (const char *)(*networkBuffer);

    stringSegStart = copyPtr;
    saveString = NULL;
    saveLen = 0;
    len = 0;

    do {
        if (*networkBufferLength != 0) {
            if (*copyPtr == '\0') {
                /* End of the string reached */
                copyNeeded = TRUE;
                done = TRUE;
            }
            len++;
            (*networkBufferLength)--;
            copyPtr = &(copyPtr[1]);
        } else {
            /* End of this fragment reached */
            copyNeeded = TRUE;
        }
        if (copyNeeded) {
            if (len > 0) {
                newString = os_malloc(len);
                NW_CONFIDENCE(len > saveLen);
                NW_CONFIDENCE((saveLen == 0) == (saveString == NULL));
                if (saveString != NULL) {
                    memcpy(newString, saveString, saveLen);
                    os_free(saveString);
                }
                memcpy(&(newString[saveLen]), stringSegStart, len-saveLen);
                saveString = newString;
                saveLen = len;
            }
            if (!done) {
                /* Not finished yet, need to retrieve more fragments */
                *networkBuffer = (nw_data)copyPtr;
                nw_plugReceiveChannelGetNextFragment(plugChannel,
                    networkBuffer, networkBufferLength);
                NW_CONFIDENCE(*networkBuffer != NULL);
                NW_CONFIDENCE(*networkBufferLength > 0);
                copyPtr = (const char *)(*networkBuffer);
                stringSegStart = copyPtr;
            }
            copyNeeded = FALSE;
        }
    } while (!done);

    *networkBuffer = (nw_data)copyPtr;

    return newString;
}

/* message is an IN/OUT param, if it still contains a message, no new message should be created */
void
nw_bridgeRead(
    nw_bridge bridge,
    nw_seqNr channelId,
    v_message *message,
    const nw_typeLookupAction typeLookupAction,
    nw_typeLookupArg typeLookupArg,
    plugReceiveStatistics prs)
{
    nw_plugChannel plugChannel;
    v_networkHashValue hashValue;
    char *partitionName;
    char *topicName;
    c_type type;
    nw_stream stream;
    c_bool result;

    NW_CONFIDENCE(bridge != NULL);
    NW_CONFIDENCE(message != NULL);
    NW_CONFIDENCE(typeLookupAction != NULL);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

    assert((bridge != NULL) && (message != NULL));
    if ((bridge != NULL) && (message != NULL) &&
        NW_CHANNEL_ID_IS_VALID(bridge, channelId)) {
        /* Find the channel to read from */
        plugChannel = NW_CHANNEL_BY_ID(bridge, channelId);
        NW_CONFIDENCE(plugChannel != NULL);
        /* Init out parameters */
        /* *message = NULL;  - now an IN/OUT */
        NW_REPORT_INFO(5, "entering transport read");

    	if (*message != NULL) {
            /*
             * If there is still a message present, don't read another message,
             * but only process incoming data from the network.
             */
    		 NW_REPORT_INFO(5, "previous message still pending in out-queue, just processing incoming messages");

            nw_plugReceiveChannelProcessIncoming(plugChannel);
        } else {
            NW_STRUCT(nw_senderInfo) senderInfo;
            memset(&senderInfo, 0, sizeof(senderInfo));
            stream = NW_STREAM_BY_ID(bridge, channelId);
            result = nw_stream_readBegin(stream, &senderInfo, prs);

            if (result) {
                partitionName = NULL;
                topicName = NULL;
                nw_stream_readOpaq(stream,sizeof(hashValue),&hashValue);
                nw_stream_readString(stream,NULL,&partitionName); /* TODO partitionName: expensive string duplication each iteration */
                if (partitionName) {
                    if(nw_configurationUseComplementPartitions()){
                        /* Change first character case, will leave built-in par-
                         * titions alone (start with _). */
                        partitionName[0] = (toupper(partitionName[0]) == partitionName[0])
                                            ? tolower(partitionName[0]) : toupper(partitionName[0]);
                    }
                    nw_stream_readString(stream,NULL,&topicName);
                    if (topicName) {

                    	type = typeLookupAction(hashValue,
                                                partitionName, topicName, /* TODO: topicName: expensive string duplication each iteration */
                                                typeLookupArg);

                        if (prs != NULL) {
                            if (prs->enabled) {
                                prs->numberOfMessagesReceived++;
                            }
                        }


                    	NW_REPORT_INFO_2(5, "reading message type %s via %s", topicName, partitionName);

                        if (type) {
                        	/* in any case parse the data from package */
                          	NW_REPORT_INFO_2(4, "parsing instance of %s from %s", topicName, (senderInfo.dn));
                            *message = nw_stream_read(stream, type);

                            if (!(*message)) {
                            	/* slow path */
                            	NW_REPORT_ERROR_2("nw_bridgeRead", "failed to parse instance of topic %s from partition %s", topicName, partitionName);
                            	/* TODO in this case the networking service is in undefined state and should restart */
                            } else {

                            	/* else-branch is fast path */
								/* if sender has not permission to send that data item, ignore and delete it */
								if (!(NW_SECURITY_CHECK_FOR_PUBLISH_PERMISSION_OF_SENDER_ON_RECEIVER_SIDE(senderInfo, partitionName, topicName))) {
									NW_REPORT_INFO_2(4, "sender no permission granted, dropping message %s from %s", topicName, (senderInfo.dn));
									c_free(*message);
									*message = NULL;
								} else {
									/* fast path */
									v_messageSetAllocTime(*message);
									if (prs != NULL) {
                                        if (prs->enabled) {
                                            prs->numberOfMessagesDelivered++;
                                        }
									}
								}
                            }
                        } else {
                            /* Trash rest of message */
                        	NW_REPORT_INFO_2(5, "type unknown, trash message type %s via %s", topicName, partitionName);

                        	*message = nw_stream_read(stream, type); /* otherwise *message stays defined and
																		causing side effects in next iteration */
                            assert(*message == NULL);
                            if (prs != NULL) {
                                if (prs->enabled) {
                                    prs->numberOfMessagesNotInterested++;
                                }
                            }
                        }
                        os_free(topicName);
                    }
                    os_free(partitionName);
                }
                nw_stream_readEnd(stream,prs);
            }
        }
    }
}


void
nw_bridgeTrigger(
    nw_bridge bridge,
    nw_seqNr channelId)
{
    nw_plugChannel plugChannel;

    if ((bridge != NULL) && NW_CHANNEL_ID_IS_VALID(bridge, channelId)) {

        /* Find the channel to read from */
        plugChannel = NW_CHANNEL_BY_ID(bridge, channelId);
        NW_CONFIDENCE(plugChannel != NULL);

        nw_plugReceiveChannelWakeUp(plugChannel);
    }
}

nw_receiveChannel
nw_bridgeNewReceiveChannel(
    nw_bridge bridge,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    nw_receiveChannel result = NULL;
    nw_plugChannel plugChannel = NULL;
    nw_seqNr currentId;
    nw_stream stream;

    NW_CONFIDENCE(bridge != NULL);

    if (bridge != NULL) {
        /* Create a channel on the bridge */
        plugChannel = nw_plugNetworkNewReceiveChannel(
            bridge->plugNetwork, pathName,onFatal,onFatalUsrData);
        if (plugChannel != NULL) {
            currentId = nw_plugChannelGetId(plugChannel);
            NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, currentId));
            stream = nw_stream_readOpen(plugChannel);
            /* Store it for later reference */
            NW_STREAM_BY_ID(bridge, currentId) = stream;
            /* And finally create the facade */
            result = nw_receiveChannelNew(bridge, currentId);
        }
    }

    return result;
}


nw_sendChannel
nw_bridgeNewSendChannel(
    nw_bridge bridge,
    const char *pathName,
    nw_onFatalCallBack onFatal,
    c_voidp onFatalUsrData)
{
    nw_sendChannel result = NULL;
    nw_plugChannel plugChannel = NULL;
    nw_seqNr currentId;
    nw_stream stream;

    NW_CONFIDENCE(bridge);

    if (bridge != NULL) {
        /* Create a channel on the bridge */
        plugChannel = nw_plugNetworkNewSendChannel(
                          bridge->plugNetwork,
                          pathName,
                          onFatal,
                          onFatalUsrData);
        if (plugChannel != NULL) {
            currentId = nw_plugChannelGetId(plugChannel);
            NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, currentId));
            stream = nw_stream_writeOpen(plugChannel);
            /* Store it for later reference */
            NW_STREAM_BY_ID(bridge, currentId) = stream;
            /* And finally create the facade */
            result = nw_sendChannelNew(bridge, currentId);
        }
    }

    return result;
}

#undef NW_DEFAULT_RELIABILITY
#undef NW_DEFAULT_PRIORITY
#undef NW_DEFAULT_LATENCYBUDGET


void
nw_bridgeFreeChannel(
    nw_bridge bridge,
    nw_seqNr channelId)
{
    nw_stream stream;

    NW_CONFIDENCE(bridge != NULL);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

    if ((bridge != NULL) &&
        NW_CHANNEL_ID_IS_VALID(bridge, channelId)) {
        stream = NW_STREAM_BY_ID(bridge, channelId);
        if (stream) {
            nw_stream_close(stream);
            NW_STREAM_BY_ID(bridge, channelId) = NULL;
        }
    }
}


void
nw_bridgeNotifyNodeStarted(
    nw_bridge bridge,
    v_networkId networkId,
    os_sockaddr_storage address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyNodeStarted(channel,
                                            (nw_networkId)networkId,
                                            address);
        }
    }
}

void
nw_bridgeNotifyNodeStopped(
    nw_bridge bridge,
    v_networkId networkId,
    os_sockaddr_storage address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyNodeStopped(channel,
                                            (nw_networkId)networkId,
                                            address);
        }
    }
}


void
nw_bridgeNotifyNodeDied(
    nw_bridge bridge,
    v_networkId networkId,
    os_sockaddr_storage address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyNodeDied(channel,
                                         (nw_networkId)networkId,
                                         address);
        }
    }
}

void
nw_bridgeNotifyGpAdd(
    nw_bridge bridge,
    v_networkId networkId,
    os_sockaddr_storage address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyGpAdd(channel,
                                            (nw_networkId)networkId,
                                            address);
        }
    }
}


void
nw_bridgeNotifyGpRemove(
    nw_bridge bridge,
    v_networkId networkId,
    os_sockaddr_storage address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyGpRemove(channel,
                                            (nw_networkId)networkId,
                                            address);
        }
    }
}




nw_channelType
nw_channelTypeFromString(
    const char *string)
{
    nw_channelType result = NW_CT_INVALID;

#define __NW_CT_FROM_STRING__(ct) \
    if (strcmp(string, #ct) == 0) result = NW_CT_##ct

    __NW_CT_FROM_STRING__(BEST_EFFORT);
/*    __NW_CT_FROM_STRING__(RELIABLE);     */
/*    __NW_CT_FROM_STRING__(RELIABLE_P2P); */
/*    __NW_CT_FROM_STRING__(GUARDED_P2P);  */

#undef __NW_CT_FROM_STRING__

    return result;
}


#undef NW_CHANNEL_ID_IS_VALID
#undef NW_CHANNEL_BY_ID

#define _STREAM_

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
#ifdef _STREAM_
#include "nw_stream.h"
#endif

/* removal of inefficient code */
/* This implementation (_RP_) is not yet correct since it always swap the data.
 * The original code is endianess independent.
#define _RP_
 */

/* Bridge class definition */

#define NW_CHANNEL_ID_IS_VALID(bridge,id) (id < bridge->nofChannels)
#ifdef _STREAM_
#define NW_STREAM_BY_ID(bridge,id) bridge->streams[id]
#define NW_CHANNEL_BY_ID(bridge,id) nw_stream_channel(NW_STREAM_BY_ID(bridge,id))
#else
#define NW_CHANNEL_BY_ID(bridge,id)       (bridge->channels[id])
#endif

NW_STRUCT(nw_bridge) {
    nw_plugNetwork plugNetwork;
    nw_seqNr nofChannels;
#ifdef _STREAM_
    nw_stream *streams;
#else
    nw_plugChannel *channels; /* [nofChannels] */
#endif
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


/* ------------------------- Wire protocol message -------------------------- */

#ifdef _RP_

#define nw_bridgeHostToNetwork(v) nw_seqNrSwap(v)
#define nw_bridgeNetworkToHost(v) nw_seqNrSwap(v)

static nw_seqNr
nw_seqNrSwap(
    nw_seqNr value)
{
    nw_seqNr result;

    unsigned char *src = (unsigned char *)&value;
    unsigned char *dst = (unsigned char *)&result;

    dst[0] = src[3];
    dst[1] = src[2];
    dst[2] = src[1];
    dst[3] = src[0];

    return result;
}

#else

/* Macro's needed for platform independent representation */
/* MSB */
#define NW_GET_BYTE_4(uintval) (((uintval) >> 24)       )
#define NW_GET_BYTE_3(uintval) (((uintval) >> 16) & 0xFFU)
#define NW_GET_BYTE_2(uintval) (((uintval) >>  8) & 0xFFU)
#define NW_GET_BYTE_1(uintval) (((uintval)      ) & 0xFFU)
/* LSB */

typedef unsigned char nw_charArray[4];

static nw_seqNr
nw_bridgeHostToNetwork(
    nw_seqNr value)
{
    union valPtr {
        nw_seqNr  value;
        nw_charArray array;
    } resultUnion;
    unsigned char *bytePtr;

    bytePtr = resultUnion.array;

    /* Note that the following code is endianess aware! */
    bytePtr[0] = (unsigned char)NW_GET_BYTE_4(value);
    bytePtr[1] = (unsigned char)NW_GET_BYTE_3(value);
    bytePtr[2] = (unsigned char)NW_GET_BYTE_2(value);
    bytePtr[3] = (unsigned char)NW_GET_BYTE_1(value);

    return resultUnion.value;
}

#undef NW_GET_BYTE_4
#undef NW_GET_BYTE_3
#undef NW_GET_BYTE_2
#undef NW_GET_BYTE_1

#define NW_SET_BYTE_4(res,uintval) (res) += ((nw_seqNr)(uintval) << 24)
#define NW_SET_BYTE_3(res,uintval) (res) += ((nw_seqNr)(uintval) << 16)
#define NW_SET_BYTE_2(res,uintval) (res) += ((nw_seqNr)(uintval) <<  8)
#define NW_SET_BYTE_1(res,uintval) (res) += ((nw_seqNr)(uintval)      )

static nw_seqNr
nw_bridgeNetworkToHost(
    nw_seqNr value)
{
    nw_seqNr result;
    union valPtr {
        nw_seqNr value;
        nw_charArray array;
    } resultUnion;
    unsigned char *bytePtr;

    resultUnion.value = value;
    bytePtr = resultUnion.array;

    /* Note that the following code is endianess aware! */
    result = 0;
    NW_SET_BYTE_4(result, bytePtr[0]);
    NW_SET_BYTE_3(result, bytePtr[1]);
    NW_SET_BYTE_2(result, bytePtr[2]);
    NW_SET_BYTE_1(result, bytePtr[3]);

    return result;
}

#undef NW_SET_BYTE_4
#undef NW_SET_BYTE_3
#undef NW_SET_BYTE_2
#undef NW_SET_BYTE_1

#endif

NW_STRUCT(nw_bridgeMessage) {
    v_networkHashValue hashValue;
#ifndef _STREAM_
    nw_globalId sender;
    nw_globalId receiver;
    nw_seqNr seqNr;
#endif
};
NW_CLASS(nw_bridgeMessage);

#ifndef _STREAM_
#ifdef _RP_
#define nw_bridgeMessageSetDummySender(message) \
        memset(&message->sender,0,sizeof(message->sender))

#define nw_bridgeMessageSetDummyReceiver(message) \
        memset(&message->receiver,0,sizeof(message->receiver))

#define nw_bridgeMessageResetSeqNr(message) \
        message->seqNr = 0
#endif

static nw_globalId
nw_bridgeMessageGetSender(
    nw_bridgeMessage message)
{
    nw_globalId result;

    result.nodeId =   nw_bridgeNetworkToHost(message->sender.nodeId);
    result.localId1 = nw_bridgeNetworkToHost(message->sender.localId1);
    result.localId2 = nw_bridgeNetworkToHost(message->sender.localId2);

    return result;
}

static void
nw_bridgeMessageSetSender(
    nw_bridgeMessage message,
    nw_globalId sender)
{
    message->sender.nodeId =   nw_bridgeHostToNetwork(sender.nodeId);
    message->sender.localId1 = nw_bridgeHostToNetwork(sender.localId1);
    message->sender.localId2 = nw_bridgeHostToNetwork(sender.localId2);
}

static nw_globalId
nw_bridgeMessageGetReceiver(
    nw_bridgeMessage message)
{
    nw_globalId result;

    result.nodeId =   nw_bridgeNetworkToHost(message->receiver.nodeId);
    result.localId1 = nw_bridgeNetworkToHost(message->receiver.localId1);
    result.localId2 = nw_bridgeNetworkToHost(message->receiver.localId2);

    return result;
}

static void
nw_bridgeMessageSetReceiver(
    nw_bridgeMessage message,
    nw_globalId receiver)
{
    message->receiver.nodeId =   nw_bridgeHostToNetwork(receiver.nodeId);
    message->receiver.localId1 = nw_bridgeHostToNetwork(receiver.localId1);
    message->receiver.localId2 = nw_bridgeHostToNetwork(receiver.localId2);
}

#ifdef _RP_
#define nw_bridgeMessageGetSeqNr(message) \
        nw_bridgeNetworkToHost(message->seqNr)

#define nw_bridgeMessageSetSeqNr(message,seqNr) \
        message->seqNr = nw_bridgeHostToNetwork(seqNr)
#else
static nw_seqNr
nw_bridgeMessageGetSeqNr(
    nw_bridgeMessage message)
{
    nw_seqNr result;

    result = nw_bridgeNetworkToHost(message->seqNr);

    return result;
}
static void
nw_bridgeMessageSetSeqNr(
    nw_bridgeMessage message,
    nw_seqNr seqNr)
{
    message->seqNr = nw_bridgeHostToNetwork(seqNr);
}
#endif
#endif

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
#ifdef _STREAM_
        result->streams = (nw_stream *)os_malloc(
            result->nofChannels * (os_uint32)sizeof(*result->streams));
        NW_CONFIDENCE(result->streams);
        if (result->streams) {
            for (i=0; i<result->nofChannels; i++) {
                NW_STREAM_BY_ID(result, i) = NULL;
#else
        result->channels = (nw_plugChannel *)os_malloc(
            result->nofChannels * (os_uint32)sizeof(*result->channels));
        NW_CONFIDENCE(result->channels);
        if (result->channels) {
            for (i=0; i<result->nofChannels; i++) {
                NW_CHANNEL_BY_ID(result, i) = NULL;
#endif
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
#ifdef _STREAM_
        os_free(bridge->streams);
#else
        os_free(bridge->channels);
#endif
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
    nw_signedLength *bytesLeft)
{
    nw_plugChannel plugChannel;

    NW_CONFIDENCE(bridge != NULL);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

    if ((bridge != NULL) && NW_CHANNEL_ID_IS_VALID(bridge, channelId)) {

        /* Find the channel to read from */
        plugChannel = NW_CHANNEL_BY_ID(bridge, channelId);
        NW_CONFIDENCE(plugChannel != NULL);

        nw_plugSendChannelPeriodicAction(plugChannel,bytesLeft);
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
    nw_signedLength *bytesLeft)
{
    c_ulong result = 0;

#ifdef _STREAM_
    nw_stream stream;
#else
    nw_data networkBuffer;
    nw_length networkBufferLength;
    struct nw_outOfBuffersArg_s outOfBuffersArg;
    nw_bridgeMessage bridgeMessage;
    nw_plugChannel plugChannel;
#endif

    NW_CONFIDENCE(bridge != NULL);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

#ifdef _STREAM_
    stream = NW_STREAM_BY_ID(bridge, channelId);

    nw_stream_writeBegin(stream, partitionId, bytesLeft);

    result  = nw_stream_writeOpaq(stream,sizeof(hashValue),(c_voidp)&hashValue);
    result += nw_stream_writeString(stream,NULL,(c_voidp)&partitionName);
    result += nw_stream_writeString(stream,NULL,(c_voidp)&topicName);
    result += nw_stream_write(stream,message);
    nw_stream_writeEnd(stream);
#else
    plugChannel = NW_CHANNEL_BY_ID(bridge, channelId);
    NW_CONFIDENCE(plugChannel != NULL);

    nw_plugSendChannelMessageStart(plugChannel, &networkBuffer, &networkBufferLength, partitionId, bytesLeft);

    outOfBuffersArg.plugChannel = plugChannel;
    outOfBuffersArg.byteCount = networkBufferLength;

    /* Fill the headers like flags, version number and hash value.
     * If reliable, also fill senderGid, receiverGid and seqNr */
 
    NW_CONFIDENCE(networkBufferLength >= sizeof(*bridgeMessage));
    bridgeMessage = (nw_bridgeMessage)networkBuffer;
    bridgeMessage->hashValue = hashValue;
#ifdef _RP_
    nw_bridgeMessageSetDummySender(bridgeMessage);
    nw_bridgeMessageSetDummyReceiver(bridgeMessage);
    nw_bridgeMessageResetSeqNr(bridgeMessage);
#else
    nw_bridgeMessageSetSender(bridgeMessage, dummyId);
    nw_bridgeMessageSetReceiver(bridgeMessage, dummyId);
    nw_bridgeMessageSetSeqNr(bridgeMessage, 0);
#endif
    networkBufferLength = networkBufferLength - sizeof(*bridgeMessage);
    networkBuffer = &(networkBuffer[sizeof(*bridgeMessage)]);
    /* Copy partition and topicName */
    outOfBuffersArg.byteCount -= networkBufferLength;
    writeStringToPlugChannel(plugChannel, &networkBuffer,
        &networkBufferLength, partitionName);
    writeStringToPlugChannel(plugChannel, &networkBuffer,
        &networkBufferLength, topicName);
    outOfBuffersArg.byteCount += networkBufferLength;

    /* serialize the data */
    c_serialize(message, (c_octet **)&networkBuffer, &networkBufferLength,
        onSerializerOutOfBuffers, &outOfBuffersArg);
    result = outOfBuffersArg.byteCount - networkBufferLength + sizeof(*bridgeMessage);

    nw_plugSendChannelMessageEnd(plugChannel, networkBuffer);
#endif

    return result;
}

nw_bool
nw_bridgeFlush(
    nw_bridge bridge,
    nw_seqNr channelId,
    nw_bool all,
    nw_signedLength *bytesLeft)
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

    result = nw_plugSendChannelMessagesFlush(plugChannel, all, bytesLeft);

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
    nw_typeLookupArg typeLookupArg)
{
    nw_plugChannel plugChannel;
    v_networkHashValue hashValue;
    char *partitionName;
    char *topicName;
    c_type type;
#ifdef _STREAM_
    nw_stream stream;
    c_bool result;
#else
    nw_data networkBuffer;
    nw_globalId sender;
    nw_globalId receiver;
    nw_seqNr seqNr;
    nw_length networkBufferLength;
    nw_bridgeMessage bridgeMessage;
    nw_address senderAddress;
#endif

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

        if (*message != NULL) {
            /*
             * If there is still a message present, don't read another message,
             * but only process incoming data from the network.
             */
            nw_plugReceiveChannelProcessIncoming(plugChannel);
        } else {
#ifdef _STREAM_
            stream = NW_STREAM_BY_ID(bridge, channelId);
            result = nw_stream_readBegin(stream);
            if (result) {
                partitionName = NULL;
                topicName = NULL;
                nw_stream_readOpaq(stream,sizeof(hashValue),&hashValue);
                nw_stream_readString(stream,NULL,&partitionName);
                if (partitionName) {
                    nw_stream_readString(stream,NULL,&topicName);
                    if (topicName) {
                        type = typeLookupAction(hashValue,
                                                partitionName, topicName,
                                                typeLookupArg);

                        if (type) {
                            *message = nw_stream_read(stream, type);
                            v_messageSetAllocTime(*message);
                        } else {
                            /* Trash rest of message */
                            nw_stream_read(stream, type);
                        }
                        os_free(topicName);
                    }
                    os_free(partitionName);
                }
                nw_stream_readEnd(stream);
            }
#else
            /* Find the channel to read from */
            plugChannel = NW_CHANNEL_BY_ID(bridge, channelId);
            NW_CONFIDENCE(plugChannel != NULL);

            nw_plugReceiveChannelMessageStart(plugChannel,
                &networkBuffer, &networkBufferLength, &senderAddress);

            if (networkBufferLength > 0U) {
                NW_CONFIDENCE(networkBuffer != NULL);
                NW_CONFIDENCE(networkBufferLength >= sizeof(*bridgeMessage));
                bridgeMessage = (nw_bridgeMessage)networkBuffer;
                hashValue = bridgeMessage->hashValue;
#ifndef _STREAM_
                sender = nw_bridgeMessageGetSender(bridgeMessage);
                receiver = nw_bridgeMessageGetReceiver(bridgeMessage);
                seqNr = nw_bridgeMessageGetSeqNr(bridgeMessage);
#endif
                NW_CONFIDENCE(networkBufferLength > sizeof(*bridgeMessage));
                networkBufferLength = networkBufferLength - sizeof(*bridgeMessage);
                networkBuffer = &(networkBuffer[sizeof(*bridgeMessage)]);

                /* Get partitionName and topicName from data and
                 * call the action routine */
#ifdef _STREAM_
                networkBuffer = &networkBuffer[1];
                networkBufferLength--;
                if (networkBufferLength == 0U) {
                    nw_plugReceiveChannelGetNextFragment(plugChannel,
                        &networkBuffer, &networkBufferLength);
                }
#endif
                partitionName = getStringFromPlugChannel(plugChannel,
                    &networkBuffer, &networkBufferLength);

#ifdef NW_LOOPBACK
#define NW_UNDERSCORE (unsigned char)'_'
#define NW_UPPERSCORE (unsigned char)'^'
                if (nw_configurationUseComplementPartitions()) {
                    if (islower((int)*partitionName)) {
                        *partitionName = (char)toupper((int)*partitionName);
                    } else {
                        if (isupper((int)*partitionName)) {
                            *partitionName = (char)tolower((int)*partitionName);
                        } else {
                            switch (*partitionName) {
                            case NW_UNDERSCORE:
                                *partitionName = NW_UPPERSCORE;
                            break;
                            case NW_UPPERSCORE:
                                *partitionName = NW_UNDERSCORE;
                            break;
                            default:
                                NW_REPORT_WARNING_1("nw_bridgeRead",
                                    "Malformed partition name\"%s\"", partitionName);
                            break;
                            }
                        }
                    }
                }
#undef NW_UNDERSCORE
#undef NW_UPPERSCORE
#endif
                /* Timestamps available in:
                 * nw_plugReceiveChannel(plugChannel)->lastReturnedBuffer
                 * nw_plugReceiveChannel(plugChannel)->lastReturnedHolder
                 */

#ifdef _STREAM_
                networkBuffer = &networkBuffer[1];
                networkBufferLength--;
                if (networkBufferLength == 0U) {
                    nw_plugReceiveChannelGetNextFragment(plugChannel,
                        &networkBuffer, &networkBufferLength);
                }
#endif
                topicName = getStringFromPlugChannel(plugChannel,
                    &networkBuffer, &networkBufferLength);
                type = typeLookupAction(hashValue, partitionName, topicName, typeLookupArg);

                os_free(partitionName);
                os_free(topicName);
                if (type != NULL) {

#ifdef _MSG_STAMP_
                    os_time _time = os_hrtimeGet();
#ifdef NW_TIMESTAMP
                    os_time *_htime = nw_plugReceiveChannelLastHolderTimestamps(plugChannel);
                    os_time *_btime = nw_plugReceiveChannelLastBufferTimestamps(plugChannel);
#endif
                    *message = c_deserialize(type,
                        (c_octet **)&networkBuffer, &networkBufferLength,
                        onDeserializerOutOfBuffers, plugChannel);

#ifdef NW_TIMESTAMP
                    V_MESSAGE_SETSTAMP(*message,nwBufferFullTime,
                                       _btime[NW_BUF_TIMESTAMP_FILLED]);
                    V_MESSAGE_SETSTAMP(*message,nwFlushBufferTime,
                                       _btime[NW_BUF_TIMESTAMP_FLUSH]);
                    V_MESSAGE_SETSTAMP(*message,nwSendTime,
                                       _btime[NW_BUF_TIMESTAMP_SEND]);
                    V_MESSAGE_HOPINC(*message);
                    V_MESSAGE_SETSTAMP(*message,nwReceiveTime,
                                       _btime[NW_BUF_TIMESTAMP_RECEIVE]);
                    V_MESSAGE_SETSTAMP(*message,nwInsertTime,
                                       _btime[NW_BUF_TIMESTAMP_HANDLE]);
#endif
                    V_MESSAGE_SETSTAMP(*message,writerAllocTime,_time);
                    V_MESSAGE_STAMP(*message,writerCopyTime);
                    V_MESSAGE_STAMP(*message,writerLookupTime);
#else
                    *message = c_deserialize(type,
                        (c_octet **)&networkBuffer, &networkBufferLength,
                        onDeserializerOutOfBuffers, plugChannel);
#endif

#ifdef NW_LOOPBACK
                    if (nw_configurationUseLoopback() && (*message != NULL)) {
                        /* During loopback-testing, the writerGid has to be adapted in
                         * order to simulate that the data comes from another kernel.
                         * Note that this requires some internal knowledge on the
                         * Gid-structure... */
                         memset(&((*message)->writerGID), 0, 4);
                         /* Change time a tiny bit. Note: this assumes that
                          * nanoseconds does not have the value 999999999 :-) */
                         (*message)->writeTime.nanoseconds++;
                    }
#endif
                    nw_plugReceiveChannelMessageEnd(plugChannel);
                } else {
                    nw_plugReceiveChannelMessageIgnore(plugChannel);
                }
            } else { 
                NW_CONFIDENCE(networkBuffer == NULL);
            }
#endif
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
#ifdef _STREAM_
    nw_stream stream;
#endif

    NW_CONFIDENCE(bridge != NULL);

    if (bridge != NULL) {
        /* Create a channel on the bridge */
        plugChannel = nw_plugNetworkNewReceiveChannel(
            bridge->plugNetwork, pathName,onFatal,onFatalUsrData);
        if (plugChannel != NULL) {
            currentId = nw_plugChannelGetId(plugChannel);
            NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, currentId));
#ifdef _STREAM_
            stream = nw_stream_readOpen(plugChannel);
            /* Store it for later reference */
            NW_STREAM_BY_ID(bridge, currentId) = stream;
#else
            /* Store it for later reference */
            NW_CHANNEL_BY_ID(bridge, currentId) = plugChannel;
#endif
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
#ifdef _STREAM_
    nw_stream stream;
#endif

    NW_CONFIDENCE(bridge);

    if (bridge != NULL) {
        /* Create a channel on the bridge */
        plugChannel = nw_plugNetworkNewSendChannel(bridge->plugNetwork, pathName,onFatal,onFatalUsrData);
        if (plugChannel != NULL) {
            currentId = nw_plugChannelGetId(plugChannel);
            NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, currentId));
#ifdef _STREAM_
            stream = nw_stream_writeOpen(plugChannel);
            /* Store it for later reference */
            NW_STREAM_BY_ID(bridge, currentId) = stream;
#else
            /* Store it for later reference */
            NW_CHANNEL_BY_ID(bridge, currentId) = plugChannel;
#endif
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
#ifdef _STREAM_
    nw_stream stream;
#endif

    NW_CONFIDENCE(bridge != NULL);
    NW_CONFIDENCE(NW_CHANNEL_ID_IS_VALID(bridge, channelId));

    if ((bridge != NULL) &&
        NW_CHANNEL_ID_IS_VALID(bridge, channelId)) {
#ifdef _STREAM_
        stream = NW_STREAM_BY_ID(bridge, channelId);
        if (stream) {
            nw_stream_close(stream);
            NW_STREAM_BY_ID(bridge, channelId) = NULL;
        }
#else
        NW_CHANNEL_BY_ID(bridge, channelId) = NULL;
#endif
    }
}


void
nw_bridgeNotifyNodeStarted(
    nw_bridge bridge,
    v_networkId networkId,
    nw_address address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyNodeStarted(channel, (nw_networkId)networkId, address);
        }
    }
}

void
nw_bridgeNotifyNodeStopped(
    nw_bridge bridge,
    v_networkId networkId,
    nw_address address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyNodeStopped(channel, (nw_networkId)networkId, address);
        }
    }
}


void
nw_bridgeNotifyNodeDied(
    nw_bridge bridge,
    v_networkId networkId,
    nw_address address)
{
    nw_seqNr i;
    nw_plugChannel channel;

    NW_CONFIDENCE(sizeof(networkId) == sizeof(nw_networkId));
    for (i=0; i<bridge->nofChannels; i++) {
        channel = NW_CHANNEL_BY_ID(bridge, i);
        if (channel != NULL) {
            nw_plugChannelNotifyNodeDied(channel, (nw_networkId)networkId, address);
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

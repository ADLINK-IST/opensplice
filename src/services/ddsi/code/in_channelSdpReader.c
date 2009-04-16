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
#include "in_channelSdp.h"
#include "in_channelReader.h"
#include "in_channelSdpReader.h"
#include "in_channelSdpWriter.h"
#include "in__configChannel.h"
#include "in_connectivityAdmin.h"
#include "os_heap.h"
#include "os_time.h"

OS_STRUCT(in_channelSdpReader)
{
    OS_EXTENDS(in_channelReader);
    in_streamReader reader;
    in_streamReaderCallbackTable callbacks;
};

static os_boolean
in_channelSdpReaderInit(
    in_channelSdpReader sdp,
    in_channelSdp channel,
    in_configDiscoveryChannel config);

static void
in_channelSdpReaderDeinit(
    in_object obj);

static void*
in_channelSdpReaderStart(
    in_runnable _this);

static void
in_channelSdpReaderTrigger(
    in_runnable _this);

static in_result
in_channelSdpReaderProcessPeerEntity(
    in_streamReaderCallbackArg _this,
    in_discoveredPeer peer);

static in_result
in_channelSdpReaderProcessData(
    in_streamReaderCallbackArg _this,
    v_message message,
    in_connectivityPeerWriter peerWriter,
    in_ddsiReceiver receiver);

static in_result
in_channelSdpReaderProcessDataFrag(
    in_streamReaderCallbackArg _this,
    in_ddsiDataFrag event,
    in_ddsiReceiver receiver);

static in_result
in_channelSdpReaderProcessHeartbeat(
    in_streamReaderCallbackArg _this,
    in_ddsiHeartbeat event,
    in_ddsiReceiver receiver);

static in_result
in_channelSdpReaderProcessAckNack(
    in_streamReaderCallbackArg _this,
    in_ddsiAckNack event,
    in_ddsiReceiver receiver);

static in_result
in_channelSdpReaderProcessNackFrag(
    in_streamReaderCallbackArg _this,
    in_ddsiNackFrag event,
    in_ddsiReceiver receiver);

static in_result
in_channelSdpReaderRequestNackFrag(
    in_streamReaderCallbackArg _this,
    in_ddsiNackFragRequest request);

in_channelSdpReader
in_channelSdpReaderNew(
    in_channelSdp channel,
    in_configDiscoveryChannel config)
{
    in_channelSdpReader sdp;
    os_boolean success;

    sdp = os_malloc(OS_SIZEOF(in_channelSdpReader));

    if(sdp)
    {
        success = in_channelSdpReaderInit(sdp, channel, config);

        if(!success)
        {
            os_free(sdp);
            sdp = NULL;
        }
    }
    return sdp;
}

static os_boolean
in_channelSdpReaderInit(
    in_channelSdpReader sdp,
    in_channelSdp channel,
    in_configDiscoveryChannel config)
{
    os_boolean success;
    in_stream stream;

    assert(sdp);
    assert(in_channelSdpIsValid(channel));
    assert(config);

    success = in_channelReaderInit(
            in_channelReader(sdp),
            IN_OBJECT_KIND_SDP_READER,
            in_channelSdpReaderDeinit, "sdpReader",
            in_configChannelGetPathName(in_configChannel(config)),
            in_channelSdpReaderStart, in_channelSdpReaderTrigger,
            in_channel(channel));

    if(success)
    {
        sdp->callbacks = (in_streamReaderCallbackTable)(
                os_malloc(OS_SIZEOF(in_streamReaderCallbackTable)));

        if(sdp->callbacks)
        {
            sdp->callbacks->processPeerEntity = in_channelSdpReaderProcessPeerEntity;
            sdp->callbacks->processData = in_channelSdpReaderProcessData;
            sdp->callbacks->processDataFrag = in_channelSdpReaderProcessDataFrag;
            sdp->callbacks->processAckNack = in_channelSdpReaderProcessAckNack;
            sdp->callbacks->processNackFrag = in_channelSdpReaderProcessNackFrag;
            sdp->callbacks->processHeartbeat = in_channelSdpReaderProcessHeartbeat;
            sdp->callbacks->requestNackFrag = in_channelSdpReaderRequestNackFrag;

            stream = in_channelGetStream(in_channel(channel));
            sdp->reader = in_streamGetReader(stream);
            in_streamFree(stream);

            in_runnableStart(in_runnable(sdp));
        } else
        {
            in_channelReaderDeinit(in_object(sdp));
            success = OS_FALSE;
        }
    }
    return success;
}

static void
in_channelSdpReaderDeinit(
    in_object obj)
{
    in_channelSdpReader _this;

    assert(in_channelSdpReaderIsValid(obj));
    _this = in_channelSdpReader(obj);

    in_runnableStop(in_runnable(obj));
    os_free(_this->callbacks);
    _this->callbacks = NULL;
    in_streamReaderFree(_this->reader);

    in_channelReaderDeinit(obj);
}

static void*
in_channelSdpReaderStart(
    in_runnable runnable)
{
    in_channelSdpReader _this;
    os_time sleepTime;
    in_result result;

    assert(in_channelSdpReaderIsValid(runnable));

    _this = in_channelSdpReader(runnable);
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = 10 * 1000 * 1000; /* 10 ms */

    while(!in_runnableTerminationRequested(runnable))
    {
        result = in_streamReaderScan(
                _this->reader,
                _this->callbacks,
                _this,
                &sleepTime);

        if(!in_runnableTerminationRequested(runnable))
        {
            os_nanoSleep(sleepTime);
        }
    }
    return NULL;
}

static void
in_channelSdpReaderTrigger(
    in_runnable runnable)
{
    assert(in_channelSdpReaderIsValid(runnable));

    return;
}

static in_result
in_channelSdpReaderProcessPeerEntity(
    in_streamReaderCallbackArg _this,
    in_discoveredPeer peer)
{
    in_connectivityAdmin admin;
    in_channelSdpReader sdpr;
    in_channel channel;
    in_channelSdpWriter sdpw;
    in_result result;

    sdpr = in_channelSdpReader(_this);
    channel = in_channelReaderGetChannel(in_channelReader(sdpr));
    sdpw = in_channelSdpWriter(in_channelGetWriter(channel));
    admin = in_connectivityAdminGetInstance();

    switch(in_objectGetKind(in_object(peer->discoveredPeerEntity)))
    {
        case IN_OBJECT_KIND_PEER_PARTICIPANT:
            result = in_connectivityAdminAddPeerParticipant(
                    admin, in_connectivityPeerParticipant(
                            peer->discoveredPeerEntity));
        break;
        case IN_OBJECT_KIND_PEER_READER:
            result = in_connectivityAdminAddPeerReader(
                    admin, in_connectivityPeerReader(
                            peer->discoveredPeerEntity));
        break;
        case IN_OBJECT_KIND_PEER_WRITER:
            result = in_connectivityAdminAddPeerWriter(
                    admin, in_connectivityPeerWriter(
                            peer->discoveredPeerEntity));
        break;
        default:
            assert(FALSE);
            result = IN_RESULT_ERROR;
        break;
    }
    if(result == IN_RESULT_OK)
    {
        /*TODO: uncomment next line when function is available*/
        /*in_channelSdpWriterAddPeerEntity(sdpw, peer);*/
    }

    return result;
}

static in_result
in_channelSdpReaderProcessData(
    in_streamReaderCallbackArg _this,
    v_message message,
    in_connectivityPeerWriter peerWriter,
    in_ddsiReceiver receiver)
{
    return IN_RESULT_OK;
}

static in_result
in_channelSdpReaderProcessDataFrag(
    in_streamReaderCallbackArg _this,
    in_ddsiDataFrag event,
    in_ddsiReceiver receiver)
{
    return IN_RESULT_OK;
}

static in_result
in_channelSdpReaderProcessHeartbeat(
    in_streamReaderCallbackArg _this,
    in_ddsiHeartbeat event,
    in_ddsiReceiver receiver)
{
    return IN_RESULT_OK;
}

static in_result
in_channelSdpReaderProcessAckNack(
    in_streamReaderCallbackArg _this,
    in_ddsiAckNack event,
    in_ddsiReceiver receiver)
{
    return IN_RESULT_OK;
}

static in_result
in_channelSdpReaderProcessNackFrag(
    in_streamReaderCallbackArg _this,
    in_ddsiNackFrag event,
    in_ddsiReceiver receiver)
{
    return IN_RESULT_OK;
}

static in_result
in_channelSdpReaderRequestNackFrag(
    in_streamReaderCallbackArg _this,
    in_ddsiNackFragRequest request)
{
    return IN_RESULT_OK;
}

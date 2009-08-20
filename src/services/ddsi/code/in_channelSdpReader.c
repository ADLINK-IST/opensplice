#include "in_channelSdp.h"
#include "in_channelReader.h"
#include "in_channelSdpReader.h"
#include "in_channelSdpWriter.h"
#include "in__configChannel.h"
#include "in_connectivityAdmin.h"
#include "os_heap.h"
#include "os_time.h"
#include "in_report.h"

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
in_channelSdpReaderProcessAckNackFunc(
        in_streamReaderCallbackArg _this,
        in_ddsiAckNack event,
        in_ddsiReceiver receiver);

static in_result in_channelSdpReaderProcessHeartbeatFunc(
    in_streamReaderCallbackArg _this,
    in_ddsiHeartbeat event,
    in_ddsiReceiver receiver);

static os_boolean in_channelSdpReaderIsLocalEntityFunc(
        in_streamReaderCallbackArg _this,
        in_ddsiGuidPrefixRef guidPrefixRef);

OS_STRUCT(in_channelSdpReader)
{
    OS_EXTENDS(in_channelReader);
    in_streamReader reader;
};

in_channelSdpReader
in_channelSdpReaderNew(
    in_channelSdp channel,
    in_configDiscoveryChannel config)
{
    in_channelSdpReader _this;
    os_boolean success;

    _this = os_malloc(OS_SIZEOF(in_channelSdpReader));
    if(_this)
    {
        success = in_channelSdpReaderInit(_this, channel, config);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
            IN_TRACE_1(Construction,2,"in_channelSdpReader creation successful = %x",_this);
        } else
        {
            IN_TRACE_1(Construction,2,"in_channelSdpReader creation failed = %x",_this);
        }
    }
    return _this;
}

static OS_STRUCT(in_streamReaderCallbackTable)
in_channelSdpReaderCallbackTable =
{
        in_channelSdpReaderProcessPeerEntity,
        NULL, /* processData */
        NULL, /* processDataFrag */
        in_channelSdpReaderProcessAckNackFunc,
        NULL, /* processNackFrag */
        in_channelSdpReaderProcessHeartbeatFunc, /* processHeartbeat */
        NULL, /* requestNackFrag */
        in_channelSdpReaderIsLocalEntityFunc
};

os_boolean
in_channelSdpReaderInit(
    in_channelSdpReader _this,
    in_channelSdp channel,
    in_configDiscoveryChannel config)
{
    os_boolean success;
    in_stream stream;

    assert(_this);
    assert(in_channelSdpIsValid(channel));
    assert(config);

    success = in_channelReaderInit(
        in_channelReader(_this),
        IN_OBJECT_KIND_SDP_READER,
        in_channelSdpReaderDeinit,
        "sdpReader",
        in_configChannelGetPathName(in_configChannel(config)),
        in_channelSdpReaderStart,
        in_channelSdpReaderTrigger,
        in_channel(channel));
    if(success)
    {
        stream = in_channelGetStream(in_channel(channel));
        _this->reader = in_streamGetReader(stream);
        in_streamFree(stream);
    }
    return success;
}

void
in_channelSdpReaderDeinit(
    in_object obj)
{
    in_channelSdpReader _this;

    assert(obj);
    assert(in_channelSdpReaderIsValid(obj));

    _this = in_channelSdpReader(obj);
    in_runnableStop(in_runnable(obj));
    if(_this->reader)
    {
        in_streamReaderFree(_this->reader);
        _this->reader = NULL;
    }
    in_channelReaderDeinit(obj);
}

void*
in_channelSdpReaderStart(
    in_runnable runnable)
{
    const os_uint32 ERROR_INTERVAL = 500;
    os_uint32 errorCounter = 0;

    const os_time POLL_PERIOD = {0, 10*1000*1000}; /* 50 ms */

    in_channelSdpReader _this;
    os_time pollTimeout;
    in_result result;

    assert(runnable);
    assert(in_channelSdpReaderIsValid(runnable));

    _this = in_channelSdpReader(runnable);

    IN_TRACE(Receive, 3, "in_channelSDPReader started");
    while(!in_runnableTerminationRequested(runnable))
    {
        /* must be renewed */
        pollTimeout = POLL_PERIOD;

        result = in_streamReaderScan(
            _this->reader,
            &in_channelSdpReaderCallbackTable, /* static vtable */
            _this,
            &pollTimeout);
        /* may return before timeout exceeded, then pollTimeout contains
         * the remaining time, which may be used for other activities until
         * POLL_PERIOD interval has passed.  */
        if (result != IN_RESULT_OK && result != IN_RESULT_TIMEDOUT)
        {
            if (errorCounter == 0)
            {
                /* first occurance */
                IN_REPORT_WARNING(IN_SPOT, "unexpected data read error");
            } else if (errorCounter >= ERROR_INTERVAL)
            {
                IN_REPORT_WARNING_1(IN_SPOT, "unexpected data read error (repeated %d times)", errorCounter);
                errorCounter = 0;
            }
            ++errorCounter;
            /* POST: errorCounter >= 1 */
        }
    }
    return NULL;
}

void
in_channelSdpReaderTrigger(
    in_runnable runnable)
{
    assert(runnable);
    assert(in_channelSdpReaderIsValid(runnable));
}

in_result
in_channelSdpReaderProcessPeerEntity(
    in_streamReaderCallbackArg _this,
    in_discoveredPeer peer)
{
    in_connectivityAdmin admin;
    in_channelSdpReader sdpr;
    in_channel channel;
    in_channelSdpWriter sdpw;
    in_result result;

    assert(_this);
    assert(peer);

    IN_TRACE(Receive, 3, "in_channelSdpReaderProcessPeerEntity CALLBACK");

    sdpr = in_channelSdpReader(_this);
    channel = in_channelReaderGetChannel(in_channelReader(sdpr));
    sdpw = in_channelSdpWriter(in_channelGetWriter(channel));
    admin = in_connectivityAdminGetInstance();
    switch(in_objectGetKind(in_object(peer->discoveredPeerEntity)))
    {
        case IN_OBJECT_KIND_PEER_PARTICIPANT:
            result = in_connectivityAdminAddPeerParticipant(
                admin,
                in_connectivityPeerParticipant(peer->discoveredPeerEntity));
            IN_TRACE_1(Send, 2, "in_channelSdpReaderProcessPeerEntity - participant result = %x", result);
        break;
        case IN_OBJECT_KIND_PEER_READER:
            result = in_connectivityAdminAddPeerReader(
                admin,
                in_connectivityPeerReader(peer->discoveredPeerEntity),
                peer->sequenceNumber);
          IN_TRACE_1(Send, 2, "in_channelSdpReaderProcessPeerEntity - reader result = %x", result);
        break;
        case IN_OBJECT_KIND_PEER_WRITER:
            result = in_connectivityAdminAddPeerWriter(
                admin,
                in_connectivityPeerWriter(peer->discoveredPeerEntity),
                peer->sequenceNumber);
            IN_TRACE_1(Send, 2, "in_channelSdpReaderProcessPeerEntity - writer result = %x", result);
        break;
        default:
            assert(FALSE);
            result = IN_RESULT_ERROR;
        break;
    }
    if(result == IN_RESULT_OK)
    {
        result = in_channelSdpWriterAddPeerEntity(sdpw, peer->discoveredPeerEntity);
    }
    in_channelSdpWriterFree(sdpw);
    in_channelFree(channel);
    return result;
}

in_result
in_channelSdpReaderProcessAckNackFunc(
    in_streamReaderCallbackArg _this,
    in_ddsiAckNack event,
    in_ddsiReceiver receiver)
{
    in_channelSdpReader sdpr;
    in_channel channel;
    in_channelSdpWriter sdpw;
    in_result result;

    assert(_this);
    assert(event);
    assert(receiver);

    IN_TRACE(Receive,3,"in_channelSdpReaderProcessAckNackFunc CALLBACK");

    sdpr = in_channelSdpReader(_this);
    channel = in_channelReaderGetChannel(in_channelReader(sdpr));
    sdpw = in_channelSdpWriter(in_channelGetWriter(channel));

    result = in_channelSdpWriterAddAckNack(sdpw, event, receiver);
    in_channelSdpWriterFree(sdpw);
    in_channelFree(channel);
    return result;
}

in_result
in_channelSdpReaderProcessHeartbeatFunc(
    in_streamReaderCallbackArg _this,
    in_ddsiHeartbeat event,
    in_ddsiReceiver receiver)
{
    in_result result = IN_RESULT_OK;
    in_channelSdpReader sdpr;
    in_channel channel;
    in_channelSdpWriter sdpw;

    assert(_this);
    assert(event);
    assert(receiver);

    sdpr = in_channelSdpReader(_this);
    channel = in_channelReaderGetChannel(in_channelReader(sdpr));
    sdpw = in_channelSdpWriter(in_channelGetWriter(channel));

    result = in_channelSdpWriterAddHeartbeatEvent(sdpw, event, receiver);
    in_channelSdpWriterFree(sdpw);
    in_channelFree(channel);
    return result;
}

os_boolean 
in_channelSdpReaderIsLocalEntityFunc(
	in_streamReaderCallbackArg _this,
	in_ddsiGuidPrefixRef guidPrefixRef)
{
	in_connectivityAdmin admin;
	os_boolean result = OS_FALSE;
	
	admin = in_connectivityAdminGetInstance();
	
	assert(admin);
    
	/* TODO, optme - could be cached */ 
	result = in_connectivityAdminIsLocalEntity(admin, guidPrefixRef);
	
	return result;
}

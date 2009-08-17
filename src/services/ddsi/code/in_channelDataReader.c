#include "os_heap.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_networkReader.h"
#include "in__configChannel.h"
#include "in__configDataChannel.h"
#include "in__ddsiPublication.h"
#include "in_commonTypes.h"
#include "in_streamReader.h"
#include "in_channel.h"
#include "in_channelReader.h"
#include "in_channelDataReader.h"
#include "in_report.h"
#include "in_connectivityAdmin.h"

static in_result
in_channelDataReaderProcessDataFunc(
    in_streamReaderCallbackArg _this,
    v_message message,
    in_connectivityPeerWriter peerWriter,
    in_ddsiReceiver receiver);

static in_result
in_channelDataReaderProcessAckNackFunc(
    in_streamReaderCallbackArg _this,
    in_ddsiAckNack event,
    in_ddsiReceiver receiver);

static os_boolean 
in_channelDataReaderIsLocalEntityFunc(
	in_streamReaderCallbackArg _this,
	in_ddsiGuidPrefixRef guidPrefixRef);

static void
in_channelDataReaderMain(
    v_entity e,
    c_voidp arg);

static void *
in_channelDataReaderMainFunc(
    in_runnable runnable);

static void
in_channelDataReaderTrigger(
    in_runnable runnable);

static void
in_channelDataReaderDeinit(
    in_object obj);

static os_boolean
in_channelDataReaderInit(
    in_channelDataReader _this,
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader);

OS_STRUCT(in_channelDataReader)
{
    OS_EXTENDS(in_channelReader);
    in_streamReader streamReader;
    u_networkReader userReader;
    v_networkReader kernelReader;
    os_time timeout;
};

static OS_STRUCT(in_streamReaderCallbackTable)
in_channelDataReaderCallbackTable =
{
        NULL, /* processPeerEntity*/
        in_channelDataReaderProcessDataFunc, /* processDataFunc */
        NULL, /* processDataFrag */
        in_channelDataReaderProcessAckNackFunc, /* processAckNack */
        NULL, /* processNackFrag */
        NULL, /* processHeartbeat */
        NULL, /* requestNackFrag */
        in_channelDataReaderIsLocalEntityFunc
};

in_channelDataReader
in_channelDataReaderNew(
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader)
{
    in_channelDataReader _this;
    os_boolean success;

    assert(channelData);
    assert(config);
    assert(reader);

    _this = (in_channelDataReader)os_malloc(sizeof(*_this));
    if(_this)
    {
        success = in_channelDataReaderInit(
            _this,
            channelData,
            config,
            reader);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
            IN_TRACE_1(Construction,2,"in_channelDataReader creation failed = %x",_this);
        } else
        {
            IN_TRACE_1(Construction, 2, "in_channelDataReader creation successful = %x", _this);
        }
    }
    return _this;
}

os_boolean
in_channelDataReaderInit(
    in_channelDataReader _this,
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader)
{
    os_boolean success;
    in_stream stream;

    assert(_this);
    assert(channelData);
    assert(config);
    assert(reader);

    success = in_channelReaderInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_DATA_CHANNEL_READER,
        in_channelDataReaderDeinit,
        in_configDataChannelGetName(in_configDataChannel(config)),
        in_configChannelGetPathName(config),
        in_channelDataReaderMainFunc,
        in_channelDataReaderTrigger,
        in_channel(channelData));

    if(success)
    {
        stream = in_channelGetStream(in_channel(channelData));
        _this->streamReader = in_streamGetReader(stream);
        in_streamFree(stream);
        _this->userReader = reader;
        _this->timeout.tv_sec = 1;
        _this->timeout.tv_nsec = 0;
    }
    return success;
}

void
in_channelDataReaderDeinit(
    in_object obj)
{
    in_channelDataReader _this;

    assert(obj);
    assert(in_channelDataReaderIsValid(obj));

    _this = (in_channelDataReader)obj;

    in_streamReaderFree(_this->streamReader);

    /*Call parent deinit*/
    in_channelReaderDeinit(obj);
}

in_result
in_channelDataReaderProcessDataFunc(
    in_streamReaderCallbackArg _this,
    v_message message,
    in_connectivityPeerWriter peerWriter,
    in_ddsiReceiver receiver)
{
    in_result result = IN_RESULT_ERROR;
    in_channelDataReader channelReader;
    v_kernel kernel = NULL;
    in_ddsiDiscoveredWriterData data;
    v_networkReaderEntry entry = NULL;
    v_writeResult kernelResult;

    assert(_this);
    assert(message);
    assert(peerWriter);
    assert(receiver);

    channelReader = in_channelDataReader(_this);
    IN_TRACE(Receive, 3, "in_channelDataReaderProcessDataFunc");
    kernel = v_objectKernel(channelReader->kernelReader);
    if ( kernel )
    {
        os_char* partitionName;
        v_group group = NULL;

        data = in_connectivityPeerWriterGetInfo(peerWriter);
        /* resolve the group, based on the topic- and partitionname from the
         * peerwriter
         * ToDo: currently we only write to the first partition in the
         * partition-array
         */

        partitionName =  (os_char*)(data->topicData.info.partition.name[0]);

	if(data->topicData.info.partition.name[0])
        {
            partitionName =  os_strdup(data->topicData.info.partition.name[0]);
        } else
        {
            /* If there was no partition listed, create the default empty string partition */
            partitionName = os_strdup("");
        }
        group = v_groupSetGet(
            kernel->groupSet,
            partitionName,
            data->topicData.info.topic_name);

        IN_TRACE_2(
            Receive,
            3,
            "in_channelDataReader Process Message (%s,%s)",
            partitionName,
            data->topicData.info.topic_name);
        os_free(partitionName);

        if ( group )
        {
            /* find the entry to obtain the networkId */
            entry = v_networkReaderLookupEntry(channelReader->kernelReader, group);
            if ( entry )
            {
                /* Write the v_message to the group */
                kernelResult = v_groupWrite(group, message, NULL, entry->networkId);
                if ( kernelResult == V_WRITE_SUCCESS )
                {
                   result = IN_RESULT_OK;
                }
            }
        }
    }
    return result;
}

in_result
in_channelDataReaderProcessAckNackFunc(
        in_streamReaderCallbackArg _this,
        in_ddsiAckNack event,
        in_ddsiReceiver receiver)
{
    IN_TRACE(Receive,3,"in_channelDataReaderProcessDataFuncFunc CALLBACK");
    return IN_RESULT_OK;
}

void
in_channelDataReaderMain(
    v_entity e,
    c_voidp arg)
{
    const os_uint32 ERROR_INTERVAL = 500;
    os_uint32 errorCounter = 0;

    os_time pollTimeout = {0,0};
    in_channelDataReader channelReader;
    in_result result = IN_RESULT_ERROR;

    assert(e);
    assert(arg);

    channelReader = in_channelDataReader(arg);
    channelReader->kernelReader = v_networkReader(e);
    while (!(int)in_runnableTerminationRequested((in_runnable)channelReader))
    {
        /* renew timeout */
        pollTimeout = channelReader->timeout;

        result = in_streamReaderScan(
            channelReader->streamReader,
            &in_channelDataReaderCallbackTable, /* static vtable */
            (in_streamReaderCallbackArg)channelReader,
            &pollTimeout);
        /** may return before timeout as exceeded, pollTimeout
         * contains the remaning time  */
        if (result != IN_RESULT_OK && result != IN_RESULT_TIMEDOUT)
        {
            if (errorCounter == 0)
            {
                /* first occurance */
                IN_REPORT_WARNING(IN_SPOT, "unexpected data read error");
            } else if (errorCounter >= ERROR_INTERVAL)
            {
                IN_REPORT_WARNING_1(
                    IN_SPOT,
                    "unexpected data read error (repeated %d times)",
                    errorCounter);
                errorCounter = 0;
            }
            ++errorCounter;
            /* POST: errorCounter >= 1 */
        }
    }
    channelReader->kernelReader = NULL;
}

void*
in_channelDataReaderMainFunc(
    in_runnable runnable)
{
    u_result result;
    in_channelDataReader channelReader;

    assert(runnable);

    channelReader = in_channelDataReader(runnable);

    in_runnableSetRunState(runnable, IN_RUNSTATE_RUNNING);
    result = u_entityAction(
        u_entity(channelReader->userReader),
        in_channelDataReaderMain,
        channelReader);
    in_runnableSetRunState(runnable, IN_RUNSTATE_TERMINATED);

    return NULL;
}

void
in_channelDataReaderTrigger(
    in_runnable runnable)
{
    /* TODO tbd */
}

os_boolean 
in_channelDataReaderIsLocalEntityFunc(
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

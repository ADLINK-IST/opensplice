#include "os_heap.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_networkReader.h"
#include "v_networkQueue.h"
#include "in__configDataChannel.h"
#include "in_commonTypes.h"
#include "in_streamWriter.h"
#include "in_connectivityAdmin.h"
#include "in_channel.h"
#include "in_channelWriter.h"
#include "in_channelDataWriter.h"
#include "in_report.h"
#include "in_endpointDiscoveryData.h"

static void
in_channelDataWriterMain(
    v_entity e,
    c_voidp arg);

static void*
in_channelDataWriterMainFunc(
    in_runnable runnable);

static void
in_channelDataWriterTrigger(
    in_runnable runnable);

static void
in_channelDataWriterDeinit(
    in_object obj);

static os_boolean
in_channelDataWriterInit(
    in_channelDataWriter _this,
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader,
    in_endpointDiscoveryData discoveryData);

OS_STRUCT(in_channelDataWriter)
{
    OS_EXTENDS(in_channelWriter);
    in_streamWriter streamWriter;
    u_networkReader userReader;
    v_networkReader kernelReader;
    c_ulong queueId;
    in_endpointDiscoveryData discoveryData;
};

in_channelDataWriter
in_channelDataWriterNew(
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader,
    in_endpointDiscoveryData discoveryData)
{
    in_channelDataWriter _this;
    os_boolean success;

    assert(channelData);
    assert(config);
    assert(reader);
    assert(discoveryData);
    assert(reader);

    _this = in_channelDataWriter(os_malloc(OS_SIZEOF(in_channelDataWriter)));
    if(_this)
    {
        success = in_channelDataWriterInit(
            _this,
            channelData,
            config,
            reader,
            discoveryData);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
            IN_TRACE_1(Construction,2,"in_channelDataWriter creation failed = %x",_this);
        } else
        {
            IN_TRACE_1(Construction,2,"in_channelDataWriter creation successful = %x",_this);
        }
    }
    return _this;
}

os_boolean
in_channelDataWriterInit(
    in_channelDataWriter _this,
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader,
    in_endpointDiscoveryData discoveryData)
{
    os_boolean success;
    u_result ures;
    c_time resolution = {0,20000000}; /* TODO hardcoded for now */
    in_stream stream;

    assert(_this);
    assert(channelData);
    assert(config);
    assert(reader);
    assert(discoveryData);

    success = in_channelWriterInit(
        OS_SUPER(_this),
        in_channel(channelData),
        IN_OBJECT_KIND_DATA_CHANNEL_WRITER,
        in_channelDataWriterDeinit,
        in_configDataChannelGetName(in_configDataChannel(config)),
        in_configChannelGetPathName(config),
        in_channelDataWriterMainFunc,
        in_channelDataWriterTrigger);

    if(success)
    {
        stream = in_channelGetStream(in_channel(channelData));
        _this->streamWriter = in_streamGetWriter(stream);
        in_streamFree(stream);
        _this->userReader = reader;
        _this->discoveryData = in_endpointDiscoveryDataKeep(discoveryData);
        /* connect to darareader and init queueId */
        ures = u_networkReaderCreateQueue(
            reader,
            in_configDataChannelGetGroupQueueSize(in_configDataChannel(config)),
            in_configDataChannelGetPriority(in_configDataChannel(config)),
            FALSE, /* reliability */
            FALSE, /* p2p */
            resolution,
            in_configDataChannelGetIsDefault(in_configDataChannel(config)),
            &_this->queueId,
            in_configDataChannelGetName(in_configDataChannel(config)));
        if (ures != U_RESULT_OK)
        {
            success = OS_FALSE;
        }
    }
    return success;
}

void
in_channelDataWriterDeinit(
    in_object obj)
{
    in_channelDataWriter _this;

    assert(obj);
    assert(in_channelDataWriterIsValid(obj));

    _this = in_channelDataWriter(obj);
    in_endpointDiscoveryDataFree(_this->discoveryData);
    in_streamWriterFree(_this->streamWriter);

    /*Call parent deinit*/
    in_channelWriterDeinit(obj);
}

void
in_channelDataWriterMain(
    v_entity e,
    c_voidp arg)
{
    in_channelDataWriter channelWriter;
    in_connectivityWriterFacade facade = NULL;
    v_message message = NULL;
    Coll_List *locators;
    v_networkQueue queue;
    c_bool more = TRUE;
    v_networkReaderWaitResult waitResult;

    assert(e);
    assert(arg);

    channelWriter = in_channelDataWriter(arg);
    channelWriter->kernelReader = v_networkReader(e);
    while (!(int)in_runnableTerminationRequested((in_runnable)channelWriter))
    {
        /* Wait for message from Queue */
        waitResult = v_networkReaderWait(
            channelWriter->kernelReader,
            channelWriter->queueId,
            &queue);
        if (waitResult & V_WAITRESULT_MSGWAITING)
        {
            more = TRUE;
            while (more && !(int)in_runnableTerminationRequested((in_runnable)channelWriter))
            {
                v_networkReaderEntry dummy_entry;
                c_ulong dummy_sequenceNumber;
                v_gid dummy_sender;
                c_bool dummy_sendTo;
                v_gid dummy_receiver;
                c_time dummy_sendBefore;
                c_ulong dummy_priority;

                /* Read message from the Queue */
                v_networkQueueTakeFirst(
                    queue,
                    &message,
                    &dummy_entry,
                    &dummy_sequenceNumber,
                    &dummy_sender,
                    &dummy_sendTo,
                    &dummy_receiver,
                    &dummy_sendBefore,
                    &dummy_priority,
                    &more);

                /* Resolve Writerfacade for this v_message */
                facade = in_connectivityAdminFindWriter(
                    in_connectivityAdminGetInstance(),
                    message);

                if (facade)
                {
                    /* Obtain locator list from connectivity admin for this facade */
                    locators = in_connectivityWriterFacadeGetLocators(facade);
                    /* Write message to Stream with the facade and the locatorlist */
                    in_streamWriterAppendData(
                        channelWriter->streamWriter,
                        message,
                        channelWriter->discoveryData,
                        facade,
                        TRUE, /* recipient expects inlineQos */
                        locators);
                    /* Flush the stream */
                    in_streamWriterFlush(
                        channelWriter->streamWriter,
                        locators);
                    in_connectivityWriterFacadeFree(facade);
                }
                if(message)
                {
                    c_free(message);
                }
            }
        }
    }
    channelWriter->kernelReader = NULL;
}

void*
in_channelDataWriterMainFunc(
    in_runnable runnable)
{
    u_result result;
    in_channelDataWriter channelWriter;

    assert(runnable);

    channelWriter = in_channelDataWriter(runnable);

    in_runnableSetRunState(runnable, IN_RUNSTATE_RUNNING);
    result = u_entityAction(
        u_entity(channelWriter->userReader),
        in_channelDataWriterMain,
        channelWriter);
    in_runnableSetRunState(runnable, IN_RUNSTATE_TERMINATED);

    return NULL;
}

void
in_channelDataWriterTrigger(
    in_runnable runnable)
{
    /* TODO tbd */
}

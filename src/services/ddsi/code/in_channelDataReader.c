#include "os_heap.h"
#include "v_group.h"
#include "v_groupSet.h"
#include "v_networkReader.h"
#include "in__configDataChannel.h"
#include "in_commonTypes.h"
#include "in_streamReader.h"
#include "in_channel.h"
#include "in_channelReader.h"
#include "in_channelDataReader.h"

OS_STRUCT(in_channelDataReader){
    OS_EXTENDS(in_channelReader);
    in_streamReader streamReader;
    u_networkReader userReader;
    v_networkReader kernelReader;
    in_streamReaderCallbackTable callbackTable;
    os_time timeout;
};

static in_result
in_channelDataReaderProcessDataFunc(
    in_streamReaderCallbackArg _this,
    v_message message,
    in_connectivityPeerWriter peerWriter,
    in_ddsiReceiver receiver)
{
    in_result result = IN_RESULT_ERROR;
    in_channelDataReader channelReader = in_channelDataReader(_this);
    v_kernel kernel = NULL;

    kernel = v_objectKernel(channelReader->kernelReader);
    if ( kernel ){
        v_group group = NULL;

        /* resolve the group, based on the topic- and partitionname from the peerwriter */
        /* ToDo: currently we only write to the first partition in the partition-array */
        group = v_groupSetGet(
                    kernel->groupSet,
                    (c_string)(in_connectivityPeerWriterGetInfo(peerWriter)->partition.name),
                    in_connectivityPeerWriterGetInfo(peerWriter)->topic_name);

        if ( group ) {
            v_networkReaderEntry entry = NULL;

            /* find the entry to obtain the networkId */
            entry = v_networkReaderLookupEntry(channelReader->kernelReader, group);
            if ( entry ) {
                v_writeResult kernelResult;

                /* Write the v_message to the group */
                kernelResult = v_groupWrite(group, message, NULL, entry->networkId);

                if ( kernelResult == V_WRITE_SUCCESS ){
                   result = IN_RESULT_OK;
                }
            }
        }
    }
    return result;
}


/** main of the in_runnable
*/
static void
in_channelDataReaderMain(
    v_entity e,
    c_voidp arg)
{
    in_channelDataReader channelReader;

    channelReader = in_channelDataReader(arg);
    channelReader->kernelReader = v_networkReader(e);

    while (!(int)in_runnableTerminationRequested((in_runnable)channelReader)) {

        in_streamReaderScan(channelReader->streamReader,channelReader->callbackTable,(in_streamReaderCallbackArg)channelReader, &(channelReader->timeout));
    }
    channelReader->kernelReader = NULL;
}

/** mainfunc of the in_runnable
*/
static void *
in_channelDataReaderMainFunc(
    in_runnable runnable)
{
    u_result result;
    in_channelDataReader channelReader = in_channelDataReader(runnable);

    in_runnableSetRunState(runnable, IN_RUNSTATE_RUNNING);
    result = u_entityAction(u_entity(channelReader->userReader),
                            in_channelDataReaderMain, channelReader);
    in_runnableSetRunState(runnable, IN_RUNSTATE_TERMINATED);

    return NULL;
}

static void
in_channelDataReaderDeinit(
    in_object obj)
{
    in_channelDataReader _this;

    assert(in_channelDataReaderIsValid(obj));

    _this = (in_channelDataReader)obj;

    in_streamReaderFree(_this->streamReader);
    os_free(_this->callbackTable);

    /*Call parent deinit*/
    in_channelReaderDeinit(obj);
}

static os_boolean
in_channelDataReaderInit(
    in_channelDataReader _this,
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader)
{
    os_boolean success;

    assert(_this);
    assert(channelData);


    success = in_channelReaderInit(
        OS_SUPER(_this),
        IN_OBJECT_KIND_DATA_CHANNEL_READER,
        in_channelDataReaderDeinit,
        in_configDataChannelGetName(in_configDataChannel(config)),
        in_configChannelGetPathName(config),
        in_channelDataReaderMainFunc,
        NULL /* triggerfunc*/,
        in_channel(channelData));

    if(success)
    {
        _this->streamReader = in_streamGetReader(in_channelGetStream(in_channel(channelData)));
        _this->userReader = reader;
        _this->timeout.tv_sec = 1;
        _this->timeout.tv_nsec = 0;


        /* Init callback table */
        _this->callbackTable = (in_streamReaderCallbackTable)os_malloc((os_uint32)sizeof(*_this->callbackTable));
        _this->callbackTable->processPeerEntity = NULL;

        _this->callbackTable->processData = in_channelDataReaderProcessDataFunc;
        _this->callbackTable->processDataFrag = NULL;
        _this->callbackTable->processAckNack = NULL;
        _this->callbackTable->processNackFrag = NULL;
        _this->callbackTable->processHeartbeat = NULL;
        _this->callbackTable->requestNackFrag = NULL;
    }
    return success;
}



/* ------------------------------- Public ----------------------------------- */

in_channelDataReader
in_channelDataReaderNew(
    in_channelData channelData,
    in_configChannel config,
    u_networkReader reader)
{
    in_channelDataReader _this;
    os_boolean success;

    _this = (in_channelDataReader)os_malloc((os_uint32)sizeof(*_this));

    if(_this)
    {
        success = in_channelDataReaderInit(
            _this,
            channelData,config,reader);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}



#include "in_controller.h"
#include "in__config.h"
#include "in__configChannel.h"
#include "in_report.h"
#include "in_factory.h"
#include "in_stream.h"
#include "in_transport.h"
#include "u_participant.h"
#include "u_subscriber.h"
#include "u_subscriberQos.h"
#include "in_endpointDiscoveryData.h"

static os_boolean
in_controllerInit(
    in_controller _this,
    u_service service);

static void
in_controllerDeinit(
    in_controller _this);

static void
in_controllerCreateDataChannels(
    in_controller _this);

static void
in_controllerCreateDiscoveryChannel(
    in_controller _this);

/* Will block execution of the controller-thread for a specific amount of time defined in the 
 * config. */
static void
in_controllerWaitForDebugger(
    in_controller _this);

static os_boolean
in_controllerAddTransportToDiscoveryData(
    in_endpointDiscoveryData discoveryData,
    in_transport transport,
    os_boolean addMetaLocator);

OS_CLASS(in_controllerChannelData);

OS_STRUCT(in_controller)
{
    u_service service;
    u_subscriber subscriber;
    u_networkReader reader;
    in_plugKernel plug;
    Coll_List channels;
    in_endpointDiscoveryData discoveryData;
    in_controllerChannelData discoveryChannel;
};


OS_STRUCT(in_controllerChannelData)
{
    in_transport transport;
    in_stream stream;
    in_channel channel;
    in_plugKernel plug;
};

in_controller
in_controllerNew(
    u_service service)
{
    in_controller _this;
    os_boolean success;

    assert(service);

    _this = os_malloc(sizeof(OS_STRUCT(in_controller)));
    if(_this)
    {
        success = in_controllerInit(
            _this,
            service);
        if(!success)
        {
            os_free(_this);
            _this = NULL;
            IN_TRACE(Construction, 2, "in_controller creation failed");
        } else
        {
            IN_TRACE_1(Construction, 2 , "in_controller created = %x",_this);
        }
    }
    return _this;
}

os_boolean
in_controllerInit(
    in_controller _this,
    u_service service)
{
    v_subscriberQos subscriberQos;

    assert(_this);
    assert(service);

    _this->service = service;
    _this->discoveryChannel = NULL;

    subscriberQos = u_subscriberQosNew(NULL);

    /* Do not autoconnect, but react on newGroup notifications */
    os_free(subscriberQos->partition);
    subscriberQos->partition = NULL;

    _this->subscriber = u_subscriberNew(u_participant(service), "DDSi Subscriber", subscriberQos, TRUE);
    _this->reader = u_networkReaderNew(_this->subscriber, "DDSi Reader", NULL, TRUE);
    Coll_List_init(&(_this->channels));
    _this->discoveryData = in_endpointDiscoveryDataNew();
    _this->plug = in_plugKernelNew(service);

    os_free(subscriberQos);

    return OS_TRUE;
}

void
in_controllerFree(
    in_controller _this)
{
    assert(_this);

    in_controllerDeinit(_this);
    os_free(_this);
}

void
in_controllerDeinit(
    in_controller _this)
{
    in_controllerChannelData data;

    assert(_this);

    while(Coll_List_getNrOfElements(&(_this->channels)) > 0)
    {
        data = (in_controllerChannelData)Coll_List_popBack(&(_this->channels));
        in_objectFree(in_object(data->channel));
        in_objectFree(in_object(data->stream));
        in_objectFree(in_object(data->transport));
        in_plugKernelFree(data->plug);
       
        os_free(data);
    }
    if(_this->discoveryChannel)
    {
    	in_objectFree(in_object(_this->discoveryChannel->channel));
    	in_objectFree(in_object(_this->discoveryChannel->stream));
        in_objectFree(in_object(_this->discoveryChannel->transport));
        in_plugKernelFree(_this->discoveryChannel->plug);
        os_free(_this->discoveryChannel);
    }
    if (_this->discoveryData) {
        in_endpointDiscoveryDataFree(_this->discoveryData);
    }
    if(_this->plug)
    {
    	in_plugKernelFree(_this->plug);
    }
    u_networkReaderFree(_this->reader);
    u_subscriberFree(_this->subscriber);
}

void
in_controllerStart(
    in_controller _this)
{
    Coll_Iter* iterator;
    in_controllerChannelData channelData;

    assert(_this);

    /* wait for the debugger to attach to this process */
    in_controllerWaitForDebugger(_this);
    
    in_controllerCreateDiscoveryChannel(_this);
    in_controllerCreateDataChannels(_this);

    /* start up the discovery channel, verify the discovery has been "enabled" */
    if (_this->discoveryChannel) {
    	in_channelActivate(_this->discoveryChannel->channel);
    }

    iterator = Coll_List_getFirstElement(&(_this->channels));
    while(iterator)
    {
        channelData = (in_controllerChannelData)Coll_Iter_getObject(iterator);
        /* start up the data channel */
        in_channelActivate(channelData->channel);
        iterator = Coll_Iter_getNext(iterator);
    }

    u_networkReaderRemoteActivityDetected(_this->reader);
    IN_TRACE(Construction,2,"in_controller started");
}

void
in_controllerStop(
    in_controller _this)
{
    in_controllerChannelData channelData;
    Coll_Iter* iterator;

    assert(_this);

    u_networkReaderRemoteActivityLost(_this->reader);

    /* stop the discovery channel, verify the discovery channel has been enabled */
    if (_this->discoveryChannel) {
    	in_channelDeactivate(_this->discoveryChannel->channel);
    }
    
    iterator = Coll_List_getFirstElement(&(_this->channels));
    while(iterator)
    {
        channelData = (in_controllerChannelData)Coll_Iter_getObject(iterator);

        /* stop the data channel */
        in_channelDeactivate(channelData->channel);

        iterator = Coll_Iter_getNext(iterator);
    }
    IN_TRACE(Construction,2,"in_controller stopped");
}

os_boolean
in_controllerAddTransportToDiscoveryData(
    in_endpointDiscoveryData discoveryData,
    in_transport transport,
    os_boolean addMetaLocator)
{
    os_boolean result = OS_TRUE;
    in_transportReceiver rec;
    in_locator mcastLoc;
    in_locator ucastLoc;
    in_locator metatrafficUcastLoc;
    in_locator metatrafficMcastLoc;

    rec = in_transportGetReceiver(transport);
    mcastLoc = in_transportReceiverGetDataMulticastLocator(rec);
    ucastLoc = in_transportReceiverGetDataUnicastLocator(rec);
    metatrafficUcastLoc = in_transportReceiverGetCtrlUnicastLocator(rec);
    metatrafficMcastLoc = in_transportReceiverGetCtrlMulticastLocator(rec);

    in_transportReceiverFree(rec);

    if (!mcastLoc || !ucastLoc)
    {
        if(mcastLoc)
        {
        	in_locatorFree(mcastLoc);
        }
        if(ucastLoc)
        {
        	in_locatorFree(ucastLoc);
        }
        result = OS_FALSE;
    } else
    {
        if(!addMetaLocator)
        {
			in_endpointDiscoveryDataAddDefaultMulticastLocator(discoveryData,
				mcastLoc);
			in_endpointDiscoveryDataAddDefaultUnicastLocator(discoveryData,
				ucastLoc);
        }
        if(addMetaLocator)
        {
            /* explicit metatraffic resource only for unicast, the
             * multicast metatraffic shall go via the default mcast locator */
            in_endpointDiscoveryDataAddMetatrafficUnicastLocator(
                discoveryData,
                metatrafficUcastLoc);
            in_endpointDiscoveryDataAddMetatrafficMulticastLocator(
                discoveryData,
                metatrafficMcastLoc);
        }
        in_locatorFree(mcastLoc);
        in_locatorFree(ucastLoc);
        result = OS_TRUE;
    }
    in_locatorFree(metatrafficUcastLoc);
    in_locatorFree(metatrafficMcastLoc);

    return result;
}

void
in_controllerCreateDataChannels(
    in_controller _this)
{
    in_config config;
    os_char* serviceName;
    in_configDdsiService ddsiService;
    Coll_List* dataChannelConfigs;
    Coll_Iter* iterator;
    in_configChannel dataChannelConfig;
    in_controllerChannelData channelData;
    os_uint32 retCode;

    assert(_this);

    config = in_configGetInstance();
    serviceName = u_serviceGetName(_this->service);
    ddsiService = in_configGetDdsiServiceByName(config, serviceName);
    dataChannelConfigs = in_configDdsiServiceGetChannels(ddsiService);

    IN_TRACE_1(Construction,2,"in_controllerCreateDataChannels will create %d datachannels", Coll_List_getNrOfElements(dataChannelConfigs));
    iterator = Coll_List_getFirstElement(dataChannelConfigs);
    while(iterator)
    {
        in_endpointDiscoveryData channelSpecificEndpoint = NULL;
        dataChannelConfig = in_configChannel(Coll_Iter_getObject(iterator));

        IN_TRACE_1(Construction,2,"CREATE DATA channel %s ",in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
        if (in_configChannelIsEnabled(in_configChannel(dataChannelConfig))) {

            channelData = os_malloc(sizeof(OS_STRUCT(in_controllerChannelData)));

            IN_TRACE_1(Construction,2,"CREATE DATA channel %s(create transport) ",in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
            if(!channelData)
            {
                IN_REPORT_ERROR_1(IN_SPOT, "Initializing data channel failed for %s",
                        in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                break;
            }
            channelData->transport = in_factoryCreateTransport(dataChannelConfig);
            if(!channelData->transport)
            {
                os_free(channelData);
                IN_REPORT_ERROR_1(IN_SPOT, "Initializing transport failed for %s",
                        in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                break;
            } else
            {
                IN_TRACE_1(Construction,2,"CREATE DATA channel %s(create plugkernel) ",in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                channelSpecificEndpoint =
                    in_endpointDiscoveryDataNew();

                channelData->plug = in_plugKernelKeep(_this->plug);

                if (!channelData->plug ||
                    !channelSpecificEndpoint ||
                    !in_controllerAddTransportToDiscoveryData(_this->discoveryData, channelData->transport, OS_FALSE) ||
                    !in_controllerAddTransportToDiscoveryData(channelSpecificEndpoint, channelData->transport, OS_FALSE))
                {
                    IN_REPORT_ERROR_1(IN_SPOT, "Initializing data channel endpoints failed for %s",
                            in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));

                    in_transportFree(channelData->transport);
                    in_streamFree(channelData->stream);
                    if (channelSpecificEndpoint) {
                        in_endpointDiscoveryDataFree(channelSpecificEndpoint);
                    }
                    os_free(channelData);
                    break;
                } else
                {
                    IN_TRACE_1(Construction,2,"CREATE DATA channel %s(create stream) ",in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                    channelData->stream = in_factoryCreateStream(
                        dataChannelConfig,
                        channelData->plug,
                        channelData->transport);

                    if(!channelData->stream)
                    {
                        in_transportFree(channelData->transport);
                        os_free(channelData);

                        IN_REPORT_ERROR_1(IN_SPOT, "Initializing data channel failed for %s",
                                  in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                        break;
                    } else
                    {
                        IN_TRACE_1(Construction,2,"CREATE DATA channel %s(create channel) ",in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                        channelData->channel = in_factoryCreateChannel(
                            dataChannelConfig,
                            channelData->plug,
                            channelData->stream,
                            channelSpecificEndpoint);

                        if(!channelData->channel)
                        {
                            in_transportFree(channelData->transport);
                            in_streamFree(channelData->stream);
                            os_free(channelData);
                            IN_REPORT_ERROR_1(IN_SPOT, "Initializing data channel failed for %s",
                                    in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                            break;
                        }
                    }
                }
                in_endpointDiscoveryDataFree(channelSpecificEndpoint);
            }
            retCode = Coll_List_pushBack(&_this->channels, channelData);
            if(retCode != COLL_OK)
            {
                in_channelFree(channelData->channel);
                in_streamFree(channelData->stream);
                in_transportFree(channelData->transport);
                os_free(channelData);
                IN_REPORT_ERROR_1(IN_SPOT, "Initializing data channel failed for %s",
                           in_configDataChannelGetName(in_configDataChannel(dataChannelConfig)));
                break;
            }
        }
        iterator = Coll_Iter_getNext(iterator);
    }
    os_free(serviceName);
}


void
in_controllerCreateDiscoveryChannel(
    in_controller _this)
{
    in_config config = NULL;
    in_configDdsiService ddsiService = NULL;
    in_configChannel discoveryChannelConfig = NULL;
    os_char* serviceName = NULL;
    in_controllerChannelData channelData = NULL;

    assert(_this);

    config = in_configGetInstance();
    serviceName = u_serviceGetName(_this->service);
    ddsiService = in_configGetDdsiServiceByName(config, serviceName);
    os_free(serviceName);

    discoveryChannelConfig = in_configChannel(in_configDdsiServiceGetDiscoveryChannel(ddsiService));

    if (!in_configChannelIsEnabled(discoveryChannelConfig)) {
    	IN_REPORT_WARNING(IN_SPOT, "The discovery protocol has been disabled, "
								   "no auto-discovery or peers provided.");
    } else {
        IN_TRACE(Construction,2,"CREATE DISCOVERY channel");

        channelData = os_malloc(sizeof(OS_STRUCT(in_controllerChannelData)));
        if(!channelData)
        {
            IN_REPORT_ERROR_1(IN_SPOT, "Initializing discovery channel failed for %s",
                    in_configDataChannelGetName(in_configDataChannel(discoveryChannelConfig)));
        }
        channelData->transport = in_factoryCreateTransport(discoveryChannelConfig);
        if(!channelData->transport ||
           !in_controllerAddTransportToDiscoveryData(
               _this->discoveryData,
               channelData->transport,
               OS_TRUE))
        {
            os_free(channelData);
            channelData = NULL;
            IN_REPORT_ERROR_1(IN_SPOT, "Initializing discovery channel failed for %s",
                     in_configDataChannelGetName(in_configDataChannel(discoveryChannelConfig)));
        } else
        {
            channelData->plug = in_plugKernelKeep(_this->plug);

            if(!channelData->plug)
            {
                IN_REPORT_ERROR_1(IN_SPOT, "Initializing discovery channel failed for %s",
                         in_configDataChannelGetName(in_configDataChannel(discoveryChannelConfig)));
                in_transportFree(channelData->transport);
                in_streamFree(channelData->stream);
                os_free(channelData);
                channelData = NULL;
            } else
            {
                channelData->stream = in_factoryCreateStream(
                    discoveryChannelConfig,
                    channelData->plug,
                    channelData->transport);
                if(!channelData->stream)
                {
                    in_transportFree(channelData->transport);
                    os_free(channelData);
                    channelData = NULL;
                    IN_REPORT_ERROR_1(IN_SPOT, "Initializing discovery channel failed for %s",
                             in_configDataChannelGetName(in_configDataChannel(discoveryChannelConfig)));
                } else
                {
                    channelData->channel = in_factoryCreateDiscoveryChannel(
                        in_configDiscoveryChannel(discoveryChannelConfig),
                        channelData->plug,
                        channelData->stream,
                        _this->discoveryData);

                    if(!channelData->channel)
                    {
                        in_transportFree(channelData->transport);
                        in_streamFree(channelData->stream);
                        os_free(channelData);
                        channelData = NULL;
                        IN_REPORT_ERROR_1(IN_SPOT, "Initializing discovery channel failed for %s",
                                 in_configDataChannelGetName(in_configDataChannel(discoveryChannelConfig)));
                    }
                }
            }
        }
    }
    _this->discoveryChannel = channelData;
}

void
in_controllerWaitForDebugger(
    in_controller _this)
{
	in_config config = NULL;                 /* referencing singleton */
	in_configDdsiService ddsiService = NULL; /* referencing singleton */
	in_configDebug ddsiDebug = NULL;         /* referencing singleton */
	os_char* serviceName = NULL;
	os_uint32 waitTime = 0; /* seconds */
	os_time secondSleep = {1L,0L};

	assert(_this);

	config = in_configGetInstance();
	serviceName = u_serviceGetName(_this->service);
	assert(serviceName);
	
	ddsiService = in_configGetDdsiServiceByName(config, serviceName);
	assert(ddsiService);
	
	ddsiDebug = in_configDdsiServiceGetDebugging(ddsiService);
	
	if (!ddsiDebug) {
		IN_REPORT_INFO(3, "No debug settings");
	} else {
		waitTime = in_configDebugGetWaitTime(ddsiDebug);
		
		IN_REPORT_INFO_1(3, "WaitForDebugger defined as %d seconds", waitTime);
		while(waitTime-- > 0) {
			IN_REPORT_INFO_1(3, "WaitForDebugger remaining %d seconds", waitTime);
			os_nanoSleep(secondSleep);
		}
	}
 	os_free(serviceName);
}



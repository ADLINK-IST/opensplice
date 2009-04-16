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
#include "in_controller.h"
#include "in__config.h"
#include "in__configChannel.h"
#include "in_factory.h"
#include "in_streamPair.h"

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

OS_CLASS(in_controllerChannelData);

OS_STRUCT(in_controller)
{
    u_service service;
    Coll_List channels;
    in_controllerChannelData discoveryChannel;
};


OS_STRUCT(in_controllerChannelData)
{
    in_transport transport;
    in_streamPair stream;
    in_channel channel;
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
        }
    }
    return _this;
}

os_boolean
in_controllerInit(
    in_controller _this,
    u_service service)
{
    assert(_this);
    assert(service);

    _this->service = service;
    Coll_List_init(&(_this->channels));

    return OS_TRUE;
}

void
in_controllerFree(
    in_controller _this)
{
    assert(_this);

    in_controllerDeinit(_this);
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
        os_free(data->transport);
        os_free(data->stream);
        os_free(data->channel);
        os_free(data);
    }

    os_free(_this->discoveryChannel->transport);
    os_free(_this->discoveryChannel->stream);
    os_free(_this->discoveryChannel->channel);
    os_free(_this->discoveryChannel);
}

void
in_controllerStart(
    in_controller _this)
{
    Coll_Iter* iterator;
    in_controllerChannelData channelData;

    assert(_this);

    in_controllerCreateDataChannels(_this);
    in_controllerCreateDiscoveryChannel(_this);

    /* TODO add code to start up the discovery channel */

    iterator = Coll_List_getFirstElement(&(_this->channels));
    while(iterator)
    {
        channelData = (in_controllerChannelData)Coll_Iter_getObject(iterator);
        /* TODO add code to start up the data channel */
        iterator = Coll_Iter_getNext(iterator);
    }
}

void
in_controllerStop(
    in_controller _this)
{
    in_controllerChannelData channelData;
    Coll_Iter* iterator;

    assert(_this);

    /* TODO add code to stop the discovery channel */

    iterator = Coll_List_getFirstElement(&(_this->channels));
    while(iterator)
    {
        channelData = (in_controllerChannelData)Coll_Iter_getObject(iterator);
        /* TODO add code to stop the data channel */
        iterator = Coll_Iter_getNext(iterator);
    }
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
    iterator = Coll_List_getFirstElement(dataChannelConfigs);
    while(iterator)
    {
        dataChannelConfig = in_configChannel(Coll_Iter_getObject(iterator));
        channelData = os_malloc(sizeof(OS_STRUCT(in_controllerChannelData)));
        if(!channelData)
        {
            /* TODO report error */
            break;
        }
        channelData->transport = in_factoryCreateTransport(dataChannelConfig);
        if(!channelData->transport)
        {
            os_free(channelData);
            /* TODO report error */
            break;
        } else
        {
            channelData->stream = in_factoryCreateStream(dataChannelConfig);
            if(!channelData->stream)
            {
                in_transportFree(channelData->transport);
                os_free(channelData);
                /* TODO report error */
                break;
            } else
            {
                channelData->channel = in_factoryCreateChannel(dataChannelConfig);
                if(!channelData->channel)
                {
                    in_transportFree(channelData->transport);
                    in_streamPairFree(channelData->stream);
                    os_free(channelData);
                    /* TODO report error */
                    break;
                }
            }
        }
        retCode = Coll_List_pushBack(&_this->channels, channelData);
        if(retCode != COLL_OK)
        {
            in_transportFree(channelData->transport);
            in_streamPairFree(channelData->stream);
            in_channelFree(channelData->channel);
            os_free(channelData);
           /* TODO report error */
            break;
        }
        iterator = Coll_Iter_getNext(iterator);
    }

    os_free(serviceName);
}

void
in_controllerCreateDiscoveryChannel(
    in_controller _this)
{
    in_config config;
    in_configDdsiService ddsiService;
    in_configChannel discoveryChannelConfig;
    os_char* serviceName;
    in_controllerChannelData channelData;

    assert(_this);

    config = in_configGetInstance();
    serviceName = u_serviceGetName(_this->service);
    ddsiService = in_configGetDdsiServiceByName(config, serviceName);

    discoveryChannelConfig = in_configChannel(in_configDdsiServiceGetDiscoveryChannel(ddsiService));

    channelData = os_malloc(sizeof(OS_STRUCT(in_controllerChannelData)));
    if(!channelData)
    {
        /* TODO report error */
    }
    channelData->transport = in_factoryCreateTransport(discoveryChannelConfig);
    if(!channelData->transport)
    {
        os_free(channelData);
        /* TODO report error */
    } else
    {
        channelData->stream = in_factoryCreateStream(discoveryChannelConfig);
        if(!channelData->stream)
        {
            in_transportFree(channelData->transport);
            os_free(channelData);
            /* TODO report error */
        } else
        {
            channelData->channel = in_factoryCreateChannel(discoveryChannelConfig);
            if(!channelData->channel)
            {
                in_transportFree(channelData->transport);
                in_streamPairFree(channelData->stream);
                os_free(channelData);
                /* TODO report error */
            }
        }
    }
    _this->discoveryChannel = channelData;
}

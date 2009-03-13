#include "in__plugKernel.h"
#include "u_entity.h"
#include "u_participant.h"
#include "v_participant.h"
#include "v_subscriber.h"
#include "os_heap.h"

OS_STRUCT(in_plugKernel)
{
    OS_EXTENDS(in_object);
    u_service service;
    u_networkReader reader;
};

static os_boolean
in_plugKernelInit(
    in_plugKernel _this,
    u_service service);

static void
in_plugKernelDeinit(
    in_object object);

static void
in_plugKernelResolveNetworkReader(
    v_entity entity,
    c_voidp args);

static c_bool
in_plugKernelResolveSubscriber(
    c_object o,
    c_voidp arg);

static c_bool
in_plugKernelResolveReader(
    c_object o,
    c_voidp arg);

in_plugKernel
in_plugKernelNew(
    u_service service)
{
    os_boolean success;
    in_plugKernel plug;

    assert(service);

    plug = in_plugKernel(os_malloc(OS_SIZEOF(in_plugKernel)));

    if(plug)
    {
        success = in_plugKernelInit(plug, service);

        if(!success)
        {
            os_free(plug);
            plug = NULL;
        }
    }
    return plug;
}

static os_boolean
in_plugKernelInit(
    in_plugKernel _this,
    u_service service)
{
    os_boolean success;
    u_result result;
    u_networkReader reader;

    assert(_this);
    assert(service);

    success = in_objectInit(
            in_object(_this),
            IN_OBJECT_KIND_PLUG_KERNEL,
            in_plugKernelDeinit);

    if(success)
    {
        _this->service = service;
        result = u_entityAction(u_entity(_this->service), in_plugKernelResolveNetworkReader, &reader);

        if(result == U_RESULT_OK)
        {
            _this->reader = (u_networkReader)(u_entityNew(v_entity(reader),
                    u_participant(_this->service), FALSE));
        } else
        {
            in_objectDeinit(in_object(_this));
        }

    }
    return success;
}

static void
in_plugKernelDeinit(
    in_object object)
{
    in_plugKernel _this;

    assert(in_plugKernelIsValid(object));

    _this = in_plugKernel(object);

    if(_this->reader)
    {
        u_entityFree(u_entity(_this->reader));
    }
    in_objectDeinit(object);
}

u_service
in_plugKernelGetService(
    in_plugKernel _this)
{
    assert(in_plugKernelIsValid(_this));
    return _this->service;
}

u_networkReader
in_plugKernelGetNetworkReader(
    in_plugKernel _this)
{
    assert(in_plugKernelIsValid(_this));

    return _this->reader;
}

static void
in_plugKernelResolveNetworkReader(
    v_entity entity,
    c_voidp args)
{
    v_networkReader* reader;
    v_subscriber* subscriber;

    reader = (v_networkReader*)args;
    c_walk(v_participant(entity)->entities, in_plugKernelResolveSubscriber, subscriber);

    if(subscriber){
        c_walk((*subscriber)->readers, in_plugKernelResolveReader, args);
    } else {
        *reader = NULL;
    }
    return;
}

static c_bool
in_plugKernelResolveSubscriber(
    c_object o,
    c_voidp arg)
{
    v_subscriber* subscriber;
    c_bool cont;

    if(v_object(o)->kind == K_SUBSCRIBER)
    {
        subscriber = (v_subscriber*)arg;
        *subscriber = v_subscriber(o);
        cont = FALSE;
    } else
    {
        cont = TRUE;
    }
    return cont;
}

static c_bool
in_plugKernelResolveReader(
    c_object o,
    c_voidp arg)
{
    v_networkReader* reader;
    c_bool cont;

    if(v_object(o)->kind == K_NETWORKREADER)
    {
        reader = (v_networkReader*)arg;
        *reader = o;
        cont = FALSE;
    } else
    {
        cont = TRUE;
    }
    return cont;
}


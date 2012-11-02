#include "in__plugKernel.h"
#include "u_entity.h"
#include "u_participant.h"
#include "v_participant.h"
#include "v_subscriber.h"
#include "os_heap.h"
#include "in_report.h"
#include "v_topic.h"
#include "c_typebase.h"

OS_STRUCT(in_plugKernel)
{
    OS_EXTENDS(in_object);
    u_service service;
    u_networkReader reader;
    c_base base;
    /* ES: this shouldnt be here, but is needed in the short term. In the long
     * term we shouldnt have a reference to the database at this location. It
     * is currently needed to construct c_strings during deserialisation of
     * discovery data
     */
};

static os_boolean
in_plugKernelInit(
    in_plugKernel _this,
    u_service service);

static void
in_plugKernelDeinit(
    in_object object);

static void
in_plugKernelGetBaseEntityAction(
   v_entity e,
   c_voidp arg);

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

    IN_TRACE_1(Construction,2,"in_plugKernel created = %x",plug);

    return plug;
}

static void
in_plugKernelGetBaseEntityAction(
   v_entity e,
   c_voidp arg)
{
   c_base* base;

   assert(e);
   assert(arg);

   base = (c_base*)arg;

   *base = c_getBase(v_objectKernel(v_object(e)));
}

static os_boolean
in_plugKernelInit(
    in_plugKernel _this,
    u_service service)
{
    os_boolean success;
    u_result result;
    v_networkReader reader;

    assert(_this);
    assert(service);

    success = in_objectInit(
            in_object(_this),
            IN_OBJECT_KIND_PLUG_KERNEL,
            in_plugKernelDeinit);

    if(success)
    {
        _this->service = service;
        result =
        	u_entityAction(
        			u_entity(_this->service),
        			in_plugKernelResolveNetworkReader, &reader);

        if(result == U_RESULT_OK && reader)
        {
            _this->reader = (u_networkReader)(u_entityNew(v_entity(reader),
                    u_participant(_this->service), FALSE));
        } else
        {
            in_objectDeinit(in_object(_this));
            success = OS_FALSE;
        }
        if(success)
        {
        	/* init with defined value attribute */
        	_this->base = NULL;
        	/* assign base reference */
        	result = u_entityAction (u_entity(service), in_plugKernelGetBaseEntityAction, &(_this->base));
        	if(result != U_RESULT_OK || _this->base == NULL)
        	{
        		IN_REPORT_ERROR(IN_SPOT, "Getting base entity from kernel failed");

        		in_plugKernelDeinit(in_object(_this));
        		success = OS_FALSE;
        	}
        }
    }
    return success;
}

c_base
in_plugKernelGetBase(
    in_plugKernel _this)
{
    assert(_this);

    return _this->base;
}

static void
in_plugKernelDeinit(
    in_object object)
{
    in_plugKernel _this;

    assert(in_plugKernelIsValid(object));

    _this = in_plugKernel(object);
    /* Note: we do not need to release the ->base here, as it is
     * a singleton and it is not ref-counted. */

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

/** */
OS_CLASS(resolveTypeParam);

/** */
OS_STRUCT(resolveTypeParam)
{
    c_char *in_topic_name;
    v_topic out_topic;
};

static void
resolveType (
    v_entity e,
    c_voidp arg)
{
    /* narrow */
    resolveTypeParam inOut =
        (resolveTypeParam) arg;

    v_kernel kernel = v_kernel(v_object(e)->kernel);

    if (kernel && inOut && inOut->in_topic_name) {
        v_topic topic =
            v_lookupTopic(
                kernel,
                inOut->in_topic_name);

        inOut->out_topic = topic;
    }
}

v_topic
in_plugKernelLookupTopic(
    in_plugKernel _this,
    const c_char* topic_name)
{
    v_topic result = NULL;
    OS_STRUCT(resolveTypeParam) arg;
    u_result actionResult;

    assert(in_plugKernelIsValid(_this));

    arg.in_topic_name = (c_char*) topic_name;
    arg.out_topic = NULL;

    actionResult=
        u_entityAction(
                u_entity(_this->service),
                resolveType,
                &arg);

    if (U_RESULT_OK!=actionResult) {
        result = NULL;
    } else {
        result = arg.out_topic;
    }

    return result;
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
    v_subscriber subscriber = NULL;

    reader = (v_networkReader*)args;
    c_walk(v_participant(entity)->entities, in_plugKernelResolveSubscriber, &subscriber);

    if(subscriber){
        c_walk(subscriber->readers, in_plugKernelResolveReader, args);
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


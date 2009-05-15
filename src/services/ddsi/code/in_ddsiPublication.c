/*
 * in_ddsiPublication.c
 *
 */

#include "os_defs.h"
#include "os_heap.h"

#include "Coll_List.h"

#include "in__ddsiPublication.h"
#include "in__locator.h"
#include "in_locatorList.h"

#include "in_ddsiPublication.h"

static os_boolean
in_ddsiDiscoveredWriterDataInit(
    in_ddsiDiscoveredWriterData _this);

static void
in_ddsiDiscoveredWriterDataDeinit(
    in_object obj);


static os_boolean
in_ddsiPublicationBuiltinTopicDataInit(
        in_ddsiPublicationBuiltinTopicData _this)
{
    os_boolean result = OS_TRUE;
    const v_builtinTopicKey KEY_DEF = {0 ,0, 0};

    struct v_publicationInfo *q = &(_this->info);
    memset(_this, 0, sizeof(*_this)); /* paranoid zero out */
    /* does not match DDSi-Guid */
    q->key = KEY_DEF;

    /* does not match DDSi-Guid */
    q->participant_key = KEY_DEF;

    q->topic_name = NULL;
    q->type_name = NULL;
    q->durability.kind = V_DURABILITY_VOLATILE;
    q->deadline.period = C_TIME_INFINITE;
    q->latency_budget.duration = C_TIME_ZERO;
    q->liveliness.kind = V_LIVELINESS_AUTOMATIC;
    q->liveliness.lease_duration = C_TIME_ZERO;
    q->reliability.kind = V_RELIABILITY_BESTEFFORT;
    q->reliability.max_blocking_time = C_TIME_ZERO;
    q->lifespan.duration = C_TIME_INFINITE;
    q->user_data.value = NULL; /* C_ARRAY<c_octet> not supported yet */
    q->ownership.kind = V_OWNERSHIP_SHARED;
    q->ownership_strength.value = 0;
    q->presentation.access_scope = V_PRESENTATION_INSTANCE;
    q->presentation.coherent_access = FALSE;
    q->presentation.ordered_access = FALSE;
    q->partition.name = NULL;
    q->topic_data.value = NULL;
    q->group_data.value = NULL;
    q->destination_order.kind = V_ORDERBY_RECEPTIONTIME;
    q->lifecycle.autodispose_unregistered_instances = TRUE;
    q->lifecycle.autopurge_suspended_samples_delay  = C_TIME_INFINITE;
    q->lifecycle.autounregister_instance_delay      = C_TIME_INFINITE;
    q->alive = TRUE;

    return  result;
}

static void
in_ddsiPublicationBuiltinTopicDataDeinit(
        in_ddsiPublicationBuiltinTopicData _this)
{
    struct v_publicationInfo *q = &(_this->info);

    /* conditional frees */
    if (q->topic_name) os_free(q->topic_name);
    if (q->type_name) os_free(q->type_name);
    if (q->user_data.value) c_free(q->user_data.value);
    /*if (q->partition.name) c_free(q->partition.name);*/
    if (q->topic_data.value) c_free(q->topic_data.value);
    if (q->group_data.value) c_free(q->group_data.value);
 }


/**
 */
static os_boolean
in_ddsiWriterProxyInit(
        in_ddsiWriterProxy _this)
{
    os_boolean result = OS_TRUE;

    /* also inits the remoteWriterGuid */
    memset(&_this->remoteWriterGuid, 0, sizeof(_this->remoteWriterGuid));

    in_locatorListInit(&_this->unicastLocatorList);
    in_locatorListInit(&_this->multicastLocatorList);

    return  result;
}

static void
in_ddsiWriterProxyDeinit(
        in_ddsiWriterProxy _this)
{
    in_locatorListDeinit(&_this->unicastLocatorList);
    in_locatorListDeinit(&_this->multicastLocatorList);
}

in_ddsiDiscoveredWriterData
in_ddsiDiscoveredWriterDataNew(void)
{
    os_boolean success;
    in_ddsiDiscoveredWriterData _this;

    _this = in_ddsiDiscoveredWriterData(os_malloc(OS_SIZEOF(in_ddsiDiscoveredWriterData)));

    if(_this)
    {
        success = in_ddsiDiscoveredWriterDataInit(_this);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}

static os_boolean
in_ddsiDiscoveredWriterDataInit(
    in_ddsiDiscoveredWriterData _this)
{
    os_boolean success;
    assert(_this);

    success =  in_objectInit(in_object(_this),
                             IN_OBJECT_KIND_DISCOVERED_WRITER_DATA,
                             in_ddsiDiscoveredWriterDataDeinit) &&
               in_ddsiPublicationBuiltinTopicDataInit(&_this->topicData) &&
               in_ddsiWriterProxyInit(&_this->proxy);


    return success;
}

static void
in_ddsiDiscoveredWriterDataDeinit(
    in_object obj)
{
    in_ddsiDiscoveredWriterData _this;

    assert(in_ddsiDiscoveredWriterDataIsValid(obj));
    _this = in_ddsiDiscoveredWriterData(obj);

    in_ddsiWriterProxyDeinit(&_this->proxy);
    in_ddsiPublicationBuiltinTopicDataDeinit(&_this->topicData);

    in_objectDeinit(obj);
}

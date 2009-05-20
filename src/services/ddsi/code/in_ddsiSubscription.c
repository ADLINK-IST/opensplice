/*
 * in_ddsiSubscription.c
 *
 */

#include "os_defs.h"
#include "os_heap.h"

#include "Coll_List.h"

#include "in__ddsiSubscription.h"
#include "in__locator.h"
#include "in_locatorList.h"

#include "in_ddsiSubscription.h"

static os_boolean
in_ddsiDiscoveredReaderDataInit(
    in_ddsiDiscoveredReaderData _this);

static void
in_ddsiDiscoveredReaderDataDeinit(
    in_object obj);

static os_boolean
in_ddsiSubscriptionBuiltinTopicDataInit(
        in_ddsiSubscriptionBuiltinTopicData _this)
{
    os_boolean result = OS_TRUE;

    const v_builtinTopicKey KEY_DEF = {0 ,0, 0};

    struct v_subscriptionInfo *q = &(_this->info);
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
    q->ownership.kind = V_OWNERSHIP_SHARED;
    q->destination_order.kind = V_ORDERBY_RECEPTIONTIME;
    q->user_data.value = NULL; /* C_ARRAY<c_octet> not supported yet */
    q->time_based_filter.minSeperation = C_TIME_ZERO;
    q->presentation.access_scope = V_PRESENTATION_INSTANCE;
    q->presentation.coherent_access = FALSE;
    q->presentation.ordered_access = FALSE;
    q->partition.name = NULL;
    q->topic_data.value = NULL;
    q->group_data.value = NULL;
    q->lifespan.duration = C_TIME_INFINITE;
    q->lifespan.used = FALSE;
    return  result;
}

static void
in_ddsiSubscriptionBuiltinTopicDataDeinit(
        in_ddsiSubscriptionBuiltinTopicData _this)
{
    struct v_subscriptionInfo *q = &(_this->info);

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
in_ddsiReaderProxyInit(
        in_ddsiReaderProxy _this)
{
    os_boolean result = OS_TRUE;

    memset(_this, 0, sizeof(*_this));
    memset(&_this->remoteReaderGuid, 0, sizeof(_this->remoteReaderGuid));
    _this->expectsInlineQos = OS_FALSE;

    in_locatorListInit(&_this->unicastLocatorList);
    in_locatorListInit(&_this->multicastLocatorList);

    return  result;
}

static void
in_ddsiReaderProxyDeinit(
        in_ddsiReaderProxy _this)
{
    in_locatorListDeinit(&_this->unicastLocatorList);
    in_locatorListDeinit(&_this->multicastLocatorList);
}

static os_boolean
in_ddsiDiscoveredReaderDataInit(
    in_ddsiDiscoveredReaderData _this)
{
    os_boolean success;
    assert(_this);

    success =
        in_objectInit(in_object(_this),
                IN_OBJECT_KIND_DISCOVERED_READER_DATA,
                in_ddsiDiscoveredReaderDataDeinit) &&
        in_ddsiSubscriptionBuiltinTopicDataInit(&_this->topicData) &&
        in_ddsiReaderProxyInit(&_this->proxy);

    return success;
}

static void
in_ddsiDiscoveredReaderDataDeinit(
    in_object obj)
{
    in_ddsiDiscoveredReaderData _this;

    assert(in_ddsiDiscoveredReaderDataIsValid(obj));
    _this = in_ddsiDiscoveredReaderData(obj);

    in_ddsiReaderProxyDeinit(&_this->proxy);
    in_ddsiSubscriptionBuiltinTopicDataDeinit(&_this->topicData);

    in_objectDeinit(obj);
}


/* **** public functions **** */


in_ddsiDiscoveredReaderData
in_ddsiDiscoveredReaderDataNew(void)
{
    os_boolean success;
    in_ddsiDiscoveredReaderData _this;

    _this = in_ddsiDiscoveredReaderData(os_malloc(OS_SIZEOF(in_ddsiDiscoveredReaderData)));

    if(_this)
    {
        success = in_ddsiDiscoveredReaderDataInit(_this);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}


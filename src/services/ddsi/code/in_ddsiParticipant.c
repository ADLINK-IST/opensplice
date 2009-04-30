/*
 * in_ddsiParticipant.h
 *
 *  Created on: Feb 26, 2009
 *      Author: frehberg
 */
#include <assert.h>

#include "os_defs.h"
#include "os_heap.h"

#include "Coll_List.h"

#include "in__ddsiParticipant.h"
#include "in__locator.h"
#include "in__object.h"
#include "in_locatorList.h"
#include "in__ddsiParticipant.h"
#include "in_report.h"

static os_boolean
in_ddsiDiscoveredParticipantDataInit(
    in_ddsiDiscoveredParticipantData _this);

static void
in_ddsiDiscoveredParticipantDataDeinit(
    in_object obj);


in_ddsiDiscoveredParticipantData
in_ddsiDiscoveredParticipantDataNew(void)
{
    os_boolean success;
    in_ddsiDiscoveredParticipantData _this;

    _this = in_ddsiDiscoveredParticipantData(os_malloc(OS_SIZEOF(in_ddsiDiscoveredParticipantData)));

    if(_this)
    {
        success = in_ddsiDiscoveredParticipantDataInit(_this);

        if(!success)
        {
            os_free(_this);
            _this = NULL;
        }
    }

    return _this;
}


static os_boolean
in_ddsiParticipantBuiltinTopicDataInit(
        in_ddsiParticipantBuiltinTopicData _this)
{
    os_boolean result = OS_TRUE;

    struct v_participantInfo *q =
        &(_this->info);
    memset(_this, 0, sizeof(*_this));

    q->user_data.value                            = NULL;
    q->user_data.size                             = 0;
    /* init array[3] */
    memset(&(q->key), 0, sizeof(q->key));

    return result;
}

static void
in_ddsiParticipantBuiltinTopicDataDeinit(
        in_ddsiParticipantBuiltinTopicData _this)
{
    struct v_participantInfo *q =
        &(_this->info);
    if (q->user_data.value) c_free(q->user_data.value);
}

static os_boolean
in_ddsiParticipantProxyInit(
        in_ddsiParticipantProxy _this)
{
    const OS_STRUCT(in_ddsiProtocolVersion) PROTOCOL_VERSION_DEF =
        IN_DDSI_PROTOCOL_VERSION_2_1;
    const in_ddsiGuidPrefix GUID_PREFIX_DEF =
        IN_GUIDPREFIX_UNKNOWN;
    const in_ddsiVendorId VENDOR_ID_DEF =
        IN_DDSI_VENDOR_UNKNOWN;
    const OS_STRUCT(in_ddsiBuiltinEndpointSet) AVAILABLE_ENDPOINT_SETS_DEF =
        { IN_DISC_BUILTIN_ENDPOINT_SET_ZERO };

    os_boolean result = OS_TRUE;
    memset(_this, 0, sizeof(*_this));

    _this->protocolVersion = PROTOCOL_VERSION_DEF;
    memcpy(&(_this->guidPrefix[0]), GUID_PREFIX_DEF, sizeof(_this->guidPrefix));

    memcpy(&(_this->vendorId[0]), VENDOR_ID_DEF, sizeof(_this->vendorId));

    _this->expectsInlineQos = OS_FALSE;
    _this->availableBuiltinEndpoints = AVAILABLE_ENDPOINT_SETS_DEF;

    in_locatorListInit(&_this->metatrafficUnicastLocatorList);
    in_locatorListInit(&_this->metatrafficMulticastLocatorList);
    in_locatorListInit(&_this->defaultMulticastLocatorList);
    in_locatorListInit(&_this->defaultUnicastLocatorList);
    return result;
}

static void
in_ddsiParticipantProxyDeinit(
        in_ddsiParticipantProxy _this)
{
    in_locatorListDeinit(&_this->metatrafficUnicastLocatorList);
    in_locatorListDeinit(&_this->metatrafficMulticastLocatorList);
    in_locatorListDeinit(&_this->defaultMulticastLocatorList);
    in_locatorListDeinit(&_this->defaultUnicastLocatorList);
}


static os_boolean
in_ddsiDiscoveredParticipantDataInit(
    in_ddsiDiscoveredParticipantData _this)
{
    os_boolean success;
    assert(_this);

    success =  in_objectInit(in_object(_this),
            IN_OBJECT_KIND_DISCOVERED_PARTICIPANT_DATA,
            in_ddsiDiscoveredParticipantDataDeinit);

    if(success)
    {
        success = in_ddsiParticipantBuiltinTopicDataInit(&_this->builtinTopicData);
        if(success)
        {
            success = in_ddsiParticipantProxyInit(&_this->proxy);
        }
        _this->leaseDuration = C_TIME_ZERO;
    }
    return success;
}

static void
in_ddsiDiscoveredParticipantDataDeinit(
    in_object obj)
{
    /* narrow */
    in_ddsiDiscoveredParticipantData _this =
        in_ddsiDiscoveredParticipantData(obj);

    assert(in_ddsiDiscoveredParticipantDataIsValid(obj));

    in_ddsiParticipantBuiltinTopicDataDeinit(&_this->builtinTopicData);
    in_ddsiParticipantProxyDeinit(&_this->proxy);
    _this->leaseDuration = C_TIME_ZERO;
    in_objectDeinit(obj);
}


/*
 * in_endpointDiscoveryData.c
 *
 *  Created on: Mar 19, 2009
 *      Author: frehberg
 */

/* **** interface headers **** */
#include "in__endpointDiscoveryData.h"

/* **** implementation headers **** */
#include "in_ddsiDefinitions.h"
#include "os_heap.h"

/* **** private functions **** */

static void
in_endpointDiscoveryDataDeinit(in_object _this)
{
    in_endpointDiscoveryData narrowed =
        (in_endpointDiscoveryData) _this;

    if (narrowed) {
        in_locatorListDeinit(&narrowed->metatrafficUnicastLocatorList);
        in_locatorListDeinit(&narrowed->metatrafficMulticastLocatorList);
        in_locatorListDeinit(&narrowed->defaultMulticastLocatorList);
        in_locatorListDeinit(&narrowed->defaultUnicastLocatorList);
    }
}
/* **** public functions **** */

in_endpointDiscoveryData
in_endpointDiscoveryDataNew(void)
{
    const OS_STRUCT(in_ddsiProtocolVersion) PROTOCOL_VERSION_2_1 =
        IN_DDSI_PROTOCOL_VERSION_2_1;
    const OS_STRUCT(in_ddsiVendor) VENDOR_PT =
        { IN_DDSI_VENDOR_PT };
    const OS_STRUCT(in_ddsiBuiltinEndpointSet) AVAILABLE_ENDPOINT_SETS_DEF =
        { IN_DISC_BUILTIN_ENDPOINT_SET_DEFAULT };

    in_endpointDiscoveryData result =
       (in_endpointDiscoveryData) os_malloc(sizeof(*result));

    if (result) {
        if (!in_objectInit(OS_SUPER(result),
                IN_OBJECT_KIND_ENDPOINT_DISCOVERY_DATA,
                in_endpointDiscoveryDataDeinit)) {
            os_free(result);
            result = NULL;
        } else {
            result->protocolVersion = PROTOCOL_VERSION_2_1;
            result->vendor = VENDOR_PT;
            result->availableBuiltinEndpoints = AVAILABLE_ENDPOINT_SETS_DEF;

            in_locatorListInit(&result->metatrafficUnicastLocatorList);
            in_locatorListInit(&result->metatrafficMulticastLocatorList);
            in_locatorListInit(&result->defaultMulticastLocatorList);
            in_locatorListInit(&result->defaultUnicastLocatorList);
        }
    }

    return result;
}

os_boolean
in_endpointDiscoveryDataAddDefaultMulticastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc)
{
    os_boolean result;

    result = in_locatorListPushBack(&_this->defaultMulticastLocatorList, loc);

    return result;
}


os_boolean
in_endpointDiscoveryDataAddDefaultUnicastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc)
{
    os_boolean result;

    result = in_locatorListPushBack(&_this->defaultUnicastLocatorList, loc);

    return result;
}


os_boolean
in_endpointDiscoveryDataAddMetatrafficMulticastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc)
{
    os_boolean result;

    in_locatorList *list = &_this->metatrafficMulticastLocatorList;
    result = in_locatorListPushBack(list, loc);

    return result;
}


os_boolean
in_endpointDiscoveryDataAddMetatrafficUnicastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc)
{
    os_boolean result;

    in_locatorList *list = &_this->metatrafficUnicastLocatorList;
    result = in_locatorListPushBack(list, loc);

    return result;
}


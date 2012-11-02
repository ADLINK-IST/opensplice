/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2011 PrismTech
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE
 *
 *   for full copyright notice and license terms.
 *
 */
#ifndef IN__ENDPOINT_DISCOVERY_DATA_H_
#define IN__ENDPOINT_DISCOVERY_DATA_H_

#include "in_endpointDiscoveryData.h"
#include "in_ddsiElements.h"
#include "in_locatorList.h"
#include "in__object.h"


#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(in_endpointDiscoveryData)
{
    OS_EXTENDS(in_object);
    OS_STRUCT(in_ddsiProtocolVersion) protocolVersion;
    OS_STRUCT(in_ddsiVendor) vendor;
    OS_STRUCT(in_ddsiBuiltinEndpointSet) availableBuiltinEndpoints;
    in_locatorList metatrafficUnicastLocatorList;
    in_locatorList metatrafficMulticastLocatorList;
    in_locatorList defaultMulticastLocatorList;
    in_locatorList defaultUnicastLocatorList;
};

#if defined (__cplusplus)
}
#endif


#endif /* IN__ENDPOINT_DISCOVERY_DATA_H_ */

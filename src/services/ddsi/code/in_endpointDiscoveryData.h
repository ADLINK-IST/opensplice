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
#ifndef IN_DISCOVERY_DATA_H_
#define IN_DISCOVERY_DATA_H_

#include "in_commonTypes.h"

#if defined (__cplusplus)
extern "C" {
#endif

/* TODO rename endpointDiscoveryData to serviceConfig */
OS_CLASS(in_endpointDiscoveryData);



in_endpointDiscoveryData
in_endpointDiscoveryDataNew(void);

#define in_endpointDiscoveryData(_o) \
    ((in_endpointDiscoveryData)_o)

#define in_endpointDiscoveryDataKeep(_o) \
    ((in_endpointDiscoveryData) in_objectKeep(in_object(_o)))

#define in_endpointDiscoveryDataFree(_o) \
    in_objectFree(in_object(_o))

/** */
os_boolean
in_endpointDiscoveryDataAddDefaultMulticastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc);

/** */
os_boolean
in_endpointDiscoveryDataAddDefaultUnicastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc);


os_boolean
in_endpointDiscoveryDataAddMetatrafficMulticastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc);

os_boolean
in_endpointDiscoveryDataAddMetatrafficUnicastLocator(
        in_endpointDiscoveryData _this,
        in_locator loc);

#if defined (__cplusplus)
}
#endif


#endif /* IN_DISCOVERY_DATA_H_ */

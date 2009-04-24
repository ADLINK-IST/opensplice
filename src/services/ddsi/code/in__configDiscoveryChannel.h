/* Prevent failure due to multiple inclusion of this file. */
#ifndef IN_CONFIG_DISCOVERY_CHANNEL_H
#define IN_CONFIG_DISCOVERY_CHANNEL_H

/* OS abstraction includes. */
#include "in__object.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif



/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_configDiscoveryChannel(_this) ((in_configDiscoveryChannel)_this)

in_configDiscoveryChannel
in_configDiscoveryChannelNew(
    os_boolean isEnabled,
    in_configDdsiService owningService);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_DISCOVERY_CHANNEL_H */

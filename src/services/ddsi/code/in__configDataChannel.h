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
#ifndef IN_CONFIG_DATA_CHANNEL_H
#define IN_CONFIG_DATA_CHANNEL_H

#include "in__object.h"
#include "in__configDdsiService.h"

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
#define in_configDataChannel(_this) ((in_configDataChannel)_this)

in_configDataChannel
in_configDataChannelNew(
    os_char* name,
    os_uint32 priority,
    os_boolean isDefault,
    os_boolean isEnabled,
    in_configDdsiService owningService);

void
in_configDataChannelSetGroupQueueSize(
    in_configDataChannel _this,
    os_uint32 queueSize);

void
in_configDataChannelSetFragmentSize(
    in_configDataChannel _this,
    os_uint32 fragmentSize);

os_char*
in_configDataChannelGetName(
    in_configDataChannel _this);

/* \brief check */
os_boolean
in_configDataChannelHasName(
    in_configDataChannel _this,
    const os_char *name);

os_uint32
in_configDataChannelGetPriority(
    in_configDataChannel _this);

os_boolean
in_configDataChannelGetIsDefault(
    in_configDataChannel _this);


os_uint32
in_configDataChannelGetGroupQueueSize(
    in_configDataChannel _this);

os_uint32
in_configDataChannelGetFragmentSize(
    in_configDataChannel _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CONFIG_DATA_CHANNEL_H */

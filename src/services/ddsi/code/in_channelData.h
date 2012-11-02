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
#ifndef IN_CHANNEL_DATA_H
#define IN_CHANNEL_DATA_H

#include "in__object.h"
#include "u_networkReader.h"
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
#define in_channelData(_this) ((in_channelData)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_channelDataIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_channelDataKeep(_this) in_channelData(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_channelDataFree(_this) in_objectFree(in_object(_this))

in_channelData
in_channelDataNew(
    in_configChannel config,
    in_stream stream,
    in_plugKernel plug,
    in_endpointDiscoveryData discoveryData);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_DATA_H */


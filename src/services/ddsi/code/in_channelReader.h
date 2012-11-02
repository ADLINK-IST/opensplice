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
#ifndef IN_CHANNEL_READER_H
#define IN_CHANNEL_READER_H

#include "in__object.h"
#include "in_runnable.h"
/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif


OS_STRUCT(in_channelReader)
{
    OS_EXTENDS(in_runnable);
    in_objectRef channel;
};

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_channelReader(_this) ((in_channelReader)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_channelReaderIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_channelReaderKeep(_this) in_channelReader(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_channelReaderFree(_this) in_objectFree(in_object(_this))

os_boolean
in_channelReaderInit(
    in_channelReader _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    const os_char* name,
    const os_char* pathName,
    const in_runnableMainFunc runnableMainFunc,
    const in_runnableTriggerFunc triggerFunc,
    in_channel channel);

void
in_channelReaderDeinit(
    in_object _this);

in_channel
in_channelReaderGetChannel(
    in_channelReader _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_READER_H */


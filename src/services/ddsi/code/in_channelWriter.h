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
#ifndef IN_CHANNEL_WRITER_H
#define IN_CHANNEL_WRITER_H

/* OS abstraction includes. */
#include "os_defs.h"
#include "os_classbase.h"
#include "os_stdlib.h"

#include "in_runnable.h"
#include "in_channel.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif


OS_STRUCT(in_channelWriter)
{
    OS_EXTENDS(in_runnable);
    in_objectRef channel;
};

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_channelWriter(_this) ((in_channelWriter)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_channelWriterIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_channelWriterKeep(_this) in_channelWriter(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_channelWriterFree(_this) in_objectFree(in_object(_this))

os_boolean
in_channelWriterInit(
    in_channelWriter _this,
    in_channel channel,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    const os_char* name,
    const os_char* pathName,
    const in_runnableMainFunc runnableMainFunc,
    const in_runnableTriggerFunc triggerFunc);

void
in_channelWriterDeinit(
    in_object _this);

in_channel
in_channelWriterGetChannel(
    in_channelWriter _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_WRITER_H */


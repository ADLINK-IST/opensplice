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
#ifndef IN_CHANNEL_H
#define IN_CHANNEL_H

#include "in__object.h"
#include "in_stream.h"
#include "in_channelReader.h"
#include "in_channelWriter.h"
#include "in__plugKernel.h"
/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(in_channel)
{
    OS_EXTENDS(in_object);
    in_stream stream;
    in_channelWriter writer;
    in_channelReader reader;
    in_plugKernel plug;
    in_configChannel config;
};

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_channel(_this) ((in_channel)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_channelIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_channelKeep(_this) in_channel(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_channelFree(_this) in_objectFree(in_object(_this))

os_boolean
in_channelInit(
    in_channel _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_stream stream,
    in_plugKernel plug,
    in_configChannel config);

in_configChannel
in_channelGetConfig(
    in_channel _this);

void
in_channelDeinit(
    in_object _this);

in_plugKernel
in_channelGetPlugKernel(
    in_channel _this);

in_channelReader
in_channelGetReader(
    in_channel _this);

in_channelWriter
in_channelGetWriter(
    in_channel _this);

void
in_channelSetReader(
    in_channel _this,
    in_channelReader reader);

void
in_channelSetWriter(
    in_channel _this,
    in_channelWriter writer);

in_stream
in_channelGetStream(
    in_channel _this);

void
in_channelActivate(
    in_channel _this);

void
in_channelDeactivate(
    in_channel _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_CHANNEL_H */


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
#ifndef IN_STREAM_H
#define IN_STREAM_H

/* OS abstraction includes. */
#include "in__object.h"
#include "in_streamReader.h"
#include "in_streamWriter.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(in_stream)
{
    OS_EXTENDS(in_object);
    in_streamReader reader;
    in_streamWriter writer;
};

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_stream(_this) ((in_stream)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_streamIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_streamKeep(_this) in_stream(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_streamFree(_this) in_objectFree(in_object(_this))

os_boolean
in_streamInit(
    in_stream _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_streamReader reader,
    in_streamWriter writer);

void
in_streamDeinit(
    in_object _this);

in_streamReader
in_streamGetReader(
    in_stream _this);

in_streamWriter
in_streamGetWriter(
    in_stream _this);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_STREAM_H */


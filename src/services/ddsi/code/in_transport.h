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
#ifndef IN_TRANSPORT_H
#define IN_TRANSPORT_H

/* OS abstraction includes. */
#include "in__object.h"
#include "in_transportReceiver.h"
#include "in_transportSender.h"

/**
 * Allow usage of this C code from C++ code.
 */
#if defined (__cplusplus)
extern "C" {
#endif

OS_STRUCT(in_transport)
{
    OS_EXTENDS(in_object);
    in_transportSender sender;
    in_transportReceiver receiver;
};

/**
 * Macro that allows the implementation of type checking when casting an
 * object. The signature of the 'casting macro' must look like this:
 */
#define in_transport(_this) ((in_transport)_this)

/**
 * Macro that calls the in_objects validity check function with the
 * appropiate type
 */
#define in_transportIsValid(_this) in_objectIsValid(in_object(_this))

/**
 * Macro to make the in_objectKeep operation type specific
 */
#define in_transportKeep(_this) in_transport(in_objectKeep(in_object(_this)))

/**
 * Macro to make the in_objectFree operation type specific
 */
#define in_transportFree(t) in_objectFree(in_object(t))


os_boolean
in_transportInit(
    in_transport _this,
    in_objectKind kind,
    in_objectDeinitFunc deinit,
    in_transportReceiver receiver,
    in_transportSender sender);

void
in_transportDeinit(
    in_object _this);

in_transportReceiver
in_transportGetReceiver(
    in_transport _this);

in_transportSender
in_transportGetSender(
    in_transport _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_TRANSPORT_H */


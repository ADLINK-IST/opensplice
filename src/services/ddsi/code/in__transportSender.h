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
#ifndef IN__TRANSPORTSENDER_H_
#define IN__TRANSPORTSENDER_H_

/* public interface*/
#include "in_transportSender.h"

/* dependencies */
#include "in__object.h"
#include "in_abstractSendBuffer.h"
#include "c_time.h"
#include "in_locator.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif

/** \brief virtual operations */
typedef OS_STRUCT(in_abstractSendBuffer)* (*in_transportSenderGetBufferFunc)(in_transportSender writer);
typedef os_result (*in_transportSenderSendToFunc)(in_transportSender _this, in_locator sendTo, in_abstractSendBuffer buffer, os_size_t length, os_boolean isControl, os_time *timeout);
typedef void (*in_transportSenderPeriodicActionFunc)(in_transportSender writer, os_time *timeout);
typedef void (*in_transportSenderReleaseBufferFunc)(in_transportSender writer, in_abstractSendBuffer iBuffer);
typedef OS_STRUCT(in_locator)* (*in_transportSenderGetDataMulticastLocatorFunc)(in_transportSender writer);
typedef OS_STRUCT(in_locator)* (*in_transportSenderGetDataUnicastLocatorFunc)(in_transportSender writer);
typedef OS_STRUCT(in_locator)* (*in_transportSenderGetCtrlUnicastLocatorFunc)(in_transportSender writer);


/* */
OS_CLASS(in_transportSenderPublicVTable);
OS_STRUCT(in_transportSenderPublicVTable)
{
    in_transportSenderGetBufferFunc      getBuffer;
    in_transportSenderSendToFunc         sendTo;
    in_transportSenderPeriodicActionFunc periodicAction;
    in_transportSenderReleaseBufferFunc  releaseBuffer;
    in_transportSenderGetDataUnicastLocatorFunc getDataUnicastLocator;
    in_transportSenderGetDataMulticastLocatorFunc getDataMulticastLocator;
    in_transportSenderGetCtrlUnicastLocatorFunc getCtrlUnicastLocator;
};


/** \brief abstract class  */
OS_STRUCT(in_transportSender)
{
    OS_EXTENDS(in_object);
    /* referencing a static memory area*/
    in_transportSenderPublicVTable publicVTable;
};

/** \brief init */
void
in_transportSenderInitParent(
		in_transportSender _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		/* must reference a static memory area*/
		in_transportSenderPublicVTable staticPublicVTable);

/** \brief deinit */
void
in_transportSenderDeinit(
		in_transportSender _this);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN__TRANSPORTSENDER_H_ */

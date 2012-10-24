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
#ifndef IN__TRANSPORTRECEIVER_H_
#define IN__TRANSPORTRECEIVER_H_

#include "in_transportReceiver.h"

#include "in__object.h"
#include "in_abstractReceiveBuffer.h"
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
typedef OS_STRUCT(in_abstractReceiveBuffer)* (*in_transportReceiverReceiveFunc)(in_transportReceiver reader, os_boolean *isControl, os_time *timeout, in_result *errorState);
typedef void (*in_transportReceiverPeriodicActionFunc)(in_transportReceiver reader, os_time *timeout);
typedef void (*in_transportReceiverReleaseBufferFunc)(in_transportReceiver reader, in_abstractReceiveBuffer  buffer);
typedef OS_STRUCT(in_locator)* (*in_transportReceiverGetLocatorFunc)(in_transportReceiver reader);

/** \brief  vtable  */
OS_CLASS(in_transportReceiverPublicVTable);
/** \brief vtable  */
OS_STRUCT(in_transportReceiverPublicVTable)
{
    in_transportReceiverReceiveFunc    receive;
    in_transportReceiverPeriodicActionFunc periodicAction;
    in_transportReceiverReleaseBufferFunc  releaseBuffer;
    in_transportReceiverGetLocatorFunc getDataUnicastLocator;
    in_transportReceiverGetLocatorFunc getDataMulticastLocator;
    in_transportReceiverGetLocatorFunc getCtrlUnicastLocator;
    in_transportReceiverGetLocatorFunc getCtrlMulticastLocator;
};

/** \brief abstract class  */
OS_STRUCT(in_transportReceiver)
{
	OS_EXTENDS(in_object);
	/* referencing static structure */
	in_transportReceiverPublicVTable publicVTable;
};

/** \brief init */
void
in_transportReceiverInitParent(
		in_transportReceiver _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_transportReceiverPublicVTable staticPublicVTable);

/** \brief deinit */
void
in_transportReceiverDeinit(
		in_transportReceiver _this);

/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN__TRANSPORTRECEIVER_H_ */

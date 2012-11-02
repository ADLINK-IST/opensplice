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
#ifndef IN_TRANSPORTSENDER_H_
#define IN_TRANSPORTSENDER_H_

#include "in_locator.h"
#include "in_abstractSendBuffer.h"
#include "in_commonTypes.h"
#include "os_time.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif


/** \brief narrow */
#define in_transportSender(_o) \
	((in_transportSender)_o)

#define in_transportSenderIsValid(t) in_objectIsValid(in_object(t))

#define in_transportSenderKeep(t) \
    in_transportSender(in_objectKeep(in_object(t)))

#define in_transportSenderFree(t) in_objectFree(in_object(t))

/** \brief virtual function */
os_result
in_transportSenderSendTo(
		in_transportSender _this,
		in_locator sendTo,
		in_abstractSendBuffer buffer,
		os_size_t length,
		os_boolean isControl,
		os_time *timeout);

/** \brief virtual function */
in_abstractSendBuffer
in_transportSenderGetBuffer(
		in_transportSender _this);

/** \brief virtual function */
void
in_transportSenderPeriodicAction(
		in_transportSender _this,
		os_time *timeout);

/** \brief virtual function */
void
in_transportSenderReleaseBuffer(
		in_transportSender _this,
		in_abstractSendBuffer buffer);

/** \brief virtual function */
in_locator
in_transportSenderGetDataUnicastLocator(
        in_transportSender reader);

/** \brief virtual function */
in_locator
in_transportSenderGetDataMulticastLocator(
        in_transportSender reader);

/** \brief virtual function */
in_locator
in_transportSenderGetCtrlUnicastLocator(
        in_transportSender reader);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_TRANSPORTSENDER_H_ */

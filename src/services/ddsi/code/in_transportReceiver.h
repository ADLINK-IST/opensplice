/*
 *                         OpenSplice DDS
 *
 *   This software and documentation are Copyright 2006 to 2009 PrismTech 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $OSPL_HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */
/*
 * in_transportReceiver.h
 *
 *  Created on: Feb 10, 2009
 *      Author: frehberg
 */

#ifndef IN_TRANSPORTRECEIVER_H_
#define IN_TRANSPORTRECEIVER_H_

#include "os_time.h"

#include "in_locator.h"
#include "in_abstractReceiveBuffer.h"
#include "in_commonTypes.h"
#include "in_result.h"
/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif


/** \brief narrow */
#define in_transportReceiver(_o) \
	((in_transportReceiver)_o)

/** \brief virtual function */
in_abstractReceiveBuffer
in_transportReceiverReceive(
		in_transportReceiver reader,
		os_boolean *isControl,
		os_time *timeout,
		in_result *errorState);

/** \brief virtual function */
void
in_transportReceiverPeriodicAction(
		in_transportReceiver reader,
		os_time *timeout);

/** \brief virtual function */
void
in_transportReceiverReleaseBuffer(
		in_transportReceiver reader,
		in_abstractReceiveBuffer buffer);

/** \brief virtual function */
in_locator
in_transportReceiverGetDataUnicastLocator(
		in_transportReceiver reader);

/** \brief virtual function */
in_locator
in_transportReceiverGetDataMulticastLocator(
		in_transportReceiver reader);

/** \brief virtual function */
in_locator
in_transportReceiverGetCtrlUnicastLocator(
		in_transportReceiver reader);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_TRANSPORTRECEIVER_H_ */

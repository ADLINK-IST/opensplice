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
 * in__dataBuffer.h
 *
 *  Created on: Feb 8, 2009
 *      Author: frehberg
 */

#ifndef IN_ABSTRACTRECEIVEBUFFER_h_
#define IN_ABSTRACTRECEIVEBUFFER_h_

/* interface */
#include "in_abstractReceiveBuffer.h"

/* implementation */
#include "in__object.h"
#include "in_align.h"
#include "in__locator.h"

/**
 * Allow usage of this C code from C++ code. Always include this in a header
 * file.
 */
#if defined (__cplusplus)
extern "C" {
#endif


/** \brief first octet alignment constraint for all buffers */
#define IN_RECEIVE_BUFFER_ALIGN_BOUNDARY (8L)

#define IN_RECEIVE_BUFFER_ALIGN_PTR_CEIL(_ptr) \
	IN_ALIGN_PTR_CEIL((_ptr),IN_RECEIVE_BUFFER_ALIGN_BOUNDARY)

#define IN_RECEIVE_BUFFER_ALIGN_LENGTH_FLOOR(_len) \
	IN_ALIGN_UINT_FLOOR((_len),IN_RECEIVE_BUFFER_ALIGN_BOUNDARY)

/** \brief abstract class */
OS_STRUCT(in_abstractReceiveBuffer)
{
	OS_EXTENDS(in_object);
	in_octet *buffer;
	in_ulong  length; /* Note: ideal for pointer arithmetic */
	OS_STRUCT(in_locator) sender; /* could be extended in
									 future to hold senders credentials */
};

/** \brief init */
void
in_abstractReceiveBufferInitParent(
		in_abstractReceiveBuffer _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_octet *buffer,
		in_ulong length);

/** \brief deinit */
void
in_abstractReceiveBufferDeinit(in_abstractReceiveBuffer _this);

/** \brief set the length of the buffer
 *
 * Number of valid octets in the buffer. */
void
in_abstractReceiveBufferSetLength(in_abstractReceiveBuffer _this, in_long length);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN_ABSTRACTRECEIVEBUFFER_h_ */

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
 * in_receiveBuffer.c
 *
 *  Created on: Feb 9, 2009
 *      Author: frehberg
 */

/* interface */
#include "in_receiveBuffer.h"
#include "in__receiveBuffer.h"

/* implementation */

#include <assert.h>
#include "string.h" /* memset */
#include "in__abstractReceiveBuffer.h"
#include "os_heap.h"


OS_STRUCT(in_receiveBuffer)
{
	OS_EXTENDS(in_abstractReceiveBuffer);
	/* the real length of buffer in_abstractReceiveBufferLength may be less */
	in_long  bufferLength;
	in_octet buffer[4]; /* variable array buffer[bufferLength],
	                       should be zero-sized array, but not accepted by
	                       integrity C-compiler  */
	/* Note: no further struct members must follow */
};

/** \brief deinit */
static void
in_receiveBufferDeinit(in_receiveBuffer _this)
{
	in_abstractReceiveBufferDeinit(OS_SUPER(_this));
}

/** \brief init */
static void
in_receiveBufferInit(in_receiveBuffer _this, in_ulong bufferLength)
{
	in_octet *bufferPtrAligned =
		IN_RECEIVE_BUFFER_ALIGN_PTR_CEIL(_this->buffer);
	const in_ulong bufferLengthFloor =
		IN_RECEIVE_BUFFER_ALIGN_LENGTH_FLOOR(bufferLength);

	assert(bufferPtrAligned >= _this->buffer);
	assert(bufferPtrAligned < _this->buffer + IN_RECEIVE_BUFFER_ALIGN_BOUNDARY);
	assert(bufferLengthFloor <= bufferLength);

	in_abstractReceiveBufferInitParent(OS_SUPER(_this),
			IN_OBJECT_KIND_RECEIVE_BUFFER_BASIC,
			(in_objectDeinitFunc) in_receiveBufferDeinit,
			bufferPtrAligned,
			bufferLengthFloor);
	_this->bufferLength = bufferLength;
	/* zero buffer */
	memset(_this->buffer, 0, bufferLength);
}


/** \brief constructor */
in_receiveBuffer
in_receiveBufferNew(in_long capacity)
{
	const os_size_t capacityWithAlignmentOffset =
		(sizeof(in_octet)*capacity) +
		IN_RECEIVE_BUFFER_ALIGN_BOUNDARY-1;

	const os_size_t objectSize =
		OS_SIZEOF(in_receiveBuffer) +
		capacityWithAlignmentOffset;

	in_receiveBuffer result =
		in_receiveBuffer(os_malloc(objectSize));

	assert(capacity > 0);

	if (result) {
		in_receiveBufferInit(result, capacity);
	}

	return result;
}

/** \brief destructor */
void
in_receiveBufferFree(in_receiveBuffer _this)
{
	in_abstractReceiveBufferFree(OS_SUPER(_this));
}

/* \brief reset the length attribute and check locator's refcounter is set to 1 */
void
in_receiveBufferReset(in_receiveBuffer _this)
{
	in_locator embeddedLocator =
		in_abstractReceiveBufferGetSender((in_abstractReceiveBuffer)_this);

	in_abstractReceiveBufferSetLength(OS_SUPER(_this), _this->bufferLength);
	/* The refcount must be 1, lifecycle of locator is bound to databuffer */
	assert(in_objectGetRefCount(in_object(embeddedLocator)) == 1);
}

/* \brief protected */
void
in_receiveBufferSetLength(in_receiveBuffer _this, in_long length)
{
	assert(length <= _this->bufferLength);
	in_abstractReceiveBufferSetLength(in_abstractReceiveBuffer(_this), length);
}

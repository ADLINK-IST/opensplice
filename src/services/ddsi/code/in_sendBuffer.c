/*
 * in_sendBuffer.c
 *
 *  Created on: Feb 9, 2009
 *      Author: frehberg
 */

/* interface */
#include "in__abstractSendBuffer.h"
#include "in__sendBuffer.h"

/* implementation */
#include <assert.h>
#include "string.h" /* memset */
#include "os_heap.h"


OS_STRUCT(in_sendBuffer)
{
	OS_EXTENDS(in_abstractSendBuffer);
	/* the real length of buffer in_abstractSendBufferLength may be less */
	in_long  bufferLength;
	in_octet buffer[4]; /* variable array buffer[bufferLength],
	                       should be zero-sized array, but not accepted by
	                       integrity C-compiler  
	                       FIXME (mj): How will this work on 64 bit OS?!?: */
	
	/* Note: no further struct members must follow */
};

/** \brief deinit */
static void
in_sendBufferDeinit(in_sendBuffer _this)
{
	in_abstractSendBufferDeinit(OS_SUPER(_this));
}

/** \brief init */
static void
in_sendBufferInit(in_sendBuffer _this, in_long bufferLength)
{
	in_octet *bufferPtrAligned =
		IN_SEND_BUFFER_ALIGN_PTR_CEIL(_this->buffer);
	const in_long bufferLengthFloor =
		IN_SEND_BUFFER_ALIGN_LENGTH_FLOOR(bufferLength);

	assert(bufferPtrAligned >= _this->buffer);
	assert(bufferPtrAligned < _this->buffer + IN_SEND_BUFFER_ALIGN_BOUNDARY);
	assert(bufferLengthFloor <= bufferLength);

	in_abstractSendBufferInitParent(OS_SUPER(_this),
			IN_OBJECT_KIND_SEND_BUFFER_BASIC,
			(in_objectDeinitFunc) in_sendBufferDeinit,
			bufferPtrAligned,
			bufferLengthFloor);
	_this->bufferLength = bufferLength;
	/* zero buffer */
	memset(_this->buffer, 0, bufferLength);
}


/** \brief constructor */
in_sendBuffer
in_sendBufferNew(in_long capacity)
{
	const os_size_t capacityWithAlignmentOffset =
		(sizeof(in_octet)*capacity) +
		IN_SEND_BUFFER_ALIGN_BOUNDARY-1;

	const os_size_t objectSize =
		OS_SIZEOF(in_sendBuffer) +
		capacityWithAlignmentOffset;

	in_sendBuffer result =
		in_sendBuffer(os_malloc(objectSize));

	assert(capacity > 0);

	if (result) {
		in_sendBufferInit(result, capacity);
	}

	return result;
}

/** \brief destructor */
void
in_sendBufferFree(in_sendBuffer _this)
{
	in_abstractSendBufferFree(OS_SUPER(_this));
}

/* \brief reset the length attribute and check locator's refcounter is set to 1 */
void
in_sendBufferReset(in_sendBuffer _this)
{
	in_abstractSendBufferSetLength(OS_SUPER(_this), _this->bufferLength);
}

/* \brief protected */
void
in_sendBufferSetLength(in_sendBuffer _this, in_long length)
{
	assert(length <= _this->bufferLength);
	in_abstractSendBufferSetLength(in_abstractSendBuffer(_this), length);
}

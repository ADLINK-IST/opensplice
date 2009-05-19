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
#include "in_report.h"

OS_STRUCT(in_receiveBuffer)
{
	OS_EXTENDS(in_abstractReceiveBuffer);
	/* the real length of buffer in_abstractReceiveBufferLength may be less */
	in_long  bufferLength;
	in_octet buffer[4]; /* variable array buffer[bufferLength],
	                       should be zero-sized array, but not accepted by
	                       "Integrity C-compiler"  */
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
in_receiveBufferInit(
        in_receiveBuffer _this,
        os_size_t bufferLength)
{
    /* the followin calculation ensures that first and last octet of buffer is
     * aligned by IN_RECEIVE_BUFFER_ALIGN_BOUNDARY (8U). This will allow us to handle
     * CDR fragmentation constraints more performant, where the first octet in each fragment
     * and last octet of fragment must be aligned to this boundary.  */
	in_octet *bufferPtrAlignedCeil =
		IN_RECEIVE_BUFFER_ALIGN_PTR_CEIL(_this->buffer); /* first octet of zero sized array */

	assert(bufferPtrAlignedCeil >= _this->buffer);
	assert(bufferPtrAlignedCeil < _this->buffer + IN_RECEIVE_BUFFER_ALIGN_BOUNDARY);

	in_abstractReceiveBufferInitParent(OS_SUPER(_this),
			IN_OBJECT_KIND_RECEIVE_BUFFER_BASIC,
			(in_objectDeinitFunc) in_receiveBufferDeinit,
			bufferPtrAlignedCeil,
			bufferLength); /* use the length declared by user */

	/* store the real buffer size, required to reset the buffer */
	_this->bufferLength = bufferLength;
}


/** \brief constructor */
in_receiveBuffer
in_receiveBufferNew(os_size_t capacity)
{
	const os_size_t capacityWithAlignOffset = /* take into account alignment offsets */
		(sizeof(in_octet)*capacity) +
		IN_RECEIVE_BUFFER_ALIGN_BOUNDARY-1; /* alignment at begin */

	const os_size_t objectSize =
		OS_SIZEOF(in_receiveBuffer) +
		capacityWithAlignOffset;

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
	in_locatorFree(embeddedLocator);
	assert(in_objectGetRefCount(in_object(embeddedLocator)) == 1);
}

/* \brief protected */
void
in_receiveBufferSetLength(in_receiveBuffer _this, in_long length)
{
	assert(length <= _this->bufferLength);
	in_abstractReceiveBufferSetLength(in_abstractReceiveBuffer(_this), length);
}

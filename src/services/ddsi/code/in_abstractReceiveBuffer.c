/*
 * in_abstractReceiveBuffer.c
 *
 *  Created on: Feb 8, 2009
 *      Author: frehberg
 */

#include "in__abstractReceiveBuffer.h"
#include "in__object.h"

void
in_abstractReceiveBufferInitParent(
		in_abstractReceiveBuffer _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_octet *buffer,
		os_size_t length)
{
	OS_STRUCT(in_address) addr;
	in_objectInit(OS_SUPER(_this), kind, deinit);
	_this->buffer = buffer;
	_this->length = length;
	in_addressInitAny(&addr);
	_this->sender = in_locatorNew(0, &addr);
}

void
in_abstractReceiveBufferDeinit(in_abstractReceiveBuffer _this)
{
	in_locatorFree(_this->sender);
	in_objectDeinit(OS_SUPER(_this));
}


/** \brief decrease refcount */
void
in_abstractReceiveBufferFree(in_abstractReceiveBuffer _this)
{
	in_objectFree(OS_SUPER(_this));
}


/** \brief return pointer to first octet in buffer */
in_octet*
in_abstractReceiveBufferBegin(in_abstractReceiveBuffer dataBuffer)
{
	return dataBuffer->buffer;
}

/** \brief return number of octets in buffer
 */
os_size_t
in_abstractReceiveBufferLength(in_abstractReceiveBuffer dataBuffer)
{
	return dataBuffer->length;
}

/** \brief set the length of the buffer
 *
 * Number of valid octets in the buffer. */
void
in_abstractReceiveBufferSetLength(in_abstractReceiveBuffer _this, os_size_t length)
{
	_this->length = length;
}


in_locator
in_abstractReceiveBufferGetSender(in_abstractReceiveBuffer _this)
{
	return in_locatorKeep(_this->sender);
}

void
in_abstractReceiveBufferSetSender(in_abstractReceiveBuffer _this, in_locator locator)
{
	in_locatorFree(_this->sender);
	_this->sender = in_locatorKeep(locator);
}



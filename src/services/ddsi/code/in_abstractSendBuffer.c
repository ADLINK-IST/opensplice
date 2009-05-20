/*
 * in_abstractSendBuffer.c
 *
 *  Created on: Feb 8, 2009
 *      Author: frehberg
 */

#include "in__abstractSendBuffer.h"
#include "in__object.h"

/** \brief destructor */
void
in_abstractSendBufferFree(in_abstractSendBuffer _this)
{
	in_objectFree(in_object(_this));
}

void
in_abstractSendBufferInitParent(
		in_abstractSendBuffer _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_octet *buffer,
		in_long length)
{
	in_objectInit(OS_SUPER(_this), kind, deinit);
	_this->buffer = buffer;
	_this->length = length;
}

void
in_abstractSendBufferDeinit(in_abstractSendBuffer _this)
{
	in_objectDeinit(OS_SUPER(_this));
}


/** \brief return pointer to first octet in buffer */
in_octet*
in_abstractSendBufferBegin(in_abstractSendBuffer dataBuffer)
{
	return dataBuffer->buffer;
}

/** \brief return number of octets in buffer
 */
in_ulong
in_abstractSendBufferLength(in_abstractSendBuffer dataBuffer)
{
	return dataBuffer->length;
}

/** \brief set the length of the buffer
 *
 * Number of valid octets in the buffer. */
void
in_abstractSendBufferSetLength(in_abstractSendBuffer _this, in_long length)
{
	_this->length = length;
}



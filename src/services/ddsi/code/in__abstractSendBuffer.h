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
#ifndef IN__ABSTRACTSENDBUFFER_H_
#define IN__ABSTRACTSENDBUFFER_H_

/* interface */
#include "in_abstractSendBuffer.h"

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
#define IN_SEND_BUFFER_ALIGN_BOUNDARY (8L)

#define IN_SEND_BUFFER_ALIGN_PTR_CEIL(_ptr) \
	IN_ALIGN_PTR_CEIL((_ptr),IN_SEND_BUFFER_ALIGN_BOUNDARY)

#define IN_SEND_BUFFER_ALIGN_LENGTH_FLOOR(_len) \
	IN_ALIGN_UINT_FLOOR((_len),IN_SEND_BUFFER_ALIGN_BOUNDARY)

/** \brief abstract class */
OS_STRUCT(in_abstractSendBuffer)
{
	OS_EXTENDS(in_object);
	in_octet *buffer;
	in_ulong   length;
};

/** \brief init */
void
in_abstractSendBufferInitParent(
		in_abstractSendBuffer _this,
		in_objectKind kind,
		in_objectDeinitFunc deinit,
		in_octet *buffer,
		in_long length);

/** \brief deinit */
void
in_abstractSendBufferDeinit(in_abstractSendBuffer _this);

/** \brief set the length of the buffer
 *
 * Number of valid octets in the buffer. */
void
in_abstractSendBufferSetLength(in_abstractSendBuffer _this, in_long length);


/* Close the brace that allows the usage of this code in C++. */
#if defined (__cplusplus)
}
#endif

#endif /* IN__ABSTRACTSENDBUFFER_H_ */
